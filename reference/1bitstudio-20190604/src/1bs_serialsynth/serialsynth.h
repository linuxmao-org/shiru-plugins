#include <cmath>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"1bitstudio - serialsynth"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v0.1 04.06.19"
#define PLUGIN_UID				'1srs'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			16

#define SYNTH_CHANNELS			8

#define PULSE_WIDTH_MAX_US		2000

#define OVERSAMPLING			4

#define CHUNK_SIZE				65536

#define MAX_NAME_LEN			32



enum {
	pIdOsc1Active=0,
	pIdOsc2Active,
	pIdOsc3Active,
	pIdOsc1Mode,
	pIdOsc2Mode,
	pIdOsc3Mode,
	pIdOsc1Note,
	pIdOsc2Note,
	pIdOsc3Note,
	pIdOsc1Detune,
	pIdOsc2Detune,
	pIdOsc3Detune,
	pIdOsc1Phase,
	pIdOsc2Phase,
	pIdOsc3Phase,
	pIdOsc1Output,
	pIdOsc2Output,
	pIdOsc3Output,
	pIdOscMixMode,
	pIdPolyphony,
	pIdPortaSpeed,
	pIdNoteCut,
	pIdVelTarget,
	pIdOutputGain,
	NUM_PARAMS
};




const char* const programDefaultNames[NUM_PROGRAMS]={
	"Mod 1",
	"Mod 2",
	"Octaves",
	"Mod 3",
	"User 5",
	"User 6",
	"User 7",
	"User 8",
	"User 9",
	"User 10",
	"User 11",
	"User 12",
	"User 13",
	"User 14",
	"User 15",
	"User 16"
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



const char* const pOsc1ActiveNames[3]={
	"Always",
	"Osc 2 out",
	"Osc 3 out"
};



const char* const pOsc2ActiveNames[3]={
	"Always",
	"Osc 1 out",
	"Osc 3 out"
};



const char* const pOsc3ActiveNames[3]={
	"Always",
	"Osc 1 out",
	"Osc 2 out"
};



const char* const pOscMixModeNames[5]={
	"ADD 4:4:4",
	"ADD 1:2:4",
	"AND",
	"OR",
	"XOR"
};



const char *pVelTargetNames[3]={
	"Volume",
	"Detune",
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

	float acc1;
	float acc2;
	float acc3;

	float add1;
	float add2;
	float add3;

	VstInt32 out1;
	VstInt32 out2;
	VstInt32 out3;

	float volume;

	float duration;
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")

#define O1AC		TAG("O1AC")
#define O1MD		TAG("O1MD")
#define O1NT		TAG("O1NT")
#define O1DE		TAG("O1DE")
#define O1PH		TAG("O1PH")
#define O1UT		TAG("O1UT")

#define O2AC		TAG("O2AC")
#define O2MD		TAG("O2MD")
#define O2NT		TAG("O2NT")
#define O2DE		TAG("O2DE")
#define O2PH		TAG("O2PH")
#define O2UT		TAG("O2UT")

#define O3AC		TAG("O3AC")
#define O3MD		TAG("O3MD")
#define O3NT		TAG("O3NT")
#define O3DE		TAG("O3DE")
#define O3PH		TAG("O3PH")
#define O3UT		TAG("O3UT")

#define OMIX		TAG("OMIX")
#define POLY		TAG("POLY")
#define POSP		TAG("POSP")
#define NCUT		TAG("NCUT")
#define VTGT		TAG("VTGT")
#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const float ChunkPresetData[]={

	DATA,

	PROG,0,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.380f,O1DE,0.500f,O1PH,0.000f,O1UT,0.000f,
	O2AC,0.500f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.500f,O3MD,1.000f,O3NT,0.500f,O3DE,0.454f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.922f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,1,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.467f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.576f,O2DE,0.545f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.633f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,1.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,2,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.385f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.633f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,3,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,0.000f,
	O2AC,0.500f,O2MD,1.000f,O2NT,0.632f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.527f,O3MD,1.000f,O3NT,0.554f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.842f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,4,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,5,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,6,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,7,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,8,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,9,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,10,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,11,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,12,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,13,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,14,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,
	PROG,15,
	O1AC,0.000f,O1MD,1.000f,O1NT,0.500f,O1DE,0.500f,O1PH,0.000f,O1UT,1.000f,
	O2AC,0.000f,O2MD,1.000f,O2NT,0.500f,O2DE,0.500f,O2PH,0.000f,O2UT,1.000f,
	O3AC,0.000f,O3MD,1.000f,O3NT,0.500f,O3DE,0.500f,O3PH,0.000f,O3UT,1.000f,
	OMIX,0.000f,POLY,1.000f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,

	DONE

};



class SerialSynth:public AudioEffectX
{
public:

	SerialSynth(audioMasterCallback audioMaster);

	~SerialSynth();

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

	float pOsc1Active[NUM_PROGRAMS];
	float pOsc1Mode  [NUM_PROGRAMS];
	float pOsc1Note  [NUM_PROGRAMS];
	float pOsc1Detune[NUM_PROGRAMS];
	float pOsc1Phase [NUM_PROGRAMS];
	float pOsc1Output[NUM_PROGRAMS];
	float pOsc2Active[NUM_PROGRAMS];
	float pOsc2Mode  [NUM_PROGRAMS];
	float pOsc2Note  [NUM_PROGRAMS];
	float pOsc2Detune[NUM_PROGRAMS];
	float pOsc2Phase [NUM_PROGRAMS];
	float pOsc2Output[NUM_PROGRAMS];
	float pOsc3Active[NUM_PROGRAMS];
	float pOsc3Mode  [NUM_PROGRAMS];
	float pOsc3Note  [NUM_PROGRAMS];
	float pOsc3Detune[NUM_PROGRAMS];
	float pOsc3Phase [NUM_PROGRAMS];
	float pOsc3Output[NUM_PROGRAMS];
	float pOscMixMode[NUM_PROGRAMS];
	float pPolyphony [NUM_PROGRAMS];
	float pPortaSpeed[NUM_PROGRAMS];
	float pNoteCut   [NUM_PROGRAMS];
	float pVelTarget [NUM_PROGRAMS];
	float pOutputGain[NUM_PROGRAMS];

	char ProgramName[NUM_PROGRAMS][MAX_NAME_LEN];

	float FloatToRelativeNote(float n);
	float FloatToAbsoluteNote(float n);

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
