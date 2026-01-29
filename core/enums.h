#pragma once

#include <cstdint>

enum class MsgCmd : uint8_t
{   //Response Tx = MsgCmd + 128
    Null = 0,
    Can = 1,
    Inputs = 5,
    Outputs = 10,
    OutputsPwm = 11,
    VirtualInputs = 15,
    Wiper = 20,
    WiperSpeed = 21,
    WiperDelays = 22,
    Flashers = 25,
    StarterDisable = 30,
    CanInputs = 35,
    CanInputsId = 36,
    CanInputsOffset = 37,
    CanInputsOperand = 38,
    Counters = 40,
    Conditions = 45,
    Keypad = 50,
    KeypadLed = 51,
    KeypadButton = 52,
    KeypadButtonLed = 53,
    KeypadDial = 54,
    Version = 120,
    Sleep = 121,
    Bootloader = 125,
    BurnSettings = 127
};

enum class MsgCmdResult : uint8_t
{
    Invalid = 0,
    Request = 1,
    Write = 2
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
    NotEqual,
    GreaterThan,
    LessThan,
    GreaterThanOrEqual,
    LessThanOrEqual,
    BitwiseAnd,
    BitwiseNand
};

enum class BoolOperator : uint8_t
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
    Momentary,
    Latching
};

enum class InputEdge : uint8_t
{
    Rising,
    Falling,
    Both
};

enum class InputPull : uint8_t
{
    None,
    Up,
    Down
};

enum class PdmState : uint8_t
{
    Run,
    Sleep,
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
  ErrTemp,
  ErrPwm
};

enum class PwmChannel : uint8_t
{
    Ch1 = 0,
    Ch2 = 1,
    Ch3 = 2,
    Ch4 = 3
};

enum class KeypadModel : uint8_t
{
    Blink2Key,
    Blink4Key,
    Blink5Key,
    Blink6Key,
    Blink8Key,
    Blink10Key,
    Blink12Key,
    Blink15Key,
    Blink13Key2Dial,
    BlinkRacepad,
    Grayhill6Key,
    Grayhill8Key,
    Grayhill15Key,
    Grayhill20Key
};

enum class BlinkMarineButtonColor : uint8_t
{
    Off = 0x00,
    Red = 0x01,
    Green = 0x02,
    Orange = 0x03,
    Blue = 0x04,
    Violet = 0x05,
    Cyan = 0x06,
    White = 0x07,
};

enum class BlinkMarineBacklightColor : uint8_t
{
    Off = 0x00,
    Red = 0x01,
    Green = 0x02,
    Blue = 0x03,
    Yellow = 0x04, 
    Cyan = 0x05,
    Violet = 0x06,
    White = 0x07,
    Amber = 0x08,
    YellowGreen = 0x09
};
