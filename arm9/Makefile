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


# relative path (if you are outside arm9 folder then $(DIR_ARM9), arm7 is $(DIR_ARM7))
RELATIVE_PATH =	

#Prepare ARM9 dirs
DIRLIBS_HDR = $(foreach dirres,$(DIRS_ARM9_HEADER),-I "$(dirres)" )	$(DIRLIBS_ALLOWED)	$(TARGET_LIBRARY_HEADERS_9_INC)
DIRLIBS_SRC = $(foreach dirres,$(DIRS_ARM9_SRC),-I "$(dirres)" )

file_arm9_src_c=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm9_src_s=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm9_src_SUPP=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )
file_arm9_src_cpp=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.cpp))  )
file_arm9_src_bin=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.bin))  )

# Object Target
objs_arm9:=	${file_arm9_src_c:.c=.o}	${file_arm9_src_s:.s=.o}	${file_arm9_src_SUPP:.S=.o}		${file_arm9_src_cpp:.cpp=.o}
d_objs_arm9:=	${file_arm9_src_c:.c=.d}	${file_arm9_src_s:.s=.d}	${file_arm9_src_SUPP:.S=.d}	${file_arm9_src_cpp:.cpp=.d}

#ARM9 specific definitions.	-fno-common so we force initialized data to rodata
MAPFILE = $(DIR_ARM9).map
SHARED_FLAGS	=	-march=armv5te	-mthumb-interwork
AS_ASFLAGS		=	$(SHARED_FLAGS)	-defsym debug=0	-defsym	appver='$(EXECUTABLE_VERSION)'	-defsym	ARM9='1'
CC_ASFLAGS		=	$(SHARED_FLAGS)	-Wall	-DNDS -DARM9	-marm	-Os -s	-fno-exceptions	-MMD -MP	-Ddebug=0	-nostdinc	-Dappver='$(EXECUTABLE_VERSION)'
CPP_FLAGS		=	$(SHARED_FLAGS)	$(CC_ASFLAGS)
BUILD			= 	$(BUILD_ARM9)
DIR_SRC			=	$(DIRS_ARM9_SRC)

#--gc-sections: allows relocatable sections linked (already) to be included, otherwise unused sections are discarded.
#check arm9.map "Discarded input sections" to see --gc-sections in action.
LDFLAGS	=	-specs=special_nds_arm9_ld.specs	-Wl,--no-gc-sections,-Map,$(MAPFILE),-z,defs
LDFLAGS_DSI	:=	-specs=special_dsi_arm9_ld.specs	-Wl,--no-gc-sections,-Map,arm9-nonstripped_dsi.map
LIBRARIES_NTR =	-ltgdsfilesystem9	-l$(TARGET_LIBRARY_TGDS_NTR_9)	-lhardware9	-lcnano9	-lgcc	
LIBRARIES_TWL =	-ltgdsfilesystem9i	-l$(TARGET_LIBRARY_TGDS_TWL_9)	-lhardware9i	-lstdc++	-lcnano9	-lgcc
################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS = 	$(objs_arm9)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG = 	$(addprefix $(OBJDIR)/, $(NONSTRIPELF_ARM9))


all: $(OBJDIR)|	$(TARGET_LIBRARY_CRT0_FILE_9).o	$(BINSTRIP_RULE_9)
#all: debug

#Check $(OBJDIR) if not exists: create it
$(OBJDIR):
	@echo "Folder $(OBJDIR) does not exist. Creating"
	mkdir -p $@

#rule for reading *.bin in /data folder, converts to .C source code, which then gets compiled as object so it can be linked later
$(OBJDIR)/%.o : %.bin
	-@echo 'trying to convert .BIN ($<) -> .C output: ${@:.o=.c} : arrayName: $(subst build/,,${@:.o=})'
	$(BIN2C)	$<	${@:.o=.c}	$(subst build/,,${@:.o=})
	$(CC)	$(CC_ASFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_PROHIBITED)	-o $@ -c ${@:.o=.c}

#Do C Compile now!
$(OBJDIR)/%.o : %.c
	-@echo 'C Compile start: $@'
	$(CC)	$(CC_ASFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_PROHIBITED)	-o $@ -c $<
	-@echo 'C Compile End: $@'

#Do C++ (CXX) Compile now!
$(OBJDIR)/%.o : %.cpp
	-@echo 'C++ Compile start: $@'
	$(CXX)	$(CPP_FLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	-o $@ -c $<
	-@echo 'C Compile End: $@'

#Coto: Special "exception" rule. Fixes tricky projects such as SnemulDS's opcodes2.s where GCC AS is used, while compatibility with .S is required for newer projects.
#.S is GCC CC/CPP for compiling assembler with cpp headers
#.s is GCC AS Assembler for compiling assembler the usual way

#Do s (GCC AS) Compile now!
$(OBJDIR)/%.o : %.s
	-@echo 's Compile start: $@'
	$(AS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(AS_ASFLAGS)	-o $@ -c $<
	-@echo 's Compile end: $@'

#Do S (GCC CC) Compile now!
$(OBJDIR)/%.o : %.S
	-@echo 'S Compile start: $@'
	$(CXX)	-x assembler-with-cpp	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(CC_ASFLAGS)	-o $@ -c $<
	-@echo 'S Compile end: $@'

#Special rule: compile crt0 for arm9 entrypoint
$(TARGET_LIBRARY_CRT0_FILE_9).o:	$(TARGET_LIBRARY_LINKER_FILE_9)
	-@echo 'S Compile start: crt0s'
	$(CXX)	-x assembler-with-cpp	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(CC_ASFLAGS)	-o $(OBJDIR)/$@ -c $<
	-@echo 'S Compile end: crt0s'

#Linkage. 
$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJECTS))
	-@echo 'Entering Linking phase'
	-$(CC) $^ $(LDFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_SHARED)	-L	$(TARGET_LIBRARY_PATH)/	-L	$(TOOLCHAIN_PATH)/		$(LIBRARIES_NTR)	-L	$(TARGET_LIBRARY_LINKER_PATH_LD)	-o $@
	-$(CC) $^	$(LDFLAGS_DSI)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_SHARED)	-L	$(TARGET_LIBRARY_PATH)/	-L	$(TOOLCHAIN_PATH)/		$(LIBRARIES_TWL)	-L	$(TARGET_LIBRARY_LINKER_PATH_LD)	-o ${@:.elf=_dsi.elf}
	-@echo 'Leaving Linking phase. Output: $@'

#$(OBJSTRIP) --strip-unneeded ${@:.elf=-nonstripped_dsi.elf}
#xxxx-nonstripped.elf -> xxxx.elf STRIP operation
$(ELF_ARM9): $(OBJPROG)
	-@echo '$@ Unstripped. STRIP Start!'
	-mv build/${@:.elf=-nonstripped_dsi.elf}	$(CURDIR)
	$(OBJSTRIP)	-s --strip-debug	$<	
	$(OBJSTRIP)	-s --strip-debug	${@:.elf=-nonstripped_dsi.elf}
	-mv	$<	$@
	
$(BINSTRIP_RULE_9): $(ELF_ARM9)
	-@echo 'STRIP End: Remove ELF header and build baremetal Binary executable: $<	->	$@ !'
	$(OBJCOPY) 	-S -O binary	$<	$@
	$(OBJCOPY) 	-S -O binary	${<:.elf=-nonstripped_dsi.elf}	${@:.bin=_dsi.bin}
	
	-rm	$<
################################################################################################

clean:
	-@echo 'cleanup $(DIR_ARM9)'
	-@rm -fr $(OBJDIR)	$(BINSTRIP_RULE_9)	$(MAPFILE)	$(CURDIR)/arm9-nonstripped_dsi.elf $(CURDIR)/arm9-nonstripped_dsi.map	$(CURDIR)/arm9_dsi.bin