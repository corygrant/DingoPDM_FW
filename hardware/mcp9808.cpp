#include "mcp9808.h"

bool MCP9808::Init(float upperLimit, float critLimit)
{
    if (!CheckId())
        return false;

    if (!SetResolution(MCP9808_RESOLUTION_0_5DEG))
        return false;

    if (!SetLimit(MCP9808_REG_CRIT_TEMP, critLimit))
        return false;

    if (!SetLimit(MCP9808_REG_UPPER_TEMP, upperLimit))
        return false;

    if (!SetLimit(MCP9808_REG_LOWER_TEMP, 0.0))
        return false;

    if (!LockLimits())
        return false;

    return true;
}

bool MCP9808::CheckId()
{
    uint16_t nManufId;
    uint16_t nDeviceId;

    if (!Read16(MCP9808_REG_MANUF_ID, &nManufId))
        return false;

    if (!Read16(MCP9808_REG_DEVICE_ID, &nDeviceId))
        return false;

    if (nManufId != MCP9808_MANUF_ID)
        return false;
    if (nDeviceId != MCP9808_DEVICE_ID)
        return false;

    return true;
}

float MCP9808::GetTemp()
{
    float temp = 0.0;
    uint16_t t;
    GetTempRegister(&t);

    temp = t & 0x0FFF; // Remove limit and sign bits
    temp /= 16.0;      // Remove fractional bits
    if (t & 0x1000)    // Check sign bit
        temp -= 256;

    return temp;
}

int16_t MCP9808::GetTempInt()
{
    return (int16_t)GetTemp();
}

uint16_t MCP9808::GetTempUint()
{
    uint16_t t;
    GetTempRegister(&t);

    t = t & 0x0FFF; // Remove limit and sign bits
    t = t >> 4;     // Remove fractional bits

    return t;
}

bool MCP9808::GetTempRegister(uint16_t *temp)
{
    return Read16(MCP9808_REG_AMBIENT_TEMP, temp);
}

bool MCP9808::GetResolution(uint8_t *res)
{
    return Read8(MCP9808_REG_RESOLUTION, res);
}

bool MCP9808::SetResolution(uint8_t resolution)
{
    return Write8(MCP9808_REG_RESOLUTION, resolution & 0x03);
}

bool MCP9808::LockLimits()
{
    // Set Alert pin function
    // B0 = 0 (comparator output)
    // B1 = 0 (active low)
    // B2 = 1 (alert output for Tcrit only)
    // B3 = 1 (alert output enabled)
    // B4 = 0 (alert output asserted - status output - not a setting)
    // B5 = 0 (interrupt clear - no effect)
    // B6 = 1 (Tupper Tlower window locked)
    // B7 = 1 (Tcrit locked)
    // B8 = 0 (continous conversion)
    // B9-10 = 01 (Tupper Tlower hysterisis +1.5 deg C)
    // B11-15 = 00000 (not used)
    return Write16(MCP9808_REG_CONFIG, (MCP9808_REG_CONFIG_ALERTSEL |
                                        MCP9808_REG_CONFIG_ALERTCTRL |
                                        MCP9808_REG_CONFIG_WINLOCKED |
                                        MCP9808_REG_CONFIG_CRITLOCKED |
                                        MCP9808_REG_CONFIG_HYST_1_5));
}

bool MCP9808::Shutdown()
{
    uint16_t config;
    if (!Read16(MCP9808_REG_CONFIG, &config))
        return false;

    return Write16(MCP9808_REG_CONFIG, config | MCP9808_REG_CONFIG_SHUTDOWN);
}

bool MCP9808::Wake()
{
    uint16_t config;
    if (!Read16(MCP9808_REG_CONFIG, &config))
        return false;

    return Write16(MCP9808_REG_CONFIG, config & ~MCP9808_REG_CONFIG_SHUTDOWN);
}

bool MCP9808::SetLimit(uint8_t reg, float val)
{
    if(!Write16(reg, val * 16.0))
        return false;

    uint16_t raw;
    if(!Read16(reg, &raw))
        return false;

    if (val != RawToTemp(raw))  
        return false;
    
    return true;
}

float MCP9808::RawToTemp(uint16_t raw)
{
    float temp = 0.0;
    if (raw != 0xFFFF)
    {
        temp = raw & 0x0FFF;
        temp /= 16.0;
        if (raw & 0x1000)
            temp -= 256;
    }
    return temp;
}

bool MCP9808::CritTempLimit()
{
    uint16_t temp;
    GetTempRegister(&temp);
    return ((temp & MCP9808_REG_CRITICALTEMP) >> MCP9808_POS_CRITICALTEMP);
}

bool MCP9808::OverTempLimit()
{
    uint16_t temp;
    GetTempRegister(&temp);
    return ((temp & MCP9808_REG_OVERTEMP) >> MCP9808_POS_OVERTEMP);
}

bool MCP9808::UnderTempLimit()
{
    uint16_t temp;
    GetTempRegister(&temp);
    return ((temp & MCP9808_REG_UNDERTEMP) >> MCP9808_POS_UNDERTEMP);
}

bool MCP9808::Write16(uint8_t reg, uint16_t val)
{
    msg_t status;

    uint8_t tx[3];

    tx[0] = reg;
    tx[1] = val >> 8;
    tx[2] = val & 0xFF;

    i2cAcquireBus(m_driver);

    status = i2cMasterTransmitTimeout(m_driver,
                                      m_addr,
                                      tx,
                                      sizeof(tx),
                                      NULL,
                                      0,
                                      TIME_MS2I(MCP9808_TIMEOUT));

    i2cReleaseBus(m_driver);

    if (status != MSG_OK)
    {
        lastErrors = i2cGetErrors(&I2CD1);
        return false;
        /* TODO use i2c error*/
    }

    return true;
}

bool MCP9808::Read16(uint8_t reg, uint16_t *val)
{
    msg_t status;

    uint8_t tx[1];
    uint8_t rx[2];

    tx[0] = reg;

    i2cAcquireBus(m_driver);

    status = i2cMasterTransmitTimeout(m_driver,
                                      m_addr,
                                      tx,
                                      sizeof(tx),
                                      rx,
                                      sizeof(rx),
                                      TIME_MS2I(MCP9808_TIMEOUT));

    i2cReleaseBus(m_driver);

    if (status != MSG_OK)
    {
        lastErrors = i2cGetErrors(&I2CD1);
        return false;
        /* TODO use i2c error*/
    }

    *val = (rx[0] << 8 | rx[1]);

    return true;
}

bool MCP9808::Write8(uint8_t reg, uint8_t val)
{
    msg_t status;

    uint8_t tx[2];

    tx[0] = reg;
    tx[1] = val;

    i2cAcquireBus(m_driver);

    status = i2cMasterTransmitTimeout(m_driver,
                                      m_addr,
                                      tx,
                                      sizeof(tx),
                                      NULL,
                                      0,
                                      TIME_MS2I(MCP9808_TIMEOUT));

    i2cReleaseBus(m_driver);

    if (status != MSG_OK)
    {
        lastErrors = i2cGetErrors(&I2CD1);
        return false;
        /* TODO use i2c error*/
    }

    return true;
}

bool MCP9808::Read8(uint8_t reg, uint8_t *val)
{
    msg_t status;

    uint8_t tx[1];
    uint8_t rx[1];

    tx[0] = reg;

    i2cAcquireBus(m_driver);

    status = i2cMasterTransmitTimeout(m_driver,
                                      m_addr,
                                      tx,
                                      sizeof(tx),
                                      rx,
                                      sizeof(rx),
                                      TIME_MS2I(MCP9808_TIMEOUT));

    i2cReleaseBus(m_driver);

    if (status != MSG_OK)
    {
        lastErrors = i2cGetErrors(&I2CD1);
        return false;
        /* TODO use i2c error*/
    }

    *val = rx[0];

    return true;
}

float DegCToF(float degC)
{
    return degC * 9.0 / 5.0 + 32;
}