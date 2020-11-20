#!/bin/bash
git checkout Inc/usbh_conf.h Inc/usbd_conf.h
git checkout Src/usbd_conf.c
git checkout Src/usb_device.c Src/usb_host.c Makefile
rm -f Src/usbd_audio_if.c Inc/usbd_audio_if.h
