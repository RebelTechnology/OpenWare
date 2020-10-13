#!/bin/bash
git checkout Src/usb_device.c 
git checkout Src/usbd_conf.c Inc/usbd_conf.h
rm -f Src/usbd_audio_if.c Inc/usbd_audio_if.h
