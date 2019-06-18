#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"1bitstudio - sweepsynth"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v0.13 03.06.19"
#define PLUGIN_UID				'1sws'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			16

#define SYNTH_CHANNELS			8

#define PULSE_WIDTH_MAX_US		4000

#define OVERSAMPLING			16

#define CHUNK_SIZE				65536

#define MAX_NAME_LEN			32



enum {
	pIdPulseWidth=0,
	pIdPulseMin,
	pIdPulseMax,
	pIdPulseSweep,
	pIdPolyphony,
	pIdPortaSpeed,
	pIdNoteCut,
	pIdVelTarget,
	pIdOutputGain,
	NUM_PARAMS
};




const char* const programDefaultNames[NUM_PROGRAMS]={
	"Pronounced",
	"Thin",
	"Sitar",
	"Fat",
	"Slow",
	"Decay",
	"User 1",
	"User 2",
	"User 3",
	"User 4",
	"User 5",
	"User 6",
	"User 7",
	"User 8",
	"User 9",
	"User 10"
};



const char* const pPolyphonyModeNames[4]={
	"Mono",
	"Poly PWM",
	"Poly XOR",
	"Poly ADD",
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

	float acc;
	float add;

	float pulse;
	float pulse_width;
	float pulse_sweep;

	float duration;

	float volume;
};



const char *pVelTargetNames[3]={
	"Volume",
	"Pulse sweep",
	"Note cut"
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define PWDT		TAG("PWDT")
#define PMIN		TAG("PMIN")
#define PMAX		TAG("PMAX")
#define PSWP		TAG("PSWP")
#define POLY		TAG("POLY")
#define POSP		TAG("POSP")
#define NCUT		TAG("NCUT")
#define VTGT		TAG("VTGT")
#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const float ChunkPresetData[]=
{

	DATA,

	PROG,0,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,1,
	PWDT,0.016f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,2,
	PWDT,0.110f,PMIN,0.018f,PMAX,1.000f,PSWP,0.406f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,3,
	PWDT,0.890f,PMIN,0.262f,PMAX,1.000f,PSWP,0.146f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,4,
	PWDT,0.041f,PMIN,0.000f,PMAX,0.880f,PSWP,0.871f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,5,
	PWDT,0.015f,PMIN,0.000f,PMAX,1.000f,PSWP,0.494f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,6,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,7,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,8,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,9,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,10,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,11,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,12,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,13,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,14,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,15,
	PWDT,0.098f,PMIN,0.000f,PMAX,1.000f,PSWP,0.500f,POLY,0.900f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,

	DONE

};



class SweepSynth:public AudioEffectX
{
public:

	SweepSynth(audioMasterCallback audioMaster);

	~SweepSynth();

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

	float pPulseWidth[NUM_PROGRAMS];
	float pPulseMin  [NUM_PROGRAMS];
	float pPulseMax  [NUM_PROGRAMS];
	float pPulseSweep[NUM_PROGRAMS];
	float pPolyphony [NUM_PROGRAMS];
	float pPortaSpeed[NUM_PROGRAMS];
	float pNoteCut   [NUM_PROGRAMS];
	float pVelTarget [NUM_PROGRAMS];
	float pOutputGain[NUM_PROGRAMS];

	char ProgramName [NUM_PROGRAMS][MAX_NAME_LEN];

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
