################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include misc/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
arm9.elf \

ABS += \
arm9.abs \


# All Target
all: 000SNEmulDS.nds

dependents:
	-cd C:/SVN/SNEmulDS/src/arm7 && $(MAKE) clean all

# Tool invocations
000SNEmulDS.nds: $(ABS)
	@echo 'Building target: $@'
	@echo 'Invoking: NDS Tool'
	C:/devkitPro2007/devkitARM/bin/ndstool -v -g SNDS AR SNEmulDS 1 -7  ../../arm7/bin/pocketspc.arm7 -r7 0x037f8000 -e7  0x037f8000  -c "000SNEmulDS.nds"  -9 $(ABS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

arm9.elf: $(OBJS) $(USER_OBJS)
	@echo 'Invoking: GCC C++ Linker'
	C:/devkitPro2007/devkitARM/bin/arm-eabi-gcc$(OBJS) $(USER_OBJS) $(LIBS) -mno-fpu -lfat -lnds9 -L"/c/devkitPro2007/libfat/lib" -L"/c/devkitPro2007/libnds/lib" -Wl,-Map,$(strip $(EXECUTABLES)).map -mthumb-interwork -specs=ds_arm9.specs -o"arm9.elf"
	@echo 'Finished building: $@'
	@echo ' '

arm9.abs: $(EXECUTABLES)
	@echo 'Invoking: Object Copy'
	C:/devkitPro2007/devkitARM/bin/arm-eabi-objcopy -O binary $(EXECUTABLES) "arm9.abs"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(NDS)$(OBJS)$(ABS)$(EXECUTABLES)$(C_DEPS) 000SNEmulDS.nds
	-@echo ' '

post-build:
	-cp 000SNEmulDS.nds ../../../001SNEmulDS.nds
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
