################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/at45db_api.c \
../Core/Src/buttons_actions.c \
../Core/Src/byte_convertion.c \
../Core/Src/functions.c \
../Core/Src/init_config.c \
../Core/Src/irqs.c \
../Core/Src/learn_prev_next.c \
../Core/Src/led_f.c \
../Core/Src/main.c \
../Core/Src/mult_mode_processing.c \
../Core/Src/poly_mode_processing.c \
../Core/Src/sequencer_mode_processing.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/struct_init_print.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/trans_mode_processing.c 

OBJS += \
./Core/Src/at45db_api.o \
./Core/Src/buttons_actions.o \
./Core/Src/byte_convertion.o \
./Core/Src/functions.o \
./Core/Src/init_config.o \
./Core/Src/irqs.o \
./Core/Src/learn_prev_next.o \
./Core/Src/led_f.o \
./Core/Src/main.o \
./Core/Src/mult_mode_processing.o \
./Core/Src/poly_mode_processing.o \
./Core/Src/sequencer_mode_processing.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/struct_init_print.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/trans_mode_processing.o 

C_DEPS += \
./Core/Src/at45db_api.d \
./Core/Src/buttons_actions.d \
./Core/Src/byte_convertion.d \
./Core/Src/functions.d \
./Core/Src/init_config.d \
./Core/Src/irqs.d \
./Core/Src/learn_prev_next.d \
./Core/Src/led_f.d \
./Core/Src/main.d \
./Core/Src/mult_mode_processing.d \
./Core/Src/poly_mode_processing.d \
./Core/Src/sequencer_mode_processing.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/struct_init_print.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/trans_mode_processing.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/at45db_api.cyclo ./Core/Src/at45db_api.d ./Core/Src/at45db_api.o ./Core/Src/at45db_api.su ./Core/Src/buttons_actions.cyclo ./Core/Src/buttons_actions.d ./Core/Src/buttons_actions.o ./Core/Src/buttons_actions.su ./Core/Src/byte_convertion.cyclo ./Core/Src/byte_convertion.d ./Core/Src/byte_convertion.o ./Core/Src/byte_convertion.su ./Core/Src/functions.cyclo ./Core/Src/functions.d ./Core/Src/functions.o ./Core/Src/functions.su ./Core/Src/init_config.cyclo ./Core/Src/init_config.d ./Core/Src/init_config.o ./Core/Src/init_config.su ./Core/Src/irqs.cyclo ./Core/Src/irqs.d ./Core/Src/irqs.o ./Core/Src/irqs.su ./Core/Src/learn_prev_next.cyclo ./Core/Src/learn_prev_next.d ./Core/Src/learn_prev_next.o ./Core/Src/learn_prev_next.su ./Core/Src/led_f.cyclo ./Core/Src/led_f.d ./Core/Src/led_f.o ./Core/Src/led_f.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/mult_mode_processing.cyclo ./Core/Src/mult_mode_processing.d ./Core/Src/mult_mode_processing.o ./Core/Src/mult_mode_processing.su ./Core/Src/poly_mode_processing.cyclo ./Core/Src/poly_mode_processing.d ./Core/Src/poly_mode_processing.o ./Core/Src/poly_mode_processing.su ./Core/Src/sequencer_mode_processing.cyclo ./Core/Src/sequencer_mode_processing.d ./Core/Src/sequencer_mode_processing.o ./Core/Src/sequencer_mode_processing.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/struct_init_print.cyclo ./Core/Src/struct_init_print.d ./Core/Src/struct_init_print.o ./Core/Src/struct_init_print.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/trans_mode_processing.cyclo ./Core/Src/trans_mode_processing.d ./Core/Src/trans_mode_processing.o ./Core/Src/trans_mode_processing.su

.PHONY: clean-Core-2f-Src

