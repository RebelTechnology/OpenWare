# Oneiroi firmware

Firmware for Befaco's Oneiroi. Developed by Roberto Noris (<https://github.com/hirnlego>).

## Building and flashing the firmware

The firmware is flashed via USB MIDI thanks to the bootloader.

To build and flash the Oneiroi firmware you can either run

`./programFirmware.sh`

to do everything via command line or

```bash
make clean
make -j17 CONFIG=Release|| exit 1
FirmwareSender -in Build/Oneiroi.bin -flash `crc32 Build/Oneiroi.bin` -save Build/Oneiroi.syx || exit 1
```

to build the firmware and then flash Oneiroi.syx using the web tool

<https://pingdynasty.github.io/OwlWebControl/firmware.html>

## About the bootloader

To build and flash the Oneiroi bootloader to the OWL enter `../MidiBoot3` and run

```bash
make clean
make PLATFORM=Oneiroi CONFIG=Release
make PLATFORM=Oneiroi upload
```

Note that you will need a programmer like the ST-LINK v2.