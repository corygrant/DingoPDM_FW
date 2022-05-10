################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../PDM/Src/can_input.c \
../PDM/Src/canboard.c \
../PDM/Src/dingo_pdm.c \
../PDM/Src/flasher.c \
../PDM/Src/logger.c \
../PDM/Src/pdm_config.c \
../PDM/Src/pdm_input.c \
../PDM/Src/pushbutton.c \
../PDM/Src/starter.c \
../PDM/Src/virtual_input.c \
../PDM/Src/wipers.c 

OBJS += \
./PDM/Src/can_input.o \
./PDM/Src/canboard.o \
./PDM/Src/dingo_pdm.o \
./PDM/Src/flasher.o \
./PDM/Src/logger.o \
./PDM/Src/pdm_config.o \
./PDM/Src/pdm_input.o \
./PDM/Src/pushbutton.o \
./PDM/Src/starter.o \
./PDM/Src/virtual_input.o \
./PDM/Src/wipers.o 

C_DEPS += \
./PDM/Src/can_input.d \
./PDM/Src/canboard.d \
./PDM/Src/dingo_pdm.d \
./PDM/Src/flasher.d \
./PDM/Src/logger.d \
./PDM/Src/pdm_config.d \
./PDM/Src/pdm_input.d \
./PDM/Src/pushbutton.d \
./PDM/Src/starter.d \
./PDM/Src/virtual_input.d \
./PDM/Src/wipers.d 


# Each subdirectory must supply rules for building sources it contributes
PDM/Src/%.o PDM/Src/%.su: ../PDM/Src/%.c PDM/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F303xC -DDEBUG -c -I../Core/Inc -I../Components/Inc -I../PDM/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-PDM-2f-Src

clean-PDM-2f-Src:
	-$(RM) ./PDM/Src/can_input.d ./PDM/Src/can_input.o ./PDM/Src/can_input.su ./PDM/Src/canboard.d ./PDM/Src/canboard.o ./PDM/Src/canboard.su ./PDM/Src/dingo_pdm.d ./PDM/Src/dingo_pdm.o ./PDM/Src/dingo_pdm.su ./PDM/Src/flasher.d ./PDM/Src/flasher.o ./PDM/Src/flasher.su ./PDM/Src/logger.d ./PDM/Src/logger.o ./PDM/Src/logger.su ./PDM/Src/pdm_config.d ./PDM/Src/pdm_config.o ./PDM/Src/pdm_config.su ./PDM/Src/pdm_input.d ./PDM/Src/pdm_input.o ./PDM/Src/pdm_input.su ./PDM/Src/pushbutton.d ./PDM/Src/pushbutton.o ./PDM/Src/pushbutton.su ./PDM/Src/starter.d ./PDM/Src/starter.o ./PDM/Src/starter.su ./PDM/Src/virtual_input.d ./PDM/Src/virtual_input.o ./PDM/Src/virtual_input.su ./PDM/Src/wipers.d ./PDM/Src/wipers.o ./PDM/Src/wipers.su

.PHONY: clean-PDM-2f-Src

