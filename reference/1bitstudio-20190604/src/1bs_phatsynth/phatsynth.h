#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"1bitstudio - phatsynth"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v0.12 03.06.19"
#define PLUGIN_UID				'1fts'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			16

#define SYNTH_CHANNELS			8
#define VOICES_MAX				16

#define PULSE_WIDTH_MAX_US		2000

#define OVERSAMPLING			16

#define CHUNK_SIZE				65536

#define MAX_NAME_LEN			32



enum {
	pIdType=0,
	pIdPulseWidth,
	pIdVoices,
	pIdDetune,
	pIdPhase,
	pIdVoiceMix,
	pIdPolyphony,
	pIdPortaSpeed,
	pIdNoteCut,
	pIdVelTarget,
	pIdOutputGain,
	NUM_PARAMS
};




const char* const programDefaultNames[NUM_PROGRAMS]={
	"Fat square",
	"Fat pulse",
	"Three pins",
	"Super pin 1",
	"Super pin 2",
	"Power",
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



const char* const pVoiceMixNames[2]={
	"OR",
	"XOR"
};



const char* const pPolyphonyModeNames[4]={
	"Mono",
	"Poly PWM",
	"Poly XOR",
	"Poly ADD",
};



const char *pVelTargetNames[5]={
	"Volume",
	"Detune",
	"Phase reset",
	"Duty/Width",
	"Note cut"
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

	float acc[VOICES_MAX];
	float add;

	float detune[VOICES_MAX];

	float pulse;
	float pulse_width;

	float duration;

	float volume;
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define TYPE		TAG("TYPE")
#define PWDT		TAG("PWDT")
#define VOIC		TAG("VOIC")
#define DETU		TAG("DETU")
#define PHAS		TAG("PHAS")
#define VMIX		TAG("VMIX")
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
	TYPE,0.000f,PWDT,0.500f,VOIC,0.091f,DETU,0.220f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,1,
	TYPE,0.000f,PWDT,0.912f,VOIC,0.082f,DETU,0.114f,PHAS,0.000f,VMIX,0.931f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,2,
	TYPE,0.821f,PWDT,0.265f,VOIC,0.150f,DETU,0.142f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,3,
	TYPE,0.986f,PWDT,0.142f,VOIC,0.486f,DETU,0.197f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,4,
	TYPE,1.000f,PWDT,0.395f,VOIC,0.484f,DETU,0.371f,PHAS,0.000f,VMIX,1.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,5,
	TYPE,0.000f,PWDT,0.063f,VOIC,0.627f,DETU,0.104f,PHAS,0.517f,VMIX,0.990f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,6,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,7,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,8,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,9,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,10,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,11,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,12,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,13,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,14,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,15,
	TYPE,0.000f,PWDT,0.500f,VOIC,0.000f,DETU,0.000f,PHAS,0.000f,VMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,

	DONE

};



class PhatSynth:public AudioEffectX
{
public:

	PhatSynth(audioMasterCallback audioMaster);

	~PhatSynth();

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

	float pType      [NUM_PROGRAMS];
	float pPulseWidth[NUM_PROGRAMS];
	float pVoices    [NUM_PROGRAMS];
	float pDetune    [NUM_PROGRAMS];
	float pPhase     [NUM_PROGRAMS];
	float pVoiceMix  [NUM_PROGRAMS];
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
