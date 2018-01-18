PROJECT = Magus
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/mcu/STM32F427ZI.mk

C_SRC = $(filter-out test.c,$(notdir $(wildcard Src/*.c)))
# C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += sdram.c
C_SRC += cs4272.c
C_SRC += usbh_midi.c
C_SRC += ssd1309.c
C_SRC += HAL_MAX11300.c
C_SRC += HAL_TLC5946.c
C_SRC += HAL_Encoders.c
# C_SRC += HAL_OLED.c
CPP_SRC += ScreenBuffer.cpp ScreenBufferMono.cpp Graphics.cpp
S_SRC = startup_stm32f427xx.s

include $(OPENWARE)/mcu/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(USB_HOST_FILE)/Core/Src/usbh_core.o
OBJS += $(USB_HOST_FILE)/Core/Src/usbh_pipes.o
OBJS += $(USB_HOST_FILE)/Core/Src/usbh_ioreq.o
OBJS += $(USB_HOST_FILE)/Core/Src/usbh_ctlreq.o

include $(OPENWARE)/mcu/f4.mk
