#ifndef INC_MCP9808_H_
#define INC_MCP9808_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"

#define MCP9808_OK 1
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

uint8_t MCP9808_Init(I2C_HandleTypeDef* hi2c, uint16_t addr);
float MCP9808_ReadTempC(I2C_HandleTypeDef* hi2c, uint16_t addr);
int16_t MCP9808_ReadTempC_Int(I2C_HandleTypeDef* hi2c, uint16_t addr);
float MCP9808_ReadTempF(I2C_HandleTypeDef* hi2c, uint16_t addr);
float MCP9808_ConvertToF(float degC);
uint8_t MCP9808_GetResolution(I2C_HandleTypeDef* hi2c, uint16_t addr);
void MCP9808_SetResolution(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t val);
void MCP9808_Shutdown(I2C_HandleTypeDef* hi2c, uint16_t addr);
void MCP9808_Wake(I2C_HandleTypeDef* hi2c, uint16_t addr);
uint8_t MCP9808_SetLimit(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, float val);
float MCP9808_RawToTemp(uint16_t raw);
void MCP9808_MapLimitBits(uint16_t raw);
void MCP9808_Write16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint16_t val);
uint16_t MCP9808_Read16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg);
void MCP9808_Write8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint16_t val);
uint8_t MCP9808_Read8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg);
uint8_t MCP9808_GetCriticalTemp();
uint8_t MCP9808_GetOvertemp();
uint8_t MCP9808_GetUndertemp();
#endif /* INC_MCP9808_H_ */
