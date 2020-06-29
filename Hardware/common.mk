# Name of binaries
ELF=$(BUILD)/$(PROJECT).elf
BIN=$(BUILD)/$(PROJECT).bin
HEX=$(BUILD)/$(PROJECT).hex
SYX=$(BUILD)/$(PROJECT).syx

# Flags
CPPFLAGS += -I$(OPENWARE)/LibSource -I$(OPENWARE)/Source -ISrc
GIT_REVISION = $(shell git rev-parse --abbrev-ref HEAD) $(shell git rev-parse --short HEAD) $(CONFIG)
CPPFLAGS += -DGIT_REVISION='"$(GIT_REVISION)"'

# Tool path
TOOLROOT ?= ~/bin/gcc-arm-none-eabi-9-2019-q4-major/bin/

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
OPENOCD ?= openocd -f $(OPENWARE)/Hardware/openocd.cfg

# Set up search path
vpath %.s $(BUILDROOT)/Src
vpath %.c $(BUILDROOT)/Src
vpath %.cpp $(BUILDROOT)/Src
vpath %.c $(OPENWARE)/Source
vpath %.cpp $(OPENWARE)/Source
vpath %.c $(OPENWARE)/LibSource
vpath %.cpp $(OPENWARE)/LibSource
vpath %.c $(OPENWARE)/Libraries/syscalls

all: bin

.PHONY: clean size debug flash attach all sysex

# Build executable 
$(ELF) : $(OBJS) $(LDSCRIPT)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# compile and generate dependency info
$(BUILD)/%.o: %.c
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@
	@$(CC) -MM -MT"$@" $(CPPFLAGS) $(CFLAGS) $< > $(@:.o=.d)

$(BUILD)/%.o: %.cpp
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@
	@$(CXX) -MM -MT"$@" $(CPPFLAGS) $(CXXFLAGS) $< > $(@:.o=.d)

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
	FirmwareSender -save $@ -in $< -flash `crc32 $<`

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
	@rm -f $(OBJS) $(BUILD)/*.d $(ELF) $(CLEANOTHER) $(BIN) $(ELF:.elf=.s) gdbscript

flash:
	$(OPENOCD) -c "program Build/$(PROJECT).elf verify reset exit"

debug: $(ELF)
	@$(GDB) -ex "target extended-remote localhost:3333" -ex "monitor reset hard" -ex "monitor arm semihosting enable" -ex "load" $(ELF)

attach:
	@$(GDB) -ex "target extended-remote localhost:3333" -ex "monitor reset hard" -ex "monitor arm semihosting enable" $(ELF)

sysex: $(SYX)

# pull in dependencies
-include $(OBJS:.o=.d)
