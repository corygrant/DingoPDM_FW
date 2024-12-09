#include "ch.h"
#include "hal.h"
#include "pdm.h"

/*
 * Application entry point.
 */
int main(void) {

  halInit();
  chSysInit();

  InitPdm();
  
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
