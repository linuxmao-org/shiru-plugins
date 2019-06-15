//code by Shiru, 2018, released under WTFPL

//uses resonant IIR low pass filter code by Olli Niemitalo from early 2000s
//http://www.musicdsp.org/showArchiveComment.php?ArchiveID=27



#include "ChipDrum.h"
#include "GUI.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new ChipDrum(audioMaster);
}



ChipDrum::ChipDrum(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 i,pgm,note,pn_ptr;

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

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		strcpy(ProgramName[pgm],"default");

		for(note=0;note<SYNTH_NOTES;++note)
		{
			pn_ptr=PN_PTR(pgm,note);

			pToneLevel  [pn_ptr]=1.0f;
			pToneDecay  [pn_ptr]=0.25f;
			pToneSustain[pn_ptr]=0.25f;
			pToneRelease[pn_ptr]=0.25f;
			pTonePitch  [pn_ptr]=0.2f;
			pToneSlide  [pn_ptr]=0.25f;
			pToneWave   [pn_ptr]=0;
			pToneOver   [pn_ptr]=0;

			pNoiseLevel  [pn_ptr]=0;
			pNoiseDecay  [pn_ptr]=0;
			pNoiseSustain[pn_ptr]=0;
			pNoiseRelease[pn_ptr]=0;
			pNoisePitch1 [pn_ptr]=1.0f;
			pNoisePitch2 [pn_ptr]=.25f;
			pNoisePitch2Off[pn_ptr]=0;
			pNoisePitch2Len[pn_ptr]=0;
			pNoisePeriod [pn_ptr]=1.0f;
			pNoiseSeed   [pn_ptr]=0;
			pNoiseType   [pn_ptr]=0;

			pRetrigTime  [pn_ptr]=0;
			pRetrigCount [pn_ptr]=0;
			pRetrigRoute [pn_ptr]=0;

			pFilterLP    [pn_ptr]=1.0f;
			pFilterHP    [pn_ptr]=0;
			pFilterRoute [pn_ptr]=0;

			pDrumGroup   [pn_ptr]=0;
			pDrumBitDepth[pn_ptr]=1.0f;
			pDrumUpdateRate[pn_ptr]=1.0f;
			pDrumVolume  [pn_ptr]=.5f;
			pDrumPan     [pn_ptr]=.5f;

			pVelDrumVolume[pn_ptr]=1.0f;
			pVelTonePitch [pn_ptr]=.5f;
			pVelNoisePitch[pn_ptr]=.5f;
			pVelToneOver      [pn_ptr]=.5f;
		}

		pHat1Length  [pgm]=.5f;
		pHat2Length  [pgm]=.6f;
		pHat3Length  [pgm]=.7f;
		pHatPanWidth [pgm]=0;

		pTom1Pitch   [pgm]=.5f;
		pTom2Pitch   [pgm]=.6f;
		pTom3Pitch   [pgm]=.7f;
		pTomPanWidth [pgm]=.5f;

		pOutputGain[pgm]=1.0f;
	}

	LoadPresetChunk((float*)ChunkPresetData);

	memset(SynthChannel,0,sizeof(SynthChannel));

	CopyBufActive=false;

	srand(1);

	for(i=0;i<65536;++i) Noise[i]=rand();

	MidiQueue.clear();

	UpdateGuiFlag=true;

	suspend();
}



ChipDrum::~ChipDrum()
{
}



float ChipDrum::FloatToHz(float value,float range)
{
	float hz;

	if(value<0.0f) value=0.0f;
	if(value>1.0f) value=1.0f;

	hz=-range*log10(1.0f-value)/3.0f;

	if(hz<0) hz=0;
	if(hz>range) hz=range;

	return hz;
}



int ChipDrum::FloatToNoisePeriod(float value)
{
	const int period[]={7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};

	return period[(int)(value*13.99f)];
}



float ChipDrum::OverdriveValue(float value)
{
	return 1.0f+value*(OVERDRIVE_MAX-1.0f);
}



float* ChipDrum::getVarPtr(VstInt32 index)
{
	float *v=NULL;

	switch(index)
	{
	case pIdToneLevel:   v=pToneLevel; break;
	case pIdToneDecay:   v=pToneDecay; break;
	case pIdToneSustain: v=pToneSustain; break;
	case pIdToneRelease: v=pToneRelease; break;
	case pIdTonePitch:   v=pTonePitch; break;
	case pIdToneSlide:   v=pToneSlide; break;
	case pIdToneWave:    v=pToneWave; break;
	case pIdToneOver:    v=pToneOver; break;

	case pIdNoiseLevel:   v=pNoiseLevel; break;
	case pIdNoiseDecay:   v=pNoiseDecay; break;
	case pIdNoiseSustain: v=pNoiseSustain; break;
	case pIdNoiseRelease: v=pNoiseRelease; break;
	case pIdNoisePitch1:  v=pNoisePitch1; break;
	case pIdNoisePitch2:  v=pNoisePitch2; break;
	case pIdNoisePitch2Off: v=pNoisePitch2Off; break;
	case pIdNoisePitch2Len: v=pNoisePitch2Len; break;
	case pIdNoisePeriod:  v=pNoisePeriod; break;
	case pIdNoiseSeed:    v=pNoiseSeed; break;
	case pIdNoiseType:    v=pNoiseType; break;

	case pIdRetrigTime:   v=pRetrigTime; break;
	case pIdRetrigCount:  v=pRetrigCount; break;
	case pIdRetrigRoute:  v=pRetrigRoute; break;

	case pIdFilterLP:     v=pFilterLP; break;
	case pIdFilterHP:     v=pFilterHP; break;
	case pIdFilterRoute:  v=pFilterRoute; break;

	case pIdDrumGroup:    v=pDrumGroup; break;
	case pIdDrumBitDepth: v=pDrumBitDepth; break;
	case pIdDrumUpdateRate: v=pDrumUpdateRate; break;
	case pIdDrumVolume:   v=pDrumVolume; break;
	case pIdDrumPan:      v=pDrumPan; break;

	case pIdVelDrumVolume: v=pVelDrumVolume; break;
	case pIdVelTonePitch:  v=pVelTonePitch; break;
	case pIdVelNoisePitch: v=pVelNoisePitch; break;
	case pIdVelToneOver:   v=pVelToneOver; break;

	case pIdHat1Length:   v=pHat1Length; break;
	case pIdHat2Length:   v=pHat2Length; break;
	case pIdHat3Length:   v=pHat3Length; break;
	case pIdHatPanWidth:  v=pHatPanWidth; break;

	case pIdTom1Pitch:    v=pTom1Pitch; break;
	case pIdTom2Pitch:    v=pTom2Pitch; break;
	case pIdTom3Pitch:    v=pTom3Pitch; break;
	case pIdTomPanWidth:  v=pTomPanWidth; break;

	case pIdOutputGain:   v=pOutputGain; break;
	}

	return v;
}



bool ChipDrum::isPatchLevelVar(VstInt32 index)
{
	switch(index)
	{
	case pIdOutputGain:
	case pIdHat1Length:
	case pIdHat2Length:
	case pIdHat3Length:
	case pIdHatPanWidth:
	case pIdTom1Pitch:
	case pIdTom2Pitch:
	case pIdTom3Pitch:
	case pIdTomPanWidth:

		return true;
	}

	return false;		
}



void ChipDrum::setParameter(VstInt32 index,float value)
{
	float *v;
	int newnote;

	switch(index)
	{
	case pIdNoteMapping:
		{
			pNoteMapping[Program]=value;

			newnote=(int)(pNoteMapping[Program]*(SYNTH_NOTES-.01f));

			if(pNoteMappingInt!=newnote)
			{
				pNoteMappingInt=newnote;
				updateDisplay();
			}
		}
		break;

	default:

		v=getVarPtr(index);

		if(v)
		{
			if(isPatchLevelVar(index)) v[Program]=value; else v[PN_PTR(Program,pNoteMappingInt)]=value;
		}
	}
}



float ChipDrum::getParameter(VstInt32 index)
{
	float *v;

	v=getVarPtr(index);

	if(v)
	{
		if(isPatchLevelVar(index)) return v[Program]; else return v[PN_PTR(Program,pNoteMappingInt)];
	}

	return 0;
} 



void ChipDrum::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdNoteMapping:	strcpy(label,"Mapped Note"); break;

	case pIdToneLevel:		strcpy(label,"Tone Level");  break;
	case pIdToneDecay:		strcpy(label,"Tone Decay Time"); break;
	case pIdToneSustain:	strcpy(label,"Tone Sustain Level");  break;
	case pIdToneRelease:	strcpy(label,"Tone Release Time"); break;
	case pIdTonePitch:		strcpy(label,"Tone Pitch");     break;
	case pIdToneSlide:		strcpy(label,"Tone Slide");     break;
	case pIdToneWave:		strcpy(label,"Tone Waveform");  break;
	case pIdToneOver:		strcpy(label,"Tone Overdrive"); break;

	case pIdNoiseLevel:		strcpy(label,"Noise Level");  break;
	case pIdNoiseDecay:		strcpy(label,"Noise Decay Time"); break;
	case pIdNoiseSustain:	strcpy(label,"Noise Sustain Level");  break;
	case pIdNoiseRelease:	strcpy(label,"Noise Release Time"); break;
	case pIdNoisePitch1:	strcpy(label,"Noise Main Pitch"); break;
	case pIdNoisePitch2:	strcpy(label,"Noise Secondary Pitch"); break;
	case pIdNoisePitch2Off:	strcpy(label,"Noise Secondary Pitch Offset"); break;
	case pIdNoisePitch2Len:	strcpy(label,"Noise Secondary Pitch Duration"); break;
	case pIdNoisePeriod:	strcpy(label,"Noise Period");  break;
	case pIdNoiseSeed:	    strcpy(label,"Noise Seed");    break;
	case pIdNoiseType:	    strcpy(label,"Noise Type");    break;

	case pIdRetrigTime:		strcpy(label,"Retrigger Time");  break;
	case pIdRetrigCount:	strcpy(label,"Retrigger Count"); break;
	case pIdRetrigRoute:	strcpy(label,"Retrigger Route"); break;

	case pIdFilterLP:	    strcpy(label,"Filter Low Pass"); break;
	case pIdFilterHP:	    strcpy(label,"Filter High Pass"); break;
	case pIdFilterRoute:	strcpy(label,"Filter Route"); break;

	case pIdDrumGroup:		strcpy(label,"Drum Output"); break;
	case pIdDrumBitDepth:   strcpy(label,"Drum Bit Depth"); break;
	case pIdDrumUpdateRate: strcpy(label,"Drum Update Rate"); break;
	case pIdDrumVolume:		strcpy(label,"Drum Volume"); break;
	case pIdDrumPan:		strcpy(label,"Drum Pan"); break;

	case pIdVelDrumVolume:  strcpy(label,"Vel to Volume"); break;

	case pIdVelTonePitch:   strcpy(label,"Vel to Tone Pitch"); break;
	case pIdVelNoisePitch:  strcpy(label,"Vel to Noise Pitch"); break;
	case pIdVelToneOver:        strcpy(label,"Vel to Tone Overdrive"); break;

	case pIdHat1Length:		strcpy(label,"Hat F# Length"); break;
	case pIdHat2Length:		strcpy(label,"Hat G# Length"); break;
	case pIdHat3Length:		strcpy(label,"Hat A# Length"); break;
	case pIdHatPanWidth:	strcpy(label,"Hat Pan Width"); break;

	case pIdTom1Pitch:		strcpy(label,"Tom G Pitch Offset"); break;
	case pIdTom2Pitch:		strcpy(label,"Tom A Pitch Offset"); break;
	case pIdTom3Pitch:		strcpy(label,"Tom B Pitch Offset"); break;
	case pIdTomPanWidth:	strcpy(label,"Tom Pan Width"); break;

	case pIdOutputGain:		strcpy(label,"Output Gain"); break;

	default:				strcpy(label,"");
	}
} 



void ChipDrum::getParameterDisplay(VstInt32 index,char *text)
{
	VstInt32 pn_ptr;

	pn_ptr=PN_PTR(Program,pNoteMappingInt);

	switch(index)
	{
	case pIdNoteMapping: strcpy(text,pNoteNames[pNoteMappingInt]); break;

	case pIdToneLevel:	 float2string(pToneLevel[pn_ptr],text,5); break;
	case pIdToneDecay:	 float2string(DECAY_TIME_MAX_MS*pToneDecay[pn_ptr],text,5); break;
	case pIdToneSustain: float2string(pToneSustain[pn_ptr],text,5); break;
	case pIdToneRelease: float2string(RELEASE_TIME_MAX_MS*pToneRelease[pn_ptr],text,5); break;
	case pIdTonePitch:   sprintf(text,"%5.1f",FloatToHz(pTonePitch[pn_ptr],TONE_PITCH_MAX_HZ)); break;
	case pIdToneSlide:   float2string(-1.0f+pToneSlide[pn_ptr]*2.0f,text,6); break;
	case pIdToneWave:    strcpy(text,pWaveformNames[(VstInt32)(pToneWave[pn_ptr]*3.99f)]); break;
	case pIdToneOver:	 float2string(OverdriveValue(pToneOver[pn_ptr]),text,5); break;

	case pIdNoiseLevel:   float2string(pNoiseLevel[pn_ptr],text,5); break;
	case pIdNoiseDecay:   float2string(DECAY_TIME_MAX_MS*pNoiseDecay[pn_ptr],text,5); break;
	case pIdNoiseSustain: float2string(pNoiseSustain[pn_ptr],text,5); break;
	case pIdNoiseRelease: float2string(RELEASE_TIME_MAX_MS*pNoiseRelease[pn_ptr],text,5); break;
	case pIdNoisePitch1:  sprintf(text,"%5.1f",(pNoisePitch1[pn_ptr]*NOISE_PITCH_MAX_HZ)); break;
	case pIdNoisePitch2:  sprintf(text,"%5.1f",(pNoisePitch2[pn_ptr]*NOISE_PITCH_MAX_HZ)); break;
	case pIdNoisePitch2Off: sprintf(text,"%5.1f",(pNoisePitch2Off[pn_ptr]*DECAY_TIME_MAX_MS)); break;
	case pIdNoisePitch2Len: sprintf(text,"%5.1f",(pNoisePitch2Len[pn_ptr]*NOISE_BURST_MAX_MS)); break;
	case pIdNoisePeriod:  sprintf(text,"%i",(FloatToNoisePeriod(pNoisePeriod[pn_ptr])+1)); break;
	case pIdNoiseSeed:    if(pNoiseSeed[pn_ptr]>0) float2string(pNoiseSeed[pn_ptr],text,6); else strcpy(text,"Random"); break;
	case pIdNoiseType:    strcpy(text,pNoiseType[pn_ptr]<.5f?"White":"Digital"); break;

	case pIdRetrigTime:   float2string(RETRIGGER_MAX_MS*pRetrigTime[pn_ptr],text,5); break;
	case pIdRetrigCount:  sprintf(text,"%i",(VstInt32)(1+RETRIGGER_MAX_COUNT*pRetrigCount[pn_ptr])); break;
	case pIdRetrigRoute:  strcpy(text,pRetrigRouteNames[(VstInt32)(pRetrigRoute[pn_ptr]*2.99f)]); break;
	
	case pIdFilterLP:	  if(pFilterLP[pn_ptr]<1.0f) float2string(FILTER_CUTOFF_MAX_HZ*pFilterLP[pn_ptr],text,5); else strcpy(text,"Disabled"); break;
	case pIdFilterHP:	  if(pFilterHP[pn_ptr]>0.0f) float2string(FILTER_CUTOFF_MAX_HZ*pFilterHP[pn_ptr],text,5); else strcpy(text,"Disabled"); break;
	case pIdFilterRoute:  strcpy(text,pFilterRouteNames[(VstInt32)(pFilterRoute[pn_ptr]*3.99f)]); break;

	case pIdDrumGroup:    sprintf(text,"%i",(VstInt32)(pDrumGroup[pn_ptr]*((NUM_OUTPUTS/2)-0.1f))); break;
	case pIdDrumBitDepth: strcpy(text,pBitDepthNames[(VstInt32)(pDrumBitDepth[pn_ptr]*7.99f)]); break;
	case pIdDrumUpdateRate: if(pDrumUpdateRate[pn_ptr]<1.0f) float2string(MIN_UPDATE_RATE+pDrumUpdateRate[pn_ptr]*MAX_UPDATE_RATE,text,5); else strcpy(text,"Analog"); break;
	case pIdDrumVolume:   dB2string(pDrumVolume[pn_ptr],text,3); break;
	case pIdDrumPan:      float2string(pDrumPan[pn_ptr]-.5f,text,5); break;

	case pIdVelDrumVolume: float2string(pVelDrumVolume[pn_ptr],text,5); break;
	case pIdVelTonePitch:  float2string(pVelTonePitch [pn_ptr]*2.0f-1.0f,text,5); break;
	case pIdVelNoisePitch: float2string(pVelNoisePitch[pn_ptr]*2.0f-1.0f,text,5); break;
	case pIdVelToneOver:   float2string(pVelToneOver  [pn_ptr]*2.0f-1.0f,text,5); break;

	case pIdHat1Length:	  float2string(pHat1Length [Program]*2.0f-1.0f,text,5); break;
	case pIdHat2Length:	  float2string(pHat2Length [Program]*2.0f-1.0f,text,5); break;
	case pIdHat3Length:	  float2string(pHat3Length [Program]*2.0f-1.0f,text,5); break;
	case pIdHatPanWidth:  float2string(pHatPanWidth[Program]*2.0f-1.0f,text,5); break;

	case pIdTom1Pitch:	  float2string(pTom1Pitch  [Program]*2.0f-1.0f,text,5);break;
	case pIdTom2Pitch:	  float2string(pTom2Pitch  [Program]*2.0f-1.0f,text,5); break;
	case pIdTom3Pitch:    float2string(pTom3Pitch  [Program]*2.0f-1.0f,text,5); break;
	case pIdTomPanWidth:  float2string(pTomPanWidth[Program]*2.0f-1.0f,text,5); break;

	case pIdOutputGain:   dB2string(pOutputGain[Program],text,3); break;

	default:			  strcpy(text,"");
	}
} 



void ChipDrum::getParameterLabel(VstInt32 index,char *label)
{
	VstInt32 pn_ptr;

	pn_ptr=PN_PTR(Program,pNoteMappingInt);

	switch(index)
	{   
	case pIdToneDecay:
	case pIdToneRelease:
	case pIdNoiseDecay:
	case pIdNoiseRelease:
	case pIdNoisePitch2Off:
	case pIdNoisePitch2Len:
	case pIdRetrigTime:  strcpy(label,"ms"); break;
	case pIdTonePitch:
	case pIdNoisePitch1:
	case pIdNoisePitch2: strcpy(label,"Hz"); break;
	case pIdFilterLP: strcpy(label,pFilterLP[pn_ptr]<1.0f?"Hz":""); break;
	case pIdFilterHP: strcpy(label,pFilterHP[pn_ptr]>0.0f?"Hz":""); break;
	case pIdNoisePeriod: strcpy(label,"samples"); break;
	case pIdDrumUpdateRate: strcpy(label,pDrumUpdateRate[pn_ptr]<1.0f?"Hz":""); break;
	case pIdDrumVolume:
	case pIdOutputGain:  strcpy(label,"dB"); break;
	default:             strcpy(label,""); break;
	}
} 



VstInt32 ChipDrum::getProgram(void)
{
	return Program;
}



void ChipDrum::setProgram(VstInt32 program)
{
	Program=program;

	UpdateGuiFlag=true;
}



void ChipDrum::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void ChipDrum::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 ChipDrum::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



bool ChipDrum::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
	if(index<0||index>=NUM_OUTPUTS) return false;

	sprintf(properties->label,"Group %1d Out",index+1);

	properties->flags=kVstPinIsStereo|kVstPinIsActive;
	properties->arrangementType=kSpeakerArrStereo;

	return true;
}



VstInt32 ChipDrum::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 ChipDrum::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 ChipDrum::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 ChipDrum::LoadStringChunk(char *str,int max_length,float *src)
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



void ChipDrum::CopyDrum(void)
{
	VstInt32 pn_ptr,ptr;

	pn_ptr=PN_PTR(Program,pNoteMappingInt);

	ptr=0;

	CopyBuf[ptr++]=pToneLevel  [pn_ptr];
	CopyBuf[ptr++]=pToneDecay  [pn_ptr];
	CopyBuf[ptr++]=pToneSustain[pn_ptr];
	CopyBuf[ptr++]=pToneRelease[pn_ptr];
	CopyBuf[ptr++]=pTonePitch  [pn_ptr];
	CopyBuf[ptr++]=pToneSlide  [pn_ptr];
	CopyBuf[ptr++]=pToneWave   [pn_ptr];
	CopyBuf[ptr++]=pToneOver   [pn_ptr];

	CopyBuf[ptr++]=pNoiseLevel  [pn_ptr];
	CopyBuf[ptr++]=pNoiseDecay  [pn_ptr];
	CopyBuf[ptr++]=pNoiseSustain[pn_ptr];
	CopyBuf[ptr++]=pNoiseRelease[pn_ptr];
	CopyBuf[ptr++]=pNoisePitch1 [pn_ptr];
	CopyBuf[ptr++]=pNoisePitch2 [pn_ptr];
	CopyBuf[ptr++]=pNoisePitch2Off[pn_ptr];
	CopyBuf[ptr++]=pNoisePitch2Len[pn_ptr];
	CopyBuf[ptr++]=pNoisePeriod [pn_ptr];
	CopyBuf[ptr++]=pNoiseSeed   [pn_ptr];
	CopyBuf[ptr++]=pNoiseType   [pn_ptr];

	CopyBuf[ptr++]=pRetrigTime  [pn_ptr];
	CopyBuf[ptr++]=pRetrigCount [pn_ptr];
	CopyBuf[ptr++]=pRetrigRoute [pn_ptr];

	CopyBuf[ptr++]=pFilterLP    [pn_ptr];
	CopyBuf[ptr++]=pFilterHP    [pn_ptr];
	CopyBuf[ptr++]=pFilterRoute [pn_ptr];

	CopyBuf[ptr++]=pDrumGroup   [pn_ptr];
	CopyBuf[ptr++]=pDrumBitDepth[pn_ptr];
	CopyBuf[ptr++]=pDrumUpdateRate[pn_ptr];
	CopyBuf[ptr++]=pDrumVolume  [pn_ptr];
	CopyBuf[ptr++]=pDrumPan     [pn_ptr];

	CopyBuf[ptr++]=pVelDrumVolume[pn_ptr];
	CopyBuf[ptr++]=pVelTonePitch [pn_ptr];
	CopyBuf[ptr++]=pVelNoisePitch[pn_ptr];
	CopyBuf[ptr++]=pVelToneOver  [pn_ptr];

	CopyBufActive=true;
}



void ChipDrum::PasteDrum(void)
{
	VstInt32 pn_ptr,ptr;

	if(!CopyBufActive) return;

	pn_ptr=PN_PTR(Program,pNoteMappingInt);

	ptr=0;

	pToneLevel  [pn_ptr]=CopyBuf[ptr++];
	pToneDecay  [pn_ptr]=CopyBuf[ptr++];
	pToneSustain[pn_ptr]=CopyBuf[ptr++];
	pToneRelease[pn_ptr]=CopyBuf[ptr++];
	pTonePitch  [pn_ptr]=CopyBuf[ptr++];
	pToneSlide  [pn_ptr]=CopyBuf[ptr++];
	pToneWave   [pn_ptr]=CopyBuf[ptr++];
	pToneOver   [pn_ptr]=CopyBuf[ptr++];

	pNoiseLevel  [pn_ptr]=CopyBuf[ptr++];
	pNoiseDecay  [pn_ptr]=CopyBuf[ptr++];
	pNoiseSustain[pn_ptr]=CopyBuf[ptr++];
	pNoiseRelease[pn_ptr]=CopyBuf[ptr++];
	pNoisePitch1 [pn_ptr]=CopyBuf[ptr++];
	pNoisePitch2 [pn_ptr]=CopyBuf[ptr++];
	pNoisePitch2Off[pn_ptr]=CopyBuf[ptr++];
	pNoisePitch2Len[pn_ptr]=CopyBuf[ptr++];
	pNoisePeriod [pn_ptr]=CopyBuf[ptr++];
	pNoiseSeed   [pn_ptr]=CopyBuf[ptr++];
	pNoiseType   [pn_ptr]=CopyBuf[ptr++];

	pRetrigTime [pn_ptr]=CopyBuf[ptr++];
	pRetrigCount[pn_ptr]=CopyBuf[ptr++];
	pRetrigRoute[pn_ptr]=CopyBuf[ptr++];

	pFilterLP   [pn_ptr]=CopyBuf[ptr++];
	pFilterHP   [pn_ptr]=CopyBuf[ptr++];
	pFilterRoute[pn_ptr]=CopyBuf[ptr++];

	pDrumGroup   [pn_ptr]=CopyBuf[ptr++];
	pDrumBitDepth[pn_ptr]=CopyBuf[ptr++];
	pDrumUpdateRate[pn_ptr]=CopyBuf[ptr++];
	pDrumVolume  [pn_ptr]=CopyBuf[ptr++];
	pDrumPan     [pn_ptr]=CopyBuf[ptr++];

	pVelDrumVolume[pn_ptr]=CopyBuf[ptr++];
	pVelTonePitch [pn_ptr]=CopyBuf[ptr++];
	pVelNoisePitch[pn_ptr]=CopyBuf[ptr++];
	pVelToneOver  [pn_ptr]=CopyBuf[ptr++];

	UpdateGuiFlag=true;
}



VstInt32 ChipDrum::SavePresetChunk(float *chunk)
{
	int ptr,pgm,pn_ptr,note;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		for(note=0;note<SYNTH_NOTES;++note)
		{
			chunk[ptr++]=NOTE; chunk[ptr++]=(float)note;

			pn_ptr=PN_PTR(pgm,note);

			chunk[ptr++]=TLVL; chunk[ptr++]=pToneLevel  [pn_ptr];
			chunk[ptr++]=TDC1; chunk[ptr++]=pToneDecay  [pn_ptr];
			chunk[ptr++]=TDCL; chunk[ptr++]=pToneSustain[pn_ptr];
			chunk[ptr++]=TDC2; chunk[ptr++]=pToneRelease[pn_ptr];
			chunk[ptr++]=TPIT; chunk[ptr++]=pTonePitch  [pn_ptr];
			chunk[ptr++]=TSLD; chunk[ptr++]=pToneSlide  [pn_ptr];
			chunk[ptr++]=TWAV; chunk[ptr++]=pToneWave   [pn_ptr];
			chunk[ptr++]=TOVR; chunk[ptr++]=pToneOver   [pn_ptr];
		
			chunk[ptr++]=NLVL; chunk[ptr++]=pNoiseLevel  [pn_ptr];
			chunk[ptr++]=NDC1; chunk[ptr++]=pNoiseDecay  [pn_ptr];
			chunk[ptr++]=NDCL; chunk[ptr++]=pNoiseSustain[pn_ptr];
			chunk[ptr++]=NDC2; chunk[ptr++]=pNoiseRelease[pn_ptr];
			chunk[ptr++]=NPT1; chunk[ptr++]=pNoisePitch1 [pn_ptr];
			chunk[ptr++]=NPT2; chunk[ptr++]=pNoisePitch2 [pn_ptr];
			chunk[ptr++]=N2OF; chunk[ptr++]=pNoisePitch2Off[pn_ptr];
			chunk[ptr++]=N2LN; chunk[ptr++]=pNoisePitch2Len[pn_ptr];
			chunk[ptr++]=NPRD; chunk[ptr++]=pNoisePeriod [pn_ptr];
			chunk[ptr++]=NPSE; chunk[ptr++]=pNoiseSeed   [pn_ptr];
			chunk[ptr++]=NTYP; chunk[ptr++]=pNoiseType   [pn_ptr];

			chunk[ptr++]=RTME; chunk[ptr++]=pRetrigTime  [pn_ptr];
			chunk[ptr++]=RCNT; chunk[ptr++]=pRetrigCount [pn_ptr];
			chunk[ptr++]=RRTE; chunk[ptr++]=pRetrigRoute [pn_ptr];

			chunk[ptr++]=LLPF; chunk[ptr++]=pFilterLP    [pn_ptr];
			chunk[ptr++]=HHPF; chunk[ptr++]=pFilterHP    [pn_ptr];
			chunk[ptr++]=FLTR; chunk[ptr++]=pFilterRoute [pn_ptr];

			chunk[ptr++]=GRPO; chunk[ptr++]=pDrumGroup   [pn_ptr];
			chunk[ptr++]=BDPT; chunk[ptr++]=pDrumBitDepth[pn_ptr];
			chunk[ptr++]=UPDR; chunk[ptr++]=pDrumUpdateRate[pn_ptr];
			chunk[ptr++]=VOLU; chunk[ptr++]=pDrumVolume  [pn_ptr];
			chunk[ptr++]=PANO; chunk[ptr++]=pDrumPan     [pn_ptr];

			chunk[ptr++]=VLDV; chunk[ptr++]=pVelDrumVolume[pn_ptr];
			chunk[ptr++]=VLTP; chunk[ptr++]=pVelTonePitch [pn_ptr];
			chunk[ptr++]=VLNP; chunk[ptr++]=pVelNoisePitch[pn_ptr];
			chunk[ptr++]=VLOD; chunk[ptr++]=pVelToneOver  [pn_ptr];
		}

		chunk[ptr++]=H1LN; chunk[ptr++]=pHat1Length [pgm];
		chunk[ptr++]=H2LN; chunk[ptr++]=pHat2Length [pgm];
		chunk[ptr++]=H3LN; chunk[ptr++]=pHat3Length [pgm];
		chunk[ptr++]=HPAN; chunk[ptr++]=pHatPanWidth[pgm];

		chunk[ptr++]=T1PT; chunk[ptr++]=pTom1Pitch  [pgm];
		chunk[ptr++]=T2PT; chunk[ptr++]=pTom2Pitch  [pgm];
		chunk[ptr++]=T3PT; chunk[ptr++]=pTom3Pitch  [pgm];
		chunk[ptr++]=TPAN; chunk[ptr++]=pTomPanWidth[pgm];

		chunk[ptr++]=GAIN; chunk[ptr++]=pOutputGain [pgm];
	}

	chunk[ptr++]=DONE;

	return ptr*sizeof(float);//size in bytes
}



void ChipDrum::LoadPresetChunk(float *chunk)
{
	int pgm,note,pn_ptr;
	float tag;

	while(1)
	{
		tag=*chunk++;

		if(tag==DONE) break;

		if(tag==PROG) pgm =(int)*chunk++;
		if(tag==NOTE) note=(int)*chunk++;

		if(tag==NAME) chunk+=LoadStringChunk(ProgramName[pgm],MAX_NAME_LEN,chunk);

		pn_ptr=PN_PTR(pgm,note);

		if(tag==TLVL) pToneLevel  [pn_ptr]=*chunk++;
		if(tag==TDC1) pToneDecay  [pn_ptr]=*chunk++;
		if(tag==TDCL) pToneSustain[pn_ptr]=*chunk++;
		if(tag==TDC2) pToneRelease[pn_ptr]=*chunk++;
		if(tag==TPIT) pTonePitch  [pn_ptr]=*chunk++;
		if(tag==TSLD) pToneSlide  [pn_ptr]=*chunk++;
		if(tag==TWAV) pToneWave   [pn_ptr]=*chunk++;
		if(tag==TOVR) pToneOver   [pn_ptr]=*chunk++;
		
		if(tag==NLVL) pNoiseLevel  [pn_ptr]=*chunk++;
		if(tag==NDC1) pNoiseDecay  [pn_ptr]=*chunk++;
		if(tag==NDCL) pNoiseSustain[pn_ptr]=*chunk++;
		if(tag==NDC2) pNoiseRelease[pn_ptr]=*chunk++;
		if(tag==NPT1) pNoisePitch1 [pn_ptr]=*chunk++;
		if(tag==NPT2) pNoisePitch2 [pn_ptr]=*chunk++;
		if(tag==N2OF) pNoisePitch2Off[pn_ptr]=*chunk++;
		if(tag==N2LN) pNoisePitch2Len[pn_ptr]=*chunk++;
		if(tag==NPRD) pNoisePeriod [pn_ptr]=*chunk++;
		if(tag==NPSE) pNoiseSeed   [pn_ptr]=*chunk++;
		if(tag==NTYP) pNoiseType   [pn_ptr]=*chunk++;

		if(tag==RTME) pRetrigTime  [pn_ptr]=*chunk++;
		if(tag==RCNT) pRetrigCount [pn_ptr]=*chunk++;
		if(tag==RRTE) pRetrigRoute [pn_ptr]=*chunk++;

		if(tag==LLPF) pFilterLP    [pn_ptr]=*chunk++;
		if(tag==HHPF) pFilterHP    [pn_ptr]=*chunk++;
		if(tag==FLTR) pFilterRoute [pn_ptr]=*chunk++;

		if(tag==GRPO) pDrumGroup   [pn_ptr]=*chunk++;
		if(tag==BDPT) pDrumBitDepth[pn_ptr]=*chunk++;
		if(tag==UPDR) pDrumUpdateRate[pn_ptr]=*chunk++;
		if(tag==VOLU) pDrumVolume  [pn_ptr]=*chunk++;
		if(tag==PANO) pDrumPan     [pn_ptr]=*chunk++;

		if(tag==VLDV) pVelDrumVolume[pn_ptr]=*chunk++;
		if(tag==VLTP) pVelTonePitch [pn_ptr]=*chunk++;
		if(tag==VLNP) pVelNoisePitch[pn_ptr]=*chunk++;
		if(tag==VLOD) pVelToneOver  [pn_ptr]=*chunk++;

		if(tag==H1LN) pHat1Length [pgm]=*chunk++;
		if(tag==H2LN) pHat2Length [pgm]=*chunk++;
		if(tag==H3LN) pHat3Length [pgm]=*chunk++;
		if(tag==HPAN) pHatPanWidth[pgm]=*chunk++;

		if(tag==T1PT) pTom1Pitch  [pgm]=*chunk++;
		if(tag==T2PT) pTom2Pitch  [pgm]=*chunk++;
		if(tag==T3PT) pTom3Pitch  [pgm]=*chunk++;
		if(tag==TPAN) pTomPanWidth[pgm]=*chunk++;

		if(tag==GAIN) pOutputGain [pgm]=*chunk++;
	}

	pNoteMappingInt=(int)(pNoteMapping[Program]*(SYNTH_NOTES-.01f));
}



VstInt32 ChipDrum::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 ChipDrum::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	UpdateGuiFlag=true;

	return 0;
}



void ChipDrum::UpdateGUI(bool display)
{
#ifdef _GUI_ACTIVE_
	if(editor) ((GUI*)editor)->Update();
#endif

	if(display) updateDisplay();
}



void ChipDrum::MidiAddNewNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type=mEventTypeNote;
	entry.delta=delta;
	entry.note=note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void ChipDrum::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



VstInt32 ChipDrum::processEvents(VstEvents* ev) 
{ 
	VstMidiEvent* event;
	VstInt32 i,status,note,velocity;
	char* midiData;

	for(i=0;i<ev->numEvents;++i) 
	{ 
		event=(VstMidiEvent*)ev->events[i];

		if(event->type!=kVstMidiType) continue; 

		midiData=event->midiData;

		status=midiData[0]&0xf0;

		if(status==0x90||status==0x80)	//note on, note off
		{ 
			note=midiData[1]&0x7f; 

			velocity=midiData[2]&0x7f; 

			if(status==0x80) velocity=0; 

			MidiAddNewNote(event->deltaFrames,note,velocity);
		}

		if(status==0xc0)	//program change
		{
			MidiAddProgramChange(event->deltaFrames,midiData[1]&0x7f);
		}
	} 

	return 1; 
} 



void ChipDrum::SynthRestartTone(VstInt32 chn)
{
	VstInt32 pn_ptr;
	float dc,rl,freq,div1,div2;

	pn_ptr=PN_PTR(Program,SynthChannel[chn].note);

	dc=pToneDecay  [pn_ptr];
	rl=pToneRelease[pn_ptr];

	if(SynthChannel[chn].notem==6)
	{
		dc+=(pHat1Length[Program]-.5f);
		rl+=(pHat1Length[Program]-.5f);
	}

	if(SynthChannel[chn].notem==8)
	{
		dc+=(pHat2Length[Program]-.5f);
		rl+=(pHat2Length[Program]-.5f);
	}

	if(SynthChannel[chn].notem==10)
	{
		dc+=(pHat3Length[Program]-.5f);
		rl+=(pHat3Length[Program]-.5f);
	}

	if(dc<0) dc=0;
	if(dc>1) dc=1;
	if(rl<0) rl=0;
	if(rl>1) rl=1;

	div1=dc*(DECAY_TIME_MAX_MS  /1000.0f)*sampleRate;
	div2=rl*(RELEASE_TIME_MAX_MS/1000.0f)*sampleRate;

	SynthChannel[chn].tone_level  =pToneLevel[pn_ptr];
	SynthChannel[chn].tone_sustain=pToneLevel[pn_ptr]*pToneSustain[pn_ptr];
	SynthChannel[chn].tone_decay  =div1>0?((SynthChannel[chn].tone_level-SynthChannel[chn].tone_sustain)/div1/OVERSAMPLING):1.0f;
	SynthChannel[chn].tone_release=div2>0?(SynthChannel[chn].tone_sustain/div2/OVERSAMPLING):1.0f;

	SynthChannel[chn].tone_env_acc=0;
	SynthChannel[chn].tone_env_add1=div1>0?(1.0f/div1/OVERSAMPLING):1.0f;
	SynthChannel[chn].tone_env_add2=div2>0?(1.0f/div2/OVERSAMPLING):1.0f;

	if(SynthChannel[chn].tone_decay==0) SynthChannel[chn].tone_decay=.000001f;

	freq=pTonePitch[pn_ptr]+(pVelTonePitch[pn_ptr]*2.0f-1.0f)*(1.0f-SynthChannel[chn].velocity)*.1f;

	if(SynthChannel[chn].notem==7 ) freq+=(pTom1Pitch[Program]*2.0f-1.0f)*.1f;
	if(SynthChannel[chn].notem==9 ) freq+=(pTom2Pitch[Program]*2.0f-1.0f)*.1f;
	if(SynthChannel[chn].notem==11) freq+=(pTom3Pitch[Program]*2.0f-1.0f)*.1f;

	SynthChannel[chn].tone_acc=0;
	SynthChannel[chn].tone_add=FloatToHz(freq,TONE_PITCH_MAX_HZ)/sampleRate/OVERSAMPLING;
	SynthChannel[chn].tone_delta=(-1.0f+pToneSlide[pn_ptr]*2.0f)*(48000.0f/sampleRate)/(50.0f*sampleRate)/OVERSAMPLING;
}



void ChipDrum::SynthRestartNoise(VstInt32 chn)
{
	VstInt32 pn_ptr;
	float dc,rl,div1,div2,freq1,freq2;

	pn_ptr=PN_PTR(Program,SynthChannel[chn].note);

	dc=pNoiseDecay  [pn_ptr];
	rl=pNoiseRelease[pn_ptr];

	if(SynthChannel[chn].notem==6)
	{
		dc+=(pHat1Length[Program]-.5f);
		rl+=(pHat1Length[Program]-.5f);
	}

	if(SynthChannel[chn].notem==8)
	{
		dc+=(pHat2Length[Program]-.5f);
		rl+=(pHat2Length[Program]-.5f);
	}

	if(SynthChannel[chn].notem==10)
	{
		dc+=(pHat3Length[Program]-.5f);
		rl+=(pHat3Length[Program]-.5f);
	}

	if(dc<0) dc=0;
	if(dc>1) dc=1;
	if(rl<0) rl=0;
	if(rl>1) rl=1;

	div1=dc*(DECAY_TIME_MAX_MS  /1000.0f)*sampleRate;
	div2=rl*(RELEASE_TIME_MAX_MS/1000.0f)*sampleRate;

	SynthChannel[chn].noise_level  =pNoiseLevel[pn_ptr];
	SynthChannel[chn].noise_sustain=pNoiseLevel[pn_ptr]*pNoiseSustain[pn_ptr];
	SynthChannel[chn].noise_decay  =div1>0?((SynthChannel[chn].noise_level-SynthChannel[chn].noise_sustain)/div1/OVERSAMPLING):1.0f;
	SynthChannel[chn].noise_release=div2>0?(SynthChannel[chn].noise_sustain/div2/OVERSAMPLING):1.0f;

	SynthChannel[chn].noise_env_acc=0;
	SynthChannel[chn].noise_env_add1=div1>0?(1.0f/div1/OVERSAMPLING):1.0f;
	SynthChannel[chn].noise_env_add2=div2>0?(1.0f/div2/OVERSAMPLING):1.0f;

	freq1=pNoisePitch1[pn_ptr]+(pVelNoisePitch[pn_ptr]*2.0f-1.0f)*(1.0f-SynthChannel[chn].velocity)*.1f;
	freq2=pNoisePitch2[pn_ptr]+(pVelNoisePitch[pn_ptr]*2.0f-1.0f)*(1.0f-SynthChannel[chn].velocity)*.1f;

	SynthChannel[chn].noise_acc=0;
	SynthChannel[chn].noise_add1=freq1*NOISE_PITCH_MAX_HZ/sampleRate/OVERSAMPLING;
	SynthChannel[chn].noise_add2=freq2*NOISE_PITCH_MAX_HZ/sampleRate/OVERSAMPLING;

	if(pNoiseSeed[pn_ptr]>0) SynthChannel[chn].noise_seed=(VstInt32)(pNoiseSeed[pn_ptr]*65535.9f); else SynthChannel[chn].noise_seed=rand();

	SynthChannel[chn].noise_type=pNoiseType[pn_ptr]<.5f?0:1;

	div1=pNoisePitch2Off[pn_ptr]*(DECAY_TIME_MAX_MS/1000.0f)*sampleRate;
	div2=pNoisePitch2Len[pn_ptr]*(NOISE_BURST_MAX_MS/1000.0f)*sampleRate;

	SynthChannel[chn].noise_ptr=0;
	SynthChannel[chn].noise_frame_acc=0;
	SynthChannel[chn].noise_frame_add1=div1>0?(1.0f/div1/OVERSAMPLING):1.0f;
	SynthChannel[chn].noise_frame_add2=div2>0?(1.0f/div2/OVERSAMPLING):1.0f;

	SynthChannel[chn].noise_mask=FloatToNoisePeriod(pNoisePeriod[pn_ptr]);
}



inline float ChipDrum::SynthGetSample(VstInt32 wave,float acc,float over)
{
	float sample;

	switch(wave)
	{
	case 0:	//square
		sample=acc<.5f?1.0f:0;
		break;

	case 1:	//saw
		sample=acc;
		break;

	case 2:	//tri
		sample=(((acc<.5f)?(acc/.5f):((1.0f-acc)/.5f))-.5f)*2.0f;
		break;

	case 3:	//sine
		sample=sinf(F_PI*2.0f*acc);
		break;

	default:
		sample=0;
	}

	sample*=over;

	if(sample<-1.0f) sample=-1.0f;
	if(sample> 1.0f) sample= 1.0f;

	return sample;
}



void ChipDrum::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL[NUM_OUTPUTS/2],*outR[NUM_OUTPUTS/2];
	float level_l[NUM_OUTPUTS/2],level_r[NUM_OUTPUTS/2];
	float vol,out,tone_output,noise_output,pan;
	float normal_output,filter_output;
	VstInt32 i,s,nv,chn,note,notem,pn_ptr;
	double q,w;
	bool update;

	outL[0]=outputs[0];
	outR[0]=outputs[1];
	outL[1]=outputs[2];
	outR[1]=outputs[3];
	outL[2]=outputs[4];
	outR[2]=outputs[5];
	outL[3]=outputs[6];
	outR[3]=outputs[7];

	sampleRate=(float)updateSampleRate();	//variable stored in the main class and accessible for other functions

	while(--sampleFrames>=0)
	{
		for(i=0;i<(VstInt32)MidiQueue.size();++i)
		{
			if(MidiQueue[i].delta>=0)
			{
				--MidiQueue[i].delta;

				if(MidiQueue[i].delta<0)//new note
				{
					switch(MidiQueue[i].type)
					{
					case mEventTypeNote:
						{
							note=MidiQueue[i].note%12;
							notem=note;

							switch(note)
							{
							case 0:  chn=0; break;//C
							case 1:  chn=8; break;//C#
							case 2:  chn=1; break;//D
							case 3:  chn=7; break;//D#
							case 4:  chn=2; break;//E
							case 5:  chn=3; break;//F
							case 6:  chn=7; note=6; break;//F#
							case 7:  chn=4; note=7; break;//G
							case 8:  chn=7; note=6; break;//G#
							case 9:  chn=5; note=7; break;//A
							case 10: chn=7; note=6; break;//A#
							case 11: chn=6; note=7; break;//B
							}

							if(MidiQueue[i].velocity)//key on
							{
								SynthChannel[chn].note=note;
								SynthChannel[chn].notem=notem;

								SynthChannel[chn].velocity=((float)MidiQueue[i].velocity/100.0f);

								pn_ptr=PN_PTR(Program,note);

								SynthRestartTone(chn);
								SynthRestartNoise(chn);

								vol=pDrumVolume[pn_ptr]*(1.0f*(1.0f-pVelDrumVolume[pn_ptr])+SynthChannel[chn].velocity*pVelDrumVolume[pn_ptr]);

								pan=pDrumPan[pn_ptr];

								if(notem==6)  pan-=pHatPanWidth[Program]*.5f;
								if(notem==7)  pan-=pTomPanWidth[Program]*.5f;
								if(notem==10) pan+=pHatPanWidth[Program]*.5f;
								if(notem==11) pan+=pTomPanWidth[Program]*.5f;

								if(pan<0.0f) pan=0.0f;
								if(pan>1.0f) pan=1.0f;

								SynthChannel[chn].volume_l=vol*(1.0f-pan);
								SynthChannel[chn].volume_r=vol*pan;

								SynthChannel[chn].tone_wave=(VstInt32)(pToneWave[pn_ptr]*3.99f);

								SynthChannel[chn].tone_over=OverdriveValue(pToneOver[pn_ptr]+(pVelToneOver[pn_ptr]*2.0f-1.0f)*(1.0f-SynthChannel[chn].velocity)*.5f);

								SynthChannel[chn].group=(VstInt32)(pDrumGroup[pn_ptr]*((NUM_OUTPUTS/2)-.1f));

								SynthChannel[chn].bit_depth=pBitDepth[(VstInt32)(pDrumBitDepth[pn_ptr]*7.99f)];

								SynthChannel[chn].frame_acc=1.0f;	//force first update

								if(pDrumUpdateRate[pn_ptr]<1.0f)
								{
									SynthChannel[chn].frame_add=1.0f/(sampleRate/(MIN_UPDATE_RATE+pDrumUpdateRate[pn_ptr]*MAX_UPDATE_RATE));
								}
								else
								{
									SynthChannel[chn].frame_add=0;	//0 for instant update
								}

								//calculate filter coefficients, does not change during note play

								SynthChannel[chn].filter_route=(VstInt32)(pFilterRoute[pn_ptr]*3.99f);

								if(pFilterLP[pn_ptr]<1.0f)
								{
									SynthChannel[chn].lpf_resofreq=pFilterLP[pn_ptr]*FILTER_CUTOFF_MAX_HZ;
								}
								else
								{
									SynthChannel[chn].lpf_resofreq=0;
								}

								//if(SynthChannel[chn].lpf_resofreq<FILTER_CUTOFF_MIN_HZ) SynthChannel[chn].lpf_resofreq=FILTER_CUTOFF_MIN_HZ;
								//if(SynthChannel[chn].lpf_resofreq>FILTER_CUTOFF_MAX_HZ) SynthChannel[chn].lpf_resofreq=FILTER_CUTOFF_MAX_HZ;
					
								w=2.0*M_PI*SynthChannel[chn].lpf_resofreq/sampleRate;		// Pole angle
								q=1.0-w/(2.0*(1.0+0.5/(1.0+w))+w-2.0); // Pole magnitude

								SynthChannel[chn].lpf_r=q*q;
								SynthChannel[chn].lpf_c=SynthChannel[chn].lpf_r+1.0-2.0*cos(w)*q;

								if(pFilterHP[pn_ptr]>0.0f)
								{
									SynthChannel[chn].hpf_resofreq=pFilterHP[pn_ptr]*FILTER_CUTOFF_MAX_HZ;
								}
								else
								{
									SynthChannel[chn].hpf_resofreq=0;
								}

								//if(SynthChannel[chn].hpf_resofreq<FILTER_CUTOFF_MIN_HZ) SynthChannel[chn].hpf_resofreq=FILTER_CUTOFF_MIN_HZ;
								//if(SynthChannel[chn].hpf_resofreq>FILTER_CUTOFF_MAX_HZ) SynthChannel[chn].hpf_resofreq=FILTER_CUTOFF_MAX_HZ;
					
								w=2.0*M_PI*SynthChannel[chn].hpf_resofreq/sampleRate;		// Pole angle
								q=1.0-w/(2.0*(1.0+0.5/(1.0+w))+w-2.0); // Pole magnitude

								SynthChannel[chn].hpf_r=q*q;
								SynthChannel[chn].hpf_c=SynthChannel[chn].hpf_r+1.0-2.0*cos(w)*q;

								SynthChannel[chn].lpf_vibrapos=0;
								SynthChannel[chn].lpf_vibraspeed=0;
								SynthChannel[chn].hpf_vibrapos=0;
								SynthChannel[chn].hpf_vibraspeed=0;

								//retrigger

								SynthChannel[chn].retrigger_acc=0;
								SynthChannel[chn].retrigger_add=1.0f/(RETRIGGER_MAX_MS*pRetrigTime[pn_ptr]/1000.0f*sampleRate);
								SynthChannel[chn].retrigger_count=(VstInt32)(pRetrigCount[pn_ptr]*RETRIGGER_MAX_COUNT);
								SynthChannel[chn].retrigger_route=(VstInt32)(pRetrigRoute[pn_ptr]*2.99f);
							}
						}
						break;

					case mEventTypeProgram:
						{
							Program=MidiQueue[i].program;

							UpdateGuiFlag=true;
						}
						break;
					}
				}
			}
		}

		level_l[0]=0;
		level_l[1]=0;
		level_l[2]=0;
		level_l[3]=0;
		level_r[0]=0;
		level_r[1]=0;
		level_r[2]=0;
		level_r[3]=0;

		for(chn=0;chn<SYNTH_CHANNELS;++chn)
		{
			//frame counter

			update=false;

			if(SynthChannel[chn].frame_add>0)
			{
				SynthChannel[chn].frame_acc+=SynthChannel[chn].frame_add;

				while(SynthChannel[chn].frame_acc>=1.0f)
				{
					SynthChannel[chn].frame_acc-=1.0f;

					update=true;
				}
			}
			else
			{
				update=true;
			}
			
			if(update)
			{
				SynthChannel[chn].tone_adda  =SynthChannel[chn].tone_add;
				SynthChannel[chn].noise_add1a=SynthChannel[chn].noise_add1;
				SynthChannel[chn].noise_add2a=SynthChannel[chn].noise_add2;
			}

			tone_output=0;
			noise_output=0;

			for(s=0;s<OVERSAMPLING;++s)
			{
				//process tone

				if(SynthChannel[chn].tone_level>0)
				{
					//envelope

					if(SynthChannel[chn].tone_env_acc<1.0f)
					{
						SynthChannel[chn].tone_env_acc+=SynthChannel[chn].tone_env_add1;

						SynthChannel[chn].tone_level-=SynthChannel[chn].tone_decay;

						if(SynthChannel[chn].tone_level<SynthChannel[chn].tone_sustain) SynthChannel[chn].tone_level=SynthChannel[chn].tone_sustain;
					}
					else
					{
						SynthChannel[chn].tone_env_acc+=SynthChannel[chn].tone_env_add2;

						SynthChannel[chn].tone_level-=SynthChannel[chn].tone_release;

						if(SynthChannel[chn].tone_level<0) SynthChannel[chn].tone_level=0;
					}

					//oscillator

					SynthChannel[chn].tone_acc+=SynthChannel[chn].tone_adda;
					SynthChannel[chn].tone_add+=SynthChannel[chn].tone_delta;

					while(SynthChannel[chn].tone_acc>=1.0f) SynthChannel[chn].tone_acc-=1.0f;

					if(SynthChannel[chn].tone_add<0.0f) SynthChannel[chn].tone_add=0.0f;
					if(SynthChannel[chn].tone_add>1.0f) SynthChannel[chn].tone_add=1.0f;

					SynthChannel[chn].tone_sample=SynthGetSample(SynthChannel[chn].tone_wave,SynthChannel[chn].tone_acc,SynthChannel[chn].tone_over)*SynthChannel[chn].tone_level;

					tone_output+=SynthChannel[chn].tone_sample;
				}

				//process noise

				if(SynthChannel[chn].noise_level>0)
				{
					//envelope

					if(SynthChannel[chn].noise_env_acc<1.0f)
					{
						SynthChannel[chn].noise_env_acc+=SynthChannel[chn].noise_env_add1;

						SynthChannel[chn].noise_level-=SynthChannel[chn].noise_decay;

						if(SynthChannel[chn].noise_level<SynthChannel[chn].noise_sustain) SynthChannel[chn].noise_level=SynthChannel[chn].noise_sustain;
					}
					else
					{
						SynthChannel[chn].noise_env_acc+=SynthChannel[chn].noise_env_add2;

						SynthChannel[chn].noise_level-=SynthChannel[chn].noise_release;

						if(SynthChannel[chn].noise_level<0) SynthChannel[chn].noise_level=0;
					}

					//burst sequencer and oscillator

					if(SynthChannel[chn].noise_frame_acc>=1.0f&&SynthChannel[chn].noise_frame_acc<2.0f)
					{
						SynthChannel[chn].noise_acc+=SynthChannel[chn].noise_add2a;
					}
					else
					{
						SynthChannel[chn].noise_acc+=SynthChannel[chn].noise_add1a;
					}

					while(SynthChannel[chn].noise_acc>=1.0f)
					{
						SynthChannel[chn].noise_acc-=1.0f;

						++SynthChannel[chn].noise_ptr;
					}

					nv=Noise[((SynthChannel[chn].noise_ptr&SynthChannel[chn].noise_mask)+SynthChannel[chn].noise_seed)&65535];

					if(SynthChannel[chn].noise_type)
					{
						SynthChannel[chn].noise_sample=(nv&1)?SynthChannel[chn].noise_level*.75f:0;
					}
					else
					{
						SynthChannel[chn].noise_sample=(float)nv*(1.0f/256.0f)*SynthChannel[chn].noise_level;
					}

					noise_output+=SynthChannel[chn].noise_sample;
				}

				if(SynthChannel[chn].noise_frame_acc<1.0f)
				{
					SynthChannel[chn].noise_frame_acc+=SynthChannel[chn].noise_frame_add1;
				}
				else
				{
					SynthChannel[chn].noise_frame_acc+=SynthChannel[chn].noise_frame_add2;
				}
			}

			tone_output/=OVERSAMPLING;
			noise_output/=OVERSAMPLING;

			//apply bit reduction if needed

			if(SynthChannel[chn].bit_depth>0)
			{
				tone_output =floorf(tone_output *SynthChannel[chn].bit_depth)/SynthChannel[chn].bit_depth;
				noise_output=floorf(noise_output*SynthChannel[chn].bit_depth)/SynthChannel[chn].bit_depth;
			}

			//filter

			switch(SynthChannel[chn].filter_route)
			{
			case 0:
				normal_output=0;
				filter_output=tone_output+noise_output;
				break;

			case 1:
				normal_output=noise_output;
				filter_output=tone_output;
				break;

			case 2:
				normal_output=tone_output;
				filter_output=noise_output;
				break;

			default:
				normal_output=tone_output+noise_output;
				filter_output=0;
			}

			//12db lpf filter

			if(SynthChannel[chn].lpf_resofreq>0)
			{
				SynthChannel[chn].lpf_vibraspeed+=(filter_output*32768.0-SynthChannel[chn].lpf_vibrapos)*SynthChannel[chn].lpf_c;

				SynthChannel[chn].lpf_vibrapos+=SynthChannel[chn].lpf_vibraspeed;

				SynthChannel[chn].lpf_vibraspeed*=SynthChannel[chn].lpf_r;

				filter_output=(float)SynthChannel[chn].lpf_vibrapos;

				filter_output/=32768.0;
			}

			//12db hpf filter

			if(SynthChannel[chn].hpf_resofreq>0)
			{
				SynthChannel[chn].hpf_vibraspeed+=(filter_output*32768.0-SynthChannel[chn].hpf_vibrapos)*SynthChannel[chn].hpf_c;

				SynthChannel[chn].hpf_vibrapos+=SynthChannel[chn].hpf_vibraspeed;

				SynthChannel[chn].hpf_vibraspeed*=SynthChannel[chn].hpf_r;

				out=(float)SynthChannel[chn].hpf_vibrapos;

				out/=32768.0;

				filter_output-=out;
			}

			//mix outputs

			out=normal_output+filter_output;

			level_l[SynthChannel[chn].group]+=out*SynthChannel[chn].volume_l;
			level_r[SynthChannel[chn].group]+=out*SynthChannel[chn].volume_r;

			//retrigger

			if(SynthChannel[chn].retrigger_count)
			{
				SynthChannel[chn].retrigger_acc+=SynthChannel[chn].retrigger_add;

				if(SynthChannel[chn].retrigger_acc>=1.0f)
				{
					SynthChannel[chn].retrigger_acc-=1.0f;

					if(SynthChannel[chn].retrigger_route<2) SynthRestartTone(chn);
					if(SynthChannel[chn].retrigger_route!=1) SynthRestartNoise(chn);

					--SynthChannel[chn].retrigger_count;
				}
			}
		}

		level_l[0]*=pOutputGain[Program];
		level_l[1]*=pOutputGain[Program];
		level_l[2]*=pOutputGain[Program];
		level_l[3]*=pOutputGain[Program];
		level_r[0]*=pOutputGain[Program];
		level_r[1]*=pOutputGain[Program];
		level_r[2]*=pOutputGain[Program];
		level_r[3]*=pOutputGain[Program];

		if(level_l[0]<-1.0f) level_l[0]=-1.0f; else if(level_l[0]> 1.0f) level_l[0]= 1.0f;
		if(level_l[1]<-1.0f) level_l[1]=-1.0f; else if(level_l[1]> 1.0f) level_l[1]= 1.0f;
		if(level_l[2]<-1.0f) level_l[2]=-1.0f; else if(level_l[2]> 1.0f) level_l[2]= 1.0f;
		if(level_l[3]<-1.0f) level_l[3]=-1.0f; else if(level_l[3]> 1.0f) level_l[3]= 1.0f;
		if(level_r[0]<-1.0f) level_r[0]=-1.0f; else if(level_r[0]> 1.0f) level_r[0]= 1.0f;
		if(level_r[1]<-1.0f) level_r[1]=-1.0f; else if(level_r[1]> 1.0f) level_r[1]= 1.0f;
		if(level_r[2]<-1.0f) level_r[2]=-1.0f; else if(level_r[2]> 1.0f) level_r[2]= 1.0f;
		if(level_r[3]<-1.0f) level_r[3]=-1.0f; else if(level_r[3]> 1.0f) level_r[3]= 1.0f;

		(*outL[0]++)=level_l[0];
		(*outR[0]++)=level_r[0];
		(*outL[1]++)=level_l[1];
		(*outR[1]++)=level_r[1];
		(*outL[2]++)=level_l[2];
		(*outR[2]++)=level_r[2];
		(*outL[3]++)=level_l[3];
		(*outR[3]++)=level_r[3];
	}

	MidiQueue.clear();

	if(UpdateGuiFlag)
	{
		UpdateGUI(true);

		UpdateGuiFlag=false;
	}
}
