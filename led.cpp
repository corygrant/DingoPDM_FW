#include "led.h"

void Led::Solid(bool bOn)
{
    if(bOn)
        palSetLine(m_line);
    else
        palClearLine(m_line);

    bState = bOn;
}

void Led::Code(uint32_t nTimeNow, uint8_t nCode)
{
    //Blinking code
    if (nBlinkState == 0){
      //Blink until code is done
      if (nBlinkCount <= nCode){

        //This blink done
        if(nTimeNow > nUntil){
          //On, turn off
          if(bState){
            Led::Solid(false);
            nUntil = nTimeNow + (LED_BLINK_SPLIT*2);
          }
          //Off, turn on
          else{
            Led::Solid(true);
            nBlinkCount++;
            nUntil = nTimeNow + (LED_BLINK_SPLIT*2);
          }
        }
      }
      else{
        //Pause after code
        nBlinkCount = 0;
        nUntil = nTimeNow + LED_BLINK_PAUSE;
        nBlinkState = 1;
      }
    }
    //Pause between blinks 
    else{
      Led::Solid(true);

      //Done pausing, back to blinking
      if (nTimeNow > nUntil){
        nBlinkState = 0;
      }
    }
}

void Led::Blink(uint32_t nTimeNow){

    if(nTimeNow > nUntil){
      if(bState){
        Solid(false);
        nUntil = nTimeNow + LED_BLINK_SPLIT;
      }
      else{
        Solid(true);
        nBlinkCount++;
        nUntil = nTimeNow + LED_BLINK_SPLIT;
      }
    }
}