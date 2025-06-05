##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

ifeq ($(BOARD),)
	BOARD = dingopdm_v7
	#BOARD = dingopdmmax_v1
endif

BOARDDIR = boards/$(BOARD)

# Compiler options here.
ifeq ($(USE_OPT),)
	USE_OPT = -O0 -ggdb -fomit-frame-pointer -falign-functions=16 -fsingle-precision-constant
#           ^^^
# If planning to attach a debugger, change to -O0
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
	USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
	USE_CPPOPT = -std=c++20 -Wno-register -fno-rtti -fno-threadsafe-statics -fno-exceptions -fno-use-cxa-atexit -Wno-deprecated -Werror=shadow
endif

# Enable this if you want the linker to remove unused code and data.
ifeq ($(USE_LINK_GC),)
	USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
	USE_LDOPT = --print-memory-usage
endif

# Enable this if you want link time optimizations (LTO).
ifeq ($(USE_LTO),)
	USE_LTO = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
	USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
	USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
	USE_PROCESS_STACKSIZE = 0x1000
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
	USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU (no, softfp, hard).
ifeq ($(USE_FPU),)
	USE_FPU = hard
endif

# FPU-related options.
ifeq ($(USE_FPU_OPT),)
	USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv4-sp-d16
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, target, sources and paths
#

# Define project name here
PROJECT = $(BOARD)

# Target settings.
MCU  = cortex-m4

# Imported source files and paths.
CHIBIOS  := ./ChibiOS
CONFDIR  := ./cfg
BUILDDIR := ./build
DEPDIR   := ./.dep

MCUDIR := boards/$(MCU)

BOOTLOADERASM := $(MCUDIR)/enter_bootloader.S
	
# Licensing files.
include $(CHIBIOS)/os/license/license.mk

# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/various/cpp_wrappers/chcpp.mk
# Auto-build files in ./source recursively.
include $(CHIBIOS)/tools/mk/autobuild.mk

# include board.mk that sets per-board options
include $(BOARDDIR)/board.mk

# Define linker script file here
# LDSCRIPT= $(STARTUPLD)/STM32F446xE.ld
# Use custom linker script to override Reset_Handler
LDSCRIPT = $(BOARDDIR)/$(BOARD).ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(ALLCSRC)

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(ALLCPPSRC) \
				 $(BOARDDIR)/port.cpp \
				 $(MCUDIR)/mcu_utils.cpp \
				 msg.cpp \
				 can_input.cpp \
				 can.cpp \
				 condition.cpp \
				 config.cpp \
				 config_handler.cpp \
				 counter.cpp \
				 crc.cpp \
				 digital.cpp \
				 error.cpp \
				 flasher.cpp \
				 infomsg.cpp \
				 input.cpp \
				 led.cpp \
				 mailbox.cpp \
				 pdm.cpp \
				 pwm.cpp \
				 profet.cpp \
				 hw_devices.cpp \
				 request_msg.cpp \
				 sleep.cpp \
				 starter.cpp \
				 status.cpp \
				 usb.cpp \
				 virtual_input.cpp \
				 wiper/wiper_digin.cpp \
				 wiper/wiper_intin.cpp \
				 wiper/wiper_mixin.cpp \
				 wiper/wiper.cpp \
				 hardware/mcp9808.cpp \
				 hardware/mb85rc.cpp \
				 keypad/keypad_button.cpp \
				 keypad/keypad_dial.cpp \
				 keypad/keypad.cpp \
				 main.cpp

# List ASM source files here.
ASMSRC = $(ALLASMSRC)

# List ASM with preprocessor source files here.
ASMXSRC = $(ALLXASMSRC) $(BOOTLOADERASM)

# Inclusion directories.
INCDIR = $(CONFDIR) $(ALLINC) $(TESTINC)

# Define C warning options here.
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here.
CPPWARN = -Wall -Wextra -Wundef

#
# Project, target, sources and paths
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS = 

# Define ASM defines here
UADEFS = 

# List all user directories here
UINCDIR = ./boards/$(MCU)

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = -lm --specs=nosys.specs

#
# End of user section
##############################################################################

##############################################################################
# Common rules
#

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk

#
# Common rules
##############################################################################

##############################################################################
# Custom rules
#

#
# Custom rules
##############################################################################