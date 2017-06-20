PROJECT = Player
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

# Debug / Release
CONFIG ?= Debug
ifeq ($(CONFIG),Debug)
  CPPFLAGS = -g -Wall -Wcpp -Wunused-function -DDEBUG # -DUSE_FULL_ASSERT
  ASFLAGS  = -g
  CFLAGS   = -g
endif
ifeq ($(CONFIG),Release)
  CPPFLAGS = -O2
  ASFLAGS  = -O2
  CFLAGS   = -O2
endif

# Compilation Flags
LDFLAGS += -Wl,--gc-sections
LDSCRIPT = Src/STM32F746ZGTx_FLASH.ld
CPPFLAGS += --specs=nano.specs
CPPFLAGS += -DEXTERNAL_SRAM -DARM_CORTEX
# CPPFLAGS += -fpic -fpie
CPPFLAGS += -fdata-sections
CPPFLAGS += -ffunction-sections
#CPPFLAGS +=  -mno-unaligned-access
#CPPFLAGS  += -fno-omit-frame-pointer
CPPFLAGS += -nostdlib -nostartfiles -fno-builtin -ffreestanding
CXXFLAGS = -fno-rtti -fno-exceptions -std=c++11
CFLAGS  += -std=gnu99
# ARCH_FLAGS = -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16
# ARCH_FLAGS += -fsingle-precision-constant
# # DEF_FLAGS = -DUSE_STDPERIPH_DRIVER -DARM_MATH_CM4 -D__FPU_PRESENT -D__FPU_USED=1
# DEF_FLAGS = -DSTM32F745xx -DARM_MATH_CM7 -D__FPU_PRESENT

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += sdram.c
C_SRC += cs4272.c

include $(OPENWARE)/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o) $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(BUILD)/startup_stm32f439xx.o

include $(OPENWARE)/libs.mk
include $(OPENWARE)/f7.mk
