PROJECT = OwlPedal
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/mcu/STM32F407ZG.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += wm8731.c
S_SRC = startup_stm32f407xx.s

include $(OPENWARE)/mcu/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_i2s.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_i2s_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_i2c.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_i2c_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_sram.o
OBJS += $(DRIVERS)/Src/stm32f4xx_ll_fsmc.o

include $(OPENWARE)/mcu/f4.mk
