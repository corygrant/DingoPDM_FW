#pragma once

#include <cstdint>

//=============================================================================
// System State
//=============================================================================
enum class DeviceState : uint8_t
{
    Run,
    Sleep,
    OverTemp,
    Error
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
    ErrPwm,
    ErrVarMap
};

//=============================================================================
// Message Enums
//=============================================================================
enum class MsgCmd : uint8_t
{
    Null = 0,
    Read = 1,
    Write = 2,
    ReadParamNotFound = 5,

    ReadAll = 10,
    ReadAllRsp = 11,
    ReadAllComplete = 12,
    ReadAllModified = 13,

    WriteAll = 20,
    WriteAllVal = 21,
    WriteAllComplete = 22,
    WriteAllModified = 23,
    WriteAllParamNotFound = 25,
    WriteAllOutOfRange = 26,

    BurnSettings = 30,
    Version = 31,
    Sleep = 32,
    Bootloader = 33,
    CheckCrc = 34,
    CheckCrcRsp = 35,

    Invalid = 0xFF
};

enum class MsgType : uint8_t
{
    Info = 'F',
    Warning = 'R',
    Error = 'E'
};

enum class MsgSrc : uint8_t
{
    State_Run = 1,
    State_Sleep,
    State_Overtemp,
    State_Error,
    Overcurrent,
    Voltage,
    CANBus,
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

//=============================================================================
// Logic Operators
//=============================================================================
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

//=============================================================================
// CAN
//=============================================================================
enum class CanBitrate : uint8_t
{
    Bitrate_1000K,
    Bitrate_500K,
    Bitrate_250K,
    Bitrate_125K,
    Bitrate_100K
};

enum class ByteOrder : uint8_t
{
    LittleEndian = 0, // Intel byte order
    BigEndian = 1     // Motorola byte order
};

//=============================================================================
// Input
//=============================================================================
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

//=============================================================================
// Output (Profet)
//=============================================================================
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

//=============================================================================
// PWM
//=============================================================================
enum class PwmChannel : uint8_t
{
    Ch1 = 0,
    Ch2 = 1,
    Ch3 = 2,
    Ch4 = 3
};

//=============================================================================
// Wiper
//=============================================================================
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

//=============================================================================
// Keypad
//=============================================================================
enum class KeypadModel : uint8_t
{
    Blink2Key = 0,
    Blink4Key = 1,
    Blink5Key = 2,
    Blink6Key = 3,
    Blink8Key = 4,
    Blink10Key = 5,
    Blink12Key = 6,
    Blink15Key = 7,
    Blink15Key2Dial = 8,
    Grayhill1Key = 10,
    Grayhill6Key = 20,
    Grayhill8Key = 21,
    Grayhill12Key = 22,
    Grayhill15Key = 23,
    Grayhill20Key = 24
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

enum class BlinkMarineMessageId : uint16_t
{
    Nmt = 0x00,
    ButtonState = 0x180,
    SetLed = 0x200,
    DialState1 = 0x280,
    SetLedBlink = 0x300,
    DialState2 = 0x380,
    LedBrightness = 0x400,
    AnalogInput = 0x480,
    Backlight = 0x500,
    SdoResponse = 0x580,
    SdoRequest = 0x600,
    Heartbeat = 0x700
};

enum class GrayhillMessageId : uint16_t
{
    ButtonState = 0x180,
    LedControl = 0x200,
    BrightnessControl = 0x300
};