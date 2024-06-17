#include "status_led.h"

/*
* @brief  Set the LED to a steady state
* @param  out: Pointer to the LED output
* @param  bState: true for ON, false for OFF
*/
void LedSetSteady(Led_Output* out, bool bState)
{
    if(bState){
        out->bState = true;
        out->On();
    } else{
        out->bState = false;
        out->Off();
    }
}

/*
* @brief  Set the LED to blink a number of times
* @param  out: Pointer to the LED output
* @param  nCode: Number of blinks
*/
void LedSetCode(uint32_t nNow, Led_Output* out, uint8_t nCode)
{
    //Blinking code
    if (out->nBlinkState == 0){
      //Blink until code is done
      if(out->nBlinkCount <= nCode){

        //This blink done
        if(nNow > out->nUntil){
          //On, turn off
          if(out->bState){
            LedSetSteady(out, false);
            out->nUntil = nNow + (LED_BLINK_SPLIT*2);
          }
          //Off, turn on
          else{
            LedSetSteady(out, true);
            out->nBlinkCount++;
            out->nUntil = nNow + (LED_BLINK_SPLIT*2);
          }
        }
      }
      else{
        //Pause after code
        out->nBlinkCount = 0;
        out->nUntil = nNow + LED_BLINK_PAUSE;
        out->nBlinkState = 1;
      }
    }
    //Pause between blinks 
    else{
      LedSetSteady(out, true);

      //Done pausing, back to blinking
      if (nNow > out->nUntil){
        out->nBlinkState = 0;
      }
    }
}

/*
* @brief  Blink the LED
* @param  nNow: Current time
* @param  out: Pointer to the LED output
*/
void LedBlink(uint32_t nNow, Led_Output* out){

    if(nNow > out->nUntil){
      //On, turn off
      if(out->bState){
        LedSetSteady(out, false);
        out->nUntil = nNow + LED_BLINK_SPLIT;
      }
      //Off, turn on
      else{
        LedSetSteady(out, true);
        out->nBlinkCount++;
        out->nUntil = nNow + LED_BLINK_SPLIT;
      }
    }
}