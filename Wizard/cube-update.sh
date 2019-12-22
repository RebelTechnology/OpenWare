#!/bin/bash
# git checkout Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/
mv Src/usb_device.c Src/usbd_conf.c Src/usbd_desc.c Src/usbh_conf.c usbd-hs/
git checkout Inc/usbd_desc.h
git checkout Src/stm32f4xx_it.c
rm -f Src/usbd_audio_if.c Inc/usbd_audio_if.h
rm -f Src/*.tmp
