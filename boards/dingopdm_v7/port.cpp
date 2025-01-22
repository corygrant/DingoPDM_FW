#include "port.h"

static const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 36MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=4, Seq 1=15 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_SJW(0) | CAN_BTR_BRP(3)  | CAN_BTR_TS1(14) | CAN_BTR_TS2(1),
};

const CANConfig& GetCanConfig() {
    return canConfig500;
}

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

void EnableBackupDomain(void) {
    // Enable PWR clock
    rccEnablePWRInterface(false);
    
    // Enable backup domain access
    PWR->CR |= PWR_CR_DBP;
    
    // Enable backup SRAM clock
    RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
    
    // Wait for backup domain write protection disable
    while(!(PWR->CR & PWR_CR_DBP));
}

#define BOOT_ADDR  0x1FFF0000

struct boot_vectable_{
  uint32_t Initial_SP;
  void (*Reset_Handler)(void);
};

#define BOOTVTAB ((struct boot_vectable_*)BOOT_ADDR)
 void JumpToBootloader(void)
 {
    //Disable all interrupts
    __disable_irq();

    chSysDisable();

    // Reset USB
    USB_OTG_FS->GOTGCTL |= USB_OTG_DCTL_SDIS;
    USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;

    // Stop all ChibiOS drivers
    adcStop(&ADCD1);
    canStop(&CAND1);
    i2cStop(&I2CD1);

    // Reset all GPIO pins
    GPIOA->MODER = 0;
    GPIOB->MODER = 0;
    GPIOC->MODER = 0;
    GPIOD->MODER = 0;

    // Disable all peripheral clocks
    RCC->AHB1ENR = 0;
    RCC->AHB2ENR = 0;
    RCC->APB1ENR = 0;
    RCC->APB2ENR = 0;

    // Reset system clock
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));
    RCC->CFGR = 0;

    //Disable systick timer
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    //Clear interrupt enable register and interrupt pending register
    for (uint8_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); i++)
    {
      NVIC->ICER[i] = 0xFFFFFFFF;
      NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // Enable SYSCFG clock and remap memory
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->MEMRMP = SYSCFG_MEMRMP_MEM_MODE_0;

    //Re-enable all interrupts
    __enable_irq();

    //Set the MSP
    __set_PSP(BOOTVTAB->Initial_SP);
    __set_MSP(BOOTVTAB->Initial_SP);

    //Jump to bootloader
    BOOTVTAB->Reset_Handler();
 }

 void RequestBootloader()
{
    // Set the magic code in the reserved SRAM location
    *(volatile uint32_t*)BOOTLOADER_FLAG_ADDRESS = BOOTLOADER_MAGIC_CODE;

    // -Reset the microcontroller to start the bootloader on next boot
    // -Using this approach solves the issue of tracking down every possible 
    // peripheral that could be enabled and preventing entering the bootloader properly
    NVIC_SystemReset();
    
    // No further code will execute after this point
}

void CheckBootloaderRequest()
{
    EnableBackupDomain();

    // Check if the magic code is present in the reserved SRAM location
    if(*(volatile uint32_t*)BOOTLOADER_FLAG_ADDRESS == BOOTLOADER_MAGIC_CODE)
    {
        // Clear the magic code
        *(volatile uint32_t*)BOOTLOADER_FLAG_ADDRESS = 0;
        
        // Enter the bootloader
        JumpToBootloader();
    }
}