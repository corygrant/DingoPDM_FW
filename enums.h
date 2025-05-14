#pragma once

#include <cstdint>

enum class MsgCmd : uint8_t
{   //Response Tx = MsgCmd + 128
    Null = 0,
    Can = 1,
    Inputs = 5,
    //InputsName = 6, //Future use
    Outputs = 10,
    OutputsPwm = 11,
    //OutputsName = 12, //Future use
    VirtualInputs = 15,
    //VirtualInputsName = 16, //Future use
    Wiper = 20,
    WiperSpeed = 21,
    WiperDelays = 22,
    Flashers = 25,
    //FlashersName = 26, //Future use
    StarterDisable = 30,
    CanInputs = 35,
    CanInputsId = 36,
    //CanInputsName = 37, //Future use
    Counters = 40,
    //CountersName = 41, //Future use
    Conditions = 45,
    //ConditionsName = 46, //Future use
    Keypad = 50,
    KeypadLed = 51,
    KeypadButton = 52,
    KeypadButtonLed = 53,
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
    BLINK_2_KEY,
    BLINK_4_KEY,
    BLINK_5_KEY,
    BLINK_6_KEY,
    BLINK_8_KEY,
    BLINK_10_KEY,
    BLINK_12_KEY,
    BLINK_15_KEY,
    BLINK_13_KEY_2_DIAL,
    BLINK_RACEPAD,
    GRAYHILL_6_KEY,
    GRAYHILL_8_KEY,
    GRAYHILL_15_KEY,
    GRAYHILL_20_KEY
};

enum class BlinkMarineButtonColor : uint8_t
{
    BTN_OFF = 0x00,
    BTN_RED = 0x01,
    BTN_GREEN = 0x02,
    BTN_ORANGE = 0x03,
    BTN_BLUE = 0x04,
    BTN_VIOLET = 0x05,
    BTN_CYAN = 0x06,
    BTN_WHITE = 0x07,
};

enum class BlinkMarineBacklightColor : uint8_t
{
    BL_OFF = 0x00,
    BL_RED = 0x01,
    BL_GREEN = 0x02,
    BL_BLUE = 0x03,
    BL_YELLOW = 0x04, 
    BL_CYAN = 0x05,
    BL_VIOLET = 0x06,
    BL_WHITE = 0x07,
    BL_AMBER = 0x08,
    BL_YELLOWGREEN = 0x09
};