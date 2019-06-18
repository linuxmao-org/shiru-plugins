#include "BitDrive.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new BitDrive(audioMaster);
}



BitDrive::BitDrive(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	setNumInputs(NUM_INPUTS);
	setNumOutputs(NUM_OUTPUTS);
	setUniqueID(PLUGIN_UID);

	canProcessReplacing();

	strcpy(ProgramName,"Default"); // default program name 

	pInputGain = .5f;
	pThreshold = .5f;
	pOutputGain=1.0f;

	suspend();
}



BitDrive::~BitDrive()
{
}



void BitDrive::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdInputGain:	pInputGain=value; break;
	case pIdTreshold:   pThreshold =value; break;
	case pIdOutputGain: pOutputGain=value; break;
	}
}



float BitDrive::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdInputGain:	return pInputGain;
	case pIdTreshold:   return pThreshold;
	case pIdOutputGain: return pOutputGain;
	}

	return 0;
} 



void BitDrive::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdInputGain:	strcpy(label,"Input gain");  break;
	case pIdTreshold:   strcpy(label,"Threshold");   break;
	case pIdOutputGain: strcpy(label,"Output gain"); break;
	}
} 



void BitDrive::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdInputGain:  dB2string(pInputGain ,text,4); break;
	case pIdTreshold:   float2string(pThreshold*2.0f-1.0f,text,4); break;
	case pIdOutputGain: dB2string(pOutputGain,text,4); break;
	}
} 



void BitDrive::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdTreshold:   strcpy(label,""); break;
	case pIdInputGain:  
	case pIdOutputGain: strcpy(label,"dB"); break;
	}
} 



VstInt32 BitDrive::getProgram(void)
{
	return 0;
}



void BitDrive::setProgram(VstInt32 program)
{
}



void BitDrive::getProgramName(char* name)
{
	strcpy(name,ProgramName); 
}



void BitDrive::setProgramName(char* name)
{
	strncpy(ProgramName,name,MAX_NAME_LEN);

	ProgramName[MAX_NAME_LEN-1]='\0';
} 



void BitDrive::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *inL =inputs[0];
	float *inR =inputs[1];
	float *outL=outputs[0];
	float *outR=outputs[1];
	float level,threshold;

	threshold=(pThreshold*2.0f-1.0f)/16.0f;

	while(--sampleFrames>=0)
	{
		level=(*inL++)*pInputGain;

		if(threshold<0)
		{
			if(level<threshold) level=-pOutputGain; else level=0;
		}
		else
		{
			if(level>threshold) level=pOutputGain; else level=0;
		}

		(*outL++)=level;

		level=(*inR++)*pInputGain;

		if(threshold<0)
		{
			if(level<threshold) level=-pOutputGain; else level=0;
		}
		else
		{
			if(level>threshold) level=pOutputGain; else level=0;
		}

		(*outR++)=level;
	}
}
