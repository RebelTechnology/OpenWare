PROJECT = ACDC
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

LDSCRIPT = $(OPENWARE)/Hardware/xibeca-big.ld
include $(OPENWARE)/Hardware/owl3.mk

C_SRC = $(wildcard Core/Src/*.c)
CPP_SRC = $(wildcard Core/Src/*.cpp)
C_SRC += $(OPENWARE)/Source/pcm3168a.c
C_SRC += $(OPENWARE)/Source/sdram-as4cXm32.c
C_SRC += $(OPENWARE)/Source/flash-qspi.c
S_SRC = $(BUILDROOT)/Core/Src/startup_stm32h743xx.s

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_SAI)
C_SRC += $(C_SRC_SDRAM)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_I2C)
C_SRC += $(C_SRC_QSPI)
C_SRC += $(C_SRC_MDMA)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_USBD_AUDIO)

include $(OPENWARE)/Hardware/h7.mk

dfu: Build/${PROJECT}.bin
	$(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8000000 -D $<
	@echo Uploaded $< to Xibeca bootsector
