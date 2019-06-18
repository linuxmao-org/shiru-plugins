#include <cmath>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"1bitstudio - noisesynth"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v0.12 03.06.19"
#define PLUGIN_UID				'1nss'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			16

#define SYNTH_CHANNELS			8

#define NOISE_MAX_DURATION_MS	1000.0f
#define LFO_MAX_FREQ_HZ			200.0f
#define DETUNE_RANGE_HZ			500.0f

#define OVERSAMPLING			4

#define CHUNK_SIZE				65536

#define MAX_NAME_LEN			32



enum {
	pIdDurationA=0,
	pIdOscBaseA,
	pIdOscTypeA,
	pIdOscSlideA,
	pIdPeriodA,
	pIdSeedA,
	pIdDurationB,
	pIdOscBaseB,
	pIdOscTypeB,
	pIdOscSlideB,
	pIdPeriodB,
	pIdSeedB,
	pIdMixMode,
	pIdOscDetune,
	pIdPolyphony,
	pIdPortaSpeed,
	pIdNoteCut,
	pIdVelTarget,
	pIdOutputGain,
	NUM_PARAMS
};




const char* const programDefaultNames[NUM_PROGRAMS]={
	"Electric guitar",
	"Disrupting",
	"Glitchy",
	"Dive",
	"Pluck",
	"Engine",
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



const char* const pOscTypeNames[3]={
	"Relative note",
	"Fixed note",
	"LFO"
};



const char* const pMixModeNames[3]={
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



const char *noteNames[96+1]={
	"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1",
	"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2",
	"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3",
	"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4",
	"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5",
	"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6",
	"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7",
	"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8",
	"C-9"
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

	float slideA;
	float slideB;

	float slideDeltaA;
	float slideDeltaB;

	VstInt32 ptrA;
	VstInt32 ptrB;

	VstInt32 periodA;
	VstInt32 periodB;

	VstInt32 seedA;
	VstInt32 seedB;

	VstInt32 outA;
	VstInt32 outB;

	float durationA;
	float durationB;

	float volume;

	float detune;
	float duration;
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define DURA		TAG("DUAA")
#define DURB		TAG("DUBB")
#define BASA		TAG("BSAA")
#define BASB		TAG("BSBB")
#define OFFA		TAG("OFAA")
#define OFFB		TAG("OFBB")
#define SLDA		TAG("SLAA")
#define SLDB		TAG("SLBB")
#define PRDA		TAG("PRAA")
#define PRDB		TAG("PRBB")
#define SEEA		TAG("SEAA")
#define SEEB		TAG("SEBB")
#define DETU		TAG("DETU")
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
	DURA,1.000f,DURB,1.000f,BASA,0.500f,BASB,0.380f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,0.000f,PRDB,0.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.511f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,1,
	DURA,1.000f,DURB,1.000f,BASA,0.000f,BASB,0.500f,OFFA,0.518f,OFFB,0.000f,SLDA,0.500f,SLDB,0.481f,PRDA,1.000f,PRDB,0.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.333f,DETU,1.000f,POLY,0.254f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,2,
	DURA,0.232f,DURB,0.238f,BASA,0.334f,BASB,0.500f,OFFA,0.976f,OFFB,0.009f,SLDA,0.500f,SLDB,0.500f,PRDA,0.000f,PRDB,0.000f,SEEA,0.000f,SEEB,0.036f,MODE,0.000f,DETU,0.500f,POLY,0.385f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,3,
	DURA,1.000f,DURB,1.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.427f,SLDB,0.435f,PRDA,0.000f,PRDB,0.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,4,
	DURA,0.042f,DURB,0.146f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,0.045f,PRDB,0.000f,SEEA,0.000f,SEEB,0.000f,MODE,1.000f,DETU,0.500f,POLY,0.487f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,5,
	DURA,1.000f,DURB,1.000f,BASA,0.633f,BASB,0.000f,OFFA,0.105f,OFFB,0.000f,SLDA,0.500f,SLDB,0.508f,PRDA,1.000f,PRDB,0.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,6,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,7,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,8,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,9,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,10,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,11,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,12,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,13,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,14,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,15,
	DURA,0.200f,DURB,0.000f,BASA,0.500f,BASB,0.500f,OFFA,0.000f,OFFB,0.000f,SLDA,0.500f,SLDB,0.500f,PRDA,1.000f,PRDB,1.000f,SEEA,0.000f,SEEB,0.000f,MODE,0.000f,DETU,0.500f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,

	DONE

};



class NoiseSynth:public AudioEffectX
{
public:

	NoiseSynth(audioMasterCallback audioMaster);

	~NoiseSynth();

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

	float pDurationA [NUM_PROGRAMS];
	float pOscBaseA  [NUM_PROGRAMS];
	float pOscTypeA  [NUM_PROGRAMS];
	float pOscSlideA [NUM_PROGRAMS];
	float pPeriodA   [NUM_PROGRAMS];
	float pSeedA     [NUM_PROGRAMS];
	float pDurationB [NUM_PROGRAMS];
	float pOscBaseB  [NUM_PROGRAMS];
	float pOscTypeB  [NUM_PROGRAMS];
	float pOscSlideB [NUM_PROGRAMS];
	float pPeriodB   [NUM_PROGRAMS];
	float pSeedB     [NUM_PROGRAMS];
	float pMixMode   [NUM_PROGRAMS];
	float pOscDetune [NUM_PROGRAMS];
	float pPolyphony [NUM_PROGRAMS];
	float pPortaSpeed[NUM_PROGRAMS];
	float pNoteCut   [NUM_PROGRAMS];
	float pVelTarget [NUM_PROGRAMS];
	float pOutputGain[NUM_PROGRAMS];

	char ProgramName[NUM_PROGRAMS][MAX_NAME_LEN];

	int FloatToNoisePeriod(float value);

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

	unsigned char Noise[8192];
};
