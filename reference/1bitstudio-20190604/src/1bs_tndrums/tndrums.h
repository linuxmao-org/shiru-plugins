#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"1bitstudio - tndrums"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v0.23 03.06.19"
#define PLUGIN_UID				'1tnd'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			8

#define TONE_MAX_PITCH			16000.0f
#define NOISE_MAX_PITCH			32000.0f

#define SYNTH_NOTES				12
#define SYNTH_CHANNELS			8

#define PULSE_WIDTH_MAX_US		2000

#define CHUNK_SIZE				65536

#define MAX_NAME_LEN			32



enum {
	pIdNoteMapping=0,
	pIdToneDuration,
	pIdTonePitch,
	pIdToneSlide,
	pIdToneType,
	pIdToneWidth,
	pIdNoiseDuration,
	pIdNoisePitch,
	pIdNoiseSlide,
	pIdNoisePeriod,
	pIdMixMode,
	pIdDrumVolume,
	pIdPolyphony,
	pIdOutputGain,
	NUM_PARAMS
};



const char* const programDefaultNames[NUM_PROGRAMS]={
	"Kit 1",
	"Kit 2",
	"Kit 3",
	"User 1",
	"User 2",
	"User 3",
	"User 4",
	"User 5"
};



const char* const pNoteNames[12]={
	"C (Kick)",
	"C# (Crash)",
	"D (Snare)",
	"D# (Ride)",
	"E (Clap)",
	"F (Cowbell)",
	"F# (Closed hat)",
	"G (Low tom)",
	"G# (Medium hat)",
	"A (Mid tom)",
	"A# (Open hat)",
	"B (Hi tom)"
};



const char* const pMixModeNames[3]={
	"OR",
	"AND",
	"XOR"
};



enum {
	mEventTypeNote=0,
	mEventTypeProgram
};



struct MidiQueueStruct
{
	VstInt32 type;
	VstInt32 delta;
	VstInt32 note;
	VstInt32 velocity;
	VstInt32 program;
};




struct SynthChannelStruct
{
	VstInt32 toneOutput;
	float toneDuration;
	float toneAccumulator;
	float tonePulse;
	float toneIncrement;
	float toneIncrementDelta;
	float toneType;
	float toneWidth;

	VstInt32 noiseOutput;
	float noiseDuration;
	float noiseAccumulator;
	float noiseIncrement;
	float noiseIncrementDelta;
	VstInt32 noisePeriodMask;
	VstInt32 noisePtr;

	VstInt32 mixMode;

	float volume;
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define NOTE		TAG("NOTE")
#define TDUR		TAG("TNDU")
#define TPIT		TAG("TNPI")
#define TSLD		TAG("TNSL")
#define TTYP		TAG("TTYP")
#define TWDT		TAG("TWDT")
#define NDUR		TAG("NSDU")
#define NPIT		TAG("NSPI")
#define NSLD		TAG("NSSL")
#define NPRD		TAG("NSPR")
#define MODE		TAG("MODE")
#define VOLU		TAG("VOLU")
#define NMAP		TAG("NMAP")
#define POLY		TAG("POLY")
#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const float ChunkPresetData[]={

	DATA,

	PROG,0,
	NOTE,0,TDUR,0.078f,TPIT,0.165f,TSLD,0.199f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,1,TDUR,0.500f,TPIT,0.918f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.500f,NPIT,0.060f,NSLD,0.500f,NPRD,0.867f,MODE,0.484f,VOLU,0.734f,
	NOTE,2,TDUR,0.154f,TPIT,0.184f,TSLD,0.413f,TTYP,0.000f,TWDT,0.500f,NDUR,0.099f,NPIT,0.486f,NSLD,0.500f,NPRD,1.000f,MODE,0.546f,VOLU,1.000f,
	NOTE,3,TDUR,0.136f,TPIT,0.803f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.085f,NPIT,1.000f,NSLD,0.500f,NPRD,0.000f,MODE,1.000f,VOLU,0.539f,
	NOTE,4,TDUR,0.098f,TPIT,0.141f,TSLD,0.121f,TTYP,0.000f,TWDT,0.500f,NDUR,0.095f,NPIT,0.359f,NSLD,0.324f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,5,TDUR,0.085f,TPIT,0.342f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.051f,NPIT,0.744f,NSLD,0.500f,NPRD,0.263f,MODE,0.560f,VOLU,0.952f,
	NOTE,6,TDUR,0.041f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.104f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.405f,VOLU,0.532f,
	NOTE,7,TDUR,0.161f,TPIT,0.163f,TSLD,0.351f,TTYP,0.000f,TWDT,0.500f,NDUR,0.053f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,0.774f,
	NOTE,8,TDUR,0.098f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.165f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.500f,VOLU,0.533f,
	NOTE,9,TDUR,0.182f,TPIT,0.200f,TSLD,0.364f,TTYP,0.000f,TWDT,0.500f,NDUR,0.042f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,0.783f,
	NOTE,10,TDUR,0.255f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.190f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.418f,VOLU,0.518f,
	NOTE,11,TDUR,0.189f,TPIT,0.240f,TSLD,0.378f,TTYP,0.000f,TWDT,0.500f,NDUR,0.051f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,0.777f,
	POLY,1.000f,GAIN,1.000f,

	PROG,1,
	NOTE,0,TDUR,0.042f,TPIT,0.209f,TSLD,0.236f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,1,TDUR,0.000f,TPIT,0.918f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.500f,NPIT,0.025f,NSLD,0.500f,NPRD,1.000f,MODE,0.003f,VOLU,0.734f,
	NOTE,2,TDUR,0.076f,TPIT,0.275f,TSLD,0.193f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,3,TDUR,0.000f,TPIT,0.803f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.085f,NPIT,1.000f,NSLD,0.500f,NPRD,0.000f,MODE,1.000f,VOLU,0.539f,
	NOTE,4,TDUR,0.000f,TPIT,0.141f,TSLD,0.121f,TTYP,0.000f,TWDT,0.500f,NDUR,0.059f,NPIT,0.039f,NSLD,0.324f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,5,TDUR,0.059f,TPIT,0.738f,TSLD,0.000f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.744f,NSLD,0.500f,NPRD,0.263f,MODE,0.000f,VOLU,0.952f,
	NOTE,6,TDUR,0.000f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.036f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,0.532f,
	NOTE,7,TDUR,0.085f,TPIT,0.163f,TSLD,0.351f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,0.774f,
	NOTE,8,TDUR,0.000f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.079f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,0.533f,
	NOTE,9,TDUR,0.098f,TPIT,0.200f,TSLD,0.364f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,0.783f,
	NOTE,10,TDUR,0.000f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.154f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,0.518f,
	NOTE,11,TDUR,0.126f,TPIT,0.240f,TSLD,0.378f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,0.777f,
	POLY,1.000f,GAIN,1.000f,

	PROG,2,
	NOTE,0,TDUR,0.197f,TPIT,0.139f,TSLD,0.342f,TTYP,1.000f,TWDT,1.000f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,1,TDUR,0.504f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.100f,NDUR,0.499f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,2,TDUR,0.311f,TPIT,0.177f,TSLD,0.417f,TTYP,1.000f,TWDT,0.233f,NDUR,0.044f,NPIT,0.067f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,3,TDUR,0.243f,TPIT,0.958f,TSLD,0.500f,TTYP,0.000f,TWDT,0.678f,NDUR,0.256f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.431f,VOLU,1.000f,
	NOTE,4,TDUR,0.178f,TPIT,0.197f,TSLD,0.407f,TTYP,1.000f,TWDT,0.536f,NDUR,0.142f,NPIT,0.191f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,5,TDUR,0.075f,TPIT,0.500f,TSLD,0.500f,TTYP,1.000f,TWDT,0.043f,NDUR,0.027f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,6,TDUR,0.040f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.769f,NDUR,0.040f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.495f,VOLU,1.000f,
	NOTE,7,TDUR,0.499f,TPIT,0.169f,TSLD,0.438f,TTYP,1.000f,TWDT,0.500f,NDUR,0.499f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.463f,VOLU,1.000f,
	NOTE,8,TDUR,0.096f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.746f,NDUR,0.105f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.573f,VOLU,1.000f,
	NOTE,9,TDUR,0.490f,TPIT,0.210f,TSLD,0.434f,TTYP,1.000f,TWDT,0.500f,NDUR,0.504f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.490f,VOLU,1.000f,
	NOTE,10,TDUR,0.229f,TPIT,1.000f,TSLD,0.500f,TTYP,0.000f,TWDT,0.788f,NDUR,0.224f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.532f,VOLU,1.000f,
	NOTE,11,TDUR,0.488f,TPIT,0.232f,TSLD,0.438f,TTYP,1.000f,TWDT,0.500f,NDUR,0.497f,NPIT,1.000f,NSLD,0.500f,NPRD,1.000f,MODE,0.504f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,3,
	NOTE,0,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,1,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,2,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,3,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,4,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,5,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,6,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,7,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,8,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,9,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,10,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,11,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,4,
	NOTE,0,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,1,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,2,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,3,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,4,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,5,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,6,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,7,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,8,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,9,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,10,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,11,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,5,
	NOTE,0,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,1,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,2,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,3,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,4,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,5,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,6,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,7,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,8,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,9,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,10,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,11,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,6,
	NOTE,0,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,1,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,2,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,3,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,4,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,5,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,6,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,7,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,8,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,9,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,10,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,11,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,7,
	NOTE,0,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,1,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,2,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,3,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,4,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,5,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,6,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,7,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,8,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,9,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,10,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	NOTE,11,TDUR,0.500f,TPIT,0.500f,TSLD,0.500f,TTYP,0.000f,TWDT,0.500f,NDUR,0.000f,NPIT,0.500f,NSLD,0.500f,NPRD,1.000f,MODE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	DONE

};



class TNDrums:public AudioEffectX
{
public:

	TNDrums(audioMasterCallback audioMaster);

	~TNDrums();

	virtual void setParameter(VstInt32 index,float value);
	virtual float getParameter(VstInt32 index);
	virtual void getParameterLabel(VstInt32 index,char *label);
	virtual void getParameterDisplay(VstInt32 index,char *text);
	virtual void getParameterName(VstInt32 index,char *text); 

	virtual VstInt32 getProgram();
	virtual void setProgram(VstInt32 program);
	virtual void getProgramName(char* name);
	virtual void setProgramName(char* name);

	virtual bool getEffectName(char *name) { strcpy(name,PLUGIN_NAME); return true; }
	virtual bool getVendorString(char *text) { strcpy(text,PLUGIN_VENDOR); return true; }
	virtual bool getProductString(char *text) { strcpy(text, PLUGIN_PRODUCT); return true; }
	virtual VstInt32 getVendorVersion() { return 1000; } 

	VstInt32 canDo(char* text);
	VstInt32 getNumMidiInputChannels(void);
	VstInt32 getNumMidiOutputChannels(void);

	virtual VstInt32 getChunk(void** data,bool isPreset=false);
	virtual VstInt32 setChunk(void* data,VstInt32 byteSize,bool isPreset=false);

	VstInt32 processEvents(VstEvents* ev);
	void processReplacing(float**inputs,float **outputs,VstInt32 sampleFrames);

protected:

	VstInt32 Program;

	float pNoteMapping   [NUM_PROGRAMS];
	int   pNoteMappingInt;

	float pToneDuration  [NUM_PROGRAMS][SYNTH_NOTES];
	float pTonePitch     [NUM_PROGRAMS][SYNTH_NOTES];
	float pToneSlide     [NUM_PROGRAMS][SYNTH_NOTES];
	float pToneType      [NUM_PROGRAMS][SYNTH_NOTES];
	float pToneWidth     [NUM_PROGRAMS][SYNTH_NOTES];
	float pNoiseDuration [NUM_PROGRAMS][SYNTH_NOTES];
	float pNoisePitch    [NUM_PROGRAMS][SYNTH_NOTES];
	float pNoiseSlide    [NUM_PROGRAMS][SYNTH_NOTES];
	float pNoisePeriod   [NUM_PROGRAMS][SYNTH_NOTES];
	float pMixMode       [NUM_PROGRAMS][SYNTH_NOTES];
	float pDrumVolume    [NUM_PROGRAMS][SYNTH_NOTES];

	float pPolyphony     [NUM_PROGRAMS];
	float pOutputGain    [NUM_PROGRAMS];

	char ProgramName     [NUM_PROGRAMS][MAX_NAME_LEN];

	float Chunk[CHUNK_SIZE];

	float FloatToHz(float value,float range);
	int FloatToNoisePeriod(float value);

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	VstInt32 SaveStringChunk(char *str,float *dst);
	VstInt32 LoadStringChunk(char *str,int max_length,float *src);

	void MidiAddNewNote(VstInt32 delta,VstInt32 note,VstInt32 velocity);
	void MidiAddProgramChange(VstInt32 delta,VstInt32 program);

	vector<MidiQueueStruct> MidiQueue;

	SynthChannelStruct SynthChannel[SYNTH_CHANNELS];

	unsigned int Noise[2048];
};
