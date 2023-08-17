/*
 * dingo_pdm.c
 *
 *  Created on: Oct 22, 2020
 *      Author: coryg
 */

#include "dingo_pdm.h"
#include "pdm_config.h"
#include "main.h"

//========================================================================
// Task Delays
//========================================================================
#define MAIN_TASK_DELAY 2
#define CAN_TX_DELAY 50

//========================================================================
// I2C Addresses
//========================================================================
#define MCP9808_ADDRESS 0x18
#define MB85RC_ADDRESS 0x50

//========================================================================
// Board Temperature Limits
//========================================================================
#define BOARD_TEMP_MAX 50
#define BOARD_TEMP_MIN 0
#define BOARD_TEMP_CRIT 80

//========================================================================
// STM ADC Counts
//========================================================================
#define ADC_1_COUNT 5
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
// Profets
//========================================================================
volatile ProfetTypeDef pf[PDM_NUM_OUTPUTS];
volatile uint16_t nILTotal;

//========================================================================
// User Digital Inputs
//========================================================================
bool bUserDigInput[PDM_NUM_INPUTS];

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
// VREFINT
//========================================================================
volatile uint16_t nAdc1Data[ADC_1_COUNT];
volatile uint16_t nAdc4Data[ADC_4_COUNT];
volatile uint16_t nBattSense;
volatile uint16_t nStmTemp;
const uint16_t* const STM32_TEMP_3V3_30C =  (uint16_t*)(0x1FFFF7B8);
const uint16_t* const STM32_TEMP_3V3_110C =  (uint16_t*)(0x1FFFF7C2);
volatile float fVDDA;
volatile uint16_t nVREFINT;
const uint16_t* const STM32_VREF_INT_CAL = (uint16_t*)(0x1FFFF7BA);
volatile uint16_t nVREFINTCAL;

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
void Profet_Default_Init();

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
void PdmMainTask(osThreadId_t* thisThreadId, ADC_HandleTypeDef* hadc1, ADC_HandleTypeDef* hadc4, I2C_HandleTypeDef* hi2c1){

  //=====================================================================================================
  // MCP9808 Temperature Sensor Configuration
  //=====================================================================================================
  MCP9808_Init(hi2c1, MCP9808_ADDRESS);
  //if(MCP9808_Init(hi2c1, MCP9808_ADDRESS) != MCP9808_OK)
    //printf("MCP9808 Init FAIL\n");

  MCP9808_SetResolution(hi2c1, MCP9808_ADDRESS, MCP9808_RESOLUTION_0_5DEG);

  MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_UPPER_TEMP, BOARD_TEMP_MAX);
  //if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_UPPER_TEMP, BOARD_TEMP_MAX) != MCP9808_OK)
    //printf("MCP9808 Set Upper Limit Failed\n");
  MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_LOWER_TEMP, BOARD_TEMP_MIN);
  //if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_LOWER_TEMP, BOARD_TEMP_MIN) != MCP9808_OK)
    //printf("MCP9808 Set Lower Limit Failed\n");
  MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_CRIT_TEMP, BOARD_TEMP_CRIT);
  //if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_CRIT_TEMP, BOARD_TEMP_CRIT) != MCP9808_OK)
    //printf("MCP9808 Set Critical Limit Failed\n");

  //Setup configuration
  //Enable alert pin
  //Lock Tupper/Tlower window settings
  //Lock Tcrit settings
  //Set Tupper/Tlower hysteresis to +1.5 deg C
  MCP9808_Write16(hi2c1, MCP9808_ADDRESS, MCP9808_REG_CONFIG, (MCP9808_REG_CONFIG_ALERTCTRL | MCP9808_REG_CONFIG_WINLOCKED | MCP9808_REG_CONFIG_CRITLOCKED | MCP9808_REG_CONFIG_HYST_1_5));

  //=====================================================================================================
  // Start ADC DMA
  //=====================================================================================================
  HAL_ADC_Start_DMA(hadc1, (uint32_t*) nAdc1Data, ADC_1_COUNT);
  HAL_ADC_Start_DMA(hadc4, (uint32_t*) nAdc4Data, ADC_4_COUNT);

  //=====================================================================================================
  // Init Profet Settings
  //=====================================================================================================
  Profet_Default_Init();

  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_WritePin(EXTRA1_GPIO_Port, EXTRA1_Pin, GPIO_PIN_SET);


    //=====================================================================================================
    // ADC channels
    // ADC1 = VREFint and device temperature
    // ADC4 = Battery sense
    //=====================================================================================================
    nBattSense = (uint16_t)(((float)nAdc4Data[0]) * 0.0519 - 11.3);
    nStmTemp = (uint16_t)(80.0 / ((float)(*STM32_TEMP_3V3_110C) - (float)(*STM32_TEMP_3V3_30C)) *
                          (((float)nAdc1Data[3]) - (float)(*STM32_TEMP_3V3_30C)) + 30.0);
    nVREFINT = nAdc1Data[4];
    nVREFINTCAL = *STM32_VREF_INT_CAL;
    fVDDA = (float)((3.3 * (float)(*STM32_VREF_INT_CAL)) / nVREFINT);

    //=====================================================================================================
    // Set Profet
    // DSEL to channel 1
    // Enable all DEN
    //=====================================================================================================
    HAL_GPIO_WritePin(PF_DEN1_GPIO_Port, PF_DEN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PF_DEN2_GPIO_Port, PF_DEN2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PF_DEN3_4_GPIO_Port, PF_DEN3_4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PF_DSEL3_4_GPIO_Port, PF_DSEL3_4_Pin, GPIO_PIN_SET);
    //Wait for DSEL changeover (up to 60us)
    osDelay(1);

    //=====================================================================================================
    // Update output current
    //=====================================================================================================
    Profet_UpdateIS(&pf[0], nAdc1Data[2], fVDDA);
    Profet_UpdateIS(&pf[1], nAdc1Data[0], fVDDA);
    Profet_UpdateIS(&pf[3], nAdc1Data[1], fVDDA);

    //=====================================================================================================
    //Flip Profet DSEL to channel 2
    //=====================================================================================================
    HAL_GPIO_WritePin(PF_DSEL3_4_GPIO_Port, PF_DSEL3_4_Pin, GPIO_PIN_RESET);
    //Wait for DSEL changeover (up to 60us)
    osDelay(1);

    //=====================================================================================================
    // Update output current
    //=====================================================================================================
    Profet_UpdateIS(&pf[2], nAdc1Data[1], fVDDA);

    //=====================================================================================================
    // Update status inputs
    //=====================================================================================================
    pf[4].bST = HAL_GPIO_ReadPin(PF_ST5_6_GPIO_Port, PF_ST5_6_Pin) == GPIO_PIN_SET;
    pf[5].bST = HAL_GPIO_ReadPin(PF_ST5_6_GPIO_Port, PF_ST5_6_Pin) == GPIO_PIN_SET;
    pf[6].bST = HAL_GPIO_ReadPin(PF_ST7_8_GPIO_Port, PF_ST7_8_Pin) == GPIO_PIN_SET;
    pf[7].bST = HAL_GPIO_ReadPin(PF_ST7_8_GPIO_Port, PF_ST7_8_Pin) == GPIO_PIN_SET;

    //=====================================================================================================
    // Update digital inputs
    //=====================================================================================================
    bUserDigInput[0] = HAL_GPIO_ReadPin(DIG_IN1_GPIO_Port, DIG_IN1_Pin) == GPIO_PIN_SET;
    bUserDigInput[1] = HAL_GPIO_ReadPin(DIG_IN2_GPIO_Port, DIG_IN2_Pin) == GPIO_PIN_SET;

    //=====================================================================================================
    // Compound Input/Output Logic
    //=====================================================================================================
    InputLogic();
    OutputLogic();

    //=====================================================================================================
    // Profet State Machine
    //=====================================================================================================
    for(int i=0; i<PDM_NUM_OUTPUTS; i++){
      Profet_SM(&pf[i]);
    }

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
    // Wiper logic state machine
    //=====================================================================================================
    WiperSM(&stWiper);

    //=====================================================================================================
    // MCP9808 temperature sensor
    //=====================================================================================================
    nBoardTempC = MCP9808_ReadTempC_Int(hi2c1, MCP9808_ADDRESS);

    if(MCP9808_GetOvertemp());// printf("*******MCP9808 Overtemp Detected*******\n");
    if(MCP9808_GetCriticalTemp());// printf("*******MCP9808 CRITICAL Overtemp Detected*******\n");

    //=====================================================================================================
    // Check for CAN RX messages in queue
    //=====================================================================================================
    MsgQueueRx_t stMsgRx;
    osStatus_t eStatus;
    nMsgCnt = osMessageQueueGetCount(qMsgQueueRx);
    eStatus = osMessageQueueGet(qMsgQueueRx, &stMsgRx, NULL, 0U);
    if(eStatus == osOK){
      for(int i=0; i<PDM_NUM_CAN_INPUTS; i++){
        EvaluateCANInput(&stMsgRx.stCanRxHeader, stMsgRx.nRxData, &stPdmConfig.stCanInput[i], &nCanInputs[i]);
      }
      //Check for settings change messages
    }

#ifdef MEAS_HEAP_USE
    __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif

    //Debug GPIO
    //EXTRA3_GPIO_Port->ODR ^= EXTRA3_Pin;
    HAL_GPIO_WritePin(EXTRA1_GPIO_Port, EXTRA1_Pin, GPIO_PIN_RESET);

    osDelay(MAIN_TASK_DELAY);
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

      MsgQueueTx_t stMsgTx;
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
      nCanTxData[0] = bUserDigInput[0];
      nCanTxData[1] = bUserDigInput[1];
      nCanTxData[2] = 0;
      nCanTxData[3] = 0;
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

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 1 (Device status)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 1;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = 0;
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
      //Build Msg 3 (Unused)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 3;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = 0;
      nCanTxData[1] = 0;
      nCanTxData[2] = 0;
      nCanTxData[3] = 0;
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

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 4 (Unused)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 4;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = 0;
      nCanTxData[1] = 0;
      nCanTxData[2] = 0;
      nCanTxData[3] = 0;
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

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 5 (Out 1-8 Status)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 5;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = (pf[1].eState << 4) + pf[0].eState;
      nCanTxData[1] = (pf[3].eState << 4) + pf[2].eState;
      nCanTxData[2] = (pf[5].eState << 4) + pf[4].eState;
      nCanTxData[3] = (pf[7].eState << 4) + pf[6].eState;
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
      //Build Msg 7 (Unused)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 7;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = 0;
      nCanTxData[1] = 0;
      nCanTxData[2] = 0;
      nCanTxData[3] = 0;
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

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 8 (Unused)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 8;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = 0;
      nCanTxData[1] = 0;
      nCanTxData[2] = 0;
      nCanTxData[3] = 0;
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

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 9 (Out 1-8 Reset Count)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 9;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = pf[0].nOC_Count;
      nCanTxData[1] = pf[1].nOC_Count;
      nCanTxData[2] = pf[2].nOC_Count;
      nCanTxData[3] = pf[3].nOC_Count;
      nCanTxData[4] = pf[4].nOC_Count;
      nCanTxData[5] = pf[5].nOC_Count;
      nCanTxData[6] = pf[6].nOC_Count;
      nCanTxData[7] = pf[7].nOC_Count;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      //osDelay(CAN_TX_MSG_SPLIT);
      for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 10 (Unused)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 10;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = 0;
      nCanTxData[1] = 0;
      nCanTxData[2] = 0;
      nCanTxData[3] = 0;
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
    //pf[i].eReqState = 1;
    pf[i].eReqState = (ProfetStateTypeDef)(*stPdmConfig.stOutput[i].pInput && nStarterDisable[i] && nOutputFlasher[i]);
  }
}


//Overwrite printf _write to send to ITM_SendChar
int _write(int file, char *ptr, int len){
  int i=0;
  for(i=0; i<len; i++){
    ITM_SendChar((*ptr++));
  }
  return len;
}

void Profet_Default_Init(){

  pf[0].eModel = BTS7002_1EPP;
  pf[0].nNum = 0;
  pf[0].nIN_Port = PF_IN1_GPIO_Port;
  pf[0].nIN_Pin = PF_IN1_Pin;
  pf[0].nDEN_Port = PF_DEN1_GPIO_Port;
  pf[0].nDEN_Pin = PF_DEN1_Pin;
  pf[0].fKILIS = 254795;//227000;

  pf[1].eModel = BTS7002_1EPP;
  pf[1].nNum = 1;
  pf[1].nIN_Port = PF_IN2_GPIO_Port;
  pf[1].nIN_Pin = PF_IN2_Pin;
  pf[1].nDEN_Port = PF_DEN2_GPIO_Port;
  pf[1].nDEN_Pin = PF_DEN2_Pin;
  pf[1].fKILIS = 254795;//227000;

  pf[2].eModel = BTS7008_2EPA_CH1;
  pf[2].nNum = 2;
  pf[2].nIN_Port = PF_IN3_GPIO_Port;
  pf[2].nIN_Pin = PF_IN3_Pin;
  pf[2].nDEN_Port = PF_DEN3_4_GPIO_Port;
  pf[2].nDEN_Pin = PF_DEN3_4_Pin;
  pf[2].fKILIS = 59258;//54000;

  pf[3].eModel = BTS7008_2EPA_CH2;
  pf[3].eState = OFF;
  pf[3].nNum = 3;
  pf[3].nIN_Port = PF_IN4_GPIO_Port;
  pf[3].nIN_Pin = PF_IN4_Pin;
  pf[3].nDEN_Port = PF_DEN3_4_GPIO_Port;
  pf[3].nDEN_Pin = PF_DEN3_4_Pin;
  pf[3].fKILIS = 59258;//54000;

  pf[4].eModel = BTS724_CH1;
  pf[4].eState = OFF;
  pf[4].nNum = 4;
  pf[4].nIN_Port = PF_IN5_GPIO_Port;
  pf[4].nIN_Pin = PF_IN5_Pin;
  pf[4].nDEN_Port = 0;
  pf[4].nDEN_Pin = 0;
  pf[4].fKILIS = 0;

  pf[5].eModel = BTS724_CH2;
  pf[5].eState = OFF;
  pf[5].nNum = 5;
  pf[5].nIN_Port = PF_IN6_GPIO_Port;
  pf[5].nIN_Pin = PF_IN6_Pin;
  pf[5].nDEN_Port = 0;
  pf[5].nDEN_Pin = 0;
  pf[5].fKILIS = 0;

  pf[6].eModel = BTS724_CH3;
  pf[6].eState = OFF;
  pf[6].nNum = 6;
  pf[6].nIN_Port = PF_IN7_GPIO_Port;
  pf[6].nIN_Pin = PF_IN7_Pin;
  pf[6].nDEN_Port = 0;
  pf[6].nDEN_Pin = 0;
  pf[6].fKILIS = 0;

  pf[7].eModel = BTS724_CH4;
  pf[7].eState = OFF;
  pf[7].nNum = 7;
  pf[7].nIN_Port = PF_IN8_GPIO_Port;
  pf[7].nIN_Pin = PF_IN8_Pin;
  pf[7].nDEN_Port = 0;
  pf[7].nDEN_Pin = 0;
  pf[7].fKILIS = 0;
}

//******************************************************
//Must be called before any tasks are started
//******************************************************
uint8_t InitPdmConfig(I2C_HandleTypeDef* hi2c1)
{
  PdmConfig_SetDefault(&stPdmConfig);

  //Map config to profet values
  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    pf[i].nIL_Limit = stPdmConfig.stOutput[i].nCurrentLimit;
    pf[i].nIL_InRushLimit = stPdmConfig.stOutput[i].nInrushLimit;
    pf[i].nIL_InRushTime = stPdmConfig.stOutput[i].nInrushTime;
    pf[i].nOC_ResetLimit = stPdmConfig.stOutput[i].nResetLimit;
    pf[i].nOC_ResetTime = stPdmConfig.stOutput[i].nResetTime;
    pf[i].eResetMode = stPdmConfig.stOutput[i].eResetMode;
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
    stPdmConfig.stInput[i].pInput = &bUserDigInput[i];
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
