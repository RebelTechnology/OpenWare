# object files

OBJS += $(DRIVERS)/Src/stm32f4xx_hal.o
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_rcc.o
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
OBJS += $(DRIVERS)/Src/stm32f4xx_hal_spi.o
# OBJS += $(DRIVERS)/Src/stm32f4xx_hal_rng.o
# OBJS += $(DRIVERS)/Src/stm32f4xx_hal_qspi.o
# OBJS += $(DRIVERS)/Src/stm32f4xx_hal_dma2d.o

# optionals
OBJS_UART = $(DRIVERS)/Src/stm32f4xx_hal_uart.o
OBJS_DAC = $(DRIVERS)/Src/stm32f4xx_hal_dac.o
OBJS_DAC += $(DRIVERS)/Src/stm32f4xx_hal_dac_ex.o
OBJS_CRC = $(DRIVERS)/Src/stm32f4xx_hal_crc.o
OBJS_RTC = $(DRIVERS)/Src/stm32f4xx_hal_rtc.o
OBJS_RTC += $(DRIVERS)/Src/stm32f4xx_hal_pwr.o
OBJS_RTC += $(DRIVERS)/Src/stm32f4xx_hal_rtc_ex.o
OBJS_IWDG = $(DRIVERS)/Src/stm32f4xx_hal_iwdg.o

# required by OWL 2
OBJS_SAI = $(DRIVERS)/Src/stm32f4xx_hal_sai.o
OBJS_SAI += $(DRIVERS)/Src/stm32f4xx_hal_sai_ex.o

# external SDRAM
OBJS_SDRAM = $(DRIVERS)/Src/stm32f4xx_hal_sdram.o
OBJS_SDRAM += $(DRIVERS)/Src/stm32f4xx_ll_fmc.o

# required by OWL 1
OBJS_SRAM = $(DRIVERS)/Src/stm32f4xx_hal_sram.o
OBJS_SRAM += $(DRIVERS)/Src/stm32f4xx_ll_fsmc.o
OBJS_I2S = $(DRIVERS)/Src/stm32f4xx_hal_i2s.o
OBJS_I2S += $(DRIVERS)/Src/stm32f4xx_hal_i2s_ex.o
OBJS_I2S += $(DRIVERS)/Src/stm32f4xx_hal_i2c.o
OBJS_I2S += $(DRIVERS)/Src/stm32f4xx_hal_i2c_ex.o

# required by Noctua
OBJS_I2C = $(DRIVERS)/Src/stm32f4xx_hal_i2c.o
OBJS_I2C += $(DRIVERS)/Src/stm32f4xx_hal_i2c_ex.o

### USB Device Library ###
OBJS_USBD = $(DRIVERS)/Src/stm32f4xx_hal_pcd.o
OBJS_USBD += $(DRIVERS)/Src/stm32f4xx_hal_pcd_ex.o
OBJS_USBD += $(DRIVERS)/Src/stm32f4xx_ll_usb.o
OBJS_USBD += $(USB_DEVICE_FILE)/Core/Src/usbd_core.o
OBJS_USBD += $(USB_DEVICE_FILE)/Core/Src/usbd_ioreq.o
OBJS_USBD += $(USB_DEVICE_FILE)/Core/Src/usbd_ctlreq.o
# OBJS_USBD += $(USB_DEVICE_FILE)/Class/AUDIO/Src/usbd_audio.o

### USB Host Library ###
OBJS_USBH = $(DRIVERS)/Src/stm32f4xx_hal_hcd.o
OBJS_USBH += $(USB_HOST_FILE)/Core/Src/usbh_core.o
OBJS_USBH += $(USB_HOST_FILE)/Core/Src/usbh_pipes.o
OBJS_USBH += $(USB_HOST_FILE)/Core/Src/usbh_ioreq.o
OBJS_USBH += $(USB_HOST_FILE)/Core/Src/usbh_ctlreq.o

### CMSIS DSP Library ####
OBJS_DSP = $(DSPLIB)/FastMathFunctions/arm_sin_f32.o
OBJS_DSP += $(DSPLIB)/FastMathFunctions/arm_cos_f32.o
OBJS_DSP += $(DSPLIB)/CommonTables/arm_common_tables.o

OBJS_DSP += $(DSPLIB)/TransformFunctions/arm_cfft_f32.o
OBJS_DSP += $(DSPLIB)/TransformFunctions/arm_cfft_radix8_f32.o
OBJS_DSP += $(DSPLIB)/TransformFunctions/arm_bitreversal.o
OBJS_DSP += $(DSPLIB)/TransformFunctions/arm_rfft_fast_f32.o
OBJS_DSP += $(DSPLIB)/TransformFunctions/arm_rfft_fast_init_f32.o
OBJS_DSP += $(DSPLIB)/CommonTables/arm_const_structs.o

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
OBJS_OS = $(FREERTOS_DIR)/CMSIS_RTOS/cmsis_os.o
# OBJS_OS = $(FREERTOS_DIR)/CMSIS_RTOS/cmsis_os2.o
# OBJS_OS += $(FREERTOS_DIR)/portable/GCC/ARM_CM7/r0p1/port.o
OBJS_OS += $(FREERTOS_DIR)/portable/GCC/ARM_CM4F/port.o
OBJS_OS += $(FREERTOS_DIR)/tasks.o
OBJS_OS += $(FREERTOS_DIR)/timers.o
OBJS_OS += $(FREERTOS_DIR)/queue.o
OBJS_OS += $(FREERTOS_DIR)/list.o
OBJS_OS += $(FREERTOS_DIR)/croutine.o
OBJS_OS += $(FREERTOS_DIR)/portable/MemMang/heap_4.o
