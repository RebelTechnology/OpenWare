# AGENTS.md

Firmware for Rebel Technology OWL devices (STM32 F0/F1/F4/F7/H7), C/C++ built with GNU Make and `arm-none-eabi-gcc`.

## Layout
- `Source/` – shared firmware code (codecs, MIDI, USB, storage, program manager, UI).
- `Hardware/` – shared makefiles (`common.mk`, `f4.mk`, `h7.mk`, `owl*.mk`, `sources.mk`) and linker scripts.
- `Libraries/` – git submodule (OpenWareLibraries: HAL, CMSIS, FreeRTOS, USB). Run `git submodule update --init`.
- `<Device>/` (e.g. `Genius/`, `Magus/`, `Witch/`) – per-device project: `Makefile`, `Src/`, `Inc/` (or `Core/`), STM32CubeMX `.ioc`.
- `Tools/`, `Test/` – helper scripts and ad-hoc tests.

## Build
- Set `TOOLROOT` (in `Hardware/common.mk` or env) to the toolchain `bin/` path.
- Top level: `make help`, `make <device>` (e.g. `make genius`), `make all`, `CONFIG=Debug|Release`.
- In a project dir: `make all | clean | size | sysex | flash | debug`. Output goes to `<Device>/Build/`.

## Conventions
- Device-specific settings live in `<Device>/Inc/hardware.h`; shared code switches on those defines.
- Don't hand-edit CubeMX-generated sections outside `USER CODE` blocks.
- Firmware version is `FIRMWARE_VERSION` in `Source/device.h`.
