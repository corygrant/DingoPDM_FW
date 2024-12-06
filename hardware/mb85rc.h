#pragma once

#include <cstdint>
#include "hal.h"

#define MB85RC_I2CADDR_DEFAULT 0x50

#define MB85RC_TIMEOUT 1000 //ms

#define MB85RC_SLAVE_ID 0xF8
#define MB85RC_MANUF_ID 0x00A
#define MB85RC_PROD_ID 0x510

class MB85RC
{
    public:
        MB85RC(I2CDriver& i2cp, i2caddr_t addr)
            : m_driver(&i2cp)
            , m_addr(addr)
        {
        }
        bool CheckId();
        bool GetId(uint16_t* nManufId, uint16_t* nProdId);
        bool Read(uint16_t nMemAddr, uint8_t* pData, uint16_t nByteLen);
        bool Write(uint16_t nMemAddr, uint8_t* nMemVals, uint16_t nByteLen);
        i2cflags_t GetErrors() { return lastErrors; }
        
    private:
        I2CDriver* m_driver;
        const i2caddr_t m_addr;

        i2cflags_t lastErrors;
};