#!/bin/bash
PROJECT=Oneiroi
HARDWARE=1,0,0
#CONFIG=Debug
CONFIG=Release

make clean
make -j17 CONFIG=$CONFIG || exit 1
FirmwareSender -in Build/${PROJECT}.bin -flash `crc32 Build/${PROJECT}.bin` -save Build/${PROJECT}.syx || exit 1
echo "Entering bootloader mode"
amidi -p hw:${HARDWARE} -S f07d527ef7 || exit 1
sleep 3
echo "Sending firmware"
amidi -p hw:${HARDWARE} -s Build/${PROJECT}.syx || exit 1
sleep 1
echo "Restart"
amidi -p hw:${HARDWARE} -S f07d527df7 || exit 1
echo "Done!"