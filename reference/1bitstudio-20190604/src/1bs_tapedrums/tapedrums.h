#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME			"1bitstudio - tapedrums"
#define PLUGIN_VENDOR		"Shiru"
#define PLUGIN_PRODUCT		"v0.13 03.06.19"
#define PLUGIN_UID			'1tpd'

#define NUM_INPUTS			0
#define NUM_OUTPUTS			2
#define NUM_PROGRAMS		8

#define SYNTH_NOTES			12
#define SYNTH_CHANNELS		8

#define MAX_DURATION		3000.0f

#define MIN_FREQUENCY		200.0f
#define MAX_FREQUENCY		4000.0f
#define FREQ_RANGE			(MAX_FREQUENCY-MIN_FREQUENCY)

#define OVERSAMPLING		4

#define CHUNK_SIZE			65536

#define MAX_NAME_LEN		32



enum {
	pIdNoteMapping=0,
	pIdDuration,
	pIdMin,
	pIdMax,
	pIdType,
	pIdSeed,
	pIdDrumVolume,
	pIdCarrierFreq,
	pIdPolyphony,
	pIdOutputGain,
	NUM_PARAMS
};



const char* const programDefaultNames[NUM_PROGRAMS]={
	"Kit 1",
	"Kit 2",
	"User 2",
	"User 3",
	"User 4",
	"User 5",
	"User 6",
	"User 7"
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
	float duration;

	float acc;
	float add;

	VstInt32 byte;
	VstInt32 bit;
	VstInt32 byte_min;
	VstInt32 byte_max;

	VstInt32 ptr;
	VstInt32 type;

	float volume;
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define NOTE		TAG("NOTE")
#define DURA		TAG("DURA")
#define PMIN		TAG("PMIN")
#define PMAX		TAG("PMAX")
#define SEED		TAG("SEED")
#define TYPE		TAG("TYPE")
#define VOLU		TAG("VOLU")
#define NMAP		TAG("NMAP")
#define CARR		TAG("CARR")
#define POLY		TAG("POLY")
#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const float ChunkPresetData[]={

	DATA,

	PROG,0,
	NOTE,0,DURA,0.013f,PMIN,0.509f,PMAX,0.560f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.106f,PMIN,0.000f,PMAX,0.031f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	NOTE,2,DURA,0.034f,PMIN,0.536f,PMAX,0.547f,SEED,0.600f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.049f,PMIN,0.928f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.045f,PMIN,0.252f,PMAX,0.429f,SEED,0.812f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.011f,PMIN,0.143f,PMAX,0.287f,SEED,0.105f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.012f,PMIN,0.000f,PMAX,0.022f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	NOTE,7,DURA,0.046f,PMIN,0.666f,PMAX,0.690f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.029f,PMIN,0.000f,PMAX,0.014f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	NOTE,9,DURA,0.049f,PMIN,0.693f,PMAX,0.756f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.045f,PMIN,0.000f,PMAX,0.008f,SEED,0.615f,TYPE,1.000f,VOLU,1.000f,
	NOTE,11,DURA,0.045f,PMIN,0.854f,PMAX,0.930f,SEED,0.173f,TYPE,0.000f,VOLU,1.000f,
	CARR,0.212f,POLY,1.000f,GAIN,1.000f,

	PROG,1,
	NOTE,0,DURA,0.016f,PMIN,0.137f,PMAX,0.155f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.090f,PMIN,0.000f,PMAX,0.008f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	NOTE,2,DURA,0.025f,PMIN,0.742f,PMAX,0.774f,SEED,0.344f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.035f,PMIN,0.000f,PMAX,0.095f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.016f,PMIN,0.636f,PMAX,0.651f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.009f,PMIN,0.000f,PMAX,0.133f,SEED,0.105f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.003f,PMIN,0.996f,PMAX,1.000f,SEED,1.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.063f,PMIN,0.000f,PMAX,0.161f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.011f,PMIN,0.994f,PMAX,1.000f,SEED,0.247f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.067f,PMIN,0.000f,PMAX,0.176f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.016f,PMIN,0.994f,PMAX,1.000f,SEED,0.344f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.063f,PMIN,0.000f,PMAX,0.134f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	CARR,0.158f,POLY,1.000f,GAIN,1.000f,

	PROG,2,
	NOTE,0,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	CARR,0.223f,POLY,1.000f,GAIN,1.000f,

	PROG,3,
	NOTE,0,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	CARR,0.223f,POLY,1.000f,GAIN,1.000f,

	PROG,4,
	NOTE,0,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	CARR,0.223f,POLY,1.000f,GAIN,1.000f,

	PROG,5,
	NOTE,0,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	CARR,0.223f,POLY,1.000f,GAIN,1.000f,

	PROG,6,
	NOTE,0,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	CARR,0.223f,POLY,1.000f,GAIN,1.000f,

	PROG,7,
	NOTE,0,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.100f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	CARR,0.223f,POLY,1.000f,GAIN,1.000f,

	DONE

};



class TapeDrums:public AudioEffectX
{
public:

	TapeDrums(audioMasterCallback audioMaster);

	~TapeDrums();

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

	float pNoteMapping[NUM_PROGRAMS];
	int pNoteMappingInt;

	float pDuration   [NUM_PROGRAMS][SYNTH_NOTES];
	float pMin        [NUM_PROGRAMS][SYNTH_NOTES];
	float pMax        [NUM_PROGRAMS][SYNTH_NOTES];
	float pType       [NUM_PROGRAMS][SYNTH_NOTES];
	float pSeed       [NUM_PROGRAMS][SYNTH_NOTES];
	float pDrumVolume [NUM_PROGRAMS][SYNTH_NOTES];

	float pCarrierFreq[NUM_PROGRAMS];
	float pPolyphony  [NUM_PROGRAMS];
	float pOutputGain [NUM_PROGRAMS];

	char ProgramName  [NUM_PROGRAMS][MAX_NAME_LEN];

	float Chunk[CHUNK_SIZE];

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	VstInt32 SaveStringChunk(char *str,float *dst);
	VstInt32 LoadStringChunk(char *str,int max_length,float *src);

	void MidiAddNewNote(VstInt32 delta,VstInt32 note,VstInt32 velocity);
	void MidiAddProgramChange(VstInt32 delta,VstInt32 program);

	vector<MidiQueueStruct> MidiQueue;

	SynthChannelStruct SynthChannel[SYNTH_CHANNELS];

	float sPulse;

	unsigned char Noise[16384];
};
