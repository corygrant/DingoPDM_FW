/*
 * dingo_pdm.c
 *
 *  Created on: Oct 22, 2020
 *      Author: coryg
 */

#include "dingo_pdm.h"
#include "pdm_config.h"
#include "main.h"

//TODO V3 Add PCA9539 Profet GPIO reset pin logic

//========================================================================
// Task Delays
//========================================================================
#define MAIN_TASK_DELAY 10
#define I2C_TASK_DELAY 6
#define CAN_TX_DELAY 50

//========================================================================
// I2C Addresses
//========================================================================
#define PCA9635_ADDRESS 0x30
#define MCP9808_ADDRESS 0x18
#define PCA9539_ADDRESS_BANK1 0x74
#define PCA9539_ADDRESS_BANK2 0x74
#define MAX11613_ADDRESS_PF_BANK1 0x34
#define MAX11613_ADDRESS_PF_BANK2 0x34
#define PCAL9554B_ADDRESS 0x20
#define MB85RC_ADDRESS 0x50

//========================================================================
// LED Flash
//========================================================================
#define PCA9635_FLASH_FREQ 2 //Period(s) = (PCA9635_FLASH_FREQ + 1) / 24
#define PCA9635_FLASH_DUTY_CYCLE 128 //Duty Cycle = PCA9635_FLASH_DUTY_CYCLE / 256

//========================================================================
// Board Temperature Limits
//========================================================================
#define BOARD_TEMP_MAX 50
#define BOARD_TEMP_MIN 0
#define BOARD_TEMP_CRIT 80

//========================================================================
// Profet DSEL and DEN Pins
//========================================================================
#define PF_BANK1_DSEL 0x2200
#define PF_BANK2_DSEL 0x0220
#define PF_BANK1_DEN 0x4441
#define PF_BANK2_DEN 0x0445

//========================================================================
// CAN Termination
//========================================================================
#define CAN_TERM 0x8000

//========================================================================
// STM ADC Counts
//========================================================================
#define ADC_1_COUNT 1
#define ADC_4_COUNT 1

//========================================================================
// CAN
//========================================================================
#define CAN_TX_BASE_ID 2000
#define CAN_TX_MSG_SPLIT 1 //ms

//========================================================================
// STM Internal Calibration Voltages
//========================================================================
#define STM32_TEMP_CALIB_VOLT 3.3
#define STM32_TEMP_REF_VOLT 3.3

//========================================================================
// LED Test Sequence
//========================================================================
#define LED_TEST_SEQ_DELAY 50

//==============================================================================================================================================
//========================================================================
// PDM Config
//========================================================================
PdmConfig_t stPdmConfig;

//========================================================================
// Message Queues
//========================================================================
osMessageQueueId_t qMsgQueueRx;
osMessageQueueId_t qMsgQueueTx;

//========================================================================
// Mode/State Enums
//========================================================================
DeviceMode_t eDevMode;
DeviceState_t eDevState;

//========================================================================
// Profets
// I2C GPIO Outputs
// I2C ADC Inputs
//========================================================================
volatile ProfetTypeDef pf[PDM_NUM_OUTPUTS];
volatile uint16_t nILTotal;
uint16_t pfGpioBank1, pfGpioBank2;
uint16_t nPfISBank1Raw[4], nPfISBank2Raw[4];

//========================================================================
// User Digital Inputs
//========================================================================
uint8_t nUserDigInputRaw;
uint8_t nUserDigInput[PDM_NUM_INPUTS];

//========================================================================
// Output Logic
//========================================================================
uint8_t nOutputLogic[PDM_NUM_OUTPUTS];

//========================================================================
// PCB Temperature
//========================================================================
int16_t nBoardTempC;

//========================================================================
// STM ADC
// STM Interal Temperature
// Battery Voltage Sense
//========================================================================
volatile uint16_t nAdc1Data[ADC_1_COUNT];
volatile uint16_t nAdc4Data[ADC_4_COUNT];
volatile uint16_t nBattSense;
volatile uint16_t nStmTemp;
const uint16_t* const STM32_TEMP_3V3_30C =  (uint16_t*)(0x1FFFF7B8);
const uint16_t* const STM32_TEMP_3V3_110C =  (uint16_t*)(0x1FFFF7C2);

//========================================================================
// Status LEDs
//========================================================================
PCA9635_LEDOnState_t eStatusLeds[PDM_NUM_LEDS];
uint8_t nLEDTestSeqIndex;
uint32_t nLEDTestSeqValues;
uint32_t nLEDTestSeqLastTime;

//========================================================================
// CAN
//========================================================================
CAN_TxHeaderTypeDef stCanTxHeader;
CAN_RxHeaderTypeDef stCanRxHeader;
uint8_t nCanTxData[8];
uint8_t nCanRxData[8];
uint32_t nCanTxMailbox;
uint32_t nLastCanUpdate;

//========================================================================
// CANBoard
//========================================================================
static volatile CANBoard_RX_t stCANBoard_RX;
static volatile CANBoard_TX_t stCANBoard_TX;

//========================================================================
// Wipers
//========================================================================
static Wiper_t stWiper;

//========================================================================
// Variable and Input Mapping
//========================================================================
uint16_t* pVariableMap[PDM_VAR_MAP_SIZE];
uint16_t nPdmInputs[PDM_NUM_INPUTS];
uint16_t nCanInputs[PDM_NUM_CAN_INPUTS];
uint16_t nVirtInputs[PDM_NUM_VIRT_INPUTS];
uint16_t nOutputs[PDM_NUM_OUTPUTS];
uint16_t nStarterDisable[PDM_NUM_OUTPUTS];
uint16_t nOutputFlasher[PDM_NUM_OUTPUTS];

uint32_t nMsgCnt;


void InputLogic();
void OutputLogic();
void SetPfStatusLed(PCA9635_LEDOnState_t *ledState, volatile ProfetTypeDef *profet);

//========================================================================
// CAN Receive Callback
//========================================================================
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

  if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &stCanRxHeader, nCanRxData) != HAL_OK)
  {
    Error_Handler();
  }

  //Store latest receive time
  //Use to determine connection status
  nLastCanUpdate = HAL_GetTick();

  MsgQueueRx_t stMsg;
  stMsg.eMsgSrc = CAN_RX;
  stMsg.nRxLen = (uint8_t)stCanRxHeader.DLC;
  memcpy(&stMsg.stCanRxHeader, &stCanRxHeader, sizeof(stCanRxHeader));
  memcpy(&stMsg.nRxData, &nCanRxData, sizeof(nCanRxData));
  osMessageQueuePut(qMsgQueueRx, &stMsg, 0U, 0U);

}

//========================================================================
//========================================================================
// MAIN
//========================================================================
//========================================================================
void PdmMainTask(osThreadId_t* thisThreadId, ADC_HandleTypeDef* hadc1, ADC_HandleTypeDef* hadc4){

  HAL_ADC_Start_DMA(hadc1, (uint32_t*) nAdc1Data, ADC_1_COUNT);
  HAL_ADC_Start_DMA(hadc4, (uint32_t*) nAdc4Data, ADC_4_COUNT);

  Profet_Default_Init(pf, &pfGpioBank1, &pfGpioBank2);

  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_WritePin(EXTRA3_GPIO_Port, EXTRA3_Pin, GPIO_PIN_SET);

    //=====================================================================================================
    // ADC channels
    // ADC1 = Vbat and device temperature
    // ADC4 = Battery sense
    //=====================================================================================================
    nBattSense = (uint16_t)(((float)nAdc4Data[0]) * 0.0519 - 11.3);
    nStmTemp = (uint16_t)(80.0 / ((float)(*STM32_TEMP_3V3_110C) - (float)(*STM32_TEMP_3V3_30C)) *
                          (((float)nAdc1Data[0]) - (float)(*STM32_TEMP_3V3_30C)) + 30.0);

    //=====================================================================================================
    // CANBoard check connection
    //=====================================================================================================
    CANBoardCheckConnection(&stCANBoard_RX);

    //=====================================================================================================
    // Totalize current
    //=====================================================================================================
    nILTotal = 0;
    for(int i=0;i<PDM_NUM_OUTPUTS;i++)
      nILTotal += pf[i].nIL;

    //=====================================================================================================
    // Profet State Machine
    //=====================================================================================================
    for(int i=0; i<PDM_NUM_OUTPUTS; i++){
      Profet_SM(&pf[i]);
    }

    //=====================================================================================================
    // Wiper logic state machine
    //=====================================================================================================
    WiperSM(&stWiper);

    //=====================================================================================================
    // Check for CAN RX messages in queue
    //=====================================================================================================
    MsgQueueRx_t stMsgRx;
    osStatus_t eStatus;
    nMsgCnt = osMessageQueueGetCount(qMsgQueueRx);
    eStatus = osMessageQueueGet(qMsgQueueRx, &stMsgRx, NULL, 0U);
    if(eStatus == osOK){
      if(stMsgRx.eMsgSrc == CAN_RX){
        for(int i=0; i<PDM_NUM_CAN_INPUTS; i++){
          EvaluateCANInput(&stMsgRx.stCanRxHeader, stMsgRx.nRxData, &stPdmConfig.stCanInput[i], &nCanInputs[i]);
        }
      }
    }


#ifdef MEAS_HEAP_USE
    __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif



    //Debug GPIO
    //EXTRA3_GPIO_Port->ODR ^= EXTRA3_Pin;
    HAL_GPIO_WritePin(EXTRA3_GPIO_Port, EXTRA3_Pin, GPIO_PIN_RESET);

    osDelay(MAIN_TASK_DELAY);
  }
}

void I2CTask(osThreadId_t* thisThreadId, I2C_HandleTypeDef* hi2c1, I2C_HandleTypeDef* hi2c2){
  //=====================================================================================================
  // MCP9808 Temperature Sensor Configuration
  //=====================================================================================================
  if(MCP9808_Init(hi2c1, MCP9808_ADDRESS) != MCP9808_OK)
    printf("MCP9808 Init FAIL\n");

  MCP9808_SetResolution(hi2c1, MCP9808_ADDRESS, MCP9808_RESOLUTION_0_5DEG);

  if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_UPPER_TEMP, BOARD_TEMP_MAX) != MCP9808_OK)
    printf("MCP9808 Set Upper Limit Failed\n");
  if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_LOWER_TEMP, BOARD_TEMP_MIN) != MCP9808_OK)
    printf("MCP9808 Set Lower Limit Failed\n");
  if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_CRIT_TEMP, BOARD_TEMP_CRIT) != MCP9808_OK)
    printf("MCP9808 Set Critical Limit Failed\n");

  //Setup configuration
  //Enable alert pin
  //Lock Tupper/Tlower window settings
  //Lock Tcrit settings
  //Set Tupper/Tlower hysteresis to +1.5 deg C
  MCP9808_Write16(hi2c1, MCP9808_ADDRESS, MCP9808_REG_CONFIG, (MCP9808_REG_CONFIG_ALERTCTRL | MCP9808_REG_CONFIG_WINLOCKED | MCP9808_REG_CONFIG_CRITLOCKED | MCP9808_REG_CONFIG_HYST_1_5));

  //=====================================================================================================
  // PCAL9554B User Input Configuration
  //=====================================================================================================
  //Set configuration registers (all to input = 1)
  PCAL9554B_WriteReg8(hi2c1, PCAL9554B_ADDRESS, PCAL9554B_CMD_CFG, 0xFF);
  //Set latch register (no latch = 0)
  PCAL9554B_WriteReg8(hi2c1, PCAL9554B_ADDRESS, PCAL9554B_CMD_IN_LATCH, 0x00);
  //Set pullup/pulldown enable register (all enable = 1)
  PCAL9554B_WriteReg8(hi2c1, PCAL9554B_ADDRESS, PCAL9554B_CMD_PU_PD_ENABLE, 0xFF);
  //Set pullup/pulldown selection register (all to pullup = 1)
  PCAL9554B_WriteReg8(hi2c1, PCAL9554B_ADDRESS, PCAL9554B_CMD_PU_PD_SELECT, 0xFF);
  //Set interrupt mask (all to disable interrupt = 1)
  PCAL9554B_WriteReg8(hi2c1, PCAL9554B_ADDRESS, PCAL9554B_CMD_INT_MASK, 0xFF);


  //=====================================================================================================
  // PCA9539 Profet GPIO Configuration
  //=====================================================================================================
  HAL_GPIO_WritePin(PF_RESET_GPIO_Port, PF_RESET_Pin, GPIO_PIN_SET);
  //Set all outputs to push-pull
  PCA9539_WriteReg8(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_OUT_PORT_CONFIG, 0x00);
  //Set configuration registers (all to output)
  PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_CONFIG_PORT0, 0x0000);
  //Enable all pullup/pulldown
  PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_PU_PD_ENABLE_PORT0, 0xFFFF);
  //Set all outputs to pulldown
  PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_PU_PD_SELECT_PORT0, 0x0000);
  //Set to highest output drive strength
  PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_OUT_DRIVE_STRENGTH_PORT0, 0xFFFF);

  //=====================================================================================================
  // MAX11613 Analog In Configuration
  //=====================================================================================================
  if(MAX1161x_SendSetup(hi2c1, MAX11613_ADDRESS_PF_BANK1) != HAL_OK)
  {
    Error_Handler();
  }

  //=====================================================================================================
  // PCA9539 Profet GPIO Configuration
  //=====================================================================================================
  //Set all outputs to push-pull
  PCA9539_WriteReg8(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_OUT_PORT_CONFIG, 0x00);
  //Set configuration registers (all to output)
  PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_CONFIG_PORT0, 0x0000);
  //Enable all pullup/pulldown
  PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_PU_PD_ENABLE_PORT0, 0xFFFF);
  //Set all outputs to pulldown
  PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_PU_PD_SELECT_PORT0, 0x0000);
  //Set to highest output drive strength
  PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_OUT_DRIVE_STRENGTH_PORT0, 0xFFFF);

  //=====================================================================================================
  // MAX11613 Analog In Configuration
  //=====================================================================================================
  if(MAX1161x_SendSetup(hi2c2, MAX11613_ADDRESS_PF_BANK2) != HAL_OK)
  {
    Error_Handler();
  }

  //=====================================================================================================
  // PCA9635 LED Configuration
  //=====================================================================================================
  //Send configuration, set to blink/flasher
  PCA9635_Init(hi2c2, PCA9635_ADDRESS, PCA9635_BLINK);

  //Set flashing frequency
  PCA9635_SetGroupFreq(hi2c2, PCA9635_ADDRESS, PCA9635_FLASH_FREQ);

  //Set PWM duty cycle for each channel (overriden by group PWM)
  for(int i=0; i<PDM_NUM_LEDS; i++){
    PCA9635_SetPWM(hi2c2, PCA9635_ADDRESS, i, 255);
  }

  //Set flashing duty cycle
  PCA9635_SetGroupPWM(hi2c2, PCA9635_ADDRESS, PCA9635_FLASH_DUTY_CYCLE); //Have to set individual brightness levels first

  //Start LED test sequence
  nLEDTestSeqIndex = 1;
  nLEDTestSeqLastTime = HAL_GetTick();

  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_WritePin(EXTRA1_GPIO_Port, EXTRA1_Pin, GPIO_PIN_SET);
   //=====================================================================================================
   // PCAL9554B User Input
   //=====================================================================================================
   nUserDigInputRaw = PCAL9554B_ReadReg8(hi2c1, PCAL9554B_ADDRESS, PCAL9554B_CMD_IN_PORT);
   nUserDigInput[0] = !GET_BIT_AT(nUserDigInputRaw, 3);
   nUserDigInput[1] = !GET_BIT_AT(nUserDigInputRaw, 2);
   nUserDigInput[2] = !GET_BIT_AT(nUserDigInputRaw, 1);
   nUserDigInput[3] = !GET_BIT_AT(nUserDigInputRaw, 0);
   nUserDigInput[4] = !GET_BIT_AT(nUserDigInputRaw, 4);
   nUserDigInput[5] = !GET_BIT_AT(nUserDigInputRaw, 5);
   nUserDigInput[6] = !GET_BIT_AT(nUserDigInputRaw, 6);
   nUserDigInput[7] = !GET_BIT_AT(nUserDigInputRaw, 7);

   //=====================================================================================================
   // Set Profet
   // DSEL to channel 1
   // Enable all DEN
   //=====================================================================================================
   CLEAR_BIT(pfGpioBank1, PF_BANK1_DSEL);
   SET_BIT(pfGpioBank1, PF_BANK1_DEN);

   PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_OUT_PORT0, pfGpioBank1);

   //=====================================================================================================
   // MAX11613 Analog Input
   //=====================================================================================================
   for(int i = 0; i < 4; i++){
     //Read channel value
     if(MAX1161x_ReadADC(hi2c1, MAX11613_ADDRESS_PF_BANK1, i, &nPfISBank1Raw[i]) != HAL_OK)
     {
       Error_Handler();
     }
   }

   Profet_UpdateIS(&pf[0], nPfISBank1Raw[3]);
   Profet_UpdateIS(&pf[1], nPfISBank1Raw[2]);
   Profet_UpdateIS(&pf[2], nPfISBank1Raw[1]);
   Profet_UpdateIS(&pf[4], nPfISBank1Raw[0]);

   //=====================================================================================================
   //Flip Profet DSEL to channel 2
   //=====================================================================================================
   SET_BIT(pfGpioBank1, PF_BANK1_DSEL);

   PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_OUT_PORT0, pfGpioBank1);

   for(int i = 0; i < 2; i++){
     //Read channel value
     if(MAX1161x_ReadADC(hi2c1, MAX11613_ADDRESS_PF_BANK1, i, &nPfISBank1Raw[i]) != HAL_OK)
     {
       Error_Handler();
     }
   }

   //=====================================================================================================
   // Scale to IS Values
   //=====================================================================================================
   Profet_UpdateIS(&pf[3], nPfISBank1Raw[1]);
   Profet_UpdateIS(&pf[5], nPfISBank1Raw[0]);

   //=====================================================================================================
   // Profet I2C GPIO
   // PCA9555
   // PF1-6 Bank 1
   //=====================================================================================================
   InputLogic();
   OutputLogic();
   PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_OUT_PORT0, pfGpioBank1);

   //=====================================================================================================
   // MCP9808 temperature sensor
   //=====================================================================================================
   nBoardTempC = MCP9808_ReadTempC_Int(hi2c1, MCP9808_ADDRESS);

   if(MCP9808_GetOvertemp()) printf("*******MCP9808 Overtemp Detected*******\n");
   if(MCP9808_GetCriticalTemp()) printf("*******MCP9808 CRITICAL Overtemp Detected*******\n");

   //=====================================================================================================
   // Set Profet
   // DSEL to channel 1
   // Enable all DEN
   //=====================================================================================================
   CLEAR_BIT(pfGpioBank2, PF_BANK2_DSEL);
   SET_BIT(pfGpioBank2, PF_BANK2_DEN);

   //=====================================================================================================
   // Set CAN Terminating Resistor
   // Output 16 of Bank 2
   //=====================================================================================================
   if(stPdmConfig.stDevConfig.nCanTerm == 1)
   {
     SET_BIT(pfGpioBank2, CAN_TERM);
   }
   else
   {
     CLEAR_BIT(pfGpioBank2, CAN_TERM);
   }

   PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_OUT_PORT0, pfGpioBank2);

   //=====================================================================================================
   // MAX11613 Analog Input
   //=====================================================================================================
   for(int i = 0; i < 4; i++){
     //Read channel value
     if(MAX1161x_ReadADC(hi2c2, MAX11613_ADDRESS_PF_BANK2, i, &nPfISBank2Raw[i]) != HAL_OK)
     {
       Error_Handler();
     }
   }

   Profet_UpdateIS(&pf[6], nPfISBank2Raw[0]);
   Profet_UpdateIS(&pf[7], nPfISBank2Raw[1]);
   Profet_UpdateIS(&pf[9], nPfISBank2Raw[2]);
   Profet_UpdateIS(&pf[11], nPfISBank2Raw[3]);

   //=====================================================================================================
   //Flip Profet DSEL to channel 2
   //=====================================================================================================
   SET_BIT(pfGpioBank2, PF_BANK2_DSEL);

   PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_OUT_PORT0, pfGpioBank2);

   for(int i = 0; i < 4; i++){
     if(MAX1161x_ReadADC(hi2c2, MAX11613_ADDRESS_PF_BANK2, i, &nPfISBank2Raw[i]) != HAL_OK)
     {
       Error_Handler();
     }
   }

   //=====================================================================================================
   // Scale to IS Values
   //=====================================================================================================
   Profet_UpdateIS(&pf[8], nPfISBank2Raw[2]);
   Profet_UpdateIS(&pf[10], nPfISBank2Raw[3]);

   //=====================================================================================================
   // Profet I2C GPIO
   // PCA9555
   // PF1-6 Bank 1
   // PF7-12 Bank 2
   //=====================================================================================================
   InputLogic();
   OutputLogic();
   PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_OUT_PORT0, pfGpioBank2);

   //=====================================================================================================
   // Status LEDs
   //=====================================================================================================
   if(nLEDTestSeqIndex > 0)
   {
     nLEDTestSeqValues = (0x00000001 << ((nLEDTestSeqIndex-1)*2));

     PCA9635_SetAllNum(hi2c2, PCA9635_ADDRESS, nLEDTestSeqValues);

     if((HAL_GetTick() - nLEDTestSeqLastTime) > LED_TEST_SEQ_DELAY)
     {
       nLEDTestSeqLastTime = HAL_GetTick();
       nLEDTestSeqIndex++;
     }

     //Last step
     if(nLEDTestSeqIndex > 16)
       nLEDTestSeqIndex = 0;
   }
   else
   {
     for(int i=0; i<PDM_NUM_OUTPUTS; i++){
       SetPfStatusLed(&eStatusLeds[i], &pf[i]);
     }
     eStatusLeds[12] = (eDevMode == DEVICE_AUTO) + ((eDevMode == DEVICE_MANUAL) * LED_FLASH);              //State
     eStatusLeds[13] = 0;
     eStatusLeds[14] = (HAL_GetTick() - nLastCanUpdate) < 1000;              //CAN
     eStatusLeds[15] = (eDevState == DEVICE_ERROR);   //Fault
     PCA9635_SetAll(hi2c2, PCA9635_ADDRESS, eStatusLeds);
   }

   //Debug GPIO
   //HAL_GPIO_TogglePin(EXTRA1_GPIO_Port, EXTRA1_Pin);
   HAL_GPIO_WritePin(EXTRA1_GPIO_Port, EXTRA1_Pin, GPIO_PIN_RESET);
   //EXTRA1_GPIO_Port->ODR ^= EXTRA1_Pin;

#ifdef MEAS_HEAP_USE
   __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif

   osDelay(I2C_TASK_DELAY);
 }
}

void CanTxTask(osThreadId_t* thisThreadId, CAN_HandleTypeDef* hcan)
{
  //Configure the CAN Filter
  CAN_FilterTypeDef  sFilterConfig;
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(hcan, &sFilterConfig) != HAL_OK)
  {
    /* Filter configuration Error */
    Error_Handler();
  }

  //Start the CAN periphera
  if (HAL_CAN_Start(hcan) != HAL_OK)
  {
    /* Start Error */
    Error_Handler();
  }

  //Activate CAN RX notification
  if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
    /* Notification Error */
    Error_Handler();
  }

  //Configure Transmission
  stCanTxHeader.StdId = 1620;
  stCanTxHeader.ExtId = 0;
  stCanTxHeader.RTR = CAN_RTR_DATA;
  stCanTxHeader.IDE = CAN_ID_STD;
  stCanTxHeader.DLC = 8;
  stCanTxHeader.TransmitGlobalTime = DISABLE;

  for(;;){
    HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_SET);
    if(stPdmConfig.stCanOutput.nEnabled &&
        (stPdmConfig.stCanOutput.nUpdateTime > 0) &&
        stPdmConfig.stCanOutput.nBaseId > 0 &&
        stPdmConfig.stCanOutput.nBaseId < 2048){

      MsgQueueCanTx_t stMsgTx;
      osStatus_t stStatus;
      //Keep sending queued messages until empty
      do{
        stStatus = osMessageQueueGet(qMsgQueueTx, &stMsgTx, NULL, 0U);
        if(stStatus == osOK){
          stMsgTx.stTxHeader.ExtId = 0;
          stMsgTx.stTxHeader.IDE = CAN_ID_STD;
          stMsgTx.stTxHeader.RTR = CAN_RTR_DATA;
          stMsgTx.stTxHeader.TransmitGlobalTime = DISABLE;

          if(HAL_CAN_AddTxMessage(hcan, &stMsgTx.stTxHeader, stMsgTx.nTxData, &nCanTxMailbox) != HAL_OK){
            //Send failed - add back to queue
            osMessageQueuePut(qMsgQueueTx, &stMsgTx, 0U, 0U);
          }
        }
        //Pause for preemption - TX is not that important
        osDelay(CAN_TX_MSG_SPLIT);
      }while(stStatus == osOK);


      //=======================================================
      //Build Msg 0 (Digital inputs 1-8)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 0;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = nUserDigInput[0];
      nCanTxData[1] = nUserDigInput[1];
      nCanTxData[2] = nUserDigInput[2];
      nCanTxData[3] = nUserDigInput[3];
      nCanTxData[4] = nUserDigInput[4];
      nCanTxData[5] = nUserDigInput[5];
      nCanTxData[6] = nUserDigInput[6];
      nCanTxData[7] = nUserDigInput[7];

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 1 (Device status)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 1;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = eDevState;
      nCanTxData[1] = 0;
      nCanTxData[2] = nILTotal >> 8;
      nCanTxData[3] = nILTotal;
      nCanTxData[4] = nBattSense >> 8;
      nCanTxData[5] = nBattSense;
      nCanTxData[6] = nBoardTempC >> 8;
      nCanTxData[7] = nBoardTempC;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 2 (Out 1-4 Current)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 2;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = pf[0].nIL >> 8;
      nCanTxData[1] = pf[0].nIL;
      nCanTxData[2] = pf[1].nIL >> 8;
      nCanTxData[3] = pf[1].nIL;
      nCanTxData[4] = pf[2].nIL >> 8;
      nCanTxData[5] = pf[2].nIL;
      nCanTxData[6] = pf[3].nIL >> 8;
      nCanTxData[7] = pf[3].nIL;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 3 (Out 5-8 Current)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 3;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = pf[4].nIL >> 8;
      nCanTxData[1] = pf[4].nIL;
      nCanTxData[2] = pf[5].nIL >> 8;
      nCanTxData[3] = pf[5].nIL;
      nCanTxData[4] = pf[6].nIL >> 8;
      nCanTxData[5] = pf[6].nIL;
      nCanTxData[6] = pf[7].nIL >> 8;
      nCanTxData[7] = pf[7].nIL;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 4 (Out 9-12 Current)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 4;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = pf[8].nIL >> 8;
      nCanTxData[1] = pf[8].nIL;
      nCanTxData[2] = pf[9].nIL >> 8;
      nCanTxData[3] = pf[9].nIL;
      nCanTxData[4] = pf[10].nIL >> 8;
      nCanTxData[5] = pf[10].nIL;
      nCanTxData[6] = pf[11].nIL >> 8;
      nCanTxData[7] = pf[11].nIL;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 5 (Out 1-12 Status)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 5;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = (pf[1].eState << 4) + pf[0].eState;
      nCanTxData[1] = (pf[3].eState << 4) + pf[2].eState;
      nCanTxData[2] = (pf[5].eState << 4) + pf[4].eState;
      nCanTxData[3] = (pf[7].eState << 4) + pf[6].eState;
      nCanTxData[4] = (pf[9].eState << 4) + pf[8].eState;
      nCanTxData[5] = (pf[11].eState << 4) + pf[10].eState;
      nCanTxData[6] = 0;
      nCanTxData[7] = 0;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 6 (Out 1-4 Current Limit)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 6;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = pf[0].nIL_Limit >> 8;
      nCanTxData[1] = pf[0].nIL_Limit;
      nCanTxData[2] = pf[1].nIL_Limit >> 8;
      nCanTxData[3] = pf[1].nIL_Limit;
      nCanTxData[4] = pf[2].nIL_Limit >> 8;
      nCanTxData[5] = pf[2].nIL_Limit;
      nCanTxData[6] = pf[3].nIL_Limit >> 8;
      nCanTxData[7] = pf[3].nIL_Limit;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 7 (Out 5-8 Current Limit)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 7;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = pf[4].nIL_Limit >> 8;
      nCanTxData[1] = pf[4].nIL_Limit;
      nCanTxData[2] = pf[5].nIL_Limit >> 8;
      nCanTxData[3] = pf[5].nIL_Limit;
      nCanTxData[4] = pf[6].nIL_Limit >> 8;
      nCanTxData[5] = pf[6].nIL_Limit;
      nCanTxData[6] = pf[7].nIL_Limit >> 8;
      nCanTxData[7] = pf[7].nIL_Limit;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 8 (Out 9-12 Current Limit)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 8;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = pf[8].nIL_Limit >> 8;
      nCanTxData[1] = pf[8].nIL_Limit;
      nCanTxData[2] = pf[9].nIL_Limit >> 8;
      nCanTxData[3] = pf[9].nIL_Limit;
      nCanTxData[4] = pf[10].nIL_Limit >> 8;
      nCanTxData[5] = pf[10].nIL_Limit;
      nCanTxData[6] = pf[11].nIL_Limit >> 8;
      nCanTxData[7] = pf[11].nIL_Limit;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 9 (Out 1-8 Reset Count)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 9;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = pf[0].nOC_ResetCount;
      nCanTxData[1] = pf[1].nOC_ResetCount;
      nCanTxData[2] = pf[2].nOC_ResetCount;
      nCanTxData[3] = pf[3].nOC_ResetCount;
      nCanTxData[4] = pf[4].nOC_ResetCount;
      nCanTxData[5] = pf[5].nOC_ResetCount;
      nCanTxData[6] = pf[6].nOC_ResetCount;
      nCanTxData[7] = pf[7].nOC_ResetCount;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 10 (Out 9-12 Reset Count)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 10;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = pf[8].nOC_ResetCount;
      nCanTxData[1] = pf[9].nOC_ResetCount;
      nCanTxData[2] = pf[10].nOC_ResetCount;
      nCanTxData[3] = pf[11].nOC_ResetCount;
      nCanTxData[4] = 0;
      nCanTxData[5] = 0;
      nCanTxData[6] = 0;
      nCanTxData[7] = 0;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

    }

#ifdef MEAS_HEAP_USE
      __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif

    //Debug GPIO
    //HAL_GPIO_TogglePin(EXTRA2_GPIO_Port, EXTRA2_Pin);
    HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_RESET);
    osDelay(stPdmConfig.stCanOutput.nUpdateTime);
  }
}

void InputLogic(){
  for(int i=0; i<PDM_NUM_INPUTS; i++)
    EvaluateInput(&stPdmConfig.stInput[i], &nPdmInputs[i]);

  for(int i=0; i<PDM_NUM_VIRT_INPUTS; i++)
    EvaluateVirtInput(&stPdmConfig.stVirtualInput[i], &nVirtInputs[i]);

  //Map profet state to integer for use as virtual input pointer
  for(int i=0; i<PDM_NUM_OUTPUTS; i++){
    nOutputs[i] = pf[i].eState == ON;
    EvaluateStarter(&stPdmConfig.stStarter, i, &nStarterDisable[i]);
  }

  for(int i=0; i<PDM_NUM_OUTPUTS; i++){
      if( (stPdmConfig.stFlasher[0].nOutput != i) &&
          (stPdmConfig.stFlasher[1].nOutput != i) &&
          (stPdmConfig.stFlasher[2].nOutput != i) &&
          (stPdmConfig.stFlasher[3].nOutput != i))
        nOutputFlasher[i] = 1;
  }
  for(int i=0; i<PDM_NUM_FLASHERS; i++){
    EvaluateFlasher(&stPdmConfig.stFlasher[i], nOutputFlasher);
  }
}

void OutputLogic(){
  //Copy output logic to profet requested state
  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    pf[i].eReqState = (ProfetStateTypeDef)(*stPdmConfig.stOutput[i].pInput && nStarterDisable[i] && nOutputFlasher[i]);
  }
}

void SetPfStatusLed(PCA9635_LEDOnState_t *ledState, volatile ProfetTypeDef *profet)
{
  //0 = Off
  //1 = On
  //3 = Flash
  *ledState = (profet->eState == ON) +          //On
              (profet->eState == IN_RUSH) +     //On
              (profet->eState == OVERCURRENT)   * LED_FLASH +
              (profet->eState == SHORT_CIRCUIT) * LED_FLASH +
              (profet->eState == SUSPENDED)     * LED_FLASH +
              (profet->eState == FAULT)         * LED_FLASH;
}

//Overwrite printf _write to send to ITM_SendChar
int _write(int file, char *ptr, int len){
  int i=0;
  for(i=0; i<len; i++){
    ITM_SendChar((*ptr++));
  }
  return len;
}

//******************************************************
//Must be called before any tasks are started
//******************************************************
uint8_t InitPdmConfig(I2C_HandleTypeDef* hi2c2)
{
  PdmConfig_SetDefault(&stPdmConfig);

  //Map config to profet values
  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    pf[i].nIL_Limit = stPdmConfig.stOutput[i].nCurrentLimit;
    pf[i].nIL_InRush_Limit = stPdmConfig.stOutput[i].nInrushLimit;
    pf[i].nIL_InRush_Time = stPdmConfig.stOutput[i].nInrushTime;
    //pf[i]. = stPdmConfig.stOutput[i].eResetMode;
    //pf[i] = stPdmConfig.stOutput[i].nResetTime;
    pf[i].nOC_ResetLimit = stPdmConfig.stOutput[i].nResetLimit;

  }

  //Map the variable map first before using
  //User inputs
  for(int i=0; i<PDM_NUM_INPUTS; i++)
    pVariableMap[i+1] = &nPdmInputs[i];

  //CAN inputs
  for(int i=0; i<PDM_NUM_CAN_INPUTS; i++)
    pVariableMap[i + 9] = &nCanInputs[i];

  for(int i=0; i<PDM_NUM_VIRT_INPUTS; i++)
    pVariableMap[i + 39] = &nVirtInputs[i];

  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    pVariableMap[i + 59] = &nOutputs[i];
  }

  pVariableMap[71] = &stWiper.nSlowOut;
  pVariableMap[72] = &stWiper.nFastOut;

  //Assign variable map values
  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    stPdmConfig.stOutput[i].pInput = pVariableMap[stPdmConfig.stOutput[i].nInput];
  }

  //Map input values to config structure
  for(int i=0; i<PDM_NUM_INPUTS; i++)
  {
    stPdmConfig.stInput[i].pInput = &nUserDigInput[i];
  }

  for(int i=0; i<PDM_NUM_VIRT_INPUTS; i++)
  {
    stPdmConfig.stVirtualInput[i].pVar0 = pVariableMap[stPdmConfig.stVirtualInput[i].nVar0];
    stPdmConfig.stVirtualInput[i].pVar1 = pVariableMap[stPdmConfig.stVirtualInput[i].nVar1];
    stPdmConfig.stVirtualInput[i].pVar2 = pVariableMap[stPdmConfig.stVirtualInput[i].nVar2];
  }

  stWiper.nEnabled = stPdmConfig.stWiper.nEnabled;
  stWiper.eMode = stPdmConfig.stWiper.nMode;
  stWiper.pSlowInput = pVariableMap[stPdmConfig.stWiper.nSlowInput];
  stWiper.pFastInput = pVariableMap[stPdmConfig.stWiper.nFastInput];
  stWiper.pInterInput = pVariableMap[stPdmConfig.stWiper.nInterInput];
  stWiper.pSwipeInput = pVariableMap[stPdmConfig.stWiper.nSwipeInput];
  stWiper.pOnSw = pVariableMap[stPdmConfig.stWiper.nOnInput];
  stWiper.pParkSw = pVariableMap[stPdmConfig.stWiper.nParkInput];
  stWiper.pSpeedInput = pVariableMap[stPdmConfig.stWiper.nSpeedInput];
  stWiper.pWashInput = pVariableMap[stPdmConfig.stWiper.nWashInput];
  stWiper.nWashWipeCycles = stPdmConfig.stWiper.nWashWipeCycles;
  for(int i=0; i<PDM_NUM_WIPER_INTER_DELAYS; i++)
    stWiper.nInterDelays[i] = stPdmConfig.stWiper.nIntermitTime[i];
  for(int i=0; i<PDM_NUM_WIPER_SPEED_MAP; i++)
    stWiper.eSpeedMap[i] = (WiperSpeed_t)stPdmConfig.stWiper.nSpeedMap[i];

  stPdmConfig.stStarter.pInput = pVariableMap[stPdmConfig.stStarter.nInput];

  for(int i=0; i<PDM_NUM_FLASHERS; i++)
    stPdmConfig.stFlasher[i].pInput = pVariableMap[stPdmConfig.stFlasher[i].nInput];

  return PDM_OK;
}
