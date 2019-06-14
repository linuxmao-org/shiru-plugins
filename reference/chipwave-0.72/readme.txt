ChipWave by Shiru



Overview


This project has started as a simple alternative to the Medusa 2 VSTi, 
with a little bit less aliasing, extended pitch bend range to be 
compatible with ChipArp, and maybe a few more minor features. Eventually 
I added a lot of my experience with 1-bit synths, many early sound 
chips, and subtractive synths, so it became kind of a bridge between 
sound chips and 'big' synthesizers. 

The plugin has capatilbities to emulate some distinctive features of 
sound chips of the past, such as AY-3-8910, Atari 2600, POKEY, or SID, 
as well as many other synth features to create more complex and 
interesting timbres with chiptune flavour in it. Unlike VST sound chip 
emulators, the plugin does not emulate the limited 'granularity' of the 
chips, such as low res frequency counters or DACs, instead providing max 
possible quality and precision whenever possible, thus providing an 
idealized version of the old school sounds. Downside of this approach is 
the plugin being quite CPU heavy in the result. 

The synth does not include any chorus or delay, because there is tons of 
plugins out there that doing it much better than mine would. You would 
want to use a delay or a chorus plugin to make it sound big, though. I 
would recommend ReaDelay and TAL-Chorus-LX out of free ones. 

There is no presets loaded at startup. Use FXB bank file provided to 
load them. 






Features


- Two oscillators
 - Pulse, triangle/saw, and sine waves
 - Variable duty cycle (or waveform 'skew')
 - Digital noise generator with variable period, capable of short
   periodic tones
 
- ADSR envelope for modulation effects
- ADSR envelope for output volume

- LFO for many types of modulation effects

- Frequency slide, useful to create drum sounds

- Many types of velocity-based modulation effects, such as variable
  duty cycle

- Portamento mode and modulation wheel support




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

OscA    - the first oscillator
OscB    - the second oscillator, the same with a detune slider
Mix     - oscillator mixing settings
Env     - the usual ADSR envelope, can be applied to many parameters
Amp     - the usual ADSR envelope for volume
Filter  - LP filter settings
Slide   - frequency slide settings
Env Amt - amount of the Env output applied to various synth parameters
LFO Amt - LFO frequency and its amount for various synth parameters
Vel Amt - amount of the MIDI velocity applied to synth parameters
Output  - output gain and portamento settings


Parameters inside each block are:


 OscA and OscB

  Waveform
  
   Select one of four waveforms - pulse wave (square), triangle/saw,
   sine, or digital noise. There is no separate saw waveform selection,
   because it can be gained off the triangle wave with the next
   setting.
   
  Skew/Duty
  
   Set duty cycle for the pulse wave, or skew the triangle or sine
   waveform. When the triangle skewed to the max, it turns into saw
   waveform. For noise waveform this slider sets the noise period, with
   longest period it sounds like a white noise, with short period it
   eventually turns into distorted low tone (like some 2600/POKEY
   sounds).
   
  Overdrive
  
   Attenuate or amplify oscillator output, including overdrive with
   clipping. You can make square wave out of sine or triangle using
   high overdrive values, or just get different timbres in between.
   
  Cut
  
   Disables oscillator after defined amount of time. Zero means the
   oscillator will never get cut. It is useful to add short bursts
   of noise at beginning of some sounds, like toms or snare drum.
   
  Div/Mul
  
   Divide or multiple frequency of the oscillator to a power of two,
   effectively increasing or reducing the pitch by specified number
   of octaves.
   
  Noise Seed
  
   PRNG seed for digital noise generator. Zero value makes the noise
   sequence to start from arbitrary place each time, introducing
   randomness into low pitched noises. Non-zero values set specified
   seed value, so noise sequence starts from the same place with each
   note. This is useful for tones made out of the periodic noise,
   allowing to select specific tone timbres.
   
  Detune (OscB only)
  
   Detunes the oscillator up to 7 semitones from the other one. The
   limit is there because you can get the whole octave detuning range
   by changing the Div/Mul settings, and smallest detune values the
   most useful ones anyway. Normally you detune two oscillators by
   a few cents to add thickness into the sound, chorus-like effect.
	
	
 Mix
 
  Balance
  
   In the Add mode it sets the balance of the oscillators output,
   more of the first or the second one, with 50/50 mixing in the
   middle. In the Mul mode it affects the mixing in a special way,
   that is difficult to expalin, but effectively it changes the timbre.
   
  Mix Mode
  
   Select one of two mixing mode for oscillators. In the Add mode the
   outputs just sums up. In the Mul mode they get multiplied, which
   allows to create the envelope tones of the AY-3-8910 sound chip,
   when a square wave masks out parts of a differently pitched triangle
   or saw waveform. In general this is more interesting mixing mode
   capable to provide different timbres.
  
  
 Env and Amp
 
  Attack
  
   The usual Attack time given in ms. During this time the output level
   of the envelope reaches the maximum value.
   
  Decay
  
   The usual Decay time given in ms. During this time the output level
   falls down from the maximum value to the sustain level.
   
  Sustain
  
   The usual Sustain level of the envelope. Once envelope get through
   attack and decay stages, it settles down on this level and keeps it
   until a key is released.
   
  Release
  
   The usual Release time given in ms. During this time the output
   level falls from whenever current level it had down to zero.
  
  
 Filter
 
  Cutoff
  
   Cutoff frequency of the low pass filter. Maximum value disables the
   filter.
   
  Resonance
  
   Filter resonance.
  
  
 Slide
 
  Delay
   
   The time in ms delays beginning of the slide. It can be also delayed
   until a key is released. Together with non-zero release time of the
   Amp envelope it allows to create pitch fall backs on a key release.
   
  Speed
  
   Speed of sliding up or down.
   
  Route
  
   The slide can be applied to both or just one of the oscillators.
  
  
 Env Amt
 
  OscA and OscB Skew Depth

   The amount of envelope output that gets applied to the skew depth.
   This allows to dynamically change the timbre of the sound.
   
  OscB Detune Depth
  
   The amount of envelope output applied to the OscB detune.
   
  Balance Depth
  
   The amount of envelope output applied to the Osc Balance.
   
  Filter Depth
  
   The amount of envelope output applied to the Filter Cutoff.
   
  LFO Depth
  
   The amount of envelope output applied to the LFO output strength.
   It may be useful to create vibrato that slowly comes in. Envelope
   with slow attack will gradually increase influence of LFO applied
   to the pitch.
  
  
 LFO Amt
 
  Frequency
  
   LFO frequency.
   
  Pitch Depth
  
   The amount of LFO output applied to the pitch, creaing vibrato
   effect.
   
  OscA and OscB Skew Depth
  
   The amount of LFO  output applied to the skew depth.
   
  Balance Depth
  
   The amount of LFO output applied to the Osc Balance.
   
  Filter Depth
  
   The amount of LFO output applied to the Filter Cutoff.
  
  
 Vel Amt
 
  Vel to Amp
  
   The amount of MIDI velocity applied to the volume. With this
   slider you can gradually change between full velocity effect
   to make the velocity effectively turned off, always getting
   notes at full volume. Or just reduce the strength of the
   velocity, raising the lowest possible volume.
   
  OscA and OscB Skew Depth
  
   The amount of velocity applied to the skew depth. This way
   you can dynamically vary timbres depending on key pressing
   strength.
   
  Balance Depth
  
   The amount of velocity applied to the Osc Balance.
   
  Filter Cutoff
   
   The amount of velocity applied to the Filter Cutoff.
   
  Filter Resonance
  
   The amount of velocity applied to the Filter Resonance.
  
  
 Output
 
  Polyphony
  
   Selects polyphonic or monophonic mode.
   
  Porta Speed
  
   Sets portamento speed in the monophonic mode. Useful for smooth
   theremin-like solos.
   
  Gain
  
   Just sets the general output volume of the plugin.



  
License 


The plugin and its source code come without any warranty. You can
redistribute it and/or modify it under the terms of the WTFPL.
See http://www.wtfpl.net/ for more details. 




Credits


Plugin uses resonant low pass IIR filter code by Olli Niemitalo:

http://www.musicdsp.org/showArchiveComment.php?ArchiveID=27

Pinball Wizzard helped to test WIP versions and gave some
suggestions on the interface.




History

v0.72 03.06.19 - x64 support.
v0.71 23.06.18 - Minor fixes.
v0.7  09.06.18 - Various bug fixes. First public release version.
v0.6  08.06.18 - Noise seed, oscillator overdrive, balance for
                 multiply mode, code cleanup
v0.5  07.06.18 - Various minor pre-release fixes and improvements
v0.4  06.06.18 - Previously missing GUI has been fully implemented
v0.3  04.06.18 - More updates, code cleanup, a range of velocity
                settings added
v0.2  03.06.18 - Lots of changes after testing, number of controls
                get doubled
v0.1  02.06.18 - Initial version, basic features, no GUI




Mail:	 shiru@mail.ru
Web:     http://shiru.untergrund.net
Support: https://www.patreon.com/shiru8bit