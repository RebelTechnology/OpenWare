PROJECT = EffectsBox
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl2.mk

C_SRC = main.c
C_SRC += freertos.c
C_SRC += stm32f4xx_hal_msp.c system_stm32f4xx.c stm32f4xx_it.c
C_SRC += usbd_conf.c usbd_desc.c usb_device.c 

# C_SRC += test.c EffectsBox_Test.c
# C_SRC += HAL_OLED.c
# C_SRC += usbd_midi.c usbd_midi_if.c

C_SRC += sdram.c
C_SRC += cs4272.c
C_SRC += ssd1309.c
CPP_SRC += ScreenBuffer.cpp ScreenBufferMono.cpp Graphics.cpp

include $(OPENWARE)/Hardware/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_SAI)
OBJS += $(OBJS_SDRAM)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_DSP)
OBJS += $(OBJS_OS)

include $(OPENWARE)/Hardware/f4.mk
