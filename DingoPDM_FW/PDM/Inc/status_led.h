#ifndef INC_STATUS_LED_H_
#define INC_STATUS_LED_H_

#define LED_BLINK_SPLIT 200

#include "stdint.h"
#include "stdbool.h"

//Create a typedef defining a simple function pointer
//to be used for LED's
typedef void (*LedFunc)(void);

//this struct holds function pointers to turn each LED
//on and off
typedef struct
{
  const LedFunc On;
  const LedFunc Off;
  bool bBlinkOn; //Blink by turning on = true, blink by turning off = false
  uint32_t nOnUntil;
}Led_Output;

typedef enum{
  LED_STEADY,
  LED_BLINK,
  LED_CODE
}LedState_t;

extern Led_Output StatusLed;
extern Led_Output ErrorLed;

void LedSetSteady(Led_Output* out, bool bState);
void LedSetCode(Led_Output* out, uint8_t nCode);
void LedUpdate(uint32_t nNow, Led_Output* out);
void LedBlink(uint32_t nNow, Led_Output* out);

#endif /* INC_STATUS_LED_H_ */