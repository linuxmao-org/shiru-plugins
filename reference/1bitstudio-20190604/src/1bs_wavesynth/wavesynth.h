#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"1bitstudio - wavesynth"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v0.22 03.06.19"
#define PLUGIN_UID				'1wvs'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			16

#define SYNTH_CHANNELS			8

#define OVERSAMPLING			16

#define MOD_MAX_RATE			10

#define CHUNK_SIZE				65536

#define MAX_NAME_LEN			32



enum {
	pIdWaveformLength=0,
	pIdWaveformNibble0,
	pIdWaveformNibble1,
	pIdWaveformNibble2,
	pIdWaveformNibble3,
	pIdWaveformNibble4,
	pIdWaveformNibble5,
	pIdWaveformNibble6,
	pIdWaveformNibble7,
	pIdModulation,
	pIdPolyphony,
	pIdPortaSpeed,
	pIdNoteCut,
	pIdVelTarget,
	pIdOutputGain,
	NUM_PARAMS
};



const unsigned int pWaveformLengthList[4]={4,8,16,32};



const char* const pWaveformBitPatterns[16]={
	"0000",
	"1000",
	"0100",
	"1100",
	"0010",
	"1010",
	"0110",
	"1110",
	"0001",
	"1001",
	"0101",
	"1101",
	"0011",
	"1011",
	"0111",
	"1111"
};



const char* const programDefaultNames[NUM_PROGRAMS]={
	"Pulse 25%",
	"Pulse 50%",
	"Double soft",
	"Double sharp",
	"Double medium",
	"Harsh",
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

	float volume;

	float duration;
};



const char *pVelTargetNames[3]={
	"Volume",
	"Note cut"
};



const char* const pPolyMixModeNames[4]={
	"Mono",
	"Poly ADD",
	"Poly OR",
	"Poly XOR"
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define WLEN		TAG("WLEN")
#define MODU		TAG("MODU")
#define POLY		TAG("POLY")
#define POSP		TAG("POSP")
#define NCUT		TAG("NCUT")
#define VTGT		TAG("VTGT")
#define GAIN		TAG("GAIN")
#define WN00		TAG("WN00")
#define WN01		TAG("WN01")
#define WN02		TAG("WN02")
#define WN03		TAG("WN03")
#define WN04		TAG("WN04")
#define WN05		TAG("WN05")
#define WN06		TAG("WN06")
#define WN07		TAG("WN07")
#define DONE		TAG("DONE")



const float ChunkPresetData[]={

	DATA,

	PROG,0,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,1,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.233f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,2,
	WLEN,0.460f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.325f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,3,
	WLEN,0.590f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.079f,WN01,0.159f,WN02,0.279f,WN03,0.539f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,4,
	WLEN,0.675f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,1.000f,WN01,0.000f,WN02,0.761f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,5,
	WLEN,1.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.000f,WN01,0.634f,WN02,0.000f,WN03,0.754f,WN04,0.000f,WN05,0.866f,WN06,0.000f,WN07,1.000f,
	PROG,6,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,7,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,8,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,9,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,10,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,11,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,12,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,13,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,14,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,
	PROG,15,
	WLEN,0.000f,MODU,0.000f,POLY,0.260f,POSP,1.000f,NCUT,0.000f,VTGT,0.000f,GAIN,1.000f,WN00,0.100f,WN01,0.000f,WN02,0.000f,WN03,0.000f,WN04,0.000f,WN05,0.000f,WN06,0.000f,WN07,0.000f,

	DONE

};



class WaveSynth:public AudioEffectX
{
public:

	WaveSynth(audioMasterCallback audioMaster);

	~WaveSynth();

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

	VstInt32 pWaveformLengthInt;

	float pWaveformLength[NUM_PROGRAMS];
	float pWaveformNibble[NUM_PROGRAMS][8];
	float pModulation    [NUM_PROGRAMS];
	float pPolyphony     [NUM_PROGRAMS];
	float pPortaSpeed    [NUM_PROGRAMS];
	float pNoteCut       [NUM_PROGRAMS];
	float pVelTarget     [NUM_PROGRAMS];
	float pOutputGain    [NUM_PROGRAMS];

	char ProgramName[NUM_PROGRAMS][MAX_NAME_LEN];

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	VstInt32 SaveStringChunk(char *str,float *dst);
	VstInt32 LoadStringChunk(char *str,int max_length,float *src);

	void NibbleToString(VstInt32 nibble,char *text);

	VstInt32 SynthAllocateVoice(VstInt32 note);
	void SynthChannelChangeNote(VstInt32 chn,VstInt32 note);
	void SynthUpdateWaveform(void);

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
	float sModCnt;

	unsigned int sWaveformSrc;
	unsigned int sWaveform;
	unsigned int sWaveformShift;

	float Chunk[CHUNK_SIZE];
};
