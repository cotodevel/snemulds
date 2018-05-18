################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include fat/subdir.mk
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

# Tool invocations
000SNEmulDS.nds: $(ABS)
	@echo 'Building target: $@'
	@echo 'Invoking: NDS Tool'
	$(DEVKITARM2007)/bin/ndstool -v -g SNDS AR SNEmulDS 1  -c "000SNEmulDS.nds"  -9 $(ABS)
	@echo 'Finished building target: $@'
	@echo ' '

arm9.elf: $(OBJS) $(USER_OBJS)
	@echo 'Invoking: GCC C++ Linker'
	$(DEVKITARM2007)/bin/arm-eabi-gcc $(OBJS) $(USER_OBJS) $(LIBS) -mno-fpu -lfat -lnds9 -L"D:\devkitpro2007\libfat\lib" -L"D:\devkitpro2007\libnds\lib" -Wl,-Map,$(strip $(EXECUTABLES)).map -mthumb-interwork -specs=ds_arm9.specs -o"arm9.elf"
	@echo 'Finished building: $@'
	@echo ' '

arm9.abs: $(EXECUTABLES)
	@echo 'Invoking: Object Copy'
	$(DEVKITARM2007)/bin/arm-eabi-objcopy -O binary $(EXECUTABLES) "arm9.abs"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(NDS)$(OBJS)$(ABS)$(EXECUTABLES)$(C_DEPS) 000SNEmulDS.nds
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
