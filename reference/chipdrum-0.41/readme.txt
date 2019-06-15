ChipDrum by Shiru



Overview

This is a counter part to the ChipWave, a companion drum synth based
on the same ideas and principles. It kind of blends features of old
sound chips and 'big' synthesizers of the past in regards of drum
sound synthesis.

The plugin has capabilities to emulated typical chip music drums,
either square and noise based, like AY-3-8910 or SID music have, or
pure digital noise, like older NES games have, or periodic noise based
drums, or 808/909 alike sounds, even DPCM alike sound.

Unlike VST sound chip emulators, the plugin does not emulate the limited
'granularity' of the chips, such as low res frequency counters or DACs,
instead providing max  possible quality and precision whenever possible,
thus providing an  idealized version of the old school sounds. Downside
of this approach is  the plugin being quite CPU heavy in the result. 

There is no presets loaded at startup. Use FXB bank file provided to 
load them. 




Features


- Two oscillators
 - Tone has square, triangle, saw, and sine wave
 - Noise has digital noise generator with variable period, capable of
   short periodic tones
 
- DSR envelope for each of oscillators

- Four separate stereo output, HP/LP filter for each drum sound, and more




Parameters


All synth parameters grouped into a few functional blocks. As there is 
not much room on the screen to display all their names and meaning, it 
is explained in this doc and in the pop up text hint in the interface, 
displayed when you hover the mouse on a slider. 

You can change value of a slider by dragging it with the left mouse 
button. To make tiny adjustments, which may be useful for some 
parameters, hold Shift key and move mouse up or down. Some sliders has 
few predefined fixed value, slider jumps to the closest one when you 
release the left mouse button. For sliders that has zero position in the 
middle (marked with + -) you can use the right mouse button to reset 
them to zero. 



The blocks are:

Tone           - tone oscillator
Noise          - noise oscillator
Tone/Noise Env - amplitude DSR envelopes for each of the oscillators
Retrig         - retrigger, useful for clap sound
Filter         - LP/HP filter, acting as an EQ unit
Vel Amt        - amount of the MIDI velocity applied to synth parameters
Drum           - other per-drum settings such as pan, volume, bit depth
Hats           - hat group duration and pan
Toms           - tom group tuning and pan
Out            - output gain



Parameters inside each block are:


 Tone
 
  Waveform
  
   Select one of four waveforms for the tone oscillator - square wave,
   triangle, saw, or sine. There is no duty cycle or wave skewing, as
   it proved to be not very useful for percussive sounds.
   
  Overdrive
  
   Amplify oscillator output beyond maximum level, with clipping. You can
   make square wave out of sine or triangle using high overdrive values,
   or just get different timbres in between.
   
  Pitch
  
   Starting pitch of the oscillator.
   
  Slide
  
   Pitch slide down/up speed, the main mean to create tonal percussive
   sounds, such as kick drum or toms, or tonal component of snare drum.
   
 
 Noise
 
  Type
  
   Selection between so called analog or digital noise types. The analog
   is just a periodic nosie with output 256 levels, while digital noise
   is strictly 1-bit, as found in all sound chips, producing more harsh
   sound.
   
  Period
  
   Noise period length. Longest period sounds like a white noise, short
   periods eventually turns into distorted low tone (like some 2600/POKEY
   sounds).
   
  Seed
  
   PRNG seed for digital noise generator. Zero value makes the noise
   sequence to start from arbitrary place each time, introducing
   randomness into low pitched noises. Non-zero values set specified
   seed value, so noise sequence starts from the same place with each
   note. This is useful for tones made out of the periodic noise,
   allowing to select specific tone timbres.
   
  Main and Secondary Pitch
  Secondary Pitch Offset and Duration
  
   There is two noise pitches. One is defined with the Main Pitch slider,
   it sounds all the time. It gets interrupted for a defined time with
   the noise of Secondary Pitch. This is very useful to create chiptune
   snare drum, as most of them rely on a jump in the noise pitch, or a
   gap in the middle of noise pitch, also achieveable with this feature.
   
  
 Tone Env and Noise Env

  Level
  
   As percussive sounds don't naturally feature a slow attack, starting
   from a hit, there is just a starting level instead.
   
  Decay
  
   The usual Decay time given in ms. During this time the output level
   falls down from the initial Lelel value to the Sustain level.
   
  Sustain
  
   Sustain level of the envelope. Unlike music synths, level does not
   stay there and immediately goes to the release stage. This helps
   to create volume spikes for the initial hit, with quieter decaying
   sound after the hit.
   
  Release
  
   The usual Release time given in ms. During this time the output
   level falls from whenever current level it had down to zero.

   
 Retrig
 
  Time
  
   Defines the time given in ms that passes between retriggers, i.e.
   between restarts of the tone/noise pitch and volume envelopes. The
   retrigger feature is very helpful for clap sounds, using relatively
   large times, as well as to make attack stage of some sounds more
   defined using very short retrigger times.
   
  Count
  
   Number of times to retrigger a drum sound, 0 disables the retrigger
   feature.
   
  Route
  
   Selects the target of retrigger. It can be either tone, or noise, or
   both.
   

 Filter
 
  Low and High Pass
  
   The usual filter paratemets.
   
  Route
  
   Selects the target of retrigger. It can be either tone, or noise, or
   both, or none.
  

 Vel Amt

  Volume
  
   The amount of MIDI velocity applied to the volume. With this
   slider you can gradually change between full velocity effect
   to make the velocity effectively turned off, always getting
   notes at full volume. Or just reduce the strength of the
   velocity, raising the lowest possible volume.
   
  Tone and Noise Pitch
  
   The amount of velocity applied to the pitch.
   
  Overdrive
  
   The amount of velocity applied to the tone overdrive.
  
  
 Drum

  Volume and Pan
  
   Volume and panning of a particular drum sound, can be set
   separately for each of 8 sound types. Hats and toms also has
   their pan width setting, this pan setting sets the central
   point for them.

  Output
  
   Selects one of four plugin outputs to route the drum sound.
   This way you can process groups of drum instruments with external
   plugins using separate effect chains.
   
  Depth
  
   Sets bit depth of a drum sound, introducing more transients and
   acting like a bit crusher of sorts. Useful to create kick drums,
   as well as to imitate chiptune drums (those are 4 bit usually).
   
  Update Rate
  
   This feature used to emulate the most prominent characteristic
   of a chip tune drums which is a limited pitch update rate. Set
   it to 50 or 60 Hz to get typical sound, or even lower to add
   more of that character.
   
  
 Hats

  F#/G#/A# Length
  
   Sets duration of each of the hat sounds. All other settings is
   shared between all three of them.
   
  Pan Width
  
   Sets pannign width for all hats, with F# being at the left,
   G# in the middle, and A# on the right relative to the central
   hat pan position.
   
  
 Toms

  G/A/B Pitch Offset
  
   Sets pitch of each of the tom sounds. All other settings is
   shared between all three of them.
   
  Pan Width
  
   Sets pannign width for all toms, similar to the hats pan width.
  
  
 Out
 
  Final output gain of the plugin.
  
  
  
  
License 


The plugin and its source code come without any warranty. You can
redistribute it and/or modify it under the terms of the WTFPL.
See http://www.wtfpl.net/ for more details. 




Credits


Plugin uses low pass IIR filter code by Olli Niemitalo, also acting
as high pass:

http://www.musicdsp.org/showArchiveComment.php?ArchiveID=27

Pinball Wizzard helped to test WIP versions and gave some
suggestions on the interface.




History

v0.41 03.06.19 - x64 support.
v0.4  23.06.18 - GUI implemented. Correct triangle waveform. First public release.
v0.3  19.06.18 - Better LP/HP filter, update rate parameter, velocity parameters.
v0.2  17.06.18 - Many bug fixes and tweaks. Pan added. Better noise pitch system.
v0.1  14.06.18 - Initial version with all planned features but velocity effects, no GUI.



Mail:	 shiru@mail.ru
Web:     http://shiru.untergrund.net
Support: https://www.patreon.com/shiru8bit