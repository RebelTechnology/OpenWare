# object files
# OBJS += $(PERIPH) 

# C_SRC += font.c
# C_SRC += gpio.c

# # OBJS += $(OPENWARE)/Source/font.o
# # OBJS += $(OPENWARE)/Source/gpio.o
# OBJS += $(OPENWARE)/Source/crc32.o
# OBJS += $(OPENWARE)/Source/eepromcontrol.o
# OBJS += $(OPENWARE)/Source/errorhandlers.o
# # OBJS += $(OPENWARE)/Source/font.o
# # OBJS += $(OPENWARE)/Source/gpio.o
# OBJS += $(OPENWARE)/Source/sysex.o
# OBJS += $(OPENWARE)/Source/sramalloc.o
# OBJS += $(OPENWARE)/Source/cs4272.o
# OBJS += $(OPENWARE)/Source/sdram.o
# OBJS += $(OPENWARE)/Source/usbd_midi.o
# OBJS += $(OPENWARE)/Source/usbd_midi_if.o

# OBJS += $(OPENWARE)/Source/ApplicationSettings.o
# OBJS += $(OPENWARE)/Source/MidiHandler.o
# OBJS += $(OPENWARE)/Source/Codec.o
# OBJS += $(OPENWARE)/Source/MidiReader.o
# OBJS += $(OPENWARE)/Source/DigitalBusHandler.o       
# OBJS += $(OPENWARE)/Source/operators.o
# OBJS += $(OPENWARE)/Source/DigitalBusReader.o	      
# OBJS += $(OPENWARE)/Source/Owl.o
# OBJS += $(OPENWARE)/Source/DigitalBusStreamReader.o  
# OBJS += $(OPENWARE)/Source/PatchRegistry.o
# OBJS += $(OPENWARE)/Source/FactoryPatches.o	      
# OBJS += $(OPENWARE)/Source/ProgramManager.o
# OBJS += $(OPENWARE)/Source/FlashStorage.o	      
# OBJS += $(OPENWARE)/Source/ServiceCall.o
# OBJS += $(OPENWARE)/Source/message.o		      
# OBJS += $(OPENWARE)/Source/StorageBlock.o
# OBJS += $(OPENWARE)/Source/MidiController.o

# OBJS += $(OPENWARE)/LibSource/PatchProcessor.o
# OBJS += $(OPENWARE)/LibSource/FloatArray.o
# OBJS += $(OPENWARE)/LibSource/StompBox.o
# OBJS += $(OPENWARE)/LibSource/basicmaths.o

OBJS += $(DRIVERS)/Src/stm32f4xx_hal.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_rcc.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_dac.o
# OBJS += $(DRIVERS)/Src/stm32f4xx_hal_i2c.o
# OBJS += $(DRIVERS)/Src/stm32f4xx_hal_i2c_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_dma.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_adc.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_gpio.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_adc_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_dma_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_pwr_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_rcc_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_flash.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_flash_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_tim_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_tim.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_cortex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_sai.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_sai_ex.o
# OBJS += $(DRIVERS)/Src/stm32f4xx_hal_rng.o
# OBJS += $(DRIVERS)/Src/stm32f4xx_hal_qspi.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_uart.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_spi.o
# OBJS += $(DRIVERS)/Src/stm32f4xx_hal_dma2d.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_hcd.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_pcd.o # USB Device
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_pcd_ex.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_sdram.o
OBJS += $(DRIVERS)/Src/stm32f4xx_ll_fmc.o
OBJS += $(DRIVERS)/Src/stm32f4xx_ll_usb.o

OBJS += $(USB_DEVICE_FILE)/Core/Src/usbd_core.o
OBJS += $(USB_DEVICE_FILE)/Core/Src/usbd_ioreq.o
OBJS += $(USB_DEVICE_FILE)/Core/Src/usbd_ctlreq.o
OBJS += $(USB_DEVICE_FILE)/Class/AUDIO/Src/usbd_audio.o

# OBJS += $(USB_HOST_FILE)/Core/Src/usbh_core.o
# OBJS += $(USB_HOST_FILE)/Core/Src/usbh_pipes.o
# OBJS += $(USB_HOST_FILE)/Core/Src/usbh_ioreq.o
# OBJS += $(USB_HOST_FILE)/Core/Src/usbh_ctlreq.o
# OBJS += $(USB_HOST_FILE)/Class/AUDIO/Src/usbh_audio.o

# OBJS += $(OPENWARE)/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f439xx.o
OBJS += $(OPENWARE)/Libraries/syscalls/libnosys_gnu.o

########

# OBJS += $(USB_DEVICE) $(USB_OTG)
OBJS += $(DSPLIB)/FastMathFunctions/arm_sin_f32.o
OBJS += $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
OBJS += $(DSPLIB)/CommonTables/arm_common_tables.o

OBJS += $(DSPLIB)/TransformFunctions/arm_cfft_f32.o
OBJS += $(DSPLIB)/TransformFunctions/arm_cfft_radix8_f32.o
OBJS += $(DSPLIB)/TransformFunctions/arm_bitreversal.o
OBJS += $(DSPLIB)/TransformFunctions/arm_rfft_fast_f32.o
OBJS += $(DSPLIB)/TransformFunctions/arm_rfft_fast_init_f32.o
OBJS += $(DSPLIB)/CommonTables/arm_const_structs.o

# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_init_f32.o
# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_f32.o

# OBJS += $(DSPLIB)/SupportFunctions/arm_copy_f32.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q31.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q31_to_float.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q15.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q15_to_float.o

# OBJS += $(DSPLIB)/FastMathFunctions/arm_sin_f32.o
# OBJS += $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_f32.o
# OBJS += $(DSPLIB)/FilteringFunctions/arm_biquad_cascade_df1_init_f32.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q31.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q31_to_float.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_float_to_q15.o
# OBJS += $(DSPLIB)/SupportFunctions/arm_q15_to_float.o

### FreeRTOS ###
OBJS += $(FREERTOS_DIR)/CMSIS_RTOS/cmsis_os.o
# OBJS += $(FREERTOS_DIR)/portable/GCC/ARM_CM7/r0p1/port.o
OBJS += $(FREERTOS_DIR)/portable/GCC/ARM_CM4F/port.o
OBJS += $(FREERTOS_DIR)/tasks.o
OBJS += $(FREERTOS_DIR)/timers.o
OBJS += $(FREERTOS_DIR)/queue.o
OBJS += $(FREERTOS_DIR)/list.o
OBJS += $(FREERTOS_DIR)/croutine.o
OBJS += $(FREERTOS_DIR)/portable/MemMang/heap_4.o
