#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME			"1bitstudio - clickydrums"
#define PLUGIN_VENDOR		"Shiru"
#define PLUGIN_PRODUCT		"v0.13 03.06.19"
#define PLUGIN_UID			'1cld'

#define NUM_INPUTS			0
#define NUM_OUTPUTS			2
#define NUM_PROGRAMS		8

#define SYNTH_NOTES			12
#define SYNTH_CHANNELS		8

#define CHUNK_SIZE			65536

#define MAX_NAME_LEN		32



enum {
	pIdNoteMapping=0,
	pIdDuration,
	pIdMin,
	pIdMax,
	pIdSeed,
	pIdType,
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
	VstInt32 output;
	float duration;
	float accumulator;
	float delay;
	float delay_min;
	float delay_max;
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
#define POLY		TAG("POLY")
#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const float ChunkPresetData[]={

	DATA,

	PROG,0,
	NOTE,0,DURA,0.012f,PMIN,0.000f,PMAX,1.000f,SEED,0.146f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.025f,PMIN,0.000f,PMAX,0.014f,SEED,0.000f,TYPE,1.000f,VOLU,0.688f,
	NOTE,2,DURA,0.012f,PMIN,0.019f,PMAX,0.057f,SEED,1.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.016f,PMIN,0.000f,PMAX,0.015f,SEED,0.000f,TYPE,0.000f,VOLU,0.648f,
	NOTE,4,DURA,0.021f,PMIN,0.677f,PMAX,1.000f,SEED,0.783f,TYPE,1.000f,VOLU,1.000f,
	NOTE,5,DURA,0.017f,PMIN,0.000f,PMAX,0.012f,SEED,0.000f,TYPE,0.000f,VOLU,0.829f,
	NOTE,6,DURA,0.005f,PMIN,0.000f,PMAX,0.009f,SEED,0.000f,TYPE,1.000f,VOLU,0.721f,
	NOTE,7,DURA,0.067f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	NOTE,8,DURA,0.012f,PMIN,0.000f,PMAX,0.010f,SEED,0.000f,TYPE,1.000f,VOLU,0.739f,
	NOTE,9,DURA,0.076f,PMIN,0.000f,PMAX,0.792f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	NOTE,10,DURA,0.016f,PMIN,0.000f,PMAX,0.006f,SEED,0.000f,TYPE,0.869f,VOLU,0.779f,
	NOTE,11,DURA,0.067f,PMIN,0.000f,PMAX,0.620f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,1,
	NOTE,0,DURA,0.048f,PMIN,0.428f,PMAX,0.855f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,1.000f,PMIN,0.000f,PMAX,0.014f,SEED,0.000f,TYPE,1.000f,VOLU,0.688f,
	NOTE,2,DURA,0.080f,PMIN,0.096f,PMAX,0.171f,SEED,0.459f,TYPE,1.000f,VOLU,1.000f,
	NOTE,3,DURA,0.250f,PMIN,0.000f,PMAX,0.015f,SEED,0.000f,TYPE,0.000f,VOLU,0.648f,
	NOTE,4,DURA,0.076f,PMIN,0.130f,PMAX,0.182f,SEED,0.127f,TYPE,1.000f,VOLU,1.000f,
	NOTE,5,DURA,0.083f,PMIN,0.000f,PMAX,0.012f,SEED,0.000f,TYPE,0.000f,VOLU,0.829f,
	NOTE,6,DURA,0.025f,PMIN,0.000f,PMAX,0.009f,SEED,0.000f,TYPE,1.000f,VOLU,0.721f,
	NOTE,7,DURA,0.301f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	NOTE,8,DURA,0.045f,PMIN,0.000f,PMAX,0.010f,SEED,0.000f,TYPE,1.000f,VOLU,0.739f,
	NOTE,9,DURA,0.311f,PMIN,0.000f,PMAX,0.792f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	NOTE,10,DURA,0.076f,PMIN,0.000f,PMAX,0.006f,SEED,0.000f,TYPE,0.869f,VOLU,0.779f,
	NOTE,11,DURA,0.307f,PMIN,0.000f,PMAX,0.620f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,2,
	NOTE,0,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.650f,PMIN,0.000f,PMAX,0.001f,SEED,0.035f,TYPE,1.000f,VOLU,0.500f,
	NOTE,2,DURA,0.209f,PMIN,0.094f,PMAX,0.202f,SEED,0.145f,TYPE,1.000f,VOLU,1.000f,
	NOTE,3,DURA,0.219f,PMIN,0.000f,PMAX,0.027f,SEED,0.614f,TYPE,0.000f,VOLU,0.500f,
	NOTE,4,DURA,0.146f,PMIN,0.031f,PMAX,0.129f,SEED,0.126f,TYPE,1.000f,VOLU,1.000f,
	NOTE,5,DURA,0.045f,PMIN,0.000f,PMAX,0.012f,SEED,0.806f,TYPE,0.000f,VOLU,0.811f,
	NOTE,6,DURA,0.042f,PMIN,0.000f,PMAX,0.017f,SEED,0.000f,TYPE,1.000f,VOLU,0.504f,
	NOTE,7,DURA,0.189f,PMIN,0.348f,PMAX,0.507f,SEED,0.000f,TYPE,0.958f,VOLU,1.000f,
	NOTE,8,DURA,0.078f,PMIN,0.000f,PMAX,0.016f,SEED,0.000f,TYPE,1.000f,VOLU,0.504f,
	NOTE,9,DURA,0.230f,PMIN,0.297f,PMAX,0.393f,SEED,0.000f,TYPE,0.986f,VOLU,1.000f,
	NOTE,10,DURA,0.161f,PMIN,0.000f,PMAX,0.018f,SEED,0.000f,TYPE,1.000f,VOLU,0.509f,
	NOTE,11,DURA,0.220f,PMIN,0.242f,PMAX,0.340f,SEED,0.000f,TYPE,1.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,3,
	NOTE,0,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,4,
	NOTE,0,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,5,
	NOTE,0,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,6,
	NOTE,0,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	PROG,7,
	NOTE,0,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,1,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,2,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,3,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,4,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,5,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,6,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,7,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,8,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,9,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,10,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	NOTE,11,DURA,0.080f,PMIN,0.000f,PMAX,1.000f,SEED,0.000f,TYPE,0.000f,VOLU,1.000f,
	POLY,1.000f,GAIN,1.000f,

	DONE
};



class ClickyDrums:public AudioEffectX
{
public:

	ClickyDrums(audioMasterCallback audioMaster);

	~ClickyDrums();

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

	float pDuration  [NUM_PROGRAMS][SYNTH_NOTES];
	float pSeed      [NUM_PROGRAMS][SYNTH_NOTES];
	float pMin       [NUM_PROGRAMS][SYNTH_NOTES];
	float pMax       [NUM_PROGRAMS][SYNTH_NOTES];
	float pType      [NUM_PROGRAMS][SYNTH_NOTES];
	float pDrumVolume[NUM_PROGRAMS][SYNTH_NOTES];

	float pPolyphony [NUM_PROGRAMS];
	float pOutputGain[NUM_PROGRAMS];

	char  ProgramName[NUM_PROGRAMS][MAX_NAME_LEN];

	float Chunk[CHUNK_SIZE];

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	VstInt32 SaveStringChunk(char *str,float *dst);
	VstInt32 LoadStringChunk(char *str,int max_length,float *src);

	void MidiAddNewNote(VstInt32 delta,VstInt32 note,VstInt32 velocity);
	void MidiAddProgramChange(VstInt32 delta,VstInt32 program);

	vector<MidiQueueStruct> MidiQueue;

	SynthChannelStruct SynthChannel[SYNTH_CHANNELS];

	unsigned char Noise[16384];
};
