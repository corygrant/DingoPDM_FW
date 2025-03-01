MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K  /* Flash memory */
    RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 128K  /* SRAM memory */
}

/* Include the default ChibiOS linker script */
INCLUDE STM32F446xE.ld

/* Override Reset_Handler */
PROVIDE(Reset_Handler = enter_bootloader);

/* Override or extend specific sections */
SECTIONS
{
    .text :
    {
        *(.text.Reset_Handler)
        *(.text*)
    } > FLASH
}