#pragma once

#include <cstdint>
#include "hal.h"

#define MCP9808_TIMEOUT 100 //ms

#define MCP9808_MANUF_ID 0x0054
#define MCP9808_DEVICE_ID 0x0400

#define MCP9808_I2CADDR_DEFAULT 0x18 ///< I2C address
#define MCP9808_REG_CONFIG 0x01      ///< MCP9808 config register

#define MCP9808_REG_CONFIG_HYST_6_0 0x0600
#define MCP9808_REG_CONFIG_HYST_3_0 0x0400
#define MCP9808_REG_CONFIG_HYST_1_5 0x0200
#define MCP9808_REG_CONFIG_SHUTDOWN 0x0100   ///< shutdown config
#define MCP9808_REG_CONFIG_CRITLOCKED 0x0080 ///< critical trip lock
#define MCP9808_REG_CONFIG_WINLOCKED 0x0040  ///< alarm window lock
#define MCP9808_REG_CONFIG_INTCLR 0x0020     ///< interrupt clear
#define MCP9808_REG_CONFIG_ALERTSTAT 0x0010  ///< alert output status
#define MCP9808_REG_CONFIG_ALERTCTRL 0x0008  ///< alert output control
#define MCP9808_REG_CONFIG_ALERTSEL 0x0004   ///< alert output select
#define MCP9808_REG_CONFIG_ALERTPOL 0x0002   ///< alert output polarity
#define MCP9808_REG_CONFIG_ALERTMODE 0x0001  ///< alert output mode

#define MCP9808_REG_UPPER_TEMP 0x02   ///< upper alert boundary
#define MCP9808_REG_LOWER_TEMP 0x03   ///< lower alert boundery
#define MCP9808_REG_CRIT_TEMP 0x04    ///< critical temperature
#define MCP9808_REG_AMBIENT_TEMP 0x05 ///< ambient temperature
#define MCP9808_REG_MANUF_ID 0x06     ///< manufacture ID
#define MCP9808_REG_DEVICE_ID 0x07    ///< device ID
#define MCP9808_REG_RESOLUTION 0x08   ///< resolutin
#define MCP9808_REG_UNDERTEMP         ((uint16_t)0x2000U)
#define MCP9808_REG_OVERTEMP          ((uint16_t)0x4000U)
#define MCP9808_REG_CRITICALTEMP      ((uint16_t)0x8000U)
#define MCP9808_POS_UNDERTEMP 13
#define MCP9808_POS_OVERTEMP 14
#define MCP9808_POS_CRITICALTEMP 15

#define MCP9808_RESOLUTION_0_5DEG 0x0
#define MCP9808_RESOLUTION_0_25DEG 0x1
#define MCP9808_RESOLUTION_0_125DEG 0x2
#define MCP9808_RESOLUTION_0_0625DEG 0x3

class MCP9808
{
    public:
        MCP9808(I2CDriver& i2cp, i2caddr_t addr)
            : m_driver(&i2cp)
            , m_addr(addr)
        {
        }

        bool Init();
        bool CheckId();
        float GetTemp();
        int16_t GetTempInt();
        uint16_t GetTempUint();
        bool GetTempRegister(uint16_t* temp);
        bool GetResolution(uint8_t* res);
        bool SetResolution(uint8_t resolution);
        bool LockLimits();
        bool Shutdown();
        bool Wake();
        bool SetLimit(uint8_t reg, float val);
        float RawToTemp(uint16_t raw);
        bool CritTempLimit();
        bool OverTempLimit();
        bool UnderTempLimit();
        i2cflags_t GetErrors() { return lastErrors; }

    private:
        I2CDriver* m_driver;
        const i2caddr_t m_addr;

        i2cflags_t lastErrors;

        bool Write16(uint8_t reg, uint16_t val);
        bool Read16(uint8_t reg, uint16_t* val);
        bool Write8(uint8_t reg, uint8_t val);
        bool Read8(uint8_t reg, uint8_t* val);
};

float DegCToF(float degC);