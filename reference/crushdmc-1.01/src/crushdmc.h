#include <stdio.h>
#include <math.h>

#include "audioeffectx.h"


#define PLUGIN_NAME			"CrushDMC"
#define PLUGIN_VENDOR		"Shiru"
#define PLUGIN_PRODUCT		"v0.11 02.06.19"
#define PLUGIN_UID			'cdmc'

#define NUM_INPUTS			2
#define NUM_OUTPUTS			2
#define NUM_PROGRAMS		1

#define MAX_NAME_LEN		32



enum {
	pIdInputGain=0,
	pIdSampleRate,
	pIdOutputGain,
	NUM_PARAMS
};



const double sSampleRate[]={

	4177.40,	//PAL 0..15
	4696.63,
	5261.41,
	5579.22,
	6023.94,
	7044.94,
	7917.18,
	8397.01,
	9446.63,
	11233.8,
	12595.5,
	14089.9,
	16965.4,
	21315.5,
	25191.0,
	33252.1,

	4181.71,	//NTSC 0..15
	4709.93,
	5264.04,
	5593.04,
	6257.95,
	7046.35,
	7919.35,
	8363.42,
	9419.86,
	11186.1,
	12604.0,
	13982.6,
	16884.6,
	21306.8,
	24858.0,
	33143.9

};



class CrushDMC:public AudioEffectX
{
public:

	CrushDMC(audioMasterCallback audioMaster);

	~CrushDMC();

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
	float pSampleRate;
	float pOutputGain;

	char ProgramName[MAX_NAME_LEN];

	float sSampleIn;
	float sSampleAcc;
	float sSampleMax;
	float sOutBuf[4];

	VstInt32 sOutputCurrent;
	VstInt32 sSilence;
};
