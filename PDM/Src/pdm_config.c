#include "pdm_config.h"
/*
static MsgQueueUsbTx_t stMsgUsbTx;
static MsgQueueCanTx_t stMsgCanTx;
static uint8_t nSend;

static uint8_t nInNum;
static uint8_t nOutNum;
static uint8_t nVirtInNum;
static uint8_t nFlasherNum;
static uint8_t nCanInputNum;
*/
/*
uint8_t PdmConfig_Check(I2C_HandleTypeDef* hi2c, uint8_t nAddr, PdmConfig_t* pConfig)
{
  //Verifty that FRAM is communicating
  if(MB85RC_CheckId(hi2c, nAddr) != HAL_OK){
      return 0;
  }

  uint16_t nSizeOfConfig = sizeof(*pConfig);
  uint16_t nSizeInMem = 0;

  //Get size from 2 bytes after config
  if(!(MB85RC_Read(hi2c, nAddr, sizeof(*pConfig), (uint8_t*)&nSizeInMem, sizeof(nSizeInMem)) == HAL_OK))
  {
    return 0;
  }

  //Check that the value stored matches the config size
  if(nSizeInMem == nSizeOfConfig) return 1;

  return 0;
}

uint8_t PdmConfig_Read(I2C_HandleTypeDef* hi2c, uint8_t nAddr, PdmConfig_t* pConfig)
{
  //Verifty that FRAM is communicating
  if(MB85RC_CheckId(hi2c, nAddr) != HAL_OK){
      return 0;
  }

  //Takes approx. 60ms to read entire struct
  if(!MB85RC_Read(hi2c, nAddr, 0x0, (uint8_t*)pConfig, sizeof(*pConfig)) == HAL_OK)
  {
    return 0;
  }

  return 1;
}
*/
/*
uint8_t PdmConfig_Write(I2C_HandleTypeDef* hi2c, uint8_t nAddr, PdmConfig_t* pConfig)
{
  //Verifty that FRAM is communicating
  if(MB85RC_CheckId(hi2c, nAddr) != HAL_OK){
      return 0;
  }

  if(!MB85RC_Write(hi2c, nAddr, 0x0, (uint8_t*)pConfig, sizeof(*pConfig)) == HAL_OK)
  {
    return 0;
  }

  uint16_t nSizeOfConfig = sizeof(*pConfig);

  if(!MB85RC_Write(hi2c, nAddr, sizeof(*pConfig), (uint8_t*)&nSizeOfConfig, sizeof(nSizeOfConfig)) == HAL_OK)
  {
    return 0;
  }

  return 1;
}
*/

/*

uint8_t PdmConfig_Set(PdmConfig_t* pConfig, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueCanTx)
{

  nSend = 0;

  switch((MsgQueueRxCmd_t)stMsgRx->nRxData[0]){

    //Set CAN Settings
    // 'C'
    case MSG_RX_SET_CAN:
      if(stMsgRx->nRxLen == 5){

        pConfig->stDevConfig.nCanEnabled = stMsgRx->nRxData[1] & 0x01;
        pConfig->stCanOutput.nEnabled = (stMsgRx->nRxData[1] & 0x02) >> 1;
        pConfig->stDevConfig.nCanTerm = (stMsgRx->nRxData[1] & 0x04) >> 2;
        pConfig->stDevConfig.nCanSpeed = (stMsgRx->nRxData[1] & 0xF0) >> 4;

        pConfig->stCanOutput.nBaseId = (stMsgRx->nRxData[2] << 8) + stMsgRx->nRxData[3];
        pConfig->stCanOutput.nUpdateTime = stMsgRx->nRxData[4] * 100;
        nSend = 1;
      }

      if((stMsgRx->nRxLen == 1) || (nSend)){
        stMsgCanTx.stTxHeader.DLC = 5;

        stMsgCanTx.nTxData[0] = MSG_TX_SET_CAN;
        stMsgCanTx.nTxData[1] = ((pConfig->stDevConfig.nCanSpeed & 0x0F) << 4) + ((pConfig->stDevConfig.nCanTerm & 0x01) << 2) + ((pConfig->stCanOutput.nEnabled & 0x01) << 1) + (pConfig->stDevConfig.nCanEnabled & 0x01);
        stMsgCanTx.nTxData[2] = (uint8_t)((pConfig->stCanOutput.nBaseId & 0xFF00) >> 8);
        stMsgCanTx.nTxData[3] = (uint8_t)(pConfig->stCanOutput.nBaseId & 0x00FF);
        stMsgCanTx.nTxData[4] = (uint8_t)((pConfig->stCanOutput.nUpdateTime) / 100);
        stMsgCanTx.nTxData[5] = 0;
        stMsgCanTx.nTxData[6] = 0;
        stMsgCanTx.nTxData[7] = 0;
        nSend = 1;
      }
    break;

    //Set Logging
    // 'L'
    case MSG_RX_SET_LOGGING:
      if(stMsgRx->nRxLen == 3){
        //TODO:Send response
      }
      if((stMsgRx->nRxLen == 1) || (nSend)){

      }
    break;

    //Set Input Settings
    // 'I'
    case MSG_RX_SET_INPUTS:
      if(stMsgRx->nRxLen == 3){
        nInNum = (stMsgRx->nRxData[1] & 0xF0) >> 4;
        if(nInNum < PDM_NUM_INPUTS){
          pConfig->stInput[nInNum].nEnabled = (stMsgRx->nRxData[1] & 0x01);
          pConfig->stInput[nInNum].eMode = (stMsgRx->nRxData[1] & 0x06) >> 1;
          //TODO:Include binary on level on V3 PCB
          //pConfig->stInput[nInNum].nOnLevel = (stMsgRx->nRxData[1] & 0x08) >> 3;
          pConfig->stInput[nInNum].nDebounceTime = stMsgRx->nRxData[2] * 100;
          nSend = 1;
        }
      }

      if(stMsgRx->nRxLen == 2){
        nInNum = (stMsgRx->nRxData[1] & 0xF0) >> 4;
        if(nInNum < PDM_NUM_INPUTS){
          nSend = 1;
        }
      }

      if(nSend){
        stMsgCanTx.stTxHeader.DLC = 3;

        stMsgCanTx.nTxData[0] = MSG_TX_SET_INPUTS;
        //TODO:Add binary On Level on V3 PCB
        stMsgCanTx.nTxData[1] = ((nInNum & 0x0F) << 4) + ((pConfig->stInput[nInNum].eMode & 0x03) << 2) + (pConfig->stInput[nInNum].nEnabled & 0x01);
        stMsgCanTx.nTxData[2] = (uint8_t)(pConfig->stInput[nInNum].nDebounceTime / 100);
        stMsgCanTx.nTxData[3] = 0;
        stMsgCanTx.nTxData[4] = 0;
        stMsgCanTx.nTxData[5] = 0;
        stMsgCanTx.nTxData[6] = 0;
        stMsgCanTx.nTxData[7] = 0;
      }

    break;

    //Set Output Settings
    // 'O'
    case MSG_RX_SET_OUTPUTS:
      if(stMsgRx->nRxLen == 8){
        nOutNum = (stMsgRx->nRxData[1] & 0xF0) >> 4;
        if(nOutNum < PDM_NUM_OUTPUTS){
          pConfig->stOutput[nOutNum].nEnabled = (stMsgRx->nRxData[1] & 0x01);
          pConfig->stOutput[nOutNum].nInput = stMsgRx->nRxData[2];
          pConfig->stOutput[nOutNum].nCurrentLimit = stMsgRx->nRxData[3] / 10;
          pConfig->stOutput[nOutNum].eResetMode = (stMsgRx->nRxData[4] & 0x0F);
          pConfig->stOutput[nOutNum].nResetLimit = (stMsgRx->nRxData[4] & 0xF0) >> 4;
          pConfig->stOutput[nOutNum].nResetTime = stMsgRx->nRxData[5] * 100;
          pConfig->stOutput[nOutNum].nInrushLimit = stMsgRx->nRxData[6] / 10;
          pConfig->stOutput[nOutNum].nInrushTime = stMsgRx->nRxData[7] * 100;
          nSend = 1;
        }
      }

      if(stMsgRx->nRxLen == 2){
        nOutNum = (stMsgRx->nRxData[1] & 0xF0) >> 4;
        if(nOutNum < PDM_NUM_OUTPUTS){
          nSend = 1;
        }
      }

      if(nSend){
        stMsgCanTx.stTxHeader.DLC = 8;

        stMsgCanTx.nTxData[0] = MSG_TX_SET_OUTPUTS;
        stMsgCanTx.nTxData[1] = ((nOutNum & 0x0F) << 4) + (pConfig->stOutput[nOutNum].nEnabled & 0x01);
        stMsgCanTx.nTxData[2] = pConfig->stOutput[nOutNum].nInput;
        stMsgCanTx.nTxData[3] = (uint8_t)(pConfig->stOutput[nOutNum].nCurrentLimit * 10);
        stMsgCanTx.nTxData[4] = ((pConfig->stOutput[nOutNum].nResetLimit & 0x0F) << 4) + (pConfig->stOutput[nOutNum].eResetMode & 0x0F);
        stMsgCanTx.nTxData[5] = (uint8_t)(pConfig->stOutput[nOutNum].nResetTime / 100);
        stMsgCanTx.nTxData[6] = (uint8_t)(pConfig->stOutput[nOutNum].nInrushLimit * 10);
        stMsgCanTx.nTxData[7] = (uint8_t)(pConfig->stOutput[nOutNum].nInrushTime / 100);
      }
    break;

    //Set Virtual Input Settings
    // 'U'
    case MSG_RX_SET_VIRTUAL_INPUTS:
      if(stMsgRx->nRxLen == 7){
        nVirtInNum = (stMsgRx->nRxData[2]);
        if(nVirtInNum < PDM_NUM_VIRT_INPUTS){
          pConfig->stVirtualInput[nVirtInNum].nEnabled = (stMsgRx->nRxData[1] & 0x01);
          pConfig->stVirtualInput[nVirtInNum].nNot0 = (stMsgRx->nRxData[1] & 0x02) >> 1;
          pConfig->stVirtualInput[nVirtInNum].nNot1 = (stMsgRx->nRxData[1] & 0x04) >> 2;
          pConfig->stVirtualInput[nVirtInNum].nNot2 = (stMsgRx->nRxData[1] & 0x08) >> 3;

          pConfig->stVirtualInput[nVirtInNum].nVar0 = stMsgRx->nRxData[3];
          pConfig->stVirtualInput[nVirtInNum].nVar1 = stMsgRx->nRxData[4];
          pConfig->stVirtualInput[nVirtInNum].nVar2 = stMsgRx->nRxData[5];

          pConfig->stVirtualInput[nVirtInNum].eCond0 = (stMsgRx->nRxData[6] & 0x03);
          pConfig->stVirtualInput[nVirtInNum].eCond1 = (stMsgRx->nRxData[6] & 0x0C) >> 2;
          pConfig->stVirtualInput[nVirtInNum].eMode = (stMsgRx->nRxData[6] & 0xC0) >> 6;
          nSend = 1;
        }
      }

      if(stMsgRx->nRxLen == 2){
        nVirtInNum = (stMsgRx->nRxData[1]);
        if(nVirtInNum < PDM_NUM_VIRT_INPUTS){
          nSend = 1;
        }
      }

      if(nSend){
        stMsgCanTx.stTxHeader.DLC = 7;

        stMsgCanTx.nTxData[0] = MSG_TX_SET_VIRTUAL_INPUTS;
        stMsgCanTx.nTxData[1] = ((pConfig->stVirtualInput[nVirtInNum].nNot2 & 0x01) << 3) + ((pConfig->stVirtualInput[nVirtInNum].nNot1 & 0x01) << 2) +
                                ((pConfig->stVirtualInput[nVirtInNum].nNot0 & 0x01) << 1) + (pConfig->stVirtualInput[nVirtInNum].nEnabled & 0x01);
        stMsgCanTx.nTxData[2] = nVirtInNum;
        stMsgCanTx.nTxData[3] = pConfig->stVirtualInput[nVirtInNum].nVar0;
        stMsgCanTx.nTxData[4] = pConfig->stVirtualInput[nVirtInNum].nVar1;
        stMsgCanTx.nTxData[5] = pConfig->stVirtualInput[nVirtInNum].nVar2;
        stMsgCanTx.nTxData[6] = ((pConfig->stVirtualInput[nVirtInNum].eMode & 0x0F) << 4) + ((pConfig->stVirtualInput[nVirtInNum].eCond0 & 0x03) << 2) +
                                (pConfig->stVirtualInput[nVirtInNum].eCond1 & 0x03);
        stMsgCanTx.nTxData[7] = 0;
      }
    break;

    //Set Wiper Settings
    // 'W'
    case MSG_RX_SET_WIPER:
      if(stMsgRx->nRxLen == 8){
        pConfig->stWiper.nEnabled = (stMsgRx->nRxData[1] & 0x01);
        pConfig->stWiper.nMode = (stMsgRx->nRxData[1] & 0x06) >> 1;
        pConfig->stWiper.nParkStopLevel = (stMsgRx->nRxData[1] & 0x08) >> 3;
        pConfig->stWiper.nWashWipeCycles = (stMsgRx->nRxData[1] * 0xF0) >> 4;

        pConfig->stWiper.nSlowInput = stMsgRx->nRxData[2];

        pConfig->stWiper.nFastInput = stMsgRx->nRxData[3];

        pConfig->stWiper.nInterInput = stMsgRx->nRxData[4];

        pConfig->stWiper.nOnInput = stMsgRx->nRxData[5];

        pConfig->stWiper.nParkInput = stMsgRx->nRxData[6];

        pConfig->stWiper.nWashInput = stMsgRx->nRxData[7];
        nSend = 1;
      }
      if((stMsgRx->nRxLen == 1) || nSend){
        stMsgCanTx.stTxHeader.DLC = 8;

        stMsgCanTx.nTxData[0] = MSG_TX_SET_WIPER;
        stMsgCanTx.nTxData[1] = ((pConfig->stWiper.nWashWipeCycles & 0x0F) << 4) + ((pConfig->stWiper.nParkStopLevel & 0x01) << 3) +
                                ((pConfig->stWiper.nMode & 0x03) << 2) + (pConfig->stWiper.nEnabled & 0x01);
        stMsgCanTx.nTxData[2] = pConfig->stWiper.nSlowInput;
        stMsgCanTx.nTxData[3] = pConfig->stWiper.nFastInput;
        stMsgCanTx.nTxData[4] = pConfig->stWiper.nInterInput;
        stMsgCanTx.nTxData[5] = pConfig->stWiper.nOnInput;
        stMsgCanTx.nTxData[6] = pConfig->stWiper.nParkInput;
        stMsgCanTx.nTxData[7] = pConfig->stWiper.nWashInput;
        nSend = 1;
      }
    break;

    //Set Wiper Speed Settings
    // 'P'
    case MSG_RX_SET_WIPER_SPEED:
      if(stMsgRx->nRxLen == 7){
        pConfig->stWiper.nSwipeInput = stMsgRx->nRxData[1];

        pConfig->stWiper.nSpeedInput = stMsgRx->nRxData[2];

        pConfig->stWiper.nSpeedMap[0] = (stMsgRx->nRxData[3] * 0x0F);
        pConfig->stWiper.nSpeedMap[1] = (stMsgRx->nRxData[3] * 0xF0) >> 4;

        pConfig->stWiper.nSpeedMap[2] = (stMsgRx->nRxData[4] * 0x0F);
        pConfig->stWiper.nSpeedMap[3] = (stMsgRx->nRxData[4] * 0xF0) >> 4;

        pConfig->stWiper.nSpeedMap[4] = (stMsgRx->nRxData[5] * 0x0F);
        pConfig->stWiper.nSpeedMap[5] = (stMsgRx->nRxData[5] * 0xF0) >> 4;

        pConfig->stWiper.nSpeedMap[6] = (stMsgRx->nRxData[6] * 0x0F);
        pConfig->stWiper.nSpeedMap[7] = (stMsgRx->nRxData[6] * 0xF0) >> 4;
        nSend = 1;
      }
      if((stMsgRx->nRxLen == 1) || nSend){
        stMsgCanTx.stTxHeader.DLC = 7;

        stMsgCanTx.nTxData[0] = MSG_TX_SET_WIPER_SPEED;
        stMsgCanTx.nTxData[1] = pConfig->stWiper.nSwipeInput;
        stMsgCanTx.nTxData[2] = pConfig->stWiper.nSpeedInput;
        stMsgCanTx.nTxData[3] = ((pConfig->stWiper.nSpeedMap[1] & 0x0F) << 4) + (pConfig->stWiper.nSpeedMap[0] & 0x0F);
        stMsgCanTx.nTxData[4] = ((pConfig->stWiper.nSpeedMap[3] & 0x0F) << 4) + (pConfig->stWiper.nSpeedMap[2] & 0x0F);
        stMsgCanTx.nTxData[5] = ((pConfig->stWiper.nSpeedMap[5] & 0x0F) << 4) + (pConfig->stWiper.nSpeedMap[4] & 0x0F);
        stMsgCanTx.nTxData[6] = ((pConfig->stWiper.nSpeedMap[7] & 0x0F) << 4) + (pConfig->stWiper.nSpeedMap[6] & 0x0F);
        stMsgCanTx.nTxData[7] = 0;
        nSend = 1;
      }
    break;

    //Set Wiper Intermit Delays Settings
    // 'Y'
    case MSG_RX_SET_WIPER_DELAYS:
      if(stMsgRx->nRxLen == 7){
        pConfig->stWiper.nIntermitTime[0] = stMsgRx->nRxData[1] * 100;
        pConfig->stWiper.nIntermitTime[1] = stMsgRx->nRxData[2] * 100;
        pConfig->stWiper.nIntermitTime[2] = stMsgRx->nRxData[3] * 100;
        pConfig->stWiper.nIntermitTime[3] = stMsgRx->nRxData[4] * 100;
        pConfig->stWiper.nIntermitTime[4] = stMsgRx->nRxData[5] * 100;
        pConfig->stWiper.nIntermitTime[5] = stMsgRx->nRxData[6] * 100;
        nSend = 1;
      }
      if((stMsgRx->nRxLen == 1) || nSend){
        stMsgCanTx.stTxHeader.DLC = 7;

        stMsgCanTx.nTxData[0] = MSG_TX_SET_WIPER_DELAYS;
        stMsgCanTx.nTxData[1] = (uint8_t)(pConfig->stWiper.nIntermitTime[0] / 100);
        stMsgCanTx.nTxData[2] = (uint8_t)(pConfig->stWiper.nIntermitTime[1] / 100);
        stMsgCanTx.nTxData[3] = (uint8_t)(pConfig->stWiper.nIntermitTime[2] / 100);
        stMsgCanTx.nTxData[4] = (uint8_t)(pConfig->stWiper.nIntermitTime[3] / 100);
        stMsgCanTx.nTxData[5] = (uint8_t)(pConfig->stWiper.nIntermitTime[4] / 100);
        stMsgCanTx.nTxData[6] = (uint8_t)(pConfig->stWiper.nIntermitTime[5] / 100);
        stMsgCanTx.nTxData[7] = 0;
        nSend = 1;
      }
    break;

    //Set Flasher Settings
    // 'H'
    case MSG_RX_SET_FLASHER:
      if(stMsgRx->nRxLen == 6){
        nFlasherNum = (stMsgRx->nRxData[1] & 0xF0) >> 4;
        if(nFlasherNum < PDM_NUM_FLASHERS){
          pConfig->stFlasher[nFlasherNum].nEnabled = (stMsgRx->nRxData[1] & 0x01);
          pConfig->stFlasher[nFlasherNum].nSingleCycle = (stMsgRx->nRxData[1] & 0x02) >> 1;

          pConfig->stFlasher[nFlasherNum].nInput = stMsgRx->nRxData[2];

          pConfig->stFlasher[nFlasherNum].nOutput = stMsgRx->nRxData[3];

          pConfig->stFlasher[nFlasherNum].nFlashOnTime = stMsgRx->nRxData[4] * 100;

          pConfig->stFlasher[nFlasherNum].nFlashOffTime = stMsgRx->nRxData[5] * 100;
          nSend = 1;
        }
      }

      if(stMsgRx->nRxLen == 2){
        nFlasherNum = (stMsgRx->nRxData[1] & 0xF0) >> 4;
        if(nFlasherNum < PDM_NUM_FLASHERS){
          nSend = 1;
        }
      }

      if(nSend){
        stMsgCanTx.stTxHeader.DLC = 6;

        stMsgCanTx.nTxData[0] = MSG_TX_SET_FLASHER;
        stMsgCanTx.nTxData[1] = ((nFlasherNum & 0x0F) << 4) + ((pConfig->stFlasher[nFlasherNum].nSingleCycle & 0x01) << 1) +
                                (pConfig->stFlasher[nFlasherNum].nEnabled & 0x01);
        stMsgCanTx.nTxData[2] = pConfig->stFlasher[nFlasherNum].nInput;
        stMsgCanTx.nTxData[3] = pConfig->stFlasher[nFlasherNum].nOutput;
        stMsgCanTx.nTxData[4] = (uint8_t)(pConfig->stFlasher[nFlasherNum].nFlashOnTime / 100);
        stMsgCanTx.nTxData[5] = (uint8_t)(pConfig->stFlasher[nFlasherNum].nFlashOffTime / 100);
        stMsgCanTx.nTxData[6] = 0;
        stMsgCanTx.nTxData[7] = 0;
      }
    break;

    //Set Starter Disable Settings
    // 'D'
    case MSG_RX_SET_STARTER:
      if(stMsgRx->nRxLen == 5){
        pConfig->stStarter.nEnabled = (stMsgRx->nRxData[1] & 0x01);

        pConfig->stStarter.nInput = stMsgRx->nRxData[2];

        pConfig->stStarter.nDisableOut[0] = (stMsgRx->nRxData[3] & 0x01);
        pConfig->stStarter.nDisableOut[1] = (stMsgRx->nRxData[3] & 0x02) >> 1;
        pConfig->stStarter.nDisableOut[2] = (stMsgRx->nRxData[3] & 0x04) >> 2;
        pConfig->stStarter.nDisableOut[3] = (stMsgRx->nRxData[3] & 0x08) >> 3;
        pConfig->stStarter.nDisableOut[4] = (stMsgRx->nRxData[3] & 0x10) >> 4;
        pConfig->stStarter.nDisableOut[5] = (stMsgRx->nRxData[3] & 0x20) >> 5;
        pConfig->stStarter.nDisableOut[6] = (stMsgRx->nRxData[3] & 0x40) >> 6;
        pConfig->stStarter.nDisableOut[7] = (stMsgRx->nRxData[3] & 0x80) >> 7;

        pConfig->stStarter.nDisableOut[8] = (stMsgRx->nRxData[4] & 0x01);
        pConfig->stStarter.nDisableOut[9] = (stMsgRx->nRxData[4] & 0x02) >> 1;
        pConfig->stStarter.nDisableOut[10] = (stMsgRx->nRxData[4] & 0x04) >> 2;
        pConfig->stStarter.nDisableOut[11] = (stMsgRx->nRxData[4] & 0x08) >> 3;
        nSend = 1;
      }

      if((stMsgRx->nRxLen == 1) || nSend){
        stMsgCanTx.stTxHeader.DLC = 5;

        stMsgCanTx.nTxData[0] = MSG_TX_SET_STARTER;
        stMsgCanTx.nTxData[1] = (pConfig->stStarter.nEnabled & 0x01);
        stMsgCanTx.nTxData[2] = pConfig->stStarter.nInput;
        stMsgCanTx.nTxData[3] = ((pConfig->stStarter.nDisableOut[7] & 0x01) << 7) + ((pConfig->stStarter.nDisableOut[6] & 0x01) << 6) +
                                ((pConfig->stStarter.nDisableOut[5] & 0x01) << 5) + ((pConfig->stStarter.nDisableOut[4] & 0x01) << 4) +
                                ((pConfig->stStarter.nDisableOut[3] & 0x01) << 3) + ((pConfig->stStarter.nDisableOut[2] & 0x01) << 2) +
                                ((pConfig->stStarter.nDisableOut[1] & 0x01) << 1) + (pConfig->stStarter.nDisableOut[0] & 0x01);
        stMsgCanTx.nTxData[4] = ((pConfig->stStarter.nDisableOut[11] & 0x01) << 3) + ((pConfig->stStarter.nDisableOut[10] & 0x01) << 2) +
                                ((pConfig->stStarter.nDisableOut[9] & 0x01) << 1) + (pConfig->stStarter.nDisableOut[8] & 0x01);
        stMsgCanTx.nTxData[5] = 0;
        stMsgCanTx.nTxData[6] = 0;
        stMsgCanTx.nTxData[7] = 0;
        nSend = 1;
      }
    break;

    //Set CAN Input Settings
    // 'N'
    case MSG_RX_SET_CAN_INPUTS:
       if(stMsgRx->nRxLen == 7){
         nCanInputNum = (stMsgRx->nRxData[2]);
         if(nCanInputNum < PDM_NUM_CAN_INPUTS){
           pConfig->stCanInput[nCanInputNum].nEnabled = (stMsgRx->nRxData[1] & 0x01);
           pConfig->stCanInput[nCanInputNum].eMode = (stMsgRx->nRxData[1] & 0x06) >> 1;
           pConfig->stCanInput[nCanInputNum].eOperator = (stMsgRx->nRxData[1] & 0xF0) >> 4;

           pConfig->stCanInput[nCanInputNum].nId = (stMsgRx->nRxData[3] << 8) + stMsgRx->nRxData[4];

           pConfig->stCanInput[nCanInputNum].nLowByte = (stMsgRx->nRxData[5] & 0x0F);
           pConfig->stCanInput[nCanInputNum].nHighByte = (stMsgRx->nRxData[5] & 0xF0) >> 4;

           pConfig->stCanInput[nCanInputNum].nOnVal = stMsgRx->nRxData[6];

           nSend = 1;
         }
       }

       if(stMsgRx->nRxLen == 2){
          nCanInputNum = (stMsgRx->nRxData[1]);
          if(nCanInputNum < PDM_NUM_CAN_INPUTS){
            nSend = 1;
          }
       }

       if(nSend){
         stMsgCanTx.stTxHeader.DLC = 7;

         stMsgCanTx.nTxData[0] = MSG_TX_SET_CAN_INPUTS;
         stMsgCanTx.nTxData[1] = ((pConfig->stCanInput[nCanInputNum].eOperator & 0x0F) << 4) + ((pConfig->stCanInput[nCanInputNum].eMode & 0x03) << 1) +
                                 (pConfig->stCanInput[nCanInputNum].nEnabled & 0x01);
         stMsgCanTx.nTxData[2] = nCanInputNum;
         stMsgCanTx.nTxData[3] = (uint8_t)(pConfig->stCanInput[nCanInputNum].nId >> 8);
         stMsgCanTx.nTxData[4] = (uint8_t)(pConfig->stCanInput[nCanInputNum].nId & 0xFF);
         stMsgCanTx.nTxData[5] = ((pConfig->stCanInput[nCanInputNum].nHighByte & 0xF) << 4) + (pConfig->stCanInput[nCanInputNum].nLowByte & 0xF);
         stMsgCanTx.nTxData[6] = (uint8_t)(pConfig->stCanInput[nCanInputNum].nOnVal);
       }
    break;

    //Get Version
    // 'V'
    case MSG_RX_GET_VERSION:
      if(stMsgRx->nRxLen == 1){
        nSend = 1;
        stMsgCanTx.stTxHeader.DLC = 5;

        stMsgCanTx.nTxData[0] = MSG_TX_GET_VERSION;
        stMsgCanTx.nTxData[1] = (uint8_t)PDM_MAJOR_VERSION;
        stMsgCanTx.nTxData[2] = (uint8_t)PDM_MINOR_VERSION;
        stMsgCanTx.nTxData[3] = (uint8_t)(PDM_BUILD >> 8);
        stMsgCanTx.nTxData[4] = (uint8_t)(PDM_BUILD & 0xFF);
        stMsgCanTx.nTxData[5] = 0;
        stMsgCanTx.nTxData[6] = 0;
        stMsgCanTx.nTxData[7] = 0;
      }
    break;

    default:
      return 0;
    }

  if(nSend){
    stMsgCanTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + 20;
    osMessageQueuePut(*qMsgQueueCanTx, &stMsgCanTx, 0U, 0U);
  }

  return 1;

}
*/
void PdmConfig_SetDefault(PdmConfig_t* pConfig){
  //Device Configuration
  pConfig->stDevConfig.nVersion = 3;
  pConfig->stDevConfig.nCanEnabled = 1;
  pConfig->stDevConfig.nCanSpeed = 6;

  //Inputs
  pConfig->stInput[0].nEnabled = 1;
  pConfig->stInput[0].eMode = MODE_MOMENTARY;
  pConfig->stInput[0].nOnLevel = 0;
  pConfig->stInput[0].nDebounceTime = 20;

  pConfig->stInput[1].nEnabled = 1;
  pConfig->stInput[1].eMode = MODE_MOMENTARY;
  pConfig->stInput[1].nOnLevel = 0;
  pConfig->stInput[1].nDebounceTime = 20;

  //Outputs
  pConfig->stOutput[0].nEnabled = 1;
  pConfig->stOutput[0].nInput = 1;
  pConfig->stOutput[0].nCurrentLimit = 250; //Current * 10
  pConfig->stOutput[0].nInrushLimit = 300; //Current * 10
  pConfig->stOutput[0].nInrushTime = 2000; //ms
  pConfig->stOutput[0].eResetMode = RESET_COUNT;
  pConfig->stOutput[0].nResetTime = 1000; //ms
  pConfig->stOutput[0].nResetLimit = 3; //count

  pConfig->stOutput[1].nEnabled = 1;
  pConfig->stOutput[1].nInput = 2;
  pConfig->stOutput[1].nCurrentLimit = 250;
  pConfig->stOutput[1].nInrushLimit = 300;
  pConfig->stOutput[1].nInrushTime = 2000;
  pConfig->stOutput[1].eResetMode = RESET_ENDLESS;
  pConfig->stOutput[1].nResetTime = 1000;
  pConfig->stOutput[1].nResetLimit = 2;

  pConfig->stOutput[2].nEnabled = 1;
  pConfig->stOutput[2].nInput = 1;
  pConfig->stOutput[2].nCurrentLimit = 130;
  pConfig->stOutput[2].nInrushLimit = 160;
  pConfig->stOutput[2].nInrushTime = 2000;
  pConfig->stOutput[2].eResetMode = RESET_COUNT;
  pConfig->stOutput[2].nResetTime = 1000;
  pConfig->stOutput[2].nResetLimit = 3;

  pConfig->stOutput[3].nEnabled = 1;
  pConfig->stOutput[3].nInput = 2;
  pConfig->stOutput[3].nCurrentLimit = 130;
  pConfig->stOutput[3].nInrushLimit = 160;
  pConfig->stOutput[3].nInrushTime = 2000;
  pConfig->stOutput[3].eResetMode = RESET_ENDLESS;
  pConfig->stOutput[3].nResetTime = 1000;
  pConfig->stOutput[3].nResetLimit = 2;

  pConfig->stOutput[4].nEnabled = 1;
  pConfig->stOutput[4].nInput = 1;
  pConfig->stOutput[4].nCurrentLimit = 80;
  pConfig->stOutput[4].nInrushLimit = 160;
  pConfig->stOutput[4].nInrushTime = 2000;
  pConfig->stOutput[4].eResetMode = RESET_ENDLESS;
  pConfig->stOutput[4].nResetTime = 1000;
  pConfig->stOutput[4].nResetLimit = 2;

  pConfig->stOutput[5].nEnabled = 1;
  pConfig->stOutput[5].nInput = 2;
  pConfig->stOutput[5].nCurrentLimit = 80;
  pConfig->stOutput[5].nInrushLimit = 160;
  pConfig->stOutput[5].nInrushTime = 2000;
  pConfig->stOutput[5].eResetMode = RESET_ENDLESS;
  pConfig->stOutput[5].nResetTime = 1000;
  pConfig->stOutput[5].nResetLimit = 2;

  pConfig->stOutput[6].nEnabled = 1;
  pConfig->stOutput[6].nInput = 1;
  pConfig->stOutput[6].nCurrentLimit = 150;
  pConfig->stOutput[6].nInrushLimit = 300;
  pConfig->stOutput[6].nInrushTime = 2000;
  pConfig->stOutput[6].eResetMode = RESET_ENDLESS;
  pConfig->stOutput[6].nResetTime = 1000;
  pConfig->stOutput[6].nResetLimit = 2;

  pConfig->stOutput[7].nEnabled = 1;
  pConfig->stOutput[7].nInput = 2;
  pConfig->stOutput[7].nCurrentLimit = 150;
  pConfig->stOutput[7].nInrushLimit = 300;
  pConfig->stOutput[7].nInrushTime = 2000;
  pConfig->stOutput[7].eResetMode = RESET_ENDLESS;
  pConfig->stOutput[7].nResetTime = 1000;
  pConfig->stOutput[7].nResetLimit = 2;

  //Virtual Inputs
  pConfig->stVirtualInput[0].nEnabled = 0;
  pConfig->stVirtualInput[0].nNot0 = 0;
  pConfig->stVirtualInput[0].nVar0 = 10;
  pConfig->stVirtualInput[0].eCond0 = COND_AND;
  pConfig->stVirtualInput[0].nNot1 = 1;
  pConfig->stVirtualInput[0].nVar1 = 63;
  pConfig->stVirtualInput[0].eCond1 = COND_OR;
  pConfig->stVirtualInput[0].nNot2 = 0;
  pConfig->stVirtualInput[0].nVar2 = 0;
  pConfig->stVirtualInput[0].eMode = MODE_LATCHING;

  pConfig->stVirtualInput[1].nEnabled = 0;
  pConfig->stVirtualInput[1].nNot0 = 0;
  pConfig->stVirtualInput[1].nVar0 = 11;
  pConfig->stVirtualInput[1].eCond0 = COND_AND;
  pConfig->stVirtualInput[1].nNot1 = 1;
  pConfig->stVirtualInput[1].nVar1 = 62;
  pConfig->stVirtualInput[1].eCond1 = COND_OR;
  pConfig->stVirtualInput[1].nNot2 = 0;
  pConfig->stVirtualInput[1].nVar2 = 0;
  pConfig->stVirtualInput[1].eMode = MODE_LATCHING;

  pConfig->stVirtualInput[2].nEnabled = 0;
  pConfig->stVirtualInput[2].nNot0 = 0;
  pConfig->stVirtualInput[2].nVar0 = 0;
  pConfig->stVirtualInput[2].eCond0 = COND_AND;
  pConfig->stVirtualInput[2].nNot1 = 0;
  pConfig->stVirtualInput[2].nVar1 = 0;
  pConfig->stVirtualInput[2].eCond1 = COND_OR;
  pConfig->stVirtualInput[2].nNot2 = 0;
  pConfig->stVirtualInput[2].nVar2 = 0;
  pConfig->stVirtualInput[2].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[3].nEnabled = 0;
  pConfig->stVirtualInput[3].nNot0 = 0;
  pConfig->stVirtualInput[3].nVar0 = 0;
  pConfig->stVirtualInput[3].eCond0 = COND_AND;
  pConfig->stVirtualInput[3].nNot1 = 0;
  pConfig->stVirtualInput[3].nVar1 = 0;
  pConfig->stVirtualInput[3].eCond1 = COND_OR;
  pConfig->stVirtualInput[3].nNot2 = 0;
  pConfig->stVirtualInput[3].nVar2 = 0;
  pConfig->stVirtualInput[3].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[4].nEnabled = 0;
  pConfig->stVirtualInput[4].nNot0 = 0;
  pConfig->stVirtualInput[4].nVar0 = 0;
  pConfig->stVirtualInput[4].eCond0 = COND_AND;
  pConfig->stVirtualInput[4].nNot1 = 0;
  pConfig->stVirtualInput[4].nVar1 = 0;
  pConfig->stVirtualInput[4].eCond1 = COND_OR;
  pConfig->stVirtualInput[4].nNot2 = 0;
  pConfig->stVirtualInput[4].nVar2 = 0;
  pConfig->stVirtualInput[4].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[5].nEnabled = 0;
  pConfig->stVirtualInput[5].nNot0 = 0;
  pConfig->stVirtualInput[5].nVar0 = 0;
  pConfig->stVirtualInput[5].eCond0 = COND_AND;
  pConfig->stVirtualInput[5].nNot1 = 0;
  pConfig->stVirtualInput[5].nVar1 = 0;
  pConfig->stVirtualInput[5].eCond1 = COND_OR;
  pConfig->stVirtualInput[5].nNot2 = 0;
  pConfig->stVirtualInput[5].nVar2 = 0;
  pConfig->stVirtualInput[5].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[6].nEnabled = 0;
  pConfig->stVirtualInput[6].nNot0 = 0;
  pConfig->stVirtualInput[6].nVar0 = 0;
  pConfig->stVirtualInput[6].eCond0 = COND_AND;
  pConfig->stVirtualInput[6].nNot1 = 0;
  pConfig->stVirtualInput[6].nVar1 = 0;
  pConfig->stVirtualInput[6].eCond1 = COND_OR;
  pConfig->stVirtualInput[6].nNot2 = 0;
  pConfig->stVirtualInput[6].nVar2 = 0;
  pConfig->stVirtualInput[6].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[7].nEnabled = 0;
  pConfig->stVirtualInput[7].nNot0 = 0;
  pConfig->stVirtualInput[7].nVar0 = 0;
  pConfig->stVirtualInput[7].eCond0 = COND_AND;
  pConfig->stVirtualInput[7].nNot1 = 0;
  pConfig->stVirtualInput[7].nVar1 = 0;
  pConfig->stVirtualInput[7].eCond1 = COND_OR;
  pConfig->stVirtualInput[7].nNot2 = 0;
  pConfig->stVirtualInput[7].nVar2 = 0;
  pConfig->stVirtualInput[7].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[8].nEnabled = 0;
  pConfig->stVirtualInput[8].nNot0 = 0;
  pConfig->stVirtualInput[8].nVar0 = 0;
  pConfig->stVirtualInput[8].eCond0 = COND_AND;
  pConfig->stVirtualInput[8].nNot1 = 0;
  pConfig->stVirtualInput[8].nVar1 = 0;
  pConfig->stVirtualInput[8].eCond1 = COND_OR;
  pConfig->stVirtualInput[8].nNot2 = 0;
  pConfig->stVirtualInput[8].nVar2 = 0;
  pConfig->stVirtualInput[8].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[9].nEnabled = 0;
  pConfig->stVirtualInput[9].nNot0 = 0;
  pConfig->stVirtualInput[9].nVar0 = 0;
  pConfig->stVirtualInput[9].eCond0 = COND_AND;
  pConfig->stVirtualInput[9].nNot1 = 0;
  pConfig->stVirtualInput[9].nVar1 = 0;
  pConfig->stVirtualInput[9].eCond1 = COND_OR;
  pConfig->stVirtualInput[9].nNot2 = 0;
  pConfig->stVirtualInput[9].nVar2 = 0;
  pConfig->stVirtualInput[9].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[10].nEnabled = 0;
  pConfig->stVirtualInput[10].nNot0 = 0;
  pConfig->stVirtualInput[10].nVar0 = 0;
  pConfig->stVirtualInput[10].eCond0 = COND_AND;
  pConfig->stVirtualInput[10].nNot1 = 0;
  pConfig->stVirtualInput[10].nVar1 = 0;
  pConfig->stVirtualInput[10].eCond1 = COND_OR;
  pConfig->stVirtualInput[10].nNot2 = 0;
  pConfig->stVirtualInput[10].nVar2 = 0;
  pConfig->stVirtualInput[10].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[11].nEnabled = 0;
  pConfig->stVirtualInput[11].nNot0 = 0;
  pConfig->stVirtualInput[11].nVar0 = 0;
  pConfig->stVirtualInput[11].eCond0 = COND_AND;
  pConfig->stVirtualInput[11].nNot1 = 0;
  pConfig->stVirtualInput[11].nVar1 = 0;
  pConfig->stVirtualInput[11].eCond1 = COND_OR;
  pConfig->stVirtualInput[11].nNot2 = 0;
  pConfig->stVirtualInput[11].nVar2 = 0;
  pConfig->stVirtualInput[11].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[12].nEnabled = 0;
  pConfig->stVirtualInput[12].nNot0 = 0;
  pConfig->stVirtualInput[12].nVar0 = 0;
  pConfig->stVirtualInput[12].eCond0 = COND_AND;
  pConfig->stVirtualInput[12].nNot1 = 0;
  pConfig->stVirtualInput[12].nVar1 = 0;
  pConfig->stVirtualInput[12].eCond1 = COND_OR;
  pConfig->stVirtualInput[12].nNot2 = 0;
  pConfig->stVirtualInput[12].nVar2 = 0;
  pConfig->stVirtualInput[12].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[13].nEnabled = 0;
  pConfig->stVirtualInput[13].nNot0 = 0;
  pConfig->stVirtualInput[13].nVar0 = 0;
  pConfig->stVirtualInput[13].eCond0 = COND_AND;
  pConfig->stVirtualInput[13].nNot1 = 0;
  pConfig->stVirtualInput[13].nVar1 = 0;
  pConfig->stVirtualInput[13].eCond1 = COND_OR;
  pConfig->stVirtualInput[13].nNot2 = 0;
  pConfig->stVirtualInput[13].nVar2 = 0;
  pConfig->stVirtualInput[13].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[14].nEnabled = 0;
  pConfig->stVirtualInput[14].nNot0 = 0;
  pConfig->stVirtualInput[14].nVar0 = 0;
  pConfig->stVirtualInput[14].eCond0 = COND_AND;
  pConfig->stVirtualInput[14].nNot1 = 0;
  pConfig->stVirtualInput[14].nVar1 = 0;
  pConfig->stVirtualInput[14].eCond1 = COND_OR;
  pConfig->stVirtualInput[14].nNot2 = 0;
  pConfig->stVirtualInput[14].nVar2 = 0;
  pConfig->stVirtualInput[14].eMode = MODE_MOMENTARY;

  pConfig->stVirtualInput[15].nEnabled = 0;
  pConfig->stVirtualInput[15].nNot0 = 0;
  pConfig->stVirtualInput[15].nVar0 = 0;
  pConfig->stVirtualInput[15].eCond0 = COND_AND;
  pConfig->stVirtualInput[15].nNot1 = 0;
  pConfig->stVirtualInput[15].nVar1 = 0;
  pConfig->stVirtualInput[15].eCond1 = COND_OR;
  pConfig->stVirtualInput[15].nNot2 = 0;
  pConfig->stVirtualInput[15].nVar2 = 0;
  pConfig->stVirtualInput[15].eMode = MODE_MOMENTARY;

  //Wiper
  pConfig->stWiper.nEnabled = 0;
  pConfig->stWiper.nMode = 2;
  pConfig->stWiper.nSlowInput = 0;
  pConfig->stWiper.nFastInput = 0;
  pConfig->stWiper.nInterInput = 0;
  pConfig->stWiper.nOnInput = 15;
  pConfig->stWiper.nSpeedInput = 7;
  pConfig->stWiper.nParkInput = 1;
  pConfig->stWiper.nParkStopLevel = 0;
  pConfig->stWiper.nSwipeInput = 4;
  pConfig->stWiper.nWashInput = 14;
  pConfig->stWiper.nWashWipeCycles = 2;
  pConfig->stWiper.nSpeedMap[0] = 3;
  pConfig->stWiper.nSpeedMap[1] = 4;
  pConfig->stWiper.nSpeedMap[2] = 5;
  pConfig->stWiper.nSpeedMap[3] = 6;
  pConfig->stWiper.nSpeedMap[4] = 7;
  pConfig->stWiper.nSpeedMap[5] = 8;
  pConfig->stWiper.nSpeedMap[6] = 1;
  pConfig->stWiper.nSpeedMap[7] = 2;
  pConfig->stWiper.nIntermitTime[0] = 1000;
  pConfig->stWiper.nIntermitTime[1] = 2000;
  pConfig->stWiper.nIntermitTime[2] = 3000;
  pConfig->stWiper.nIntermitTime[3] = 4000;
  pConfig->stWiper.nIntermitTime[4] = 5000;
  pConfig->stWiper.nIntermitTime[5] = 6000;

  //Flasher
  pConfig->stFlasher[0].nEnabled = 1;
  pConfig->stFlasher[0].nInput = 7;
  pConfig->stFlasher[0].nFlashOnTime = 500;
  pConfig->stFlasher[0].nFlashOffTime = 500;
  pConfig->stFlasher[0].nSingleCycle = 0;
  pConfig->stFlasher[0].nOutput = 10;

  pConfig->stFlasher[1].nEnabled = 1;
  pConfig->stFlasher[1].nInput = 8;
  pConfig->stFlasher[1].nFlashOnTime = 250;
  pConfig->stFlasher[1].nFlashOffTime = 250;
  pConfig->stFlasher[1].nSingleCycle = 0;
  pConfig->stFlasher[1].nOutput = 11;

  pConfig->stFlasher[2].nEnabled = 0;
  pConfig->stFlasher[2].nInput = 0;
  pConfig->stFlasher[2].nFlashOnTime = 0;
  pConfig->stFlasher[2].nFlashOffTime = 0;
  pConfig->stFlasher[2].nSingleCycle = 0;
  pConfig->stFlasher[2].nOutput = 0;

  pConfig->stFlasher[3].nEnabled = 0;
  pConfig->stFlasher[3].nInput = 0;
  pConfig->stFlasher[3].nFlashOnTime = 0;
  pConfig->stFlasher[3].nFlashOffTime = 0;
  pConfig->stFlasher[3].nSingleCycle = 0;
  pConfig->stFlasher[3].nOutput = 0;

  //Starter
  pConfig->stStarter.nEnabled = 0;
  pConfig->stStarter.nInput = 8;
  pConfig->stStarter.nDisableOut[0] = 0;
  pConfig->stStarter.nDisableOut[1] = 1;
  pConfig->stStarter.nDisableOut[2] = 0;
  pConfig->stStarter.nDisableOut[3] = 0;
  pConfig->stStarter.nDisableOut[4] = 0;
  pConfig->stStarter.nDisableOut[5] = 0;
  pConfig->stStarter.nDisableOut[6] = 0;
  pConfig->stStarter.nDisableOut[7] = 0;
  pConfig->stStarter.nDisableOut[8] = 0;
  pConfig->stStarter.nDisableOut[9] = 0;
  pConfig->stStarter.nDisableOut[10] = 0;
  pConfig->stStarter.nDisableOut[11] = 0;

  //CAN Input
  pConfig->stCanInput[0].nEnabled = 1;
  pConfig->stCanInput[0].nId = 1602;
  pConfig->stCanInput[0].nLowByte = 0;
  pConfig->stCanInput[0].nHighByte = 0;
  pConfig->stCanInput[0].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[0].nOnVal = 0xF;
  pConfig->stCanInput[0].eMode = MODE_NUM;

  pConfig->stCanInput[1].nEnabled = 1;
  pConfig->stCanInput[1].nId = 1602;
  pConfig->stCanInput[1].nLowByte = 4;
  pConfig->stCanInput[1].nHighByte = 0;
  pConfig->stCanInput[1].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[1].nOnVal = 0x1;
  pConfig->stCanInput[1].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[2].nEnabled = 1;
  pConfig->stCanInput[2].nId = 1602;
  pConfig->stCanInput[2].nLowByte = 4;
  pConfig->stCanInput[2].nHighByte = 0;
  pConfig->stCanInput[2].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[2].nOnVal = 0x2;
  pConfig->stCanInput[2].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[3].nEnabled = 1;
  pConfig->stCanInput[3].nId = 1602;
  pConfig->stCanInput[3].nLowByte = 4;
  pConfig->stCanInput[3].nHighByte = 0;
  pConfig->stCanInput[3].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[3].nOnVal = 0x4;
  pConfig->stCanInput[3].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[4].nEnabled = 1;
  pConfig->stCanInput[4].nId = 1602;
  pConfig->stCanInput[4].nLowByte = 4;
  pConfig->stCanInput[4].nHighByte = 0;
  pConfig->stCanInput[4].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[4].nOnVal = 0x8;
  pConfig->stCanInput[4].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[5].nEnabled = 1;
  pConfig->stCanInput[5].nId = 1602;
  pConfig->stCanInput[5].nLowByte = 4;
  pConfig->stCanInput[5].nHighByte = 0;
  pConfig->stCanInput[5].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[5].nOnVal = 0x10;
  pConfig->stCanInput[5].eMode = MODE_LATCHING;

  pConfig->stCanInput[6].nEnabled = 1;
  pConfig->stCanInput[6].nId = 1602;
  pConfig->stCanInput[6].nLowByte = 4;
  pConfig->stCanInput[6].nHighByte = 0;
  pConfig->stCanInput[6].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[6].nOnVal = 0x20;
  pConfig->stCanInput[6].eMode = MODE_LATCHING;

  pConfig->stCanInput[7].nEnabled = 1;
  pConfig->stCanInput[7].nId = 1602;
  pConfig->stCanInput[7].nLowByte = 4;
  pConfig->stCanInput[7].nHighByte = 0;
  pConfig->stCanInput[7].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[7].nOnVal = 0x40;
  pConfig->stCanInput[7].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[8].nEnabled = 1;
  pConfig->stCanInput[8].nId = 1602;
  pConfig->stCanInput[8].nLowByte = 4;
  pConfig->stCanInput[8].nHighByte = 0;
  pConfig->stCanInput[8].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[8].nOnVal = 0x80;
  pConfig->stCanInput[8].eMode = MODE_LATCHING;

  pConfig->stCanInput[9].nEnabled = 1;
  pConfig->stCanInput[9].nId = 1620;
  pConfig->stCanInput[9].nLowByte = 4;
  pConfig->stCanInput[9].nHighByte = 5;
  pConfig->stCanInput[9].eOperator = OPER_EQUAL;
  pConfig->stCanInput[9].nOnVal = 1;
  pConfig->stCanInput[9].eMode = MODE_NUM;

  pConfig->stCanInput[10].nEnabled = 1;
  pConfig->stCanInput[10].nId = 1620;
  pConfig->stCanInput[10].nLowByte = 2;
  pConfig->stCanInput[10].nHighByte = 3;
  pConfig->stCanInput[10].eOperator = OPER_EQUAL;
  pConfig->stCanInput[10].nOnVal = 1;
  pConfig->stCanInput[10].eMode = MODE_NUM;

  pConfig->stCanInput[11].nEnabled = 1;
  pConfig->stCanInput[11].nId = 1620;
  pConfig->stCanInput[11].nLowByte = 0;
  pConfig->stCanInput[11].nHighByte = 1;
  pConfig->stCanInput[11].eOperator = OPER_EQUAL;
  pConfig->stCanInput[11].nOnVal = 1;
  pConfig->stCanInput[11].eMode = MODE_NUM;

  pConfig->stCanInput[12].nEnabled = 1;
  pConfig->stCanInput[12].nId = 1620;
  pConfig->stCanInput[12].nLowByte = 6;
  pConfig->stCanInput[12].nHighByte = 7;
  pConfig->stCanInput[12].eOperator = OPER_EQUAL;
  pConfig->stCanInput[12].nOnVal = 1;
  pConfig->stCanInput[12].eMode = MODE_NUM;

  pConfig->stCanInput[13].nEnabled = 1;
  pConfig->stCanInput[13].nId = 1621;
  pConfig->stCanInput[13].nLowByte = 0;
  pConfig->stCanInput[13].nHighByte = 1;
  pConfig->stCanInput[13].eOperator = OPER_EQUAL;
  pConfig->stCanInput[13].nOnVal = 1;
  pConfig->stCanInput[13].eMode = MODE_NUM;

  pConfig->stCanInput[14].nEnabled = 1;
  pConfig->stCanInput[14].nId = 1621;
  pConfig->stCanInput[14].nLowByte = 2;
  pConfig->stCanInput[14].nHighByte = 3;
  pConfig->stCanInput[14].eOperator = OPER_EQUAL;
  pConfig->stCanInput[14].nOnVal = 1;
  pConfig->stCanInput[14].eMode = MODE_NUM;

  pConfig->stCanInput[15].nEnabled = 1;
  pConfig->stCanInput[15].nId = 1622;
  pConfig->stCanInput[15].nLowByte = 0;
  pConfig->stCanInput[15].nHighByte = 0;
  pConfig->stCanInput[15].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[15].nOnVal = 0x01;
  pConfig->stCanInput[15].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[16].nEnabled = 1;
  pConfig->stCanInput[16].nId = 1622;
  pConfig->stCanInput[16].nLowByte = 0;
  pConfig->stCanInput[16].nHighByte = 0;
  pConfig->stCanInput[16].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[16].nOnVal = 0x02;
  pConfig->stCanInput[16].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[17].nEnabled = 1;
  pConfig->stCanInput[17].nId = 1622;
  pConfig->stCanInput[17].nLowByte = 0;
  pConfig->stCanInput[17].nHighByte = 0;
  pConfig->stCanInput[17].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[17].nOnVal = 0x04;
  pConfig->stCanInput[17].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[18].nEnabled = 1;
  pConfig->stCanInput[18].nId = 1622;
  pConfig->stCanInput[18].nLowByte = 0;
  pConfig->stCanInput[18].nHighByte = 0;
  pConfig->stCanInput[18].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[18].nOnVal = 0x08;
  pConfig->stCanInput[18].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[19].nEnabled = 1;
  pConfig->stCanInput[19].nId = 1622;
  pConfig->stCanInput[19].nLowByte = 0;
  pConfig->stCanInput[19].nHighByte = 0;
  pConfig->stCanInput[19].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[19].nOnVal = 0x10;
  pConfig->stCanInput[19].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[20].nEnabled = 1;
  pConfig->stCanInput[20].nId = 1622;
  pConfig->stCanInput[20].nLowByte = 0;
  pConfig->stCanInput[20].nHighByte = 0;
  pConfig->stCanInput[20].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[20].nOnVal = 0x20;
  pConfig->stCanInput[20].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[21].nEnabled = 1;
  pConfig->stCanInput[21].nId = 1622;
  pConfig->stCanInput[21].nLowByte = 0;
  pConfig->stCanInput[21].nHighByte = 0;
  pConfig->stCanInput[21].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[21].nOnVal = 0x40;
  pConfig->stCanInput[21].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[22].nEnabled = 1;
  pConfig->stCanInput[22].nId = 1622;
  pConfig->stCanInput[22].nLowByte = 0;
  pConfig->stCanInput[22].nHighByte = 0;
  pConfig->stCanInput[22].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[22].nOnVal = 0x80;
  pConfig->stCanInput[22].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[23].nEnabled = 1;
  pConfig->stCanInput[23].nId = 1622;
  pConfig->stCanInput[23].nLowByte = 1;
  pConfig->stCanInput[23].nHighByte = 0;
  pConfig->stCanInput[23].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[23].nOnVal = 0x01;
  pConfig->stCanInput[23].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[24].nEnabled = 1;
  pConfig->stCanInput[24].nId = 1622;
  pConfig->stCanInput[24].nLowByte = 1;
  pConfig->stCanInput[24].nHighByte = 0;
  pConfig->stCanInput[24].eOperator = OPER_BITWISE_AND;
  pConfig->stCanInput[24].nOnVal = 0x02;
  pConfig->stCanInput[24].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[25].nEnabled = 0;
  pConfig->stCanInput[25].nId = 0;
  pConfig->stCanInput[25].nLowByte = 0;
  pConfig->stCanInput[25].nHighByte = 0;
  pConfig->stCanInput[25].eOperator = OPER_EQUAL;
  pConfig->stCanInput[25].nOnVal = 0;
  pConfig->stCanInput[25].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[26].nEnabled = 0;
  pConfig->stCanInput[26].nId = 0;
  pConfig->stCanInput[26].nLowByte = 0;
  pConfig->stCanInput[26].nHighByte = 0;
  pConfig->stCanInput[26].eOperator = OPER_EQUAL;
  pConfig->stCanInput[26].nOnVal = 0;
  pConfig->stCanInput[26].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[27].nEnabled = 0;
  pConfig->stCanInput[27].nId = 0;
  pConfig->stCanInput[27].nLowByte = 0;
  pConfig->stCanInput[27].nHighByte = 0;
  pConfig->stCanInput[27].eOperator = OPER_EQUAL;
  pConfig->stCanInput[27].nOnVal = 0;
  pConfig->stCanInput[27].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[28].nEnabled = 0;
  pConfig->stCanInput[28].nId = 0;
  pConfig->stCanInput[28].nLowByte = 0;
  pConfig->stCanInput[28].nHighByte = 0;
  pConfig->stCanInput[28].eOperator = OPER_EQUAL;
  pConfig->stCanInput[28].nOnVal = 0;
  pConfig->stCanInput[28].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[29].nEnabled = 0;
  pConfig->stCanInput[29].nId = 0;
  pConfig->stCanInput[29].nLowByte = 0;
  pConfig->stCanInput[29].nHighByte = 0;
  pConfig->stCanInput[29].eOperator = OPER_EQUAL;
  pConfig->stCanInput[29].nOnVal = 0;
  pConfig->stCanInput[29].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[30].nEnabled = 0;
  pConfig->stCanInput[30].nId = 0;
  pConfig->stCanInput[30].nLowByte = 0;
  pConfig->stCanInput[30].nHighByte = 0;
  pConfig->stCanInput[30].eOperator = OPER_EQUAL;
  pConfig->stCanInput[30].nOnVal = 0;
  pConfig->stCanInput[30].eMode = MODE_MOMENTARY;

  pConfig->stCanInput[31].nEnabled = 0;
  pConfig->stCanInput[31].nId = 0;
  pConfig->stCanInput[31].nLowByte = 0;
  pConfig->stCanInput[31].nHighByte = 0;
  pConfig->stCanInput[31].eOperator = OPER_EQUAL;
  pConfig->stCanInput[31].nOnVal = 0;
  pConfig->stCanInput[31].eMode = MODE_MOMENTARY;

  //CAN Output
  pConfig->stCanOutput.nEnabled = 1;
  pConfig->stCanOutput.nBaseId = 2000;
  pConfig->stCanOutput.nUpdateTime = 50;
}

/*
0   None
1   PDM In 1
2   PDM In 2
3   PDM In 3
4   PDM In 4
5   PDM In 5
6   PDM In 6
7   PDM In 7
8   PDM In 8
9   CAN In 1
10  CAN In 2
11  CAN In 3
12  CAN In 4
13  CAN In 5
14  CAN In 6
15  CAN In 7
16  CAN In 8
17  CAN In 9
18  CAN In 10
19  CAN In 11
20  CAN In 12
21  CAN In 13
22  CAN In 14
23  CAN In 15
24  CAN In 16
25  CAN In 17
26  CAN In 18
27  CAN In 19
28  CAN In 20
29  CAN In 21
30  CAN In 22
31  CAN In 23
32  CAN In 24
33  CAN In 25
34  CAN In 26
35  CAN In 27
36  CAN In 28
37  CAN In 29
38  CAN In 30
39  Virtual In 1
40  Virtual In 2
41  Virtual In 3
42  Virtual In 4
43  Virtual In 5
44  Virtual In 6
45  Virtual In 7
46  Virtual In 8
47  Virtual In 9
48  Virtual In 10
49  Virtual In 11
50  Virtual In 12
51  Virtual In 13
52  Virtual In 14
53  Virtual In 15
54  Virtual In 16
55  Virtual In 17
56  Virtual In 18
57  Virtual In 19
58  Virtual In 20
59  Output 1
60  Output 2
61  Output 3
62  Output 4
63  Output 5
64  Output 6
65  Output 7
66  Output 8
67  Output 9
68  Output 10
69  Output 11
70  Output 12
71  Wiper Slow Out
72  Wiper Fast Out
 */
