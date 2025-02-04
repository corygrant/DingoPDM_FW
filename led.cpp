#include "led.h"

void Led::Solid(bool bOn)
{
    if(bOn)
        palSetLine(m_line);
    else
        palClearLine(m_line);

    bState = bOn;
}

void Led::Code(uint8_t nCode)
{
    uint32_t nNow = SYS_TIME;

    if(bFirst){
      nUntil = nNow;
      bFirst = false;
    }

    //Blinking code
    if (nBlinkState == 0){
      //Blink until code is done
      if (nBlinkCount <= nCode){

        //This blink done
        if(SYS_TIME > nUntil){
          //On, turn off
          if(bState){
            Led::Solid(false);
            nUntil = SYS_TIME + (LED_BLINK_SPLIT*2);
          }
          //Off, turn on
          else{
            Led::Solid(true);
            nBlinkCount++;
            nUntil = SYS_TIME + (LED_BLINK_SPLIT*2);
          }
        }
      }
      else{
        //Pause after code
        nBlinkCount = 0;
        nUntil = SYS_TIME + LED_BLINK_PAUSE;
        nBlinkState = 1;
      }
    }
    //Pause between blinks 
    else{
      Led::Solid(true);

      //Done pausing, back to blinking
      if (SYS_TIME > nUntil){
        nBlinkState = 0;
      }
    }
}

void Led::Blink(){

    if(SYS_TIME > nUntil){
      if(bState){
        Solid(false);
        nUntil = SYS_TIME + LED_BLINK_SPLIT;
      }
      else{
        Solid(true);
        nBlinkCount++;
        nUntil = SYS_TIME + LED_BLINK_SPLIT;
      }
    }
}