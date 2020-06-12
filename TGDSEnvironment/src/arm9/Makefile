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

#Prepare ARM9 dirs
DIRLIBS_HDR = $(foreach dirres,$(DIRS_ARM9_HEADER),-I "$(CURDIR)/$(dirres)" )	$(DIRLIBS_ALLOWED)
DIRLIBS_SRC = $(foreach dirres,$(DIRS_ARM9_SRC),-I "$(CURDIR)/$(dirres)" )

dir_arm9_src_h=  $(foreach dir,$(DIRS_ARM9_HEADER), $(wildcard $(CURDIR)/$(RELATIVE_PATH)$(dir)*.h))
file_arm9_src_c=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm9_src_s=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm9_src_SUPP=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )
file_arm9_src_cpp=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.cpp))  )
file_arm9_src_bin=  $(foreach dir,$(DIRS_ARM9_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.bin))  )

# Object Target
objs_arm9:=	${file_arm9_src_bin:.bin=.o}	${file_arm9_src_c:.c=.o}	${file_arm9_src_s:.s=.o}	${file_arm9_src_SUPP:.S=.o}		${file_arm9_src_cpp:.cpp=.o}
d_objs_arm9:=	${file_arm9_src_c:.c=.d}	${file_arm9_src_s:.s=.d}	${file_arm9_src_SUPP:.S=.d}	${file_arm9_src_cpp:.cpp=.d}

#ARM9 specific definitions. -fno-common so we force initialized data to rodata
MAPFILE = $(DIR_ARM9).map
AS_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_AS_COMPILER_FLAGS_9)		$(GLOBAL_AS_COMPILER_FLAGS)
CC_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_CXX_COMPILER_FLAGS_9)	$(GLOBAL_CC_COMPILER_FLAGS)	-DASM_OPCODES	-fPIE
CPP_FLAGS		=	$(GLOBAL_SHARED_FLAGS_9)	$(SPECIFIC_CXX_COMPILER_FLAGS_9)	$(GLOBAL_CXX_COMPILER_FLAGS)	-DASM_OPCODES	-fPIE
BUILD			= 	$(BUILD_ARM9)
DIR_SRC			=	$(DIRS_ARM9_SRC)
################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS = 	$(objs_arm9)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG = 	$(addprefix $(OBJDIR)/, $(TARGET_LIBRARY_FILE_9))

all: $(OBJDIR)|$(OBJPROG)
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
	$(CPP)	$(CPP_FLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	-o $@ -c $<
	-@echo 'C Compile End: $@'

#Coto: Special "exception" rule. Assembler (.s) support ie: .ifdef/.endif (which differs from CPP-like syntax Assembler: ie: #ifdef/#endif by .S)
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
	$(CPP)	-x assembler-with-cpp	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(CC_ASFLAGS)	-o $@ -c $<
	-@echo 'S Compile end: $@'

#Builder.
$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJECTS))
	-@echo 'Entering Linking phase'
	$(AR) rvs	$@	$^
ifeq ($(TGDS_ENV),windows)
	-mkdir -p $(TARGET_LIBRARY_PATH) && cp	$(OBJPROG)	$(TARGET_LIBRARY_PATH)
	-mkdir -p $(TARGET_LIBRARY_HEADERS_9) && cp $(dir_arm9_src_h) $(TARGET_LIBRARY_HEADERS_9)
endif
ifeq ($(TGDS_ENV),linux)
	-sudo mkdir -p $(TARGET_LIBRARY_PATH) && sudo cp	$(OBJPROG)	$(TARGET_LIBRARY_PATH)
	-sudo mkdir -p $(TARGET_LIBRARY_HEADERS_9) && sudo cp $(dir_arm9_src_h) $(TARGET_LIBRARY_HEADERS_9)
endif
	-@echo 'Leaving Linking phase. $(OBJPROG)'
	
################################################################################################
clean:
	-@echo 'cleanup $(TARGET_LIBRARY_FILE_9)'
ifeq ($(TGDS_ENV),windows)
	-@rm -fr $(OBJDIR)	$(TARGET_LIBRARY_FILE_9)	$(MAPFILE)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)	$(TARGET_LIBRARY_HEADERS_9)
endif
ifeq ($(TGDS_ENV),linux)
	-@sudo rm -fr $(OBJDIR)	$(TARGET_LIBRARY_FILE_9)	$(MAPFILE)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)	$(TARGET_LIBRARY_HEADERS_9)
endif
