#include "PhaseSynth.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new PhaseSynth(audioMaster);
}



PhaseSynth::PhaseSynth(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
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
		pOscTypeA  [pgm]=0;
		pOscTypeB  [pgm]=0;
		pMultipleA [pgm]=0;
		pDetuneA   [pgm]=0;
		pDutyCycleA[pgm]=.5f;
		pDutyCycleB[pgm]=.5f;
		pPhaseReset[pgm]=0;
		pMixMode   [pgm]=0;
		pPolyphony [pgm]=1.0f;
		pPortaSpeed[pgm]=1.0f;
		pNoteCut   [pgm]=0;
		pVelTarget [pgm]=0;
		pOutputGain[pgm]=1.0f;
*/
	}

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].note=-1;
		SynthChannel[chn].freq=0;
		SynthChannel[chn].freq_new=0;
		SynthChannel[chn].accA=0;
		SynthChannel[chn].accB=0;
		SynthChannel[chn].pulseA=0;
		SynthChannel[chn].pulseB=0;
		SynthChannel[chn].pulseDurationA=0;
		SynthChannel[chn].pulseDurationB=0;
		SynthChannel[chn].outA=0;
		SynthChannel[chn].outB=0;
		SynthChannel[chn].volume=1.0f/SYNTH_CHANNELS/OVERSAMPLING;
		SynthChannel[chn].duration=0;
		SynthChannel[chn].detune=0;
	}

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

	memset(MidiKeyState,0,sizeof(MidiKeyState));

	suspend();
}



PhaseSynth::~PhaseSynth()
{
}



float PhaseSynth::FloatToMultiple(float value)
{
	const float mul[]={1,2,4,8,16,32};

	return mul[(int)(value*5.99f)];
}



void PhaseSynth::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdOscTypeA:    pOscTypeA   [Program]=value; updateDisplay(); break;
	case pIdOscTypeB:    pOscTypeB   [Program]=value; updateDisplay(); break;
	case pIdMultipleA:   pMultipleA  [Program]=value; break;
	case pIdDetuneA:     pDetuneA    [Program]=value; break;
	case pIdDutyCycleA:  pDutyCycleA [Program]=value; break;
	case pIdDutyCycleB:  pDutyCycleB [Program]=value; break;
	case pIdPhaseReset:  pPhaseReset [Program]=value; break;
	case pIdOscMixMode:	 pOscMixMode [Program]=value; break;
	case pIdPolyphony:   pPolyphony  [Program]=value; break;
	case pIdPortaSpeed:  pPortaSpeed [Program]=value; break;
	case pIdNoteCut:     pNoteCut    [Program]=value; updateDisplay(); break;
	case pIdVelTarget:   pVelTarget  [Program]=value; break;
	case pIdOutputGain:  pOutputGain [Program]=value; break;
	}
}



float PhaseSynth::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdOscTypeA:    return pOscTypeA   [Program];
	case pIdOscTypeB:    return pOscTypeB   [Program];
	case pIdMultipleA:   return pMultipleA  [Program];
	case pIdDetuneA:     return pDetuneA    [Program];
	case pIdDutyCycleA:  return pDutyCycleA [Program];
	case pIdDutyCycleB:  return pDutyCycleB [Program];
	case pIdPhaseReset:  return pPhaseReset [Program];
	case pIdOscMixMode:	 return pOscMixMode [Program];
	case pIdPolyphony:   return pPolyphony  [Program];
	case pIdPortaSpeed:  return pPortaSpeed [Program];
	case pIdNoteCut:     return pNoteCut    [Program];
	case pIdVelTarget:   return pVelTarget  [Program];
	case pIdOutputGain:  return pOutputGain [Program];
	}

	return 0;
} 



void PhaseSynth::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdOscTypeA:    strcpy(label,"OscA type");       break;
	case pIdOscTypeB:    strcpy(label,"OscB type");       break;
	case pIdMultipleA:   strcpy(label,"Multiple OscA");   break;
	case pIdDetuneA:     strcpy(label,"Detune OscA");     break;
	case pIdDutyCycleA:  strcpy(label,(pOscTypeA[Program]<.5f)?"Duty cycle OscA":"Pin width OscA"); break;
	case pIdDutyCycleB:  strcpy(label,(pOscTypeB[Program]<.5f)?"Duty cycle OscB":"Pin width OscB"); break;
	case pIdPhaseReset:  strcpy(label,"Phase reset");	  break;
	case pIdOscMixMode:  strcpy(label,"OscA/B mix");      break;
	case pIdPolyphony:   strcpy(label,"Polyphony");       break;
	case pIdPortaSpeed:  strcpy(label,"Porta speed");     break;
	case pIdNoteCut:     strcpy(label,"Note cut");        break;
	case pIdVelTarget:   strcpy(label,"Velocity target"); break;
	case pIdOutputGain:  strcpy(label,"Output gain");     break;
	default:             strcpy(label,"");
	}
} 



void PhaseSynth::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdOscTypeA:	strcpy(text,(pOscTypeA[Program]<.5f)?"Square/Pulse":"Pin"); break;
	case pIdOscTypeB:	strcpy(text,(pOscTypeB[Program]<.5f)?"Square/Pulse":"Pin"); break;
	case pIdMultipleA:  sprintf(text,"%i",((int)FloatToMultiple(pMultipleA[Program]))); break;
	case pIdDetuneA:    if(pDetuneA[Program]==0) strcpy(text,"Sync"); else float2string(pDetuneA[Program]*12.0f,text,5); break;
	case pIdDutyCycleA: if(pOscTypeA[Program]<.5f) float2string(100.0f*pDutyCycleA[Program],text,5); else float2string(PULSE_WIDTH_MAX_US*pDutyCycleA[Program],text,6); break;
	case pIdDutyCycleB: if(pOscTypeB[Program]<.5f) float2string(100.0f*pDutyCycleB[Program],text,5); else float2string(PULSE_WIDTH_MAX_US*pDutyCycleB[Program],text,6); break;
	case pIdPhaseReset: if(pPhaseReset[Program]>0) float2string(pPhaseReset[Program],text,5); else strcpy(text,"Off"); break;
	case pIdOscMixMode:	strcpy(text,pOscMixModeNames[(int)(pOscMixMode[Program]*2.99f)]); break;
	case pIdPolyphony:	strcpy(text,pPolyMixModeNames[(int)(pPolyphony[Program]*3.99f)]); break;
	case pIdPortaSpeed: float2string(pPortaSpeed[Program],text,5); break;
	case pIdNoteCut:    if(pNoteCut[Program]==0) strcpy(text,"Infinite"); else float2string(1000.0f*pNoteCut[Program],text,5); break;
	case pIdVelTarget:  strcpy(text,pVelTargetNames[(int)(pVelTarget[Program]*2.99f)]); break;
	case pIdOutputGain: dB2string(pOutputGain[Program],text,3); break;
	default:            strcpy(text,"");
	}
} 



void PhaseSynth::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdMultipleA:  strcpy(label,"x"); break;
	case pIdDutyCycleA: strcpy(label,(pOscTypeA[Program]<.5f)?"%":"us"); break;
	case pIdDutyCycleB: strcpy(label,(pOscTypeB[Program]<.5f)?"%":"us"); break;
	case pIdNoteCut:    strcpy(label,(pNoteCut[Program]>0)?"ms":""); break;
	case pIdOutputGain: strcpy(label,"dB"); break;
	default:            strcpy(label,"");
	}
} 



VstInt32 PhaseSynth::getProgram(void)
{
	return Program;
}



void PhaseSynth::setProgram(VstInt32 program)
{
	Program=program;
}



void PhaseSynth::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void PhaseSynth::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 PhaseSynth::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 PhaseSynth::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 PhaseSynth::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 PhaseSynth::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 PhaseSynth::LoadStringChunk(char *str,int max_length,float *src)
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



VstInt32 PhaseSynth::SavePresetChunk(float *chunk)
{
	int ptr,pgm;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		chunk[ptr++]=OTYA; chunk[ptr++]=pOscTypeA   [pgm];
		chunk[ptr++]=OTYB; chunk[ptr++]=pOscTypeB   [pgm];
		chunk[ptr++]=MULA; chunk[ptr++]=pMultipleA  [pgm];
		chunk[ptr++]=DETA; chunk[ptr++]=pDetuneA    [pgm];
		chunk[ptr++]=DUTA; chunk[ptr++]=pDutyCycleA [pgm];
		chunk[ptr++]=DUTB; chunk[ptr++]=pDutyCycleB [pgm];
		chunk[ptr++]=PHAR; chunk[ptr++]=pPhaseReset [pgm];
		chunk[ptr++]=MODE; chunk[ptr++]=pOscMixMode [pgm];
		chunk[ptr++]=POLY; chunk[ptr++]=pPolyphony  [pgm];
		chunk[ptr++]=POSP; chunk[ptr++]=pPortaSpeed [pgm];
		chunk[ptr++]=NCUT; chunk[ptr++]=pNoteCut    [pgm];
		chunk[ptr++]=VTGT; chunk[ptr++]=pVelTarget  [pgm];
		chunk[ptr++]=GAIN; chunk[ptr++]=pOutputGain [pgm];
	}

	chunk[ptr++]=DONE;

/*
	FILE *file=fopen("g:/params.txt","wt");
	
	fprintf(file,"DATA,\n\n");

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		fprintf(file,"PROG,%i,\n",pgm);

		fprintf(file,"OTYA,%1.3ff,",pOscTypeA   [pgm]);
		fprintf(file,"OTYB,%1.3ff,",pOscTypeB   [pgm]);
		fprintf(file,"MULA,%1.3ff,",pMultipleA  [pgm]);
		fprintf(file,"DETA,%1.3ff,",pDetuneA    [pgm]);
		fprintf(file,"DUTA,%1.3ff,",pDutyCycleA [pgm]);
		fprintf(file,"DUTB,%1.3ff,",pDutyCycleB [pgm]);
		fprintf(file,"PHAR,%1.3ff,",pPhaseReset [pgm]);
		fprintf(file,"MODE,%1.3ff,",pOscMixMode [pgm]);
		fprintf(file,"POLY,%1.3ff,",pPolyphony  [pgm]);
		fprintf(file,"POSP,%1.3ff,",pPortaSpeed [pgm]);
		fprintf(file,"NCUT,%1.3ff,",pNoteCut    [pgm]);
		fprintf(file,"VTGT,%1.3ff,",pVelTarget  [pgm]);
		fprintf(file,"GAIN,%1.3ff,",pOutputGain [pgm]);

		fprintf(file,"\n");
	}

	fprintf(file,"\nDONE");
	fclose(file);
*/

	return ptr*sizeof(float);//size in bytes
}



void PhaseSynth::LoadPresetChunk(float *chunk)
{
	int pgm;
	float tag;

	if(chunk[0]!=DATA)	//load legacy data
	{
		for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
		{
			pOscTypeA   [pgm]=0;
			pOscTypeB   [pgm]=0;
			pMultipleA  [pgm]=*chunk++;
			pDetuneA    [pgm]=*chunk++;
			pDutyCycleA [pgm]=*chunk++;
			pDutyCycleB [pgm]=*chunk++;
			pPhaseReset [pgm]=*chunk++;
			pOscMixMode [pgm]=*chunk++;
			pPolyphony  [pgm]=*chunk++;
			pPortaSpeed [pgm]=*chunk++;
			pNoteCut    [pgm]=0;
			pVelTarget  [pgm]=0;
			pOutputGain [pgm]=*chunk++;

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

			if(tag==OTYA) pOscTypeA   [pgm]=*chunk++;
			if(tag==OTYB) pOscTypeB   [pgm]=*chunk++;
			if(tag==MULA) pMultipleA  [pgm]=*chunk++;
			if(tag==DETA) pDetuneA    [pgm]=*chunk++;
			if(tag==DUTA) pDutyCycleA [pgm]=*chunk++;
			if(tag==DUTB) pDutyCycleB [pgm]=*chunk++;
			if(tag==PHAR) pPhaseReset [pgm]=*chunk++;
			if(tag==MODE) pOscMixMode [pgm]=*chunk++;
			if(tag==POLY) pPolyphony  [pgm]=*chunk++;
			if(tag==POSP) pPortaSpeed [pgm]=*chunk++;
			if(tag==NCUT) pNoteCut    [pgm]=*chunk++;
			if(tag==VTGT) pVelTarget  [pgm]=*chunk++;
			if(tag==GAIN) pOutputGain [pgm]=*chunk++;
		}
	}
}



VstInt32 PhaseSynth::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 PhaseSynth::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	return 0;
}



void PhaseSynth::MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void PhaseSynth::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



void PhaseSynth::MidiAddPitchBend(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypePitchBend;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



void PhaseSynth::MidiAddModulation(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypeModulation;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



bool PhaseSynth::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



VstInt32 PhaseSynth::processEvents(VstEvents* ev) 
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



VstInt32 PhaseSynth::SynthAllocateVoice(VstInt32 note)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==note) return chn;
	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note<0) return chn;

	return -1;
}



void PhaseSynth::SynthChannelChangeNote(VstInt32 chn,VstInt32 note)
{
	SynthChannel[chn].note=note;

	if(note>=0)
	{
		SynthChannel[chn].freq_new=(float)SynthChannel[chn].note-69;

		sSlideStep=(SynthChannel[chn].freq-SynthChannel[chn].freq_new)*20.0f*log(1.0f-pPortaSpeed[Program]);
	}
}



void PhaseSynth::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float level,velocity,modulation,mod_step,sampleRate;
	unsigned int i,s,out,outn,osc_mode,vtgt,poly_mode;
	int chn,note,prev_note;

	sampleRate=(float)updateSampleRate();

	mod_step=12.0f/sampleRate*(float)M_PI;

	osc_mode =(int)(pOscMixMode[Program]*2.99f);
	poly_mode=(int)(pPolyphony[Program]*3.99f);

	vtgt=(int)(pVelTarget[Program]*2.99f);

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

									if(prev_note<0||poly_mode)
									{
										if(pPhaseReset[Program]>0)
										{
											SynthChannel[chn].accA=0;						//phase reset
											SynthChannel[chn].accB=pPhaseReset[Program];	//phase reset

											SynthChannel[chn].pulseA=0;
											SynthChannel[chn].pulseB=0;

											SynthChannel[chn].outA=0;
											SynthChannel[chn].outB=0;
										}

										SynthChannel[chn].freq=SynthChannel[chn].freq_new;
										SynthChannel[chn].volume=(vtgt==0?velocity:1.0f)/SYNTH_CHANNELS/OVERSAMPLING;
										SynthChannel[chn].duration=pNoteCut[Program]*(vtgt==2?velocity:1.0f)*sampleRate*OVERSAMPLING;

										if(pOscTypeA[Program]>=.5f) SynthChannel[chn].pulseDurationA=pDutyCycleA[Program]*PULSE_WIDTH_MAX_US*sampleRate*OVERSAMPLING/1000000.0f;
										if(pOscTypeB[Program]>=.5f) SynthChannel[chn].pulseDurationB=pDutyCycleB[Program]*PULSE_WIDTH_MAX_US*sampleRate*OVERSAMPLING/1000000.0f;
									}

									SynthChannel[chn].detune=(vtgt==1?velocity:0)*.5f;
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

			SynthChannel[chn].addA=440.0f*pow(2.0f,(SynthChannel[chn].freq+modulation+MidiPitchBend+pDetuneA[Program]*12.0f)/12.0f)*FloatToMultiple(pMultipleA[Program])/sampleRate/OVERSAMPLING;
			SynthChannel[chn].addB=440.0f*pow(2.0f,(SynthChannel[chn].freq+modulation+SynthChannel[chn].detune+MidiPitchBend)/12.0f)/sampleRate/OVERSAMPLING;
		}

		level=0;

		for(s=0;s<OVERSAMPLING;++s)
		{
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

				SynthChannel[chn].accA+=SynthChannel[chn].addA;
				SynthChannel[chn].accB+=SynthChannel[chn].addB;

				if(pOscTypeA[Program]<.5f)	//square/pulse
				{
					while(SynthChannel[chn].accA>=1.0f) SynthChannel[chn].accA-=1.0f;

					if(SynthChannel[chn].accA<pDutyCycleA[Program]) SynthChannel[chn].outA=1; else SynthChannel[chn].outA=0;
				}
				else	//pin
				{
					while(SynthChannel[chn].accA>=1.0f)
					{
						SynthChannel[chn].accA-=1.0f;
						SynthChannel[chn].pulseA=SynthChannel[chn].pulseDurationA;
					}

					if(SynthChannel[chn].pulseA>0)
					{
						SynthChannel[chn].pulseA-=1.0f;
						SynthChannel[chn].outA=1;
					}
					else
					{
						SynthChannel[chn].pulseA=0;
						SynthChannel[chn].outA=0;
					}
				}

				if(pOscTypeB[Program]<.5f)	//square/pulse
				{
					while(SynthChannel[chn].accB>=1.0f) SynthChannel[chn].accB-=1.0f;

					if(SynthChannel[chn].accB<pDutyCycleB[Program]) SynthChannel[chn].outB=1; else SynthChannel[chn].outB=0;
				}
				else	//pin
				{
					while(SynthChannel[chn].accB>=1.0f)
					{
						SynthChannel[chn].accB-=1.0f;
						SynthChannel[chn].pulseB=SynthChannel[chn].pulseDurationB;
					}

					if(SynthChannel[chn].pulseB>0)
					{
						SynthChannel[chn].pulseB-=1.0f;
						SynthChannel[chn].outB=1;
					}
					else
					{
						SynthChannel[chn].pulseB=0;
						SynthChannel[chn].outB=0;
					}
				}

				switch(osc_mode)	//osc mix
				{
				case 0:  outn=SynthChannel[chn].outA|SynthChannel[chn].outB; break;	//OR
				case 1:  outn=SynthChannel[chn].outA&SynthChannel[chn].outB; break;	//AND
				case 2:  outn=SynthChannel[chn].outA^SynthChannel[chn].outB; break;	//XOR
				default: outn=0;
				}

				switch(poly_mode)
				{
				case 0:
				case 1: if(outn) level+=SynthChannel[chn].volume; break; //ADD
				case 2: out|=outn; break; //OR
				case 3: out^=outn; break; //XOR
				}
			}

			if(out) level+=SynthChannel[0].volume;	//other than ADD
		}

		level=level*pOutputGain[Program];

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
