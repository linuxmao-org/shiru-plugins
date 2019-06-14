//code by Shiru, 2018, released under WTFPL

//uses resonant IIR low pass filter code by Olli Niemitalo from early 2000s
//http://www.musicdsp.org/showArchiveComment.php?ArchiveID=27



#include "ChipWave.h"
#include "GUI.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new ChipWave(audioMaster);
}



ChipWave::ChipWave(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 i,pgm,chn,name;

#ifdef _GUI_ACTIVE_
	setEditor((AEffEditor*)new GUI(this));
#endif

	setNumInputs(NUM_INPUTS);
	setNumOutputs(NUM_OUTPUTS);
	setUniqueID(PLUGIN_UID);

	isSynth();
	canProcessReplacing();
	programsAreChunks(true);

	Program=0;

	LoadPresetChunk((float*)ChunkPresetData);

	name=0;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		if(strlen(programDefaultNames[name]))
		{
			strcpy(ProgramName[pgm],programDefaultNames[name++]);
		}
		else
		{
			strcpy(ProgramName[pgm],"default");
		}

		pOscAWave		[pgm]=0;
		pOscADuty		[pgm]=0.5f;
		pOscAOver		[pgm]=0.5f;
		pOscACut		[pgm]=0;
		pOscAMultiple	[pgm]=0.5f;
		pOscASeed		[pgm]=0;

		pOscBWave		[pgm]=0;
		pOscBDuty		[pgm]=0.5f;
		pOscBOver		[pgm]=0.5f;
		pOscBDetune		[pgm]=0;
		pOscBCut		[pgm]=0;
		pOscBMultiple	[pgm]=0.5f;
		pOscBSeed		[pgm]=0;

		pOscBalance     [pgm]=0;
		pOscMixMode     [pgm]=0;

		pFltCutoff		[pgm]=1.0f;
		pFltReso		[pgm]=0;

		pSlideDelay		[pgm]=0;
		pSlideSpeed		[pgm]=0.5f;
		pSlideRoute		[pgm]=0;

		pEnvAttack		[pgm]=0;
		pEnvDecay		[pgm]=0;
		pEnvSustain		[pgm]=1.0f;
		pEnvRelease		[pgm]=0;
		pEnvOscADepth   [pgm]=0.5f;
		pEnvOscBDepth   [pgm]=0.5f;
		pEnvOscBDetuneDepth[pgm]=0;
		pEnvOscMixDepth [pgm]=0.5f;
		pEnvFltDepth	[pgm]=0.5f;
		pEnvLfoDepth	[pgm]=0;

		pLfoSpeed   	[pgm]=0;
		pLfoPitchDepth  [pgm]=.125f;
		pLfoOscADepth   [pgm]=0.5f;
		pLfoOscBDepth   [pgm]=0.5f;
		pLfoOscMixDepth [pgm]=0.5f;
		pLfoFltDepth	[pgm]=0.5f;

		pAmpAttack		[pgm]=0;
		pAmpDecay		[pgm]=0;
		pAmpSustain		[pgm]=1.0f;
		pAmpRelease		[pgm]=0;

		pVelAmp		    [pgm]=1.0f;
		pVelOscADepth	[pgm]=0.5f;
		pVelOscBDepth	[pgm]=0.5f;
		pVelOscMixDepth [pgm]=0.5f;
		pVelFltCutoff	[pgm]=0.5f;
		pVelFltReso		[pgm]=0.5f;

		pPolyphony		[pgm]=1.0f;
		pPortaSpeed		[pgm]=1.0f;
		pOutputGain		[pgm]=1.0f;
	}

	memset(SynthChannel,0,sizeof(SynthChannel));

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].note=-1;

		SynthChannel[chn].ev_stage=eStageReset;
		SynthChannel[chn].ef_stage=eStageReset;

		SynthChannel[chn].volume=1.0f;
	}

	srand(1);
	
	for(i=0;i<65536;++i) Noise[i]=rand()&1;

	sEnvelopeDiv=0;

	MidiQueue.clear();

	MidiRPNLSB=0;
	MidiRPNMSB=0;
	MidiDataLSB=0;
	MidiDataMSB=0;

	MidiPitchBend=0;
	MidiPitchBendRange=2.0f;
	MidiModulationDepth=0;
	MidiModulationCount=0;

	sSlideStep=0;

	memset(MidiKeyState,0,sizeof(MidiKeyState));

	UpdateGuiFlag=true;

	suspend();
}



ChipWave::~ChipWave()
{
}



float ChipWave::FloatToMultiple(float value)
{
	const float mul[]={1.0f/128,1.0f/64,1.0f/32,1.0f/16,1.0f/8,1.0f/4,1.0f/2,1,2,4,8,16,32,64,128};

	return mul[(int)(value*14.99f)];
}



float ChipWave::OverdriveValue(float value)
{
	if(value<.5f) return value*2.0f; else return 1.0f+(value-.5f)*2.0f*(OVERDRIVE_MAX-1.0f);
}



float* ChipWave::getVarPtr(VstInt32 index)
{
	float *v=NULL;

	switch(index)
	{
	case pIdOscAWave:		v=pOscAWave; break;
	case pIdOscADuty:		v=pOscADuty; break;
	case pIdOscAOver:		v=pOscAOver; break;
	case pIdOscACut:		v=pOscACut; break;
	case pIdOscAMultiple:	v=pOscAMultiple; break;
	case pIdOscASeed:		v=pOscASeed; break;

	case pIdOscBWave:		v=pOscBWave; break;
	case pIdOscBDuty:		v=pOscBDuty; break;
	case pIdOscBOver:		v=pOscBOver; break;
	case pIdOscBCut:		v=pOscBCut; break;
	case pIdOscBDetune:		v=pOscBDetune; break;
	case pIdOscBMultiple:	v=pOscBMultiple; break;
	case pIdOscBSeed:		v=pOscBSeed; break;

	case pIdOscBalance:		v=pOscBalance; break;
	case pIdOscMixMode:		v=pOscMixMode; break;

	case pIdFltCutoff:		v=pFltCutoff; break;
	case pIdFltReso:		v=pFltReso; break;

	case pIdSlideDelay:		v=pSlideDelay; break;
	case pIdSlideSpeed:		v=pSlideSpeed; break;
	case pIdSlideRoute:		v=pSlideRoute; break;

	case pIdEnvAttack:		v=pEnvAttack; break;
	case pIdEnvDecay:		v=pEnvDecay; break;
	case pIdEnvSustain:		v=pEnvSustain; break;
	case pIdEnvRelease:		v=pEnvRelease; break;
	case pIdEnvOscADepth:	v=pEnvOscADepth; break;
	case pIdEnvOscBDepth:	v=pEnvOscBDepth; break;
	case pIdEnvOscBDetuneDepth:	v=pEnvOscBDetuneDepth; break;
	case pIdEnvOscMixDepth:	v=pEnvOscMixDepth; break;
	case pIdEnvFltDepth:	v=pEnvFltDepth; break;
	case pIdEnvLfoDepth:	v=pEnvLfoDepth; break;

	case pIdLfoSpeed:		v=pLfoSpeed; break;
	case pIdLfoPitchDepth:	v=pLfoPitchDepth; break;
	case pIdLfoOscADepth:	v=pLfoOscADepth; break;
	case pIdLfoOscBDepth:	v=pLfoOscBDepth; break;
	case pIdLfoOscMixDepth:	v=pLfoOscMixDepth; break;
	case pIdLfoFltDepth:	v=pLfoFltDepth; break;

	case pIdAmpAttack:		v=pAmpAttack; break;
	case pIdAmpDecay:		v=pAmpDecay; break;
	case pIdAmpSustain:		v=pAmpSustain; break;
	case pIdAmpRelease:		v=pAmpRelease; break;

	case pIdVelAmp:			v=pVelAmp; break;
	case pIdVelOscADepth:	v=pVelOscADepth; break;
	case pIdVelOscBDepth:	v=pVelOscBDepth; break;
	case pIdVelOscMixDepth:	v=pVelOscMixDepth; break;
	case pIdVelFltCutoff:	v=pVelFltCutoff; break;
	case pIdVelFltReso:		v=pVelFltReso; break;

	case pIdPolyphony:		v=pPolyphony; break;
	case pIdPortaSpeed:		v=pPortaSpeed; break;
	case pIdOutputGain:		v=pOutputGain; break;
	}

	return v;
}



void ChipWave::setParameter(VstInt32 index,float value)
{
	float *v;

	v=getVarPtr(index);

	if(v) v[Program]=value;
}



float ChipWave::getParameter(VstInt32 index)
{
	float *v;

	v=getVarPtr(index);

	if(v) return v[Program]; else return 0;
} 



void ChipWave::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdOscAWave:		strcpy(label,"OscA Waveform"); break;
	case pIdOscADuty:		strcpy(label,"OscA Skew/Duty"); break;
	case pIdOscAOver:		strcpy(label,"OscA Overdrive"); break;
	case pIdOscACut:		strcpy(label,"OscA Cut"); break;
	case pIdOscAMultiple:	strcpy(label,"OscA Div/Mul"); break;
	case pIdOscASeed:		strcpy(label,"OscA Noise Seed"); break;

	case pIdOscBWave:		strcpy(label,"OscB Waveform"); break;
	case pIdOscBDuty:		strcpy(label,"OscB Skew/Duty"); break;
	case pIdOscBOver:		strcpy(label,"OscB Overdrive"); break;
	case pIdOscBCut:		strcpy(label,"OscB Cut"); break;
	case pIdOscBDetune:		strcpy(label,"OscB Detune"); break;
	case pIdOscBMultiple:	strcpy(label,"OscB Div/Mul"); break;
	case pIdOscBSeed:		strcpy(label,"OscB Noise Seed"); break;

	case pIdOscBalance:		strcpy(label,"Osc Balance"); break;
	case pIdOscMixMode:		strcpy(label,"Osc Mix Mode"); break;

	case pIdFltCutoff:		strcpy(label,"Filter Cutoff"); break;
	case pIdFltReso:		strcpy(label,"Filter Resonance"); break;

	case pIdSlideDelay:		strcpy(label,"Slide Delay"); break;
	case pIdSlideSpeed:		strcpy(label,"Slide Speed"); break;
	case pIdSlideRoute:		strcpy(label,"Slide Route"); break;

	case pIdEnvAttack:		strcpy(label,"Envelope Attack"); break;
	case pIdEnvDecay:		strcpy(label,"Envelope Decay"); break;
	case pIdEnvSustain:		strcpy(label,"Envelope Sustain"); break;
	case pIdEnvRelease:		strcpy(label,"Envelope Release"); break;
	case pIdEnvOscADepth:	strcpy(label,"Env OscA Skew Depth"); break;
	case pIdEnvOscBDepth:	strcpy(label,"Env OscB Skew Depth"); break;
	case pIdEnvOscBDetuneDepth:	strcpy(label,"Env OscB Detune Depth"); break;
	case pIdEnvOscMixDepth:	strcpy(label,"Env Osc Balance Depth"); break;
	case pIdEnvFltDepth:	strcpy(label,"Env Filter Depth"); break;
	case pIdEnvLfoDepth:	strcpy(label,"Env LFO Depth"); break;

	case pIdLfoSpeed:		strcpy(label,"LFO Frequency"); break;
	case pIdLfoPitchDepth:	strcpy(label,"LFO Pitch Depth"); break;
	case pIdLfoOscADepth:	strcpy(label,"LFO OscA Skew Depth"); break;
	case pIdLfoOscBDepth:	strcpy(label,"LFO OscB Skew Depth"); break;
	case pIdLfoOscMixDepth:	strcpy(label,"LFO Osc Balance Depth"); break;
	case pIdLfoFltDepth:	strcpy(label,"LFO Filter Depth"); break;

	case pIdAmpAttack:		strcpy(label,"Amp Attack"); break;
	case pIdAmpDecay:		strcpy(label,"Amp Decay"); break;
	case pIdAmpSustain:		strcpy(label,"Amp Sustain"); break;
	case pIdAmpRelease:		strcpy(label,"Amp Release"); break;

	case pIdVelAmp:			strcpy(label,"Vel to Amp"); break;
	case pIdVelOscADepth:	strcpy(label,"Vel to OscA Skew Depth"); break;
	case pIdVelOscBDepth:	strcpy(label,"Vel to OscB Skew Depth"); break;
	case pIdVelOscMixDepth:	strcpy(label,"Vel to Osc Balance Depth"); break;
	case pIdVelFltCutoff:	strcpy(label,"Vel to Filter Cutoff"); break;
	case pIdVelFltReso:		strcpy(label,"Vel to Filter Resonance"); break;

	case pIdPolyphony:		strcpy(label,"Polyphony"); break;
	case pIdPortaSpeed:		strcpy(label,"Porta Speed"); break;
	case pIdOutputGain:		strcpy(label,"Output Gain"); break;

	default:				strcpy(label,"");
	}
} 



void ChipWave::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdOscAWave:		strcpy(text,oscWaveformNames[(int)(pOscAWave[Program]*3.99f)]); break;
	case pIdOscADuty:		float2string(pOscADuty[Program]*100.0f,text,5); break;
	case pIdOscAOver:	float2string(OverdriveValue(pOscAOver[Program]),text,5); break;
	case pIdOscACut:		if(pOscACut[Program]>0) float2string(OSC_CUT_MAX_MS*pOscACut[Program],text,5); else strcpy(text,"Inf"); break;
	case pIdOscAMultiple:	float2string(FloatToMultiple(pOscAMultiple[Program]),text,5); break;
	case pIdOscASeed:		if(pOscASeed[Program]==0) strcpy(text,"Random"); else float2string(pOscASeed[Program],text,5); break;

	case pIdOscBWave:		strcpy(text,oscWaveformNames[(int)(pOscBWave[Program]*3.99f)]); break;
	case pIdOscBDuty:		float2string(pOscBDuty[Program]*100.0f,text,5); break;
	case pIdOscBOver:	float2string(OverdriveValue(pOscBOver[Program]),text,5); break;
	case pIdOscBCut:		if(pOscBCut[Program]>0) float2string(OSC_CUT_MAX_MS*pOscBCut[Program],text,5); else strcpy(text,"Inf"); break;
	case pIdOscBDetune:		float2string(pOscBDetune[Program]*DETUNE_SEMITONES,text,5); break;
	case pIdOscBMultiple:	float2string(FloatToMultiple(pOscBMultiple[Program]),text,5); break;
	case pIdOscBSeed:		if(pOscBSeed[Program]==0) strcpy(text,"Random"); else float2string(pOscBSeed[Program],text,5); break;

	case pIdOscBalance:		float2string(pOscBalance[Program]*2.0f-1.0f,text,5); break;
	case pIdOscMixMode:		strcpy(text,pOscMixMode[Program]<.5f?"Add":"Mul"); break;

	case pIdFltCutoff:		if(pFltCutoff[Program]<1.0f) float2string(pFltCutoff[Program]*FILTER_CUTOFF_MAX_HZ,text,5); else strcpy(text,"Disabled"); break;
	case pIdFltReso:		float2string(pFltReso[Program],text,5); break;

	case pIdSlideDelay:		if(pSlideDelay[Program]<1.0f) float2string(MOD_DELAY_MAX_MS*pSlideDelay[Program],text,6); else strcpy(text,"Key Off"); break;
	case pIdSlideSpeed:		float2string(pSlideSpeed[Program]*2.0f-1.0f,text,5); break;
	case pIdSlideRoute:		strcpy(text,slideRouteNames[(int)(pSlideRoute[Program]*2.99f)]); break;

	case pIdEnvAttack:		float2string(ENVELOPE_ATTACK_MAX_MS*pEnvAttack[Program],text,6); break;
	case pIdEnvDecay:		float2string(ENVELOPE_DECAY_MAX_MS*pEnvDecay[Program],text,6); break;
	case pIdEnvSustain:		float2string(pEnvSustain[Program],text,5); break;
	case pIdEnvRelease:		float2string(ENVELOPE_RELEASE_MAX_MS*pEnvRelease[Program],text,5); break;

	case pIdEnvOscADepth:	float2string(pEnvOscADepth[Program]*2.0f-1.0f,text,5); break;
	case pIdEnvOscBDepth:	float2string(pEnvOscBDepth[Program]*2.0f-1.0f,text,5); break;
	case pIdEnvOscBDetuneDepth:	float2string(pEnvOscBDetuneDepth[Program],text,5); break;
	case pIdEnvOscMixDepth:	float2string(pEnvOscMixDepth[Program]*2.0f-1.0f,text,5); break;
	case pIdEnvFltDepth:	float2string(pEnvFltDepth[Program]*2.0f-1.0f,text,5); break;
	case pIdEnvLfoDepth:	float2string(pEnvLfoDepth[Program],text,5); break;

	case pIdAmpAttack:		float2string(ENVELOPE_ATTACK_MAX_MS*pAmpAttack[Program],text,6); break;
	case pIdAmpDecay:		float2string(ENVELOPE_DECAY_MAX_MS*pAmpDecay[Program],text,6); break;
	case pIdAmpSustain:		float2string(pAmpSustain[Program],text,5); break;
	case pIdAmpRelease:		float2string(ENVELOPE_RELEASE_MAX_MS*pAmpRelease[Program],text,5); break;

	case pIdLfoSpeed:		float2string(pLfoSpeed[Program]*LFO_MAX_HZ,text,5); break;
	case pIdLfoPitchDepth:	float2string(pLfoPitchDepth[Program],text,5); break;
	case pIdLfoOscADepth:	float2string(pLfoOscADepth[Program]*2.0f-1.0f,text,5); break;
	case pIdLfoOscBDepth:	float2string(pLfoOscBDepth[Program]*2.0f-1.0f,text,5); break;
	case pIdLfoOscMixDepth:	float2string(pLfoOscMixDepth[Program]*2.0f-1.0f,text,5); break;
	case pIdLfoFltDepth:	float2string(pLfoFltDepth[Program]*2.0f-1.0f,text,5); break;

	case pIdVelAmp:			float2string(pVelAmp[Program],text,5); break;
	case pIdVelOscADepth:	float2string(pVelOscADepth[Program]*2.0f-1.0f,text,5); break;
	case pIdVelOscBDepth:	float2string(pVelOscBDepth[Program]*2.0f-1.0f,text,5); break;
	case pIdVelOscMixDepth:	float2string(pVelOscMixDepth[Program]*2.0f-1.0f,text,5); break;
	case pIdVelFltCutoff:	float2string(pVelFltCutoff[Program]*2.0f-1.0f,text,5); break;
	case pIdVelFltReso:		float2string(pVelFltReso[Program]*2.0f-1.0f,text,5); break;


	case pIdPolyphony:		strcpy(text,pPolyphony[Program]<.5f?"Mono":"Poly"); break;
	case pIdPortaSpeed:		float2string(pPortaSpeed[Program],text,5); break;
	case pIdOutputGain:		dB2string(pOutputGain[Program],text,3); break;

	default:				strcpy(text,"");
	}
} 



void ChipWave::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdOscADuty:
	case pIdOscBDuty:		strcpy(label,"%"); break;
	case pIdOscBDetune:		strcpy(label,"semi"); break;
	case pIdOscAMultiple:
	case pIdOscBMultiple:	strcpy(label,"x"); break;
	case pIdOscACut:		strcpy(label,pOscACut[Program]>0?"ms":""); break;
	case pIdOscBCut:		strcpy(label,pOscBCut[Program]>0?"ms":""); break;
	case pIdAmpAttack:
	case pIdAmpDecay:
	case pIdAmpRelease:
	case pIdEnvAttack:
	case pIdEnvDecay:
	case pIdEnvRelease:		strcpy(label,"ms"); break;
	case pIdSlideDelay:		strcpy(label,pSlideDelay[Program]<1.0f?"ms":""); break;
	case pIdLfoSpeed:		strcpy(label,"Hz"); break;
	case pIdFltCutoff:		strcpy(label,pFltCutoff[Program]<1.0f?"Hz":""); break;
	case pIdOutputGain:		strcpy(label,"dB"); break;
	default:				strcpy(label,"");
	}
} 



VstInt32 ChipWave::getProgram(void)
{
	return Program;
}



void ChipWave::setProgram(VstInt32 program)
{
	if(Program!=program)
	{
		Program=program;
	}
}



void ChipWave::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void ChipWave::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 ChipWave::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 ChipWave::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 ChipWave::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 ChipWave::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 ChipWave::LoadStringChunk(char *str,int max_length,float *src)
{
	unsigned int c,ptr;

	ptr=0;

	while(1)
	{
		c=(unsigned int)*src++;
		
		str[ptr++]=c;

		if(!c) break;

		if(ptr==max_length-1)
		{
			str[ptr]=0;
			break;
		}
	}

	return ptr;
}



VstInt32 ChipWave::SavePresetChunk(float *chunk)
{
	int ptr,pgm;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		chunk[ptr++]=OAWF; chunk[ptr++]=pOscAWave	 [pgm];
		chunk[ptr++]=OADU; chunk[ptr++]=pOscADuty    [pgm];
		chunk[ptr++]=OAOV; chunk[ptr++]=pOscAOver    [pgm];
		chunk[ptr++]=OAMU; chunk[ptr++]=pOscAMultiple[pgm];
		chunk[ptr++]=OASD; chunk[ptr++]=pOscASeed	 [pgm];
		chunk[ptr++]=OACT; chunk[ptr++]=pOscACut	 [pgm];

		chunk[ptr++]=OBWF; chunk[ptr++]=pOscBWave    [pgm];
		chunk[ptr++]=OBDU; chunk[ptr++]=pOscBDuty    [pgm];
		chunk[ptr++]=OBOV; chunk[ptr++]=pOscBOver    [pgm];
		chunk[ptr++]=OBDE; chunk[ptr++]=pOscBDetune  [pgm];
		chunk[ptr++]=OBMU; chunk[ptr++]=pOscBMultiple[pgm];
		chunk[ptr++]=OBSD; chunk[ptr++]=pOscBSeed	 [pgm];
		chunk[ptr++]=OBCT; chunk[ptr++]=pOscBCut	 [pgm];

		chunk[ptr++]=OBAL; chunk[ptr++]=pOscBalance  [pgm];
		chunk[ptr++]=OMIX; chunk[ptr++]=pOscMixMode  [pgm];

		chunk[ptr++]=FLCU; chunk[ptr++]=pFltCutoff[pgm];
		chunk[ptr++]=FLRE; chunk[ptr++]=pFltReso  [pgm];

		chunk[ptr++]=SLDE; chunk[ptr++]=pSlideDelay[pgm];
		chunk[ptr++]=SLSP; chunk[ptr++]=pSlideSpeed[pgm];
		chunk[ptr++]=SLRO; chunk[ptr++]=pSlideRoute[pgm];
		
		chunk[ptr++]=ENAT; chunk[ptr++]=pEnvAttack [pgm];
		chunk[ptr++]=ENDC; chunk[ptr++]=pEnvDecay  [pgm];
		chunk[ptr++]=ENSU; chunk[ptr++]=pEnvSustain[pgm];
		chunk[ptr++]=ENRE; chunk[ptr++]=pEnvRelease[pgm];

		chunk[ptr++]=ENOA; chunk[ptr++]=pEnvOscADepth  [pgm];
		chunk[ptr++]=ENOB; chunk[ptr++]=pEnvOscBDepth  [pgm];
		chunk[ptr++]=ENOD; chunk[ptr++]=pEnvOscBDetuneDepth[pgm];
		chunk[ptr++]=ENMX; chunk[ptr++]=pEnvOscMixDepth[pgm];
		chunk[ptr++]=ENFL; chunk[ptr++]=pEnvFltDepth   [pgm];
		chunk[ptr++]=ENLF; chunk[ptr++]=pEnvLfoDepth   [pgm];

		chunk[ptr++]=LFSP; chunk[ptr++]=pLfoSpeed      [pgm];
		chunk[ptr++]=LFPI; chunk[ptr++]=pLfoPitchDepth [pgm];
		chunk[ptr++]=LFOA; chunk[ptr++]=pLfoOscADepth  [pgm];
		chunk[ptr++]=LFOB; chunk[ptr++]=pLfoOscBDepth  [pgm];
		chunk[ptr++]=LFMX; chunk[ptr++]=pLfoOscMixDepth[pgm];
		chunk[ptr++]=LFFL; chunk[ptr++]=pLfoFltDepth   [pgm];

		chunk[ptr++]=VOAT; chunk[ptr++]=pAmpAttack [pgm];
		chunk[ptr++]=VODE; chunk[ptr++]=pAmpDecay  [pgm];
		chunk[ptr++]=VOSU; chunk[ptr++]=pAmpSustain[pgm];
		chunk[ptr++]=VORE; chunk[ptr++]=pAmpRelease[pgm];

		chunk[ptr++]=VLAM; chunk[ptr++]=pVelAmp        [pgm];
		chunk[ptr++]=VLOA; chunk[ptr++]=pVelOscADepth  [pgm];
		chunk[ptr++]=VLOB; chunk[ptr++]=pVelOscBDepth  [pgm];
		chunk[ptr++]=VLMX; chunk[ptr++]=pVelOscMixDepth[pgm];
		chunk[ptr++]=VLFC; chunk[ptr++]=pVelFltCutoff  [pgm];
		chunk[ptr++]=VLFR; chunk[ptr++]=pVelFltReso    [pgm];

		chunk[ptr++]=POLY; chunk[ptr++]=pPolyphony [pgm];
		chunk[ptr++]=POSP; chunk[ptr++]=pPortaSpeed[pgm];
		chunk[ptr++]=GAIN; chunk[ptr++]=pOutputGain[pgm];
	}

	chunk[ptr++]=DONE;

	return ptr*sizeof(float);//size in bytes
}



void ChipWave::LoadPresetChunk(float *chunk)
{
	int pgm;
	float tag;

	while(1)
	{
		tag=*chunk++;

		if(tag==DONE) break;

		if(tag==PROG) pgm =(int)*chunk++;

		if(tag==NAME) chunk+=LoadStringChunk(ProgramName[pgm],MAX_NAME_LEN,chunk);

		if(tag==OAWF) pOscAWave	   [pgm]=*chunk++;
		if(tag==OADU) pOscADuty    [pgm]=*chunk++;
		if(tag==OAOV) pOscAOver    [pgm]=*chunk++;
		if(tag==OAMU) pOscAMultiple[pgm]=*chunk++;
		if(tag==OASD) pOscASeed    [pgm]=*chunk++;
		if(tag==OACT) pOscACut     [pgm]=*chunk++;

		if(tag==OBWF) pOscBWave    [pgm]=*chunk++;
		if(tag==OBDU) pOscBDuty    [pgm]=*chunk++;
		if(tag==OBOV) pOscBOver    [pgm]=*chunk++;
		if(tag==OBDE) pOscBDetune  [pgm]=*chunk++;
		if(tag==OBMU) pOscBMultiple[pgm]=*chunk++;
		if(tag==OBSD) pOscBSeed    [pgm]=*chunk++;
		if(tag==OBCT) pOscBCut     [pgm]=*chunk++;

		if(tag==OBAL) pOscBalance  [pgm]=*chunk++;
		if(tag==OMIX) pOscMixMode  [pgm]=*chunk++;

		if(tag==FLCU) pFltCutoff[pgm]=*chunk++;
		if(tag==FLRE) pFltReso  [pgm]=*chunk++;

		if(tag==SLDE) pSlideDelay[pgm]=*chunk++;
		if(tag==SLSP) pSlideSpeed[pgm]=*chunk++;
		if(tag==SLRO) pSlideRoute[pgm]=*chunk++;
		
		if(tag==ENAT) pEnvAttack [pgm]=*chunk++;
		if(tag==ENDC) pEnvDecay  [pgm]=*chunk++;
		if(tag==ENSU) pEnvSustain[pgm]=*chunk++;
		if(tag==ENRE) pEnvRelease[pgm]=*chunk++;

		if(tag==ENOA) pEnvOscADepth  [pgm]=*chunk++;
		if(tag==ENOB) pEnvOscBDepth  [pgm]=*chunk++;
		if(tag==ENOD) pEnvOscBDetuneDepth  [pgm]=*chunk++;
		if(tag==ENMX) pEnvOscMixDepth[pgm]=*chunk++;
		if(tag==ENFL) pEnvFltDepth   [pgm]=*chunk++;
		if(tag==ENLF) pEnvLfoDepth   [pgm]=*chunk++;

		if(tag==LFSP) pLfoSpeed	     [pgm]=*chunk++;
		if(tag==LFOA) pLfoOscADepth  [pgm]=*chunk++;
		if(tag==LFOB) pLfoOscBDepth  [pgm]=*chunk++;
		if(tag==LFMX) pLfoOscMixDepth[pgm]=*chunk++;
		if(tag==LFPI) pLfoPitchDepth [pgm]=*chunk++;
		if(tag==LFFL) pLfoFltDepth   [pgm]=*chunk++;

		if(tag==VOAT) pAmpAttack [pgm]=*chunk++;
		if(tag==VODE) pAmpDecay  [pgm]=*chunk++;
		if(tag==VOSU) pAmpSustain[pgm]=*chunk++;
		if(tag==VORE) pAmpRelease[pgm]=*chunk++;

		if(tag==VLAM) pVelAmp        [pgm]=*chunk++;
		if(tag==VLOA) pVelOscADepth  [pgm]=*chunk++;
		if(tag==VLOB) pVelOscBDepth  [pgm]=*chunk++;
		if(tag==VLMX) pVelOscMixDepth[pgm]=*chunk++;
		if(tag==VLFC) pVelFltCutoff  [pgm]=*chunk++;
		if(tag==VLFR) pVelFltReso    [pgm]=*chunk++;

		if(tag==POLY) pPolyphony [pgm]=*chunk++;
		if(tag==POSP) pPortaSpeed[pgm]=*chunk++;
		if(tag==GAIN) pOutputGain[pgm]=*chunk++;
	}
}



VstInt32 ChipWave::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 ChipWave::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	UpdateGuiFlag=true;

	return 0;
}



void ChipWave::UpdateGUI(bool display)
{
#ifdef _GUI_ACTIVE_
	if(editor) ((GUI*)editor)->Update();
#endif

	if(display) updateDisplay();
}



void ChipWave::MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void ChipWave::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



void ChipWave::MidiAddPitchBend(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypePitchBend;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



void ChipWave::MidiAddModulation(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypeModulation;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



bool ChipWave::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



VstInt32 ChipWave::processEvents(VstEvents* ev) 
{ 
	VstMidiEvent* event;
	VstInt32 i,status,note,velocity,wheel;
	char* midiData;

	for(i=0;i<ev->numEvents;++i) 
	{ 
		event=(VstMidiEvent*)ev->events[i];

		if(event->type!=kVstMidiType) continue; 

		midiData=event->midiData;

		status=midiData[0]&0xf0;

		switch(status)
		{
		case 0x80://note off
		case 0x90://note on
			{
				note=midiData[1]&0x7f; 

				velocity=midiData[2]&0x7f; 

				if(status==0x80) velocity=0; 

				MidiAddNote(event->deltaFrames,note,velocity);
			} 
			break;

		case 0xb0://control change
			{ 
				if(midiData[1]==0x64) MidiRPNLSB =midiData[2]&0x7f;
				if(midiData[1]==0x65) MidiRPNMSB =midiData[2]&0x7f;
				if(midiData[1]==0x26) MidiDataLSB=midiData[2]&0x7f;

				if(midiData[1]==0x01)
				{
					MidiModulationMSB=midiData[2]&0x7f;

					MidiAddModulation(event->deltaFrames,(float)MidiModulationMSB/128.0f);
				}

				if(midiData[1]==0x06)
				{
					MidiDataMSB=midiData[2]&0x7f;

					if(MidiRPNLSB==0&&MidiRPNMSB==0) MidiPitchBendRange=(float)MidiDataMSB*.5f;
				}

				if(midiData[1]>=0x7b)//all notes off and mono/poly mode changes that also requires to do all notes off
				{
					MidiAddNote(event->deltaFrames,-1,0);
				}
			} 
			break;

		case 0xc0:	//program change
			{
				MidiAddProgramChange(event->deltaFrames,midiData[1]&0x7f);
			}
			break;

		case 0xe0://pitch bend change
			{
				wheel=(midiData[1]&0x7f)|((midiData[2]&0x7f)<<7);

				MidiAddPitchBend(event->deltaFrames,(float)(wheel-0x2000)*MidiPitchBendRange/8192.0f);
			}
			break;
		}
	} 

	return 1; 
} 



VstInt32 ChipWave::SynthAllocateVoice(VstInt32 note)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==note) return chn;
	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note<0) return chn;
	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].ev_stage==eStageRelease) return chn;

	return -1;
}



void ChipWave::SynthChannelChangeNote(VstInt32 chn,VstInt32 note)
{
	SynthChannel[chn].note=note;

	if(note>=0)
	{
		SynthChannel[chn].freq_new=(float)SynthChannel[chn].note-69;

		sSlideStep=(SynthChannel[chn].freq-SynthChannel[chn].freq_new)*20.0f*log(1.0f-pPortaSpeed[Program]);
	}
}



float ChipWave::SynthEnvelopeTimeToDelta(float value,float max_ms)
{
	if(value<.00005f) value=.00005f;

	return (1.0f/ENVELOPE_UPDATE_RATE_HZ)/(value*max_ms/1000.0f);
}



void ChipWave::SynthRestartEnvelope(VstInt32 chn)
{
	if(pAmpAttack[Program]>0||pAmpDecay[Program]>0)
	{
		SynthChannel[chn].ev_stage=eStageAttack;
		SynthChannel[chn].ev_level=0;
		SynthChannel[chn].ev_delta=SynthEnvelopeTimeToDelta(pAmpAttack[Program],ENVELOPE_ATTACK_MAX_MS);
	}
	else
	{
		SynthChannel[chn].ev_stage=eStageSustain;
		SynthChannel[chn].ev_level=pAmpSustain[Program];
	}

	if(pEnvAttack[Program]>0||pEnvDecay[Program]>0)
	{
		SynthChannel[chn].ef_stage=eStageAttack;
		SynthChannel[chn].ef_level=0;
		SynthChannel[chn].ef_delta=SynthEnvelopeTimeToDelta(pEnvAttack[Program],ENVELOPE_ATTACK_MAX_MS);
	}
	else
	{
		SynthChannel[chn].ef_stage=eStageSustain;
		SynthChannel[chn].ef_level=pEnvSustain[Program];
	}
}



void ChipWave::SynthStopEnvelope(VstInt32 chn)
{
	SynthChannel[chn].ev_stage=eStageRelease;
	SynthChannel[chn].ef_stage=eStageRelease;
	
	SynthChannel[chn].ev_delta=SynthEnvelopeTimeToDelta(pAmpRelease[Program],ENVELOPE_RELEASE_MAX_MS)*SynthChannel[chn].ev_level;
	SynthChannel[chn].ef_delta=SynthEnvelopeTimeToDelta(pEnvRelease[Program],ENVELOPE_RELEASE_MAX_MS)*SynthChannel[chn].ef_level;

	if(pSlideDelay[Program]>=1.0f) SynthChannel[chn].slide_delay=0;
}



void ChipWave::SynthAdvanceEnvelopes(void)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		//volume

		if(SynthChannel[chn].ev_stage==eStageReset)
		{
			SynthChannel[chn].note=-1;
			SynthChannel[chn].ev_level=0;
		}
		else
		{
			switch(SynthChannel[chn].ev_stage)
			{
			case eStageAttack:			
				{
					SynthChannel[chn].ev_level+=SynthChannel[chn].ev_delta;

					if(SynthChannel[chn].ev_level>=1.0f)
					{
						SynthChannel[chn].ev_level=1.0f;
						SynthChannel[chn].ev_delta=SynthEnvelopeTimeToDelta(pAmpDecay[Program],ENVELOPE_DECAY_MAX_MS);
						SynthChannel[chn].ev_stage=eStageDecay;
					}
				}
				break;

			case eStageDecay:
				{
					SynthChannel[chn].ev_level-=SynthChannel[chn].ev_delta;

					if(SynthChannel[chn].ev_level<=pAmpSustain[Program])
					{
						SynthChannel[chn].ev_level=pAmpSustain[Program];
						SynthChannel[chn].ev_stage=eStageSustain;
					}
				}
				break;

			case eStageRelease:
				{
					SynthChannel[chn].ev_level-=SynthChannel[chn].ev_delta;

					if(SynthChannel[chn].ev_level<=0)
					{
						SynthChannel[chn].ev_level=0;
						SynthChannel[chn].ev_stage=eStageReset;
					}
				}
				break;
			}
		}

		//filter

		if(SynthChannel[chn].ef_stage==eStageReset)
		{
			SynthChannel[chn].ef_level=0;
		}
		else
		{
			switch(SynthChannel[chn].ef_stage)
			{
			case eStageAttack:			
				{
					SynthChannel[chn].ef_level+=SynthChannel[chn].ef_delta;

					if(SynthChannel[chn].ef_level>=1.0f)
					{
						SynthChannel[chn].ef_level=1.0f;
						SynthChannel[chn].ef_delta=SynthEnvelopeTimeToDelta(pEnvDecay[Program],ENVELOPE_DECAY_MAX_MS);
						SynthChannel[chn].ef_stage=eStageDecay;
					}
				}
				break;

			case eStageDecay:
				{
					SynthChannel[chn].ef_level-=SynthChannel[chn].ef_delta;

					if(SynthChannel[chn].ef_level<=pEnvSustain[Program])
					{
						SynthChannel[chn].ef_level=pEnvSustain[Program];
						SynthChannel[chn].ef_stage=eStageSustain;
					}
				}
				break;

			case eStageRelease:
				{
					SynthChannel[chn].ef_level-=SynthChannel[chn].ef_delta;

					if(SynthChannel[chn].ef_level<=0)
					{
						SynthChannel[chn].ef_level=0;
						SynthChannel[chn].ef_stage=eStageReset;
					}
				}
				break;
			}
		}
	}
}



inline float ChipWave::SynthGetSample(SynthOscStruct *osc,float over,float duty,int wave)
{
	const VstInt32 noise_mask[]={7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};
	VstInt32 mask;
	float out;

	switch(wave)
	{
	case 0://pulse
		{
			out=(osc->acc<duty)?0.0f:.5f;

			break;
		}

	case 1://tri/saw
		{
			out=((osc->acc<duty)?(osc->acc/duty):((1.0f-osc->acc)/(1.0f-duty)))-.5f;

			break;
		}

	case 2://sine
		{
			out=(osc->acc<duty)?(.5f*sinf(F_PI*(osc->acc/duty))):(-.5f*sin(F_PI*((1.0f-osc->acc)/(1.0f-duty))));

			break;
		}

	case 3://noise
		{
			mask=(int)(duty*13.99f);

			out=Noise[((osc->noise&noise_mask[mask])+osc->noise_seed)&65535]?.5f:0;

			break;
		}

	default:
		
		out=0;
	}

	out*=over;

	if(out> .5f) out= .5f;
	if(out<-.5f) out=-.5f;

	return out;
}



void ChipWave::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float level_mix,level_osc;
	float modulation,vibrato,mod_step,depth,sampleRate;
	float osca,oscb,dutya,dutyb,osc_balance,detune,overa,overb;
	double q,w;
	unsigned int i,s;
	int chn,note,prev_note;
	int oscaw,oscbw,mix_mode,slide,osc_mute;
	bool poly,key_off;

	sampleRate=(float)updateSampleRate();

	mod_step=12.0f/sampleRate*F_PI;

	poly=(pPolyphony[Program]<.5f?false:true);

	oscaw=(int)(pOscAWave[Program]*3.99f);
	oscbw=(int)(pOscBWave[Program]*3.99f);

	overa=OverdriveValue(pOscAOver[Program]);
	overb=OverdriveValue(pOscBOver[Program]);

	mix_mode=(int)(pOscMixMode[Program]<.5f?0:1);

	slide=(int)(pSlideRoute[Program]*2.99f);

	while(--sampleFrames>=0)
	{
		if(MidiModulationDepth>=.01f) modulation=sinf(MidiModulationCount)*MidiModulationDepth; else modulation=0;

		MidiModulationCount+=mod_step;

		key_off=false;

		for(i=0;i<MidiQueue.size();++i)
		{
			if(MidiQueue[i].delta>=0)
			{
				--MidiQueue[i].delta;

				if(MidiQueue[i].delta<0)//new message
				{
					switch(MidiQueue[i].type)
					{
					case mEventTypeNote:
						{
							if(MidiQueue[i].velocity)//key on
							{
								MidiKeyState[MidiQueue[i].note]=1;

								if(!poly) chn=0; else chn=SynthAllocateVoice(MidiQueue[i].note);

								if(chn>=0)
								{
									prev_note=SynthChannel[chn].note;

									SynthChannelChangeNote(chn,MidiQueue[i].note);

									if(prev_note<0||poly) SynthChannel[chn].freq=SynthChannel[chn].freq_new;

									if(poly||prev_note<0||(!poly&&SynthChannel[0].ev_stage==eStageRelease))
									{
										SynthChannel[chn].osca.acc=0;	//phase reset
										SynthChannel[chn].oscb.acc=0;

										SynthChannel[chn].osca.cut=sampleRate*(pOscACut[Program]*OSC_CUT_MAX_MS)/1000.0f;
										SynthChannel[chn].oscb.cut=sampleRate*(pOscBCut[Program]*OSC_CUT_MAX_MS)/1000.0f;

										if(pOscASeed[Program]>0)
										{
											SynthChannel[chn].osca.noise_seed=(VstInt32)(pOscASeed[Program]*65535.99f);
										}
										else
										{
											SynthChannel[chn].osca.noise_seed=rand();
										}

										if(pOscBSeed[Program]>0)
										{
											SynthChannel[chn].oscb.noise_seed=(VstInt32)(pOscBSeed[Program]*65535.99f);
										}
										else
										{
											SynthChannel[chn].oscb.noise_seed=rand();
										}

										SynthChannel[chn].velocity=((float)MidiQueue[i].velocity/100.0f);
										SynthChannel[chn].volume=1.0f*(1.0f-pVelAmp[Program])+SynthChannel[chn].velocity*pVelAmp[Program];

										SynthChannel[chn].slide_delay=sampleRate*(pSlideDelay[Program]*MOD_DELAY_MAX_MS)/1000.0f;
										SynthChannel[chn].slide_osca=0;
										SynthChannel[chn].slide_oscb=0;

										SynthChannel[chn].lfo_count=0;
										SynthChannel[chn].lfo_out=0;

										SynthChannel[chn].filter_vibrapos=0;
										SynthChannel[chn].filter_vibraspeed=0;

										SynthRestartEnvelope(chn);
									}
								}
							}
							else//key off
							{
								key_off=true;

								if(MidiQueue[i].note>=0)
								{
									MidiKeyState[MidiQueue[i].note]=0;

									if(poly)
									{
										for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==MidiQueue[i].note) SynthStopEnvelope(chn);
									}
									else
									{
										for(note=127;note>=0;--note)
										{
											if(MidiKeyState[note])
											{
												SynthChannelChangeNote(0,note);

												break;
											}
										}
									}
								}
								else
								{
									memset(MidiKeyState,0,sizeof(MidiKeyState));

									for(chn=0;chn<SYNTH_CHANNELS;++chn) SynthStopEnvelope(chn);
								}
							}
						}
						break;

					case mEventTypeProgram:
						{
							if(Program!=MidiQueue[i].program)
							{
								Program=MidiQueue[i].program;

								UpdateGuiFlag=true;
							}
						}
						break;

					case mEventTypePitchBend:
						{
							MidiPitchBend=MidiQueue[i].depth;
						}
						break;

					case mEventTypeModulation:
						{
							MidiModulationDepth=MidiQueue[i].depth/4.0f;
						}
						break;
					}
				}
			}
		}

		if(!poly&&key_off)
		{
			if(!MidiIsAnyKeyDown())
			{
				for(chn=0;chn<SYNTH_CHANNELS;++chn) SynthStopEnvelope(chn);
			}
		}

		sEnvelopeDiv+=ENVELOPE_UPDATE_RATE_HZ/sampleRate;

		if(sEnvelopeDiv>=1.0f)
		{
			sEnvelopeDiv-=1.0f;

			SynthAdvanceEnvelopes();
		}

		for(chn=0;chn<SYNTH_CHANNELS;++chn)
		{
			if(SynthChannel[chn].note<0) continue;

			//process lfo, per channel

			while(SynthChannel[chn].lfo_count>=F_PI) SynthChannel[chn].lfo_count-=F_PI;
				
			depth=1.0f*(1.0f-pEnvLfoDepth[Program])+SynthChannel[chn].ef_level*pEnvLfoDepth[Program];	//balance between just lfo and influenced by envelope

			SynthChannel[chn].lfo_out=sinf(SynthChannel[chn].lfo_count)*depth;

			SynthChannel[chn].lfo_count+=pLfoSpeed[Program]*LFO_MAX_HZ/sampleRate;

			//process slide

			if(SynthChannel[chn].slide_delay>0)
			{
				--SynthChannel[chn].slide_delay;
			}
			else
			{
				if(!slide||slide==1) SynthChannel[chn].slide_osca+=(pSlideSpeed[Program]*2.0f-1.0f)*.01f;
				if(!slide||slide==2) SynthChannel[chn].slide_oscb+=(pSlideSpeed[Program]*2.0f-1.0f)*.01f;
			}

			//process osc cut

			if(SynthChannel[chn].osca.cut>0) --SynthChannel[chn].osca.cut;
			if(SynthChannel[chn].oscb.cut>0) --SynthChannel[chn].oscb.cut;

			//process portamento

			if(pPortaSpeed[Program]>=1.0f)
			{
				SynthChannel[chn].freq=SynthChannel[chn].freq_new;
			}
			else
			{
				if(SynthChannel[chn].freq<SynthChannel[chn].freq_new)
				{
					SynthChannel[chn].freq+=sSlideStep/sampleRate;

					if(SynthChannel[chn].freq>SynthChannel[chn].freq_new) SynthChannel[chn].freq=SynthChannel[chn].freq_new;
				}

				if(SynthChannel[chn].freq>SynthChannel[chn].freq_new)
				{
					SynthChannel[chn].freq+=sSlideStep/sampleRate;

					if(SynthChannel[chn].freq<SynthChannel[chn].freq_new) SynthChannel[chn].freq=SynthChannel[chn].freq_new;
				}
			}

			//process vibrato

			vibrato=pLfoPitchDepth[Program]*SynthChannel[chn].lfo_out;

			//get adders out of frequencies

			detune=pOscBDetune[Program]+pEnvOscBDetuneDepth[Program]*.1f*SynthChannel[chn].ef_level;

			if(detune<0.0f) detune=0.0f;
			if(detune>1.0f) detune=1.0f;

			SynthChannel[chn].osca.add=440.0f*pow(2.0f,(SynthChannel[chn].freq+modulation+vibrato+SynthChannel[chn].slide_osca+MidiPitchBend)/12.0f)*FloatToMultiple(pOscAMultiple[Program])/sampleRate/OVERSAMPLING;
			SynthChannel[chn].oscb.add=440.0f*pow(2.0f,(SynthChannel[chn].freq+modulation+vibrato+SynthChannel[chn].slide_oscb+MidiPitchBend+detune*DETUNE_SEMITONES)/12.0f)*FloatToMultiple(pOscBMultiple[Program])/sampleRate/OVERSAMPLING;

			//process filter

			if(pFltCutoff[Program]<1.0f)	//calculate filter coefficients
			{
				SynthChannel[chn].filter_resofreq=pFltCutoff[Program]+(SynthChannel[chn].velocity*(pVelFltCutoff[Program]*2.0f-1.0f))+(SynthChannel[chn].ef_level*(pEnvFltDepth[Program]*2.0f-1.0f))+(SynthChannel[chn].lfo_out*(pLfoFltDepth[Program]*2.0f-1.0f));
				SynthChannel[chn].filter_resofreq*=FILTER_CUTOFF_MAX_HZ;

				if(SynthChannel[chn].filter_resofreq<FILTER_CUTOFF_MIN_HZ) SynthChannel[chn].filter_resofreq=FILTER_CUTOFF_MIN_HZ;
				if(SynthChannel[chn].filter_resofreq>FILTER_CUTOFF_MAX_HZ) SynthChannel[chn].filter_resofreq=FILTER_CUTOFF_MAX_HZ;
	
				SynthChannel[chn].filter_amp=FILTER_MIN_RESONANCE+(pFltReso[Program]+SynthChannel[chn].velocity*(pVelFltReso[Program]*2.0f-1.0f))*FILTER_MAX_RESONANCE;

				if(SynthChannel[chn].filter_amp<FILTER_MIN_RESONANCE) SynthChannel[chn].filter_amp=FILTER_MIN_RESONANCE;
				if(SynthChannel[chn].filter_amp>FILTER_MAX_RESONANCE) SynthChannel[chn].filter_amp=FILTER_MAX_RESONANCE;

				w=2.0*M_PI*SynthChannel[chn].filter_resofreq/sampleRate;		// Pole angle
				q=1.0-w/(2.0*(SynthChannel[chn].filter_amp+0.5/(1.0+w))+w-2.0); // Pole magnitude

				SynthChannel[chn].filter_r=q*q;
				SynthChannel[chn].filter_c=SynthChannel[chn].filter_r+1.0-2.0*cos(w)*q;
			}
		}

		level_mix=0;

		for(chn=0;chn<SYNTH_CHANNELS;++chn)
		{
			if(SynthChannel[chn].note<0) continue;

			dutya=pOscADuty[Program]+(pVelOscADepth[Program]*2.0f-1.0f)*SynthChannel[chn].velocity+(pEnvOscADepth[Program]*2.0f-1.0f)*SynthChannel[chn].ef_level+(pLfoOscADepth[Program]*2.0f-1.0f)*SynthChannel[chn].lfo_out;
			dutyb=pOscBDuty[Program]+(pVelOscBDepth[Program]*2.0f-1.0f)*SynthChannel[chn].velocity+(pEnvOscBDepth[Program]*2.0f-1.0f)*SynthChannel[chn].ef_level+(pLfoOscBDepth[Program]*2.0f-1.0f)*SynthChannel[chn].lfo_out;

			if(dutya<0.0f) dutya=0.0f;
			if(dutyb<0.0f) dutyb=0.0f;
			if(dutya>1.0f) dutya=1.0f;
			if(dutyb>1.0f) dutyb=1.0f;

			osc_balance=pOscBalance[Program]+(pVelOscMixDepth[Program]*2.0f-1.0f)*SynthChannel[chn].velocity+(pEnvOscMixDepth[Program]*2.0f-1.0f)*SynthChannel[chn].ef_level+(pLfoOscMixDepth[Program]*2.0f-1.0f)*SynthChannel[chn].lfo_out;

			if(osc_balance<0.0f) osc_balance=0.0f;
			if(osc_balance>1.0f) osc_balance=1.0f;

			level_osc=0;

			osc_mute=0;

			if(pOscACut[Program]>0) if(SynthChannel[chn].osca.cut<=0) osc_mute|=1;
			if(pOscBCut[Program]>0) if(SynthChannel[chn].oscb.cut<=0) osc_mute|=2;

			for(s=0;s<OVERSAMPLING;++s)
			{
				SynthChannel[chn].osca.acc+=SynthChannel[chn].osca.add;
				SynthChannel[chn].oscb.acc+=SynthChannel[chn].oscb.add;

				while(SynthChannel[chn].osca.acc>=1.0f) { SynthChannel[chn].osca.acc-=1.0f; ++SynthChannel[chn].osca.noise; }
				while(SynthChannel[chn].oscb.acc>=1.0f) { SynthChannel[chn].oscb.acc-=1.0f; ++SynthChannel[chn].oscb.noise; }

				if(!(osc_mute&1)) osca=SynthGetSample(&SynthChannel[chn].osca,overa,dutya,oscaw); else osca=0;
				if(!(osc_mute&2)) oscb=SynthGetSample(&SynthChannel[chn].oscb,overb,dutyb,oscbw); else oscb=0;

				switch(mix_mode)
				{
				case 0: level_osc+=osca*(1.0f-osc_balance)*.5f+oscb*osc_balance*.5f; break;
				case 1: level_osc+=(osca+(osc_balance*.5f))*(oscb-(osc_balance*.5f)); break;
				}
			}

			level_osc=level_osc*SynthChannel[chn].ev_level*SynthChannel[chn].volume/OVERSAMPLING/(SYNTH_CHANNELS/2);

			//12db filter

			if(pFltCutoff[Program]<1.0f)
			{
				SynthChannel[chn].filter_vibraspeed+=(level_osc*32768.0-SynthChannel[chn].filter_vibrapos)*SynthChannel[chn].filter_c;

				SynthChannel[chn].filter_vibrapos+=SynthChannel[chn].filter_vibraspeed;

				SynthChannel[chn].filter_vibraspeed*=SynthChannel[chn].filter_r;

				level_osc=(float)SynthChannel[chn].filter_vibrapos;

				if(level_osc<-32768.0f) level_osc=-32768.0f; else if(level_osc>=32768.0f) level_osc=32768.0f;

				level_osc/=32768.0;
			}
			
			level_mix+=level_osc;
		}

		level_mix=level_mix*pOutputGain[Program];

		(*outL++)=level_mix;
		(*outR++)=level_mix;
	}

	MidiQueue.clear();

	if(UpdateGuiFlag)
	{
		UpdateGUI(true);

		UpdateGuiFlag=false;
	}
}
