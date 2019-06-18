#include "TNDrums.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new TNDrums(audioMaster);
}



TNDrums::TNDrums(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 i,j,pgm,chn;
	unsigned int n;

	setNumInputs(NUM_INPUTS);
	setNumOutputs(NUM_OUTPUTS);
	setUniqueID(PLUGIN_UID);

	isSynth();
	canProcessReplacing();
	programsAreChunks(true);

	Program=0;

	LoadPresetChunk((float*)ChunkPresetData);

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm) strcpy(ProgramName[pgm],programDefaultNames[pgm]);

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].toneDuration=0;
		SynthChannel[chn].tonePulse=0;
		SynthChannel[chn].toneType=0;
		SynthChannel[chn].toneWidth=0;
		SynthChannel[chn].toneOutput=0;
		SynthChannel[chn].toneAccumulator=0;
		SynthChannel[chn].toneIncrement=0;
		SynthChannel[chn].toneIncrementDelta=0;

		SynthChannel[chn].noiseDuration=0;
		SynthChannel[chn].noiseOutput=0;
		SynthChannel[chn].noiseAccumulator=0;
		SynthChannel[chn].noiseIncrement=0;
		SynthChannel[chn].noiseIncrementDelta=0;
		SynthChannel[chn].noisePtr=0;

		SynthChannel[chn].mixMode=0;
		SynthChannel[chn].volume=1.0f;
	}

	srand(1);

	for(i=0;i<2048;++i)
	{
		n=0;

		for(j=0;j<4;++j) n=(n<<8)|(rand()&255);

		Noise[i]=n;
	}

	MidiQueue.clear();

	suspend();
}



TNDrums::~TNDrums()
{
}



float TNDrums::FloatToHz(float value,float range)
{
	float hz;

	hz=-range*log10(1.0f-value)/3.0f;

	if(hz<0) hz=0;
	if(hz>range) hz=range;

	return hz;
}



int TNDrums::FloatToNoisePeriod(float value)
{
	const int period[]={4,8,16,32,64,128,256,512,1024,2048};

	return period[(int)(value*9.99f)];
}



void TNDrums::setParameter(VstInt32 index,float value)
{
	int newnote;

	switch(index)
	{
	case pIdNoteMapping:
		{
			pNoteMapping[Program]=value;

			newnote=(int)(pNoteMapping[Program]*11.99f);

			if(pNoteMappingInt!=newnote)
			{
				pNoteMappingInt=newnote;
				updateDisplay();
			}
		}
		break;

	case pIdToneDuration:  pToneDuration [Program][pNoteMappingInt]=value; break;
	case pIdTonePitch:     pTonePitch    [Program][pNoteMappingInt]=value; break;
	case pIdToneSlide:     pToneSlide    [Program][pNoteMappingInt]=value; break;
	case pIdToneType:      pToneType     [Program][pNoteMappingInt]=value; updateDisplay(); break;
	case pIdToneWidth:     pToneWidth    [Program][pNoteMappingInt]=value; break;
	case pIdNoiseDuration: pNoiseDuration[Program][pNoteMappingInt]=value; break;
	case pIdNoisePitch:    pNoisePitch   [Program][pNoteMappingInt]=value; break;
	case pIdNoiseSlide:	   pNoiseSlide   [Program][pNoteMappingInt]=value; break;
	case pIdNoisePeriod:   pNoisePeriod  [Program][pNoteMappingInt]=value; break;
	case pIdMixMode:	   pMixMode      [Program][pNoteMappingInt]=value; break;
	case pIdDrumVolume:    pDrumVolume   [Program][pNoteMappingInt]=value; break;
	case pIdPolyphony:     pPolyphony    [Program]=value; break;
	case pIdOutputGain:    pOutputGain   [Program]=value; break;
	}
}



float TNDrums::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdNoteMapping:   return pNoteMapping  [Program];
	case pIdToneDuration:  return pToneDuration [Program][pNoteMappingInt];
	case pIdTonePitch:     return pTonePitch    [Program][pNoteMappingInt];
	case pIdToneSlide:     return pToneSlide    [Program][pNoteMappingInt];
	case pIdToneType:      return pToneType     [Program][pNoteMappingInt];
	case pIdToneWidth:     return pToneWidth    [Program][pNoteMappingInt];
	case pIdNoiseDuration: return pNoiseDuration[Program][pNoteMappingInt];
	case pIdNoisePitch:    return pNoisePitch   [Program][pNoteMappingInt];
	case pIdNoiseSlide:    return pNoiseSlide   [Program][pNoteMappingInt];
	case pIdNoisePeriod:   return pNoisePeriod  [Program][pNoteMappingInt];
	case pIdMixMode:	   return pMixMode      [Program][pNoteMappingInt];
	case pIdDrumVolume:	   return pDrumVolume   [Program][pNoteMappingInt];
	case pIdPolyphony:     return pPolyphony    [Program];
	case pIdOutputGain:    return pOutputGain   [Program];
	}

	return 0;
} 



void TNDrums::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdNoteMapping:   strcpy(label,"Mapped note");    break;
	case pIdToneDuration:  strcpy(label,"Tone duration");  break;
	case pIdTonePitch:     strcpy(label,"Tone pitch");     break;
	case pIdToneSlide:     strcpy(label,"Tone slide");     break;
	case pIdToneType:      strcpy(label,"Tone type");      break;
	case pIdToneWidth:     strcpy(label,"Tone duty");      break;
	case pIdNoiseDuration: strcpy(label,"Noise duration"); break;
	case pIdNoisePitch:    strcpy(label,"Noise pitch");    break;
	case pIdNoiseSlide:    strcpy(label,"Noise slide");    break;
	case pIdNoisePeriod:   strcpy(label,"Noise period");   break;
	case pIdMixMode:       strcpy(label,"Tone/noise mix"); break;
	case pIdDrumVolume:    strcpy(label,"Drum volume");    break;
	case pIdPolyphony:     strcpy(label,"Polyphony");      break;
	case pIdOutputGain:    strcpy(label,"Output gain");    break;
	default:               strcpy(label,"");
	}
} 



void TNDrums::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdNoteMapping:   strcpy(text,pNoteNames[pNoteMappingInt]); break;
	case pIdToneDuration:  float2string(500.0f*pToneDuration[Program][pNoteMappingInt],text,5); break;
	case pIdTonePitch:     sprintf(text,"%5.1f",FloatToHz(pTonePitch[Program][pNoteMappingInt],TONE_MAX_PITCH)); break;
	case pIdToneSlide:     float2string(-1.0f+pToneSlide[Program][pNoteMappingInt]*2.0f,text,6); break;
	case pIdToneType:      strcpy(text,pToneType[Program][pNoteMappingInt]<.5f?"Square/Pulse":"Pin"); break;
	case pIdToneWidth:
		if(pToneType[Program][pNoteMappingInt]<.5f)
		{
			float2string(100.0f*pToneWidth[Program][pNoteMappingInt],text,5);
		}
		else
		{
			float2string(pToneWidth[Program][pNoteMappingInt]*PULSE_WIDTH_MAX_US,text,6);
		}
		break;
	case pIdNoiseDuration: float2string(500.0f*pNoiseDuration[Program][pNoteMappingInt],text,5); break;
	case pIdNoisePitch:    sprintf(text,"%5.1f",(pNoisePitch[Program][pNoteMappingInt]*NOISE_MAX_PITCH/2.0f)); break;
	case pIdNoiseSlide:    float2string(-1.0f+pNoiseSlide[Program][pNoteMappingInt]*2.0f,text,6); break;
	case pIdNoisePeriod:   sprintf(text,"%i",(FloatToNoisePeriod(pNoisePeriod[Program][pNoteMappingInt])*32)); break;
	case pIdMixMode:	   strcpy(text,pMixModeNames[(int)(pMixMode[Program][pNoteMappingInt]*2.99f)]); break;
	case pIdDrumVolume:    dB2string(pDrumVolume[Program][pNoteMappingInt],text,3); break;
	case pIdPolyphony:	   strcpy(text,pPolyphony[Program]<.5f?"Disabled":"Enabled"); break;
	case pIdOutputGain:    dB2string(pOutputGain[Program],text,3); break;
	default:			   strcpy(text,"");
	}
} 



void TNDrums::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{   
	case pIdToneDuration:
	case pIdNoiseDuration: strcpy(label,"ms"); break;
	case pIdTonePitch:
	case pIdNoisePitch:    strcpy(label,"Hz"); break;
	case pIdNoisePeriod:   strcpy(label,"samples"); break;
	case pIdToneWidth:	   strcpy(label,(pToneType[Program][pNoteMappingInt]<.5f)?"%":"us"); break;
	case pIdDrumVolume:
	case pIdOutputGain:    strcpy(label,"dB"); break;
	default:               strcpy(label,""); break;
	}
} 



VstInt32 TNDrums::getProgram(void)
{
	return Program;;
}



void TNDrums::setProgram(VstInt32 program)
{
	Program=program;
}



void TNDrums::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void TNDrums::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 TNDrums::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 TNDrums::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 TNDrums::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 TNDrums::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 TNDrums::LoadStringChunk(char *str,int max_length,float *src)
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



VstInt32 TNDrums::SavePresetChunk(float *chunk)
{
	int ptr,pgm,note;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		for(note=0;note<SYNTH_NOTES;++note)
		{
			chunk[ptr++]=NOTE; chunk[ptr++]=(float)note;

			chunk[ptr++]=TDUR; chunk[ptr++]=pToneDuration [pgm][note];
			chunk[ptr++]=TPIT; chunk[ptr++]=pTonePitch    [pgm][note];
			chunk[ptr++]=TSLD; chunk[ptr++]=pToneSlide    [pgm][note];
			chunk[ptr++]=TTYP; chunk[ptr++]=pToneType     [pgm][note];
			chunk[ptr++]=TWDT; chunk[ptr++]=pToneWidth    [pgm][note];

			chunk[ptr++]=NDUR; chunk[ptr++]=pNoiseDuration[pgm][note];
			chunk[ptr++]=NPIT; chunk[ptr++]=pNoisePitch   [pgm][note];
			chunk[ptr++]=NSLD; chunk[ptr++]=pNoiseSlide   [pgm][note];
			chunk[ptr++]=NPRD; chunk[ptr++]=pNoisePeriod  [pgm][note];

			chunk[ptr++]=MODE; chunk[ptr++]=pMixMode      [pgm][note];
			chunk[ptr++]=VOLU; chunk[ptr++]=pDrumVolume   [pgm][note];
		}

		//chunk[ptr++]=NMAP; chunk[ptr++]=pNoteMapping[pgm];
		chunk[ptr++]=POLY; chunk[ptr++]=pPolyphony  [pgm];
		chunk[ptr++]=GAIN; chunk[ptr++]=pOutputGain [pgm];
	}

	chunk[ptr++]=DONE;

/*
	FILE *file=fopen("g:/params.txt","wt");

	fprintf(file,"DATA,\n\n");

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		fprintf(file,"PROG,%i,\n",pgm);

		for(note=0;note<SYNTH_NOTES;++note)
		{
			fprintf(file,"NOTE,%i,",note);

			fprintf(file,"TDUR,%1.3ff,",pToneDuration [pgm][note]);
			fprintf(file,"TPIT,%1.3ff,",pTonePitch    [pgm][note]);
			fprintf(file,"TSLD,%1.3ff,",pToneSlide    [pgm][note]);
			fprintf(file,"TTYP,%1.3ff,",pToneType     [pgm][note]);
			fprintf(file,"TWDT,%1.3ff,",pToneWidth    [pgm][note]);

			fprintf(file,"NDUR,%1.3ff,",pNoiseDuration[pgm][note]);
			fprintf(file,"NPIT,%1.3ff,",pNoisePitch   [pgm][note]);
			fprintf(file,"NSLD,%1.3ff,",pNoiseSlide   [pgm][note]);
			fprintf(file,"NPRD,%1.3ff,",pNoisePeriod  [pgm][note]);

			fprintf(file,"MODE,%1.3ff,",pMixMode      [pgm][note]);
			fprintf(file,"VOLU,%1.3ff,",pDrumVolume   [pgm][note]);
			fprintf(file,"\n");
		}

		//fprintf(file,"NMAP,%1.3ff,",pNoteMapping[pgm]);
		fprintf(file,"POLY,%1.3ff,",pPolyphony  [pgm]);
		fprintf(file,"GAIN,%1.3ff,",pOutputGain [pgm]);
		fprintf(file,"\n\n");
	}

	fprintf(file,"DONE");
	fclose(file);
*/

	return ptr*sizeof(float);//size in bytes
}



void TNDrums::LoadPresetChunk(float *chunk)
{
	int pgm,note;
	float tag;

	//default parameters for legacy data support

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		for(note=0;note<SYNTH_NOTES;++note)
		{
			pToneType [pgm][note]=0;
			pToneWidth[pgm][note]=0.5f;
		}
	}

	if(chunk[0]!=DATA)	//load legacy data
	{
		for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
		{
			for(note=0;note<SYNTH_NOTES;++note)
			{
				pToneDuration [pgm][note]=*chunk++;
				pTonePitch    [pgm][note]=*chunk++;
				pToneSlide    [pgm][note]=*chunk++;

				pNoiseDuration[pgm][note]=*chunk++;
				pNoisePitch   [pgm][note]=*chunk++;
				pNoiseSlide   [pgm][note]=*chunk++;
				pNoisePeriod  [pgm][note]=*chunk++;

				pMixMode      [pgm][note]=*chunk++;
				pDrumVolume   [pgm][note]=*chunk++;
			}

			pNoteMapping[pgm]=*chunk++;
			pPolyphony  [pgm]=*chunk++;
			pOutputGain [pgm]=*chunk++;
		}
	}
	else	//load normal data
	{
		while(1)
		{
			tag=*chunk++;

			if(tag==DONE) break;

			if(tag==PROG) pgm =(int)*chunk++;
			if(tag==NOTE) note=(int)*chunk++;

			if(tag==NAME) chunk+=LoadStringChunk(ProgramName[pgm],MAX_NAME_LEN,chunk);

			if(tag==TDUR) pToneDuration [pgm][note]=*chunk++;
			if(tag==TPIT) pTonePitch    [pgm][note]=*chunk++;
			if(tag==TSLD) pToneSlide    [pgm][note]=*chunk++;
			if(tag==TTYP) pToneType     [pgm][note]=*chunk++;
			if(tag==TWDT) pToneWidth    [pgm][note]=*chunk++;

			if(tag==NDUR) pNoiseDuration[pgm][note]=*chunk++;
			if(tag==NPIT) pNoisePitch   [pgm][note]=*chunk++;
			if(tag==NSLD) pNoiseSlide   [pgm][note]=*chunk++;
			if(tag==NPRD) pNoisePeriod  [pgm][note]=*chunk++;

			if(tag==MODE) pMixMode      [pgm][note]=*chunk++;
			if(tag==VOLU) pDrumVolume   [pgm][note]=*chunk++;

			//if(tag==NMAP) pNoteMapping[pgm]=*chunk++;
			if(tag==POLY) pPolyphony  [pgm]=*chunk++;
			if(tag==GAIN) pOutputGain [pgm]=*chunk++;
		}
	}

	pNoteMappingInt=(int)(pNoteMapping[Program]*11.99f);
}



VstInt32 TNDrums::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 TNDrums::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	return 0;
}



void TNDrums::MidiAddNewNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type=mEventTypeNote;
	entry.delta=delta;
	entry.note=note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void TNDrums::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



VstInt32 TNDrums::processEvents(VstEvents* ev) 
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



void TNDrums::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float level,sampleRate;
	unsigned int i,chn,out,note;

	sampleRate=(float)updateSampleRate();

	while(--sampleFrames>=0)
	{
		for(i=0;i<MidiQueue.size();++i)
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

							if(pPolyphony[Program]<.5f)
							{
								chn=0;
							}
							else
							{
								switch(note)
								{
								case 0:  chn=0; break;//C
								case 1:  chn=7; break;//C#
								case 2:  chn=1; break;//D
								case 3:  chn=7; break;//D#
								case 4:  chn=2; break;//E
								case 5:  chn=3; break;//F
								case 6:  chn=7; break;//F#
								case 7:  chn=4; break;//G
								case 8:  chn=7; break;//G#
								case 9:  chn=5; break;//A
								case 10: chn=7; break;//A#
								case 11: chn=6; break;//B
								}
							}

							if(MidiQueue[i].velocity)//key on
							{
								SynthChannel[chn].toneAccumulator=0;
								SynthChannel[chn].toneDuration=pToneDuration[Program][note]*sampleRate*.5f;
								SynthChannel[chn].toneIncrement=FloatToHz(pTonePitch[Program][note],TONE_MAX_PITCH)/sampleRate;
								SynthChannel[chn].toneIncrementDelta=(-1.0f+pToneSlide[Program][note]*2.0f)*(48000.0f/sampleRate)*(.25f/sampleRate);

								SynthChannel[chn].noiseAccumulator=0;
								SynthChannel[chn].noiseDuration=pNoiseDuration[Program][note]*sampleRate*.5f;
								SynthChannel[chn].noiseIncrement=pNoisePitch[Program][note]*NOISE_MAX_PITCH/sampleRate;
								SynthChannel[chn].noiseIncrementDelta=(-1.0f+pNoiseSlide[Program][note]*2.0f)*(48000.0f/sampleRate)*(2.0f/sampleRate);
								SynthChannel[chn].noisePtr=0;
								SynthChannel[chn].noisePeriodMask=FloatToNoisePeriod(pNoisePeriod[Program][note])-1;

								SynthChannel[chn].mixMode=(int)(pMixMode[Program][note]*2.99f);
								SynthChannel[chn].volume=pDrumVolume[Program][note]*((float)MidiQueue[i].velocity/100.0f);

								SynthChannel[chn].tonePulse =0;
								SynthChannel[chn].toneType  =pToneType[Program][note];
								SynthChannel[chn].toneWidth =pToneWidth[Program][note];

								SynthChannel[chn].toneOutput=0;
								SynthChannel[chn].noiseOutput=0;
							}
							/*else//key off
							{
								SynthChannel[chn].toneDuration=0;
								SynthChannel[chn].noiseDuration=0;
							}*/
						}
						break;

					case mEventTypeProgram:
						{
							Program=MidiQueue[i].program;
							updateDisplay();
						}
						break;
					}
				}
			}
		}

		level=0;

		for(chn=0;chn<SYNTH_CHANNELS;++chn)
		{
			if(SynthChannel[chn].toneDuration>0)
			{
				SynthChannel[chn].toneDuration-=1.0f;

				SynthChannel[chn].toneAccumulator+=SynthChannel[chn].toneIncrement;
				SynthChannel[chn].toneIncrement  +=SynthChannel[chn].toneIncrementDelta;

				if(SynthChannel[chn].toneType<.5f)	//pulse or square with variable duty cycle
				{
					while(SynthChannel[chn].toneAccumulator>=2.0f)
					{
						SynthChannel[chn].toneAccumulator-=2.0f;
					}

					SynthChannel[chn].toneOutput=SynthChannel[chn].toneAccumulator<SynthChannel[chn].toneWidth*2.0f?0:1;
				}
				else	//pin
				{
					while(SynthChannel[chn].toneAccumulator>=2.0f)
					{
						SynthChannel[chn].toneAccumulator-=2.0f;

						SynthChannel[chn].tonePulse+=sampleRate*PULSE_WIDTH_MAX_US*SynthChannel[chn].toneWidth/1000000.0f;
					}

					if(SynthChannel[chn].tonePulse>0)
					{
						SynthChannel[chn].tonePulse-=1.0f;
						SynthChannel[chn].toneOutput=1;
					}
					else
					{
						SynthChannel[chn].toneOutput=0;
					}
				}

				if(SynthChannel[chn].toneIncrement<0.0f) SynthChannel[chn].toneIncrement=0.0f;
				if(SynthChannel[chn].toneIncrement>1.0f) SynthChannel[chn].toneIncrement=1.0f;
			}
			else
			{
				SynthChannel[chn].toneOutput=0;
			}

			if(SynthChannel[chn].noiseDuration>0)
			{
				SynthChannel[chn].noiseDuration-=1.0f;

				SynthChannel[chn].noiseAccumulator+=SynthChannel[chn].noiseIncrement;
				SynthChannel[chn].noiseIncrement  +=SynthChannel[chn].noiseIncrementDelta;

				while(SynthChannel[chn].noiseAccumulator>=1.0f)
				{
					SynthChannel[chn].noiseAccumulator-=1.0f;

					SynthChannel[chn].noiseOutput=(Noise[(SynthChannel[chn].noisePtr>>5)&SynthChannel[chn].noisePeriodMask]>>(SynthChannel[chn].noisePtr&31))&1;

					++SynthChannel[chn].noisePtr;
				}

				if(SynthChannel[chn].noiseIncrement<0.0f) SynthChannel[chn].noiseIncrement=0.0f;
				if(SynthChannel[chn].noiseIncrement>1.0f) SynthChannel[chn].noiseIncrement=1.0f;
			}
			else
			{
				SynthChannel[chn].noiseOutput=0;
			}

			switch(SynthChannel[chn].mixMode)
			{
			case 0:  out=SynthChannel[chn].toneOutput|SynthChannel[chn].noiseOutput; break;
			case 1:  out=SynthChannel[chn].toneOutput&SynthChannel[chn].noiseOutput; break;
			case 2:  out=SynthChannel[chn].toneOutput^SynthChannel[chn].noiseOutput; break;
			default: out=0;
			}

			if(out) level+=SynthChannel[chn].volume;
		}

		level/=3.0f;//it is unusual to have more than three drum channels playing at once

		level*=pOutputGain[Program];

		if(level>1.0f) level=1.0f;

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
