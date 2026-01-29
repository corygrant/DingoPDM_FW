#include "dbc.h"

int32_t Dbc::DecodeInt(const uint8_t *pData, uint8_t nStartBit,
                       uint8_t nBitLength, ByteOrder eByteOrder, bool bSigned)
{
    if (eByteOrder == ByteOrder::BigEndian)
        return DecodeBE(pData, nStartBit, nBitLength, bSigned);
    else
        return DecodeLE(pData, nStartBit, nBitLength, bSigned);
}

void Dbc::EncodeInt(uint8_t *pData, int32_t nRawValue,
                    uint8_t nStartBit, uint8_t nBitLength, ByteOrder eByteOrder)
{
    if (eByteOrder == ByteOrder::BigEndian)
        EncodeBE(pData, nRawValue, nStartBit, nBitLength);
    else
        EncodeLE(pData, nRawValue, nStartBit, nBitLength);
}

int32_t Dbc::DecodeInt(const uint8_t *pData, uint8_t nStartBit,
                       uint8_t nBitLength, float fScale, float fOffset,
                       ByteOrder eByteOrder, bool bSigned)
{
    int32_t rawValue = DecodeInt(pData, nStartBit, nBitLength, eByteOrder, bSigned);
    return (int32_t)(ApplyScaling(rawValue, fScale, fOffset));
}

void Dbc::EncodeInt(uint8_t *pData, int32_t nValue,
                    uint8_t nStartBit, uint8_t nBitLength, float fScale, float fOffset,
                    ByteOrder eByteOrder)
{
    int32_t rawValue = ReverseScaling((float)nValue, fScale, fOffset);
    EncodeInt(pData, rawValue, nStartBit, nBitLength, eByteOrder);
}

int32_t Dbc::DecodeLE(const uint8_t *pData, uint8_t nStartBit,
                      uint8_t nBitLength, bool bSigned)
{
    if (nBitLength == 0 || nBitLength > 32)
        return 0;

    // Copy frame data to 64-bit value for easier bit manipulation
    uint64_t rawData = 0;
    for (int i = 0; i < 8; i++)
    {
        rawData |= ((uint64_t)pData[i]) << (i * 8);
    }

    // Extract bits starting from nStartBit
    uint64_t mask = (1ULL << nBitLength) - 1;
    uint64_t value = (rawData >> nStartBit) & mask;

    // Handle signed values (two's complement sign extension)
    if (bSigned)
    {
        // Check if sign bit is set
        if (value & (1ULL << (nBitLength - 1)))
        {
            // Sign extend by setting all upper bits
            value |= (0xFFFFFFFFFFFFFFFFULL << nBitLength);
        }
    }

    return (int32_t)value;
}

int32_t Dbc::DecodeBE(const uint8_t *pData, uint8_t nStartBit,
                      uint8_t nBitLength, bool bSigned)
{
    if (nBitLength == 0 || nBitLength > 32)
        return 0;

    // For big-endian (Motorola), start bit is the MSB of the signal
    // We need to convert from Motorola bit numbering to actual bit position

    // Calculate which byte and bit within byte
    uint8_t startByte = nStartBit / 8;
    uint8_t startBitInByte = nStartBit % 8;

    uint64_t value = 0;
    int bitsRemaining = nBitLength;

    // Extract bits from MSB to LSB
    while (bitsRemaining > 0)
    {
        // Calculate how many bits we can read from current byte
        int bitsToRead = (startBitInByte + 1 < bitsRemaining) ? (startBitInByte + 1) : bitsRemaining;

        // Extract bits from current byte
        uint8_t mask = ((1 << bitsToRead) - 1);
        uint8_t byteBits = (pData[startByte] >> (startBitInByte - bitsToRead + 1)) & mask;

        // Add to result
        value |= ((uint64_t)byteBits << (bitsRemaining - bitsToRead));

        // Move to next byte
        bitsRemaining -= bitsToRead;
        startByte++;
        startBitInByte = 7; // Next byte starts from MSB
    }

    // Handle signed values (two's complement sign extension)
    if (bSigned)
    {
        if (value & (1ULL << (nBitLength - 1)))
        {
            value |= (0xFFFFFFFFFFFFFFFFULL << nBitLength);
        }
    }

    return (int32_t)value;
}

void Dbc::EncodeLE(uint8_t *pData, int32_t nRawValue,
                   uint8_t nStartBit, uint8_t nBitLength)
{
    if (nBitLength == 0 || nBitLength > 32)
        return;

    // Create mask for the bits we want to set
    uint64_t mask = (1ULL << nBitLength) - 1;
    uint64_t value = (uint64_t)nRawValue & mask;

    // Copy frame data to 64-bit value
    uint64_t rawData = 0;
    for (int i = 0; i < 8; i++)
    {
        rawData |= ((uint64_t)pData[i]) << (i * 8);
    }

    // Clear the bits we're about to write
    uint64_t clearMask = ~(mask << nStartBit);
    rawData &= clearMask;

    // Set the new value
    rawData |= (value << nStartBit);

    // Write back to data array
    for (int i = 0; i < 8; i++)
    {
        pData[i] = (uint8_t)(rawData >> (i * 8));
    }
}

void Dbc::EncodeBE(uint8_t *pData, int32_t nRawValue,
                   uint8_t nStartBit, uint8_t nBitLength)
{
    if (nBitLength == 0 || nBitLength > 32)
        return;

    // Create mask for the value
    uint64_t mask = (1ULL << nBitLength) - 1;
    uint64_t value = (uint64_t)nRawValue & mask;

    // Calculate which byte and bit within byte
    uint8_t startByte = nStartBit / 8;
    uint8_t startBitInByte = nStartBit % 8;

    int bitsRemaining = nBitLength;

    // Write bits from MSB to LSB
    while (bitsRemaining > 0)
    {
        // Calculate how many bits we can write to current byte
        int bitsToWrite = (startBitInByte + 1 < bitsRemaining) ? (startBitInByte + 1) : bitsRemaining;

        // Extract bits to write
        uint8_t bitMask = ((1 << bitsToWrite) - 1);
        uint8_t bitsValue = (value >> (bitsRemaining - bitsToWrite)) & bitMask;

        // Clear target bits in byte
        uint8_t clearMask = ~(bitMask << (startBitInByte - bitsToWrite + 1));
        pData[startByte] &= clearMask;

        // Write bits
        pData[startByte] |= (bitsValue << (startBitInByte - bitsToWrite + 1));

        // Move to next byte
        bitsRemaining -= bitsToWrite;
        startByte++;
        startBitInByte = 7;
    }
}

float Dbc::ApplyScaling(int32_t nRawValue, float fScale, float fOffset)
{
    return ((float)nRawValue * fScale) + fOffset;
}

int32_t Dbc::ReverseScaling(float fPhysicalValue, float fScale, float fOffset)
{
    if (fScale == 0.0f)
        return 0;

    return (int32_t)((fPhysicalValue - fOffset) / fScale);
}

// Float decode/encode with float scaling (value = raw * scale + offset)
float Dbc::DecodeFloat(const uint8_t *pData, uint8_t nStartBit, uint8_t nBitLength,
                       float fScale, float fOffset, ByteOrder eByteOrder, bool bSigned)
{
    int32_t rawValue = DecodeInt(pData, nStartBit, nBitLength, eByteOrder, bSigned);
    return ApplyScaling(rawValue, fScale, fOffset);
}

// Float decode/encode with float scaling (value = raw * scale + offset)
void Dbc::EncodeFloat(uint8_t *pData, float fPhysicalValue, uint8_t nStartBit,
                      uint8_t nBitLength, float fScale, float fOffset, ByteOrder eByteOrder)
{
    int32_t rawValue = ReverseScaling(fPhysicalValue, fScale, fOffset);
    EncodeInt(pData, rawValue, nStartBit, nBitLength, eByteOrder);
}

// IEEE 754 32-bit float decode/encode (raw float bits, must be byte-aligned)
float Dbc::DecodeFloat(const uint8_t *pData, uint8_t nStartBit)
{
    uint8_t nStartByte = nStartBit / 8;
    uint32_t raw = pData[nStartByte] |
                   ((uint32_t)pData[nStartByte + 1] << 8) |
                   ((uint32_t)pData[nStartByte + 2] << 16) |
                   ((uint32_t)pData[nStartByte + 3] << 24);
    float result;
    __builtin_memcpy(&result, &raw, sizeof(float));
    return result;
}

// IEEE 754 32-bit float decode/encode (raw float bits, must be byte-aligned)
void Dbc::EncodeFloat(uint8_t *pData, float fValue, uint8_t nStartBit)
{
    uint8_t nStartByte = nStartBit / 8;
    uint32_t raw;
    __builtin_memcpy(&raw, &fValue, sizeof(uint32_t));
    pData[nStartByte] = raw & 0xFF;
    pData[nStartByte + 1] = (raw >> 8) & 0xFF;
    pData[nStartByte + 2] = (raw >> 16) & 0xFF;
    pData[nStartByte + 3] = (raw >> 24) & 0xFF;
}
