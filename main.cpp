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
    palSetLine(LINE_LED_STATUS);
    chThdSleepMilliseconds(500);
    palClearLine(LINE_LED_STATUS);
    chThdSleepMilliseconds(500);
  }
}
