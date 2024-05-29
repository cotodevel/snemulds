################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fat/disc_io.c \
../fat/gba_nds_fat.c \
../fat/io_efa2.c \
../fat/io_ezsd.c \
../fat/io_fcsr.c \
../fat/io_m3cf.c \
../fat/io_m3sd.c \
../fat/io_mmcf.c \
../fat/io_mpcf.c \
../fat/io_nmmc.c \
../fat/io_sccf.c \
../fat/io_scsd.c 

S_SRCS += \
../fat/io_ezsd_asm.s \
../fat/io_m3sd_asm.s \
../fat/io_scsd_asm.s 

OBJS += \
./fat/disc_io.o \
./fat/gba_nds_fat.o \
./fat/io_efa2.o \
./fat/io_ezsd.o \
./fat/io_ezsd_asm.o \
./fat/io_fcsr.o \
./fat/io_m3cf.o \
./fat/io_m3sd.o \
./fat/io_m3sd_asm.o \
./fat/io_mmcf.o \
./fat/io_mpcf.o \
./fat/io_nmmc.o \
./fat/io_sccf.o \
./fat/io_scsd.o \
./fat/io_scsd_asm.o 

C_DEPS += \
./fat/disc_io.d \
./fat/gba_nds_fat.d \
./fat/io_efa2.d \
./fat/io_ezsd.d \
./fat/io_fcsr.d \
./fat/io_m3cf.d \
./fat/io_m3sd.d \
./fat/io_mmcf.d \
./fat/io_mpcf.d \
./fat/io_nmmc.d \
./fat/io_sccf.d \
./fat/io_scsd.d 


# Each subdirectory must supply rules for building sources it contributes
fat/%.o: ../fat/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	C:/devkitPro2007/devkitARM/bin/arm-eabi-gcc -DNDS -I"C:\devkitPro\libnds\include" -I"C:\devkitPro\devkitARM\arm-eabi\include" -O2 -Wall -c -fmessage-length=0 -fsigned-char -ffast-math -fomit-frame-pointer -marm -mcpu=arm9tdmi -mtune=arm9tdmi -DARM9 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

fat/%.o: ../fat/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	C:/devkitPro2007/devkitARM/bin/arm-eabi-as  -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


