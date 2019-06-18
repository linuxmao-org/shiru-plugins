#include "NoiseDrums.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new NoiseDrums(audioMaster);
}



NoiseDrums::NoiseDrums(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
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

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		strcpy(ProgramName[pgm],programDefaultNames[pgm]);
/*
		pNoteMapping[pgm]=0;
		pPolyphony  [pgm]=1.0f;
		pOutputGain [pgm]=1.0f;

		for(i=0;i<SYNTH_NOTES;++i)
		{
			pBurstNumber[pgm][i]=0;
			pDrumVolume [pgm][i]=1.0f;

			for(j=0;j<NOISE_MAX_BURSTS;++j)
			{
				pDuration  [pgm][i][j]=j?0:.1f;
				pPitch     [pgm][i][j]=.5f;
				pPeriod    [pgm][i][j]=1.0f;
				pPulseWidth[pgm][i][j]=1.0f;
			}
		}
*/
	}

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].burst      =NOISE_MAX_BURSTS;
		SynthChannel[chn].note       =0;
		SynthChannel[chn].duration   =0;
		SynthChannel[chn].accumulator=0;
		SynthChannel[chn].increment  =0;
		SynthChannel[chn].period     =0;
		SynthChannel[chn].ptr        =0;
		SynthChannel[chn].volume     =1.0f;
		SynthChannel[chn].output     =0;
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



NoiseDrums::~NoiseDrums()
{
}



float NoiseDrums::FloatToHz(float value,float range)
{
	float hz;

	hz=-range*log10(1.0f-value)/3.0f;

	if(hz<0) hz=0;
	if(hz>range) hz=range;

	return hz;
}



int NoiseDrums::FloatToNoisePeriod(float value)
{
	const int period[]={4,8,16,32,64,128,256,512,1024,2048};

	return period[(int)(value*9.99f)];
}



void NoiseDrums::setParameter(VstInt32 index,float value)
{
	int nnote,nnumber;

	switch(index)
	{
	case pIdNoteMapping: pNoteMapping[Program]=value; break;
	case pIdBurstNumber: pBurstNumber[Program][pNoteMappingInt]=value; break;
	case pIdDuration:    pDuration   [Program][pNoteMappingInt][pBurstNumberInt]=value; break;
	case pIdPitch:       pPitch      [Program][pNoteMappingInt][pBurstNumberInt]=value; break;
	case pIdPeriod:      pPeriod     [Program][pNoteMappingInt][pBurstNumberInt]=value; break;
	case pIdPulseWidth:  pPulseWidth [Program][pNoteMappingInt][pBurstNumberInt]=value; break;
	case pIdDrumVolume:  pDrumVolume [Program][pNoteMappingInt]=value; break;
	case pIdPolyphony:   pPolyphony  [Program]=value; break;
	case pIdOutputGain:  pOutputGain [Program]=value; break;
	}

	nnote  =(int)(pNoteMapping[Program]*(SYNTH_NOTES-.1f));
	nnumber=(int)(pBurstNumber[Program][pNoteMappingInt]*(NOISE_MAX_BURSTS-.1f));

	if(pNoteMappingInt!=nnote||pBurstNumberInt!=nnumber)
	{
		pNoteMappingInt=nnote;
		pBurstNumberInt=nnumber;

		updateDisplay();
	}
}



float NoiseDrums::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdNoteMapping: return pNoteMapping[Program];
	case pIdBurstNumber: return pBurstNumber[Program][pNoteMappingInt];
	case pIdDuration:    return pDuration   [Program][pNoteMappingInt][pBurstNumberInt];
	case pIdPitch:       return pPitch      [Program][pNoteMappingInt][pBurstNumberInt];
	case pIdPeriod:      return pPeriod     [Program][pNoteMappingInt][pBurstNumberInt];
	case pIdPulseWidth:  return pPulseWidth [Program][pNoteMappingInt][pBurstNumberInt];
	case pIdDrumVolume:	 return pDrumVolume [Program][pNoteMappingInt];
	case pIdPolyphony:   return pPolyphony  [Program];
	case pIdOutputGain:  return pOutputGain [Program];
	}

	return 0;
} 



void NoiseDrums::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdNoteMapping: strcpy(label,"Mapped note");  break;
	case pIdBurstNumber: strcpy(label,"Burst number"); break;
	case pIdDuration:    strcpy(label,"Duration");     break;
	case pIdPitch:       strcpy(label,"Pitch");        break;
	case pIdPeriod:      strcpy(label,"Period");       break;
	case pIdPulseWidth:  strcpy(label,"Pulse width");  break;
	case pIdDrumVolume:  strcpy(label,"Drum volume");  break;
	case pIdPolyphony:   strcpy(label,"Polyphony");    break;
	case pIdOutputGain:  strcpy(label,"Output gain");  break;
	default:			 strcpy(label,"");
	}
} 



void NoiseDrums::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdNoteMapping: strcpy(text,pNoteNames[pNoteMappingInt]); break;
	case pIdBurstNumber: sprintf(text,"%i",((int)(pBurstNumber[Program][pNoteMappingInt]*(NOISE_MAX_BURSTS-.1f)))); break;
	case pIdDuration:    float2string(NOISE_MAX_DURATION*pDuration[Program][pNoteMappingInt][pBurstNumberInt],text,5); break;
	case pIdPitch:       float2string(pPitch[Program][pNoteMappingInt][pBurstNumberInt]*NOISE_MAX_PITCH/2.0f,text,6); break;
	case pIdPeriod:      sprintf(text,"%i",FloatToNoisePeriod(pPeriod[Program][pNoteMappingInt][pBurstNumberInt])*32); break;
	case pIdPulseWidth:	 float2string(pPulseWidth[Program][pNoteMappingInt][pBurstNumberInt],text,5); break;
	case pIdDrumVolume:  dB2string(pDrumVolume[Program][pNoteMappingInt],text,3); break;
	case pIdPolyphony:	 strcpy(text,pPolyphony[Program]<.5f?"Disabled":"Enabled"); break;
	case pIdOutputGain:  dB2string(pOutputGain[Program],text,3); break;
	default:			 strcpy(text,"");
	}
} 



void NoiseDrums::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdDuration:    strcpy(label,"ms"); break;
	case pIdPitch:       strcpy(label,"Hz"); break;
	case pIdPeriod:      strcpy(label,"samples"); break;
	case pIdDrumVolume:
	case pIdOutputGain:  strcpy(label,"dB"); break;
	default:			 strcpy(label,"");
	}
} 



VstInt32 NoiseDrums::getProgram(void)
{
	return Program;;
}



void NoiseDrums::setProgram(VstInt32 program)
{
	Program=program;
}



void NoiseDrums::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void NoiseDrums::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 NoiseDrums::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 NoiseDrums::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 NoiseDrums::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 NoiseDrums::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 NoiseDrums::LoadStringChunk(char *str,int max_length,float *src)
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



VstInt32 NoiseDrums::SavePresetChunk(float *chunk)
{
	int ptr,pgm,note,burst;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		//chunk[ptr++]=NMAP; chunk[ptr++]=pNoteMapping[pgm];
		chunk[ptr++]=POLY; chunk[ptr++]=pPolyphony  [pgm];
		chunk[ptr++]=GAIN; chunk[ptr++]=pOutputGain [pgm];

		for(note=0;note<SYNTH_NOTES;++note)
		{
			chunk[ptr++]=NOTE; chunk[ptr++]=(float)note;
			//chunk[ptr++]=BNUM; chunk[ptr++]=pBurstNumber[pgm][note];
			chunk[ptr++]=VOLU; chunk[ptr++]=pDrumVolume [pgm][note];

			for(burst=0;burst<NOISE_MAX_BURSTS;++burst)
			{
				chunk[ptr++]=BRST; chunk[ptr++]=(float)burst;

				chunk[ptr++]=DURA; chunk[ptr++]=pDuration  [pgm][note][burst];
				chunk[ptr++]=PTCH; chunk[ptr++]=pPitch     [pgm][note][burst];
				chunk[ptr++]=PERD; chunk[ptr++]=pPeriod    [pgm][note][burst];
				chunk[ptr++]=PWDT; chunk[ptr++]=pPulseWidth[pgm][note][burst];
			}
		}
	}

	chunk[ptr++]=DONE;

/*
	FILE *file=fopen("g:/params.txt","wt");

	fprintf(file,"DATA,\n\n");

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		fprintf(file,"PROG,%i,\n",pgm);

		//fprintf(file,"NMAP,%1.3ff,",pNoteMapping[pgm]);
		fprintf(file,"POLY,%1.3ff,",pPolyphony  [pgm]);
		fprintf(file,"GAIN,%1.3ff,",pOutputGain [pgm]);
		fprintf(file,"\n");

		for(note=0;note<SYNTH_NOTES;++note)
		{
			fprintf(file,"NOTE,%i,",note);
			//fprintf(file,"BNUM,%i,",pBurstNumber[pgm][note]);
			fprintf(file,"VOLU,%1.3ff,",pDrumVolume [pgm][note]);
			fprintf(file,"\n");

			for(burst=0;burst<NOISE_MAX_BURSTS;++burst)
			{
				fprintf(file,"BRST,%i,",burst);
				fprintf(file,"DURA,%1.3ff,",pDuration  [pgm][note][burst]);
				fprintf(file,"PTCH,%1.3ff,",pPitch     [pgm][note][burst]);
				fprintf(file,"PERD,%1.3ff,",pPeriod    [pgm][note][burst]);
				fprintf(file,"PWDT,%1.3ff,",pPulseWidth[pgm][note][burst]);
				fprintf(file,"\n");
			}

			fprintf(file,"\n");
		}
	}

	fprintf(file,"DONE");
	fclose(file);
*/
	return ptr*sizeof(float);//size in bytes
}



void NoiseDrums::LoadPresetChunk(float *chunk)
{
	int burst,pgm,note;
	float tag;

	if(chunk[0]!=DATA)	//load legacy data
	{
		for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
		{
			pNoteMapping[pgm]=*chunk++;
			pPolyphony  [pgm]=*chunk++;
			pOutputGain [pgm]=*chunk++;

			for(note=0;note<SYNTH_NOTES;++note)
			{
				pBurstNumber[pgm][note]=*chunk++;
				pDrumVolume [pgm][note]=*chunk++;

				for(burst=0;burst<NOISE_MAX_BURSTS;++burst)
				{
					pDuration  [pgm][note][burst]=*chunk++;
					pPitch     [pgm][note][burst]=*chunk++;
					pPeriod    [pgm][note][burst]=*chunk++;
					pPulseWidth[pgm][note][burst]=*chunk++;
				}
			}
		}
	}
	else	//load normal data
	{
		while(1)
		{
			tag=*chunk++;

			if(tag==DONE) break;

			if(tag==PROG) pgm  =(int)*chunk++;
			if(tag==NOTE) note =(int)*chunk++;
			if(tag==BRST) burst=(int)*chunk++;

			if(tag==NAME) chunk+=LoadStringChunk(ProgramName[pgm],MAX_NAME_LEN,chunk);

			//if(tag==NMAP) pNoteMapping[pgm]=*chunk++;
			if(tag==POLY) pPolyphony  [pgm]=*chunk++;
			if(tag==GAIN) pOutputGain [pgm]=*chunk++;

			//if(tag==BNUM) pBurstNumber[pgm][note]=*chunk++;
			if(tag==VOLU) pDrumVolume [pgm][note]=*chunk++;

			if(tag==DURA) pDuration  [pgm][note][burst]=*chunk++;
			if(tag==PTCH) pPitch     [pgm][note][burst]=*chunk++;
			if(tag==PERD) pPeriod    [pgm][note][burst]=*chunk++;
			if(tag==PWDT) pPulseWidth[pgm][note][burst]=*chunk++;
		}
	}

	pNoteMappingInt=(int)(pNoteMapping[Program]*(SYNTH_NOTES-.1f));
	pBurstNumberInt=(int)(pBurstNumber[Program][pNoteMappingInt]*(NOISE_MAX_BURSTS-.1f));
}



VstInt32 NoiseDrums::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 NoiseDrums::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	return 0;
}



void NoiseDrums::MidiAddNewNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type=mEventTypeNote;
	entry.delta=delta;
	entry.note=note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void NoiseDrums::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



VstInt32 NoiseDrums::processEvents(VstEvents* ev) 
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



void NoiseDrums::SynthNewNoiseBurst(VstInt32 chn)
{
	float sampleRate;

	sampleRate=(float)updateSampleRate();

	SynthChannel[chn].duration =pDuration[Program][SynthChannel[chn].note][SynthChannel[chn].burst]*NOISE_MAX_DURATION*(sampleRate/1000.0f);
	SynthChannel[chn].increment=pPitch   [Program][SynthChannel[chn].note][SynthChannel[chn].burst]*NOISE_MAX_PITCH/sampleRate;

	SynthChannel[chn].period=FloatToNoisePeriod(pPeriod[Program][SynthChannel[chn].note][SynthChannel[chn].burst])-1;
}



void NoiseDrums::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float level,sampleRate;
	unsigned int i,s,chn,note;

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
								SynthChannel[chn].burst=0;
								SynthChannel[chn].note=note;
								SynthChannel[chn].accumulator=0;
								SynthChannel[chn].ptr=0;
								SynthChannel[chn].volume=pDrumVolume[Program][note]*((float)MidiQueue[i].velocity/100.0f);

								SynthNewNoiseBurst(chn);
							}
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

		for(s=0;s<OVERSAMPLING;++s)
		{
			for(chn=0;chn<SYNTH_CHANNELS;++chn)
			{
				if(SynthChannel[chn].burst>=NOISE_MAX_BURSTS) continue;

				SynthChannel[chn].duration-=1.0f/OVERSAMPLING;

				SynthChannel[chn].accumulator+=SynthChannel[chn].increment/OVERSAMPLING;

				while(SynthChannel[chn].accumulator>=1.0f)
				{
					SynthChannel[chn].accumulator-=1.0f;

					SynthChannel[chn].output=(Noise[(SynthChannel[chn].ptr>>5)&SynthChannel[chn].period]>>(SynthChannel[chn].ptr&31))&1;

					++SynthChannel[chn].ptr;
				}

				if(SynthChannel[chn].accumulator>pPulseWidth[Program][SynthChannel[chn].note][SynthChannel[chn].burst]) SynthChannel[chn].output=0;

				if(SynthChannel[chn].output) level+=SynthChannel[chn].volume/OVERSAMPLING;

				if(SynthChannel[chn].duration<=0)
				{
					if(SynthChannel[chn].burst<NOISE_MAX_BURSTS)
					{
						++SynthChannel[chn].burst;

						SynthNewNoiseBurst(chn);
					}
					else
					{
						SynthChannel[chn].output=0;
					}
				}
			}
		}

		level/=3.0f;//it is unusual to have more than three drum channels playing at once

		level*=pOutputGain[Program];

		if(level>1.0f) level=1.0f;

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
