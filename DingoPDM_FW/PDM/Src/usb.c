#include "usb.h"

uint8_t USBD_RxBuffer[USBD_RX_DATA_SIZE];
uint8_t USBD_TxBuffer[USBD_TX_DATA_SIZE];

USBD_HandleTypeDef hUSBD;

bool bUsbConnected = false;

static int8_t USBD_CDC_Init(void);
static int8_t USBD_CDC_DeInit(void);
static int8_t USBD_CDC_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t USBD_CDC_Receive(uint8_t* pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_Interface =
{
  USBD_CDC_Init,
  USBD_CDC_DeInit,
  USBD_CDC_Control,
  USBD_CDC_Receive
};

uint8_t nUsbMsgTx[9]; //Add \r to Usb Tx message
rcvCallback_t rcvCallback;

/*
* @brief  USB Initialization
* @param  nId: CAN Base ID for USB
* @retval USBD_OK if all operations are OK else USBD_FAIL
*/
uint8_t USB_Init(rcvCallback_t cb)
{
    rcvCallback = cb;

    /* Init Device Library, add supported class and start the library. */
    if (USBD_Init(&hUSBD, &FS_Desc, DEVICE_FS) != USBD_OK)
    {
        return USBD_FAIL;
    }
    if (USBD_RegisterClass(&hUSBD, &USBD_CDC) != USBD_OK)
    {
        return USBD_FAIL;
    }
    if (USBD_CDC_RegisterInterface(&hUSBD, &USBD_Interface) != USBD_OK)
    {
        return USBD_FAIL;
    }
    if (USBD_Start(&hUSBD) != USBD_OK)
    {
        return USBD_FAIL;
    }

    return USBD_OK;
}

/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_Init(void)
{
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUSBD, USBD_TxBuffer, 0);
  USBD_CDC_SetRxBuffer(&hUSBD, USBD_RxBuffer);

  bUsbConnected = true;
  return (USBD_OK);
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USBD_CDC_DeInit(void)
{
  bUsbConnected = false;
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
  rcvCallback(Buf, Len);

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
uint8_t USB_Tx(uint8_t* Buf, uint16_t Len)
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

/**
 * @brief  Transmit a CAN message over USB in SLCAN Format
 * 
 * @param  pHeader: CAN Tx Header
 * @param  aData: CAN Data
 * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
*/
uint8_t USB_Tx_SLCAN(CAN_TxHeaderTypeDef *pHeader, uint8_t aData[])
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

	return USB_Tx(nUsbData, sizeof(nUsbData));
}

bool USB_IsConnected()
{
  return bUsbConnected;
}