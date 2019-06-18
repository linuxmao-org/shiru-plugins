Flopster by Shiru


Overview

This is a VSTi that imitates noises of a floppy disk drive when it used 
for the favorite gimmick of the old MS-DOS days and modern YouTube times 
- playing music. It is sample based, the VSTi just controls them in a 
way to provide more realistic sound. You can also just use the samples 
itself, they're available in the /samples/ folder. 

You can also tweak the samples, or make your own preset by adding a 
folder prefixed with a new preset number. Note that plugin only supports 
uncompressed 44100 Hz 16-bit mono files, and does not do any checks if 
something is off. 



Features

There is not much to control besides volume of different sounds, so no 
GUI and not many knobs. All kinds of sound that plugin can play is 
controlled by note range and velocity (there is a threshold around the 
velocity value given below): 

- Spindle motor sound. Use note C-6 with any velocity. 

- Singular head steps. They don't not have musical pitch, can be used as 
  kind of percussive instrument instead. There is 80 steps, each one 
  sounds slightly different. Any note below C-6 with velocity ~15 with 
  play particular step sound, always the same, so you can create a subtle 
  pattern out of it. Alternatively, note D-6 with any veloicty plays steps 
  in sequence. 

- Continuous sounds made of repeating singular head steps. It sounds 
  fine for really low notes, but gets obviously artifical at pitches of 
  musical range. Still, available in case you want it. Any note below C-6, 
  velocity ~40. 

- Continuous sounds made of the head buzz, only available in C-3..B-5 
  range. That's the most common way to make music with a floppy drive. 
  Head just moves back and forth, alternating direction every step, thus 
  remaining in place. This gives clean sound, but this is unusual 
  behaviour for the drive, very different from how it used for data 
  access. Velocity ~60. 

- Continuous sounds made of the head seek, only available in C-3..B-5 
  range. This is more natural way for a floppy drive to work - head moves 
  80 steps up to last position, then 80 steps back to the first. It 
  produces even cleaner sound in the middle of movement, but with loud 
  clicks when direction changes. Velocity ~120 starts this sound from the 
  initial position, velocity ~90 remembers the last head position and 
  resumes from there. 

- Four extra sounds, disk push into drive (E-6), insert (F-6), eject 
  (G-6), and pull out of drive (A-6). 


  
License 

The plugin and its source code come without any warranty. You can
redistribute it and/or modify it under the terms of the WTFPL.
See http://www.wtfpl.net/ for more details. 



History

v1.01 03.06.19 - x64 support
v1.0  30.06.17 - Initial release



Mail:	 shiru@mail.ru
Web:     http://shiru.untergrund.net
Support: https://www.patreon.com/shiru8bit