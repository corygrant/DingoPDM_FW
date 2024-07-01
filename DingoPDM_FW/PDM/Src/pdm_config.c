#include "pdm_config.h"

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

  //Write the size of the config structure to FRAM
  //Used to check that the settings have been stored in FRAM properly 
  uint16_t nSizeOfConfig = sizeof(*pConfig);

  if(!MB85RC_Write(hi2c, nAddr, sizeof(*pConfig), (uint8_t*)&nSizeOfConfig, sizeof(nSizeOfConfig)) == HAL_OK)
  {
    return 0;
  }

  return 1;
}

void SetCan(PdmConfig_t* pConfig, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void SetInputs(PdmConfig_t* pConfig, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void SetOutputs(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], volatile ProfetTypeDef profet[PDM_NUM_OUTPUTS], MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void SetVirtInputs(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void SetWiper(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], Wiper_t* pWiper, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void SetWiperSpeed(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], Wiper_t* pWiper, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void SetWiperDelay(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], Wiper_t* pWiper, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void SetFlashers(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void SetStarter(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void SetCanInputs(PdmConfig_t* pConfig, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void GetVersion(PdmConfig_t* pConfig, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);

uint8_t PdmConfig_Set(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], volatile ProfetTypeDef profet[PDM_NUM_OUTPUTS], Wiper_t* pWiper, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  switch((MsgQueueRxCmd_t)stMsgRx->nRxData[0]){

    //Set CAN Settings
    // 'C'
    case MSG_RX_SET_CAN:
      SetCan(pConfig, stMsgRx, qMsgQueueTx);
    break;

    //Set Input Settings
    // 'I'
    case MSG_RX_SET_INPUTS:
      SetInputs(pConfig, stMsgRx, qMsgQueueTx);
    break;

    //Set Output Settings
    // 'O'
    case MSG_RX_SET_OUTPUTS:
      SetOutputs(pConfig, pVariableMap, profet, stMsgRx, qMsgQueueTx);
    break;

    //Set Virtual Input Settings
    // 'U'
    case MSG_RX_SET_VIRTUAL_INPUTS:
      SetVirtInputs(pConfig, pVariableMap, stMsgRx, qMsgQueueTx);
    break;

    //Set Wiper Settings
    // 'W'
    case MSG_RX_SET_WIPER:
      SetWiper(pConfig, pVariableMap, pWiper, stMsgRx, qMsgQueueTx);
    break;

    //Set Wiper Speed Settings
    // 'P'
    case MSG_RX_SET_WIPER_SPEED:
      SetWiperSpeed(pConfig, pVariableMap, pWiper, stMsgRx, qMsgQueueTx);
    break;

    //Set Wiper Intermit Delays Settings
    // 'Y'
    case MSG_RX_SET_WIPER_DELAYS:
      SetWiperDelay(pConfig, pVariableMap, pWiper, stMsgRx, qMsgQueueTx);
    break;

    //Set Flasher Settings
    // 'H'
    case MSG_RX_SET_FLASHER:
      SetFlashers(pConfig, pVariableMap, stMsgRx, qMsgQueueTx);
    break;

    //Set Starter Disable Settings
    // 'D'
    case MSG_RX_SET_STARTER:
      SetStarter(pConfig, pVariableMap, stMsgRx, qMsgQueueTx);
    break;

    //Set CAN Input Settings
    // 'N'
    case MSG_RX_SET_CAN_INPUTS:
      SetCanInputs(pConfig, stMsgRx, qMsgQueueTx);
    break;

    //Get Version
    // 'V'
    case MSG_RX_GET_VERSION:
      GetVersion(pConfig, stMsgRx, qMsgQueueTx);
    break;

    default:
      return 0;
    }

  return 1;
}

void SetCan(PdmConfig_t* pConfig, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;
  if(stMsgRx->nRxLen == 5){
    pConfig->stDevConfig.nCanEnabled = stMsgRx->nRxData[1] & 0x01;
    pConfig->stCanOutput.nEnabled = (stMsgRx->nRxData[1] & 0x02) >> 1;
    pConfig->stDevConfig.nCanSpeed = (stMsgRx->nRxData[1] & 0xF0) >> 4;
    pConfig->stCanOutput.nBaseId = (stMsgRx->nRxData[2] << 8) + stMsgRx->nRxData[3];
    pConfig->stCanOutput.nUpdateTime = stMsgRx->nRxData[4] * 10;
  }

  if( (stMsgRx->nRxLen == 5) ||
      (stMsgRx->nRxLen == 1)){
    stMsgTx.stTxHeader.DLC = 5;

    stMsgTx.nTxData[0] = MSG_TX_SET_CAN;
    stMsgTx.nTxData[1] = ((pConfig->stDevConfig.nCanSpeed & 0x0F) << 4) + ((pConfig->stCanOutput.nEnabled & 0x01) << 1) + (pConfig->stDevConfig.nCanEnabled & 0x01);
    stMsgTx.nTxData[2] = (uint8_t)((pConfig->stCanOutput.nBaseId & 0xFF00) >> 8);
    stMsgTx.nTxData[3] = (uint8_t)(pConfig->stCanOutput.nBaseId & 0x00FF);
    stMsgTx.nTxData[4] = (uint8_t)((pConfig->stCanOutput.nUpdateTime) / 10);
    stMsgTx.nTxData[5] = 0;
    stMsgTx.nTxData[6] = 0;
    stMsgTx.nTxData[7] = 0;

    stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
    osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
  }
}

void SetInputs(PdmConfig_t* pConfig, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;
  uint8_t nIndex;

  if( (stMsgRx->nRxLen == 4) ||
      (stMsgRx->nRxLen == 2)){
    nIndex = (stMsgRx->nRxData[1] & 0xF0) >> 4;
    if(nIndex < PDM_NUM_INPUTS){

      if(stMsgRx->nRxLen == 4)
      {
        pConfig->stInput[nIndex].nEnabled = (stMsgRx->nRxData[1] & 0x01);
        pConfig->stInput[nIndex].eMode = (stMsgRx->nRxData[1] & 0x06) >> 1;
        pConfig->stInput[nIndex].bInvert = (stMsgRx->nRxData[1] & 0x08) >> 3;
        pConfig->stInput[nIndex].nDebounceTime = stMsgRx->nRxData[2] * 10;
        pConfig->stInput[nIndex].ePull = (stMsgRx->nRxData[3] & 0x03);
        //Set the last state
        pConfig->stInput[nIndex].stInVars.bLastState = pConfig->stInput[nIndex].bInvert;

        SetInputPull(pConfig->stInput[nIndex].GPIOx, pConfig->stInput[nIndex].nPin, pConfig->stInput[nIndex].ePull);
      }

      stMsgTx.stTxHeader.DLC = 4;

      stMsgTx.nTxData[0] = MSG_TX_SET_INPUTS;
      stMsgTx.nTxData[1] = ((nIndex & 0x0F) << 4) + ((pConfig->stInput[nIndex].bInvert & 0x01) << 3) + ((pConfig->stInput[nIndex].eMode & 0x03) << 1) + (pConfig->stInput[nIndex].nEnabled & 0x01);
      stMsgTx.nTxData[2] = (uint8_t)(pConfig->stInput[nIndex].nDebounceTime / 10);
      stMsgTx.nTxData[3] = pConfig->stInput[nIndex].ePull;
      stMsgTx.nTxData[4] = 0;
      stMsgTx.nTxData[5] = 0;
      stMsgTx.nTxData[6] = 0;
      stMsgTx.nTxData[7] = 0;

      stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
      osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
    }
  }
}

void SetOutputs(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], volatile ProfetTypeDef profet[PDM_NUM_OUTPUTS], MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;
  uint8_t nIndex;

  if( (stMsgRx->nRxLen == 8) ||
      (stMsgRx->nRxLen == 2)){
    nIndex = (stMsgRx->nRxData[1] & 0xF0) >> 4;
    if(nIndex < PDM_NUM_OUTPUTS){
      if(stMsgRx->nRxLen == 8){
        pConfig->stOutput[nIndex].nEnabled = (stMsgRx->nRxData[1] & 0x01);
        pConfig->stOutput[nIndex].nInput = stMsgRx->nRxData[2];
        pConfig->stOutput[nIndex].nCurrentLimit = stMsgRx->nRxData[3] * 10;
        pConfig->stOutput[nIndex].eResetMode = (stMsgRx->nRxData[4] & 0x0F);
        pConfig->stOutput[nIndex].nResetLimit = (stMsgRx->nRxData[4] & 0xF0) >> 4;
        pConfig->stOutput[nIndex].nResetTime = stMsgRx->nRxData[5] * 100;
        pConfig->stOutput[nIndex].nInrushLimit = stMsgRx->nRxData[6] * 10;
        pConfig->stOutput[nIndex].nInrushTime = stMsgRx->nRxData[7] * 100;
        pConfig->stOutput[nIndex].pInput = pVariableMap[pConfig->stOutput[nIndex].nInput];

        //Copy config values to profet
        profet[nIndex].bEnabled = pConfig->stOutput[nIndex].nEnabled;
        profet[nIndex].nIL_Limit = pConfig->stOutput[nIndex].nCurrentLimit;
        profet[nIndex].nIL_InRushLimit = pConfig->stOutput[nIndex].nInrushLimit;
        profet[nIndex].nIL_InRushTime = pConfig->stOutput[nIndex].nInrushTime;
        profet[nIndex].nOC_ResetLimit = pConfig->stOutput[nIndex].nResetLimit;
        profet[nIndex].nOC_ResetTime = pConfig->stOutput[nIndex].nResetTime;
        profet[nIndex].eResetMode = pConfig->stOutput[nIndex].eResetMode;
      }

      stMsgTx.stTxHeader.DLC = 8;

      stMsgTx.nTxData[0] = MSG_TX_SET_OUTPUTS;
      stMsgTx.nTxData[1] = ((nIndex & 0x0F) << 4) + (pConfig->stOutput[nIndex].nEnabled & 0x01);
      stMsgTx.nTxData[2] = pConfig->stOutput[nIndex].nInput;
      stMsgTx.nTxData[3] = (uint8_t)(pConfig->stOutput[nIndex].nCurrentLimit / 10);
      stMsgTx.nTxData[4] = ((pConfig->stOutput[nIndex].nResetLimit & 0x0F) << 4) + (pConfig->stOutput[nIndex].eResetMode & 0x0F);
      stMsgTx.nTxData[5] = (uint8_t)(pConfig->stOutput[nIndex].nResetTime / 100);
      stMsgTx.nTxData[6] = (uint8_t)(pConfig->stOutput[nIndex].nInrushLimit / 10);
      stMsgTx.nTxData[7] = (uint8_t)(pConfig->stOutput[nIndex].nInrushTime / 100);

      stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
      osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
    }
  }
}

void SetVirtInputs(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;
  uint8_t nIndex;

  if( (stMsgRx->nRxLen == 7) ||
      (stMsgRx->nRxLen == 2)){

    if (stMsgRx->nRxLen == 7)
      nIndex = stMsgRx->nRxData[2];
    else
      nIndex = stMsgRx->nRxData[1];
      
    if(nIndex < PDM_NUM_VIRT_INPUTS){
      if(stMsgRx->nRxLen == 7){
        pConfig->stVirtualInput[nIndex].nEnabled = (stMsgRx->nRxData[1] & 0x01);
        pConfig->stVirtualInput[nIndex].nNot0 = (stMsgRx->nRxData[1] & 0x02) >> 1;
        pConfig->stVirtualInput[nIndex].nNot1 = (stMsgRx->nRxData[1] & 0x04) >> 2;
        pConfig->stVirtualInput[nIndex].nNot2 = (stMsgRx->nRxData[1] & 0x08) >> 3;
        pConfig->stVirtualInput[nIndex].nVar0 = stMsgRx->nRxData[3];
        pConfig->stVirtualInput[nIndex].nVar1 = stMsgRx->nRxData[4];
        pConfig->stVirtualInput[nIndex].nVar2 = stMsgRx->nRxData[5];
        pConfig->stVirtualInput[nIndex].eCond0 = (stMsgRx->nRxData[6] & 0x03);
        pConfig->stVirtualInput[nIndex].eCond1 = (stMsgRx->nRxData[6] & 0x0C) >> 2;
        pConfig->stVirtualInput[nIndex].eMode = (stMsgRx->nRxData[6] & 0xC0) >> 6;
        pConfig->stVirtualInput[nIndex].pVar0 = pVariableMap[pConfig->stVirtualInput[nIndex].nVar0];
        pConfig->stVirtualInput[nIndex].pVar1 = pVariableMap[pConfig->stVirtualInput[nIndex].nVar1];
        pConfig->stVirtualInput[nIndex].pVar2 = pVariableMap[pConfig->stVirtualInput[nIndex].nVar2];
      }
      stMsgTx.stTxHeader.DLC = 7;

      stMsgTx.nTxData[0] = MSG_TX_SET_VIRTUAL_INPUTS;
      stMsgTx.nTxData[1] = ((pConfig->stVirtualInput[nIndex].nNot2 & 0x01) << 3) + ((pConfig->stVirtualInput[nIndex].nNot1 & 0x01) << 2) +
                              ((pConfig->stVirtualInput[nIndex].nNot0 & 0x01) << 1) + (pConfig->stVirtualInput[nIndex].nEnabled & 0x01);
      stMsgTx.nTxData[2] = nIndex;
      stMsgTx.nTxData[3] = pConfig->stVirtualInput[nIndex].nVar0;
      stMsgTx.nTxData[4] = pConfig->stVirtualInput[nIndex].nVar1;
      stMsgTx.nTxData[5] = pConfig->stVirtualInput[nIndex].nVar2;
      stMsgTx.nTxData[6] = ((pConfig->stVirtualInput[nIndex].eMode & 0x0F) << 6) + ((pConfig->stVirtualInput[nIndex].eCond1 & 0x03) << 2) +
                              (pConfig->stVirtualInput[nIndex].eCond0 & 0x03);
      stMsgTx.nTxData[7] = 0;

      stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
      osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
    }
  }
}

void SetWiper(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], Wiper_t* pWiper, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;

  if( (stMsgRx->nRxLen == 8) ||
      (stMsgRx->nRxLen == 1)){
    if(stMsgRx->nRxLen == 8){
      pConfig->stWiper.nEnabled = (stMsgRx->nRxData[1] & 0x01);
      pConfig->stWiper.nMode = (stMsgRx->nRxData[1] & 0x06) >> 1;
      pConfig->stWiper.nParkStopLevel = (stMsgRx->nRxData[1] & 0x08) >> 3;
      pConfig->stWiper.nWashWipeCycles = (stMsgRx->nRxData[1] & 0xF0) >> 4;
      pConfig->stWiper.nSlowInput = stMsgRx->nRxData[2];
      pConfig->stWiper.nFastInput = stMsgRx->nRxData[3];
      pConfig->stWiper.nInterInput = stMsgRx->nRxData[4];
      pConfig->stWiper.nOnInput = stMsgRx->nRxData[5];
      pConfig->stWiper.nParkInput = stMsgRx->nRxData[6];
      pConfig->stWiper.nWashInput = stMsgRx->nRxData[7];

      pWiper->nEnabled = pConfig->stWiper.nEnabled;
      pWiper->eMode = pConfig->stWiper.nMode;
      pWiper->nParkStopLevel = pConfig->stWiper.nParkStopLevel;
      pWiper->nWashWipeCycles = pConfig->stWiper.nWashWipeCycles;
      pWiper->pSlowInput = pVariableMap[pConfig->stWiper.nSlowInput];
      pWiper->pFastInput = pVariableMap[pConfig->stWiper.nFastInput];
      pWiper->pInterInput = pVariableMap[pConfig->stWiper.nInterInput];
      pWiper->pOnSw = pVariableMap[pConfig->stWiper.nOnInput];
      pWiper->pParkSw = pVariableMap[pConfig->stWiper.nParkInput];
      pWiper->pWashInput = pVariableMap[pConfig->stWiper.nWashInput];
    }

    stMsgTx.stTxHeader.DLC = 8;

    stMsgTx.nTxData[0] = MSG_TX_SET_WIPER;
    stMsgTx.nTxData[1] = ((pConfig->stWiper.nWashWipeCycles & 0x0F) << 4) + ((pConfig->stWiper.nParkStopLevel & 0x01) << 3) +
                            ((pConfig->stWiper.nMode & 0x03) << 2) + (pConfig->stWiper.nEnabled & 0x01);
    stMsgTx.nTxData[2] = pConfig->stWiper.nSlowInput;
    stMsgTx.nTxData[3] = pConfig->stWiper.nFastInput;
    stMsgTx.nTxData[4] = pConfig->stWiper.nInterInput;
    stMsgTx.nTxData[5] = pConfig->stWiper.nOnInput;
    stMsgTx.nTxData[6] = pConfig->stWiper.nParkInput;
    stMsgTx.nTxData[7] = pConfig->stWiper.nWashInput;

    stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
    osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
  }
}

void SetWiperSpeed(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], Wiper_t* pWiper, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;

  if( (stMsgRx->nRxLen == 7) ||
      (stMsgRx->nRxLen == 1)){
    if(stMsgRx->nRxLen == 7){
      pConfig->stWiper.nSwipeInput = stMsgRx->nRxData[1];
      pConfig->stWiper.nSpeedInput = stMsgRx->nRxData[2];
      pConfig->stWiper.nSpeedMap[0] = (stMsgRx->nRxData[3] & 0x0F);
      pConfig->stWiper.nSpeedMap[1] = (stMsgRx->nRxData[3] & 0xF0) >> 4;
      pConfig->stWiper.nSpeedMap[2] = (stMsgRx->nRxData[4] & 0x0F);
      pConfig->stWiper.nSpeedMap[3] = (stMsgRx->nRxData[4] & 0xF0) >> 4;
      pConfig->stWiper.nSpeedMap[4] = (stMsgRx->nRxData[5] & 0x0F);
      pConfig->stWiper.nSpeedMap[5] = (stMsgRx->nRxData[5] & 0xF0) >> 4;
      pConfig->stWiper.nSpeedMap[6] = (stMsgRx->nRxData[6] & 0x0F);
      pConfig->stWiper.nSpeedMap[7] = (stMsgRx->nRxData[6] & 0xF0) >> 4;
      pWiper->pSwipeInput = pVariableMap[pConfig->stWiper.nSwipeInput];
      pWiper->pSpeedInput = pVariableMap[pConfig->stWiper.nSpeedInput];

      for(int i=0; i<PDM_NUM_WIPER_SPEED_MAP; i++)
        pWiper->eSpeedMap[i] = (WiperSpeed_t)pConfig->stWiper.nSpeedMap[i];
    }
    stMsgTx.stTxHeader.DLC = 7;

    stMsgTx.nTxData[0] = MSG_TX_SET_WIPER_SPEED;
    stMsgTx.nTxData[1] = pConfig->stWiper.nSwipeInput;
    stMsgTx.nTxData[2] = pConfig->stWiper.nSpeedInput;
    stMsgTx.nTxData[3] = ((pConfig->stWiper.nSpeedMap[1] & 0x0F) << 4) + (pConfig->stWiper.nSpeedMap[0] & 0x0F);
    stMsgTx.nTxData[4] = ((pConfig->stWiper.nSpeedMap[3] & 0x0F) << 4) + (pConfig->stWiper.nSpeedMap[2] & 0x0F);
    stMsgTx.nTxData[5] = ((pConfig->stWiper.nSpeedMap[5] & 0x0F) << 4) + (pConfig->stWiper.nSpeedMap[4] & 0x0F);
    stMsgTx.nTxData[6] = ((pConfig->stWiper.nSpeedMap[7] & 0x0F) << 4) + (pConfig->stWiper.nSpeedMap[6] & 0x0F);
    stMsgTx.nTxData[7] = 0;

    stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
    osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
  }
}

void SetWiperDelay(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], Wiper_t* pWiper, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;

  if( (stMsgRx->nRxLen == 7) ||
      (stMsgRx->nRxLen == 1)){
    if(stMsgRx->nRxLen == 7){
      pConfig->stWiper.nIntermitTime[0] = stMsgRx->nRxData[1] * 100;
      pConfig->stWiper.nIntermitTime[1] = stMsgRx->nRxData[2] * 100;
      pConfig->stWiper.nIntermitTime[2] = stMsgRx->nRxData[3] * 100;
      pConfig->stWiper.nIntermitTime[3] = stMsgRx->nRxData[4] * 100;
      pConfig->stWiper.nIntermitTime[4] = stMsgRx->nRxData[5] * 100;
      pConfig->stWiper.nIntermitTime[5] = stMsgRx->nRxData[6] * 100;

      for(int i=0; i<PDM_NUM_WIPER_INTER_DELAYS; i++)
        pWiper->nInterDelays[i] = pConfig->stWiper.nIntermitTime[i];
    }
    stMsgTx.stTxHeader.DLC = 7;

    stMsgTx.nTxData[0] = MSG_TX_SET_WIPER_DELAYS;
    stMsgTx.nTxData[1] = (uint8_t)(pConfig->stWiper.nIntermitTime[0] / 100);
    stMsgTx.nTxData[2] = (uint8_t)(pConfig->stWiper.nIntermitTime[1] / 100);
    stMsgTx.nTxData[3] = (uint8_t)(pConfig->stWiper.nIntermitTime[2] / 100);
    stMsgTx.nTxData[4] = (uint8_t)(pConfig->stWiper.nIntermitTime[3] / 100);
    stMsgTx.nTxData[5] = (uint8_t)(pConfig->stWiper.nIntermitTime[4] / 100);
    stMsgTx.nTxData[6] = (uint8_t)(pConfig->stWiper.nIntermitTime[5] / 100);
    stMsgTx.nTxData[7] = 0;

    stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
    osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
  }
}

void SetFlashers(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;
  uint8_t nIndex;

  if( (stMsgRx->nRxLen == 6) ||
      (stMsgRx->nRxLen == 2)){
    nIndex = (stMsgRx->nRxData[1] & 0xF0) >> 4;
    if(nIndex < PDM_NUM_FLASHERS){
      if(stMsgRx->nRxLen == 6){
        pConfig->stFlasher[nIndex].nEnabled = (stMsgRx->nRxData[1] & 0x01);
        pConfig->stFlasher[nIndex].nSingleCycle = (stMsgRx->nRxData[1] & 0x02) >> 1;
        pConfig->stFlasher[nIndex].nInput = stMsgRx->nRxData[2];

        pConfig->stFlasher[nIndex].nFlashOnTime = stMsgRx->nRxData[4] * 100;
        pConfig->stFlasher[nIndex].nFlashOffTime = stMsgRx->nRxData[5] * 100;
        pConfig->stFlasher[nIndex].pInput = pVariableMap[pConfig->stFlasher[nIndex].nInput];
      }
      stMsgTx.stTxHeader.DLC = 6;

      stMsgTx.nTxData[0] = MSG_TX_SET_FLASHER;
      stMsgTx.nTxData[1] = ((nIndex & 0x0F) << 4) + ((pConfig->stFlasher[nIndex].nSingleCycle & 0x01) << 1) +
                              (pConfig->stFlasher[nIndex].nEnabled & 0x01);
      stMsgTx.nTxData[2] = pConfig->stFlasher[nIndex].nInput;

      stMsgTx.nTxData[4] = (uint8_t)(pConfig->stFlasher[nIndex].nFlashOnTime / 100);
      stMsgTx.nTxData[5] = (uint8_t)(pConfig->stFlasher[nIndex].nFlashOffTime / 100);
      stMsgTx.nTxData[6] = 0;
      stMsgTx.nTxData[7] = 0;

      stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
      osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
    }
  }
}

void SetStarter(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;
  
  if( (stMsgRx->nRxLen == 4) ||
      (stMsgRx->nRxLen == 1)){
    if(stMsgRx->nRxLen == 4){
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
      pConfig->stStarter.pInput = pVariableMap[pConfig->stStarter.nInput];
    }

    stMsgTx.stTxHeader.DLC = 4;

    stMsgTx.nTxData[0] = MSG_TX_SET_STARTER;
    stMsgTx.nTxData[1] = (pConfig->stStarter.nEnabled & 0x01);
    stMsgTx.nTxData[2] = pConfig->stStarter.nInput;
    stMsgTx.nTxData[3] = ((pConfig->stStarter.nDisableOut[7] & 0x01) << 7) + ((pConfig->stStarter.nDisableOut[6] & 0x01) << 6) +
                            ((pConfig->stStarter.nDisableOut[5] & 0x01) << 5) + ((pConfig->stStarter.nDisableOut[4] & 0x01) << 4) +
                            ((pConfig->stStarter.nDisableOut[3] & 0x01) << 3) + ((pConfig->stStarter.nDisableOut[2] & 0x01) << 2) +
                            ((pConfig->stStarter.nDisableOut[1] & 0x01) << 1) + (pConfig->stStarter.nDisableOut[0] & 0x01);
    stMsgTx.nTxData[4] = 0;
    stMsgTx.nTxData[5] = 0;
    stMsgTx.nTxData[6] = 0;
    stMsgTx.nTxData[7] = 0;

    stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
    osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
  }
}

void SetCanInputs(PdmConfig_t* pConfig, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;
  uint8_t nIndex;

  if( (stMsgRx->nRxLen == 7) ||
      (stMsgRx->nRxLen == 2)){

    if (stMsgRx->nRxLen == 7)
      nIndex = stMsgRx->nRxData[2];
    else
      nIndex = stMsgRx->nRxData[1];

    if(nIndex < PDM_NUM_CAN_INPUTS){
      if(stMsgRx->nRxLen == 7){
        pConfig->stCanInput[nIndex].nEnabled = (stMsgRx->nRxData[1] & 0x01);
        pConfig->stCanInput[nIndex].eMode = (stMsgRx->nRxData[1] & 0x06) >> 1;
        pConfig->stCanInput[nIndex].eOperator = (stMsgRx->nRxData[1] & 0xF0) >> 4;

        pConfig->stCanInput[nIndex].nId = (stMsgRx->nRxData[3] << 8) + stMsgRx->nRxData[4];

        pConfig->stCanInput[nIndex].nLowByte = (stMsgRx->nRxData[5] & 0x0F);
        pConfig->stCanInput[nIndex].nHighByte = (stMsgRx->nRxData[5] & 0xF0) >> 4;

        pConfig->stCanInput[nIndex].nOnVal = stMsgRx->nRxData[6];
      }

      stMsgTx.stTxHeader.DLC = 7;

      stMsgTx.nTxData[0] = MSG_TX_SET_CAN_INPUTS;
      stMsgTx.nTxData[1] = ((pConfig->stCanInput[nIndex].eOperator & 0x0F) << 4) + ((pConfig->stCanInput[nIndex].eMode & 0x03) << 1) +
                              (pConfig->stCanInput[nIndex].nEnabled & 0x01);
      stMsgTx.nTxData[2] = nIndex;
      stMsgTx.nTxData[3] = (uint8_t)(pConfig->stCanInput[nIndex].nId >> 8);
      stMsgTx.nTxData[4] = (uint8_t)(pConfig->stCanInput[nIndex].nId & 0xFF);
      stMsgTx.nTxData[5] = ((pConfig->stCanInput[nIndex].nHighByte & 0xF) << 4) + (pConfig->stCanInput[nIndex].nLowByte & 0xF);
      stMsgTx.nTxData[6] = (uint8_t)(pConfig->stCanInput[nIndex].nOnVal);

      stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
      osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
    }
  }
}

void GetVersion(PdmConfig_t* pConfig, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx)
{
  MsgQueueTx_t stMsgTx;

  if(stMsgRx->nRxLen == 1){
    stMsgTx.stTxHeader.DLC = 5;

    stMsgTx.nTxData[0] = MSG_TX_GET_VERSION;
    stMsgTx.nTxData[1] = (uint8_t)PDM_MAJOR_VERSION;
    stMsgTx.nTxData[2] = (uint8_t)PDM_MINOR_VERSION;
    stMsgTx.nTxData[3] = (uint8_t)(PDM_BUILD >> 8);
    stMsgTx.nTxData[4] = (uint8_t)(PDM_BUILD & 0xFF);
    stMsgTx.nTxData[5] = 0;
    stMsgTx.nTxData[6] = 0;
    stMsgTx.nTxData[7] = 0;

    stMsgTx.stTxHeader.StdId = pConfig->stCanOutput.nBaseId + CAN_TX_SETTING_ID_OFFSET;
    osMessageQueuePut(*qMsgQueueTx, &stMsgTx, 0U, 0U);
  }
}

void PdmConfig_SetDefault(PdmConfig_t* pConfig){
  //Device Configuration
  pConfig->stDevConfig.nVersion = 3;
  pConfig->stDevConfig.nCanEnabled = 1;
  pConfig->stDevConfig.nCanSpeed = 1;

  //Inputs
  for(int i=0; i<PDM_NUM_INPUTS; i++){
    pConfig->stInput[i].nEnabled = 0;
    pConfig->stInput[i].eMode = MODE_MOMENTARY;
    pConfig->stInput[i].bInvert = true;
    pConfig->stInput[i].nDebounceTime = 20;
    pConfig->stInput[i].ePull = PULLUP;
  }

  //Outputs
  for(int i=0; i<PDM_NUM_OUTPUTS; i++){
    pConfig->stOutput[i].nEnabled = 0;
    pConfig->stOutput[i].nInput = 0;
    pConfig->stOutput[i].nCurrentLimit = 20;
    pConfig->stOutput[i].nInrushLimit = 30;
    pConfig->stOutput[i].nInrushTime = 1000;
    pConfig->stOutput[i].eResetMode = RESET_ENDLESS;
    pConfig->stOutput[i].nResetTime = 1000;
    pConfig->stOutput[i].nResetLimit = 0;
  }

  //Virtual Inputs
  for(int i=0; i<PDM_NUM_VIRT_INPUTS; i++){
    pConfig->stVirtualInput[i].nEnabled = 0;
    pConfig->stVirtualInput[i].nNot0 = 0;
    pConfig->stVirtualInput[i].nVar0 = 0;
    pConfig->stVirtualInput[i].eCond0 = COND_AND;
    pConfig->stVirtualInput[i].nNot1 = 0;
    pConfig->stVirtualInput[i].nVar1 = 0;
    pConfig->stVirtualInput[i].eCond1 = COND_OR;
    pConfig->stVirtualInput[i].nNot2 = 0;
    pConfig->stVirtualInput[i].nVar2 = 0;
    pConfig->stVirtualInput[i].eMode = MODE_MOMENTARY;
  }

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
  for(int i=0; i<PDM_NUM_FLASHERS; i++){
    pConfig->stFlasher[i].nEnabled = 0;
    pConfig->stFlasher[i].nInput = 0;
    pConfig->stFlasher[i].nFlashOnTime = 500;
    pConfig->stFlasher[i].nFlashOffTime = 500;
    pConfig->stFlasher[i].nSingleCycle = 0;
  }

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

  //CAN Input
  for(int i=0; i<PDM_NUM_CAN_INPUTS; i++){
    pConfig->stCanInput[i].nEnabled = 0;
    pConfig->stCanInput[i].nId = 0;
    pConfig->stCanInput[i].nLowByte = 0;
    pConfig->stCanInput[i].nHighByte = 0;
    pConfig->stCanInput[i].nOnVal = 0;
    pConfig->stCanInput[i].eMode = MODE_MOMENTARY;
    pConfig->stCanInput[i].eOperator = OPER_BITWISE_AND;
  }

  //CAN Output
  pConfig->stCanOutput.nEnabled = 1;
  pConfig->stCanOutput.nBaseId = 2000;
  pConfig->stCanOutput.nUpdateTime = 50;
}

/*
0   None
1   PDM In 1
2   PDM In 2
3   CAN In 1
4   CAN In 2
5   CAN In 3
6   CAN In 4
7   CAN In 5
8   CAN In 6
9   CAN In 7
10  CAN In 8
11  CAN In 9
12  CAN In 10
13  CAN In 11
14  CAN In 12
15  CAN In 13
16  CAN In 14
17  CAN In 15
18  CAN In 16
19  CAN In 17
20  CAN In 18
21  CAN In 19
22  CAN In 20
23  CAN In 21
24  CAN In 22
25  CAN In 23
26  CAN In 24
27  CAN In 25
28  CAN In 26
29  CAN In 27
30  CAN In 28
31  CAN In 29
32  CAN In 30
33  CAN In 31
34  CAN In 32
35  Virtual In 1
36  Virtual In 2
37  Virtual In 3
38  Virtual In 4
39  Virtual In 5
40  Virtual In 6
41  Virtual In 7
42  Virtual In 8
43  Virtual In 9
44  Virtual In 10
45  Virtual In 11
46  Virtual In 12
47  Virtual In 13
48  Virtual In 14
49  Virtual In 15
50  Virtual In 16
51  Output 1
52  Output 2
53  Output 3
54  Output 4
55  Output 5
56  Output 6
57  Output 7
58  Output 8
59  Wiper Slow Out
60  Wiper Fast Out
61  Always On
 */
