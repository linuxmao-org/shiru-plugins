#include "PulseDrive.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new PulseDrive(audioMaster);
}



PulseDrive::PulseDrive(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	setNumInputs(NUM_INPUTS);
	setNumOutputs(NUM_OUTPUTS);
	setUniqueID(PLUGIN_UID);

	canProcessReplacing();

	strcpy(ProgramName,"Default"); // default program name 

	pInputGain       = .5f;
	pThreshold       = .5f;
	pOutputGain      =1.0f;
	pOutputPulseWidth= .5f;
	pVolumeDetection =1.0f*.16f;
	pOversampling    =1.0f/5.9f*5.0f;

	ResetChannel(0);
	ResetChannel(1);

	suspend();
}



PulseDrive::~PulseDrive()
{
}



void PulseDrive::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdInputGain:	     pInputGain       =value; break;
	case pIdTreshold:        pThreshold       =value; break;
	case pIdPulseWidth:      pOutputPulseWidth=value; break;
	case pIdVolumeDetection: pVolumeDetection =value; break;
	case pIdOversampling:    pOversampling    =value; break;
	case pIdOutputGain:      pOutputGain      =value; break;
	}
}



float PulseDrive::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdInputGain:	     return pInputGain;
	case pIdTreshold:        return pThreshold;
	case pIdPulseWidth:      return pOutputPulseWidth;
	case pIdVolumeDetection: return pVolumeDetection;
	case pIdOversampling:    return pOversampling;
	case pIdOutputGain:      return pOutputGain;
	}

	return 0;
} 



void PulseDrive::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdInputGain:	     strcpy(label,"Input gain");  break;
	case pIdTreshold:        strcpy(label,"Threshold");   break;
	case pIdPulseWidth:      strcpy(label,"Pulse width"); break;
	case pIdVolumeDetection: strcpy(label,"Volume detection"); break;
	case pIdOversampling:    strcpy(label,"Oversampling"); break;
	case pIdOutputGain:      strcpy(label,"Output gain"); break;
	}
} 



void PulseDrive::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdInputGain:       dB2string(pInputGain,text,4); break;
	case pIdTreshold:        float2string(pThreshold,text,4); break;
	case pIdPulseWidth:      float2string(1000000.0f/64.0f*pOutputPulseWidth/40.0f,text,5); break;
	case pIdVolumeDetection: float2string(100.0f*pVolumeDetection,text,4); break;
	case pIdOversampling:    int2string((int)pOversamplingValueArray[int(pOversampling*5.9f)],text,2);break;
	case pIdOutputGain:      dB2string(pOutputGain  ,text,4); break;
	}
} 



void PulseDrive::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdInputGain:
	case pIdOutputGain:      strcpy(label,"dB"); break;
	case pIdTreshold:        strcpy(label,""); break;
	case pIdPulseWidth:      strcpy(label,"us"); break;
	case pIdVolumeDetection: strcpy(label,"ms"); break;
	case pIdOversampling:    strcpy(label,"x"); break;
	}
} 



VstInt32 PulseDrive::getProgram(void)
{
	return 0;
}



void PulseDrive::setProgram(VstInt32 program)
{
}



void PulseDrive::getProgramName(char* name)
{
	strcpy(name,ProgramName); 
}



void PulseDrive::setProgramName(char* name)
{
	strncpy(ProgramName,name,MAX_NAME_LEN);

	ProgramName[MAX_NAME_LEN-1]='\0';
} 



void PulseDrive::ResetChannel(int channel)
{
	sAverageVolume           [channel]=0;
	sAverageVolumeAccumulator[channel]=0;
	sAverageVolumeCounter    [channel]=0;
	sPrevState               [channel]=0;
	sPulseWidthCounter       [channel]=0;
}



float PulseDrive::NextSample(int channel,float in)
{
	int i,state;
	float out,threshold;

	in=in*pInputGain;
	out=0;

	threshold=(pThreshold*2.0f-1.0f)/16.0f;
	
	if(threshold<0)
	{
		if(in<threshold) state=1; else state=0;
	}
	else
	{
		if(in>threshold) state=1; else state=0;
	}

	sAverageVolumeAccumulator[channel]+=(float)fabs(in);
	
	++sAverageVolumeCounter[channel];

	if(sAverageVolumeCounter[channel]>=pVolumeBlockSize)
	{
		sAverageVolume           [channel]=sAverageVolumeAccumulator[channel]/pVolumeBlockSize;
		sAverageVolumeAccumulator[channel]=0;
		sAverageVolumeCounter    [channel]=0;

		if(sAverageVolume[channel]>1.0f) sAverageVolume[channel]=1.0f;
	}

	if(state!=sPrevState[channel])
	{
		sPrevState        [channel]=state;
		sPulseWidthCounter[channel]=pOutputPulseWidthValue*sAverageVolume[channel];
	}

	for(i=0;i<(int)pOversamplingValue;++i)
	{
		if(sPulseWidthCounter[channel]>0)
		{
			sPulseWidthCounter[channel]-=1.0f;

			if(sPulseWidthCounter[channel]<0) sPulseWidthCounter[channel]=0;

			out+=pOutputGain;
		}
	}

	out/=pOversamplingValue;

	return (pThreshold<.5f)?-out:out;
}



void PulseDrive::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *inL =inputs[0];
	float *inR =inputs[1];
	float *outL=outputs[0];
	float *outR=outputs[1];

	pVolumeBlockSize=getSampleRate()*100.0f*pVolumeDetection/1000.0f;

	if(pVolumeBlockSize<1.0f) pVolumeBlockSize=1.0f;

	pOversamplingValue=pOversamplingValueArray[int(pOversampling*5.9f)];

	pOutputPulseWidthValue=getSampleRate()/64.0f*pOutputPulseWidth*pOversamplingValue;

	while(--sampleFrames>=0)
	{
		(*outL++)=NextSample(0,*inL++);
		(*outR++)=NextSample(1,*inR++);
	}
}
