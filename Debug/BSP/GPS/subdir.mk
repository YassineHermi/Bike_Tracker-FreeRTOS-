################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSP/GPS/GPS.c 

OBJS += \
./BSP/GPS/GPS.o 

C_DEPS += \
./BSP/GPS/GPS.d 


# Each subdirectory must supply rules for building sources it contributes
BSP/GPS/%.o BSP/GPS/%.su: ../BSP/GPS/%.c BSP/GPS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L476xx -DGPS_NEO6M -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/insat/PFE/Sofiatech/Test_freeRTOS/BSP/Flash Memory" -I"D:/insat/PFE/Sofiatech/Test_freeRTOS/BSP/GPS" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I"D:/insat/PFE/Sofiatech/Test_freeRTOS/BSP/BLE" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSP-2f-GPS

clean-BSP-2f-GPS:
	-$(RM) ./BSP/GPS/GPS.d ./BSP/GPS/GPS.o ./BSP/GPS/GPS.su

.PHONY: clean-BSP-2f-GPS

