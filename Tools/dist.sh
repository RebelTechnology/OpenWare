#!/bin/bash
mkdir -p dist
# make syx files
for nm in Alchemist Wizard Magus Lich Witch OwlPedal2
do
    make -C $nm clean all sysex && cp $nm/Build/$nm.syx dist
done
# make OWL Pedal and OWL Modular bin and syx files
make -C OwlPedal PLATFORM=Pedal clean all sysex && cp OwlPedal/Build/OwlPedal.{syx,bin} dist
make -C OwlPedal PLATFORM=Modular clean all sysex && cp OwlPedal/Build/OwlModular.{syx,bin} dist
make -C MidiBoot1 PLATFORM=Pedal clean all sysex && cp MidiBoot1/Build/MidiBoot-Pedal.{syx,bin} dist
make -C MidiBoot1 PLATFORM=Modular clean all sysex && cp MidiBoot1/Build/MidiBoot-Modular.{syx,bin} dist
