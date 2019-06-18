#include "PulseSynth.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new PulseSynth(audioMaster);
}



PulseSynth::PulseSynth(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
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

		/*pAttack    [pgm]=0;
		pDecay     [pgm]=0;
		pSustain   [pgm]=1.0f;
		pRelease   [pgm]=0;
		pPulseWidth[pgm]=.25f;
		pDetune    [pgm]=.5f;
		pLowBoost  [pgm]=0;
		pPolyphony [pgm]=1.0f;
		pSlideSpeed[pgm]=1.0f;
		pOutputGain[pgm]=1.0f;*/
	}

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].note=-1;
		SynthChannel[chn].freq=0;
		SynthChannel[chn].freq_new=0;
		SynthChannel[chn].acc=0;
		SynthChannel[chn].e_stage=eStageReset;
		SynthChannel[chn].e_delta=0;
		SynthChannel[chn].e_level=0;
		SynthChannel[chn].pulse=0;
		SynthChannel[chn].volume=1.0f;
	}

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

	suspend();
}



PulseSynth::~PulseSynth()
{
}



void PulseSynth::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdAttack:     pAttack    [Program]=value; break;
	case pIdDecay:      pDecay     [Program]=value; break;
	case pIdSustain:    pSustain   [Program]=value; break;
	case pIdRelease:    pRelease   [Program]=value; break;
	case pIdPulseWidth: pPulseWidth[Program]=value; break;
	case pIdDetune:     pDetune    [Program]=value; break;
	case pIdLowBoost:   pLowBoost  [Program]=value; break;
	case pIdPolyphony:  pPolyphony [Program]=value; break;
	case pIdPortaSpeed: pPortaSpeed[Program]=value; break;
	case pIdOutputGain: pOutputGain[Program]=value; break;
	}
}



float PulseSynth::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdAttack:     return pAttack    [Program];
	case pIdDecay:      return pDecay     [Program];
	case pIdSustain:    return pSustain   [Program];
	case pIdRelease:    return pRelease   [Program];
	case pIdPulseWidth: return pPulseWidth[Program];
	case pIdDetune:     return pDetune    [Program];
	case pIdLowBoost:   return pLowBoost  [Program];
	case pIdPolyphony:  return pPolyphony [Program];
	case pIdPortaSpeed: return pPortaSpeed[Program];
	case pIdOutputGain: return pOutputGain[Program];
	}

	return 0;
} 



void PulseSynth::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdAttack:     strcpy(label,"Attack time");   break;
	case pIdDecay:      strcpy(label,"Decay time");    break;
	case pIdSustain:    strcpy(label,"Sustain level"); break;
	case pIdRelease:    strcpy(label,"Release time");  break;
	case pIdPulseWidth: strcpy(label,"Pulse width");   break;
	case pIdDetune:     strcpy(label,"Detune");        break;
	case pIdLowBoost:   strcpy(label,"Boost low end"); break;
	case pIdPolyphony:  strcpy(label,"Polyphony");     break;
	case pIdPortaSpeed: strcpy(label,"Porta speed");   break;
	case pIdOutputGain: strcpy(label,"Output gain");   break;
	default:			strcpy(label,"");
	}
} 



void PulseSynth::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdAttack:     float2string(ENVELOPE_ATTACK_MAX_MS*pAttack[Program],text,6); break;
	case pIdDecay:      float2string(ENVELOPE_DECAY_MAX_MS*pDecay[Program],text,6); break;
	case pIdSustain:    float2string(pSustain[Program],text,5); break;
	case pIdRelease:    float2string(ENVELOPE_RELEASE_MAX_MS*pRelease[Program],text,5); break;
	case pIdPulseWidth: float2string(PULSE_WIDTH_MAX_US*pPulseWidth[Program],text,5); break;
	case pIdDetune:     float2string(-1.0f+pDetune[Program]*2.0f,text,5); break;
	case pIdLowBoost:	strcpy(text,pLowBoost[Program]<.5f?"Disabled":"Enabled"); break;
	case pIdPolyphony:	strcpy(text,pPolyphonyModeNames[(int)(pPolyphony[Program]*2.99f)]); break;
	case pIdPortaSpeed: float2string(pPortaSpeed[Program],text,5); break;
	case pIdOutputGain: dB2string(pOutputGain[Program],text,3); break;
	default:			strcpy(text,"");
	}
} 



void PulseSynth::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdAttack:
	case pIdDecay:
	case pIdRelease:    strcpy(label,"ms");   break;
	case pIdPulseWidth: strcpy(label,"us");  break;
	case pIdOutputGain: strcpy(label,"dB"); break;
	default:			strcpy(label,"");
	}
} 



VstInt32 PulseSynth::getProgram(void)
{
	return Program;
}



void PulseSynth::setProgram(VstInt32 program)
{
	Program=program;
}



void PulseSynth::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void PulseSynth::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 PulseSynth::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 PulseSynth::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 PulseSynth::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 PulseSynth::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 PulseSynth::LoadStringChunk(char *str,int max_length,float *src)
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



VstInt32 PulseSynth::SavePresetChunk(float *chunk)
{
	int ptr,pgm;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		chunk[ptr++]=EATT; chunk[ptr++]=pAttack    [pgm];
		chunk[ptr++]=EDEC; chunk[ptr++]=pDecay     [pgm];
		chunk[ptr++]=ESUS; chunk[ptr++]=pSustain   [pgm];
		chunk[ptr++]=EREL; chunk[ptr++]=pRelease   [pgm];
		chunk[ptr++]=PWDT; chunk[ptr++]=pPulseWidth[pgm];
		chunk[ptr++]=DETU; chunk[ptr++]=pDetune    [pgm];
		chunk[ptr++]=LOWB; chunk[ptr++]=pLowBoost  [pgm];
		chunk[ptr++]=POLY; chunk[ptr++]=pPolyphony [pgm];
		chunk[ptr++]=POSP; chunk[ptr++]=pPortaSpeed[pgm];
		chunk[ptr++]=GAIN; chunk[ptr++]=pOutputGain[pgm];
	}

	chunk[ptr++]=DONE;

/*
	FILE *file=fopen("g:/params.txt","wt");
	
	fprintf(file,"DATA,\n\n");

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		fprintf(file,"PROG,%i,\n",pgm);

		fprintf(file,"EATT,%1.3ff,",pAttack    [pgm]);
		fprintf(file,"EDEC,%1.3ff,",pDecay     [pgm]);
		fprintf(file,"ESUS,%1.3ff,",pSustain   [pgm]);
		fprintf(file,"EREL,%1.3ff,",pRelease   [pgm]);
		fprintf(file,"PWDT,%1.3ff,",pPulseWidth[pgm]);
		fprintf(file,"DETU,%1.3ff,",pDetune    [pgm]);
		fprintf(file,"LOWB,%1.3ff,",pLowBoost  [pgm]);
		fprintf(file,"POLY,%1.3ff,",pPolyphony [pgm]);
		fprintf(file,"POSP,%1.3ff,",pPortaSpeed[pgm]);
		fprintf(file,"GAIN,%1.3ff,",pOutputGain[pgm]);
		fprintf(file,"\n");
	}

	fprintf(file,"\nDONE");
	fclose(file);
*/
	return ptr*sizeof(float);//size in bytes
}



void PulseSynth::LoadPresetChunk(float *chunk)
{
	int pgm;
	float tag;

	//default parameters for legacy data support

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		pLowBoost[pgm]=0;
	}

	if(chunk[0]!=DATA)	//load legacy data
	{
		for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
		{
			pAttack    [pgm]=*chunk++;
			pDecay     [pgm]=*chunk++;
			pSustain   [pgm]=*chunk++;
			pRelease   [pgm]=*chunk++;
			pPulseWidth[pgm]=*chunk++;
			pDetune    [pgm]=*chunk++;
			pPolyphony [pgm]=*chunk++;
			pPortaSpeed[pgm]=*chunk++;
			pOutputGain[pgm]=*chunk++;
		}
	}
	else	//load normal data
	{
		while(1)
		{
			tag=*chunk++;

			if(tag==DONE) break;

			if(tag==PROG) pgm =(int)*chunk++;

			if(tag==NAME) chunk+=LoadStringChunk(ProgramName[pgm],MAX_NAME_LEN,chunk);

			if(tag==EATT) pAttack    [pgm]=*chunk++;
			if(tag==EDEC) pDecay     [pgm]=*chunk++;
			if(tag==ESUS) pSustain   [pgm]=*chunk++;
			if(tag==EREL) pRelease   [pgm]=*chunk++;
			if(tag==PWDT) pPulseWidth[pgm]=*chunk++;
			if(tag==DETU) pDetune    [pgm]=*chunk++;
			if(tag==LOWB) pLowBoost  [pgm]=*chunk++;
			if(tag==POLY) pPolyphony [pgm]=*chunk++;
			if(tag==POSP) pPortaSpeed[pgm]=*chunk++;
			if(tag==GAIN) pOutputGain[pgm]=*chunk++;
		}
	}
}



VstInt32 PulseSynth::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 PulseSynth::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	return 0;
}



void PulseSynth::MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void PulseSynth::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



void PulseSynth::MidiAddPitchBend(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypePitchBend;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



void PulseSynth::MidiAddModulation(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypeModulation;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



bool PulseSynth::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



VstInt32 PulseSynth::processEvents(VstEvents* ev) 
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



VstInt32 PulseSynth::SynthAllocateVoice(VstInt32 note)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==note) return chn;
	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note<0) return chn;
	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].e_stage==eStageRelease) return chn;

	return -1;
}



void PulseSynth::SynthChannelChangeNote(VstInt32 chn,VstInt32 note)
{
	SynthChannel[chn].note=note;

	if(note>=0)
	{
		SynthChannel[chn].freq_new=(float)SynthChannel[chn].note-69;

		sSlideStep=(SynthChannel[chn].freq-SynthChannel[chn].freq_new)*20.0f*log(1.0f-pPortaSpeed[Program]);
	}
}



float PulseSynth::SynthEnvelopeTimeToDelta(float value,float max_ms)
{
	return (1.0f/ENVELOPE_UPDATE_RATE_HZ)/(value*max_ms/1000.0f);
}



void PulseSynth::SynthRestartEnvelope(VstInt32 chn)
{
	if(pAttack[Program]>0||pDecay[Program]>0)
	{
		SynthChannel[chn].e_stage=eStageAttack;
		SynthChannel[chn].e_level=0;
		SynthChannel[chn].e_delta=SynthEnvelopeTimeToDelta(pAttack[Program],ENVELOPE_ATTACK_MAX_MS);
	}
	else
	{
		SynthChannel[chn].e_stage=eStageSustain;
		SynthChannel[chn].e_level=pSustain[Program];
	}
}



void PulseSynth::SynthStopEnvelope(VstInt32 chn)
{
	SynthChannel[chn].e_stage=eStageRelease;
	SynthChannel[chn].e_delta=SynthEnvelopeTimeToDelta(pRelease[Program],ENVELOPE_RELEASE_MAX_MS);
}



void PulseSynth::SynthAdvanceEnvelopes(void)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		if(SynthChannel[chn].e_stage==eStageReset)
		{
			SynthChannel[chn].note=-1;
			SynthChannel[chn].e_level=0;
		}
		else
		{
			switch(SynthChannel[chn].e_stage)
			{
			case eStageAttack:			
				{
					SynthChannel[chn].e_level+=SynthChannel[chn].e_delta;

					if(SynthChannel[chn].e_level>=1.0f)
					{
						SynthChannel[chn].e_level=1.0f;
						SynthChannel[chn].e_delta=SynthEnvelopeTimeToDelta(pDecay[Program],ENVELOPE_DECAY_MAX_MS);
						SynthChannel[chn].e_stage=eStageDecay;
					}
				}
				break;

			case eStageDecay:
				{
					SynthChannel[chn].e_level-=SynthChannel[chn].e_delta;

					if(SynthChannel[chn].e_level<=pSustain[Program])
					{
						SynthChannel[chn].e_level=pSustain[Program];
						SynthChannel[chn].e_stage=eStageSustain;
					}
				}
				break;

			case eStageRelease:
				{
					SynthChannel[chn].e_level-=SynthChannel[chn].e_delta;

					if(SynthChannel[chn].e_level<=0) SynthChannel[chn].e_stage=eStageReset;
				}
				break;
			}
		}
	}
}



void PulseSynth::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float level,pulsefix,modulation,mod_step,sampleRate;
	unsigned int i,s,poly,out;
	int chn,note,prev_note;
	bool key_off;

	sampleRate=(float)updateSampleRate();

	mod_step=12.0f/sampleRate*(float)M_PI;

	poly=(int)(pPolyphony[Program]*2.99f);

	pulsefix=sampleRate*PULSE_WIDTH_MAX_US*OVERSAMPLING*pPulseWidth[Program];

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

									if(poly||prev_note<0)
									{
										SynthChannel[chn].acc=0;	//phase reset

										SynthChannel[chn].volume=((float)MidiQueue[i].velocity/100.0f);

										if(pLowBoost[Program]>=.5f)	//even out energy spread to make lower end louder
										{
											SynthChannel[chn].volume*=.021f*pow(2.0f,(127.0f-MidiQueue[i].note)/12.0f);
										}

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

			SynthChannel[chn].add=440.0f*pow(2.0f,(SynthChannel[chn].freq+modulation+MidiPitchBend+pDetune[Program]*2.0f-1.0f)/12.0f)/sampleRate/OVERSAMPLING;
		}

		level=0;

		if(poly<2)	//poly PWM
		{
			for(s=0;s<OVERSAMPLING;++s)
			{
				out=0;

				for(chn=0;chn<SYNTH_CHANNELS;++chn)
				{
					if(SynthChannel[chn].note<0) continue;

					SynthChannel[chn].acc+=SynthChannel[chn].add;

					while(SynthChannel[chn].acc>=1.0f)
					{
						SynthChannel[chn].acc-=1.0f;

						SynthChannel[0].pulse+=pulsefix*SynthChannel[chn].e_level*SynthChannel[chn].volume/1000000.0f;
					}
				}

				if(SynthChannel[0].pulse>0)
				{
					SynthChannel[0].pulse-=1.0f;

					out=1;
				}
				else
				{
					SynthChannel[0].pulse=0;
				}

				if(out) level+=1.0f/OVERSAMPLING;
			}
		}
		else	//poly ADD
		{
			for(s=0;s<OVERSAMPLING;++s)
			{
				for(chn=0;chn<SYNTH_CHANNELS;++chn)
				{
					if(SynthChannel[chn].note<0) continue;

					SynthChannel[chn].acc+=SynthChannel[chn].add;

					while(SynthChannel[chn].acc>=1.0f)
					{
						SynthChannel[chn].acc-=1.0f;

						SynthChannel[chn].pulse+=pulsefix*SynthChannel[chn].e_level*SynthChannel[chn].volume/1000000.0f;
					}

					if(SynthChannel[chn].pulse>0)
					{
						SynthChannel[chn].pulse-=1.0f;

						level+=1.0f/OVERSAMPLING/(SYNTH_CHANNELS/4);
					}
					else
					{
						SynthChannel[chn].pulse=0;
					}
				}
			}
		}

		level=level*pOutputGain[Program];

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
