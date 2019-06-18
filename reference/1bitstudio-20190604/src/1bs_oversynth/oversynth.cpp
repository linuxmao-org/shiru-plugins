#include "OverSynth.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new OverSynth(audioMaster);
}



OverSynth::OverSynth(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 chn;

	setNumInputs(NUM_INPUTS);
	setNumOutputs(NUM_OUTPUTS);
	setUniqueID(PLUGIN_UID);

	isSynth();
	canProcessReplacing();

	strcpy(ProgramName,"Default");

	pPulseWidth=.05f;
	pDetune=.5f;
	pPhaseReset=1.0f;
	pNoteCut=0;
	pVelTarget=0;
	pOutputGain=1.0f;

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].note=-1;
		SynthChannel[chn].freq=0;
		SynthChannel[chn].freq_new=0;
		SynthChannel[chn].acc=0;
		SynthChannel[chn].pulse=0;
		SynthChannel[chn].duration=0;
		SynthChannel[chn].volume=1.0f/SYNTH_CHANNELS/OVERSAMPLING;
	}

	sOutput=0;
	sVelocity=1.0f;

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

	memset(MidiKeyState,0,sizeof(MidiKeyState));

	suspend();
}



OverSynth::~OverSynth()
{
}



void OverSynth::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdPulseWidth: pPulseWidth=value; break;
	case pIdDetune:     pDetune    =value; break;
	case pIdPhaseReset: pPhaseReset=value; break;
	case pIdNoteCut:    pNoteCut   =value; updateDisplay(); break;
	case pIdVelTarget:  pVelTarget =value; break;
	case pIdOutputGain: pOutputGain=value; break;
	}
}



float OverSynth::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdPulseWidth: return pPulseWidth;
	case pIdDetune:     return pDetune;
	case pIdPhaseReset: return pPhaseReset;
	case pIdNoteCut:    return pNoteCut;
	case pIdVelTarget:  return pVelTarget;
	case pIdOutputGain: return pOutputGain;
	}

	return 0;
} 



void OverSynth::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdPulseWidth: strcpy(label,"Pulse width");     break;
	case pIdDetune:     strcpy(label,"Detune");          break;
	case pIdPhaseReset: strcpy(label,"Phase reset");     break;
	case pIdNoteCut:    strcpy(label,"Note cut");        break;
	case pIdVelTarget:  strcpy(label,"Velocity target"); break;
	case pIdOutputGain: strcpy(label,"Output gain");	 break;
	default:			strcpy(label,"");
	}
} 



void OverSynth::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdPulseWidth: float2string(PULSE_WIDTH_MAX_US*pPulseWidth,text,5); break;
	case pIdDetune:     float2string(-1.0f+pDetune*2.0f,text,5); break;
	case pIdPhaseReset: if(pPhaseReset==0) strcpy(text,"Disabled"); else float2string(pPhaseReset,text,5); break;
	case pIdNoteCut:    if(pNoteCut==0) strcpy(text,"Infinite"); else float2string(1000.0f*pNoteCut,text,5); break;
	case pIdVelTarget:  strcpy(text,pVelTargetNames[(int)(pVelTarget*2.99f)]); break;
	case pIdOutputGain: dB2string(pOutputGain,text,3); break;
	default:			strcpy(text,"");
	}
} 



void OverSynth::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdPulseWidth: strcpy(label,"us");  break;
	case pIdDetune:     strcpy(label,"");   break;
	case pIdNoteCut:    strcpy(label,(pNoteCut>0)?"ms":""); break;
	case pIdOutputGain: strcpy(label,"dB"); break;
	default:			strcpy(label,"");
	}
} 



VstInt32 OverSynth::getProgram(void)
{
	return 0;
}



void OverSynth::setProgram(VstInt32 program)
{
}



void OverSynth::getProgramName(char* name)
{
	strcpy(name,ProgramName); 
}



void OverSynth::setProgramName(char* name)
{
	strncpy(ProgramName,name,MAX_NAME_LEN);

	ProgramName[MAX_NAME_LEN-1]='\0';
} 



VstInt32 OverSynth::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 OverSynth::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 OverSynth::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



void OverSynth::MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void OverSynth::MidiAddPitchBend(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypePitchBend;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



void OverSynth::MidiAddModulation(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypeModulation;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



bool OverSynth::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



VstInt32 OverSynth::processEvents(VstEvents* ev) 
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



VstInt32 OverSynth::SynthAllocateVoice(VstInt32 note)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note<0) return chn;

	return -1;
}



void OverSynth::SynthChannelChangeNote(VstInt32 chn,VstInt32 note)
{
	SynthChannel[chn].note=note;

	if(note>=0) SynthChannel[chn].freq_new=(float)SynthChannel[chn].note-69;
}



void OverSynth::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float level,pulsefix,freq,modulation,mod_step,velocity,sampleRate;
	unsigned int i,s,vtgt;
	int chn,chna,prev_note;

	sampleRate=(float)updateSampleRate();

	mod_step=12.0f/sampleRate*(float)M_PI;

	vtgt=(int)(pVelTarget*2.99f);

	pulsefix=sampleRate*PULSE_WIDTH_MAX_US*OVERSAMPLING*pPulseWidth/1000000.0f;

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

								chn=SynthAllocateVoice(MidiQueue[i].note);

								if(chn>=0)
								{
									prev_note=SynthChannel[chn].note;

									SynthChannelChangeNote(chn,MidiQueue[i].note);

									velocity=((float)MidiQueue[i].velocity/100.0f);

									if(prev_note<0)
									{
										if(pPhaseReset>0) SynthChannel[chn].acc=pPhaseReset;	//reset phase
										SynthChannel[chn].freq=SynthChannel[chn].freq_new;
										SynthChannel[chn].duration=pNoteCut*(vtgt==2?velocity:1.0f)*sampleRate*OVERSAMPLING;
										SynthChannel[chn].pulse_add=pulsefix*(vtgt==1?velocity:1.0f);

										sVelocity=(vtgt==0?velocity:1.0f);
									}
								}
							}
							else//key off
							{
								if(MidiQueue[i].note>=0)
								{
									MidiKeyState[MidiQueue[i].note]=0;

									for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==MidiQueue[i].note) SynthChannel[chn].note=-1;
								}
								else
								{
									memset(MidiKeyState,0,sizeof(MidiKeyState));

									for(chn=0;chn<SYNTH_CHANNELS;++chn) SynthChannel[chn].note=-1;
								}

								if(!MidiIsAnyKeyDown()) sOutput=0;
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

		chna=0;

		for(chn=0;chn<SYNTH_CHANNELS;++chn)
		{
			if(SynthChannel[chn].note<0) continue;

			freq=SynthChannel[chn].freq+modulation+MidiPitchBend;

			if(chna&1) freq+=-.5f+pDetune;

			SynthChannel[chn].add=440.0f*pow(2.0f,freq/12.0f)/sampleRate/OVERSAMPLING;

			++chna;
		}

		level=0;

		for(s=0;s<OVERSAMPLING;++s)
		{
			for(chn=0;chn<SYNTH_CHANNELS;++chn)
			{
				if(SynthChannel[chn].note<0) continue;

				if(SynthChannel[chn].duration>0)
				{
					if(pNoteCut>0)
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

				while(SynthChannel[chn].acc>=1.0f)
				{
					SynthChannel[chn].acc-=1.0f;

					SynthChannel[chn].pulse+=SynthChannel[chn].pulse_add;

					if(!(chn&1)) sOutput=1; else sOutput=0;
				}

				if(SynthChannel[chn].pulse>0)
				{
					SynthChannel[chn].pulse-=1.0f;

					if(SynthChannel[chn].pulse<=0) if(!(chn&1)) sOutput=0; else sOutput=1;
				}
			}

			if(sOutput) level+=1.0f/OVERSAMPLING;
		}

		level=level*sVelocity*pOutputGain;

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
