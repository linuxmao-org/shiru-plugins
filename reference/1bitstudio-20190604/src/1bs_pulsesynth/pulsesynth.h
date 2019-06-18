#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"1bitstudio - pulsesynth"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v0.23 03.06.19"
#define PLUGIN_UID				'1pls'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			16

#define SYNTH_CHANNELS			8

#define ENVELOPE_UPDATE_RATE_HZ	250

#define ENVELOPE_ATTACK_MAX_MS	10000
#define ENVELOPE_DECAY_MAX_MS	10000
#define ENVELOPE_RELEASE_MAX_MS	1000

#define PULSE_WIDTH_MAX_US		1000

#define OVERSAMPLING			16

#define CHUNK_SIZE				65536

#define MAX_NAME_LEN			32



enum {
	pIdAttack=0,
	pIdDecay,
	pIdSustain,
	pIdRelease,
	pIdPulseWidth,
	pIdDetune,
	pIdLowBoost,
	pIdPolyphony,
	pIdPortaSpeed,
	pIdOutputGain,
	NUM_PARAMS
};




const char* const programDefaultNames[NUM_PROGRAMS]={
	"Standard",
	"Strong",
	"Hard",
	"Thin",
	"Slow",
	"User 1",
	"User 2",
	"User 3",
	"User 4",
	"User 5",
	"User 6",
	"User 7",
	"User 8",
	"User 9",
	"User 10",
	"User 11"
};



const char* const pPolyphonyModeNames[3]={
	"Mono",
	"Poly PWM",
	"Poly ADD"
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



enum {
	eStageReset=0,
	eStageAttack,
	eStageDecay,
	eStageSustain,
	eStageRelease
};



struct SynthChannelStruct
{
	VstInt32 note;

	float freq;
	float freq_new;

	float acc;
	float add;

	VstInt32 e_stage;
	float e_level;
	float e_delta;
	
	float pulse;
	float volume;
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define EATT		TAG("EATT")
#define EDEC		TAG("EDEC")
#define ESUS		TAG("ESUS")
#define EREL		TAG("EREL")
#define PWDT		TAG("PWDT")
#define DETU		TAG("DETU")
#define LOWB		TAG("LOWB")
#define POLY		TAG("POLY")
#define POSP		TAG("POSP")
#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const float ChunkPresetData[]=
{

	DATA,

	PROG,0,
	EATT,0.000f,EDEC,0.250f,ESUS,0.000f,EREL,0.000f,PWDT,0.098f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,1.000f,GAIN,1.000f,
	PROG,1,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.500f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,2,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.175f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,3,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.050f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,4,
	EATT,0.058f,EDEC,0.038f,ESUS,0.000f,EREL,0.000f,PWDT,1.000f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,5,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,6,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,7,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,8,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,9,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,10,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,11,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,12,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,13,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,14,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,
	PROG,15,
	EATT,0.000f,EDEC,0.000f,ESUS,1.000f,EREL,0.000f,PWDT,0.108f,DETU,0.500f,LOWB,0.000f,POLY,0.500f,POSP,0.500f,GAIN,1.000f,

	DONE

};



class PulseSynth:public AudioEffectX
{
public:

	PulseSynth(audioMasterCallback audioMaster);

	~PulseSynth();

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

	float pAttack    [NUM_PROGRAMS];
	float pDecay     [NUM_PROGRAMS];
	float pSustain   [NUM_PROGRAMS];
	float pRelease   [NUM_PROGRAMS];
	float pPulseWidth[NUM_PROGRAMS];
	float pDetune    [NUM_PROGRAMS];
	float pLowBoost  [NUM_PROGRAMS];
	float pPolyphony [NUM_PROGRAMS];
	float pPortaSpeed[NUM_PROGRAMS];
	float pOutputGain[NUM_PROGRAMS];

	char ProgramName [NUM_PROGRAMS][MAX_NAME_LEN];

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	VstInt32 SaveStringChunk(char *str,float *dst);
	VstInt32 LoadStringChunk(char *str,int max_length,float *src);

	VstInt32 SynthAllocateVoice(VstInt32 note);
	void SynthChannelChangeNote(VstInt32 chn,VstInt32 note);
	float SynthEnvelopeTimeToDelta(float value,float max_ms);
	void SynthRestartEnvelope(VstInt32 chn);
	void SynthStopEnvelope(VstInt32 chn);
	void SynthAdvanceEnvelopes(void);

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

	float sEnvelopeDiv;
	float sSlideStep;

	float Chunk[CHUNK_SIZE];
};
