#include <cmath>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"1bitstudio - phasesynth"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v0.31 03.06.19"
#define PLUGIN_UID				'1phs'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			16

#define SYNTH_CHANNELS			8

#define PULSE_WIDTH_MAX_US		2000

#define OVERSAMPLING			4

#define CHUNK_SIZE				65536

#define MAX_NAME_LEN			32



enum {
	pIdOscTypeA=0,
	pIdOscTypeB,
	pIdMultipleA,
	pIdDetuneA,
	pIdDutyCycleA,
	pIdDutyCycleB,
	pIdPhaseReset,
	pIdOscMixMode,
	pIdPolyphony,
	pIdPortaSpeed,
	pIdNoteCut,
	pIdVelTarget,
	pIdOutputGain,
	NUM_PARAMS
};




const char* const programDefaultNames[NUM_PROGRAMS]={
	"Default",
	"Slow drift",
	"Fat",
	"Harsh 1",
	"Harsh 2",
	"Detuned",
	"Thin",
	"Feedback",
	"Power harmonic",
	"Overdrive",
	"Jumping",
	"User 1",
	"User 2",
	"User 3",
	"User 4",
	"User 5"
};



const char* const pOscMixModeNames[3]={
	"OR",
	"AND",
	"XOR"
};



const char *pVelTargetNames[3]={
	"Volume",
	"Detune",
	"Note cut"
};



const char* const pPolyMixModeNames[4]={
	"Mono",
	"Poly ADD",
	"Poly OR",
	"Poly XOR"
};


enum {
	mEventTypeNote=0,
	mEventTypeProgram,
	mEventTypePitchBend,
	mEventTypeModulation
};



struct MidiQueueStruct
{
	VstInt32 type;
	VstInt32 delta;
	VstInt32 note;
	VstInt32 velocity;
	VstInt32 program;

	float depth;
};



struct SynthChannelStruct
{
	VstInt32 note;

	float freq;
	float freq_new;

	float accA;
	float accB;

	float addA;
	float addB;

	float pulseA;
	float pulseB;

	float pulseDurationA;
	float pulseDurationB;

	VstInt32 outA;
	VstInt32 outB;

	float volume;

	float duration;
	float detune;
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define OTYA		TAG("OTYA")
#define OTYB		TAG("OTYB")
#define MULA		TAG("MULA")
#define DETA		TAG("DETA")
#define DUTA		TAG("DUTA")
#define DUTB		TAG("DUTB")
#define PHAR		TAG("PHAR")
#define MODE		TAG("MODE")
#define POLY		TAG("POLY")
#define POSP		TAG("POSP")
#define NCUT		TAG("NCUT")
#define VTGT		TAG("VTGT")
#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const float ChunkPresetData[]={

	DATA,

	PROG,0,
	MULA,0.000f,DETA,0.000f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,1,
	MULA,0.000f,DETA,0.002f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,2,
	MULA,0.000f,DETA,0.016f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,0.979f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,3,
	MULA,0.204f,DETA,0.000f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,1.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,4,
	MULA,0.498f,DETA,0.000f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,1.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,5,
	MULA,0.217f,DETA,0.004f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,1.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,6,
	MULA,0.000f,DETA,0.000f,OTYA,0.000f,DUTA,0.969f,OTYB,0.000f,DUTB,0.500f,PHAR,0.013f,MODE,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,7,
	MULA,0.000f,DETA,0.003f,OTYA,0.000f,DUTA,0.372f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,0.955f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,8,
	OTYA,0.513f,OTYB,0.532f,MULA,0.232f,DETA,0.589f,DUTA,0.115f,DUTB,0.995f,PHAR,0.000f,MODE,0.133f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,9,
	OTYA,0.531f,OTYB,0.220f,MULA,0.000f,DETA,0.004f,DUTA,0.775f,DUTB,0.500f,PHAR,0.000f,MODE,0.816f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,10,
	OTYA,1.000f,OTYB,0.000f,MULA,0.292f,DETA,0.021f,DUTA,0.089f,DUTB,0.709f,PHAR,0.000f,MODE,0.971f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,11,
	MULA,0.000f,DETA,0.000f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,12,
	MULA,0.000f,DETA,0.000f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,13,
	MULA,0.000f,DETA,0.000f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,14,
	MULA,0.000f,DETA,0.000f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,15,
	MULA,0.000f,DETA,0.000f,OTYA,0.000f,DUTA,0.500f,OTYB,0.000f,DUTB,0.500f,PHAR,0.000f,MODE,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,

	DONE

};



class PhaseSynth:public AudioEffectX
{
public:

	PhaseSynth(audioMasterCallback audioMaster);

	~PhaseSynth();

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

	float pDutyCycleA[NUM_PROGRAMS];
	float pDutyCycleB[NUM_PROGRAMS];
	float pMultipleA [NUM_PROGRAMS];
	float pDetuneA   [NUM_PROGRAMS];
	float pOscTypeA  [NUM_PROGRAMS];
	float pOscTypeB  [NUM_PROGRAMS];
	float pPhaseReset[NUM_PROGRAMS];
	float pOscMixMode[NUM_PROGRAMS];
	float pPolyphony [NUM_PROGRAMS];
	float pPortaSpeed[NUM_PROGRAMS];
	float pNoteCut   [NUM_PROGRAMS];
	float pVelTarget [NUM_PROGRAMS];
	float pOutputGain[NUM_PROGRAMS];

	char ProgramName[NUM_PROGRAMS][MAX_NAME_LEN];

	float FloatToMultiple(float value);

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	VstInt32 SaveStringChunk(char *str,float *dst);
	VstInt32 LoadStringChunk(char *str,int max_length,float *src);

	VstInt32 SynthAllocateVoice(VstInt32 note);
	void SynthChannelChangeNote(VstInt32 chn,VstInt32 note);

	void MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity);
	void MidiAddProgramChange(VstInt32 delta,VstInt32 program);
	void MidiAddPitchBend(VstInt32 delta,float depth);
	void MidiAddModulation(VstInt32 delta,float depth);
	bool MidiIsAnyKeyDown(void);

	vector<MidiQueueStruct> MidiQueue;

	unsigned char MidiKeyState[128];

	float MidiPitchBend;
	float MidiPitchBendRange;
	float MidiModulationDepth;
	float MidiModulationCount;

	VstInt32 MidiRPNLSB;
	VstInt32 MidiRPNMSB;
	VstInt32 MidiDataLSB;
	VstInt32 MidiDataMSB;

	VstInt32 MidiModulationMSB;

	SynthChannelStruct SynthChannel[SYNTH_CHANNELS];

	float sSlideStep;

	float Chunk[CHUNK_SIZE];
};
