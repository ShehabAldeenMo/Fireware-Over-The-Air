################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Det.c \
../Src/Dio_Cfg.c \
../Src/Dio_Prg.c \
../Src/Mcu_Cfg.c \
../Src/Mcu_Prg.c \
../Src/Port_Cfg.c \
../Src/Port_Prg.c \
../Src/main.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/Det.o \
./Src/Dio_Cfg.o \
./Src/Dio_Prg.o \
./Src/Mcu_Cfg.o \
./Src/Mcu_Prg.o \
./Src/Port_Cfg.o \
./Src/Port_Prg.o \
./Src/main.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/Det.d \
./Src/Dio_Cfg.d \
./Src/Dio_Prg.d \
./Src/Mcu_Cfg.d \
./Src/Mcu_Prg.d \
./Src/Port_Cfg.d \
./Src/Port_Prg.d \
./Src/main.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/Det.cyclo ./Src/Det.d ./Src/Det.o ./Src/Det.su ./Src/Dio_Cfg.cyclo ./Src/Dio_Cfg.d ./Src/Dio_Cfg.o ./Src/Dio_Cfg.su ./Src/Dio_Prg.cyclo ./Src/Dio_Prg.d ./Src/Dio_Prg.o ./Src/Dio_Prg.su ./Src/Mcu_Cfg.cyclo ./Src/Mcu_Cfg.d ./Src/Mcu_Cfg.o ./Src/Mcu_Cfg.su ./Src/Mcu_Prg.cyclo ./Src/Mcu_Prg.d ./Src/Mcu_Prg.o ./Src/Mcu_Prg.su ./Src/Port_Cfg.cyclo ./Src/Port_Cfg.d ./Src/Port_Cfg.o ./Src/Port_Cfg.su ./Src/Port_Prg.cyclo ./Src/Port_Prg.d ./Src/Port_Prg.o ./Src/Port_Prg.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

