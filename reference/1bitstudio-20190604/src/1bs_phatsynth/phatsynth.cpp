#include "PhatSynth.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new PhatSynth(audioMaster);
}



PhatSynth::PhatSynth(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 v,pgm,chn;

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
		pType      [pgm]=0;
		pPulseWidth[pgm]=.5f;
		pVoices    [pgm]=0;
		pDetune    [pgm]=0;
		pPhase     [pgm]=0;
		pVoiceMix  [pgm]=0;
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
		SynthChannel[chn].pulse=0;
		SynthChannel[chn].pulse_width=0;
		SynthChannel[chn].duration=0;
		SynthChannel[chn].volume=1.0f;

		for(v=0;v<VOICES_MAX;++v) SynthChannel[chn].acc[v]=0;
	}

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

	suspend();
}



PhatSynth::~PhatSynth()
{
}



void PhatSynth::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdType:       pType      [Program]=value; updateDisplay(); break;
	case pIdPulseWidth: pPulseWidth[Program]=value; break;
	case pIdVoices:     pVoices    [Program]=value; break;
	case pIdDetune:     pDetune    [Program]=value; break;		
	case pIdPhase:      pPhase     [Program]=value; break;		
	case pIdVoiceMix:   pVoiceMix  [Program]=value; break;		
	case pIdPolyphony:  pPolyphony [Program]=value; break;
	case pIdPortaSpeed: pPortaSpeed[Program]=value; break;
	case pIdNoteCut:    pNoteCut   [Program]=value; break;
	case pIdVelTarget:  pVelTarget [Program]=value; break;
	case pIdOutputGain: pOutputGain[Program]=value; break;
	}
}



float PhatSynth::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdType:       return pType      [Program];
	case pIdPulseWidth: return pPulseWidth[Program];
	case pIdVoices:     return pVoices    [Program];
	case pIdDetune:     return pDetune    [Program];
	case pIdPhase:      return pPhase     [Program];
	case pIdVoiceMix:   return pVoiceMix  [Program];	
	case pIdPolyphony:  return pPolyphony [Program];
	case pIdPortaSpeed: return pPortaSpeed[Program];
	case pIdNoteCut:    return pNoteCut   [Program];
	case pIdVelTarget:  return pVelTarget [Program];
	case pIdOutputGain: return pOutputGain[Program];
	}

	return 0;
} 



void PhatSynth::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdType:       strcpy(label,"Type");             break;
	case pIdPulseWidth: strcpy(label,"Duty/Width");       break;
	case pIdVoices:     strcpy(label,"Voices");           break;
	case pIdDetune:     strcpy(label,"Phat");             break;
	case pIdPhase:      strcpy(label,"Phase reset");      break;
	case pIdVoiceMix:	strcpy(label,"Square/pulse mix"); break;
	case pIdPolyphony:  strcpy(label,"Polyphony");        break;
	case pIdPortaSpeed: strcpy(label,"Porta speed");      break;
	case pIdNoteCut:    strcpy(label,"Note cut");         break;
	case pIdVelTarget:  strcpy(label,"Velocity target");  break;
	case pIdOutputGain: strcpy(label,"Output gain");      break;
	default:			strcpy(label,"");
	}
} 



void PhatSynth::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdType:		strcpy(text,pType[Program]<.5f?"Square/pulse":"Pin"); break;
	case pIdPulseWidth: if(pType[Program]<.5f) float2string(100.0f*pPulseWidth[Program],text,5); else float2string(PULSE_WIDTH_MAX_US*pPulseWidth[Program],text,6); break;
	case pIdVoices:     sprintf(text,"%i",(int)(1+(pVoices[Program]*(VOICES_MAX-1)))); break;
	case pIdDetune:     float2string(pDetune[Program],text,4); break;
	case pIdPhase:      if(pPhase[Program]==0) strcpy(text,"Free running"); else float2string(pPhase[Program],text,4); break;
	case pIdVoiceMix:	strcpy(text,pVoiceMixNames[(int)(pVoiceMix[Program]*1.99f)]); break;
	case pIdPolyphony:	strcpy(text,pPolyphonyModeNames[(int)(pPolyphony[Program]*3.99f)]); break;
	case pIdPortaSpeed: float2string(pPortaSpeed[Program],text,5); break;
	case pIdNoteCut:    if(pNoteCut[Program]==0) strcpy(text,"Infinite"); else float2string(1000.0f*pNoteCut[Program],text,5); break;
	case pIdVelTarget:  strcpy(text,pVelTargetNames[(int)(pVelTarget[Program]*4.99f)]); break;
	case pIdOutputGain: dB2string(pOutputGain[Program],text,3); break;
	default:			strcpy(text,"");
	}
} 



void PhatSynth::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdPulseWidth: strcpy(label,(pType[Program]<.5f)?"%":"us"); break;
	case pIdNoteCut:	strcpy(label,"ms"); break;
	case pIdOutputGain: strcpy(label,"dB"); break;
	default:			strcpy(label,"");
	}
} 



VstInt32 PhatSynth::getProgram(void)
{
	return Program;
}



void PhatSynth::setProgram(VstInt32 program)
{
	Program=program;
}



void PhatSynth::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void PhatSynth::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 PhatSynth::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 PhatSynth::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 PhatSynth::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 PhatSynth::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 PhatSynth::LoadStringChunk(char *str,int max_length,float *src)
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



VstInt32 PhatSynth::SavePresetChunk(float *chunk)
{
	int ptr,pgm;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		chunk[ptr++]=TYPE; chunk[ptr++]=pType      [pgm];
		chunk[ptr++]=PWDT; chunk[ptr++]=pPulseWidth[pgm];
		chunk[ptr++]=VOIC; chunk[ptr++]=pVoices    [pgm];
		chunk[ptr++]=DETU; chunk[ptr++]=pDetune    [pgm];
		chunk[ptr++]=PHAS; chunk[ptr++]=pPhase     [pgm];
		chunk[ptr++]=VMIX; chunk[ptr++]=pVoiceMix  [pgm];
		chunk[ptr++]=POLY; chunk[ptr++]=pPolyphony [pgm];
		chunk[ptr++]=POSP; chunk[ptr++]=pPortaSpeed[pgm];
		chunk[ptr++]=NCUT; chunk[ptr++]=pNoteCut   [pgm];
		chunk[ptr++]=VTGT; chunk[ptr++]=pVelTarget [pgm];
		chunk[ptr++]=GAIN; chunk[ptr++]=pOutputGain[pgm];
	}

	chunk[ptr++]=DONE;

/*
	FILE *file=fopen("g:/params.txt","wt");
	
	fprintf(file,"DATA,\n\n");

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		fprintf(file,"PROG,%i,\n",pgm);

		fprintf(file,"TYPE,%1.3ff,",pType      [pgm]);
		fprintf(file,"PWDT,%1.3ff,",pPulseWidth[pgm]);
		fprintf(file,"VOIC,%1.3ff,",pVoices    [pgm]);
		fprintf(file,"DETU,%1.3ff,",pDetune    [pgm]);
		fprintf(file,"PHAS,%1.3ff,",pPhase     [pgm]);
		fprintf(file,"VMIX,%1.3ff,",pVoiceMix  [pgm]);
		fprintf(file,"POLY,%1.3ff,",pPolyphony [pgm]);
		fprintf(file,"POSP,%1.3ff,",pPortaSpeed[pgm]);
		fprintf(file,"NCUT,%1.3ff,",pNoteCut   [pgm]);
		fprintf(file,"VTGT,%1.3ff,",pVelTarget [pgm]);
		fprintf(file,"GAIN,%1.3ff,",pOutputGain[pgm]);
		fprintf(file,"\n");
	}

	fprintf(file,"\nDONE");
	fclose(file);
*/
	return ptr*sizeof(float);//size in bytes
}



void PhatSynth::LoadPresetChunk(float *chunk)
{
	int pgm;
	float tag;

	while(1)
	{
		tag=*chunk++;

		if(tag==DONE) break;

		if(tag==PROG) pgm =(int)*chunk++;

		if(tag==NAME) chunk+=LoadStringChunk(ProgramName[pgm],MAX_NAME_LEN,chunk);

		if(tag==TYPE) pType      [pgm]=*chunk++;
		if(tag==PWDT) pPulseWidth[pgm]=*chunk++;
		if(tag==VOIC) pVoices    [pgm]=*chunk++;
		if(tag==DETU) pDetune    [pgm]=*chunk++;
		if(tag==PHAS) pPhase     [pgm]=*chunk++;
		if(tag==VMIX) pVoiceMix  [pgm]=*chunk++;
		if(tag==POLY) pPolyphony [pgm]=*chunk++;
		if(tag==POSP) pPortaSpeed[pgm]=*chunk++;
		if(tag==NCUT) pNoteCut   [pgm]=*chunk++;
		if(tag==VTGT) pVelTarget [pgm]=*chunk++;
		if(tag==GAIN) pOutputGain[pgm]=*chunk++;
	}
}



VstInt32 PhatSynth::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 PhatSynth::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	return 0;
}



void PhatSynth::MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void PhatSynth::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



void PhatSynth::MidiAddPitchBend(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypePitchBend;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



void PhatSynth::MidiAddModulation(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypeModulation;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



bool PhatSynth::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



VstInt32 PhatSynth::processEvents(VstEvents* ev) 
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



VstInt32 PhatSynth::SynthAllocateVoice(VstInt32 note)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==note) return chn;
	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note<0) return chn;

	return -1;
}



void PhatSynth::SynthChannelChangeNote(VstInt32 chn,VstInt32 note)
{
	SynthChannel[chn].note=note;

	if(note>=0)
	{
		SynthChannel[chn].freq_new=(float)SynthChannel[chn].note-69;

		sSlideStep=(SynthChannel[chn].freq-SynthChannel[chn].freq_new)*20.0f*log(1.0f-pPortaSpeed[Program]);
	}
}



void PhatSynth::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float n,level,modulation,mod_step,velocity,sampleRate;
	unsigned int i,s,v,voices,poly_mode;
	int chn,note,prev_note,vtgt,vmix,out,cout;

	sampleRate=(float)updateSampleRate();

	mod_step=12.0f/sampleRate*(float)M_PI;

	poly_mode=(int)(pPolyphony[Program]*3.99f);

	vtgt=(int)(pVelTarget[Program]*4.99f);
	vmix=(int)(pVoiceMix [Program]*1.99f);

	voices=1+(int)(pVoices[Program]*(VOICES_MAX-1));

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
										if(pPhase[Program]>0)	//phase reset
										{
											for(v=0;v<VOICES_MAX;++v) SynthChannel[chn].acc[v]=(pPhase[Program]+(vtgt==2?velocity:0))*(float)v/VOICES_MAX;
										}

										n=-(float)voices/2;

										for(v=0;v<voices;++v)
										{
											SynthChannel[chn].detune[v]=(pDetune[Program]*(vtgt==1?velocity:1.0f)*n)/sampleRate/OVERSAMPLING;

											n+=1.0f;
										}

										SynthChannel[chn].freq=SynthChannel[chn].freq_new;
										SynthChannel[chn].pulse=0;
										SynthChannel[chn].duration=pNoteCut[Program]*(vtgt==4?velocity:1.0f)*sampleRate*OVERSAMPLING;
										SynthChannel[chn].volume=(vtgt==0?velocity:1.0f)/SYNTH_CHANNELS/OVERSAMPLING;

										if(pType[Program]<.5f)
										{
											SynthChannel[chn].pulse_width=pPulseWidth[Program]*(vtgt==3?velocity:1.0f);
										}
										else
										{
											SynthChannel[chn].pulse_width=pPulseWidth[Program]*(vtgt==3?velocity:1.0f)*PULSE_WIDTH_MAX_US*sampleRate*OVERSAMPLING/1000000.0f;
										}
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
				
				if(pType[Program]>=.5f)	//pin
				{
					for(v=0;v<voices;++v)
					{
						SynthChannel[chn].acc[v]+=SynthChannel[chn].add+SynthChannel[chn].detune[v];

						while(SynthChannel[chn].acc[v]>=1.0f)
						{
							SynthChannel[chn].acc[v]-=1.0f;

							SynthChannel[chn].pulse=SynthChannel[chn].pulse_width;
						}
					}

					if(SynthChannel[chn].pulse>0)
					{
						SynthChannel[chn].pulse-=1.0f;

						if(poly_mode!=2) out=1; else out^=1;
						
						if(!poly_mode||poly_mode==3) level+=SynthChannel[chn].volume;	//ADD based mixing modes
					}
					else
					{
						SynthChannel[chn].pulse=0;
					}
				}
				else	//square/pulse
				{
					cout=0;

					for(v=0;v<voices;++v)
					{
						SynthChannel[chn].acc[v]+=SynthChannel[chn].add+SynthChannel[chn].detune[v];

						while(SynthChannel[chn].acc[v]>=1.0f)
						{
							SynthChannel[chn].acc[v]-=1.0f;
						}

						switch(vmix)
						{
						case 0: cout|=SynthChannel[chn].acc[v]>=SynthChannel[chn].pulse_width?0:1; break;
						case 1: cout^=SynthChannel[chn].acc[v]>=SynthChannel[chn].pulse_width?0:1; break;
						}
					}

					if(cout>0)
					{
						if(poly_mode!=2) out=1; else out^=1;
						
						if(!poly_mode||poly_mode==3) level+=SynthChannel[chn].volume;	//ADD based mixing modes
					}
				}
			}

			if(poly_mode==1||poly_mode==2) if(out) level+=SynthChannel[0].volume;	//PWM mixing
		}

		level=level*pOutputGain[Program];

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
