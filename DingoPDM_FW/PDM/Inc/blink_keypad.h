#ifndef INC_BLINK_KEYPAD_H_
#define INC_BLINK_KEYPAD_H_

#include "stdint.h"
#include "stdbool.h"

#define BLINK_MARINE_VENDER_ID 0x3E2
#define BLINK_MARINE_PKP1600SI 0x00504B50313630301B53490000000000
#define BLINK_MARINE_PKP2200SI 0x00504B50323230301B53490000000000
#define BLINK_MARINE_PKP2300SI 0x00504B50323230301B53490000000000
#define BLINK_MARINE_PKP2400SI 0x00504B50323430301B53490000000000
#define BLINK_MARINE_PKP2500SI 0x00504B50323530301B53490000000000
#define BLINK_MARINE_PKP2600SI 0x00504B50323630301B53490000000000
#define BLINK_MARINE_PKP3500SI 0x00504B50333530301B53490000000000
#define BLINK_MARINE_PKP1200LI 0x00504B50313230301B4C490000000000
#define BLINK_MARINE_PKP1500LI 0x00504B50313530301B4C490000000000
#define BLINK_MARINE_PKP2200LI 0x00504B50323230301B4C490000000000
#define BLINK_MARINE_PKP2400LI 0x00504B50323430301B4C490000000000
#define BLINK_MARINE_PKP3500SIMT 0x00504B50333530301B4D540000000000

typedef enum{  
    PKP_1600_SI,
    PKP_2200_SI,
    PKP_2300_SI,
    PKP_2400_SI,
    PKP_2500_SI,
    PKP_2600_SI,
    PKP_3500_SI,
    PKP_1200_LI,
    PKP_1500_LI,
    PKP_2200_LI,
    PKP_2400_LI,
    PKP_2300_SI_FR,
    PKP_2400_SI_FR,
    PKP_2600_SI_FR,
    PKP_3500_SI_MT
} BlinkMarineModel_t;

typedef enum{
    BTN_RED = 0x01,
    BTN_GREEN = 0x02,
    BTN_ORANGE = 0x03,
    BTN_BLUE = 0x04,
    BTN_VIOLET = 0x05,
    BTN_CYAN = 0x03,
    BTN_WHITE = 0x07,
} BlinkMarineButtonColor_t;

typedef enum{
    BL_RED = 0x01,
    BL_GREEN = 0x02,
    BL_BLUE = 0x03,
    BL_YELLOW = 0x04, 
    BL_CYAN = 0x05,
    BL_VIOLET = 0x06,
    BL_WHITE = 0x07,
    BL_AMBER = 0x08,
    BL_YELLOWGREEN = 0x09
} BlinkMarineBacklightColor_t;

typedef struct{
    bool bDirection;
    uint8_t nTicks;
    uint16_t nTickCount;
    uint8_t nTopPos;
    uint8_t nLedZeroPos;
    bool bEncoderLedSolid[16];
    bool bEncoderLedBlink[16];
} BlinkMarineEncoder_t;

typedef struct{
    uint16_t nBaseId;
    uint8_t nTickTimer;
    uint8_t nVendorId;
    uint8_t nNumKeys;
    uint8_t nNumKnobs;
    uint8_t nNumAnalogInputs;
    uint8_t nButtonBrightness;
    uint8_t nBacklightBrightness;
    BlinkMarineBacklightColor_t eBacklightColor;
    bool bSwitchInput[16];
    bool bSwitchSolid[16];
    bool bSwitchBlink[16];
    bool bSkipButtonLed[16];
    BlinkMarineButtonColor_t eSolidColor[16];
    BlinkMarineButtonColor_t eBlinkColor[16];
    BlinkMarineEncoder_t stEncoder[2];
} BlinkMarineKeypad_t;

void BlinkMarineKeypad_Init(BlinkMarineModel_t eModel, BlinkMarineKeypad_t *stKeypad);
bool BlinkMarineKeypad_IsMsg(uint16_t nId, BlinkMarineKeypad_t *stKeypad);
void BlinkMarineKeypad_Read(BlinkMarineKeypad_t *stKeypad);
void BlinkMarineKeypad_Write(BlinkMarineKeypad_t *stKeypad);
#endif /* INC_BLINK_KEYPAD_H_ */