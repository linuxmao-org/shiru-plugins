# shiru-plugins

A set of music plugins created by Shiru, unofficial [DPF](https://github.com/DISTRHO/DPF) ports

**Official home page:** http://shiru.untergrund.net/software.shtml

## EVALUA

### Description

Synth based on evaluation of math expressions, similar to the one-liner generative music C programs.

## ChipWave

### Description

Synth plugin that blends characteristics of many old sound chips (AY, SID, POKEY) with features of old subtractive synths.

## ChipDrum

### Description

Drum synth plugin, a counterpart for ChipWave, sharing the same concept, but for drum sounds, and capable to produce a range of sounds between 1-bit, chip tune, and classic analog drum machines.

# Building

Type `make`. The build will be found in the `bin` folder.

This build requires (Debian package names):

- `libcairo2-dev`
- `libpango1.0-dev`
- `libjack-jackd2-dev` or `libjack-dev`
- `libgl1-mesa-dev` or `libgl-dev`
- `libglu1-mesa-dev` or `libglu-dev`
