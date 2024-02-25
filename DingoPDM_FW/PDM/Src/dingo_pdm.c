/*
 * dingo_pdm.c
 *
 *  Created on: Oct 22, 2020
 *      Author: corygrant
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
#define ADC_1_COUNT 8
#define ADC_1_BATT_SENSE 3
#define ADC_1_TEMP_SENSOR 6
#define ADC_1_VREF_INT 7
#define ADC_1_PF1 0 //ADC1_0
#define ADC_1_PF2 4 //ADC1_12
#define ADC_1_PF3_4 5 //ADC1_13
#define ADC_1_PF5_6 1 //ADC1_1
#define ADC_1_PF7_8 2 //ADC1_2

//========================================================================
// CAN
//========================================================================
#define CAN_TX_BASE_ID 2000
#define CAN_TX_MSG_SPLIT 2 //ms

//========================================================================
// STM Internal Calibration Voltages
//========================================================================
#define STM32_TEMP_CALIB_VOLT 3.3
#define STM32_TEMP_REF_VOLT 3.3

//========================================================================
// Device State
//========================================================================
DeviceState_t eDeviceState = DEVICE_POWER_ON;
bool bDeviceOverTemp = false;
bool bDeviceCriticalTemp = false;
uint16_t nDeviceError = 0;

//========================================================================
// PDM Config
//========================================================================
PdmConfig_t stPdmConfig;

//========================================================================
// Message Queues
//========================================================================
osMessageQueueId_t qMsgQueueRx;
osMessageQueueId_t qMsgQueueUsbTx;
osMessageQueueId_t qMsgQueueCanTx;

//========================================================================
// Profets
//========================================================================
volatile ProfetTypeDef pf[PDM_NUM_OUTPUTS];
volatile uint16_t nILTotal;

//========================================================================
// User Digital Inputs
//========================================================================
bool bRawUserInput[PDM_NUM_INPUTS];

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
volatile uint16_t nBattSense;
volatile uint16_t nStmTemp;
const uint16_t* const STM32_TEMP_3V3_30C =  (uint16_t*)(0x1FFF7A2C);
const uint16_t* const STM32_TEMP_3V3_110C =  (uint16_t*)(0x1FFF7A2E);
uint16_t temp30, temp110;

volatile float fVDDA;
volatile uint16_t nVREFINT;
const uint16_t* const STM32_VREF_INT_CAL = (uint16_t*)(0x1FFF7A2A);
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
// USB
//========================================================================
bool bUsbReady = false;

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
CANInput_Rx_t stCanInputsRx[PDM_NUM_CAN_INPUTS];
uint16_t nVirtInputs[PDM_NUM_VIRT_INPUTS];
uint16_t nOutputs[PDM_NUM_OUTPUTS];
uint16_t nStarterDisable[PDM_NUM_OUTPUTS];
uint16_t nOutputFlasher[PDM_NUM_OUTPUTS];
uint16_t nAlwaysTrue;

uint32_t nMsgCnt;

//========================================================================
// Status LEDs
//========================================================================
void StatusLedOn (void) {HAL_GPIO_WritePin(StatusLED_GPIO_Port, StatusLED_Pin, GPIO_PIN_SET);}
void StatusLedOff (void) {HAL_GPIO_WritePin(StatusLED_GPIO_Port, StatusLED_Pin, GPIO_PIN_RESET);}
Led_Output StatusLed = {StatusLedOn, StatusLedOff, false};

void ErrorLedOn (void) {HAL_GPIO_WritePin(ErrorLED_GPIO_Port, ErrorLED_Pin, GPIO_PIN_SET);}
void ErrorLedOff (void) {HAL_GPIO_WritePin(ErrorLED_GPIO_Port, ErrorLED_Pin, GPIO_PIN_RESET);}
Led_Output ErrorLed = {ErrorLedOn, ErrorLedOff, true};

//========================================================================
// Local Function Prototypes
//========================================================================
void InputLogic();
void OutputLogic();
void Profet_Default_Init();
void SendMsg17(CAN_HandleTypeDef * hcan);
void SendMsg16(CAN_HandleTypeDef *hcan);
void SendMsg15(CAN_HandleTypeDef *hcan);
void SendMsg14(CAN_HandleTypeDef *hcan);
void SendMsg13(CAN_HandleTypeDef *hcan);
void SendMsg12(CAN_HandleTypeDef *hcan);
void SendMsg11(CAN_HandleTypeDef *hcan);
void SendMsg10(CAN_HandleTypeDef *hcan);
void SendMsg9(CAN_HandleTypeDef *hcan);
void SendMsg8(CAN_HandleTypeDef *hcan);
void SendMsg7(CAN_HandleTypeDef *hcan);
void SendMsg6(CAN_HandleTypeDef *hcan);
void SendMsg5(CAN_HandleTypeDef *hcan);
void SendMsg4(CAN_HandleTypeDef *hcan);
void SendMsg3(CAN_HandleTypeDef *hcan);
void SendMsg2(CAN_HandleTypeDef *hcan);
void SendMsg1(CAN_HandleTypeDef *hcan);
void SendMsg0(CAN_HandleTypeDef *hcan);
void SendMsg(CAN_HandleTypeDef *hcan, bool bDelay);

/*
  * @brief  USB Receive Callback
  * @param  Buf: USB Rx Buffer
  * @param  Len: USB Rx Buffer Length
  * @retval None
*/
void USB_MsgRcv(uint8_t* Buf, uint32_t *Len)
{
  MsgQueueRx_t stMsg;
  stMsg.eMsgSrc = USB_RX;
  stMsg.nCRC = 0xFFFFFFFF;
  stMsg.nRxLen = 0;
  for(uint8_t i=0; i<*Len; i++){
    if(i < 8){
      stMsg.nRxData[i] = Buf[i];
      stMsg.nRxLen++;
    }
  }

  stMsg.stCanRxHeader.StdId = stPdmConfig.stCanOutput.nBaseId - 1;

  osMessageQueuePut(qMsgQueueRx, &stMsg, 0U, 0U);
}

/*
* @brief  CAN Receive Callback
* @param  hcan: CAN Handle
* @retval None
*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

  if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &stCanRxHeader, nCanRxData) != HAL_OK)
  {
    Error_Handler(PDM_ERROR_CAN);
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

/*
* @brief Main Task
* @param thisThreadId: Pointer to this thread ID
* @param hadc1: Pointer to ADC Handle
* @param hi2c1: Pointer to I2C Handle
* @retval None
*/
void PdmMainTask(osThreadId_t* thisThreadId, ADC_HandleTypeDef* hadc1, I2C_HandleTypeDef* hi2c1){
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_WritePin(EXTRA1_GPIO_Port, EXTRA1_Pin, GPIO_PIN_SET);

    switch (eDeviceState)
    {
    case DEVICE_POWER_ON:
      eDeviceState = DEVICE_STARTING;
      break;

    case DEVICE_STARTING:
      //=====================================================================================================
      // USB initialization
      //=====================================================================================================
      bUsbReady = USB_Init(USB_MsgRcv) == USBD_OK;
      if(!bUsbReady){
        Error_Handler(PDM_ERROR_USB);
      }

      //=====================================================================================================
      // MCP9808 Temperature Sensor Configuration
      //=====================================================================================================
      if(MCP9808_Init(hi2c1, MCP9808_ADDRESS) != MCP9808_OK)
        Error_Handler(PDM_ERROR_TEMP_SENSOR);

      MCP9808_SetResolution(hi2c1, MCP9808_ADDRESS, MCP9808_RESOLUTION_0_5DEG);

      if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_UPPER_TEMP, BOARD_TEMP_MAX) != MCP9808_OK)
        Error_Handler(PDM_ERROR_TEMP_SENSOR);
      if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_LOWER_TEMP, BOARD_TEMP_MIN) != MCP9808_OK)
        Error_Handler(PDM_ERROR_TEMP_SENSOR);
      if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_CRIT_TEMP, BOARD_TEMP_CRIT) != MCP9808_OK)
        Error_Handler(PDM_ERROR_TEMP_SENSOR);

      //Setup configuration
      //Enable alert pin
      //Lock Tupper/Tlower window settings
      //Lock Tcrit settings
      //Set Tupper/Tlower hysteresis to +1.5 deg C
      MCP9808_Write16(hi2c1, MCP9808_ADDRESS, MCP9808_REG_CONFIG, (MCP9808_REG_CONFIG_ALERTCTRL | MCP9808_REG_CONFIG_WINLOCKED | MCP9808_REG_CONFIG_CRITLOCKED | MCP9808_REG_CONFIG_HYST_1_5));

      //=====================================================================================================
      // Start ADC DMA
      //=====================================================================================================
      if(HAL_ADC_Start_DMA(hadc1, (uint32_t*) nAdc1Data, ADC_1_COUNT) != HAL_OK)
        Error_Handler(PDM_ERROR_ADC);

      //=====================================================================================================
      // Init Profet Settings
      //=====================================================================================================
      Profet_Default_Init();    
      
      //if successful
      eDeviceState = DEVICE_RUN;

      break;

    case DEVICE_RUN:
      LedSetSteady(&ErrorLed, false);

      if (bDeviceCriticalTemp)
      {
        eDeviceState = DEVICE_OVERTEMP;
      }
      break;

    case DEVICE_OVERTEMP:
      //Red LED solid
      //Outputs still on
      //Go back to run when temp falls

      LedSetSteady(&ErrorLed, true);

      if (!bDeviceCriticalTemp)
      {
        LedSetSteady(&ErrorLed, false);
        eDeviceState = DEVICE_RUN;
      }
      break;

    case DEVICE_ERROR:
      //Error LED - flash error code
      //No way to recover, must power cycle
      //Gets trapped in main Error_Handler()
      Error_Handler(nDeviceError);

      break;
    
    default:
      break;
    }

    //=====================================================================================================
    // Check for device errors
    //=====================================================================================================
    if(nDeviceError > 0){
      eDeviceState = DEVICE_ERROR;
    }

    //Need space for code that always runs
    //Some code will error if it hasn't been initalized in DEVICE_STARTING

    //=====================================================================================================
    // ADC channels
    // Battery sense
    // Device temperature
    // VREFint
    //=====================================================================================================
    nBattSense = (uint16_t)(((float)nAdc1Data[ADC_1_BATT_SENSE]) * 0.0519 - 11.3);
    nStmTemp = (uint16_t)( (80.0 / (float)(*STM32_TEMP_3V3_110C - *STM32_TEMP_3V3_30C)) *
                           (float)(nAdc1Data[ADC_1_TEMP_SENSOR] - *STM32_TEMP_3V3_30C) + 30.0);
    nVREFINT = nAdc1Data[ADC_1_VREF_INT];
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
    HAL_GPIO_WritePin(PF_DEN5_6_GPIO_Port, PF_DEN5_6_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PF_DEN7_8_GPIO_Port, PF_DEN7_8_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PF_DSEL3_4_GPIO_Port, PF_DSEL3_4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PF_DSEL5_6_GPIO_Port, PF_DSEL5_6_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PF_DSEL7_8_GPIO_Port, PF_DSEL7_8_Pin, GPIO_PIN_SET);
    //Wait for DSEL changeover (up to 60us)
    osDelay(1);

    //=====================================================================================================
    // Update output current
    //=====================================================================================================
    Profet_UpdateIS(&pf[0], nAdc1Data[ADC_1_PF1], fVDDA);
    Profet_UpdateIS(&pf[1], nAdc1Data[ADC_1_PF2], fVDDA);
    Profet_UpdateIS(&pf[3], nAdc1Data[ADC_1_PF3_4], fVDDA);
    Profet_UpdateIS(&pf[5], nAdc1Data[ADC_1_PF5_6], fVDDA);
    Profet_UpdateIS(&pf[7], nAdc1Data[ADC_1_PF7_8], fVDDA);

    //=====================================================================================================
    //Flip Profet DSEL to channel 2
    //=====================================================================================================
    HAL_GPIO_WritePin(PF_DSEL3_4_GPIO_Port, PF_DSEL3_4_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PF_DSEL5_6_GPIO_Port, PF_DSEL5_6_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PF_DSEL7_8_GPIO_Port, PF_DSEL7_8_Pin, GPIO_PIN_RESET);
    //Wait for DSEL changeover (up to 60us)
    osDelay(1);

    //=====================================================================================================
    // Update output current
    //=====================================================================================================
    Profet_UpdateIS(&pf[2], nAdc1Data[ADC_1_PF3_4], fVDDA);
    Profet_UpdateIS(&pf[4], nAdc1Data[ADC_1_PF5_6], fVDDA);
    Profet_UpdateIS(&pf[6], nAdc1Data[ADC_1_PF7_8], fVDDA);

    //=====================================================================================================
    // Update digital inputs
    //=====================================================================================================
    bRawUserInput[0] = HAL_GPIO_ReadPin(DIG_IN1_GPIO_Port, DIG_IN1_Pin) == GPIO_PIN_SET;
    bRawUserInput[1] = HAL_GPIO_ReadPin(DIG_IN2_GPIO_Port, DIG_IN2_Pin) == GPIO_PIN_SET;

    //=====================================================================================================
    // Compound Input/Output Logic
    //=====================================================================================================
    InputLogic();
    OutputLogic();

    //=====================================================================================================
    // Profet State Machine
    //=====================================================================================================
    for(int i=0; i<PDM_NUM_OUTPUTS; i++){
      Profet_SM(&pf[i], eDeviceState == DEVICE_RUN);
    }

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
    bDeviceOverTemp = MCP9808_GetOvertemp();
    bDeviceCriticalTemp = MCP9808_GetCriticalTemp();

    //=====================================================================================================
    // Status LEDs
    //=====================================================================================================
    LedUpdate(HAL_GetTick(), &StatusLed);
    LedUpdate(HAL_GetTick(), &ErrorLed);

    //=====================================================================================================
    // Check for CAN RX messages in queue
    //=====================================================================================================
    MsgQueueRx_t stMsgRx;
    osStatus_t eStatus;
    nMsgCnt = osMessageQueueGetCount(qMsgQueueRx);
    eStatus = osMessageQueueGet(qMsgQueueRx, &stMsgRx, NULL, 0U);
    if(eStatus == osOK){
      for(int i=0; i<PDM_NUM_CAN_INPUTS; i++){
        if(EvaluateCANInput(&stMsgRx.stCanRxHeader, stMsgRx.nRxData, &stPdmConfig.stCanInput[i], &nCanInputs[i]) == 1){
          //CAN input received
          stCanInputsRx[i].nLastRxTime = HAL_GetTick();
        }
      }

      //Burn Settings
      // 'B'
      if((MsgQueueRxCmd_t)stMsgRx.nRxData[0] == MSG_RX_BURN_SETTINGS){

    	  //Check special number sequence
    	  if(stMsgRx.nRxLen == 4){
					if((stMsgRx.nRxData[1] == 1) && (stMsgRx.nRxData[2] == 3) && (stMsgRx.nRxData[3] == 8)){
						//Write settings to FRAM
						uint8_t nRet = PdmConfig_Write(hi2c1, MB85RC_ADDRESS, &stPdmConfig);

						MsgQueueCanTx_t stMsgCanTx;

						stMsgCanTx.stTxHeader.DLC = 2;

						stMsgCanTx.nTxData[0] = MSG_TX_BURN_SETTINGS;
						stMsgCanTx.nTxData[1] = nRet;
						stMsgCanTx.nTxData[2] = 0;
						stMsgCanTx.nTxData[3] = 0;
						stMsgCanTx.nTxData[4] = 0;
						stMsgCanTx.nTxData[5] = 0;
						stMsgCanTx.nTxData[6] = 0;
						stMsgCanTx.nTxData[7] = 0;

						stMsgCanTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;

						osMessageQueuePut(qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);

            LedBlink(HAL_GetTick(), &StatusLed);
					}
    	  }
    	}

			//Check for settings change or request message
      if(stMsgRx.stCanRxHeader.StdId == stPdmConfig.stCanOutput.nBaseId - 1){
        PdmConfig_Set(&stPdmConfig, pVariableMap, pf, &stWiper, &stMsgRx, &qMsgQueueCanTx);
      }
    }

    //=====================================================================================================
    // Check CANInput receive time
    //=====================================================================================================
    for(int i=0; i<PDM_NUM_CAN_INPUTS; i++){
      stCanInputsRx[i].bRxOk = (HAL_GetTick() - stCanInputsRx[i].nLastRxTime) < stCanInputsRx[i].nRxMaxTime;
      //Set CANInput result to 0
      if(!stCanInputsRx[i].bRxOk){
        nCanInputs[i] = 0;
      }
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

/*
* @brief  CAN Transmit Task
* @param  thisThreadId: Pointer to this thread ID
* @param  hcan: Pointer to CAN Handle
*/
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
    Error_Handler(PDM_ERROR_CAN);
  }

  //Start the CAN peripheral
  if (HAL_CAN_Start(hcan) != HAL_OK)
  {
    /* Start Error */
    Error_Handler(PDM_ERROR_CAN);
  }

  //Activate CAN RX notification
  if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
    /* Notification Error */
    Error_Handler(PDM_ERROR_CAN);
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
        stPdmConfig.stCanOutput.nBaseId < 2048){ //2047 = max 11bit ID

      MsgQueueCanTx_t stMsgCanTx;

      osStatus_t stStatus;
      //Keep sending queued messages until empty
      do{
        stStatus = osMessageQueueGet(qMsgQueueCanTx, &stMsgCanTx, NULL, 0U);
        if(stStatus == osOK){
          stMsgCanTx.stTxHeader.ExtId = 0;
          stMsgCanTx.stTxHeader.IDE = CAN_ID_STD;
          stMsgCanTx.stTxHeader.RTR = CAN_RTR_DATA;
          stMsgCanTx.stTxHeader.TransmitGlobalTime = DISABLE;

          USB_Tx_SLCAN(&stMsgCanTx.stTxHeader, stMsgCanTx.nTxData);
          if(HAL_CAN_AddTxMessage(hcan, &stMsgCanTx.stTxHeader, stMsgCanTx.nTxData, &nCanTxMailbox) != HAL_OK){
            //Send failed - add back to queue
            osMessageQueuePut(qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);
          }
        }
        //Pause for preemption - TX is not that important
        osDelay(CAN_TX_MSG_SPLIT);
      }while(stStatus == osOK);

      //Send periodic messages
      SendMsg0(hcan);
      SendMsg1(hcan);
      SendMsg2(hcan);
      SendMsg3(hcan);
      SendMsg4(hcan);
      SendMsg5(hcan);
      SendMsg6(hcan);
      SendMsg7(hcan);
      SendMsg8(hcan);
      SendMsg9(hcan);
      SendMsg10(hcan);

      if(true){//(bSoftwareConnected){
        SendMsg11(hcan);
        SendMsg12(hcan);
        SendMsg13(hcan);
        SendMsg14(hcan);
        SendMsg15(hcan);
        SendMsg16(hcan);
        SendMsg17(hcan);
      }
    }

#ifdef MEAS_HEAP_USE
      __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif

    //Debug GPIO
    HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_RESET);
    
    osDelay(stPdmConfig.stCanOutput.nUpdateTime);
  }
}

void SendMsg17(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 17 (Wiper)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 17;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = (*pVariableMap[60] << 1) + *pVariableMap[59];
  nCanTxData[1] = stWiper.eState;
  nCanTxData[2] = stWiper.eSelectedSpeed;
  nCanTxData[3] = 0;
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;

  SendMsg(hcan, true);
}

void SendMsg16(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 16 (Virtual Inputs 9-16)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 16;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = nVirtInputs[8];
  nCanTxData[1] = nVirtInputs[9];
  nCanTxData[2] = nVirtInputs[10];
  nCanTxData[3] = nVirtInputs[11];
  nCanTxData[4] = nVirtInputs[12];
  nCanTxData[5] = nVirtInputs[13];
  nCanTxData[6] = nVirtInputs[14];
  nCanTxData[7] = nVirtInputs[15];

  SendMsg(hcan, true);
}

void SendMsg15(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 15 (Virtual Inputs 1-8)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 15;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = nVirtInputs[0];
  nCanTxData[1] = nVirtInputs[1];
  nCanTxData[2] = nVirtInputs[2];
  nCanTxData[3] = nVirtInputs[3];
  nCanTxData[4] = nVirtInputs[4];
  nCanTxData[5] = nVirtInputs[5];
  nCanTxData[6] = nVirtInputs[6];
  nCanTxData[7] = nVirtInputs[7];

  SendMsg(hcan, true);
}

void SendMsg14(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 14 (CAN Inputs 25-32)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 14;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = nCanInputs[24];
  nCanTxData[1] = nCanInputs[25];
  nCanTxData[2] = nCanInputs[26];
  nCanTxData[3] = nCanInputs[27];
  nCanTxData[4] = nCanInputs[28];
  nCanTxData[5] = nCanInputs[29];
  nCanTxData[6] = nCanInputs[30];
  nCanTxData[7] = nCanInputs[31];

  SendMsg(hcan, true);
}

void SendMsg13(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 13 (CAN Inputs 17-24)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 13;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = nCanInputs[16];
  nCanTxData[1] = nCanInputs[17];
  nCanTxData[2] = nCanInputs[18];
  nCanTxData[3] = nCanInputs[19];
  nCanTxData[4] = nCanInputs[20];
  nCanTxData[5] = nCanInputs[21];
  nCanTxData[6] = nCanInputs[22];
  nCanTxData[7] = nCanInputs[23];

  SendMsg(hcan, true);
}

void SendMsg12(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 12 (CAN Inputs 9-16)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 12;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = nCanInputs[8];
  nCanTxData[1] = nCanInputs[9];
  nCanTxData[2] = nCanInputs[10];
  nCanTxData[3] = nCanInputs[11];
  nCanTxData[4] = nCanInputs[12];
  nCanTxData[5] = nCanInputs[13];
  nCanTxData[6] = nCanInputs[14];
  nCanTxData[7] = nCanInputs[15];

  SendMsg(hcan, true);
}

void SendMsg11(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 11 (CAN Inputs 1-8)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 11;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = nCanInputs[0];
  nCanTxData[1] = nCanInputs[1];
  nCanTxData[2] = nCanInputs[2];
  nCanTxData[3] = nCanInputs[3];
  nCanTxData[4] = nCanInputs[4];
  nCanTxData[5] = nCanInputs[5];
  nCanTxData[6] = nCanInputs[6];
  nCanTxData[7] = nCanInputs[7];

  SendMsg(hcan, true);
}

void SendMsg10(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 10 (Flashers 1-4)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 10;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = ((nOutputFlasher[stPdmConfig.stFlasher[3].nOutput] & 0x01) << 3) + ((nOutputFlasher[stPdmConfig.stFlasher[2].nOutput] & 0x01) << 2) +
                  ((nOutputFlasher[stPdmConfig.stFlasher[1].nOutput] & 0x01) << 1) + (nOutputFlasher[stPdmConfig.stFlasher[0].nOutput] & 0x01);
  nCanTxData[1] = ((*stPdmConfig.stFlasher[3].pInput & 0x01) << 3) + ((*stPdmConfig.stFlasher[2].pInput & 0x01) << 2) +
                  ((*stPdmConfig.stFlasher[1].pInput & 0x01) << 1) + (*stPdmConfig.stFlasher[0].pInput & 0x01);
  nCanTxData[2] = 0;
  nCanTxData[3] = 0;
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;

  SendMsg(hcan, true);
}

void SendMsg9(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 9 (Out 1-8 Reset Count)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 9;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = pf[0].nOC_Count;
  nCanTxData[1] = pf[1].nOC_Count;
  nCanTxData[2] = pf[2].nOC_Count;
  nCanTxData[3] = pf[3].nOC_Count;
  nCanTxData[4] = pf[4].nOC_Count;
  nCanTxData[5] = pf[5].nOC_Count;
  nCanTxData[6] = pf[6].nOC_Count;
  nCanTxData[7] = pf[7].nOC_Count;

  SendMsg(hcan, true);
}

void SendMsg8(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 8 (Unused)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 8;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = 0;
  nCanTxData[1] = 0;
  nCanTxData[2] = 0;
  nCanTxData[3] = 0;
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;

  SendMsg(hcan, true);
}

void SendMsg7(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 7 (Out 5-8 Current Limit)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 7;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = pf[4].nIL_Limit >> 8;
  nCanTxData[1] = pf[4].nIL_Limit;
  nCanTxData[2] = pf[5].nIL_Limit >> 8;
  nCanTxData[3] = pf[5].nIL_Limit;
  nCanTxData[4] = pf[6].nIL_Limit >> 8;
  nCanTxData[5] = pf[6].nIL_Limit;
  nCanTxData[6] = pf[7].nIL_Limit >> 8;
  nCanTxData[7] = pf[7].nIL_Limit;

  SendMsg(hcan, true);
}

void SendMsg6(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 6 (Out 1-4 Current Limit)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 6;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = pf[0].nIL_Limit >> 8;
  nCanTxData[1] = pf[0].nIL_Limit;
  nCanTxData[2] = pf[1].nIL_Limit >> 8;
  nCanTxData[3] = pf[1].nIL_Limit;
  nCanTxData[4] = pf[2].nIL_Limit >> 8;
  nCanTxData[5] = pf[2].nIL_Limit;
  nCanTxData[6] = pf[3].nIL_Limit >> 8;
  nCanTxData[7] = pf[3].nIL_Limit;

  SendMsg(hcan, true);
}

void SendMsg5(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 5 (Out 1-8 Status)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 5;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = (pf[1].eState << 4) + pf[0].eState;
  nCanTxData[1] = (pf[3].eState << 4) + pf[2].eState;
  nCanTxData[2] = (pf[5].eState << 4) + pf[4].eState;
  nCanTxData[3] = (pf[7].eState << 4) + pf[6].eState;
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;

  SendMsg(hcan, true);
}

void SendMsg4(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 4 (Unused)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 4;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = 0;
  nCanTxData[1] = 0;
  nCanTxData[2] = 0;
  nCanTxData[3] = 0;
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;

  SendMsg(hcan, true);
}

void SendMsg3(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 3 (Out 5-8 Current)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 3;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = pf[4].nIL >> 8;
  nCanTxData[1] = pf[4].nIL;
  nCanTxData[2] = pf[5].nIL >> 8;
  nCanTxData[3] = pf[5].nIL;
  nCanTxData[4] = pf[6].nIL >> 8;
  nCanTxData[5] = pf[6].nIL;
  nCanTxData[6] = pf[7].nIL >> 8;
  nCanTxData[7] = pf[7].nIL;

  SendMsg(hcan, true);
}

void SendMsg2(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 2 (Out 1-4 Current)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 2;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = pf[0].nIL >> 8;
  nCanTxData[1] = pf[0].nIL;
  nCanTxData[2] = pf[1].nIL >> 8;
  nCanTxData[3] = pf[1].nIL;
  nCanTxData[4] = pf[2].nIL >> 8;
  nCanTxData[5] = pf[2].nIL;
  nCanTxData[6] = pf[3].nIL >> 8;
  nCanTxData[7] = pf[3].nIL;

  SendMsg(hcan, true);
}

void SendMsg1(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 1 ( )
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 1;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = 0;
  nCanTxData[1] = 0;
  nCanTxData[2] = 0;
  nCanTxData[3] = 0;
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;

  SendMsg(hcan, true);
}

void SendMsg0(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 0 (Digital inputs 1-2) and Device Status
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 0;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = (nPdmInputs[1] << 1) + nPdmInputs[0];
  nCanTxData[1] = 0;
  nCanTxData[2] = nILTotal >> 8;
  nCanTxData[3] = nILTotal;
  nCanTxData[4] = nBattSense >> 8;
  nCanTxData[5] = nBattSense;
  nCanTxData[6] = nBoardTempC >> 8;
  nCanTxData[7] = nBoardTempC;

  SendMsg(hcan, true);
}

/*
* @brief Send CAN message over USB and CAN
* @param hcan: Pointer to CAN Handle
* @param bDelay: True to add delay after sending
* @retval None
*/
void SendMsg(CAN_HandleTypeDef *hcan, bool bDelay)
{
  if (bUsbReady) //and bSoftwareConnected
  {
    if (USB_Tx_SLCAN(&stCanTxHeader, nCanTxData) != USBD_OK)
    {
      //Error_Handler();
      //TODO: Throws error if not connected to software
    }
  }

  if (HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK)
  {
    Error_Handler(PDM_ERROR_CAN);
  }

  if(bDelay)
    osDelay(CAN_TX_MSG_SPLIT);
}

/*
* @brief Input logic combining physical and virtual inputs
* @param None
* @retval None
*/
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

  //Flasher not used - set to 1
  for(int i=0; i<PDM_NUM_OUTPUTS; i++){
      if( (stPdmConfig.stFlasher[0].nOutput != i) &&
          (stPdmConfig.stFlasher[1].nOutput != i) &&
          (stPdmConfig.stFlasher[2].nOutput != i) &&
          (stPdmConfig.stFlasher[3].nOutput != i))
        nOutputFlasher[i] = 1;
  }

  //Set flasher outputs
  for(int i=0; i<PDM_NUM_FLASHERS; i++){
    EvaluateFlasher(&stPdmConfig.stFlasher[i], nOutputFlasher);
  }
}

/*
* @brief Output logic combining output enable, starter disable, and flasher
* @param None
* @retval None
*/
void OutputLogic(){
  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
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
  pf[0].fKILIS = 254795;

  pf[1].eModel = BTS7002_1EPP;
  pf[1].nNum = 1;
  pf[1].nIN_Port = PF_IN2_GPIO_Port;
  pf[1].nIN_Pin = PF_IN2_Pin;
  pf[1].nDEN_Port = PF_DEN2_GPIO_Port;
  pf[1].nDEN_Pin = PF_DEN2_Pin;
  pf[1].fKILIS = 254795;

  pf[2].eModel = BTS7008_2EPA_CH1;
  pf[2].nNum = 2;
  pf[2].nIN_Port = PF_IN3_GPIO_Port;
  pf[2].nIN_Pin = PF_IN3_Pin;
  pf[2].nDEN_Port = PF_DEN3_4_GPIO_Port;
  pf[2].nDEN_Pin = PF_DEN3_4_Pin;
  pf[2].fKILIS = 59258;

  pf[3].eModel = BTS7008_2EPA_CH2;
  pf[3].eState = OFF;
  pf[3].nNum = 3;
  pf[3].nIN_Port = PF_IN4_GPIO_Port;
  pf[3].nIN_Pin = PF_IN4_Pin;
  pf[3].nDEN_Port = PF_DEN3_4_GPIO_Port;
  pf[3].nDEN_Pin = PF_DEN3_4_Pin;
  pf[3].fKILIS = 59258;

  pf[4].eModel = BTS7008_2EPA_CH1;
  pf[4].eState = OFF;
  pf[4].nNum = 4;
  pf[4].nIN_Port = PF_IN5_GPIO_Port;
  pf[4].nIN_Pin = PF_IN5_Pin;
  pf[4].nDEN_Port = PF_DEN5_6_GPIO_Port;
  pf[4].nDEN_Pin = PF_DEN5_6_Pin;
  pf[4].fKILIS = 59258;

  pf[5].eModel = BTS7008_2EPA_CH2;
  pf[5].eState = OFF;
  pf[5].nNum = 5;
  pf[5].nIN_Port = PF_IN6_GPIO_Port;
  pf[5].nIN_Pin = PF_IN6_Pin;
  pf[5].nDEN_Port = PF_DEN5_6_GPIO_Port;
  pf[5].nDEN_Pin = PF_DEN5_6_Pin;
  pf[5].fKILIS = 59258;

  pf[6].eModel = BTS7008_2EPA_CH1;
  pf[6].eState = OFF;
  pf[6].nNum = 6;
  pf[6].nIN_Port = PF_IN7_GPIO_Port;
  pf[6].nIN_Pin = PF_IN7_Pin;
  pf[6].nDEN_Port = PF_DEN7_8_GPIO_Port;
  pf[6].nDEN_Pin = PF_DEN7_8_Pin;
  pf[6].fKILIS = 59258;

  pf[7].eModel = BTS7008_2EPA_CH2;
  pf[7].eState = OFF;
  pf[7].nNum = 7;
  pf[7].nIN_Port = PF_IN8_GPIO_Port;
  pf[7].nIN_Pin = PF_IN8_Pin;
  pf[7].nDEN_Port = PF_DEN7_8_GPIO_Port;
  pf[7].nDEN_Pin = PF_DEN7_8_Pin;
  pf[7].fKILIS = 59258;
}

/*
* @brief  PDM Configuration Initialisation
* @note   This function must be called before any tasks are started
* @param  hi2c1: Pointer to I2C Handle
* @retval PDM_OK if successful
*/
uint8_t InitPdmConfig(I2C_HandleTypeDef* hi2c1)
{
  //PdmConfig_SetDefault(&stPdmConfig);
  //PdmConfig_Write(hi2c1, MB85RC_ADDRESS, &stPdmConfig);

//TODO: Handle reading config at boot properly

   //Check that the data is correct, comms are OK, and FRAM device is the right ID
  if(PdmConfig_Check(hi2c1, MB85RC_ADDRESS, &stPdmConfig) == PDM_OK)
  {
    if(PdmConfig_Read(hi2c1, MB85RC_ADDRESS, &stPdmConfig) != PDM_OK)
    {
      PdmConfig_SetDefault(&stPdmConfig);
      //ErrorState(PDM_ERROR_FRAM_READ);
    }
  }
  else
  {
    PdmConfig_SetDefault(&stPdmConfig);
    //ErrorState(PDM_ERROR_FRAM_READ);
  }

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

  //Set the physical IO
  stPdmConfig.stInput[0].GPIOx = DIG_IN1_GPIO_Port;
  stPdmConfig.stInput[0].nPin = DIG_IN1_Pin;
  stPdmConfig.stInput[1].GPIOx = DIG_IN2_GPIO_Port;
  stPdmConfig.stInput[1].nPin = DIG_IN2_Pin;

  //User inputs
  for(int i=0; i<PDM_NUM_INPUTS; i++)
  {
    nPdmInputs[i] = 0;
    pVariableMap[i+1] = &nPdmInputs[i];
  
    SetInputPull(stPdmConfig.stInput[i].GPIOx, stPdmConfig.stInput[i].nPin, stPdmConfig.stInput[i].ePull);
  }

  //CAN inputs
  for(int i=0; i<PDM_NUM_CAN_INPUTS; i++)
  {
    nCanInputs[i] = 0;
    stCanInputsRx[i].nRxMaxTime = 2000; //2 second maximum receive time
    pVariableMap[i + 3] = &nCanInputs[i];
  }

  for(int i=0; i<PDM_NUM_VIRT_INPUTS; i++)
  {
    nVirtInputs[i] = 0;
    pVariableMap[i + 35] = &nVirtInputs[i];
  }

  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    pVariableMap[i + 51] = &nOutputs[i];
  }

  pVariableMap[59] = &stWiper.nSlowOut;
  pVariableMap[60] = &stWiper.nFastOut;

  nAlwaysTrue = 1;
  pVariableMap[61] = &nAlwaysTrue;

  //Assign variable map values
  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    stPdmConfig.stOutput[i].pInput = pVariableMap[stPdmConfig.stOutput[i].nInput];
  }

  //Map input values to config structure
  for(int i=0; i<PDM_NUM_INPUTS; i++)
  {
    stPdmConfig.stInput[i].pInput = &bRawUserInput[i];

    //If inverted, set last state to true to detect starting as false
    if(stPdmConfig.stInput[i].bInvert){
      stPdmConfig.stInput[i].stInVars.bLastState = true;
    }
  }

  for(int i=0; i<PDM_NUM_VIRT_INPUTS; i++)
  {
    stPdmConfig.stVirtualInput[i].pVar0 = pVariableMap[stPdmConfig.stVirtualInput[i].nVar0];
    stPdmConfig.stVirtualInput[i].pVar1 = pVariableMap[stPdmConfig.stVirtualInput[i].nVar1];
    stPdmConfig.stVirtualInput[i].pVar2 = pVariableMap[stPdmConfig.stVirtualInput[i].nVar2];
  }

  stWiper.nEnabled = stPdmConfig.stWiper.nEnabled;
  stWiper.eMode = stPdmConfig.stWiper.nMode;
  //stPdmConfig.stWiper.nParkStopLevel;
  stWiper.nWashWipeCycles = stPdmConfig.stWiper.nWashWipeCycles;
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


