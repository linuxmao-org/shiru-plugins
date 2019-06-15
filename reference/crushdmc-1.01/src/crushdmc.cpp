#include "CrushDMC.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new CrushDMC(audioMaster);
}



CrushDMC::CrushDMC(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 i;

	setNumInputs(NUM_INPUTS);
	setNumOutputs(NUM_OUTPUTS);
	setUniqueID(PLUGIN_UID);

	canProcessReplacing();

	strcpy(ProgramName,"Default"); // default program name 

	pInputGain =1.0f;
	pSampleRate=0.99f;
	pOutputGain=1.0f;

	sSampleIn=0;
	sOutputCurrent=0;
	sSilence=0;

	for(i=0;i<4;++i) sOutBuf[i]=0;

	suspend();
}



CrushDMC::~CrushDMC()
{
}



void CrushDMC::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdInputGain:	pInputGain =value; break;
	case pIdSampleRate:	pSampleRate=value; break;
	case pIdOutputGain: pOutputGain=value; break;
	}
}



float CrushDMC::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdInputGain:	return pInputGain;
	case pIdSampleRate:	return pSampleRate;
	case pIdOutputGain: return pOutputGain;
	}

	return 0;
} 



void CrushDMC::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdInputGain:	strcpy(label,"Input gain");  break;
	case pIdSampleRate:	strcpy(label,"Sample rate"); break;
	case pIdOutputGain: strcpy(label,"Output gain"); break;
	}
} 



void CrushDMC::getParameterDisplay(VstInt32 index,char *text)
{
	char str[256];
	int i;

	switch(index)
	{
	case pIdSampleRate:
		{
			i=(int)(31.999f*pSampleRate);

			sprintf(str,"%s $%X - %5.2f",(i<16?"PAL":"NTSC"),(i&15),sSampleRate[i]);

			strcpy(text,str);
		}
		break;

	case pIdInputGain:  dB2string(pInputGain ,text,4); break;
	case pIdOutputGain: dB2string(pOutputGain,text,4); break;
	}
} 



void CrushDMC::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdSampleRate: strcpy(label,"Hz"); break;
	case pIdInputGain:  
	case pIdOutputGain: strcpy(label,"dB"); break;
	}
} 



VstInt32 CrushDMC::getProgram(void)
{
	return 0;
}



void CrushDMC::setProgram(VstInt32 program)
{
}



void CrushDMC::getProgramName(char* name)
{
	strcpy(name,ProgramName); 
}



void CrushDMC::setProgramName(char* name)
{
	strncpy(ProgramName,name,MAX_NAME_LEN);

	ProgramName[MAX_NAME_LEN-1]='\0';
} 



inline float hermite4(float frac_pos, float xm1, float x0, float x1, float x2)
{
	const float c = (x1 - xm1) * 0.5f;
	const float v = x0 - x1;
	const float w = c + v;
	const float a = w + v + (x2 - x0) * 0.5f;
	const float b_neg = w + a;

	return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
}



void CrushDMC::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *inL =inputs[0];
	float *inR =inputs[1];
	float *outL=outputs[0];
	float *outR=outputs[1];
	float f,level;
	double sample_rate,new_rate;
	int indac,max_silence;

	sample_rate=getSampleRate();
	new_rate=sSampleRate[(int)(pSampleRate*31.999f)];

	sSampleMax=(float)(sample_rate/new_rate);

	max_silence=(int)(new_rate/50.0);

	while(--sampleFrames>=0)
	{
		level=(*inL++)*pInputGain;

		sSampleIn+=level;
		sSampleAcc+=1.0f;

		if(sSampleAcc>=sSampleMax)
		{
			level=sSampleIn/sSampleAcc;

			sSampleAcc-=sSampleMax;
			sSampleIn=0;

			indac=(int)(level*64.0f);

			if(sOutputCurrent<indac)
			{
				if(sOutputCurrent<62) sOutputCurrent+=2;
			}
			else
			{
				if(sOutputCurrent>=-62) sOutputCurrent-=2;
			}

			if(!(indac>=-2&&indac<=2))	//silence theshold
			{
				sSilence=0;
			}
			else
			{
				if(sSilence<max_silence) ++sSilence; else sOutputCurrent=0;	//reset output current
			}

			sOutBuf[3]=sOutBuf[2];
			sOutBuf[2]=sOutBuf[1];
			sOutBuf[1]=sOutBuf[0];
			sOutBuf[0]=((float)sOutputCurrent/64.0f);
		}

		f=sSampleAcc/sSampleMax;

		if(f>1.0f) f=1.0f;

		level=hermite4(f,sOutBuf[0],sOutBuf[1],sOutBuf[2],sOutBuf[3]);

		level=level*pOutputGain;

		(*outL++)=level;
		(*outR++)=level;
	}
}
