#include "mcu_utils.h"
#include "hal.h"

void EnterStopMode()
{
    __disable_irq();
    
    SysTick->CTRL = 0;
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	PWR->CR &= ~PWR_CR_PDDS;	// cleared PDDS means stop mode (not standby) 
	PWR->CR |= PWR_CR_FPDS;	    // turn off flash in stop mode
    PWR->CR |= PWR_CR_LPDS;	    // regulator in low power mode

    //Set wakeup sources
    palEnableLineEvent(LINE_DI1, PAL_EVENT_MODE_BOTH_EDGES | PAL_STM32_PUPDR_PULLUP);
    palEnableLineEvent(LINE_DI2, PAL_EVENT_MODE_BOTH_EDGES | PAL_STM32_PUPDR_PULLUP);
    palSetLineMode(LINE_CAN_RX, PAL_MODE_INPUT);
    palEnableLineEvent(LINE_CAN_RX, PAL_EVENT_MODE_BOTH_EDGES | PAL_STM32_PUPDR_FLOATING);

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