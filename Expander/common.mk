# name of executable
ELF=$(BUILD)/Expander.elf                    
BIN=$(BUILD)/Expander.bin

# Tool path
TOOLROOT ?= ~/bin/gcc-arm-none-eabi-9-2020-q2-update/bin/

# Tools
CC=$(TOOLROOT)/arm-none-eabi-gcc
CXX=$(TOOLROOT)/arm-none-eabi-g++
LD=$(TOOLROOT)/arm-none-eabi-gcc
AR=$(TOOLROOT)/arm-none-eabi-ar
AS=$(TOOLROOT)/arm-none-eabi-as
RANLIB=$(TOOLROOT)/arm-none-eabi-ranlib
GDB=$(TOOLROOT)/arm-none-eabi-gdb
OBJCOPY=$(TOOLROOT)/arm-none-eabi-objcopy
OBJDUMP=$(TOOLROOT)/arm-none-eabi-objdump
SIZE=$(TOOLROOT)/arm-none-eabi-size

# Set up search path
vpath %.cpp $(TEMPLATEROOT)/Source
vpath %.c $(TEMPLATEROOT)/Source
vpath %.s $(TEMPLATEROOT)/Source
vpath %.cpp $(TEMPLATEROOT)/MDK-ARM
vpath %.c $(TEMPLATEROOT)/MDK-ARM
vpath %.s $(TEMPLATEROOT)/MDK-ARM
vpath %.cpp $(TEMPLATEROOT)/Src
vpath %.c $(TEMPLATEROOT)/Src
vpath %.s $(TEMPLATEROOT)/Src
vpath %.c $(TEMPLATEROOT)/Libraries/syscalls
vpath %.c $(PERIPH_FILE)/src
vpath %.c $(PERIPH_FILE)/inc
vpath %.c $(DEVICE)
vpath %.c $(USB_DEVICE_FILE)/Core/src
vpath %.c $(USB_OTG_FILE)/src/

all: bin

# Build executable 
$(ELF) : $(OBJS) $(LDSCRIPT)
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# compile and generate dependency info
$(BUILD)/%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@
	@$(CC) -MM -MT"$@" $(CPPFLAGS) $(CFLAGS) $< > $(@:.o=.d)

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@
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
	@rm -f $(OBJS) $(BUILD)/*.d $(ELF) $(CLEANOTHER) $(BIN) $(ELF:.elf=.s) $(OBJS:.o=.s) gdbscript

flash:
	$(OPENOCD) -c "program Build/$(PROJECT).elf verify reset exit"

debug: $(ELF)
	@$(GDB) -ex "target extended-remote localhost:3333" -ex "monitor reset hard" -ex "monitor arm semihosting enable" -ex "load" $(ELF)

attach:
	@$(GDB) -ex "target extended-remote localhost:3333" -ex "monitor reset hard" -ex "monitor arm semihosting enable" $(ELF)

etags:
	find $(PERIPH_FILE) -type f -iname "*.[ch]" | xargs etags --append
	find $(DEVICE) -type f -iname "*.[ch]" | xargs etags --append
	find $(CORE) -type f -iname "*.[ch]" | xargs etags --append
	find $(DISCOVERY_FILE) -type f -iname "*.[ch]" | xargs etags --append
	find . -type f -iname "*.[ch]" | xargs etags --append

bin: $(BIN)
	@echo Successfully built $(CONFIG) firmware in $(BIN)

map : $(OBJS) $(LDSCRIPT)
	$(LD) $(LDFLAGS) -Wl,-Map=$(ELF:.elf=.map) $(OBJS) $(LDLIBS)

as: $(ELF)
	$(OBJDUMP) -S $(ELF) > $(ELF:.elf=.s)

dfu: $(BIN)
	$(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8000000:leave -D $(BIN)
	@echo Uploaded $(BIN) to firmware

# pull in dependencies
-include $(OBJS:.o=.d)
