#include <windows.h>
#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"
#include "ev.h"


#define PLUGIN_NAME				"EVALUA"
#define PLUGIN_VENDOR			"v1.01 30.05.19 Shiru"
#define PLUGIN_UID				'evau'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			128

#define MAX_PROGRAM_LEN			1024

#define MIDI_CHANNELS_ALL		1

#define CHUNK_SIZE				(NUM_PROGRAMS*MAX_PROGRAM_LEN)

#define MAX_NAME_LEN			32

#define MAX_SYNTH_CHANNELS		8

#define NORMAL_GAIN				100
#define MAX_GAIN				(NORMAL_GAIN*2)

#define MAX_PORTA_SPEED			100



enum {
	NUM_PARAMS=0
};



enum {
	mEventTypeNote=0,
	mEventTypePitchBend,
	mEventTypeModulation,
	mEventTypeProgram,
	mEventTypeControl
};



struct MidiQueueStruct
{
	VstInt32 type;
	VstInt32 delta;
	VstInt32 channel;
	VstInt32 note;
	VstInt32 velocity;
	VstInt32 program;
	VstInt32 value;

	float depth;
};




struct ProgramStruct {

	char Name[MAX_NAME_LEN];

	char Data[MAX_PROGRAM_LEN];
};



struct ChannelStruct {

	VstInt32 note;
	VstInt32 velocity;
	VstInt64 time_cnt;
	VstInt64 pitch_cnt;

	double time_acc;
	double pitch_acc;

	double freq;
	double freq_new;
};




#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define POLY		TAG("POLY")
#define POSP		TAG("POSP")
#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const char* const PresetData[]={
	"Saw","P&255",
	"Pulse","P&128",
	"Pulse Velocity","(P&256)*V/128",
	"Saw Decay","(P&255)*((255-(T/500))&255)/256",
	"Phat","(P&255)+((P*257/256)&127)",
	"Ringy","(P&255)^((P*257/256)&127)",
	"Pulsating","((P/11)^((P/12)&4095))&(T&255)",
	"Radio","(P*P/200)&255",
	"Velocity","P&V",
	"Arp 1","(P*((T/4096)&3))&255",
	"Hover","P&((P*250/256)&255)",
	"Harsh","(P+(P>>1)^(P>>2))&255",
	"Noise hit","(P*P>>(T/100))&255",
	"Copter","R&255&((P&0XFF0)>>4)",
	"Wide bass","(P&255)-(P*256/257&128)",
	"Space","(P%256)^(P%255)^(P%254)",
	"Gliss Down","(T*100/(P/2000))&128",
	"Synth Key","(T*100/(P/2000))&128",
	"Metallic 1","(P*1000000/T)&255",
	"Metallic 2","(P*10000/(T/10))&128",
	"Distortion","((P+P^T)&P*257/256)&255",
	"Digi Pad","(P*256/257)&(P^(P*257/256))",
	"Arp 2","(T/1500%6)*P&256",
	"\r"
};



class Evalua:public AudioEffectX
{
public:

	Evalua(audioMasterCallback audioMaster);

	~Evalua();

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
	virtual VstInt32 getVendorVersion() { return 1000; } 

	VstInt32 canDo(char* text);
	VstInt32 getNumMidiInputChannels(void);
	VstInt32 getNumMidiOutputChannels(void);

	virtual VstInt32 getChunk(void** data,bool isPreset=false);
	virtual VstInt32 setChunk(void* data,VstInt32 byteSize,bool isPreset=false);

	VstInt32 processEvents(VstEvents* ev);
	void processReplacing(float**inputs,float **outputs,VstInt32 sampleFrames);

	VstInt32 ProgramIndex;
	VstInt32 ProgramIndexPrev;

	VstInt32 PortaSpeed;		//0 slowest, 100 fastest
	VstInt32 Polyphony;			//max number of active channels, 1 for mono mode
	VstInt32 OutputGain;		//100 normal volume, <100 reduce, >100 amplify

	ProgramStruct Program[NUM_PROGRAMS];

	void SetUpdateGUI(void);
	void UpdateGUI(void);
	const char* Compile(void);

protected:

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	VstInt32 SaveStringChunk(char *str,float *dst);
	VstInt32 LoadStringChunk(char *str,int max_length,float *src);

	float Chunk[CHUNK_SIZE];

	void MidiAddNote(VstInt32 delta, VstInt32 channel, VstInt32 note, VstInt32 velocity);
	void MidiAddPitchBend(VstInt32 delta, VstInt32 channel, float depth);
	void MidiAddModulation(VstInt32 delta, float depth);
	void MidiAddProgramChange(VstInt32 delta, VstInt32 channel, VstInt32 program);
	void MidiAddControl(VstInt32 delta, VstInt32 channel, VstInt32 value);

	bool MidiIsAnyKeyDown(void);

	vector<MidiQueueStruct> MidiQueue;

	unsigned char MidiKeyState[128];

	double MidiPitchBend;
	double MidiPitchBendRange;

	VstInt64 MidiModulationDepth;

	VstInt32 MidiRPNLSB;
	VstInt32 MidiRPNMSB;
	VstInt32 MidiDataLSB;
	VstInt32 MidiDataMSB;

	ChannelStruct SynthChannel[MAX_SYNTH_CHANNELS];

	VstInt32 SynthAllocateVoice(VstInt32 midi_ch,VstInt32 note);
	void SynthChannelChangeNote(VstInt32 chn,VstInt32 midi_ch,VstInt32 note,VstInt32 velocity);
	void SynthChannelReleaseNote(VstInt32 midi_ch,VstInt32 note);

	bool UpdateGuiFlag;

	double SlideStep;

	EV* ev;
};
