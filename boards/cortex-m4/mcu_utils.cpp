#include "mcu_utils.h"
#include "hal.h"

void EnterStopMode()
{
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