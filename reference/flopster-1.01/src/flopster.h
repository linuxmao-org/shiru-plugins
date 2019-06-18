#include <windows.h>
#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"Flopster"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v1.01 03.06.19"
#define PLUGIN_UID				'fstr'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				2
#define NUM_PROGRAMS			10

#define MAX_NAME_LEN			MAX_PATH

#define STEP_SAMPLES_ALL		80

#define HEAD_BASE_NOTE			(12*4)
#define HEAD_BUZZ_RANGE			(12*3)
#define HEAD_SEEK_RANGE			(12*3)

#define SPECIAL_NOTE			(HEAD_BASE_NOTE+(HEAD_BUZZ_RANGE|HEAD_SEEK_RANGE))
#define SPINDLE_NOTE			(SPECIAL_NOTE+0)
#define SINGLE_STEP_NOTE		(SPECIAL_NOTE+2)
#define DISK_PUSH_NOTE			(SPECIAL_NOTE+4)
#define DISK_INSERT_NOTE		(SPECIAL_NOTE+5)
#define DISK_EJECT_NOTE			(SPECIAL_NOTE+7)
#define DISK_PULL_NOTE			(SPECIAL_NOTE+9)


enum {
	pIdHeadStepGain=0,
	pIdHeadSeekGain,
	pIdHeadBuzzGain,
	pIdSpindleGain,
	pIdNoisesGain,
	pIdOutputGain,
	NUM_PARAMS
};



enum {
	mEventTypeNote=0
};



struct MidiQueueStruct
{
	VstInt32 type;
	VstInt32 delta;
	VstInt32 note;
	VstInt32 velocity;

	float depth;
};



struct sampleStruct
{
	unsigned char *src;

	signed short int *wave;

	int loop_start;
	int loop_end;

	int length;
};



class Flopster:public AudioEffectX
{
public:

	Flopster(audioMasterCallback audioMaster);

	~Flopster();

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

	VstInt32 processEvents(VstEvents* ev);
	void processReplacing(float**inputs,float **outputs,VstInt32 sampleFrames);

protected:

	static void handle_func(void) { };

	float pHeadStepGain;
	float pHeadSeekGain;
	float pHeadBuzzGain;
	float pSpindleGain;
	float pNoisesGain;
	float pOutputGain;

	VstInt32 Program;

	char ProgramName[NUM_PROGRAMS][MAX_NAME_LEN];

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	void MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity);
	bool MidiIsAnyKeyDown(void);

	vector<MidiQueueStruct> MidiQueue;

	unsigned char MidiKeyState[128];

	sampleStruct SampleHeadStep[STEP_SAMPLES_ALL];
	sampleStruct SampleHeadBuzz[HEAD_BUZZ_RANGE];
	sampleStruct SampleHeadSeek[HEAD_SEEK_RANGE];

	sampleStruct SampleDiskPush;
	sampleStruct SampleDiskInsert;
	sampleStruct SampleDiskEject;
	sampleStruct SampleDiskPull;

	void ResetPlayer(void);
	void LoadAllSamples(void);
	void FreeAllSamples(void);

	void SampleLoad(sampleStruct *sample,char *filename);
	void SampleFree(sampleStruct *sample);
	float SampleRead(sampleStruct *sample,double pos);

	void FloppyStartHeadSample(sampleStruct *sample,float gain,bool loop,float relative);
	void FloppyStep(int pos);
	void FloppySpindle(bool enable);

	char PluginDir[MAX_PATH];

	struct
	{
		sampleStruct spindle_sample;

		double spindle_sample_ptr;
		bool spindle_enable;

		sampleStruct *head_sample;

		double head_sample_ptr;
		bool   head_sample_loop;
		float  head_sample_relative_ptr;

		int    head_pos;
		int    head_dir;

		float  head_gain;
		

		float low_freq_acc;
		float low_freq_add;

	} FDD;
};
