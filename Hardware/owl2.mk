# Debug / Release
CONFIG ?= Debug
ifeq ($(CONFIG),Debug)
  CPPFLAGS = -g3 -Wall -Wcpp -Wunused-function -DDEBUG # -DUSE_FULL_ASSERT
  ASFLAGS  = -g3
  CFLAGS   = -g3
endif
ifeq ($(CONFIG),Release)
  CPPFLAGS = -O2
  ASFLAGS  = -O2
  CFLAGS   = -O2
endif

# Compilation Flags
LDFLAGS += -Wl,--gc-sections
LDSCRIPT ?= $(OPENWARE)/Hardware/owl2.ld
LDLIBS += -lc -lm
LDLIBS += -lrdimon
# CPPFLAGS += -specs=rdimon.specs
LDFLAGS += --specs=nano.specs
CPPFLAGS += -DEXTERNAL_SRAM -DARM_CORTEX
# CPPFLAGS += -fpic -fpie
CPPFLAGS += -fdata-sections
CPPFLAGS += -ffunction-sections
#CPPFLAGS +=  -mno-unaligned-access
#CPPFLAGS  += -fno-omit-frame-pointer
# CPPFLAGS += -nostdlib -nostartfiles -fno-builtin -ffreestanding
CXXFLAGS = -fno-rtti -fno-exceptions -std=gnu++11
CFLAGS  += -std=gnu99
ARCH_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH_FLAGS += -fsingle-precision-constant
DEF_FLAGS = -DSTM32F427xx -DARM_MATH_CM4
DEF_FLAGS += -D__FPU_PRESENT=1U
S_SRC = startup_stm32f427xx.s
