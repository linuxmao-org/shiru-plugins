#include <math.h>

#include "audioeffectx.h"


#define PLUGIN_NAME			"1bitstudio - pulsedrive"
#define PLUGIN_VENDOR		"Shiru"
#define PLUGIN_PRODUCT		"v0.12 03.06.19"
#define PLUGIN_UID			'1bpd'

#define NUM_INPUTS			2
#define NUM_OUTPUTS			2
#define NUM_PROGRAMS		1

#define MAX_NAME_LEN		32



enum {
	pIdInputGain=0,
	pIdTreshold,
	pIdPulseWidth,
	pIdVolumeDetection,
	pIdOversampling,
	pIdOutputGain,
	NUM_PARAMS
};



const float pOversamplingValueArray[]={1,2,4,8,16,32};



class PulseDrive:public AudioEffectX
{
public:

	PulseDrive(audioMasterCallback audioMaster);

	~PulseDrive();

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

	void processReplacing(float**inputs,float **outputs,VstInt32 sampleFrames);

protected:

	float pInputGain;
	float pThreshold;
	float pOutputGain;
	float pOutputPulseWidth;
	float pVolumeDetection;
	float pOversampling;

	float pVolumeBlockSize;
	float pOversamplingValue;
	float pOutputPulseWidthValue;

	char ProgramName[MAX_NAME_LEN];

	void  ResetChannel(int channel);
	float NextSample(int channel,float in);

	float sAverageVolume[NUM_OUTPUTS];
	float sAverageVolumeAccumulator[NUM_OUTPUTS];
	int sAverageVolumeCounter[NUM_OUTPUTS];
	int sPrevState[NUM_OUTPUTS];
	float sPulseWidthCounter[NUM_OUTPUTS];
};
