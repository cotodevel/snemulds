################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SNES_global.c \
../apu.c \
../conf.c \
../core.c \
../crc32.c \
../engine.c \
../fs.c \
../snemul_gui.c \
../main.c \
../memmap.c \
../opcodes.c \
../ppu.c \
../snapshot.c \
../snes.c \
../touch.c \
../gui.c \
../gui_font.c \
../gui_widgets.c \
../katakana_12.c \
../smallfont_7.c \
../trebuchet_9.c \
../input.c \
../ram.c \
../snemul_str_cat.c \
../snemul_str_dan.c \
../snemul_str_eng.c \
../snemul_str_fr.c \
../snemul_str_ger.c \
../snemul_str_ita.c \
../snemul_str_jpn.c \
../snemul_str_nl.c \
../snemul_str_pol.c \
../snemul_str_pt.c \
../snemul_str_spa.c


S_SRCS += \
../intr.s \
../opcodes2.s 

OBJS += \
./SNES_global.o \
./apu.o \
./conf.o \
./core.o \
./crc32.o \
./engine.o \
./fs.o \
./snemul_gui.o \
./intr.o \
./main.o \
./memmap.o \
./opcodes.o \
./opcodes2.o \
./ppu.o \
./snapshot.o \
./snes.o \
./touch.o \
./gui.o \
./gui_font.o \
./gui_widgets.o \
./katakana_12.o \
./smallfont_7.o \
./trebuchet_9.o \
./input.o \
./ram.o \
./snemul_str_cat.o \
./snemul_str_dan.o \
./snemul_str_eng.o \
./snemul_str_fr.o \
./snemul_str_ger.o \
./snemul_str_ita.o \
./snemul_str_jpn.o \
./snemul_str_nl.o \
./snemul_str_pol.o \
./snemul_str_pt.o \
./snemul_str_spa.o

C_DEPS += \
./SNES_global.d \
./apu.d \
./conf.d \
./core.d \
./crc32.d \
./engine.d \
./fs.d \
./snemul_gui.d \
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
	C:/devkitPro2007/devkitARM/bin/arm-eabi-gcc -DNDS -DASM_OPCODES -DARM9 -I"/c/devkitPro2007/libnds/include" -I"/c/devkitPro2007/libfat/include" -I"/c/devkitPro2007/devkitARM/arm-eabi/include" -O2 -Wall -c -fmessage-length=0 -fsigned-char -ffast-math -fomit-frame-pointer -marm -mcpu=arm946e-s -mtune=arm946e-s -DARM9 -DDSEMUL_BUILD -DUSE_LIBFAT -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.s
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	C:/devkitPro2007/devkitARM/bin/arm-eabi-as  -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

opcodes2.o: ../opcodes2.s
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	C:/devkitPro2007/devkitARM/bin/arm-eabi-as -defsym debug=0 -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


