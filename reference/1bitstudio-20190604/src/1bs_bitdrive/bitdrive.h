#include <math.h>

#include "audioeffectx.h"


#define PLUGIN_NAME			"1bitstudio - bitdrive"
#define PLUGIN_VENDOR		"Shiru"
#define PLUGIN_PRODUCT		"v0.12 02.06.19"
#define PLUGIN_UID			'1bbd'

#define NUM_INPUTS			2
#define NUM_OUTPUTS			2
#define NUM_PROGRAMS		1

#define MAX_NAME_LEN		32



enum {
	pIdInputGain=0,
	pIdTreshold,
	pIdOutputGain,
	NUM_PARAMS
};



class BitDrive:public AudioEffectX
{
public:

	BitDrive(audioMasterCallback audioMaster);

	~BitDrive();

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

	char ProgramName[MAX_NAME_LEN];
};
