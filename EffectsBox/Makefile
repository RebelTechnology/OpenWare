PROJECT = EffectsBox
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/mcu/STM32F427ZI.mk

# C_SRC = $(notdir $(wildcard Src/*.c))
C_SRC = freertos.c main.c
C_SRC += stm32f4xx_hal_msp.c system_stm32f4xx.c
C_SRC += usbd_conf.c usbd_desc.c usb_device.c 
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += sdram.c
C_SRC += cs4272.c
C_SRC += ssd1309.c
S_SRC = startup_stm32f427xx.s
CPP_SRC += ScreenBuffer.cpp ScreenBufferMono.cpp Graphics.cpp

include $(OPENWARE)/mcu/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_SAI)
OBJS += $(OBJS_SDRAM)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_DSP)
OBJS += $(OBJS_OS)

include $(OPENWARE)/mcu/f4.mk
