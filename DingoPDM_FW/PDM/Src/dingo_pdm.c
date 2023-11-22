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

//==============================================================================================================================================
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
// USB
//========================================================================
char cUsbBuffer[120];
bool bUsbConnected = false;
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
CANInput_Rx_t stCanInputsRx[PDM_NUM_CAN_INPUTS];
uint16_t nVirtInputs[PDM_NUM_VIRT_INPUTS];
uint16_t nOutputs[PDM_NUM_OUTPUTS];
uint16_t nStarterDisable[PDM_NUM_OUTPUTS];
uint16_t nOutputFlasher[PDM_NUM_OUTPUTS];
uint16_t nAlwaysTrue;

uint32_t nMsgCnt;


void InputLogic();
void OutputLogic();
void Profet_Default_Init();

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

uint8_t USBD_CDC_Transmit_SLCAN(CAN_TxHeaderTypeDef *pHeader, uint8_t aData[])
{
	uint8_t nUsbData[22];
    nUsbData[0] = 't';
	nUsbData[1] = (pHeader->StdId >> 8) & 0xF;
	nUsbData[2] = (pHeader->StdId >> 4) & 0xF;
	nUsbData[3] = pHeader->StdId & 0xF;
	nUsbData[4] = (pHeader->DLC & 0xFF);
	nUsbData[5] = (aData[0] >> 4);
	nUsbData[6] = (aData[0] & 0x0F);
	nUsbData[7] = (aData[1] >> 4);
	nUsbData[8] = (aData[1] & 0x0F);
	nUsbData[9] = (aData[2] >> 4);
	nUsbData[10] = (aData[2] & 0x0F);
	nUsbData[11] = (aData[3] >> 4);
	nUsbData[12] = (aData[3] & 0x0F);
	nUsbData[13] = (aData[4] >> 4);
	nUsbData[14] = (aData[4] & 0x0F);
	nUsbData[15] = (aData[5] >> 4);
	nUsbData[16] = (aData[5] & 0x0F);
	nUsbData[17] = (aData[6] >> 4);
	nUsbData[18] = (aData[6] & 0x0F);
	nUsbData[19] = (aData[7] >> 4);
	nUsbData[20] = (aData[7] & 0x0F);
	nUsbData[21] = '\r';

	//Shift the data to ASCII, except the first 't'
	for(uint8_t j = 1; j <= 20; j++){
		if(nUsbData[j] < 0xA){
		  //Less than 0xA is a number
		  //Shift up to ASCII numbers
			nUsbData[j] += 0x30;
		}
		else{
			nUsbData[j] += 0x37;
		}
	}

	return USBD_CDC_Transmit(nUsbData, sizeof(nUsbData));
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
void PdmMainTask(osThreadId_t* thisThreadId, ADC_HandleTypeDef* hadc1, I2C_HandleTypeDef* hi2c1){
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
      stCanInputsRx[i].bRxOk = (HAL_GetTick() - stCanInputsRx[i].nLastRxTime) > stCanInputsRx[i].nRxMaxTime;
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

          USBD_CDC_Transmit_SLCAN(&stMsgCanTx.stTxHeader, stMsgCanTx.nTxData);
          if(HAL_CAN_AddTxMessage(hcan, &stMsgCanTx.stTxHeader, stMsgCanTx.nTxData, &nCanTxMailbox) != HAL_OK){
            //Send failed - add back to queue
            osMessageQueuePut(qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);
          }
        }
        //Pause for preemption - TX is not that important
        osDelay(CAN_TX_MSG_SPLIT);
      }while(stStatus == osOK);


      //=======================================================
      //Build Msg 0 (Digital inputs 1-2) and Device Status
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 0;
      stCanTxHeader.DLC = 8; //Bytes to send
      nCanTxData[0] = (nPdmInputs[1] << 1) + nPdmInputs[0];
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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 1 ( )
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 1;
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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

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
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

      //=======================================================
      //Build Msg 10 (Flashers 1-4)
      //=======================================================
      stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 10;
      stCanTxHeader.DLC = 8; //Bytes to send
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

      //=======================================================
      //Send CAN msg
      //=======================================================
#ifdef SEND_ALL_USB
      USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
      if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
        Error_Handler();
      }

      osDelay(CAN_TX_MSG_SPLIT);
      //for(int p=0; p<600; p++)

      if(true){//(bSoftwareConnected){
          //=======================================================
          //Build Msg 11 (CAN Inputs 1-8)
          //=======================================================
          stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 11;
          stCanTxHeader.DLC = 8; //Bytes to send
          nCanTxData[0] = nCanInputs[0];
          nCanTxData[1] = nCanInputs[1];
          nCanTxData[2] = nCanInputs[2];
          nCanTxData[3] = nCanInputs[3];
          nCanTxData[4] = nCanInputs[4];
          nCanTxData[5] = nCanInputs[5];
          nCanTxData[6] = nCanInputs[6];
          nCanTxData[7] = nCanInputs[7];

          //=======================================================
          //Send CAN msg
          //=======================================================
#ifdef SEND_ALL_USB
          USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
          if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
            Error_Handler();
          }

          osDelay(CAN_TX_MSG_SPLIT);
          //for(int p=0; p<600; p++)

          //=======================================================
          //Build Msg 12 (CAN Inputs 9-16)
          //=======================================================
          stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 12;
          stCanTxHeader.DLC = 8; //Bytes to send
          nCanTxData[0] = nCanInputs[8];
          nCanTxData[1] = nCanInputs[9];
          nCanTxData[2] = nCanInputs[10];
          nCanTxData[3] = nCanInputs[11];
          nCanTxData[4] = nCanInputs[12];
          nCanTxData[5] = nCanInputs[13];
          nCanTxData[6] = nCanInputs[14];
          nCanTxData[7] = nCanInputs[15];

          //=======================================================
          //Send CAN msg
          //=======================================================
#ifdef SEND_ALL_USB
          USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
          if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
            Error_Handler();
          }

          osDelay(CAN_TX_MSG_SPLIT);
          //for(int p=0; p<600; p++)

          //=======================================================
          //Build Msg 13 (CAN Inputs 17-24)
          //=======================================================
          stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 13;
          stCanTxHeader.DLC = 8; //Bytes to send
          nCanTxData[0] = nCanInputs[16];
          nCanTxData[1] = nCanInputs[17];
          nCanTxData[2] = nCanInputs[18];
          nCanTxData[3] = nCanInputs[19];
          nCanTxData[4] = nCanInputs[20];
          nCanTxData[5] = nCanInputs[21];
          nCanTxData[6] = nCanInputs[22];
          nCanTxData[7] = nCanInputs[23];

          //=======================================================
          //Send CAN msg
          //=======================================================
#ifdef SEND_ALL_USB
          USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
          if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
            Error_Handler();
          }

          osDelay(CAN_TX_MSG_SPLIT);
          //for(int p=0; p<600; p++)

          //=======================================================
          //Build Msg 14 (CAN Inputs 25-32)
          //=======================================================
          stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 14;
          stCanTxHeader.DLC = 8; //Bytes to send
          nCanTxData[0] = nCanInputs[24];
          nCanTxData[1] = nCanInputs[25];
          nCanTxData[2] = nCanInputs[26];
          nCanTxData[3] = nCanInputs[27];
          nCanTxData[4] = nCanInputs[28];
          nCanTxData[5] = nCanInputs[29];
          nCanTxData[6] = nCanInputs[30];
          nCanTxData[7] = nCanInputs[31];

          //=======================================================
          //Send CAN msg
          //=======================================================
#ifdef SEND_ALL_USB
          USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
          if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
            Error_Handler();
          }

          osDelay(CAN_TX_MSG_SPLIT);
          //for(int p=0; p<600; p++)

          //=======================================================
          //Build Msg 15 (Virtual Inputs 1-8)
          //=======================================================
          stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 15;
          stCanTxHeader.DLC = 8; //Bytes to send
          nCanTxData[0] = nVirtInputs[0];
          nCanTxData[1] = nVirtInputs[1];
          nCanTxData[2] = nVirtInputs[2];
          nCanTxData[3] = nVirtInputs[3];
          nCanTxData[4] = nVirtInputs[4];
          nCanTxData[5] = nVirtInputs[5];
          nCanTxData[6] = nVirtInputs[6];
          nCanTxData[7] = nVirtInputs[7];

          //=======================================================
          //Send CAN msg
          //=======================================================
#ifdef SEND_ALL_USB
          USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
          if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
            Error_Handler();
          }

          osDelay(CAN_TX_MSG_SPLIT);
          //for(int p=0; p<600; p++)

          //=======================================================
          //Build Msg 16 (Virtual Inputs 9-16)
          //=======================================================
          stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 16;
          stCanTxHeader.DLC = 8; //Bytes to send
          nCanTxData[0] = nVirtInputs[8];
          nCanTxData[1] = nVirtInputs[9];
          nCanTxData[2] = nVirtInputs[10];
          nCanTxData[3] = nVirtInputs[11];
          nCanTxData[4] = nVirtInputs[12];
          nCanTxData[5] = nVirtInputs[13];
          nCanTxData[6] = nVirtInputs[14];
          nCanTxData[7] = nVirtInputs[15];

          //=======================================================
          //Send CAN msg
          //=======================================================
#ifdef SEND_ALL_USB
          USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
          if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
            Error_Handler();
          }

          osDelay(CAN_TX_MSG_SPLIT);
          //for(int p=0; p<600; p++)

          //=======================================================
          //Build Msg 17 (Wiper)
          //=======================================================
          stCanTxHeader.StdId = stPdmConfig.stCanOutput.nBaseId + 17;
          stCanTxHeader.DLC = 8; //Bytes to send
          nCanTxData[0] = (*pVariableMap[60] << 1) + *pVariableMap[59];
          nCanTxData[1] = stWiper.eState;
          nCanTxData[2] = stWiper.eSelectedSpeed;
          nCanTxData[3] = 0;
          nCanTxData[4] = 0;
          nCanTxData[5] = 0;
          nCanTxData[6] = 0;
          nCanTxData[7] = 0;

          //=======================================================
          //Send CAN msg
          //=======================================================
#ifdef SEND_ALL_USB
          USBD_CDC_Transmit_SLCAN(&stCanTxHeader, nCanTxData);
#endif
          if(HAL_CAN_AddTxMessage(hcan, &stCanTxHeader, nCanTxData, &nCanTxMailbox) != HAL_OK){
            Error_Handler();
          }
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
    EvaluateVirtInput(&stPdmConfig.stVirtualInput[i], &nVirtInputs[i], stCanInputsRx);

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

void OutputLogic(){
  bool bCanInOk = false;

  //Copy output logic to profet requested state
  for(int i=0; i<PDM_NUM_OUTPUTS; i++)
  {
    //Output using CANInput
    if((stPdmConfig.stOutput[i].nInput >= 3) && (stPdmConfig.stOutput[i].nInput <= 34)){
      bCanInOk = stCanInputsRx[stPdmConfig.stOutput[i].nInput - 3].bRxOk;
    }
    else{
      bCanInOk = true;
    }

    pf[i].eReqState = (ProfetStateTypeDef)(*stPdmConfig.stOutput[i].pInput && nStarterDisable[i] && nOutputFlasher[i] && bCanInOk);
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

//******************************************************
//Must be called before any tasks are started
//******************************************************
uint8_t InitPdmConfig(I2C_HandleTypeDef* hi2c1)
{
  //PdmConfig_SetDefault(&stPdmConfig);
  //PdmConfig_Write(hi2c1, MB85RC_ADDRESS, &stPdmConfig);

   //Check that the data is correct, comms are OK, and FRAM device is the right ID
  if(PdmConfig_Check(hi2c1, MB85RC_ADDRESS, &stPdmConfig) == PDM_OK)
  {
    if(PdmConfig_Read(hi2c1, MB85RC_ADDRESS, &stPdmConfig) != PDM_OK)
    {
      PdmConfig_SetDefault(&stPdmConfig);
    }
  }
  else
  {
    PdmConfig_SetDefault(&stPdmConfig);
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
  //User inputs
  for(int i=0; i<PDM_NUM_INPUTS; i++)
  {
    nPdmInputs[i] = 0;
    pVariableMap[i+1] = &nPdmInputs[i];
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
