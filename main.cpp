#include "ch.h"
#include "hal.h"
#include "device.h"

/*
 * Application entry point.
 */
int main(void) {
  
  halInit();
  chSysInit();
  
  chThdSleepMilliseconds(500);
  
  InitDevice();
  
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
