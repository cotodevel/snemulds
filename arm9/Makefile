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
DIRLIBS_HDR = $(foreach dirres,$(DIRS_ARM9_HEADER),-I "$(CURDIR)/$(RELATIVE_PATH)/$(dirres)/" )	$(DIRLIBS_ALLOWED)	$(TARGET_LIBRARY_HEADERS_9_INC)
DIRLIBS_SRC = $(foreach dirres,$(DIRS_ARM9_SRC),-I "$(CURDIR)/$(RELATIVE_PATH)/$(dirres)/" )

file_arm9_src_c=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm9_src_s=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm9_src_SUPP=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )
file_arm9_src_cpp=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.cpp))  )

# Object Target
objs_arm9:=	${file_arm9_src_c:.c=.o}	${file_arm9_src_s:.s=.o}	${file_arm9_src_SUPP:.S=.o}	${file_arm9_src_cpp:.cpp=.o}
d_objs_arm9:=	${file_arm9_src_c:.c=.d}	${file_arm9_src_s:.s=.d}	${file_arm9_src_SUPP:.S=.d}	${file_arm9_src_cpp:.cpp=.d}

#ARM9 specific definitions.	-fno-common so we force initialized data to rodata
MAPFILE = $(DIR_ARM9).map
SHARED_FLAGS	=	-mcpu=arm946e-s	-mthumb-interwork
AS_ASFLAGS		=	$(SHARED_FLAGS)	-defsym debug=0	-defsym	appver='$(EXECUTABLE_VERSION)'	-defsym ASM_OPCODES='1'	-defsym	ARM9='1'
CC_ASFLAGS		=	$(SHARED_FLAGS)	-Wall	-DNDS -DASM_OPCODES -DARM9	-fno-common	-mtune=arm946e-s	-marm	-O2 -ffast-math -fomit-frame-pointer	-Ddebug=0	-Dappver='$(EXECUTABLE_VERSION)'
BUILD			= 	$(BUILD_ARM9)
DIR_SRC			=	$(DIRS_ARM9_SRC)

#--gc-sections: allows relocatable sections linked (already) to be included, otherwise unused sections are discarded.
#check arm9.map "Discarded input sections" to see --gc-sections in action.
LDFLAGS	=	-specs=$(TARGET_LIBRARY_LINKER_PATH_LD)nds_arm9_ld.specs	-Wl,--no-gc-sections,-Map,$(MAPFILE)


################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS = 	$(objs_arm9)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG = 	$(addprefix $(OBJDIR)/, $(ELF_ARM9))


all: $(OBJDIR)|	$(TARGET_LIBRARY_CRT0_FILE_9).o	$(BINSTRIP_RULE_9)
#all: debug

#Check $(OBJDIR) if not exists: create it
$(OBJDIR):
	@echo "Folder $(OBJDIR) does not exist. Creating"
	mkdir -p $@

#Do C Compile now!
$(OBJDIR)/%.o : %.c
	-@echo 'C Compile start: $@'
	$(CC)	$(CC_ASFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	-nostdinc	$(DIRLIBS_PROHIBITED)	-o $@ -c $<
	-@echo 'C Compile End: $@'

#Do C++ (G++) Compile now!
$(OBJDIR)/%.o : %.cpp
	-@echo 'C++ Compile start: $@'
	$(G++)	$(CC_ASFLAGS)	$(CPP_FLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	-nostdinc	$(DIRLIBS_PROHIBITED)	-o $@ -c $<
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
	$(G++)	-x assembler-with-cpp	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(CC_ASFLAGS)	-o $@ -c $<
	-@echo 'S Compile end: $@'

#Special rule: compile crt0 for arm9 entrypoint
$(TARGET_LIBRARY_CRT0_FILE_9).o:	$(TARGET_LIBRARY_LINKER_FILE_9)
	-@echo 'S Compile start: crt0s'
	$(G++)	-x assembler-with-cpp	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(CC_ASFLAGS)	-o $(OBJDIR)/$@ -c $<
	cp	-r $(TARGET_LIBRARY_LINKER_PATH_LD)nds_arm9_ld.ld $(CURDIR)
	cp	-r $(TARGET_LIBRARY_LINKER_PATH_LD)nds_arm9_ld.specs $(CURDIR)
	-@echo 'S Compile end: crt0s'

#Linkage. 
$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJECTS))
	-@echo 'Entering Linking phase'
	$(G++) $^ $(LDFLAGS)  $(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_SHARED)	-L	$(LIBPATHNEWLIB)	-l:libcnano9.a	-l:libtoolchaingen9.a	-o $@
	-@echo 'Leaving Linking phase. Output: $@'

#keep only binary sections from elf. --only-section
$(BINSTRIP_RULE_9): $(OBJPROG)
	-@echo 'Binary-only strip start'
	$(OBJCOPY) 	-O binary	$<	$@
	-@echo 'Binary-only strip finish: $<	->	$@'

################################################################################################

clean:
	-@echo 'cleanup $(DIR_ARM9)'
	-@rm -fr $(OBJDIR)	$(BINSTRIP_RULE_9)	$(MAPFILE)	$(CURDIR)/nds_arm9_ld.specs	$(CURDIR)/nds_arm9_ld.ld