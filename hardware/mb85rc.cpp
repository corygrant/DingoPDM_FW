#include "mb85rc.h"

bool MB85RC::CheckId()
{
    uint16_t nManufId;
    uint16_t nProdId;

    GetId(&nManufId, &nProdId);

    if (nManufId != MB85RC_MANUF_ID)
        return false;
    if (nProdId != MB85RC_PROD_ID)
        return false;

    return true;
}

bool MB85RC::GetId(uint16_t *nManufId, uint16_t *nProdId)
{
    //Getting device ID is a bit weird with the MB85RC
    //To get the device ID the command 0xF8 is sent as the address
    //Note: This has to be shifted to the right by 1 to get the 7-bit address
    //The actual i2c address has to be sent as the first data byte (left shifted by 1 to get i2c address)
    //The device will then respond with 3 bytes representing the manufacturer ID and product ID
    //The manufacturer ID is the first 12 bits and the product ID is the last 4 bits of the second byte and the third byte
    //fantastic

    msg_t status;

    uint8_t tx[1];
    uint8_t rx[3];

    tx[0] = (m_addr & 0xFF) << 1;

    i2cAcquireBus(m_driver);

    status = i2cMasterTransmitTimeout(  m_driver,
                                        MB85RC_SLAVE_ID >> 1,
                                        tx,
                                        1,
                                        rx,
                                        sizeof(rx),
                                        TIME_MS2I(MB85RC_TIMEOUT));

    i2cReleaseBus(m_driver);

    if (status != MSG_OK) {
       lastErrors = i2cGetErrors(&I2CD1);
       return false;
    }

    *nManufId = (rx[0] << 4) + (rx[1] >> 4);
    *nProdId = ((rx[1] & 0x0F) << 8) + rx[2];
    return true;
}

bool MB85RC::Read(uint16_t nMemAddr, uint8_t *pData, uint16_t nByteLen)
{
    msg_t status;

    i2cAcquireBus(m_driver);

    status = i2cMasterTransmitTimeout(  m_driver,
                                        m_addr,
                                        reinterpret_cast<const uint8_t*>(&nMemAddr),
                                        2,
                                        pData,
                                        nByteLen,
                                        TIME_MS2I(MB85RC_TIMEOUT));

    i2cReleaseBus(m_driver);

    if (status != MSG_OK) {
       lastErrors = i2cGetErrors(&I2CD1);
       return false;
    }

    return true;
}

bool MB85RC::Write(uint16_t nMemAddr, uint8_t *nMemVals, uint16_t nByteLen)
{   
    msg_t status;
    uint8_t addrBytes[2] = {static_cast<uint8_t>(nMemAddr & 0xFF), static_cast<uint8_t>(nMemAddr >> 8)};

    i2cAcquireBus(m_driver);

    // First send the address
    status = i2cMasterTransmitTimeout(m_driver,
                                     m_addr,
                                     addrBytes,
                                     2,
                                     NULL,
                                     0,
                                     TIME_MS2I(MB85RC_TIMEOUT));

    if (status == MSG_OK) {
        // Then send the data without releasing the bus
        status = i2cMasterTransmitTimeout(m_driver,
                                         m_addr,
                                         nMemVals,
                                         nByteLen,
                                         NULL,
                                         0,
                                         TIME_MS2I(MB85RC_TIMEOUT));
    }

    i2cReleaseBus(m_driver);

    if (status != MSG_OK) {
       lastErrors = i2cGetErrors(&I2CD1);
       return false;
    }
    return true;
}
