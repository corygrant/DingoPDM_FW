#include "ch.h"
#include "hal.h"
#include "pdm.h"

void EnterStopMode();

/*
 * Application entry point.
 */
int main(void) {

  halInit();
  chSysInit();

  CheckBootloaderRequest();
  
  InitPdm();
  
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
