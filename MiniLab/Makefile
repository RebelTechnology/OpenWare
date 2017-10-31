PROJECT = MiniLab
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/STM32F439ZG.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += sdram.c
C_SRC += cs4272.c
S_SRC = startup_stm32f439xx.s

include $(OPENWARE)/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(USB_HOST_FILE)/Core/Src/usbh_core.o
OBJS += $(USB_HOST_FILE)/Core/Src/usbh_pipes.o
OBJS += $(USB_HOST_FILE)/Core/Src/usbh_ioreq.o
OBJS += $(USB_HOST_FILE)/Core/Src/usbh_ctlreq.o
OBJS += $(USB_HOST_FILE)/Class/AUDIO/Src/usbh_audio.o

include $(OPENWARE)/f4.mk
