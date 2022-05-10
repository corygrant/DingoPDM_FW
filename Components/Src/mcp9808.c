#include "mcp9808.h"

uint8_t MCP9808_Overtemp, MCP9808_Undertemp, MCP9808_CriticalTemp;

uint8_t MCP9808_Init(I2C_HandleTypeDef* hi2c, uint16_t addr)
{
  if(MCP9808_Read16(hi2c, addr, MCP9808_REG_MANUF_ID) != 0x0054)
    return 0;
  if(MCP9808_Read16(hi2c, addr, MCP9808_REG_DEVICE_ID) != 0x0400)
    return 0;

  //Set Alert pin function
  //B0 = 0 (comparator output)
  //B1 = 0 (active low)
  //B2 = 0 (alert output for Tupper, Tlower, and Tcrit)
  //B3 = 1 (alert output enabled)
  //B4 = 0 (alert output asserted - status output - not a setting)
  //B5 = 0 (interrupt clear - no effect)
  //B6 = 0 (Tupper Tlower window unlocked)
  //B7 = 0 (Tcrit unlocked)
  //B8 = 0 (continous conversion)
  //B9-10 = 01 (Tupper Tlower hysterisis +1.5 deg C)
  //B11-15 = 00000 (not used)
  uint16_t config = (MCP9808_REG_CONFIG_ALERTCTRL | MCP9808_REG_CONFIG_HYST_1_5);
  MCP9808_Write16(hi2c, addr, MCP9808_REG_CONFIG, config);
  return 1;
}

float MCP9808_ReadTempC(I2C_HandleTypeDef* hi2c, uint16_t addr)
{
  float temp = 0.0;
  uint16_t t = MCP9808_Read16(hi2c, addr, MCP9808_REG_AMBIENT_TEMP);

  MCP9808_MapLimitBits(t);

  if (t != 0xFFFF) {
    temp = t & 0x0FFF;
    temp /= 16.0;
    if (t & 0x1000)
      temp -= 256;
  }

  return temp;
}

int16_t MCP9808_ReadTempC_Int(I2C_HandleTypeDef* hi2c, uint16_t addr)
{
  uint16_t t = MCP9808_Read16(hi2c, addr, MCP9808_REG_AMBIENT_TEMP);

  MCP9808_MapLimitBits(t);

  if (t != 0xFFFF) {
    t = t & 0x0FFF;
    t /= 16.0;
    if (t & 0x1000)
      t = -t;
  }
  return t;
}

float MCP9808_ReadTempF(I2C_HandleTypeDef* hi2c, uint16_t addr)
{
  float temp = 0.0;
  uint16_t t = MCP9808_Read16(hi2c, addr, MCP9808_REG_AMBIENT_TEMP);

  MCP9808_MapLimitBits(t);

  if (t != 0xFFFF) {
    temp = t & 0x0FFF;
    temp /= 16.0;
    if (t & 0x1000)
      temp -= 256;

    temp = temp * 9.0 / 5.0 + 32;
  }

  return temp;
}

float MCP9808_ConvertToF(float degC){
  return degC * 9.0 / 5.0 + 32;
}

uint8_t MCP9808_GetResolution(I2C_HandleTypeDef* hi2c, uint16_t addr)
{
  return MCP9808_Read8(hi2c, addr, MCP9808_REG_RESOLUTION);
}

void MCP9808_SetResolution(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t val)
{
  MCP9808_Write8(hi2c, addr, MCP9808_REG_RESOLUTION, val);
}

void MCP9808_Shutdown(I2C_HandleTypeDef* hi2c, uint16_t addr)
{
  uint16_t conf_shutdown;
  uint16_t conf_register = MCP9808_Read16(hi2c, addr, MCP9808_REG_CONFIG);

  conf_shutdown = conf_register | MCP9808_REG_CONFIG_SHUTDOWN;
  MCP9808_Write16(hi2c, addr, MCP9808_REG_CONFIG, conf_shutdown);
}

void MCP9808_Wake(I2C_HandleTypeDef* hi2c, uint16_t addr)
{
  uint16_t conf_shutdown;
  uint16_t conf_register = MCP9808_Read16(hi2c, addr, MCP9808_REG_CONFIG);

  conf_shutdown = conf_register & ~MCP9808_REG_CONFIG_SHUTDOWN;
  MCP9808_Write16(hi2c, addr, MCP9808_REG_CONFIG, conf_shutdown);
}

uint8_t MCP9808_SetLimit(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, float val)
{
  uint16_t newVal = val * 16.0;
  if(val < 0)
    newVal += 256;
  MCP9808_Write16(hi2c, addr, reg, val * 16.0);

  float temp = MCP9808_RawToTemp(MCP9808_Read16(hi2c, addr, reg));

  if(val == temp)
    return 1;
  return 0;
}

float MCP9808_RawToTemp(uint16_t raw)
{
  float temp = 0.0;
  if (raw != 0xFFFF) {
    temp = raw & 0x0FFF;
    temp /= 16.0;
    if (raw & 0x1000)
      temp -= 256;
  }
  return temp;
}

void MCP9808_MapLimitBits(uint16_t raw)
{
  MCP9808_Overtemp     = (raw & MCP9808_REG_OVERTEMP) >> MCP9808_POS_OVERTEMP;
  MCP9808_Undertemp    = (raw & MCP9808_REG_UNDERTEMP) >> MCP9808_POS_UNDERTEMP;
  MCP9808_CriticalTemp = (raw & MCP9808_REG_CRITICALTEMP) >> MCP9808_POS_CRITICALTEMP;
}

void MCP9808_Write16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint16_t val){
  uint8_t writeVals[3];

  writeVals[0] = reg;
  writeVals[1] = val >> 8;
  writeVals[2] = val & 0xFF;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 3, 100);
}
uint16_t MCP9808_Read16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg)
{
  uint8_t writeVals[1];
  uint8_t readVals[2];
  uint16_t val = 0xFFFF;

  writeVals[0] = reg;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 1, 100);

  HAL_I2C_Master_Receive(hi2c, addr << 1, readVals, 2, 100);

  val = (readVals[0] << 8 | readVals[1]);

  return val;
}

void MCP9808_Write8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint16_t val)
{
  uint8_t writeVals[2];

  writeVals[0] = reg;
  writeVals[1] = val;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 2, 100);
}

uint8_t MCP9808_Read8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg)
{
  uint8_t writeVals[1];
  uint8_t readVals[1];

  uint8_t val = 0xFF;

  writeVals[0] = reg;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 1, 100);

  HAL_I2C_Master_Receive(hi2c, addr << 1, readVals, 1, 100);

  val = readVals[0];

  return val;
}

uint8_t MCP9808_GetCriticalTemp()
{
  return MCP9808_CriticalTemp;
}

uint8_t MCP9808_GetOvertemp()
{
  return MCP9808_Overtemp;
}

uint8_t MCP9808_GetUndertemp()
{
  return MCP9808_Undertemp;
}
