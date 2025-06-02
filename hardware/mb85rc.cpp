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
    uint8_t addrBytes[2] = {static_cast<uint8_t>(nMemAddr >> 8), static_cast<uint8_t>(nMemAddr & 0xFF)};

    i2cAcquireBus(m_driver);

    status = i2cMasterTransmitTimeout(  m_driver,
                                        m_addr,
                                        addrBytes,
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
    uint16_t totalSize = 2 + nByteLen;
    
    // Allocate memory dynamically
    uint8_t *txData = (uint8_t*)chHeapAlloc(NULL, totalSize);
    if (txData == NULL) {
        return false; // Allocation failed
    }

    // Set up address bytes (MSB first, LSB second)
    txData[0] = static_cast<uint8_t>(nMemAddr >> 8);
    txData[1] = static_cast<uint8_t>(nMemAddr & 0xFF);

    // Copy data
    for (uint16_t i = 0; i < nByteLen; i++)
    {
        txData[2 + i] = nMemVals[i];
    }

    i2cAcquireBus(m_driver);

    status = i2cMasterTransmitTimeout(m_driver,
                                    m_addr,
                                    txData,
                                    totalSize,
                                    NULL,
                                    0,
                                    TIME_MS2I(MB85RC_TIMEOUT));

    i2cReleaseBus(m_driver);

    // Free the allocated memory
    chHeapFree(txData);

    if (status != MSG_OK) {
       lastErrors = i2cGetErrors(&I2CD1);
       return false;
    }
    return true;
}
