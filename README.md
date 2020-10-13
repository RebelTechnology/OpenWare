# OpenWare
Firmware for OWL devices

This repository contains firmware source code for several Rebel Technology devices and products, including Wizard, Alchemist and Magus.

Source libraries and drivers are contained in a git submodule. On first git checkout, do:

    git submodule init

To get the latest library version, do:

    git submodule update

To compile a project firmware, set the `TOOLROOT` in `Hardware/common.mk` and build with Gnu Make, either using the top level makefile (try `make help`) or by entering a project subdirectory.

Compiles with arm-gcc-none-eabi cross compiler. Debugging can be done with openocd.

STM32CubeMX `.ioc` files are available in the project subdirectories.
