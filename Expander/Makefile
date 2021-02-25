TEMPLATEROOT = .

ifndef CONFIG
  CONFIG = Debug
endif

ifeq ($(CONFIG),Debug)
  CPPFLAGS = -g -Wall -Wcpp -Wunused-function -DDEBUG -DUSE_FULL_ASSERT
  ASFLAGS  = -g
  CFLAGS   = -g
endif

ifeq ($(CONFIG),Release)
  CPPFLAGS = -O2
  ASFLAGS  = -O2
  CFLAGS   = -O2
endif

LDFLAGS += -Wl,--gc-sections
LDSCRIPT ?= $(TEMPLATEROOT)/Source/stm32_flash.ld

CPPFLAGS += --specs=nano.specs
# CPPFLAGS += -DEXTERNAL_SRAM -DARM_CORTEX
# CPPFLAGS += -fpic -fpie
CPPFLAGS += -fdata-sections
CPPFLAGS += -ffunction-sections
#CPPFLAGS +=  -mno-unaligned-access
#CPPFLAGS  += -fno-omit-frame-pointer
CPPFLAGS += -nostdlib -nostartfiles -fno-builtin -ffreestanding
CPPFLAGS += -mtune=cortex-m7
CXXFLAGS = -fno-rtti -fno-exceptions -std=gnu++11
CFLAGS  += -std=gnu99

C_SRC = $(notdir $(wildcard Src/*.c)) $(notdir $(wildcard Source/*.c)) # $(notdir $(wildcard MDK-ARM/*.c))
# Libraries/STM32Cube_FW_F1_V1.4.0/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c

CPP_SRC = $(notdir $(wildcard Src/*.cpp)) $(notdir $(wildcard Source/*.cpp)) #$ (notdir $(wildcard MDK-ARM/*.cpp)) 

OBJS = $(C_SRC:%.c=Build/%.o) $(CPP_SRC:%.cpp=Build/%.o)
# OBJS += $(BUILD)/startup_stm32f756xx.o
OBJS += $(BUILD)/startup_stm32f10x_md_vl.o # medium density value line

include $(TEMPLATEROOT)/libs.mk
include $(TEMPLATEROOT)/f1.mk
