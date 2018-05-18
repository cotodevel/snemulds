################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SNES_global.c \
../apu.c \
../core.c \
../engine.c \
../fs.c \
../gui.c \
../main.c \
../memmap.c \
../opcodes.c \
../ppu.c \
../snapshot.c \
../snes.c \
../touch.c 

S_SRCS += \
../intr.s \
../opcodes2.s 

OBJS += \
./SNES_global.o \
./apu.o \
./core.o \
./engine.o \
./fs.o \
./gui.o \
./intr.o \
./main.o \
./memmap.o \
./opcodes.o \
./opcodes2.o \
./ppu.o \
./snapshot.o \
./snes.o \
./touch.o 

C_DEPS += \
./SNES_global.d \
./apu.d \
./core.d \
./engine.d \
./fs.d \
./gui.d \
./main.d \
./memmap.d \
./opcodes.d \
./ppu.d \
./snapshot.d \
./snes.d \
./touch.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	arm-eabi-gcc -DNDS -DASM_OPCODES -DARM9 -I"C:\devkitPro\libnds\include" -I"C:\devkitPro\libfat\include" -I"C:\devkitPro\devkitARM\arm-eabi\include" -O2 -Wall -c -fmessage-length=0 -fsigned-char -ffast-math -fomit-frame-pointer -marm -mcpu=arm946e-s -mtune=arm946e-s -DARM9 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.s
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	arm-eabi-as  -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

opcodes2.o: ../opcodes2.s
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	arm-eabi-as -defsym debug=0 -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


