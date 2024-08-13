#include "dingo_pdm.h"


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
// Sleep (STM32 Stop Mode)
//========================================================================
#define ENABLE_SLEEP 1
#define SLEEP_TIMEOUT 30000

//========================================================================
// Device State
//========================================================================
DeviceState_t eDeviceState = DEVICE_POWER_ON;
DeviceState_t eLastDeviceState;
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
osMessageQueueId_t qMsgQueueTx;

//========================================================================
// Profets
//========================================================================
volatile ProfetTypeDef pf[PDM_NUM_OUTPUTS];
volatile uint16_t nILTotal;
volatile bool bAnyOutputOvercurrent = false;
volatile bool bAnyOutputFault = false;

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
CanSpeed_t eCanSpeed = BITRATE_500K;
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
uint16_t nFlashers[PDM_NUM_FLASHERS];
uint16_t nAlwaysTrue;

uint32_t nMsgCnt;

//========================================================================
// Status LEDs
//========================================================================
void StatusLedOn (void) {HAL_GPIO_WritePin(StatusLED_GPIO_Port, StatusLED_Pin, GPIO_PIN_SET);}
void StatusLedOff (void) {HAL_GPIO_WritePin(StatusLED_GPIO_Port, StatusLED_Pin, GPIO_PIN_RESET);}
Led_Output StatusLed = {StatusLedOn, StatusLedOff, false, 0};

void ErrorLedOn (void) {HAL_GPIO_WritePin(ErrorLED_GPIO_Port, ErrorLED_Pin, GPIO_PIN_SET);}
void ErrorLedOff (void) {HAL_GPIO_WritePin(ErrorLED_GPIO_Port, ErrorLED_Pin, GPIO_PIN_RESET);}
Led_Output ErrorLed = {ErrorLedOn, ErrorLedOff, true, 0};

//========================================================================
// Low Power Stop
//========================================================================
bool bSleepMsgReceived = false;
uint8_t nNumOutputsOn = 0;
uint8_t nLastNumOutputsOn = 8;
uint32_t nAllOutputsOffTime = 0;
uint8_t nTXBeforeSleep = 0;

//========================================================================
// Messages
//========================================================================
bool bLastRunInfoMsg = false;
bool bLastOvertempMsg = false;
bool bLastCritTempMsg = false;
bool bLastBattLowMsg = false;
bool bLastBattHighMsg = false;

//========================================================================
// Blink Keypad
//========================================================================
BlinkMarineKeypad_t stKeypad;

//========================================================================
// Local Function Prototypes
//========================================================================
void InputLogic();
void OutputLogic();
void Profet_Default_Init();
void SendKeypadMsg(CAN_HandleTypeDef *hcan);
void SendMsg5(CAN_HandleTypeDef *hcan);
void SendMsg4(CAN_HandleTypeDef *hcan);
void SendMsg3(CAN_HandleTypeDef *hcan);
void SendMsg2(CAN_HandleTypeDef *hcan);
void SendMsg1(CAN_HandleTypeDef *hcan);
void SendMsg0(CAN_HandleTypeDef *hcan);
void SendMsg(CAN_HandleTypeDef *hcan, bool bDelay);
bool NewTxMsg(bool bState, bool bLastState, MsgType_t eType, MsgSrc_t eSrc, uint8_t nParam1, uint8_t nParam2, uint8_t nParam3);

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
    NewTxMsg(true, false, MSG_TYPE_WARNING, MSG_SRC_CAN, 0, 0, 0);
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
      NewTxMsg(true, false, MSG_TYPE_INFO, MSG_SRC_STATE_POWER_ON, 0, 0, 0);
      eLastDeviceState = eDeviceState;
      eDeviceState = DEVICE_STARTING;
      
      break;

    case DEVICE_STARTING:
      NewTxMsg(true, false, MSG_TYPE_INFO, MSG_SRC_STATE_STARTING, 0, 0, 0);
      //=====================================================================================================
      // USB initialization
      //=====================================================================================================
      bUsbReady = USB_Init(USB_MsgRcv) == USBD_OK;
      if(!bUsbReady){
        FatalError(FATAL_ERROR_USB);
      }

      //=====================================================================================================
      // MCP9808 Temperature Sensor Configuration
      //=====================================================================================================
      if(MCP9808_Init(hi2c1, MCP9808_ADDRESS) != MCP9808_OK)
        FatalError(FATAL_ERROR_TEMP_SENSOR);

      MCP9808_SetResolution(hi2c1, MCP9808_ADDRESS, MCP9808_RESOLUTION_0_5DEG);

      if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_UPPER_TEMP, BOARD_TEMP_MAX) != MCP9808_OK)
        FatalError(FATAL_ERROR_TEMP_SENSOR);
      if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_LOWER_TEMP, BOARD_TEMP_MIN) != MCP9808_OK)
        FatalError(FATAL_ERROR_TEMP_SENSOR);
      if(MCP9808_SetLimit(hi2c1, MCP9808_ADDRESS, MCP9808_REG_CRIT_TEMP, BOARD_TEMP_CRIT) != MCP9808_OK)
        FatalError(FATAL_ERROR_TEMP_SENSOR);

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
        FatalError(FATAL_ERROR_ADC);

      //=====================================================================================================
      // Init Profet Settings
      //=====================================================================================================
      Profet_Default_Init();    

      //=====================================================================================================
      // Set CAN Transceiver to High Speed Mode
      //=====================================================================================================
      HAL_GPIO_WritePin(EXTRA3_GPIO_Port, EXTRA3_Pin, GPIO_PIN_RESET);
      
      for(int i=0; i<PDM_NUM_INPUTS; i++)
        stPdmConfig.stInput[i].stInVars.bInit = false;

      eLastDeviceState = eDeviceState;
      eDeviceState = DEVICE_RUN;

      break;

    case DEVICE_RUN:
      eLastDeviceState = eDeviceState;
      bLastRunInfoMsg = NewTxMsg((eLastDeviceState != DEVICE_RUN), bLastRunInfoMsg, MSG_TYPE_INFO, MSG_SRC_STATE_RUN, 0, 0, 0);

      if(bAnyOutputOvercurrent && !bAnyOutputFault){
        LedBlink(HAL_GetTick(), &StatusLed);
        LedSetSteady(&ErrorLed, false);
      }

      if(bAnyOutputFault){
        LedBlink(HAL_GetTick(), &StatusLed);
        LedSetSteady(&ErrorLed, true);
      }

      if(!bAnyOutputOvercurrent && !bAnyOutputFault){
        LedSetSteady(&StatusLed, true);
        LedSetSteady(&ErrorLed, false);
      }

      bLastOvertempMsg = NewTxMsg((bDeviceOverTemp && !bDeviceCriticalTemp), bLastOvertempMsg, MSG_TYPE_WARNING, MSG_SRC_STATE_OVERTEMP, nBoardTempC, 0, 0);
      if (bDeviceOverTemp && !bDeviceCriticalTemp)
      {
        eLastDeviceState = eDeviceState;
        eDeviceState = DEVICE_OVERTEMP;
      }

      bLastCritTempMsg = NewTxMsg(bDeviceCriticalTemp, bLastCritTempMsg, MSG_TYPE_ERROR, MSG_SRC_STATE_OVERTEMP, nBoardTempC, 0, 0);
      if (bDeviceCriticalTemp)
      {
        nDeviceError = FATAL_ERROR_TEMP;
      }

      bLastBattLowMsg = NewTxMsg((nBattSense < 100), bLastBattLowMsg, MSG_TYPE_WARNING, MSG_SRC_VOLTAGE, nBattSense, 0, 0);

      bLastBattHighMsg = NewTxMsg((nBattSense > 160), bLastBattHighMsg, MSG_TYPE_WARNING, MSG_SRC_VOLTAGE, nBattSense, 0, 0);

      if(ENABLE_SLEEP){

        if(bSleepMsgReceived){
          bSleepMsgReceived = false;
          eLastDeviceState = eDeviceState;
          eDeviceState = DEVICE_SLEEP;
          nTXBeforeSleep = 0;
        }
        else{
          //Count number of outputs on
          nNumOutputsOn = 0;
          for(int i=0; i<PDM_NUM_OUTPUTS; i++){
            if(!pf[i].eState == OFF){
              nNumOutputsOn++;
            }  
          }

          //All outputs just turned off, save time
          if((nNumOutputsOn == 0) && (nLastNumOutputsOn > 0)){
            nAllOutputsOffTime = HAL_GetTick();
          }
          nLastNumOutputsOn = nNumOutputsOn;
          
          //No outputs on, no CAN msgs received and no USB connected
          //Go to sleep after timeout
          if( (nNumOutputsOn == 0) && (nLastNumOutputsOn == 0) &&
              !USB_IsConnected()){
            if(((HAL_GetTick() - nAllOutputsOffTime) > SLEEP_TIMEOUT) && 
                ((HAL_GetTick() - nLastCanUpdate) > SLEEP_TIMEOUT)){
                  eLastDeviceState = eDeviceState;
                  eDeviceState = DEVICE_SLEEP;
                  nTXBeforeSleep = 0;
              }
          }
        }
      }
      break;

    case DEVICE_SLEEP:
      if (eLastDeviceState != DEVICE_SLEEP)
      {
        eLastDeviceState = eDeviceState;
        NewTxMsg(true, false, MSG_TYPE_INFO, MSG_SRC_STATE_SLEEP, 0, 0, 0);
      }
              
      //Wait for messages to TX
      //Must be more than 1 in case state changed in the middle of last TX
      if(nTXBeforeSleep > 1){
        LedSetSteady(&StatusLed, false);
        //Set CAN transceiver to Standby (power saving)
        HAL_GPIO_WritePin(EXTRA3_GPIO_Port, EXTRA3_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(CAN_STBY_GPIO_Port, CAN_STBY_Pin, GPIO_PIN_SET);
        EnterStopMode(eCanSpeed);
        //Resume here

        //Set time to now so it doesn't trigger sleep right away
        nAllOutputsOffTime = HAL_GetTick();
        nLastCanUpdate = HAL_GetTick();

        eLastDeviceState = eDeviceState;
        eDeviceState = DEVICE_WAKEUP;
      }
      break;

    case DEVICE_WAKEUP:
      //Set CAN transceiver back to High Speed Mode
      HAL_GPIO_WritePin(EXTRA3_GPIO_Port, EXTRA3_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(CAN_STBY_GPIO_Port, CAN_STBY_Pin, GPIO_PIN_RESET);
      LedSetSteady(&StatusLed, true);
      NewTxMsg(true, false, MSG_TYPE_INFO, MSG_SRC_STATE_WAKE, 0, 0, 0);
      eLastDeviceState = eDeviceState;
      eDeviceState = DEVICE_RUN;
      break;

    case DEVICE_OVERTEMP:
      //Go back to run when temp falls
      if (eLastDeviceState != DEVICE_OVERTEMP)
      {
        eLastDeviceState = eDeviceState;
        NewTxMsg(true, false, MSG_TYPE_INFO, MSG_SRC_STATE_OVERTEMP, nBoardTempC, 0, 0);
      }

      LedBlink(HAL_GetTick(), &StatusLed);
      LedBlink(HAL_GetTick(), &ErrorLed);

      bLastCritTempMsg = NewTxMsg(bDeviceCriticalTemp, bLastCritTempMsg, MSG_TYPE_ERROR, MSG_SRC_STATE_OVERTEMP, nBoardTempC, 0, 0);
      if (bDeviceCriticalTemp)
      {
        nDeviceError = FATAL_ERROR_TEMP;
      }

      if (!bDeviceOverTemp)
      {
        LedSetSteady(&StatusLed, true);
        eLastDeviceState = eDeviceState;
        eDeviceState = DEVICE_RUN;
      }
      break;

    case DEVICE_ERROR:
      //Error LED - flash error code
      //No way to recover, must power cycle
      //Gets trapped in FatalError
      NewTxMsg(true, false, MSG_TYPE_ERROR, MSG_SRC_STATE_ERROR, nDeviceError, 0, 0);
      LedSetSteady(&StatusLed, false);
      FatalError(nDeviceError);

      break;
    
    default:
      break;
    }

    //=====================================================================================================
    // Check for device errors
    //=====================================================================================================
    if(nDeviceError > 0){
      eLastDeviceState = eDeviceState;
      eDeviceState = DEVICE_ERROR;
    }

    //=====================================================================================================
    // ADC channels
    // Battery sense
    // Device temperature
    // VREFint
    //=====================================================================================================
    nBattSense = (uint16_t)(((float)nAdc1Data[ADC_1_BATT_SENSE]) * 0.089319);
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
    bAnyOutputOvercurrent = false;
    bAnyOutputFault = false;
    for(int i=0; i<PDM_NUM_OUTPUTS; i++){
      Profet_SM(&pf[i], eDeviceState == DEVICE_RUN);

      if (pf[i].eState == OVERCURRENT){
        bAnyOutputOvercurrent = true;
        if (pf[i].eLastState != OVERCURRENT){
          NewTxMsg(true, false, MSG_TYPE_WARNING, MSG_SRC_OVERCURRENT, i, pf[i].nIL, 0);
        }
      }

      if (pf[i].eState == FAULT){
        bAnyOutputFault = true;
        if (pf[i].eLastState != FAULT){
          NewTxMsg(true, false, MSG_TYPE_ERROR, MSG_SRC_OVERCURRENT, i, pf[i].nIL, 0);
        }
      }
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

			//Check for settings change or request message
      if((int)stMsgRx.stCanRxHeader.StdId == (int)(stPdmConfig.stCanOutput.nBaseId - 1)){

        //Burn Settings
        // 'B'
        if((MsgQueueRxCmd_t)stMsgRx.nRxData[0] == MSG_RX_BURN_SETTINGS){

          //Check special number sequence
          if(stMsgRx.nRxLen == 4){
            if((stMsgRx.nRxData[1] == 1) && (stMsgRx.nRxData[2] == 3) && (stMsgRx.nRxData[3] == 8)){
              //Write settings to FRAM
              uint8_t nRet = PdmConfig_Write(hi2c1, MB85RC_ADDRESS, &stPdmConfig);

              MsgQueueTx_t stMsgTx;

              stMsgTx.stTxHeader.DLC = 2;

              stMsgTx.nTxData[0] = MSG_TX_BURN_SETTINGS;
              stMsgTx.nTxData[1] = nRet;
              stMsgTx.nTxData[2] = 0;
              stMsgTx.nTxData[3] = 0;
              stMsgTx.nTxData[4] = 0;
              stMsgTx.nTxData[5] = 0;
              stMsgTx.nTxData[6] = 0;
              stMsgTx.nTxData[7] = 0;

              stMsgTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;

              osMessageQueuePut(qMsgQueueTx, &stMsgTx, 0U, 0U);

              LedBlink(HAL_GetTick(), &StatusLed);
            }
          }
        }

        //Sleep
        // 'Q'
        if((MsgQueueRxCmd_t)stMsgRx.nRxData[0] == MSG_RX_SLEEP){

          //Check special sequence 'UIT'
          if(stMsgRx.nRxLen == 4){
            if((stMsgRx.nRxData[1] == 85) && (stMsgRx.nRxData[2] == 73) && (stMsgRx.nRxData[3] == 84)){
              bSleepMsgReceived = true;

              MsgQueueTx_t stMsgTx;

              stMsgTx.stTxHeader.DLC = 2;

              stMsgTx.nTxData[0] = MSG_TX_SLEEP;
              stMsgTx.nTxData[1] = 1;
              stMsgTx.nTxData[2] = 0;
              stMsgTx.nTxData[3] = 0;
              stMsgTx.nTxData[4] = 0;
              stMsgTx.nTxData[5] = 0;
              stMsgTx.nTxData[6] = 0;
              stMsgTx.nTxData[7] = 0;

              stMsgTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;

              osMessageQueuePut(qMsgQueueTx, &stMsgTx, 0U, 0U);
            }
          }
        }
      
        //Check for config change or request message
        PdmConfig_Set(&stPdmConfig, pVariableMap, pf, &stWiper, &stMsgRx, &qMsgQueueTx);
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
  CAN_Init(hcan, eCanSpeed);

  //Set CAN Standby pin to low = enable
  HAL_GPIO_WritePin(EXTRA3_GPIO_Port, EXTRA3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(CAN_STBY_GPIO_Port, CAN_STBY_Pin, GPIO_PIN_RESET);

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
    FatalError(FATAL_ERROR_CAN);
  }

  //Start the CAN peripheral
  if (HAL_CAN_Start(hcan) != HAL_OK)
  {
    /* Start Error */
    FatalError(FATAL_ERROR_CAN);
  }

  //Activate CAN RX notification
  if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
    /* Notification Error */
    FatalError(FATAL_ERROR_CAN);
  }

  //Configure Transmission
  stCanTxHeader.StdId = 1620;
  stCanTxHeader.ExtId = 0;
  stCanTxHeader.RTR = CAN_RTR_DATA;
  stCanTxHeader.IDE = CAN_ID_STD;
  stCanTxHeader.DLC = 8;
  stCanTxHeader.TransmitGlobalTime = DISABLE;

  //Configure Blink keypad
  //BlinkMarineKeypad_Init(PKP_2600_SI, &stKeypad);

  for(;;){
    HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_SET);

    if(stPdmConfig.stCanOutput.nEnabled &&
        (stPdmConfig.stCanOutput.nUpdateTime > 0) &&
        stPdmConfig.stCanOutput.nBaseId > 0 &&
        stPdmConfig.stCanOutput.nBaseId < 2048){ //2047 = max 11bit ID

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

          USB_Tx_SLCAN(&stMsgTx.stTxHeader, stMsgTx.nTxData);
          if(HAL_CAN_AddTxMessage(hcan, &stMsgTx.stTxHeader, stMsgTx.nTxData, &nCanTxMailbox) != HAL_OK){
            //Send failed - add back to queue
            osMessageQueuePut(qMsgQueueTx, &stMsgTx, 0U, 0U);
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

      //SendKeypadMsg(hcan);
    }

    nTXBeforeSleep++;

#ifdef MEAS_HEAP_USE
      __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif

    //Debug GPIO
    HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_RESET);
    
    osDelay(stPdmConfig.stCanOutput.nUpdateTime);
  }
}

void SendKeypadMsg(CAN_HandleTypeDef *hcan)
{
  stKeypad.eBacklightColor = BL_YELLOW;
  stKeypad.nBacklightBrightness = 0x20;
  stKeypad.nButtonBrightness = 0x3F;

  //Backlight color
  stCanTxHeader.StdId = 0x515;
  stCanTxHeader.DLC = 8; // Bytes to send
  //Backlight brightness 0x00 to 0x3F
  nCanTxData[0] = stKeypad.nBacklightBrightness;
  //Backlight color
  nCanTxData[1] = stKeypad.eBacklightColor;
  nCanTxData[2] = 0;
  nCanTxData[3] = 0;
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;
  SendMsg(hcan, true);

  //Button brightness
  stCanTxHeader.StdId = 0x415;
  stCanTxHeader.DLC = 8; // Bytes to send
  //Button brightness 0x00 to 0x3F
  nCanTxData[0] = stKeypad.nButtonBrightness;
  nCanTxData[1] = 0;
  nCanTxData[2] = 0;
  nCanTxData[3] = 0;
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;
  SendMsg(hcan, true);

  //Solid color
  stCanTxHeader.StdId = 0x215;
  stCanTxHeader.DLC = 8; // Bytes to send
  //Red 1-8
  nCanTxData[0] = 0;
  //Green 1-4, Red 9-12
  nCanTxData[1] = ((pf[3].eState == ON)<<7) + ((pf[2].eState == ON)<<6) + ((pf[1].eState == ON)<<5) + ((pf[0].eState == ON)<<4);
  //Green 5-12
  nCanTxData[2] = ((pf[7].eState == ON)<<3) + ((pf[6].eState == ON)<<2) + ((pf[5].eState == ON)<<1) + (pf[4].eState == ON);
  //Blue 1-8
  nCanTxData[3] = ((pf[7].eState == OVERCURRENT)<<7) + ((pf[6].eState == OVERCURRENT)<<6) + ((pf[5].eState == OVERCURRENT)<<5) + ((pf[4].eState == OVERCURRENT)<<4) + ((pf[3].eState == OVERCURRENT)<<3) + ((pf[2].eState == OVERCURRENT)<<2) + ((pf[1].eState == OVERCURRENT)<<1) + (pf[0].eState == OVERCURRENT);
  //Blue 9-12
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;
  SendMsg(hcan, true);

  //Flashing color
  stCanTxHeader.StdId = 0x315;
  stCanTxHeader.DLC = 8; // Bytes to send
  //Red 1-8
  nCanTxData[0] = ((pf[7].eState == FAULT)<<7) + ((pf[6].eState == FAULT)<<6) + ((pf[5].eState == FAULT)<<5) + ((pf[4].eState == FAULT)<<4) + ((pf[3].eState == FAULT)<<3) + ((pf[2].eState == FAULT)<<2) + ((pf[1].eState == FAULT)<<1) + (pf[0].eState == FAULT);
  //Green 1-4, Red 9-12
  nCanTxData[1] = 0;
  //Green 5-12
  nCanTxData[2] = 0;
  //Blue 1-8
  nCanTxData[3] = 0;
  //Blue 9-12
  nCanTxData[4] = 0;
  nCanTxData[5] = 0;
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;
  SendMsg(hcan, true);
}

void SendMsg5(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 5 (CAN Inputs and Virtual Inputs)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 5;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = ((nCanInputs[7] & 0x01) << 7) + ((nCanInputs[6] & 0x01) << 6) + ((nCanInputs[5] & 0x01) << 5) + 
                  ((nCanInputs[4] & 0x01) << 4) + ((nCanInputs[3] & 0x01) << 3) + ((nCanInputs[2] & 0x01) << 2) + 
                  ((nCanInputs[1] & 0x01) << 1) + (nCanInputs[0] & 0x01);
  nCanTxData[1] = ((nCanInputs[15] & 0x01) << 7) + ((nCanInputs[14] & 0x01) << 6) + ((nCanInputs[13] & 0x01) << 5) + 
                  ((nCanInputs[12] & 0x01) << 4) + ((nCanInputs[11] & 0x01) << 3) + ((nCanInputs[10] & 0x01) << 2) + 
                  ((nCanInputs[9] & 0x01) << 1) + (nCanInputs[8] & 0x01);
  nCanTxData[2] = ((nCanInputs[23] & 0x01) << 7) + ((nCanInputs[22] & 0x01) << 6) + ((nCanInputs[21] & 0x01) << 5) + 
                  ((nCanInputs[20] & 0x01) << 4) + ((nCanInputs[19] & 0x01) << 3) + ((nCanInputs[18] & 0x01) << 2) + 
                  ((nCanInputs[17] & 0x01) << 1) + (nCanInputs[16] & 0x01);
  nCanTxData[3] = ((nCanInputs[31] & 0x01) << 7) + ((nCanInputs[30] & 0x01) << 6) + ((nCanInputs[29] & 0x01) << 5) + 
                  ((nCanInputs[28] & 0x01) << 4) + ((nCanInputs[27] & 0x01) << 3) + ((nCanInputs[26] & 0x01) << 2) + 
                  ((nCanInputs[25] & 0x01) << 1) + (nCanInputs[24] & 0x01);
  nCanTxData[4] = ((nVirtInputs[7] & 0x01) << 7) + ((nVirtInputs[6] & 0x01) << 6) + ((nVirtInputs[5] & 0x01) << 5) + 
                  ((nVirtInputs[4] & 0x01) << 4) + ((nVirtInputs[3] & 0x01) << 3) + ((nVirtInputs[2] & 0x01) << 2) + 
                  ((nVirtInputs[1] & 0x01) << 1) + (nVirtInputs[0] & 0x01);
  nCanTxData[5] = ((nVirtInputs[15] & 0x01) << 7) + ((nVirtInputs[14] & 0x01) << 6) + ((nVirtInputs[13] & 0x01) << 5) + 
                  ((nVirtInputs[12] & 0x01) << 4) + ((nVirtInputs[11] & 0x01) << 3) + ((nVirtInputs[10] & 0x01) << 2) + 
                  ((nVirtInputs[9] & 0x01) << 1) + (nVirtInputs[8] & 0x01);
  nCanTxData[6] = 0;
  nCanTxData[7] = 0;

  SendMsg(hcan, true);
}

void SendMsg4(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 4 (Out 1-8 Reset Count)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 4;
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

void SendMsg3(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 3 (Out 1-8 Status)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 3;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = (pf[1].eState << 4) + pf[0].eState;
  nCanTxData[1] = (pf[3].eState << 4) + pf[2].eState;
  nCanTxData[2] = (pf[5].eState << 4) + pf[4].eState;
  nCanTxData[3] = (pf[7].eState << 4) + pf[6].eState;
  nCanTxData[4] = (*pVariableMap[60] << 1) + *pVariableMap[59];
  nCanTxData[5] = (stWiper.eState << 4) + stWiper.eSelectedSpeed;
  nCanTxData[6] = ((nFlashers[3] & 0x01) << 3) + ((nFlashers[2] & 0x01) << 2) +
                  ((nFlashers[1] & 0x01) << 1) + (nFlashers[0] & 0x01) +
                  ((*stPdmConfig.stFlasher[3].pInput & 0x01) << 7) + ((*stPdmConfig.stFlasher[2].pInput & 0x01) << 6) +
                  ((*stPdmConfig.stFlasher[1].pInput & 0x01) << 5) + ((*stPdmConfig.stFlasher[0].pInput & 0x01) << 4);
  nCanTxData[7] = 0;

  SendMsg(hcan, true);
}

void SendMsg2(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 2 (Out 5-8 Current)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 2;
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

void SendMsg1(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 1 (Out 1-4 Current)
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 1;
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

void SendMsg0(CAN_HandleTypeDef *hcan)
{
  //=======================================================
  // Build Msg 0 (Digital inputs 1-2) and Device Status
  //=======================================================
  stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 0;
  stCanTxHeader.DLC = 8; // Bytes to send
  nCanTxData[0] = (nPdmInputs[1] << 1) + nPdmInputs[0];
  nCanTxData[1] = eDeviceState;
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
    //Too many messages 
    //NewTxMsg(MSG_TYPE_WARNING, MSG_SRC_CAN, 0, 0, 0);
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

  //Set flasher outputs
  for(int i=0; i<PDM_NUM_FLASHERS; i++)
  {
    EvaluateFlasher(HAL_GetTick(), &stPdmConfig.stFlasher[i], &nFlashers[i]);
  }
}

/*
* @brief Send a new CAN message to the TX queue
* @param bState: Current state of the message, set to true to always send
* @param bLastState: Last state of the message, set to false to always send
* @param eType: Message type
* @param nId: Message ID
* @param nParam1: Message parameter 1
* @param nParam2: Message parameter 2
* @param nParam3: Message parameter 3
* @retval None
*/
bool NewTxMsg(bool bState, bool bLastState, MsgType_t eType, MsgSrc_t eSrc, uint8_t nParam1, uint8_t nParam2, uint8_t nParam3){
    if(bState && !bLastState){
      MsgQueueTx_t stMsgTx;

      stMsgTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + CAN_TX_MSG_ID_OFFSET;
      stMsgTx.stTxHeader.ExtId = 0;
      stMsgTx.stTxHeader.IDE = CAN_ID_STD;
      stMsgTx.stTxHeader.RTR = CAN_RTR_DATA;
      stMsgTx.stTxHeader.DLC = 5;
      stMsgTx.nTxData[0] = eType;
      stMsgTx.nTxData[1] = eSrc;
      stMsgTx.nTxData[2] = nParam1;
      stMsgTx.nTxData[3] = nParam2;
      stMsgTx.nTxData[4] = nParam3;
      stMsgTx.nTxData[5] = 0;
      stMsgTx.nTxData[6] = 0;
      stMsgTx.nTxData[7] = 0;
      
      osMessageQueuePut(qMsgQueueTx, &stMsgTx, 0, 0);
    }
    return bState;
}

/*
* @brief Output logic combining output enable, starter disable, and flasher
* @param None
* @retval None
*/
void OutputLogic(){
  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    pf[i].eReqState = (ProfetStateTypeDef)(*stPdmConfig.stOutput[i].pInput && nStarterDisable[i]);
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
  pf[0].fKILIS = 229421;

  pf[1].eModel = BTS7002_1EPP;
  pf[1].nNum = 1;
  pf[1].nIN_Port = PF_IN2_GPIO_Port;
  pf[1].nIN_Pin = PF_IN2_Pin;
  pf[1].nDEN_Port = PF_DEN2_GPIO_Port;
  pf[1].nDEN_Pin = PF_DEN2_Pin;
  pf[1].fKILIS = 229421;

  pf[2].eModel = BTS7008_2EPA_CH1;
  pf[2].nNum = 2;
  pf[2].nIN_Port = PF_IN3_GPIO_Port;
  pf[2].nIN_Pin = PF_IN3_Pin;
  pf[2].nDEN_Port = PF_DEN3_4_GPIO_Port;
  pf[2].nDEN_Pin = PF_DEN3_4_Pin;
  pf[2].fKILIS = 59481;

  pf[3].eModel = BTS7008_2EPA_CH2;
  pf[3].eState = OFF;
  pf[3].nNum = 3;
  pf[3].nIN_Port = PF_IN4_GPIO_Port;
  pf[3].nIN_Pin = PF_IN4_Pin;
  pf[3].nDEN_Port = PF_DEN3_4_GPIO_Port;
  pf[3].nDEN_Pin = PF_DEN3_4_Pin;
  pf[3].fKILIS = 59481;

  pf[4].eModel = BTS7008_2EPA_CH1;
  pf[4].eState = OFF;
  pf[4].nNum = 4;
  pf[4].nIN_Port = PF_IN5_GPIO_Port;
  pf[4].nIN_Pin = PF_IN5_Pin;
  pf[4].nDEN_Port = PF_DEN5_6_GPIO_Port;
  pf[4].nDEN_Pin = PF_DEN5_6_Pin;
  pf[4].fKILIS = 59481;

  pf[5].eModel = BTS7008_2EPA_CH2;
  pf[5].eState = OFF;
  pf[5].nNum = 5;
  pf[5].nIN_Port = PF_IN6_GPIO_Port;
  pf[5].nIN_Pin = PF_IN6_Pin;
  pf[5].nDEN_Port = PF_DEN5_6_GPIO_Port;
  pf[5].nDEN_Pin = PF_DEN5_6_Pin;
  pf[5].fKILIS = 59481;

  pf[6].eModel = BTS7008_2EPA_CH1;
  pf[6].eState = OFF;
  pf[6].nNum = 6;
  pf[6].nIN_Port = PF_IN7_GPIO_Port;
  pf[6].nIN_Pin = PF_IN7_Pin;
  pf[6].nDEN_Port = PF_DEN7_8_GPIO_Port;
  pf[6].nDEN_Pin = PF_DEN7_8_Pin;
  pf[6].fKILIS = 59481;

  pf[7].eModel = BTS7008_2EPA_CH2;
  pf[7].eState = OFF;
  pf[7].nNum = 7;
  pf[7].nIN_Port = PF_IN8_GPIO_Port;
  pf[7].nIN_Pin = PF_IN8_Pin;
  pf[7].nDEN_Port = PF_DEN7_8_GPIO_Port;
  pf[7].nDEN_Pin = PF_DEN7_8_Pin;
  pf[7].fKILIS = 59481;
}

/*
* @brief  PDM Configuration Initialisation
* @note   This function must be called before any tasks are started
* @param  hi2c1: Pointer to I2C Handle
* @retval PDM_OK if successful
*/
uint8_t InitPdmConfig(I2C_HandleTypeDef* hi2c1)
{
  //NOTE: These lines must be uncommented when a new PDM is built or the config structure is changed
  //Otherwise the FRAM won't hold any config values and will fail when the byte length doesn't match
  //PdmConfig_SetDefault(&stPdmConfig);
  //PdmConfig_Write(hi2c1, MB85RC_ADDRESS, &stPdmConfig);

  //Check that the data is correct, comms are OK, and FRAM device is the right ID
  if(PdmConfig_Check(hi2c1, MB85RC_ADDRESS, &stPdmConfig) == PDM_OK)
  {
    if(PdmConfig_Read(hi2c1, MB85RC_ADDRESS, &stPdmConfig) != PDM_OK)
    {
      FatalError(FATAL_ERROR_FRAM);
    }
  }
  else
  {
    PdmConfig_SetDefault(&stPdmConfig);
    //Write default config to FRAM so it is correct on the next boot
    PdmConfig_Write(hi2c1, MB85RC_ADDRESS, &stPdmConfig);
    FatalError(FATAL_ERROR_CONFIG);
  }

  switch(stPdmConfig.stDevConfig.nCanSpeed){
    case 0:
      eCanSpeed = BITRATE_1000K;
      break;
    case 1:
      eCanSpeed = BITRATE_500K;
      break;
    case 2:
      eCanSpeed = BITRATE_250K;
      break;
    case 3:
      eCanSpeed = BITRATE_125K;
      break;
    default:
      eCanSpeed = BITRATE_500K;
      break;
  }

  //Map config to profet values
  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    pf[i].bEnabled = stPdmConfig.stOutput[i].nEnabled;
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

  pVariableMap[61] = &nFlashers[0];
  pVariableMap[62] = &nFlashers[1];
  pVariableMap[63] = &nFlashers[2];
  pVariableMap[64] = &nFlashers[3];

  nAlwaysTrue = 1;
  pVariableMap[65] = &nAlwaysTrue;

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
  stWiper.nParkStopLevel = stPdmConfig.stWiper.nParkStopLevel;
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


