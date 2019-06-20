# shiru-plugins

A set of music plugins created by Shiru, unofficial [DPF](https://github.com/DISTRHO/DPF) ports

**Official home page:** http://shiru.untergrund.net/software.shtml

[![Build Status](https://ci.appveyor.com/api/projects/status/github/jpcima/shiru-plugins?svg=true)](https://ci.appveyor.com/project/jpcima/shiru-plugins)

## Description

**EVALUA:** Synth based on evaluation of math expressions, similar to the one-liner generative music C programs.

**ChipWave:** Synth plugin that blends characteristics of many old sound chips (AY, SID, POKEY) with features of old subtractive synths.

**ChipDrum:** Drum synth plugin, a counterpart for ChipWave, sharing the same concept, but for drum sounds, and capable to produce a range of sounds between 1-bit, chip tune, and classic analog drum machines.

**CrushDMC:** Bitcrusher plugin that emulates specific distortions of the Famicom/NES DMC channel.

**Flopster:** Emulator of a floppy disk drive that is used to play music.

**1bitstudio:** A bundle of experimental plugins for 1-bit sound synthesis and processing.

# Building

Type `make`. The build will be found in the `bin` folder.

This build requires (Debian package names):

- `libcairo2-dev`
- `libpango1.0-dev`
- `libjack-jackd2-dev` or `libjack-dev`
- `libgl1-mesa-dev` or `libgl-dev`
- `libglu1-mesa-dev` or `libglu-dev`
