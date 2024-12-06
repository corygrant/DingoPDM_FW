# List of all the board related files.
BOARDSRC = ./boards/dingopdm_v7/board.c

# Required include directories
BOARDINC = ./boards/dingopdm_v7

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)

include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f4xx.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/platform.mk
include $(CHIBIOS)/os/common/ports/ARMv7-M/compilers/GCC/mk/port.mk