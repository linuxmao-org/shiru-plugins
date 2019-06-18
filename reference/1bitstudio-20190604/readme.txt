Overview 

1bit studio is a bundle of experimental GUI-less VST2 plugins, aimed to 
make some of the distinctive 1-bit music aesthetics easily accessible
via modern DAWs. 

The intended use is to provide a way to make sound design of the same 
roots as with 'true' retro tools, but advanced to the next level of 
quality, with lesser limitations and expanded capabilites, such as greater
polyphony, any combinations of all synthesis types, with unwanted sound 
artifacts eliminated. There is no purist goals set, so while these plugins 
based on the same exact principles as the real deal, they're not  trying
to emulate some particular retro hardware or software. While internal
synthesis is strictly 1-bit, output may have well more than two discrete
levels, because some of the plugins use oversampling, all of  them allow
to tweak output levels, and mixing in DAWs is performed in high resolution
with arbitrary number of sound sources anyway. 



Plugins included 


bitdrive 

A very basic level comparator. Outputs 0 while input level is below 
threshold, 1 otherwise. This is the principle to convert analog signal 
into square waves used by tape data storage systems, also used in guitar 
fuzz pedals. Could be useful as a distortion effect, used with a guitar 
or a synth, or a drum machine.


pulsedrive 

Generates narrow pulses based on input signal phase changes and averaged 
input level. Pulse width controlled by the averaged input level. When 
used with monophonic input, creates sound similar to the narrow-PWM 
1-bit music engines, with polyphonic input works as a distortion effect. 
Intended to use with a guitar or synth input. May need some dynamics 
processing on the input to have better control on the sound. 


clickydrums 

Drum synth based on short PWM noise bursts generation. Produces short 
clicky and bleepy type of percussion. Each of 12 notes have its own drum 
sound mapped to it. All sounds can be played at once, except for flat 
notes that acting as a 'hat', interrupting each other. Output mixing is 
done in high resolution, by addition of the output levels. Velocity
affects to the volume of each particular drum.


tndrums 

Drum synth based on a pair of tone and noise generators mixed together.
Both tone and noise can  have its own duration and slide. Tone generator
allows to select either pulse wave with configurable duty cycle (0.5 means
pure square wave) or a short pin, like pulsesynth. The latter creates
distinctively pronounced tone like Music Studio or Lyndon Sharp's engines.
Noise generator's PRNG has configurable period, allowing to produce metallic
tones in addition to noise. The mixing is done with OR/AND/XOR logic mixer.
Polyphonic output is mixed in high resolution. Velocity affects to the volume
of each particular drum.


noisedrums

Another simple drum synth. Generates a series of noise packets, up
to 8. Each packet has customizeable duration, pitch, period, and
pulse width. The latter affects to the volume, but adds an audible
metallic carrier tone, useful for hats.


tapedrums

A percussion synth based on the audio tape data storage encoding used
in old home computers, the 'loading noise'. It imitates data encoding
of ZX Spectrum (also Amstrad CPC), as one of the most suitable for
percussive sounds, although the carrier frequency could be changed to
make it more similar to other similar formats. Better to use it in
Monophonic mode.


phasesynth 

Phasing tone synth, based on two 1-bit generators with output combined 
by logic mixer. One of generators has a multiplier and detune 
parameters. In addition, both generators has separate duty cycle 
control, allowing to create wider range of sounds. Polyphonic output is 
mixed either in high resolution, by addition of output levels, or summed
with OR/XOR function, to create distinctive polyphonic distortions.
Velocity can be mapped either to the volume, or relative oscillator
detune (greater velocity means more detune, creating fatter sound),
or note duration.


pulsesynth 

Narrow PWM tone synth. The width of the pulse is controlled with ADSR 
envelope. Unlike other plugins, main polyphonic mixing mode is done
in 1-bit, it sums up pulse widths. This causes distinctive distortions,
well noticeable with longer pulses. There is alternative mixing mode
as well, it simply sums up outputs of the channels that eliminates these
distortions. Velocity always affects to the max width of the pulse.
Note with larger pulse widths may limit the audible range of notes in
the top octaves, a note period may become lesser than the pulse width,
resulting into lack of sound. Normally higher notes sounds much louder
than the lower ones, because of the nature of PFM synthesis (less energy
in less frequent pulses of the same width), but there is an option
to compensate it, called 'Boost low end', it increases pulse width for
the lower notes and decreases it for the higher notes, trying to keep
width for the C-4 (middle range) more or less the same.


oversynth

A very simple synth producing intermodulation distortions, similar to
the guitar overdrive. It intended to be used to play intervals of
two notes most of the time. There is detune control, which is, unlike
any other plugins, detunes channels of the synth between each other.
The synth also allows to play few instances of the same note, along
with detune this produces phasing sound. Velocity can be mapped
either to the overall volume of all channels, or initial pulse width,
or note duration.


wavesynth

Customizeable waveform synth. Sort of Konami SCC or Namco 163 sound
chips, but the waveform is 1-bit, with resolution of either 4, 8, 16,
or 32 steps. Produces harsh metallic tones, some of them nicely work
as bass. There is also optional bit shifting modulation that creates
sequences similar to the sample and hold ones on analog synths.
Polyphonic output is mixed either in high resolution, by addition of
output levels, or summed with OR/XOR function, to create distinctive
polyphonic distortions. Velocity can be mapped either to the volume
or note duration.


sweepsynth

Duty cycle sweep-based PFM synth. Allows to create sounds in the range
of the Music Studio, Lyndon Sharp's and ZX-10, as well as Octode,
Huby and some other engine timbres. Synth allows to set initial
pulse width, its minimal and maximum widths, and the sweep, i.e.
speed of changing the width in either direction. Polyphonic output is
mixed either in high resolution, by addition of output levels, or
in the PWM/XOR ways, creating distinctive polyphonic distortions.
Velocity can be mapped either to the volume, sweep scaling (lower
velocity means longer sweep), or note duration. This synth has the
same quirk regarding the pulse width and high notes as the pulsesynth.


noisesynth

Dual noise generators based synth, capable of a range of glitchy
noise or tonal sounds. Contains a pair of pseudorandom noise generators
with the same exact parameters. Each of them can have infinite
or limited duration, pitch relative to played note, or fixed to a note,
or running at very low frequencies, with or without continuous slide up
or down. Period and seed can be configured, shorter periods generate
metallic tones, seed can be used to select a particular waveform, free
running seed will generate always changing waveform. Output of the
generators is mixed with AND/OR/XOR functions, generators can be
detuned from each other to create fatter sounds. Polyphonic output is 
mixed in high resolution, by addition of output levels, or summed
with OR/XOR function, to create distinctive polyphonic distortions.
Velocity can be mapped either to the volume, or relative oscillators
detune, or note duration.


phatsynth

This synth is kind of 1-bit implementation of the SuperSaw on JP8000.
It runs up to 16 oscillators detuned around the base pitch to a specified
amount. The oscillators can generate either square/pulse with configurable
duty cycle, or narrow pins with configurable width. This allows to create
very powerful and fat sounds. Phase of the oscillators can be either
free running, or synced to a given value, thus starting the sound texture
from a specific place. Oscillators in the square/pulse can be mixed by
OR/XOR function, pin pulses always mixed as OR. Polyphonic output is
mixed in high resolution, by addition of output levels, or summed with
PWM/XOR, to create distinctive polyphonic distortions. Velocity can be
targeted to either volume, amount of the oscillators detune, phase reset
value, duty cycle/pulse width, or duration of a note. This helps to create
controllable changes in the sound, a bit similar to filter/resonance
changes, useful for bass lines.


serialsynth

This synth isn't much practical, but included for completeness of the 
bundle. It has been designed to test out 1-bit sound synthesis 
possibilities of the Intel 8253 programmable interval timer through 
different cascading and mixing schemes for the chip's three counter 
channels, thus it is modelled after the chip, although it is not an 
actual emulation. 

The plugin contains three counting channels with uniform features that 
has count enable input (gate) and output (switches between 0/1). Gate 
can be always on, or connected to another channel's output, allowing 
channels to modulate each other. Counter frequency can be relative to a 
note played (with offset in semitones and cents), or set to a fixed 
note. Counter phase, i.e. initial state, can be free running, or reset 
to a particular value. 

Sound gets picked from the channels outputs that can be enabled or 
disabled separately, then mixed digitally through OR, AND, or XOR 
function, or mixed in analog (making it not truly 1-bit), with volumes 
added with 4:4:4 or 1:2:4 ratios. As it gives much flexibility in 
modulation between channels, it can produce many weird sounds. 



Source code 

The code was written and compiled with MVS Community 2017. Project files 
configured to use VST SDK 3.6.0 located at C:\VST3 SDK\, only VST2 part 
of the SDK is used. When everything is installed correctly, plugins will 
compile without any additional actions, just with Build Solution. 

Psycle, Reaper, and VSTHost were used during development to test the 
plugins. 



License 

The plugins and source code comes without any warranty. You can 
redistribute it and/or modify it under the terms of the WTFPL. See 
http://www.wtfpl.net/ for more details. 



History 

04.06.19 - x64 support added, projects updated to MVS Community 2017,
           serialsynth is added
18.04.17 - corrected octave for the tone pin mode in tndrums. Optional
           phase reset in oversynth
14.04.17 - saving edited program names and Program Change event support
           in all synths, pin pulse mode added to phasesynth
02.04.17 - phatsynth added, mono mode fixed in sweepsynth
27.03.17 - noisesynth added
25.03.17 - sweepsynth added, tone type/width implemented in tndrums,
		   boost low end for the pulsesynth
24.03.17 - all synths updated to support vibrato (MIDI modulation),
           some extra features such as note cut, various mixing modes,
		   and velocity target added
18.08.15 - fixed noisedrums crash in Psycle and more slide issues
15.08.15 - fixed slide issues in phasesynth, pulsesynth, wavesynth
10.07.15 - tapedrums added
08.07.15 - phasesynth detune range changed from up/down tone to octave,
           up-only. noisedrums added
04.07.15 - phasesynth and pulsesynth optimized, minor bugs fixed
           oversynth and wavesynth added. Plugin filenames renamed
		   in a way for easier navigation in some older VST hosts
02.07.15 - Initial bundle release. Includes bitdrive, pulsedrive,
           clickdrums, tndrums, phasesynth, pulsesynth. All plugins
		   at v0.1, and were created during a week

		   

Mail:	 shiru@mail.ru
Web:     http://shiru.untergrund.net
Support: https://www.patreon.com/shiru8bit