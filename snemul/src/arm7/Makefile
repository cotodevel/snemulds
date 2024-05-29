#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

-include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
BUILD		:=	bin
SOURCES		:=	source
INCLUDES	:=	

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb-interwork

CFLAGS	:=	-Wall -Os\
			-mcpu=arm7tdmi -mtune=arm7tdmi -fomit-frame-pointer\
			-ffast-math \
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM7
CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=$(CURDIR)/../pocketspc_arm7.specs -g $(ARCH) -mno-fpu -Wl,-Map,$(notdir $*).map
#LDFLAGS	=	-specs=ds_arm7.specs -g $(ARCH) -mno-fpu -Wl,-Map,$(notdir $*).map

LIBS	:= -lnds7

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS)
 
  
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export ARM7BIN	:=	$(TOPDIR)/$(TARGET).arm7
export ARM7ELF	:=	$(CURDIR)/$(TARGET).arm7.elf
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
 
export CC		:=	$(PREFIX)gcc
export CXX		:=	$(PREFIX)g++
export AR		:=	$(PREFIX)ar
export OBJCOPY	:=	$(PREFIX)objcopy
 
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))

export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o) $(SPPFILES:.S=.o)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)
 
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) *.elf
 
 
#---------------------------------------------------------------------------------
else
 
DEPENDS	:=	$(OFILES:.o=.d)
 
.PHONY: ../bin/pocketspc.arm7
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

../bin/pocketspc.arm7	:	../bin/pocketspc.elf
	$(OBJCOPY) -O binary pocketspc.elf pocketspc.arm7
	@echo succesfully built pocketspc arm7 core, enjoy!

../bin/pocketspc.elf	:	$(OFILES)
	@echo linking pocketspc
	$(CC)  $(ASFLAGS) -c ../pocketspc_arm7_crt0.s -o../bin/pocketspc_arm7_crt0.o
	$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o ../bin/pocketspc.elf

#---------------------------------------------------------------------------------
# Compile Targets for C/C++
#---------------------------------------------------------------------------------
 
%.o : %.S
	echo $(notdir $<)
	$(CC) -MM $(CFLAGS) -o $*.d $<
	$(CC)  $(ASFLAGS) -c $< -o$@

-include $(DEPENDS)
 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
