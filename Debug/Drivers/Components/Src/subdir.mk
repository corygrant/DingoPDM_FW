################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Components/Src/ads1x15.c \
../Drivers/Components/Src/canboard.c \
../Drivers/Components/Src/mb85rc.c \
../Drivers/Components/Src/mcp9808.c \
../Drivers/Components/Src/pca9539.c \
../Drivers/Components/Src/pca9555.c \
../Drivers/Components/Src/pca9635.c \
../Drivers/Components/Src/pcal9554b.c \
../Drivers/Components/Src/profet.c \
../Drivers/Components/Src/pushbutton.c \
../Drivers/Components/Src/virtual_input.c \
../Drivers/Components/Src/wipers.c 

OBJS += \
./Drivers/Components/Src/ads1x15.o \
./Drivers/Components/Src/canboard.o \
./Drivers/Components/Src/mb85rc.o \
./Drivers/Components/Src/mcp9808.o \
./Drivers/Components/Src/pca9539.o \
./Drivers/Components/Src/pca9555.o \
./Drivers/Components/Src/pca9635.o \
./Drivers/Components/Src/pcal9554b.o \
./Drivers/Components/Src/profet.o \
./Drivers/Components/Src/pushbutton.o \
./Drivers/Components/Src/virtual_input.o \
./Drivers/Components/Src/wipers.o 

C_DEPS += \
./Drivers/Components/Src/ads1x15.d \
./Drivers/Components/Src/canboard.d \
./Drivers/Components/Src/mb85rc.d \
./Drivers/Components/Src/mcp9808.d \
./Drivers/Components/Src/pca9539.d \
./Drivers/Components/Src/pca9555.d \
./Drivers/Components/Src/pca9635.d \
./Drivers/Components/Src/pcal9554b.d \
./Drivers/Components/Src/profet.d \
./Drivers/Components/Src/pushbutton.d \
./Drivers/Components/Src/virtual_input.d \
./Drivers/Components/Src/wipers.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Components/Src/%.o Drivers/Components/Src/%.su: ../Drivers/Components/Src/%.c Drivers/Components/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F303xC -DDEBUG -c -I../Core/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../Drivers/Components/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Components-2f-Src

clean-Drivers-2f-Components-2f-Src:
	-$(RM) ./Drivers/Components/Src/ads1x15.d ./Drivers/Components/Src/ads1x15.o ./Drivers/Components/Src/ads1x15.su ./Drivers/Components/Src/canboard.d ./Drivers/Components/Src/canboard.o ./Drivers/Components/Src/canboard.su ./Drivers/Components/Src/mb85rc.d ./Drivers/Components/Src/mb85rc.o ./Drivers/Components/Src/mb85rc.su ./Drivers/Components/Src/mcp9808.d ./Drivers/Components/Src/mcp9808.o ./Drivers/Components/Src/mcp9808.su ./Drivers/Components/Src/pca9539.d ./Drivers/Components/Src/pca9539.o ./Drivers/Components/Src/pca9539.su ./Drivers/Components/Src/pca9555.d ./Drivers/Components/Src/pca9555.o ./Drivers/Components/Src/pca9555.su ./Drivers/Components/Src/pca9635.d ./Drivers/Components/Src/pca9635.o ./Drivers/Components/Src/pca9635.su ./Drivers/Components/Src/pcal9554b.d ./Drivers/Components/Src/pcal9554b.o ./Drivers/Components/Src/pcal9554b.su ./Drivers/Components/Src/profet.d ./Drivers/Components/Src/profet.o ./Drivers/Components/Src/profet.su ./Drivers/Components/Src/pushbutton.d ./Drivers/Components/Src/pushbutton.o ./Drivers/Components/Src/pushbutton.su ./Drivers/Components/Src/virtual_input.d ./Drivers/Components/Src/virtual_input.o ./Drivers/Components/Src/virtual_input.su ./Drivers/Components/Src/wipers.d ./Drivers/Components/Src/wipers.o ./Drivers/Components/Src/wipers.su

.PHONY: clean-Drivers-2f-Components-2f-Src

