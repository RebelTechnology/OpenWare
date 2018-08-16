#!/bin/bash
# git checkout Src/usbd_desc.c 
git checkout Src/usb_device.c 
git checkout Middlewares/Third_Party/FreeRTOS # Inc/FreeRTOSConfig.h
git checkout Src/usb_host.c Inc/usb_host.h 
rm -f Src/usbd_audio_if.c Inc/usbd_audio_if.h
rm -f Src/*.tmp
