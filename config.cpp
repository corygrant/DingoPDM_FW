#include "config.h"
#include "error.h"
#include "crc.h"

MB85RC fram(I2CD1, MB85RC_I2CADDR_DEFAULT);

bool ReadConfig(){

    // Read config
    if(!fram.Read(0x0, (uint8_t*)&stConfig, sizeof(stConfig))) {
        return false;
    }
    
    // Check version number
    if(stConfig.stDevConfig.nConfigVersion != CONFIG_VERSION) {
        return false;
    }
    
    // Read stored data CRC
    uint32_t storedCrc = 0;
    if(!fram.Read(sizeof(stConfig), (uint8_t*)&storedCrc, sizeof(storedCrc))) {
        return false;
    }
    
    // Calculate CRC of the data we just read
    uint32_t calculatedCrc = CalculateCRC32(&stConfig, sizeof(stConfig));
    
    // Compare CRCs to verify data integrity
    if(storedCrc != calculatedCrc) {
        return false; // Data corrupt or changed
    }
    
    return true;
}

bool WriteConfig(){
    if(!fram.CheckId()) {
        return false;
    }
    
    // Make sure the version is current
    stConfig.stDevConfig.nConfigVersion = CONFIG_VERSION;
    
    // Write config
    if(!fram.Write(0x0, (uint8_t*)&stConfig, sizeof(stConfig))) {
        return false;
    }
    
    // Calculate config CRC
    uint32_t dataCrc = CalculateCRC32(&stConfig, sizeof(stConfig));
    
    // Write CRC after config
    if(!fram.Write(sizeof(stConfig), (uint8_t*)&dataCrc, sizeof(dataCrc))) {
        return false;
    }
    
    return true;
}

void SetDefaultConfig()
{
    stConfig.stDevConfig.nConfigVersion = CONFIG_VERSION;
    stConfig.stDevConfig.eCanSpeed = CanBitrate::Bitrate_500K;
    stConfig.stDevConfig.bCanFilterEnabled = false;
    stConfig.stDevConfig.bSleepEnabled = true;

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
        stConfig.stVirtualInput[i].eCond0 = BoolOperator::And;
        stConfig.stVirtualInput[i].bNot1 = false;
        stConfig.stVirtualInput[i].nVar1 = 0;
        stConfig.stVirtualInput[i].eCond1 = BoolOperator::And;
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
        stConfig.stCanInput[i].bTimeoutEnabled = true;
        stConfig.stCanInput[i].nTimeout = 20;
        stConfig.stCanInput[i].nIDE = 0;
        stConfig.stCanInput[i].nSID = 0;
        stConfig.stCanInput[i].nEID = 0;
        stConfig.stCanInput[i].nDLC = 0;
        stConfig.stCanInput[i].nStartingByte = 0;
        stConfig.stCanInput[i].eOperator = Operator::Equal;
        stConfig.stCanInput[i].nOnVal = 0;
        stConfig.stCanInput[i].eMode = InputMode::Momentary;
    }

    stConfig.stCanOutput.nBaseId = 2000;

    for(uint8_t i=0; i < PDM_NUM_COUNTERS; i++)
    {
        stConfig.stCounter[i].bEnabled = false;
        stConfig.stCounter[i].nIncInput = 0;
        stConfig.stCounter[i].nDecInput = 0;
        stConfig.stCounter[i].nResetInput = 0;
        stConfig.stCounter[i].nMaxCount = 4;
        stConfig.stCounter[i].nMinCount = 0;
        stConfig.stCounter[i].bWrapAround = false;
    }

    for(uint8_t i = 0; i < PDM_NUM_CONDITIONS; i++)
    {
        stConfig.stCondition[i].bEnabled = false;
        stConfig.stCondition[i].eOperator = Operator::Equal;
        stConfig.stCondition[i].nInput = 0;
        stConfig.stCondition[i].nArg = 0;
    }

    for(uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
    {
        stConfig.stKeypad[i].bEnabled = false;
        stConfig.stKeypad[i].nNodeId = 0x15;
        stConfig.stKeypad[i].bTimeoutEnabled = false;
        stConfig.stKeypad[i].nTimeout = 20;
        stConfig.stKeypad[i].eModel = KeypadModel::Blink12Key;
        stConfig.stKeypad[i].nBacklightBrightness = 63;
        stConfig.stKeypad[i].nDimBacklightBrightness = 31;
        stConfig.stKeypad[i].nBacklightColor = (uint8_t)BlinkMarineBacklightColor::White;
        stConfig.stKeypad[i].nDimmingVar = 0;
        stConfig.stKeypad[i].nButtonBrightness = 63;
        stConfig.stKeypad[i].nDimButtonBrightness = 31;
        
        for (uint8_t j = 0; j < KEYPAD_MAX_BUTTONS ; j++)
        {
            stConfig.stKeypad[i].stButton[j].bEnabled = false;
            stConfig.stKeypad[i].stButton[j].eMode = InputMode::Momentary;
            stConfig.stKeypad[i].stButton[j].nNumOfValColors = 4;
            stConfig.stKeypad[i].stButton[j].nValColors[0] = (uint8_t)BlinkMarineButtonColor::Off;
            stConfig.stKeypad[i].stButton[j].nValColors[1] = (uint8_t)BlinkMarineButtonColor::Green;
            stConfig.stKeypad[i].stButton[j].nValColors[2] = (uint8_t)BlinkMarineButtonColor::Violet;
            stConfig.stKeypad[i].stButton[j].nValColors[3] = (uint8_t)BlinkMarineButtonColor::Blue;
            stConfig.stKeypad[i].stButton[j].nFaultColor = (uint8_t)BlinkMarineButtonColor::Red;
            stConfig.stKeypad[i].stButton[j].nVar = 0;
            stConfig.stKeypad[i].stButton[j].nFaultVar = 0;
            stConfig.stKeypad[i].stButton[j].bValBlinking[0] = false;
            stConfig.stKeypad[i].stButton[j].bValBlinking[1] = false;
            stConfig.stKeypad[i].stButton[j].bValBlinking[2] = false;
            stConfig.stKeypad[i].stButton[j].bValBlinking[3] = false;
            stConfig.stKeypad[i].stButton[j].bFaultBlinking = false;
            stConfig.stKeypad[i].stButton[j].nValBlinkingColors[0] = (uint8_t)BlinkMarineButtonColor::Blue;
            stConfig.stKeypad[i].stButton[j].nValBlinkingColors[1] = (uint8_t)BlinkMarineButtonColor::Violet;
            stConfig.stKeypad[i].stButton[j].nValBlinkingColors[2] = (uint8_t)BlinkMarineButtonColor::Green;
            stConfig.stKeypad[i].stButton[j].nValBlinkingColors[3] = (uint8_t)BlinkMarineButtonColor::White;
            stConfig.stKeypad[i].stButton[j].nFaultBlinkingColor = (uint8_t)BlinkMarineButtonColor::Orange;
        }

        for (uint8_t j = 0; j < KEYPAD_MAX_DIALS ; j++)
        {
            stConfig.stKeypad[i].stDial[j].nDialMinLed = 0;
            stConfig.stKeypad[i].stDial[j].nDialMaxLed = 0;
            stConfig.stKeypad[i].stDial[j].nDialLedOffset = 0;
        }
    }
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
        if(!WriteConfig()){
            //Couldn't write default config
            //FRAM issue 
            Error::SetFatalError(FatalErrorType::ErrFRAM, MsgSrc::Config);
        }
        else
        {
            //Wrote default config
            //Error to force power cycle
            Error::SetFatalError(FatalErrorType::ErrConfig, MsgSrc::Config);
        }
    }

}