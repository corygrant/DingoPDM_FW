#include "config.h"
#include "error.h"

MB85RC fram(I2CD1, MB85RC_I2CADDR_DEFAULT);

bool ReadConfig(){

    uint16_t nSizeInFram = 0;

    //Get size from 2 bytes after config
    if(!fram.Read(sizeof(stConfig), (uint8_t*)&nSizeInFram, sizeof(nSizeInFram))){
        return false;
    }

    if(sizeof(stConfig) != nSizeInFram){
        return false;
    }

    if(!fram.Read(0x0, (uint8_t*)&stConfig, sizeof(stConfig))){
        return false;
    }

    return true;
}

bool WriteConfig(){

    if(!fram.CheckId()){
        return false;
    }

    if(!fram.Write(0x0, (uint8_t*)&stConfig, sizeof(stConfig))){
        return false;
    }

    uint16_t nSize = sizeof(stConfig);

    if(!fram.Write(sizeof(stConfig), (uint8_t*)&nSize, sizeof(nSize))){
        return false;
    }

    return true;
}

void SetDefaultConfig()
{
    stConfig.stDevConfig.eCanSpeed = CanBitrate::Bitrate_500K;

    for(uint8_t i = 0; i < PDM_NUM_INPUTS; i++)
    {
        stConfig.stInput[i].bEnabled = false;
        stConfig.stInput[i].eMode = InputMode::Momentary;
        stConfig.stInput[i].bInvert = true;
        stConfig.stInput[i].nDebounceTime = 20;
        stConfig.stInput[i].ePull = InputPull::Up;
    }

    for(uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
    {
        stConfig.stVirtualInput[i].bEnabled = false;
        stConfig.stVirtualInput[i].bNot0 = false;
        stConfig.stVirtualInput[i].nVar0 = 0;
        stConfig.stVirtualInput[i].eCond0 = Condition::And;
        stConfig.stVirtualInput[i].bNot1 = false;
        stConfig.stVirtualInput[i].nVar1 = 0;
        stConfig.stVirtualInput[i].eCond1 = Condition::And;
        stConfig.stVirtualInput[i].bNot2 = false;
        stConfig.stVirtualInput[i].nVar2 = 0;
        stConfig.stVirtualInput[i].eMode = InputMode::Momentary;
    }

    for(uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        stConfig.stOutput[i].bEnabled = false;
        stConfig.stOutput[i].nInput = 0;
        stConfig.stOutput[i].nCurrentLimit = 20;
        stConfig.stOutput[i].nInrushLimit = 30;
        stConfig.stOutput[i].nInrushTime = 1000;
        stConfig.stOutput[i].eResetMode = ProfetResetMode::None;
        stConfig.stOutput[i].nResetTime = 1000;
        stConfig.stOutput[i].nResetLimit = 0;
    }

    stConfig.stWiper.bEnabled = false;
    stConfig.stWiper.eMode = WiperMode::DigIn;
    stConfig.stWiper.nSlowInput = 0;
    stConfig.stWiper.nFastInput = 0;
    stConfig.stWiper.nInterInput = 0;
    stConfig.stWiper.nOnInput = 0;
    stConfig.stWiper.nSpeedInput = 0;
    stConfig.stWiper.nParkInput = 0;
    stConfig.stWiper.bParkStopLevel = false;
    stConfig.stWiper.nSwipeInput = 0;
    stConfig.stWiper.nWashInput = 0;
    stConfig.stWiper.nWashWipeCycles = 0;
    stConfig.stWiper.eSpeedMap[0] = WiperSpeed::Intermittent1;
    stConfig.stWiper.eSpeedMap[1] = WiperSpeed::Intermittent2;
    stConfig.stWiper.eSpeedMap[2] = WiperSpeed::Intermittent3;
    stConfig.stWiper.eSpeedMap[3] = WiperSpeed::Intermittent4;  
    stConfig.stWiper.eSpeedMap[4] = WiperSpeed::Intermittent5;
    stConfig.stWiper.eSpeedMap[5] = WiperSpeed::Intermittent6;
    stConfig.stWiper.eSpeedMap[6] = WiperSpeed::Slow;
    stConfig.stWiper.eSpeedMap[7] = WiperSpeed::Fast;
    stConfig.stWiper.nIntermitTime[0] = 1000;
    stConfig.stWiper.nIntermitTime[1] = 2000;
    stConfig.stWiper.nIntermitTime[2] = 3000;
    stConfig.stWiper.nIntermitTime[3] = 4000;
    stConfig.stWiper.nIntermitTime[4] = 5000;
    stConfig.stWiper.nIntermitTime[5] = 6000;

    for(uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
    {
        stConfig.stFlasher[i].bEnabled = false;
        stConfig.stFlasher[i].nInput = 0;
        stConfig.stFlasher[i].nFlashOnTime = 500;
        stConfig.stFlasher[i].nFlashOffTime = 500;
        stConfig.stFlasher[i].bSingleCycle = false;
    }

    stConfig.stStarter.bEnabled = false;
    stConfig.stStarter.nInput = 0;
    for(uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        stConfig.stStarter.bDisableOut[i] = false;
    }

    for(uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
    {
        stConfig.stCanInput[i].bEnabled = false;
        stConfig.stCanInput[i].nSID = 0;
        stConfig.stCanInput[i].nEID = 0;
        stConfig.stCanInput[i].nDLC = 0;
        stConfig.stCanInput[i].nStartingByte = 0;
        stConfig.stCanInput[i].eOperator = Operator::Equal;
        stConfig.stCanInput[i].nOnVal = 0;
        stConfig.stCanInput[i].eMode = InputMode::Momentary;
    }

    stConfig.stCanOutput.bEnabled = true;
    stConfig.stCanOutput.nBaseId = 2000;

}

void InitConfig()
{
    if(!fram.CheckId())
        Error::SetFatalError(FatalErrorType::ErrFRAM, MsgSrc::Config);

    if(!ReadConfig())
    {
        if(fram.GetErrors() != 0)
            Error::SetFatalError(FatalErrorType::ErrFRAM, MsgSrc::Config);
        
        //Write default for next power cycle
        SetDefaultConfig();
        //if(!WriteConfig()){
        //    if(fram.GetErrors() != 0)
        //    {
        //        Error::SetFatalError(FatalErrorType::ErrFRAM, MsgSrc::Config);
        //    }
        //}
        //else{
        //    Error::SetFatalError(FatalErrorType::ErrConfig, MsgSrc::Config);
        //}
    }

}