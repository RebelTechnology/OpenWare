# Library path
LIBROOT=$(OPENWARE)/Libraries/Drivers

# Build path
BUILD=$(BUILDROOT)/Build

# Tool path
OPENOCD ?= openocd -f $(OPENWARE)/Hardware/openocd_h7.cfg

# Code Paths
DRIVERS=$(LIBROOT)/STM32H7xx_HAL_Driver
CMSIS_DEVICE=$(LIBROOT)/CMSIS/Device/ST/STM32H7xx
CMSIS_CORE=$(LIBROOT)/CMSIS/Include
DSPLIB=$(LIBROOT)/CMSIS/DSP/Source
USB_DEVICE_FILE=$(OPENWARE)/Libraries/Middlewares/ST/STM32_USB_Device_Library
USB_HOST_FILE=$(OPENWARE)/Libraries/Middlewares/ST/STM32_USB_Host_Library
USB_OTG_FILE=$(LIBROOT)/STM32_USB_OTG_Driver
FREERTOS_DIR=$(OPENWARE)/Libraries/Middlewares/Third_Party/FreeRTOS/Source

INC_FLAGS = -I$(CMSIS_CORE) -I$(CMSIS_DEVICE)/Include -I$(DRIVERS)/Inc 
INC_FLAGS += -I$(OPENWARE)/Source -I$(BUILDROOT)/Inc
INC_FLAGS += -I$(USB_HOST_FILE)/Core/Inc
INC_FLAGS += -I$(USB_DEVICE_FILE)/Core/Inc
INC_FLAGS += -I$(USB_OTG_FILE)/inc
INC_FLAGS += -I$(FREERTOS_DIR)/include
INC_FLAGS += -I$(FREERTOS_DIR)/portable/GCC/ARM_CM7/r0p1
INC_FLAGS += -I$(FREERTOS_DIR)/CMSIS_RTOS
INC_FLAGS += -I$(LIBROOT)/CMSIS/DSP/Include

CPPFLAGS += -mtune=cortex-m7
CFLAGS += $(ARCH_FLAGS) $(INC_FLAGS) $(DEF_FLAGS)
CXXFLAGS += $(ARCH_FLAGS) $(INC_FLAGS) $(DEF_FLAGS)
LDFLAGS += -T$(LDSCRIPT) $(ARCH_FLAGS)

include $(OPENWARE)/Hardware/h7-libs.mk
include $(OPENWARE)/Hardware/common.mk
