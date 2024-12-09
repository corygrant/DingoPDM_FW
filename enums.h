#pragma once

#include <cstdint>

enum class MsgCmdRx : uint8_t
{
    BurnSettings = 'B',
    Sleep = 'Q',
    Can = 'C',
    Inputs = 'I',
    Outputs = 'O',
    VirtualInputs = 'U',
    Wiper = 'W',
    WiperSpeed = 'P',
    WiperDelays = 'Y',
    Flashers = 'H',
    Starter = 'D',
    CanInputs = 'N',
    GetVersion = 'V',
    Bootloader = '~',
    Null = 0
};

enum class MsgCmdTx : uint8_t
{
    BurnSettings = 'b',
    Sleep = 'q',
    Can = 'c',
    Inputs = 'i',
    Outputs = 'o',
    VirtualInputs = 'u',
    Wiper = 'w',
    WiperSpeed = 'p',
    WiperDelays = 'y',
    Flashers = 'h',
    Starter = 'd',
    CanInputs = 'n',
    GetVersion = 'v'
};

enum class MsgType : uint8_t
{
    Info = 'F',
    Warning = 'R',
    Error = 'E'
};

enum class MsgSrc : uint8_t
{
  State_PowerOn = 1,
  State_Starting,
  State_Run,
  State_Overtemp,
  State_Error,
  State_Sleep,
  State_Wake,
  Overcurrent,
  Voltage,
  CAN,
  USB,
  Overtemp,
  Config,
  FRAM,
  Analog,
  I2C,
  TempSensor,
  USBConnection,
  Init
 };

enum class Operator : uint8_t
{
    Equal,
    GreaterThan,
    LessThan,
    BitwiseAnd,
    BitwiseNand
};

enum class Condition : uint8_t
{
    And,
    Or,
    Nor
};

enum class CanBitrate : uint8_t
{
    Bitrate_1000K,
    Bitrate_500K,
    Bitrate_250K,
    Bitrate_125K
};

enum class InputMode : uint8_t
{
    Num,
    Momentary,
    Latching
};

enum class InputPull : uint8_t
{
    None,
    Up,
    Down
};

enum class PdmState : uint8_t
{
    PowerOn,
    Starting,
    Run,
    Sleep,
    Wake,
    OverTemp,
    Error
};

enum class ProfetModel : uint8_t
{
    BTS7002_1EPP,
    BTS7008_2EPA_CH1,
    BTS7008_2EPA_CH2,
    BTS70012_1ESP
};

enum class ProfetState : uint8_t
{
    Off,
    On,
    Overcurrent,
    Fault
};

enum class ProfetResetMode : uint8_t
{
    None,
    Count,
    Endless
};

enum class WiperMode : uint8_t
{
    DigIn = 0,
    IntIn = 1,
    MixIn = 2
};

enum class WiperState : uint8_t
{
    Parked = 0,
    Parking = 1,
    Slow = 2,
    Fast = 3,
    IntermittentPause = 4,
    IntermittentOn = 5,
    Wash = 6,
    Swipe = 7
};

enum class WiperSpeed : uint8_t
{
    Park = 0,
    Slow = 1,
    Fast = 2,
    Intermittent1 = 3,
    Intermittent2 = 4,
    Intermittent3 = 5,
    Intermittent4 = 6,
    Intermittent5 = 7,
    Intermittent6 = 8
};

enum class MotorSpeed : uint8_t
{
    Off = 0,
    Slow = 1,
    Fast = 2
};

enum class FatalErrorType : uint8_t
{
  NoError = 0,
  ErrIWDG,
  ErrMailbox,
  ErrTask,
  ErrConfig,
  ErrFRAM,
  ErrADC,
  ErrTempSensor,
  ErrUSB,
  ErrCAN,
  ErrCRC,
  ErrI2C,
  ErrRCC,
  ErrTemp
};