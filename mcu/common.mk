# Name of executables
ELF=$(BUILD)/$(PROJECT).elf
BIN=$(BUILD)/$(PROJECT).bin

# Flags
CPPFLAGS += -I$(OPENWARE)/LibSource -I$(OPENWARE)/Source -ISrc

# Tool path
TOOLROOT ?= ~/bin/gcc-arm-none-eabi-7-2017-q4-major/bin/

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
STFLASH ?= ~/bin/st-flash

MCU_ADDRESS = 0x8000000
# MCU_ADDRESS = 0x8020000

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

# Build executable 
$(ELF) : $(OBJS) $(LDSCRIPT)
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

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

clean:
	@rm -f $(OBJS) $(BUILD)/*.d $(ELF) $(CLEANOTHER) $(BIN) $(ELF:.elf=.s) gdbscript

debug: $(ELF)
	@$(GDB) -ex "target extended localhost:4242" -ex "load $(ELF)" $(ELF)

flash:
	@$(STFLASH) write $(BIN) $(MCU_ADDRESS)

stlink:
	@$(GDB) -ex "target extended localhost:4242" $(ELF)

bin: $(BIN)
	@echo Successfully built $(CONFIG) firmware in $(BIN)

map : $(OBJS) $(LDSCRIPT)
	@$(LD) $(LDFLAGS) -Wl,-Map=$(ELF:.elf=.map) $(OBJS) $(LDLIBS)

as: $(ELF)
	@$(OBJDUMP) -S $(ELF) > $(ELF:.elf=.s)

dfu: $(BIN)
	$(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0 -s $(MCU_ADDRESS):leave -D $(BIN)
	@echo Uploaded $(BIN) to firmware

size: $(ELF) $(BIN)
	@$(NM) --print-size --size-sort $(ELF) | grep -v '^08'| tail -n 10
	@$(SIZE) $(ELF)
	@ls -sh $(BIN)

# pull in dependencies
-include $(OBJS:.o=.d)
