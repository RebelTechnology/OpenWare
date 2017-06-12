PROJECT = MicroLab
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

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

LDFLAGS += -Wl,--gc-sections
# LDSCRIPT = Source/STM32F439ZG_FLASH.ld
# LDSCRIPT = Source/flash.ld
LDSCRIPT = Src/flash.ld

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

C_SRC = $(notdir $(wildcard Src/*.c)) # $(notdir $(wildcard ../Source/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp)) # $(notdir $(wildcard ../Source/*.cpp))
C_SRC += sdram.c
C_SRC += cs4272.c

include $(OPENWARE)/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o) $(CPP_SRC:%.cpp=Build/%.o)

# OBJS += $(BUILD)/startup_stm32f439xx.o
OBJS += Src/startup_stm32f439xx.o

include $(OPENWARE)/libs.mk
include $(OPENWARE)/f4.mk
