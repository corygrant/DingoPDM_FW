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

enum class VarMap : uint8_t
{
    None,      
    DigitalIn1,  
    DigitalIn2, 
    CANIn1,
    CANIn2,    
    CANIn3,    
    CANIn4,    
    CANIn5,    
    CANIn6,    
    CANIn7,    
    CANIn8,    
    CANIn9,    
    CANIn10,   
    CANIn11,   
    CANIn12,   
    CANIn13,   
    CANIn14,   
    CANIn15,   
    CANIn16,   
    CANIn17,   
    CANIn18,   
    CANIn19,   
    CANIn20,   
    CANIn21,   
    CANIn22,   
    CANIn23,   
    CANIn24,   
    CANIn25,   
    CANIn26,   
    CANIn27,   
    CANIn28,   
    CANIn29,   
    CANIn30,   
    CANIn31,   
    CANIn32,   
    CANInVal1,  
    CANInVal2,  
    CANInVal3,  
    CANInVal4,  
    CANInVal5,  
    CANInVal6,  
    CANInVal7,  
    CANInVal8,  
    CANInVal9,  
    CANInVal10, 
    CANInVal11, 
    CANInVal12, 
    CANInVal13, 
    CANInVal14, 
    CANInVal15, 
    CANInVal16, 
    CANInVal17, 
    CANInVal18, 
    CANInVal19, 
    CANInVal20, 
    CANInVal21, 
    CANInVal22, 
    CANInVal23, 
    CANInVal24, 
    CANInVal25, 
    CANInVal26, 
    CANInVal27, 
    CANInVal28, 
    CANInVal29, 
    CANInVal30, 
    CANInVal31, 
    CANInVal32, 
    VirtualIn1,  
    VirtualIn2,  
    VirtualIn3,  
    VirtualIn4,  
    VirtualIn5,  
    VirtualIn6,  
    VirtualIn7,  
    VirtualIn8,  
    VirtualIn9,  
    VirtualIn10, 
    VirtualIn11, 
    VirtualIn12, 
    VirtualIn13, 
    VirtualIn14, 
    VirtualIn15, 
    VirtualIn16, 
    Output1On,  
    Output1OC,  
    Output1Fault, 
    Output2On,  
    Output2OC,  
    Output2Fault, 
    Output3On,  
    Output3OC,  
    Output3Fault, 
    Output4On,  
    Output4OC,  
    Output4Fault, 
    Output5On,  
    Output5OC,  
    Output5Fault, 
    Output6On,  
    Output6OC,  
    Output6Fault, 
    Output7On,  
    Output7OC,  
    Output7Fault, 
    Output8On,  
    Output8OC,  
    Output8Fault, 
    WiperSlow,   
    WiperFast,   
    WiperPark,   
    WiperInter,  
    WiperWash,  
    WiperSwipe,  
    Flasher1,
    Flasher2,   
    Flasher3,   
    Flasher4,   
    CounterVal1, 
    CounterVal2, 
    CounterVal3, 
    CounterVal4, 
    Condition1,  
    Condition2,  
    Condition3,  
    Condition4,  
    Condition5,  
    Condition6,  
    Condition7,  
    Condition8,  
    Condition9,  
    Condition10,  
    Condition11,  
    Condition12,  
    Condition13,  
    Condition14,  
    Condition15,  
    Condition16,  
    Condition17,  
    Condition18,  
    Condition19,  
    Condition20,  
    Condition21,  
    Condition22,  
    Condition23,  
    Condition24,  
    Condition25,  
    Condition26,  
    Condition27,  
    Condition28,  
    Condition29,  
    Condition30,  
    Condition31,  
    Condition32,  
    Keypad1Btn1, 
    Keypad1Btn2, 
    Keypad1Btn3, 
    Keypad1Btn4, 
    Keypad1Btn5, 
    Keypad1Btn6, 
    Keypad1Btn7, 
    Keypad1Btn8, 
    Keypad1Btn9, 
    Keypad1Btn10,
    Keypad1Btn11,
    Keypad1Btn12,
    Keypad1Btn13,
    Keypad1Btn14,
    Keypad1Btn15,
    Keypad1Btn16,
    Keypad1Btn17,
    Keypad1Btn18,
    Keypad1Btn19,
    Keypad1Btn20,
    Keypad2Btn1, 
    Keypad2Btn2, 
    Keypad2Btn3, 
    Keypad2Btn4, 
    Keypad2Btn5, 
    Keypad2Btn6, 
    Keypad2Btn7, 
    Keypad2Btn8, 
    Keypad2Btn9, 
    Keypad2Btn10,
    Keypad2Btn11,
    Keypad2Btn12,
    Keypad2Btn13,
    Keypad2Btn14,
    Keypad2Btn15,
    Keypad2Btn16,
    Keypad2Btn17,
    Keypad2Btn18,
    Keypad2Btn19,
    Keypad2Btn20,
    Keypad1Dial1,
    Keypad1Dial2,
    Keypad1Dial3,
    Keypad1Dial4,
    Keypad2Dial1,
    Keypad2Dial2,
    Keypad2Dial3,
    Keypad2Dial4,
    AlwaysOn
};   