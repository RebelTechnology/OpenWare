#!/bin/bash
git checkout Makefile
git checkout Inc/usbd_desc.h Inc/usbh_conf.h
git checkout Src/stm32f4xx_it.c
git checkout Src/usb_host.c
mv Src/usb_device.c Src/usbd_conf.c Src/usbd_desc.c Src/usbh_conf.c usbd-hs/
rm -f Src/usbd_audio_if.c Inc/usbd_audio_if.h
