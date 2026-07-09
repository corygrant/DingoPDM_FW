#include "config.h"
#include "error.h"
#include "crc.h"
#include "param_protocol.h"
#include "hardware/mb85rc.h"

#if HAS_EXT_MEMORY
MB85RC fram(I2CD1, MB85RC_I2CADDR_DEFAULT);
#endif

bool ReadConfigExt()
{
    #if HAS_EXT_MEMORY
    // Read config
    if(!fram.Read(0x0, (uint8_t*)&stConfig, sizeof(stConfig))) {
        return false;
    }
    
    // Check version number
    if(stConfig.stDevice.nConfigVersion != CONFIG_VERSION) {
        return false;
    }
    
    // Read stored data CRC
    uint32_t storedCrc = 0;
    if(!fram.Read(sizeof(stConfig), (uint8_t*)&storedCrc, sizeof(storedCrc))) {
        return false;
    }
    
    // Calculate CRC of the data we just read
    uint32_t calculatedCrc = CalculateCRC32(&stConfig, sizeof(stConfig), 0xFFFFFFFF);
    
    // Compare CRCs to verify data integrity
    if(storedCrc != calculatedCrc) {
        return false; // Data corrupt or changed
    }
    
    return true;
    #else
    return false;
    #endif
}

bool WriteConfigExt(){
    #if HAS_EXT_MEMORY
    if(!fram.CheckId()) {
        return false;
    }
    
    // Make sure the version is current
    stConfig.stDevice.nConfigVersion = CONFIG_VERSION;
    
    // Write config
    if(!fram.Write(0x0, (uint8_t*)&stConfig, sizeof(stConfig))) {
        return false;
    }
    
    // Calculate config CRC
    uint32_t dataCrc = CalculateCRC32(&stConfig, sizeof(stConfig), 0xFFFFFFFF);
    
    // Write CRC after config
    if(!fram.Write(sizeof(stConfig), (uint8_t*)&dataCrc, sizeof(dataCrc))) {
        return false;
    }
    
    return true;
    #else
    return false;
    #endif
}

void InitConfigExt()
{
    #if HAS_EXT_MEMORY
    if(!fram.CheckId())
        Error::SetFatalError(FatalErrorType::ErrFRAM, MsgSrc::Config);

    if(!ReadConfigExt())
    {
        if(fram.GetErrors() != 0)
            Error::SetFatalError(FatalErrorType::ErrFRAM, MsgSrc::Config);
        
        //Write default for next power cycle
        SetAllDefaultParams();
        if(!WriteConfigExt()){
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
    #endif
}