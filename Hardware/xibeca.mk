# Debug / Release
CONFIG ?= Release
ifeq ($(CONFIG),Debug)
  CPPFLAGS = -Os -g3 -Wall -Wcpp -Wunused-function -DDEBUG # -DUSE_FULL_ASSERT
else ifeq ($(CONFIG),Release)
  CPPFLAGS = -O3
else
  $(error Invalid CONFIG=$(CONFIG))
endif

# compile with semihosting if Debug is selected
ifeq ($(CONFIG),Debug)
  LDLIBS += -lrdimon
  LDFLAGS += -specs=rdimon.specs
else
  CPPFLAGS += -nostdlib -nostartfiles -fno-builtin -ffreestanding
  C_SRC += libnosys_gnu.c
  LDFLAGS += --specs=nano.specs
endif

# Compilation Flags
LDFLAGS += -Wl,--gc-sections
LDSCRIPT ?= $(OPENWARE)/Hardware/xibeca.ld
LDLIBS += -lc -lm
CPPFLAGS += -Wdouble-promotion -Werror=double-promotion
CPPFLAGS += -fdata-sections -ffunction-sections
CPPFLAGS += -fno-builtin -ffreestanding
LDFLAGS += -fno-builtin -ffreestanding
CXXFLAGS = -fno-rtti -fno-exceptions -std=gnu++17
CFLAGS  += -std=gnu99
ARCH_FLAGS = -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16
ARCH_FLAGS += -fsingle-precision-constant
DEF_FLAGS = -DSTM32H750xx -DSTM32H7xx -DARM_MATH_CM7
DEF_FLAGS += -D__FPU_PRESENT=1
S_SRC = $(OPENWARE)/Source/startup_owl3.s
