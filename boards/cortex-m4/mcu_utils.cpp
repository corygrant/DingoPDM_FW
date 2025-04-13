#include "mcu_utils.h"
#include "hal.h"

void EnterStopMode()
{
    chSysLock();

    //Set wakeup sources
    //Digital inputs change detection
    //Set the pull based on the current configuration
    palEnableLineEvent(LINE_DI1, PAL_EVENT_MODE_BOTH_EDGES | PAL_STM32_PUPDR_PULLUP);
    palEnableLineEvent(LINE_DI2, PAL_EVENT_MODE_BOTH_EDGES | PAL_STM32_PUPDR_PULLUP);

    //CAN receive detection
    palSetLineMode(LINE_CAN_RX, PAL_MODE_INPUT);
    palEnableLineEvent(LINE_CAN_RX, PAL_EVENT_MODE_BOTH_EDGES | PAL_STM32_PUPDR_FLOATING);

    //USB VBUS detection
    palSetLineMode(LINE_USB_VBUS, PAL_MODE_INPUT);
    palEnableLineEvent(LINE_USB_VBUS, PAL_EVENT_MODE_RISING_EDGE | PAL_STM32_PUPDR_PULLDOWN);

    PWR->CR &= ~PWR_CR_PDDS;	            // cleared PDDS means stop mode (not standby) 
	PWR->CR |= PWR_CR_CWUF | PWR_CR_CSBF;	// clear wakeup flag, clear standby flag
    PWR->CR |= PWR_CR_FPDS | PWR_CR_LPDS;	// turn off flash, regulator in low power mode
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;      // enable deep sleep mode

    __disable_irq();
    
    __WFI();

    // Resume here after wakeup
    NVIC_SystemReset();
}

 void RequestBootloader()
{
    // Set the magic code
    *((unsigned long *)0x2001FFF0) = 0xDEADBEEF; // End of RAM

    // Reset the microcontroller to start the bootloader on next boot
    // See enter_bootloader.S, which overrides the reset handler
    // to jump to the bootloader if the magic code is set
    NVIC_SystemReset();
    
    // No further code will execute after this point
}