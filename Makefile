#
#			Copyright (C) 2017  Coto
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful, but
#WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
#USA
#

# Project Specific
export EXECUTABLE_FNAME = SNEmulDS.nds
export EXECUTABLE_VERSION_HEADER =	0.6a
export EXECUTABLE_VERSION =	"$(EXECUTABLE_VERSION_HEADER)"

# MISC/Dir Definitions
#The ndstool I use requires to have the elf section removed, so these rules create elf headerless- binaries.
export BINSTRIP_RULE_7 =	arm7.bin
export BINSTRIP_RULE_9 =	arm9.bin

export DIR_ARM7 = arm7
export BUILD_ARM7	=	build

export DIR_ARM9 = arm9
export BUILD_ARM9	=	build

export ELF_ARM7 = arm7.elf
export ELF_ARM9 = arm9.elf


# Shared
export GCC_BUILD_ENV	= $(DEFAULT_GCC_PATH)
export GCC_BIN_PATH	= bin/
export ASGCC	=	arm-none-eabi-as.exe
export GCC_C++	=	arm-none-eabi-gcc.exe
export GCC_OBJCOPY	=	arm-none-eabi-objcopy.exe
export CC = $(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(GCC_C++)
export AS	= $(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(ASGCC)
export OBJCOPY	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)$(GCC_OBJCOPY)
export NDSTOOL	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)ndstool.exe
export AR	=	$(GCC_BUILD_ENV)$(GCC_BIN_PATH)arm-none-eabi-ar.exe


#GCC Defs
export TARGET_LIBRARY_NAME_7 = toolchaingen7
export TARGET_LIBRARY_NAME_9 = toolchaingen9
export TOOLCHAIN_LIBRARY_SRCPATH = $(GCC_BUILD_ENV)arm-none-eabi
export DIRLIBS_SHARED =	-I	"$(TOOLCHAIN_LIBRARY_SRCPATH)/include"
export TARGET_LIBRARY_PATH =	$(TOOLCHAIN_LIBRARY_SRCPATH)/lib/toolchain_generic
export LIBPATH	= 	$(TOOLCHAIN_LIBRARY_SRCPATH)/lib/
export TARGET_LIBRARY_LINKER_SRC = /linkers
export TARGET_LIBRARY_CRT0_FILE_7 = nds_arm_ld_crt0
export TARGET_LIBRARY_CRT0_FILE_9 = nds_arm_ld_crt0
export TARGET_LIBRARY_LINKER_FILE_7 = $(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_LINKER_SRC)/$(TARGET_LIBRARY_CRT0_FILE_7).S
export TARGET_LIBRARY_LINKER_FILE_9 = $(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_LINKER_SRC)/$(TARGET_LIBRARY_CRT0_FILE_7).S
export TARGET_LIBRARY_LINKER_PATH_LD = $(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_LINKER_SRC)/
export TARGET_LIBRARY_HEADERS_7 = $(TARGET_LIBRARY_PATH)/headers7
export TARGET_LIBRARY_HEADERS_9 = $(TARGET_LIBRARY_PATH)/headers9
export TARGET_LIBRARY_HEADERS_7_INC = -I	"$(TARGET_LIBRARY_PATH)/headers7"
export TARGET_LIBRARY_HEADERS_9_INC = -I	"$(TARGET_LIBRARY_PATH)/headers9"

export TARGET_LIBRARY_FILE_7	=	lib$(TARGET_LIBRARY_NAME_7).a
export TARGET_LIBRARY_FILE_9	=	lib$(TARGET_LIBRARY_NAME_9).a
export TARGET_LIBRARY_LIBNAME_7	=	l$(TARGET_LIBRARY_NAME_7)
export TARGET_LIBRARY_LIBNAME_9	=	l$(TARGET_LIBRARY_NAME_9)

#default built-in rules removal
MAKEFLAGS += --no-builtin-rules

.SUFFIXES:
.SUFFIXES: .all

#####################################################ARM7#####################################################

export DIRS_ARM7_SRC = source/	\
			source/interrupts/	\
			../common/
			
export DIRS_ARM7_HEADER = source/	\
			source/interrupts/	\
			include/	\
			../common/
#####################################################ARM9#####################################################

export DIRS_ARM9_SRC = source/	\
			source/gui_draw/source/	\
			source/interrupts/	\
			source/wnifilib/	\
			../common/
			
export DIRS_ARM9_HEADER = include/	\
			source/gui_draw/include/	\
			source/wnifilib/	\
			../common/

#cleanup default C/C++/ASM flags
export CFLAGS	=''
export CPPFLAGS	=''
export CPPFLAGS	=''
export ASFLAGS	=''

# Build Target(s)	(both processors here)
all: $(EXECUTABLE_FNAME)
#all:	debug

#ignore building this
.PHONY: $(ELF_ARM7)	$(ELF_ARM9)

#Make
compile	:
	$(MAKE)	-R	-C	$(DIR_ARM7)/
	$(MAKE)	-R	-C	$(DIR_ARM9)/

$(EXECUTABLE_FNAME)	:	compile
	-@echo 'ndstool begin'
	$(NDSTOOL)	-v	-c $@	-7  $(CURDIR)/arm7/$(BINSTRIP_RULE_7)	-e7  0x03800000	-9 $(CURDIR)/arm9/$(BINSTRIP_RULE_9) -e9  0x02000000 -b icon.bmp "SNEmulDS $(EXECUTABLE_VERSION_HEADER) ; SNES Emulator for DS; by archeide bubble2k gladius" 
	-@echo 'ndstool end: built: $@'
	
#---------------------------------------------------------------------------------
# Clean
each_obj = $(foreach dirres,$(dir_read_arm9_files),$(dirres).)
	
clean:
	$(MAKE)	clean	-C	$(DIR_ARM7)/
	$(MAKE)	clean	-C	$(DIR_ARM9)/
	-@rm -fr $(EXECUTABLE_FNAME)

debug:
	-@echo '$(TARGET_LIBRARY_LINKER_PATH_LD)'