#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Calculate CRC32 using the standard Ethernet/ZIP polynomial
 * 
 * @param data Pointer to data buffer
 * @param length Length of data in bytes
 * @param initialValue Initial value for CRC calculation (usually 0xFFFFFFFF)
 * @return uint32_t Calculated CRC32 value
 */
uint32_t CalculateCRC32(const void* data, size_t length, uint32_t initialValue = 0xFFFFFFFF);