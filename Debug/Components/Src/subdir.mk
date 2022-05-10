################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Components/Src/ads1x15.c \
../Components/Src/mb85rc.c \
../Components/Src/mcp9808.c \
../Components/Src/pca9539.c \
../Components/Src/pca9555.c \
../Components/Src/pca9635.c \
../Components/Src/pcal9554b.c \
../Components/Src/profet.c 

OBJS += \
./Components/Src/ads1x15.o \
./Components/Src/mb85rc.o \
./Components/Src/mcp9808.o \
./Components/Src/pca9539.o \
./Components/Src/pca9555.o \
./Components/Src/pca9635.o \
./Components/Src/pcal9554b.o \
./Components/Src/profet.o 

C_DEPS += \
./Components/Src/ads1x15.d \
./Components/Src/mb85rc.d \
./Components/Src/mcp9808.d \
./Components/Src/pca9539.d \
./Components/Src/pca9555.d \
./Components/Src/pca9635.d \
./Components/Src/pcal9554b.d \
./Components/Src/profet.d 


# Each subdirectory must supply rules for building sources it contributes
Components/Src/%.o Components/Src/%.su: ../Components/Src/%.c Components/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F303xC -DDEBUG -c -I../Core/Inc -I../Components/Inc -I../PDM/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Components-2f-Src

clean-Components-2f-Src:
	-$(RM) ./Components/Src/ads1x15.d ./Components/Src/ads1x15.o ./Components/Src/ads1x15.su ./Components/Src/mb85rc.d ./Components/Src/mb85rc.o ./Components/Src/mb85rc.su ./Components/Src/mcp9808.d ./Components/Src/mcp9808.o ./Components/Src/mcp9808.su ./Components/Src/pca9539.d ./Components/Src/pca9539.o ./Components/Src/pca9539.su ./Components/Src/pca9555.d ./Components/Src/pca9555.o ./Components/Src/pca9555.su ./Components/Src/pca9635.d ./Components/Src/pca9635.o ./Components/Src/pca9635.su ./Components/Src/pcal9554b.d ./Components/Src/pcal9554b.o ./Components/Src/pcal9554b.su ./Components/Src/profet.d ./Components/Src/profet.o ./Components/Src/profet.su

.PHONY: clean-Components-2f-Src

