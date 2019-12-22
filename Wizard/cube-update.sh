#!/bin/bash
git checkout Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ # Inc/FreeRTOSConfig.h
git checkout Src/usb_host.c Inc/usb_host.h 
git checkout Inc/usbd_desc.h
git checkout Src/stm32f4xx_it.c
rm -f Src/usbd_audio_if.c Inc/usbd_audio_if.h
rm -f Src/usb_device.c
rm -f Src/usbd_conf.c Src/usbd_desc.c
rm -f Src/*.tmp
