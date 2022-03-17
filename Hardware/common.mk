# Name of binaries
ELF=$(BUILD)/$(PROJECT).elf
BIN=$(BUILD)/$(PROJECT).bin
HEX=$(BUILD)/$(PROJECT).hex
SYX=$(BUILD)/$(PROJECT).syx

# Flags
GIT_REVISION = $(shell git rev-parse --abbrev-ref HEAD) $(shell git rev-parse --short HEAD) $(CONFIG)
CPPFLAGS += -DGIT_REVISION='"$(GIT_REVISION)"'
CPPFLAGS += -D__PROGRAM_START=1 # prevent compilation of __cmsis_start function

# Tool path
# TOOLROOT ?= ~/bin/gcc-arm-none-eabi-9-2020-q2-update/bin/

# Tools
CC=$(TOOLROOT)arm-none-eabi-gcc
CXX=$(TOOLROOT)arm-none-eabi-g++
LD=$(TOOLROOT)arm-none-eabi-gcc
AR=$(TOOLROOT)arm-none-eabi-ar
AS=$(TOOLROOT)arm-none-eabi-as
NM=$(TOOLROOT)arm-none-eabi-nm
RANLIB=$(TOOLROOT)arm-none-eabi-ranlib
GDB=$(TOOLROOT)arm-none-eabi-gdb
OBJCOPY=$(TOOLROOT)arm-none-eabi-objcopy
OBJDUMP=$(TOOLROOT)arm-none-eabi-objdump
SIZE=$(TOOLROOT)arm-none-eabi-size
MKDIR=mkdir
DFUUTIL ?= dfu-util
FIRMWARESENDER ?= FirmwareSender

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

all: build_dir bin

.PHONY: clean size debug flash attach all sysex bin build_dir

# Set up search path
OBJS = $(addprefix $(BUILD)/,$(notdir $(C_SRC:.c=.o)))
vpath %.c $(sort $(dir $(C_SRC)))
OBJS += $(addprefix $(BUILD)/,$(notdir $(CPP_SRC:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SRC)))
OBJS += $(addprefix $(BUILD)/,$(notdir $(S_SRC:.s=.o)))
vpath %.s $(sort $(dir $(S_SRC)))

# Create build directory
build_dir: $(BUILD)

$(BUILD):
	@$(MKDIR) $(BUILD)

# Build executable 
$(ELF) : $(OBJS) $(LDSCRIPT)
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# compile and generate dependency info
$(BUILD)/%.o: %.c Makefile | $(BUILD)
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD)/%.o: %.cpp Makefile | $(BUILD)
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD)/%.o: %.s
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

$(BUILD)/%.s: %.c
	@$(CC) -S $(CPPFLAGS) $(CFLAGS) $< -o $@

$(BUILD)/%.s: %.cpp
	@$(CXX) -S $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	@$(OBJCOPY) -O binary $< $@

$(BUILD)/%.hex : $(BUILD)/%.elf
	@$(OBJCOPY) -O ihex $< $@

$(BUILD)/%.syx: $(BUILD)/%.bin
	@$(FIRMWARESENDER) -q -d 0 -save $@ -in $< -flash `crc32 $<`

bin: $(BIN) $(HEX)
	@echo Built $(PROJECT) $(PLATFORM) $(CONFIG) firmware in $(BIN)

map : $(OBJS) $(LDSCRIPT)
	@$(LD) $(LDFLAGS) -Wl,-Map=$(ELF:.elf=.map) $(OBJS) $(LDLIBS)

as: $(ELF)
	@$(OBJDUMP) -S $(ELF) > $(ELF:.elf=.s)

size: $(ELF) $(BIN)
	@$(NM) --print-size --size-sort $(ELF) | grep -v '^08'| tail -n 10
	@$(SIZE) $(ELF)
	@ls -sh $(BIN)

clean:
	@rm -rf $(BUILD) $(CLEANOTHER)  gdbscript

flash:
	$(OPENOCD) -c "program Build/$(PROJECT).elf verify reset exit"

debug: $(ELF)
	@$(GDB) -ex "target extended-remote localhost:3333" -ex "monitor reset hard" -ex "monitor arm semihosting enable" -ex "load" $(ELF)

attach:
	@$(GDB) -ex "target extended-remote localhost:3333" -ex "monitor reset hard" -ex "monitor arm semihosting enable" $(ELF)

sysex: $(SYX)

# pull in dependencies
-include $(OBJS:.o=.d)
