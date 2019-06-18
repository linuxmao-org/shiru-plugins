#include "ClickyDrums.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new ClickyDrums(audioMaster);
}



ClickyDrums::ClickyDrums(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 i,chn,pgm;

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
		SynthChannel[chn].duration=0;
		SynthChannel[chn].delay=0;
		SynthChannel[chn].output=0;
		SynthChannel[chn].accumulator=0;
		SynthChannel[chn].ptr=0;
		SynthChannel[chn].type=0;
		SynthChannel[chn].volume=1.0f;
	}

	srand(1);

	for(i=0;i<16384;++i) Noise[i]=rand()&255;

	MidiQueue.clear();

	suspend();
}



ClickyDrums::~ClickyDrums()
{
}



void ClickyDrums::setParameter(VstInt32 index,float value)
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

	case pIdDuration:   pDuration  [Program][pNoteMappingInt]=value; break;
	case pIdMin:        pMin       [Program][pNoteMappingInt]=value; break;
	case pIdMax:	    pMax       [Program][pNoteMappingInt]=value; break;
	case pIdSeed:       pSeed      [Program][pNoteMappingInt]=value; break;
	case pIdType:       pType      [Program][pNoteMappingInt]=value; break;
	case pIdDrumVolume: pDrumVolume[Program][pNoteMappingInt]=value; break;
	case pIdPolyphony:  pPolyphony [Program]=value; break;
	case pIdOutputGain: pOutputGain[Program]=value; break;
	}
}



float ClickyDrums::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdNoteMapping: return pNoteMapping[Program];
	case pIdDuration:    return pDuration   [Program][pNoteMappingInt];
	case pIdMin:         return pMin        [Program][pNoteMappingInt];
	case pIdMax:         return pMax        [Program][pNoteMappingInt];
	case pIdSeed:        return pSeed       [Program][pNoteMappingInt];
	case pIdType:        return pType       [Program][pNoteMappingInt];
	case pIdDrumVolume:	 return pDrumVolume [Program][pNoteMappingInt];
	case pIdPolyphony:   return pPolyphony  [Program];
	case pIdOutputGain:  return pOutputGain [Program];
	}

	return 0;
} 



void ClickyDrums::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdNoteMapping: strcpy(label,"Mapped note"); break;
	case pIdDuration:    strcpy(label,"Duration");    break;
	case pIdMin:         strcpy(label,"Pulse min");   break;
	case pIdMax:         strcpy(label,"Pulse max");   break;
	case pIdSeed:        strcpy(label,"Rand seed");   break;
	case pIdType:        strcpy(label,"Sound type");  break;
	case pIdDrumVolume:  strcpy(label,"Drum volume"); break;
	case pIdPolyphony:   strcpy(label,"Polyphony");   break;
	case pIdOutputGain:  strcpy(label,"Output gain"); break;
	default:             strcpy(label,"");
	}
} 



void ClickyDrums::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdNoteMapping: strcpy(text,pNoteNames[pNoteMappingInt]); break;
	case pIdDuration:    float2string(250.0f*pDuration[Program][pNoteMappingInt],text,4); break;
	case pIdMin:         float2string(pMin[Program][pNoteMappingInt],text,5); break;
	case pIdMax:         float2string(pMax[Program][pNoteMappingInt],text,5); break;
	case pIdSeed:        sprintf(text,"%i",(int)(16383.0f*pSeed[Program][pNoteMappingInt])); break;
	case pIdType:	     strcpy(text,pType[Program][pNoteMappingInt]<.5f?"Clicky":"Squeaky"); break;
	case pIdDrumVolume:  dB2string(pDrumVolume[Program][pNoteMappingInt],text,3); break;
	case pIdPolyphony:	 strcpy(text,pPolyphony[Program]<.5f?"Disabled":"Enabled"); break;
	case pIdOutputGain:  dB2string(pOutputGain[Program],text,3); break;
	default:			 strcpy(text,"");
	}
} 



void ClickyDrums::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdDuration:    strcpy(label,"ms"); break;
	case pIdDrumVolume:
	case pIdOutputGain:  strcpy(label,"dB"); break;
	default:			 strcpy(label,"");
	}
} 



VstInt32 ClickyDrums::getProgram(void)
{
	return Program;
}



void ClickyDrums::setProgram(VstInt32 program)
{
	Program=program;
}



void ClickyDrums::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void ClickyDrums::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 ClickyDrums::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 ClickyDrums::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 ClickyDrums::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 ClickyDrums::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 ClickyDrums::LoadStringChunk(char *str,int max_length,float *src)
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



VstInt32 ClickyDrums::SavePresetChunk(float *chunk)
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

			chunk[ptr++]=DURA; chunk[ptr++]=pDuration  [pgm][note];
			chunk[ptr++]=PMIN; chunk[ptr++]=pMin       [pgm][note];
			chunk[ptr++]=PMAX; chunk[ptr++]=pMax       [pgm][note];
			chunk[ptr++]=SEED; chunk[ptr++]=pSeed      [pgm][note];
			chunk[ptr++]=TYPE; chunk[ptr++]=pType      [pgm][note];
			chunk[ptr++]=VOLU; chunk[ptr++]=pDrumVolume[pgm][note];
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
			fprintf(file,"DURA,%1.3ff,",pDuration  [pgm][note]);
			fprintf(file,"PMIN,%1.3ff,",pMin       [pgm][note]);
			fprintf(file,"PMAX,%1.3ff,",pMax       [pgm][note]);
			fprintf(file,"SEED,%1.3ff,",pSeed      [pgm][note]);
			fprintf(file,"TYPE,%1.3ff,",pType      [pgm][note]);
			fprintf(file,"VOLU,%1.3ff,",pDrumVolume[pgm][note]);
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



void ClickyDrums::LoadPresetChunk(float *chunk)
{
	int pgm,note;
	float tag;

	if(chunk[0]!=DATA)	//load legacy data
	{
		for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
		{
			for(note=0;note<SYNTH_NOTES;++note)
			{
				pDuration  [pgm][note]=*chunk++;
				pMin       [pgm][note]=*chunk++;
				pMax       [pgm][note]=*chunk++;
				pSeed      [pgm][note]=*chunk++;
				pType      [pgm][note]=*chunk++;
				pDrumVolume[pgm][note]=*chunk++;
			}

			*chunk++;//pNoteMapping[pgm]=*chunk++;	//skip
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

			if(tag==DURA) pDuration  [pgm][note]=*chunk++;
			if(tag==PMIN) pMin       [pgm][note]=*chunk++;
			if(tag==PMAX) pMax       [pgm][note]=*chunk++;
			if(tag==SEED) pSeed      [pgm][note]=*chunk++;
			if(tag==TYPE) pType      [pgm][note]=*chunk++;
			if(tag==VOLU) pDrumVolume[pgm][note]=*chunk++;

			//if(tag==NMAP) pNoteMapping[pgm]=*chunk++;
			if(tag==POLY) pPolyphony  [pgm]=*chunk++;
			if(tag==GAIN) pOutputGain [pgm]=*chunk++;
		}
	}

	pNoteMappingInt=(int)(pNoteMapping[Program]*11.99f);
}



VstInt32 ClickyDrums::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 ClickyDrums::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	return 0;
}



void ClickyDrums::MidiAddNewNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type=mEventTypeNote;
	entry.delta=delta;
	entry.note=note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void ClickyDrums::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



VstInt32 ClickyDrums::processEvents(VstEvents* ev) 
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



void ClickyDrums::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float min,max,level,sampleRate;
	unsigned int i,chn,note;

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
								if(pMin[Program][note]<pMax[Program][note])
								{
									min=pMin[Program][note];
									max=pMax[Program][note];
								}
								else
								{
									min=pMax[Program][note];
									max=pMin[Program][note];
								}

								SynthChannel[chn].duration=pDuration[Program][note]*sampleRate*.25f;
								SynthChannel[chn].accumulator=0;
								SynthChannel[chn].ptr=(int)(16383.0f*pSeed[Program][note]);
								SynthChannel[chn].volume=pDrumVolume[Program][note]*((float)MidiQueue[i].velocity/100.0f);
								SynthChannel[chn].delay=0;
								SynthChannel[chn].delay_min=256.0f*8.0f*min;
								SynthChannel[chn].delay_max=256.0f*8.0f*max;
								SynthChannel[chn].type=pType[Program][note]<.5f?0:1;
								SynthChannel[chn].output=0;
							}
							/*else//key off
							{
								SynthChannel[chn].duration=0;
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
			if(SynthChannel[chn].duration>0)
			{
				SynthChannel[chn].duration-=1.0f;

				SynthChannel[chn].accumulator+=(65536.0f*4.0f/sampleRate);

				while(SynthChannel[chn].accumulator>=1.0f)
				{
					SynthChannel[chn].accumulator-=1.0f;

					SynthChannel[chn].delay-=1.0f;

					if(SynthChannel[chn].delay<=0)
					{
						SynthChannel[chn].delay=(float)Noise[SynthChannel[chn].ptr&16383]*8.0f;

						if(!SynthChannel[chn].type)
						{
							if(SynthChannel[chn].delay<SynthChannel[chn].delay_min) SynthChannel[chn].delay=SynthChannel[chn].delay_min;
							if(SynthChannel[chn].delay>SynthChannel[chn].delay_max) SynthChannel[chn].delay=SynthChannel[chn].delay_max;
						}
						else
						{
							while(SynthChannel[chn].delay< SynthChannel[chn].delay_min&&SynthChannel[chn].delay_min>0) SynthChannel[chn].delay+=SynthChannel[chn].delay_min;
							while(SynthChannel[chn].delay>=SynthChannel[chn].delay_max&&SynthChannel[chn].delay_max>0) SynthChannel[chn].delay-=SynthChannel[chn].delay_max;	
						}

						++SynthChannel[chn].ptr;

						SynthChannel[chn].output^=1;
					}
				}
			}
			else
			{
				SynthChannel[chn].output=0;
			}

			if(SynthChannel[chn].output) level+=SynthChannel[chn].volume;
		}

		level/=3.0f;//normally there is no more than three drum channels playing at once

		level*=pOutputGain[Program];

		if(level>1.0f) level=1.0f;

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
