################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSP/Flash\ Memory/Memory.c 

OBJS += \
./BSP/Flash\ Memory/Memory.o 

C_DEPS += \
./BSP/Flash\ Memory/Memory.d 


# Each subdirectory must supply rules for building sources it contributes
BSP/Flash\ Memory/Memory.o: ../BSP/Flash\ Memory/Memory.c BSP/Flash\ Memory/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L476xx -DGPS_NEO6M -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/insat/PFE/Sofiatech/Test_freeRTOS/BSP/Flash Memory" -I"D:/insat/PFE/Sofiatech/Test_freeRTOS/BSP/GPS" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I"D:/insat/PFE/Sofiatech/Test_freeRTOS/BSP/BLE" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"BSP/Flash Memory/Memory.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSP-2f-Flash-20-Memory

clean-BSP-2f-Flash-20-Memory:
	-$(RM) ./BSP/Flash\ Memory/Memory.d ./BSP/Flash\ Memory/Memory.o ./BSP/Flash\ Memory/Memory.su

.PHONY: clean-BSP-2f-Flash-20-Memory

