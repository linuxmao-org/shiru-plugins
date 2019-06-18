#include "WaveSynth.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new WaveSynth(audioMaster);
}



WaveSynth::WaveSynth(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 pgm,chn;

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
		pWaveformLength[pgm]=0;
		pWaveformNibble[pgm][0]=.1f;
		pWaveformNibble[pgm][1]=0;
		pWaveformNibble[pgm][2]=0;
		pWaveformNibble[pgm][3]=0;
		pWaveformNibble[pgm][4]=0;
		pWaveformNibble[pgm][5]=0;
		pWaveformNibble[pgm][6]=0;
		pWaveformNibble[pgm][7]=0;
		pPolyphony     [pgm]=1.0f;
		pPortaSpeed    [pgm]=1.0f;
		pNoteCut       [pgm]=0;
		pVelTarget     [pgm]=0;
		pOutputGain    [pgm]=1.0f;
*/
	}

	pWaveformLengthInt=pWaveformLengthList[0];

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].note=-1;
		SynthChannel[chn].freq=0;
		SynthChannel[chn].freq_new=0;
		SynthChannel[chn].acc=0;
		SynthChannel[chn].volume=1.0f;
	}

	sWaveformSrc=0;
	sWaveform=0;
	sWaveformShift=0;

	MidiQueue.clear();

	MidiRPNLSB=0;
	MidiRPNMSB=0;
	MidiDataLSB=0;
	MidiDataMSB=0;

	MidiModulationMSB=0;

	MidiPitchBend=0;
	MidiPitchBendRange=2.0f;
	MidiModulationDepth=0;
	MidiModulationCount=0;

	sSlideStep=0;
	sModCnt=0;

	memset(MidiKeyState,0,sizeof(MidiKeyState));

	suspend();
}



WaveSynth::~WaveSynth()
{
}



void WaveSynth::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdWaveformLength:  pWaveformLength[Program]   =value; break;
	case pIdModulation:      pModulation    [Program]   =value; break;
	case pIdWaveformNibble0: pWaveformNibble[Program][0]=value; break;
	case pIdWaveformNibble1: pWaveformNibble[Program][1]=value; break;
	case pIdWaveformNibble2: pWaveformNibble[Program][2]=value; break;
	case pIdWaveformNibble3: pWaveformNibble[Program][3]=value; break;
	case pIdWaveformNibble4: pWaveformNibble[Program][4]=value; break;
	case pIdWaveformNibble5: pWaveformNibble[Program][5]=value; break;
	case pIdWaveformNibble6: pWaveformNibble[Program][6]=value; break;
	case pIdWaveformNibble7: pWaveformNibble[Program][7]=value; break;
	case pIdPolyphony:       pPolyphony     [Program]   =value; break;
	case pIdPortaSpeed:      pPortaSpeed    [Program]   =value; break;
	case pIdNoteCut:         pNoteCut       [Program]   =value; updateDisplay(); break;
	case pIdVelTarget:       pVelTarget     [Program]   =value; break;
	case pIdOutputGain:      pOutputGain    [Program]   =value; break;
	}

	pWaveformLengthInt=pWaveformLengthList[(int)(pWaveformLength[Program]*3.99f)];
}



float WaveSynth::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdWaveformLength:  return pWaveformLength[Program];
	case pIdModulation:      return pModulation    [Program];
	case pIdWaveformNibble0: return pWaveformNibble[Program][0];
	case pIdWaveformNibble1: return pWaveformNibble[Program][1];
	case pIdWaveformNibble2: return pWaveformNibble[Program][2];
	case pIdWaveformNibble3: return pWaveformNibble[Program][3];
	case pIdWaveformNibble4: return pWaveformNibble[Program][4];
	case pIdWaveformNibble5: return pWaveformNibble[Program][5];
	case pIdWaveformNibble6: return pWaveformNibble[Program][6];
	case pIdWaveformNibble7: return pWaveformNibble[Program][7];
	case pIdPolyphony:       return pPolyphony     [Program];
	case pIdPortaSpeed:      return pPortaSpeed    [Program];
	case pIdNoteCut:         return pNoteCut       [Program];
	case pIdVelTarget:       return pVelTarget     [Program];
	case pIdOutputGain:      return pOutputGain    [Program];
	}

	return 0;
} 



void WaveSynth::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdWaveformLength:  strcpy(label,"Waveform length");   break;
	case pIdModulation:      strcpy(label,"Shift speed");       break;
	case pIdWaveformNibble0: strcpy(label,"Nibble #0 (left)");  break;
	case pIdWaveformNibble1: strcpy(label,"Nibble #1");         break;
	case pIdWaveformNibble2: strcpy(label,"Nibble #2");         break;
	case pIdWaveformNibble3: strcpy(label,"Nibble #3");         break;
	case pIdWaveformNibble4: strcpy(label,"Nibble #4");         break;
	case pIdWaveformNibble5: strcpy(label,"Nibble #5");         break;
	case pIdWaveformNibble6: strcpy(label,"Nibble #6");         break;
	case pIdWaveformNibble7: strcpy(label,"Nibble #7 (right)"); break;
	case pIdPolyphony:       strcpy(label,"Polyphony");         break;
	case pIdPortaSpeed:      strcpy(label,"Porta speed");       break;
	case pIdNoteCut:         strcpy(label,"Note cut");          break;
	case pIdVelTarget:       strcpy(label,"Velocity target");   break;
	case pIdOutputGain:      strcpy(label,"Output gain");       break;
	default:				 strcpy(label,"");
	}
} 



void WaveSynth::NibbleToString(VstInt32 nibble,char *text)
{
	strcpy(text,pWaveformBitPatterns[(int)(pWaveformNibble[Program][nibble]*15.99f)]);
}



void WaveSynth::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdWaveformLength:  sprintf(text,"%i",pWaveformLengthInt); break;
	case pIdModulation:      float2string(pModulation[Program]*MOD_MAX_RATE,text,5); break;
	case pIdWaveformNibble0: NibbleToString(0,text); break;
	case pIdWaveformNibble1: NibbleToString(1,text); break;
	case pIdWaveformNibble2: NibbleToString(2,text); break;
	case pIdWaveformNibble3: NibbleToString(3,text); break;
	case pIdWaveformNibble4: NibbleToString(4,text); break;
	case pIdWaveformNibble5: NibbleToString(5,text); break;
	case pIdWaveformNibble6: NibbleToString(6,text); break;
	case pIdWaveformNibble7: NibbleToString(7,text); break;
	case pIdPolyphony:       strcpy(text,pPolyMixModeNames[(int)(pPolyphony[Program]*3.99f)]); break;
	case pIdPortaSpeed:      float2string(pPortaSpeed[Program],text,5); break;
	case pIdNoteCut:         if(pNoteCut[Program]==0) strcpy(text,"Infinite"); else float2string(1000.0f*pNoteCut[Program],text,5); break;
	case pIdVelTarget:       strcpy(text,pVelTargetNames[(int)(pVelTarget[Program]*1.99f)]); break;
	case pIdOutputGain:      dB2string(pOutputGain[Program],text,3); break;
	default:				 strcpy(text,"");
	}
} 



void WaveSynth::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdWaveformLength: strcpy(label,"bits"); break;
	case pIdModulation:     strcpy(label,"Hz"); break;
	case pIdNoteCut:        strcpy(label,(pNoteCut[Program]>0)?"ms":""); break;
	case pIdOutputGain:     strcpy(label,"dB"); break;
	default:                strcpy(label,""); 
	}
} 



VstInt32 WaveSynth::getProgram(void)
{
	return Program;
}



void WaveSynth::setProgram(VstInt32 program)
{
	Program=program;

	pWaveformLengthInt=pWaveformLengthList[(int)(pWaveformLength[Program]*3.99f)];
}



void WaveSynth::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void WaveSynth::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 WaveSynth::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 WaveSynth::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 WaveSynth::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 WaveSynth::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 WaveSynth::LoadStringChunk(char *str,int max_length,float *src)
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



VstInt32 WaveSynth::SavePresetChunk(float *chunk)
{
	int ptr,pgm;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		chunk[ptr++]=WLEN; chunk[ptr++]=pWaveformLength[pgm];
		chunk[ptr++]=MODU; chunk[ptr++]=pModulation    [pgm];
		chunk[ptr++]=POLY; chunk[ptr++]=pPolyphony     [pgm];
		chunk[ptr++]=POSP; chunk[ptr++]=pPortaSpeed    [pgm];
		chunk[ptr++]=NCUT; chunk[ptr++]=pNoteCut       [pgm];
		chunk[ptr++]=VTGT; chunk[ptr++]=pVelTarget     [pgm];
		chunk[ptr++]=GAIN; chunk[ptr++]=pOutputGain    [pgm];

		chunk[ptr++]=WN00; chunk[ptr++]=pWaveformNibble[pgm][0];
		chunk[ptr++]=WN01; chunk[ptr++]=pWaveformNibble[pgm][1];
		chunk[ptr++]=WN02; chunk[ptr++]=pWaveformNibble[pgm][2];
		chunk[ptr++]=WN03; chunk[ptr++]=pWaveformNibble[pgm][3];
		chunk[ptr++]=WN04; chunk[ptr++]=pWaveformNibble[pgm][4];
		chunk[ptr++]=WN05; chunk[ptr++]=pWaveformNibble[pgm][5];
		chunk[ptr++]=WN06; chunk[ptr++]=pWaveformNibble[pgm][6];
		chunk[ptr++]=WN07; chunk[ptr++]=pWaveformNibble[pgm][7];
	}

	chunk[ptr++]=DONE;

/*
	FILE *file=fopen("g:/params.txt","wt");
	
	fprintf(file,"DATA,\n\n");

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		fprintf(file,"PROG,%i,\n",pgm);

		fprintf(file,"WLEN,%1.3ff,",pWaveformLength[pgm]);
		fprintf(file,"MODU,%1.3ff,",pModulation    [pgm]);
		fprintf(file,"POLY,%1.3ff,",pPolyphony     [pgm]);
		fprintf(file,"POSP,%1.3ff,",pPortaSpeed    [pgm]);
		fprintf(file,"NCUT,%1.3ff,",pNoteCut       [pgm]);
		fprintf(file,"VTGT,%1.3ff,",pVelTarget     [pgm]);
		fprintf(file,"GAIN,%1.3ff,",pOutputGain    [pgm]);

		fprintf(file,"WN00,%1.3ff,",pWaveformNibble[pgm][0]);
		fprintf(file,"WN01,%1.3ff,",pWaveformNibble[pgm][1]);
		fprintf(file,"WN02,%1.3ff,",pWaveformNibble[pgm][2]);
		fprintf(file,"WN03,%1.3ff,",pWaveformNibble[pgm][3]);
		fprintf(file,"WN04,%1.3ff,",pWaveformNibble[pgm][4]);
		fprintf(file,"WN05,%1.3ff,",pWaveformNibble[pgm][5]);
		fprintf(file,"WN06,%1.3ff,",pWaveformNibble[pgm][6]);
		fprintf(file,"WN07,%1.3ff,",pWaveformNibble[pgm][7]);

		fprintf(file,"\n");
	}

	fprintf(file,"\nDONE");
	fclose(file);
*/
	return ptr*sizeof(float);//size in bytes
}



void WaveSynth::LoadPresetChunk(float *chunk)
{
	int i,pgm;
	float tag;

	if(chunk[0]!=DATA)	//load legacy data
	{
		for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
		{
			pWaveformLength[pgm]=*chunk++;
			pModulation    [pgm]=0;
			pPolyphony     [pgm]=*chunk++;
			pPortaSpeed    [pgm]=*chunk++;
			pNoteCut       [pgm]=0;
			pVelTarget     [pgm]=0;
			pOutputGain    [pgm]=*chunk++;

			for(i=0;i<8;++i) pWaveformNibble[pgm][i]=*chunk++;

			if(pPolyphony[pgm]>0) pPolyphony[pgm]=.26f;
		}
	}
	else
	{
		while(1)
		{
			tag=*chunk++;

			if(tag==DONE) break;

			if(tag==PROG) pgm =(int)*chunk++;

			if(tag==NAME) chunk+=LoadStringChunk(ProgramName[pgm],MAX_NAME_LEN,chunk);

			if(tag==WLEN) pWaveformLength[pgm]=*chunk++;
			if(tag==MODU) pModulation    [pgm]=*chunk++;
			if(tag==POLY) pPolyphony     [pgm]=*chunk++;
			if(tag==POSP) pPortaSpeed    [pgm]=*chunk++;
			if(tag==NCUT) pNoteCut       [pgm]=*chunk++;
			if(tag==VTGT) pVelTarget     [pgm]=*chunk++;
			if(tag==GAIN) pOutputGain    [pgm]=*chunk++;

			if(tag==WN00) pWaveformNibble[pgm][0]=*chunk++;
			if(tag==WN01) pWaveformNibble[pgm][1]=*chunk++;
			if(tag==WN02) pWaveformNibble[pgm][2]=*chunk++;
			if(tag==WN03) pWaveformNibble[pgm][3]=*chunk++;
			if(tag==WN04) pWaveformNibble[pgm][4]=*chunk++;
			if(tag==WN05) pWaveformNibble[pgm][5]=*chunk++;
			if(tag==WN06) pWaveformNibble[pgm][6]=*chunk++;
			if(tag==WN07) pWaveformNibble[pgm][7]=*chunk++;
		}
	}
}



VstInt32 WaveSynth::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 WaveSynth::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	return 0;
}



void WaveSynth::MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void WaveSynth::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



void WaveSynth::MidiAddPitchBend(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypePitchBend;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



void WaveSynth::MidiAddModulation(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypeModulation;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



bool WaveSynth::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



VstInt32 WaveSynth::processEvents(VstEvents* ev) 
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



VstInt32 WaveSynth::SynthAllocateVoice(VstInt32 note)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==note) return chn;
	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note<0) return chn;

	return -1;
}



void WaveSynth::SynthChannelChangeNote(VstInt32 chn,VstInt32 note)
{
	SynthChannel[chn].note=note;

	if(note>=0)
	{
		SynthChannel[chn].freq_new=(float)SynthChannel[chn].note-69;

		sSlideStep=(SynthChannel[chn].freq-SynthChannel[chn].freq_new)*20.0f*log(1.0f-pPortaSpeed[Program]);
	}
}



void WaveSynth::SynthUpdateWaveform(void)
{
	VstInt32 i,j,nibble;

	sWaveformSrc=0;

	for(i=0;i<8;++i)
	{
		nibble=(int)(pWaveformNibble[Program][i]*15.99f);

		for(j=0;j<4;++j)
		{
			sWaveformSrc>>=1;
			
			if(nibble&(1<<j)) sWaveformSrc|=0x80000000;
		}
	}
}



void WaveSynth::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float level,modulation,mod_step,velocity,smod_step,sampleRate;
	unsigned int i,c,s,poly_mode,vtgt,wavelen;
	int chn,note,prev_note,out;

	sampleRate=(float)updateSampleRate();

	mod_step=12.0f/sampleRate*(float)M_PI;
	smod_step=pModulation[Program]*MOD_MAX_RATE/sampleRate;

	poly_mode=(int)(pPolyphony[Program]*3.99f);

	wavelen=pWaveformLengthList[(int)(pWaveformLength[Program]*3.99f)];

	SynthUpdateWaveform();

	vtgt=(int)(pVelTarget[Program]*1.99f);
	
	while(--sampleFrames>=0)
	{
		if(MidiModulationDepth>=.01f) modulation=sinf(MidiModulationCount)*MidiModulationDepth; else modulation=0;

		MidiModulationCount+=mod_step;

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

								if(!poly_mode) chn=0; else chn=SynthAllocateVoice(MidiQueue[i].note);

								if(chn>=0)
								{
									prev_note=SynthChannel[chn].note;

									SynthChannelChangeNote(chn,MidiQueue[i].note);

									velocity=(float)MidiQueue[i].velocity/100.0f;

									if(prev_note<0||poly_mode) SynthChannel[chn].freq=SynthChannel[chn].freq_new;

									if(poly_mode||prev_note<0)
									{
										SynthChannel[chn].acc=0;	//phase reset

										SynthChannel[chn].volume=(vtgt==0?velocity:1.0f)/SYNTH_CHANNELS/OVERSAMPLING;
										SynthChannel[chn].duration=pNoteCut[Program]*(vtgt==1?velocity:1.0f)*sampleRate*OVERSAMPLING;
									}
								}
							}
							else//key off
							{
								if(MidiQueue[i].note>=0)
								{
									MidiKeyState[MidiQueue[i].note]=0;

									if(poly_mode)
									{
										for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==MidiQueue[i].note) SynthChannel[chn].note=-1;
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

									for(chn=0;chn<SYNTH_CHANNELS;++chn) SynthChannel[chn].note=-1;
								}
							}
						}
						break;

					case mEventTypeProgram:
						{
							Program=MidiQueue[i].program;
							updateDisplay();
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

		if(!poly_mode)
		{
			if(!MidiIsAnyKeyDown())
			{
				for(chn=0;chn<SYNTH_CHANNELS;++chn) SynthChannel[chn].note=-1;
			}
		}

		for(chn=0;chn<SYNTH_CHANNELS;++chn)
		{
			if(SynthChannel[chn].note<0) continue;

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

			SynthChannel[chn].add=440.0f*pow(2.0f,(SynthChannel[chn].freq+modulation+MidiPitchBend)/12.0f)/sampleRate/OVERSAMPLING;
		}

		level=0;

		for(s=0;s<OVERSAMPLING;++s)
		{
			sModCnt+=smod_step;
			
			while(sModCnt>=1.0f*OVERSAMPLING)
			{
				sModCnt-=1.0f*OVERSAMPLING;

				++sWaveformShift;
			}

			if(pModulation[Program]>0)
			{
				c=sWaveformSrc&(1<<(wavelen-1));

				sWaveform=sWaveformSrc<<(sWaveformShift&(wavelen-1));

				if(c) sWaveform|=1;
			}
			else
			{
				sWaveform=sWaveformSrc;
			}

			out=0;

			for(chn=0;chn<SYNTH_CHANNELS;++chn)
			{
				if(SynthChannel[chn].note<0) continue;

				if(SynthChannel[chn].duration>0)
				{
					if(pNoteCut[Program]>0)
					{
						SynthChannel[chn].duration-=1.0f;

						if(SynthChannel[chn].duration<=0)
						{
							SynthChannel[chn].duration=0;
							SynthChannel[chn].note=-1;

							continue;
						}
					}
				}

				SynthChannel[chn].acc+=SynthChannel[chn].add;

				while(SynthChannel[chn].acc>=1.0f) SynthChannel[chn].acc-=1.0f;

				if(sWaveform&(1<<(int)(SynthChannel[chn].acc*(float)(pWaveformLengthInt-.1f))))
				{
					switch(poly_mode)
					{
					case 0:
					case 1: out =1; break;
					case 2: out|=1; break;
					case 3: out^=1; break;
					}
				}
				else
				{
					if(poly_mode<2) out=0;
				}
				
				if(out) level+=SynthChannel[chn].volume;
			}
		}

		level=level*pOutputGain[Program];

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
