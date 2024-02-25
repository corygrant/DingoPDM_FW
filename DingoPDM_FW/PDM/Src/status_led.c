#include "status_led.h"

/*
* @brief  Set the LED to a steady state
* @param  out: Pointer to the LED output
* @param  bState: true for ON, false for OFF
*/
void LedSetSteady(Led_Output* out, bool bState)
{
    out->bBlinkOn = true;
    if(bState){
        out->On();
    } else{
        out->Off();
    }
}

/*
* @brief  Set the LED to blink a number of times
* @param  out: Pointer to the LED output
* @param  nCode: Number of blinks
*/
void LedSetCode(Led_Output* out, uint8_t nCode)
{
    //Call LedBlink to blink the number of times in nCode without blocking


}

/*
* @brief  Update the LED state
* @param  nNow: Current time
* @param  out: Pointer to the LED output
*/
void LedUpdate(uint32_t nNow, Led_Output* out){

    if(out->bBlinkOn){
      if(nNow < out->nOnUntil){
        out->On();
      } else{
        out->Off();
      }
    } else{
      if(nNow < out->nOnUntil){
        out->Off();
      } else{
        out->On();
      }
    }
}

/*
* @brief  Blink the LED
* @param  nNow: Current time
* @param  out: Pointer to the LED output
*/
void LedBlink(uint32_t nNow, Led_Output* out){
    out->nOnUntil = nNow + LED_BLINK_SPLIT;
}