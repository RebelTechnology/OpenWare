######################################
# target
######################################
TARGET = midi-ble


######################################
# building variables
######################################
# Debug / Release
ifndef CONFIG
  CONFIG = Release
endif

ifeq ($(CONFIG),Debug)
  CPPFLAGS = -ggdb -g3 -DDEBUG
  BUILD_DIR = build/debug
endif

ifeq ($(CONFIG),Release)
  CPPFLAGS = -O2 -DNDEBUG
  BUILD_DIR = build/release
endif

#######################################
# paths
#######################################

######################################
# source
######################################
# C sources
C_SOURCES =

C_SOURCES += $(wildcard src/*.c)
C_SOURCES += $(wildcard Application/*.c)

# C++ sources
CXX_SOURCES = $(wildcard Application/*.cpp)

# ASM sources
ASM_SOURCES =
# ASM_SOURCES = startup_stm32l152xe.s

include libs.mk


######################################
# firmware library
######################################
PERIFLIB_SOURCES = 


#######################################
# binaries
#######################################
BINPATH ?= ~/bin/gcc-arm-none-eabi-7-2018-q2-update/bin/
PREFIX = arm-none-eabi-
CC = $(BINPATH)$(PREFIX)gcc
CXX = $(BINPATH)$(PREFIX)g++
AS = $(BINPATH)$(PREFIX)gcc -x assembler-with-cpp
CP = $(BINPATH)$(PREFIX)objcopy
AR = $(BINPATH)$(PREFIX)ar
SZ = $(BINPATH)$(PREFIX)size
NM = $(BINPATH)$(PREFIX)nm
GDB = $(BINPATH)$(PREFIX)gdb
OBJDUMP = $(BINPATH)$(PREFIX)objdump
SIZE = $(BINPATH)$(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

ELF = $(BUILD_DIR)/$(TARGET).elf

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m0

# fpu
FPU = #-mfpu=fpv5-d16

# float-abi
FLOAT-ABI = -mfloat-abi=soft

# mcu
# MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
MCU = $(CPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DARM_CORTEX \
-DUSE_ARM \
-DST_LINKER_OPTION=1

C_DEFS += -DUSE_FULL_ASSERT
C_DEFS += -DHS_SPEED_XTAL=HS_SPEED_XTAL_32MHZ
C_DEFS += -DLS_SOURCE=LS_SOURCE_EXTERNAL_32KHZ 
C_DEFS += -DSMPS_INDUCTOR=SMPS_INDUCTOR_10uH

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  -Iinc
C_INCLUDES += -ILibrary/hal/inc
C_INCLUDES += -ILibrary/CMSIS/Include
C_INCLUDES += -ILibrary/BlueNRG1_Periph_Driver/inc
C_INCLUDES += -ILibrary/CMSIS/Device/ST/BlueNRG1/Include
C_INCLUDES += -ILibrary/Bluetooth_LE/inc

# compile gcc flags
CPPFLAGS += -nostdlib -nostartfiles -fno-builtin -ffreestanding -nodefaultlibs
# CPPFLAGS += -mtune=cortex-m7
# CPPFLAGS += -fpic
# CPPFLAGS += -fpie
CPPFLAGS += -fdata-sections
CPPFLAGS += -ffunction-sections

GIT_REVISION = $(shell git rev-parse --abbrev-ref HEAD) $(shell git rev-parse --short HEAD) $(CONFIG)
CPPFLAGS += -DGIT_REVISION='"$(GIT_REVISION)"'

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall $(CPPFLAGS)

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall $(CPPFLAGS)
CFLAGS += -std=gnu99

CXXFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall $(CPPFLAGS)
CXXFLAGS += -fno-rtti -fno-exceptions -std=gnu++14


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" # -MT"$(@:%.o=%.d)"
CXXFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" # -MT"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = BlueNRG1.ld

# libraries
LIBS = -lc -lm -lnosys
LIBS += Library/Bluetooth_LE/library/libbluenrg1_stack_lib_atollic.a

LIBDIR = 
LDFLAGS = $(MCU) -specs=nosys.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections
LDFLAGS += -lc -lrdimon -u _printf_float
# LDFLAGS += -u _printf_float

LDFLAGS += -flto
# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

.PHONY: clean size debug stlink flash

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CXX_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CXX_SOURCES)))

# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR) 
	$(CXX) -c $(CXXFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	

$(BUILD_DIR):
	mkdir $@		

as: $(ELF)
	@$(OBJDUMP) -S $(ELF) > $(ELF:.elf=.s)

size: $(ELF) $(BUILD_DIR)/$(TARGET).bin
	@$(NM) --print-size --size-sort $(ELF) | grep -v '^08'| tail -n 10
	@$(SIZE) $(ELF)
	@ls -sh $(BUILD_DIR)/$(TARGET).bin

debug: $(ELF)
	@$(GDB) -ex "target extended-remote localhost:3333" -ex "monitor reset hard" -ex "load" $(ELF)

stlink:
	@$(GDB) -ex "target extended-remote localhost:3333" $(ELF)

flash: # $(BUILD_DIR)/$(TARGET).bin
	openocd -f openocd.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

#######################################
# clean up
#######################################
clean:
	-rm -rf .dep $(BUILD_DIR)/*

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
