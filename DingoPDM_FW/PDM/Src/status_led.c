#include "status_led.h"

void LedSetCode(Led_Output* out, uint8_t nCode)
{
    //Call LedBlink to blink the number of times in nCode without blocking


}

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

void LedBlink(uint32_t nNow, Led_Output* out){
    out->nOnUntil = nNow + LED_BLINK_SPLIT;
}