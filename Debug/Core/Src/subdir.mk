################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/can_input.c \
../Core/Src/dingo_pdm.c \
../Core/Src/flasher.c \
../Core/Src/freertos.c \
../Core/Src/logger.c \
../Core/Src/main.c \
../Core/Src/pdm_config.c \
../Core/Src/pdm_input.c \
../Core/Src/starter.c \
../Core/Src/stm32f3xx_hal_msp.c \
../Core/Src/stm32f3xx_hal_timebase_tim.c \
../Core/Src/stm32f3xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f3xx.c \
../Core/Src/virtual_input.c 

OBJS += \
./Core/Src/can_input.o \
./Core/Src/dingo_pdm.o \
./Core/Src/flasher.o \
./Core/Src/freertos.o \
./Core/Src/logger.o \
./Core/Src/main.o \
./Core/Src/pdm_config.o \
./Core/Src/pdm_input.o \
./Core/Src/starter.o \
./Core/Src/stm32f3xx_hal_msp.o \
./Core/Src/stm32f3xx_hal_timebase_tim.o \
./Core/Src/stm32f3xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f3xx.o \
./Core/Src/virtual_input.o 

C_DEPS += \
./Core/Src/can_input.d \
./Core/Src/dingo_pdm.d \
./Core/Src/flasher.d \
./Core/Src/freertos.d \
./Core/Src/logger.d \
./Core/Src/main.d \
./Core/Src/pdm_config.d \
./Core/Src/pdm_input.d \
./Core/Src/starter.d \
./Core/Src/stm32f3xx_hal_msp.d \
./Core/Src/stm32f3xx_hal_timebase_tim.d \
./Core/Src/stm32f3xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f3xx.d \
./Core/Src/virtual_input.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F303xC -DDEBUG -c -I../Core/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../Drivers/Components/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/can_input.d ./Core/Src/can_input.o ./Core/Src/can_input.su ./Core/Src/dingo_pdm.d ./Core/Src/dingo_pdm.o ./Core/Src/dingo_pdm.su ./Core/Src/flasher.d ./Core/Src/flasher.o ./Core/Src/flasher.su ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/logger.d ./Core/Src/logger.o ./Core/Src/logger.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/pdm_config.d ./Core/Src/pdm_config.o ./Core/Src/pdm_config.su ./Core/Src/pdm_input.d ./Core/Src/pdm_input.o ./Core/Src/pdm_input.su ./Core/Src/starter.d ./Core/Src/starter.o ./Core/Src/starter.su ./Core/Src/stm32f3xx_hal_msp.d ./Core/Src/stm32f3xx_hal_msp.o ./Core/Src/stm32f3xx_hal_msp.su ./Core/Src/stm32f3xx_hal_timebase_tim.d ./Core/Src/stm32f3xx_hal_timebase_tim.o ./Core/Src/stm32f3xx_hal_timebase_tim.su ./Core/Src/stm32f3xx_it.d ./Core/Src/stm32f3xx_it.o ./Core/Src/stm32f3xx_it.su ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f3xx.d ./Core/Src/system_stm32f3xx.o ./Core/Src/system_stm32f3xx.su ./Core/Src/virtual_input.d ./Core/Src/virtual_input.o ./Core/Src/virtual_input.su

.PHONY: clean-Core-2f-Src
