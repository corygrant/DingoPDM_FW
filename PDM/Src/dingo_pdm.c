/*
 * dingo_pdm.c
 *
 *  Created on: Oct 22, 2020
 *      Author: coryg
 */

#include "dingo_pdm.h"
#include "pdm_config.h"

//TODO V3 Fix old analog input logic handling - to digital
//TODO V3 Change USB renumeration logic - changed hardware
//TODO V3 Add PCA9539 Profet GPIO reset pin logic
//TODO V3 Add SPI CS on hardware NSS
//TODO Remove mode? No MANUAL

//========================================================================
// Task Delays
//========================================================================
#define MAIN_TASK_DELAY 100
#define I2C_TASK_DELAY 5
#define CAN_TX_DELAY 50

//========================================================================
// I2C Addresses
//========================================================================
#define PCA9635_ADDRESS 0x30
#define MCP9808_ADDRESS 0x18
#define PCA9539_ADDRESS_BANK1 0x74
#define PCA9539_ADDRESS_BANK2 0x74
#define ADS1015_ADDRESS_PF_BANK1 0x48
#define ADS1015_ADDRESS_PF_BANK2 0x48
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
// STM ADC Counts
//========================================================================
#define ADC_1_COUNT 1
#define ADC_4_COUNT 1

//========================================================================
// CAN
//========================================================================
#define CAN_TX_BASE_ID 2000
#define CAN_TX_MSG_SPLIT 5 //ms

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

PdmConfig_t stPdmConfig;

//========================================================================
// Message Queues
//========================================================================
osMessageQueueId_t qMsgQueueRx;
osMessageQueueId_t qMsgQueueUsbTx;
osMessageQueueId_t qMsgQueueCanTx;

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
ads1x15Settings_t stAdcPfBank1, stAdcPfBank2;

//========================================================================
// PCAL9554B User Digital Inputs
//========================================================================
uint8_t nUserDigInputRaw;
uint8_t nUserDigInput[PDM_NUM_INPUTS];

//========================================================================
// Output Logic
//========================================================================
uint8_t nOutputLogic[PDM_NUM_OUTPUTS];

//========================================================================
// USB
//========================================================================
char cUsbBuffer[120];
bool bUsbConnected = false;

//========================================================================
// MCP9808 PCB Temperature
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
// Real Time Clock
//========================================================================
uint16_t nRtcYear;
uint8_t nRtcMonth, nRtcDay, nRtcHour, nRtcMinute, nRtcSecond;

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

uint8_t USBD_RxBuffer[USBD_RX_DATA_SIZE];
uint8_t USBD_TxBuffer[USBD_TX_DATA_SIZE];
USBD_HandleTypeDef hUSBD;

static int8_t USBD_CDC_Init(void);
static int8_t USBD_CDC_DeInit(void);
static int8_t USBD_CDC_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t USBD_CDC_Receive(uint8_t* pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_Interface_PDM =
{
  USBD_CDC_Init,
  USBD_CDC_DeInit,
  USBD_CDC_Control,
  USBD_CDC_Receive
};

uint8_t nUsbMsgTx[9]; //Add \r to Usb Tx message

uint8_t nManualOutputs[PDM_NUM_OUTPUTS];

uint8_t nReportingOn;
uint16_t nReportingDelay;
uint32_t nReportingLastTime;

uint8_t nCanEnable;
CAN_BitRate_t eCanBitRate;

//==============================================================================================================================================

/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_Init(void)
{
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUSBD, USBD_TxBuffer, 0);
  USBD_CDC_SetRxBuffer(&hUSBD, USBD_RxBuffer);
  return (USBD_OK);
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_DeInit(void)
{
  return (USBD_OK);
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:
      pbuf[0] = (uint8_t)(115200);
      pbuf[1] = (uint8_t)(115200 >> 8);
      pbuf[2] = (uint8_t)(115200 >> 16);
      pbuf[3] = (uint8_t)(115200 >> 24);
      pbuf[4] = 0; //Stop bits (1)
      pbuf[5] = 0; //Parity (none)
      pbuf[6] = 8; //Number of bits (8)
    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_Receive(uint8_t* Buf, uint32_t *Len)
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

  osMessageQueuePut(qMsgQueueRx, &stMsg, 0U, 0U);

  USBD_CDC_SetRxBuffer(&hUSBD, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUSBD);
  return (USBD_OK);
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t USBD_CDC_Transmit(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUSBD.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUSBD, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUSBD);
  return result;
}

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
void PdmMainTask(osThreadId_t* thisThreadId, ADC_HandleTypeDef* hadc1, ADC_HandleTypeDef* hadc4, RTC_HandleTypeDef* hrtc, CRC_HandleTypeDef* hcrc){

  HAL_ADC_Start_DMA(hadc1, (uint32_t*) nAdc1Data, ADC_1_COUNT);
  HAL_ADC_Start_DMA(hadc4, (uint32_t*) nAdc4Data, ADC_4_COUNT);

  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUSBD, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUSBD, &USBD_CDC) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_CDC_RegisterInterface(&hUSBD, &USBD_Interface_PDM) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_Start(&hUSBD) != USBD_OK)
  {
    Error_Handler();
  }

  /* Infinite loop */
  for(;;)
  {

    //=====================================================================================================
    // Standby
    //=====================================================================================================
    /* Check if the system was resumed from Standby mode */
    if ((__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET) ||
        (__HAL_PWR_GET_FLAG(PWR_FLAG_WU) != RESET))
    {
      /* Clear Standby flag */
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

      HAL_GPIO_WritePin(EXTRA3_GPIO_Port, EXTRA3_Pin, GPIO_PIN_RESET);

    }


    //Check standby pin
    //If no voltage - enter standby
    if(!(STANDBY_GPIO_Port->IDR & STANDBY_Pin)){

      HAL_GPIO_WritePin(EXTRA3_GPIO_Port, EXTRA3_Pin, GPIO_PIN_SET);

      HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2); //PC13

      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

      HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2); //PC13

      HAL_PWR_EnterSTANDBYMode();
    }
    else
    {
      HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2); //PC13
    }

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
    // USB Connection
    //=====================================================================================================
    if( (USB_VBUS_GPIO_Port->IDR & USB_VBUS_Pin) && !bUsbConnected){
      HAL_GPIO_WritePin(USB_PULLUP_GPIO_Port, USB_PULLUP_Pin, GPIO_PIN_SET);
      bUsbConnected = true;
    }

    if( !(USB_VBUS_GPIO_Port->IDR & USB_VBUS_Pin) && bUsbConnected){
      HAL_GPIO_WritePin(USB_PULLUP_GPIO_Port, USB_PULLUP_Pin, GPIO_PIN_RESET);
      bUsbConnected = false;
    }

    nILTotal = 0;
    for(int i=0;i<PDM_NUM_OUTPUTS;i++)
      nILTotal += pf[i].nIL;




      /*
      int nUsbNumBytes = sprintf(cUsbBuffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %c%c%c%c%c%c%c%c%c%c%c%c", pf[0].nIL, pf[1].nIL, pf[2].nIL, pf[3].nIL,
                                                                    pf[4].nIL, pf[5].nIL, pf[6].nIL, pf[7].nIL,
                                                                    pf[8].nIL, pf[9].nIL, pf[10].nIL, pf[11].nIL, nILTotal, (uint16_t)fBoardTempC,
                                                                    pf[0].cState, pf[1].cState, pf[2].cState, pf[3].cState,
                                                                    pf[4].cState, pf[5].cState, pf[6].cState, pf[7].cState,
                                                                    pf[8].cState, pf[9].cState, pf[10].cState, pf[11].cState);

      //CRC init in main.c must be set up as:
      //
      //  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
      //  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
      //  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
      //  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
      //  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
      //
      //  AND CRC value must be inverted after calculating
      //

      uint32_t nUsbCrc = HAL_CRC_Calculate(hcrc, (uint32_t *)cUsbBuffer, nUsbNumBytes);
      nUsbCrc = ~nUsbCrc;
      char sCrc[12];
      sprintf(sCrc, " %08lX\n\r", nUsbCrc);
      strcat(cUsbBuffer, sCrc);


      uint8_t nRet = USBD_CDC_Transmit((uint8_t*)cUsbBuffer, sizeof(cUsbBuffer));

      if(nRet == USBD_FAIL)
        printf("USB TX Fail\n");

      if(nRet == USBD_BUSY)
        printf("USB TX Busy\n");

      memset(cUsbBuffer,0,120);
      */

#ifdef MEAS_HEAP_USE
    __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif

    osDelay(MAIN_TASK_DELAY);

    //Debug GPIO
    //EXTRA3_GPIO_Port->ODR ^= EXTRA3_Pin;
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
    if(eDevMode == DEVICE_AUTO){
      pf[i].eReqState = (ProfetStateTypeDef)(*stPdmConfig.stOutput[i].pInput && nStarterDisable[i] && nOutputFlasher[i]);
    }
    if(eDevMode == DEVICE_MANUAL){
      pf[i].eReqState = (ProfetStateTypeDef)nManualOutputs[i];
    }
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
  HAL_GPIO_WritePin(PF_RESET_Port, PF_RESET_Pin, GPIO_PIN_SET);
  //Set all outputs to push-pull
  PCA9539_WriteReg8(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_OUT_PORT_CONFIG, 0x00);
  //Set configuration registers (all to output)
  PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_CONFIG_PORT0, 0x0000);
  //Enable all pullup/pulldown
  PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_PU_PD_ENABLE_PORT0, 0x0000);
  //Set all outputs to pulldown
  PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_PU_PD_SELECT_PORT0, 0x0000);

  //=====================================================================================================
  // ADS1x15 Analog In Configuration
  //=====================================================================================================
  stAdcPfBank1.deviceType = ADS1015;
  stAdcPfBank1.bitShift = 0;
  stAdcPfBank1.gain = GAIN_ONE;
  stAdcPfBank1.dataRate = ADS1015_DATARATE_3300SPS;

  //=====================================================================================================
  // PCA9539 Profet GPIO Configuration
  //=====================================================================================================
  //Set all outputs to push-pull
  PCA9539_WriteReg8(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_OUT_PORT_CONFIG, 0x00);
  //Set configuration registers (all to output)
  PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_CONFIG_PORT0, 0x0000);
  //Enable all pullup/pulldown
  PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_PU_PD_ENABLE_PORT0, 0x0000);
  //Set all outputs to pulldown
  PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_PU_PD_SELECT_PORT0, 0x0000);

  //=====================================================================================================
  // ADS1x15 Analog In Configuration
  //=====================================================================================================
  stAdcPfBank2.deviceType = ADS1015;
  stAdcPfBank2.bitShift = 0;
  stAdcPfBank2.gain = GAIN_ONE;
  stAdcPfBank2.dataRate = ADS1015_DATARATE_3300SPS;

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
   //=====================================================================================================
   // PCAL9554B User Input
   //=====================================================================================================
   nUserDigInputRaw = PCAL9554B_ReadReg8(hi2c1, PCAL9554B_ADDRESS, PCAL9554B_CMD_IN_PORT);
   nUserDigInput[0] = !((nUserDigInputRaw & 0x08) >> 3);
   nUserDigInput[1] = !((nUserDigInputRaw & 0x04) >> 2);
   nUserDigInput[2] = !((nUserDigInputRaw & 0x02) >> 1);
   nUserDigInput[3] = !(nUserDigInputRaw & 0x01);
   nUserDigInput[4] = !((nUserDigInputRaw & 0x10) >> 4);
   nUserDigInput[5] = !((nUserDigInputRaw & 0x20) >> 5);
   nUserDigInput[6] = !((nUserDigInputRaw & 0x40) >> 6);
   nUserDigInput[7] = !((nUserDigInputRaw & 0x80) >> 7);

   //=====================================================================================================
   // Set Profet
   // DSEL to channel 1
   // Enable all DEN
   //=====================================================================================================
   pfGpioBank1 &= ~PF_BANK1_DSEL;
   pfGpioBank1 |= PF_BANK1_DEN;

   PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_OUT_PORT0, pfGpioBank1);

   //=====================================================================================================
   // ADS1x15 Analog Input
   //=====================================================================================================
   for(int i = 0; i < 4; i++){
     //Send channel register
     //Sets ADC multiplexer - must delay after for conversion
     ADS1x15_SendRegs(hi2c1, ADS1015_ADDRESS_PF_BANK1, &stAdcPfBank1, i);

     //Delay for conversion
     //860 SPS = 1.16ms per conversion - delay 2ms
     HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_SET);
     osDelay(ADS1015_CONVERSIONDELAY);
     HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_RESET);

     //Read channel value
     if(ADS1x15_ReadADC(hi2c1, ADS1015_ADDRESS_PF_BANK1, &stAdcPfBank1, &nPfISBank1Raw[i]) != HAL_OK)
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
   pfGpioBank1 |= PF_BANK1_DSEL;

   PCA9539_WriteReg16(hi2c1, PCA9539_ADDRESS_BANK1, PCA9539_CMD_OUT_PORT0, pfGpioBank1);

   for(int i = 0; i < 2; i++){
     //Send channel register
     //Sets ADC multiplexer - must delay after for conversion
     ADS1x15_SendRegs(hi2c1, ADS1015_ADDRESS_PF_BANK1, &stAdcPfBank1, i);

     //Delay for conversion
     //860 SPS = 1.16ms per conversion - delay 2ms
     HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_SET);
     osDelay(ADS1015_CONVERSIONDELAY);
     HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_RESET);

     //Read channel value
     if(ADS1x15_ReadADC(hi2c1, ADS1015_ADDRESS_PF_BANK1, &stAdcPfBank1, &nPfISBank1Raw[i]) != HAL_OK)
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
   pfGpioBank2 &= ~PF_BANK2_DSEL;
   pfGpioBank2 |= PF_BANK2_DEN;

   PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_OUT_PORT0, pfGpioBank2);

   //=====================================================================================================
   // ADS1115 Analog Input
   //=====================================================================================================
   for(int i = 0; i < 4; i++){
     //Send channel register
     //Sets ADC multiplexer - must delay after for conversion
     ADS1x15_SendRegs(hi2c2, ADS1015_ADDRESS_PF_BANK2, &stAdcPfBank2, i);

     //Delay for conversion
     //860 SPS = 1.16ms per conversion - delay 2ms
     HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_SET);
     osDelay(ADS1015_CONVERSIONDELAY);
     HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_RESET);

     //Read channel value
     if(ADS1x15_ReadADC(hi2c2, ADS1015_ADDRESS_PF_BANK2, &stAdcPfBank2, &nPfISBank2Raw[i]) != HAL_OK)
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
   pfGpioBank2 |= PF_BANK2_DSEL;

   PCA9539_WriteReg16(hi2c2, PCA9539_ADDRESS_BANK2, PCA9539_CMD_OUT_PORT0, pfGpioBank2);

   for(int i = 0; i < 2; i++){
     //Send channel register
     //Sets ADC multiplexer - must delay after for conversion
     ADS1x15_SendRegs(hi2c2, ADS1015_ADDRESS_PF_BANK2, &stAdcPfBank2, i+2);

     //Delay for conversion
     //860 SPS = 1.16ms per conversion - delay 2ms
     HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_SET);
     osDelay(ADS1015_CONVERSIONDELAY);
     HAL_GPIO_WritePin(EXTRA2_GPIO_Port, EXTRA2_Pin, GPIO_PIN_RESET);

     //Read channel value
     if(ADS1x15_ReadADC(hi2c2, ADS1015_ADDRESS_PF_BANK2, &stAdcPfBank2, &nPfISBank2Raw[i+2]) != HAL_OK)
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
     eStatusLeds[13] = bUsbConnected;   //USB
     eStatusLeds[14] = (HAL_GetTick() - nLastCanUpdate) < 1000;              //CAN
     eStatusLeds[15] = (eDevState == DEVICE_ERROR);   //Fault
     PCA9635_SetAll(hi2c2, PCA9635_ADDRESS, eStatusLeds);
   }

   //Debug GPIO
   HAL_GPIO_TogglePin(EXTRA1_GPIO_Port, EXTRA1_Pin);
   //EXTRA1_GPIO_Port->ODR ^= EXTRA1_Pin;

#ifdef MEAS_HEAP_USE
   __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif

   osDelay(I2C_TASK_DELAY);
 }
}


void ProfetSMTask(osThreadId_t* thisThreadId)
{
  Profet_Init();

  MsgQueueUsbTx_t stMsgUsbTx;
  MsgQueueCanTx_t stMsgCanTx;

  RTC_TimeTypeDef stTime = {0};
  RTC_DateTypeDef stDate = {0};

  uint8_t nSend;

  for(;;){
    for(int i=0; i<PDM_NUM_OUTPUTS; i++){
      Profet_SM(&pf[i]);
    }
    //WiperSM(&stWiper);
    MsgQueueRx_t stMsgRx;
    osStatus_t eStatus;

    nMsgCnt = osMessageQueueGetCount(qMsgQueueRx);
    /*
    if(nMsgCnt == 16)
      EXTRA2_GPIO_Port->ODR |= EXTRA2_Pin;
    else
      EXTRA2_GPIO_Port->ODR &= ~EXTRA2_Pin;
*/
    eStatus = osMessageQueueGet(qMsgQueueRx, &stMsgRx, NULL, 0U);
    if(eStatus == osOK){
      if(stMsgRx.eMsgSrc == CAN_RX){
        for(int i=0; i<PDM_NUM_CAN_INPUTS; i++){
          EvaluateCANInput(&stMsgRx.stCanRxHeader, stMsgRx.nRxData, &stPdmConfig.stCanInput[i], &nCanInputs[i]);
        }
      }
      if((stMsgRx.eMsgSrc == CAN_RX && stMsgRx.stCanRxHeader.StdId == stPdmConfig.stCanOutput.nBaseId + 21) || (stMsgRx.eMsgSrc == USB_RX)){
        //EXTRA2_GPIO_Port->ODR ^= EXTRA2_Pin;

        nSend = 0;

        switch((MsgQueueRxCmd_t)stMsgRx.nRxData[0]){

            //Burn Settings
            // 'B'
            case MSG_RX_BURN_SETTINGS:
              //Check special number sequence
              if(stMsgRx.nRxLen == 4){
                if((stMsgRx.nRxData[1] == 1) && (stMsgRx.nRxData[2] == 23) && (stMsgRx.nRxData[3] == 20)){
                  //Write settings to FRAM
                  //uint8_t nRet = PdmConfig_Write(hi2c2, MB85RC_ADDRESS, &stPdmConfig);
                  //TODO: Use flag to I2C task

                  stMsgUsbTx.nTxLen = 2;
                  stMsgCanTx.stTxHeader.DLC = 2;

                  stMsgUsbTx.nTxData[0] = MSG_TX_BURN_SETTINGS;
                  stMsgUsbTx.nTxData[1] = 0;// nRet;
                  stMsgUsbTx.nTxData[2] = 0;
                  stMsgUsbTx.nTxData[3] = 0;
                  stMsgUsbTx.nTxData[4] = 0;
                  stMsgUsbTx.nTxData[5] = 0;
                  stMsgUsbTx.nTxData[6] = 0;
                  stMsgUsbTx.nTxData[7] = 0;

                  stMsgCanTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 20;

                  memcpy(&stMsgCanTx.nTxData, &stMsgUsbTx.nTxData, sizeof(stMsgCanTx.nTxData));

                  osMessageQueuePut(qMsgQueueUsbTx, &stMsgUsbTx, 0U, 0U);
                  osMessageQueuePut(qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);
                }
              }
            break;

           //Set Mode
           // 'M'
           case MSG_RX_SET_MODE:
             if(stMsgRx.nRxLen == 2){
               switch(eDevMode){
               case DEVICE_AUTO:
                 if(stMsgRx.nRxData[1] & 0x01){ //Manual sent
                   for(int i=0; i<12; i++)
                     nManualOutputs[i] = 0;
                   eDevMode = DEVICE_MANUAL;
                 }
                 break;

               case DEVICE_MANUAL:
                 if(!(stMsgRx.nRxData[1] & 0x01)){ //Auto sent
                   eDevMode = DEVICE_AUTO;
                 }
                 break;
               }
               nSend = 1;
             }

             if((stMsgRx.nRxLen == 1) || (nSend)){
               stMsgUsbTx.nTxLen = 2;
               stMsgCanTx.stTxHeader.DLC = 2;

               stMsgUsbTx.nTxData[0] = MSG_TX_SET_MODE;
               stMsgUsbTx.nTxData[1] = (uint8_t)eDevMode;
               stMsgUsbTx.nTxData[2] = 0;
               stMsgUsbTx.nTxData[3] = 0;
               stMsgUsbTx.nTxData[4] = 0;
               stMsgUsbTx.nTxData[5] = 0;
               stMsgUsbTx.nTxData[6] = 0;
               stMsgUsbTx.nTxData[7] = 0;

               stMsgCanTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 20;

               memcpy(&stMsgCanTx.nTxData, &stMsgUsbTx.nTxData, sizeof(stMsgCanTx.nTxData));

               osMessageQueuePut(qMsgQueueUsbTx, &stMsgUsbTx, 0U, 0U);
               osMessageQueuePut(qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);
             }
           break;


           //Force Outputs
           // 'Q'
           case MSG_RX_FORCE_OUTPUTS:
             if(stMsgRx.nRxLen == 7){
               if(eDevMode == DEVICE_MANUAL){
                 nManualOutputs[0] = (stMsgRx.nRxData[1] & 0x01);
                 nManualOutputs[1] = (stMsgRx.nRxData[1] & 0x02) >> 1;
                 nManualOutputs[2] = (stMsgRx.nRxData[1] & 0x04) >> 2;
                 nManualOutputs[3] = (stMsgRx.nRxData[1] & 0x08) >> 3;
                 nManualOutputs[4] = (stMsgRx.nRxData[1] & 0x10) >> 4;
                 nManualOutputs[5] = (stMsgRx.nRxData[1] & 0x20) >> 5;
                 nManualOutputs[6] = (stMsgRx.nRxData[1] & 0x40) >> 6;
                 nManualOutputs[7] = (stMsgRx.nRxData[1] & 0x80) >> 7;
                 nManualOutputs[8] = (stMsgRx.nRxData[2] & 0x01);
                 nManualOutputs[9] = (stMsgRx.nRxData[2] & 0x02) >> 1;
                 nManualOutputs[10] = (stMsgRx.nRxData[2] & 0x04) >> 2;
                 nManualOutputs[11] = (stMsgRx.nRxData[2] & 0x08) >> 3;
                 nSend = 1;
               }
             }
             if((stMsgRx.nRxLen == 1) || (nSend)){
               stMsgUsbTx.nTxLen = 7;
               stMsgCanTx.stTxHeader.DLC = 7;

               stMsgUsbTx.nTxData[0] = MSG_TX_FORCE_OUTPUTS;
               stMsgUsbTx.nTxData[1] = ((nManualOutputs[7] & 0x01) << 7) + ((nManualOutputs[6] & 0x01) << 6) +
                                       ((nManualOutputs[5] & 0x01) << 5) + ((nManualOutputs[4] & 0x01) << 4) +
                                       ((nManualOutputs[3] & 0x01) << 3) + ((nManualOutputs[2] & 0x01) << 2) +
                                       ((nManualOutputs[1] & 0x01) << 1) + (nManualOutputs[0] & 0x01);
               stMsgUsbTx.nTxData[2] = ((nManualOutputs[11] & 0x01) << 3) + ((nManualOutputs[10] & 0x01) << 2) +
                                       ((nManualOutputs[9] & 0x01) << 1) + (nManualOutputs[8] & 0x01);

               //TODO:Add manual output modes
               stMsgUsbTx.nTxData[3] = 0;
               stMsgUsbTx.nTxData[4] = 0;
               stMsgUsbTx.nTxData[5] = 0;
               stMsgUsbTx.nTxData[6] = 0;
               stMsgUsbTx.nTxData[7] = 0;

               stMsgCanTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 20;

               memcpy(&stMsgCanTx.nTxData, &stMsgUsbTx.nTxData, sizeof(stMsgCanTx.nTxData));

               osMessageQueuePut(qMsgQueueUsbTx, &stMsgUsbTx, 0U, 0U);
               osMessageQueuePut(qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);
             }
           break;

           //Set Reporting
           // 'R'
           case MSG_RX_SET_REPORTING:
             if(stMsgRx.nRxLen == 3){
               nReportingOn = stMsgRx.nRxData[1] & 0x01;
               nReportingDelay = stMsgRx.nRxData[2] * 100;
               nSend = 1;
             }
             if((stMsgRx.nRxLen == 1) || (nSend)){
               stMsgUsbTx.nTxLen = 3;
               stMsgCanTx.stTxHeader.DLC = 3;

               stMsgUsbTx.nTxData[0] = MSG_TX_SET_REPORTING;
               stMsgUsbTx.nTxData[1] = (nReportingOn & 0x01);
               stMsgUsbTx.nTxData[2] = (uint8_t)(nReportingDelay / 100);
               stMsgUsbTx.nTxData[3] = 0;
               stMsgUsbTx.nTxData[4] = 0;
               stMsgUsbTx.nTxData[5] = 0;
               stMsgUsbTx.nTxData[6] = 0;
               stMsgUsbTx.nTxData[7] = 0;

               stMsgCanTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 20;

               memcpy(&stMsgCanTx.nTxData, &stMsgUsbTx.nTxData, sizeof(stMsgCanTx.nTxData));

               osMessageQueuePut(qMsgQueueUsbTx, &stMsgUsbTx, 0U, 0U);
               osMessageQueuePut(qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);
             }
           break;

           //Set Time
           // 'T'
           case MSG_RX_SET_TIME:
             if(stMsgRx.nRxLen == 7){
               stTime.Hours = stMsgRx.nRxData[1];
               stTime.Minutes = stMsgRx.nRxData[2];
               stTime.Seconds = stMsgRx.nRxData[3];
               stTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
               stTime.StoreOperation = RTC_STOREOPERATION_RESET;

               stDate.Year = stMsgRx.nRxData[4];
               stDate.Month = stMsgRx.nRxData[5];
               stDate.Date = stMsgRx.nRxData[6];
               stDate.WeekDay = RTC_WEEKDAY_MONDAY;

               //HAL_RTC_SetTime(hrtc, &stTime, RTC_FORMAT_BCD);
               //HAL_RTC_SetDate(hrtc, &stDate, RTC_FORMAT_BCD);
               //TODO: Use flag to Main task
               nSend = 1;
             }

             if((stMsgRx.nRxLen == 1) || nSend){
                 //HAL_RTC_GetTime(hrtc, &stTime, RTC_FORMAT_BCD);
                 //HAL_RTC_GetDate(hrtc, &stDate, RTC_FORMAT_BCD);
                 //TODO: Use flag to Main task

                 stMsgUsbTx.nTxLen = 7;
                 stMsgCanTx.stTxHeader.DLC = 7;

                 stMsgUsbTx.nTxData[0] = MSG_TX_SET_TIME;
                 stMsgUsbTx.nTxData[1] = stTime.Hours;
                 stMsgUsbTx.nTxData[2] = stTime.Minutes;
                 stMsgUsbTx.nTxData[3] = stTime.Seconds;
                 stMsgUsbTx.nTxData[4] = stDate.Year;
                 stMsgUsbTx.nTxData[5] = stDate.Month;
                 stMsgUsbTx.nTxData[6] = stDate.Date;
                 stMsgUsbTx.nTxData[7] = 0;

                 stMsgCanTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 20;

                 memcpy(&stMsgCanTx.nTxData, &stMsgUsbTx.nTxData, sizeof(stMsgCanTx.nTxData));

                 osMessageQueuePut(qMsgQueueUsbTx, &stMsgUsbTx, 0U, 0U);
                 osMessageQueuePut(qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);
             }


             break;

           //Get Temperature
           // 'F'
           case MSG_RX_GET_TEMP:
             if((stMsgRx.nRxLen == 1) || nSend){
                  stMsgUsbTx.nTxLen = 7;
                  stMsgCanTx.stTxHeader.DLC = 7;

                  stMsgUsbTx.nTxData[0] = MSG_TX_GET_TEMP;
                  stMsgUsbTx.nTxData[1] = nBoardTempC >> 8;
                  stMsgUsbTx.nTxData[2] = nBoardTempC;
                  stMsgUsbTx.nTxData[3] = nStmTemp >> 8;
                  stMsgUsbTx.nTxData[4] = nStmTemp;
                  stMsgUsbTx.nTxData[5] = 0;
                  stMsgUsbTx.nTxData[6] = 0;
                  stMsgUsbTx.nTxData[7] = 0;

                  stMsgCanTx.stTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 20;

                  memcpy(&stMsgCanTx.nTxData, &stMsgUsbTx.nTxData, sizeof(stMsgCanTx.nTxData));

                  osMessageQueuePut(qMsgQueueUsbTx, &stMsgUsbTx, 0U, 0U);
                  osMessageQueuePut(qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);
             }
             break;

           default:
             PdmConfig_Set(&stPdmConfig, &stMsgRx, &qMsgQueueUsbTx, &qMsgQueueCanTx);
             break;
        }
      }
    }

    MsgQueueUsbTx_t stMsgTx;
    if(osMessageQueueGet(qMsgQueueUsbTx, &stMsgTx, NULL, 0U) == osOK){
      if(bUsbConnected){
        //memcpy(&nUsbMsgTx, &stMsgTx.nTxData, stMsgTx.nTxLen);
        //nUsbMsgTx[stMsgTx.nTxLen] = '\r';
        if(USBD_CDC_Transmit((uint8_t*)stMsgTx.nTxData, stMsgTx.nTxLen) != USBD_OK){

          //TODO: bUsbConnected is physical connection - CAN RX commands get queued up and not dumped
          //Send failed - add back to queue
          //osMessageQueuePut(qMsgQueueUsbTx, &stMsgTx, 0U, 0U);
        }
      }
    }

#ifdef MEAS_HEAP_USE
    __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif

    osDelay(5);
    //Debug GPIO

    //EXTRA2_GPIO_Port->ODR ^= EXTRA2_Pin;
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
    if(stPdmConfig.stCanOutput.nEnabled &&
        (stPdmConfig.stCanOutput.nUpdateTime > 0) &&
        stPdmConfig.stCanOutput.nBaseId > 0 &&
        stPdmConfig.stCanOutput.nBaseId < 2048){

      MsgQueueCanTx_t stMsgTx;
      osStatus_t stStatus;
      //Keep sending queued messages until empty
      do{
        stStatus = osMessageQueueGet(qMsgQueueCanTx, &stMsgTx, NULL, 0U);
        if(stStatus == osOK){
          stMsgTx.stTxHeader.ExtId = 0;
          stMsgTx.stTxHeader.IDE = CAN_ID_STD;
          stMsgTx.stTxHeader.RTR = CAN_RTR_DATA;
          stMsgTx.stTxHeader.TransmitGlobalTime = DISABLE;

          if(HAL_CAN_AddTxMessage(hcan, &stMsgTx.stTxHeader, stMsgTx.nTxData, &nCanTxMailbox) != HAL_OK){
            //Send failed - add back to queue
            osMessageQueuePut(qMsgQueueCanTx, &stMsgTx, 0U, 0U);
          }
        }
      }while(stStatus == osOK);


      //=======================================================
      //Build Msg 0 (Analog inputs 1-4)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 0;
      stCanTxHeader.DLC = 8; //Bytes to send
      //nCanTxData[0] = nAiBank1Raw[0] >> 8;
      //nCanTxData[1] = nAiBank1Raw[0];
      //nCanTxData[2] = nAiBank1Raw[1] >> 8;
      //nCanTxData[3] = nAiBank1Raw[1];
      //nCanTxData[4] = nAiBank1Raw[2] >> 8;
      //nCanTxData[5] = nAiBank1Raw[2];
      //nCanTxData[6] = nAiBank1Raw[3] >> 8;
      //nCanTxData[7] = nAiBank1Raw[3];

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);

      //=======================================================
      //Build Msg 1 (Analog inputs 5-6)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 1;
      stCanTxHeader.DLC = 8; //Bytes to send
      //nCanTxData[0] = nAiBank2Raw[0] >> 8;
      //nCanTxData[1] = nAiBank2Raw[0];
      //nCanTxData[2] = nAiBank2Raw[1] >> 8;
      //nCanTxData[3] = nAiBank2Raw[1];
      //nCanTxData[4] = 0;
      //nCanTxData[5] = 0;
      //nCanTxData[6] = 0;
      //nCanTxData[7] = 0;

      //=======================================================
      //Send CAN msg
      //=======================================================
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);

      //=======================================================
      //Build Msg 2 (Device status)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 2;
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

      osDelay(CAN_TX_MSG_SPLIT);

      //=======================================================
      //Build Msg 3 (Out 1-4 Current)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 3;
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

      osDelay(CAN_TX_MSG_SPLIT);

      //=======================================================
      //Build Msg 4 (Out 5-8 Current)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 4;
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

      osDelay(CAN_TX_MSG_SPLIT);

      //=======================================================
      //Build Msg 5 (Out 9-12 Current)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 5;
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

      osDelay(CAN_TX_MSG_SPLIT);

      //=======================================================
      //Build Msg 6 (Out 1-12 Status)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 6;
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

#ifdef MEAS_HEAP_USE
      __attribute__((unused)) uint32_t nThisThreadSpace = osThreadGetStackSpace(*thisThreadId);
#endif

      osDelay(stPdmConfig.stCanOutput.nUpdateTime);
    }
    else{
      osDelay(50);
    }

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

void Profet_Init(){

  pf[0].eModel = BTS7002_1EPP;
  pf[0].nNum = 0;
  pf[0].nIN_Port = &pfGpioBank1;
  pf[0].nIN_Pin = 0x0080;
  pf[0].fKilis = 2.286;

  pf[1].eModel = BTS7002_1EPP;
  pf[1].nNum = 1;
  pf[1].nIN_Port = &pfGpioBank1;
  pf[1].nIN_Pin = 0x0002;
  pf[1].fKilis = 2.286;

  pf[2].eModel = BTS7008_2EPA_CH1;
  pf[2].nNum = 2;
  pf[2].nIN_Port = &pfGpioBank1;
  pf[2].nIN_Pin = 0x8000;
  pf[2].fKilis = 0.554;

  pf[3].eModel = BTS7008_2EPA_CH2;
  pf[3].eState = OFF;
  pf[3].nNum = 3;
  pf[3].nIN_Port = &pfGpioBank1;
  pf[3].nIN_Pin = 0x1000;
  pf[3].fKilis = 0.554;

  pf[4].eModel = BTS7008_2EPA_CH1;
  pf[4].eState = OFF;
  pf[4].nNum = 4;
  pf[4].nIN_Port = &pfGpioBank1;
  pf[4].nIN_Pin = 0x0800;
  pf[4].fKilis = 0.554;

  pf[5].eModel = BTS7008_2EPA_CH2;
  pf[5].eState = OFF;
  pf[5].nNum = 5;
  pf[5].nIN_Port = &pfGpioBank1;
  pf[5].nIN_Pin = 0x0100;
  pf[5].fKilis = 0.554;

  pf[6].eModel = BTS7002_1EPP;
  pf[6].eState = OFF;
  pf[6].nNum = 6;
  pf[6].nIN_Port = &pfGpioBank2;
  pf[6].nIN_Pin = 0x0002;
  pf[6].fKilis = 2.286;

  pf[7].eModel = BTS7002_1EPP;
  pf[7].eState = OFF;
  pf[7].nNum = 7;
  pf[7].nIN_Port = &pfGpioBank2;
  pf[7].nIN_Pin = 0x0008;
  pf[7].fKilis = 2.286;

  pf[8].eModel = BTS7008_2EPA_CH1;
  pf[8].eState = OFF;
  pf[8].nNum = 8;
  pf[8].nIN_Port = &pfGpioBank2;
  pf[8].nIN_Pin = 0x0010;
  pf[8].fKilis = 0.554;

  pf[9].eModel = BTS7008_2EPA_CH2;
  pf[9].eState = OFF;
  pf[9].nNum = 9;
  pf[9].nIN_Port = &pfGpioBank2;
  pf[9].nIN_Pin = 0x0080;
  pf[9].fKilis = 0.554;

  pf[10].eModel = BTS7008_2EPA_CH1;
  pf[10].eState = OFF;
  pf[10].nNum = 10;
  pf[10].nIN_Port = &pfGpioBank2;
  pf[10].nIN_Pin = 0x0100;
  pf[10].fKilis = 0.554;

  pf[11].eModel = BTS7008_2EPA_CH2;
  pf[11].eState = OFF;
  pf[11].nNum = 11;
  pf[11].nIN_Port = &pfGpioBank2;
  pf[11].nIN_Pin = 0x0800;
  pf[11].fKilis = 0.554;
}


//Overwrite printf _write to send to ITM_SendChar
int _write(int file, char *ptr, int len){
  int i=0;
  for(i=0; i<len; i++){
    ITM_SendChar((*ptr++));
  }
  return len;
}

uint8_t ReadPdmConfig()
{
  PdmConfig_SetDefault(&stPdmConfig);

  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    pf[i].nIL_Limit = stPdmConfig.stOutput[i].nCurrentLimit;
    pf[i].nIL_InRush_Limit = stPdmConfig.stOutput[i].nInrushLimit;
    pf[i].nIL_InRush_Time = stPdmConfig.stOutput[i].nInrushTime;
    //pf[i]. = stPdmConfig.stOutput[i].eResetMode;
    //pf[i] = stPdmConfig.stOutput[i].nResetTime;
    pf[i].nOC_ResetLimit = stPdmConfig.stOutput[i].nResetLimit;

  }
  /*
  //PdmConfig_Write(hi2c2, MB85RC_ADDRESS, &stPdmConfig);

  if(PdmConfig_Read(hi2c2, MB85RC_ADDRESS, &stPdmConfig) == 0){
    PdmConfig_SetDefault(&stPdmConfig);
  }
  */

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
