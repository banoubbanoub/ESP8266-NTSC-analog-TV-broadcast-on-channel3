# tnx to mamalala
# Changelog
# Changed the variables to include the header file directory
# Added global var for the XTENSA tool root
#
# This make file still needs some work.
#
# Updated for SDK 0.9.2
#
# Output directors to store intermediate compiled files
# relative to the project directory
ROOT		= $(HOME)/tmp/esp-shit/opt/Espressif
BUILD_BASE	= build
FW_BASE		= firmware

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= $(ROOT)/crosstool-NG/builds/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ROOT)/ESP8266_SDK

#Esptool.py path and port
ESPTOOL		?= esptool.py
ESPPORT		?= /dev/ttyUSB0

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
MODULES		= driver user
EXTRA_INCDIR    = include $(ROOT)/include/ 

# libraries used in this project, mainly provided by the SDK
LIBS		= c gcc hal phy net80211 lwip wpa upgrade upgrade_ssl main pp

# compiler flags using during compilation of source files
CFLAGS		= -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH
CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# we create two different files for uploading into the flash
# these are the names and options to generate them
FW_FILE_1	= 0x00000
FW_FILE_1_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec
FW_FILE_2	= 0x40000
FW_FILE_2_ARGS	= -es .irom0.text $@ -ec

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
CXX		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-g++
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc



####
#### no user configurable options below here
####
FW_TOOL		?= /usr/bin/esptool
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*))
C_OBJ		:= $(patsubst %.c,%.o,$(SRC))
CXX_OBJ		:= $(patsubst %.cpp,%.o,$(C_OBJ))
OBJ		:= $(patsubst %.o,$(BUILD_BASE)/%.o,$(CXX_OBJ))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2).bin)

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
$1/%.o: %.cpp
	$(vecho) "C+ $$<"
	$(Q) $(CXX) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CXXFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)

$(FW_FILE_1): $(TARGET_OUT)
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_1_ARGS)

$(FW_FILE_2): $(TARGET_OUT)
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_2_ARGS)

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

flash: firmware/0x00000.bin firmware/0x40000.bin
	-$(ESPTOOL) --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin
	sleep 3
	-$(ESPTOOL) --port $(ESPPORT) write_flash 0x40000 firmware/0x40000.bin

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)


	$(Q) rm -f $(FW_FILE_1)
	$(Q) rm -f $(FW_FILE_2)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
