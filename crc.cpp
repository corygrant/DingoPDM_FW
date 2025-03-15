#include "crc.h"

uint32_t CalculateCRC32(const void* data, size_t length, uint32_t initialValue) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t crc = initialValue;
    
    // Standard CRC-32 (Ethernet, ZIP, etc.) calculation
    for (size_t i = 0; i < length; i++) {
        crc ^= bytes[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 * (crc & 1));
        }
    }
    
    return ~crc; // Final XOR with 0xFFFFFFFF
}