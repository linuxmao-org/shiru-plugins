Overview

EVALUA is an experimental VSTi synth plugin that uses math equations to 
describe various waveforms, ranging from the most basic ones to very 
complex. Inspired by the one-liner music programs that rocked the web 
back in 2011, as well as by the 8BitSynth module in the Caustic 3 
software that came out sometime later. 

Please note that this plugin only shares the same concept, but has a
very different implementation, thus it is not directly compatible with
either of the mentioned things. 



Features

- 64-bit integer math only
- Binary operations only, no unary or ternary for now
- Custom parser with very fast RPN-based calculator
- A number of variables to use in the equations
- Polyphonic or monophonic modes
- Portamento in the monophonic mode



Usage

Waveform is defined by a math equation that may include decimal and 
hexadecimal integer values, variables containing time counters, and other 
parameters. Result of the equation calculation is the output sample value.
The  output value considered to be within range of -256..256 (yes,
not 255).  If it goes out of these limits, it gets clamped. 

Calculations is always done in signed 64-bit numbers. Supported operations
and their syntax: 

+		add
-		subtract
*		multiple
/		divide
%		modulo (division remainder)
&		and
|		or
^		xor
<<		shift left
>>		shift right
( )		parenthesis

Operations priority is the usual one, mul/div/modulo take priority, 
everything else calculated in order of appearance, the order can be
modified with parenthesis.


What makes it a synth rather than an useless calculator is the
variables. You can use:

P		pitch counter, it changes in a way that 256 counts would
        make a single wave cycle for a note played
T		time since note started to play, fixed to 65536 counts
        per second
V		velocity 0..127
M		modulation wheel 0..127
R		random 0..32767

P is the most important variable, as you can define a waveform that has
correct pitch for note that is played. The most basic example is this:

P&255

It clamps counter to 0..255 range (256 counts repeated) and uses it as
output value that is repeatedly rising 0 to 255 in a loop, with loop
speed depending from the pitch. This makes a regular saw waveform.

You can also make a square waveform:

P&256

It uses the 8th bit that gets inverted each 256 counts, as output value,
making a regular square waveform.

These are simplest examples. The equation can be very long and complex,
producing long convoluted waveforms. Try some presets, or just experiment
to get more interesting sounds.


There is three global parameters that affects the sound. They're not
saved into a particular preset and get applied over any preset selected:

Gain	simply output gain of the plugin, 100 being normal volume

Poly	how many channels is there, 1 turns on monophonic mode

Porta	defines how fast a note changes in the monophonic mode



License 

The plugin and its source code comes without any warranty. You can
redistribute it and/or modify it under the terms of the WTFPL. See
http://www.wtfpl.net/ for more details.



History

v1.01 30.05.19 - Fix for issue with multiple instances
v1.0  28.05.19 - Initial release, basic functionality



Mail:	 shiru@mail.ru
Web:     http://shiru.untergrund.net
Support: https://www.patreon.com/shiru8bit