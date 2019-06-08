#include "Evalua.h"
#include "GUI.h"

#include "ev.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new Evalua(audioMaster);
}



Evalua::Evalua(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	int row,pgm,chn;

	ev = new EV();

	setNumInputs (NUM_INPUTS);
	setNumOutputs(NUM_OUTPUTS);
	setUniqueID  (PLUGIN_UID);

	isSynth();
	canProcessReplacing();
	programsAreChunks(true);

	ProgramIndex=0;
	ProgramIndexPrev=0;

	memset(Program,0,sizeof(Program));

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		sprintf(Program[pgm].Name,"%3.3u default",pgm+1);

		strncpy(Program[pgm].Data,"P&255",MAX_PROGRAM_LEN);

	}

	pgm=0;
	row=0;

	while(1)
	{
		if(PresetData[row][0]<0x20) break;

		sprintf(Program[pgm].Name,"%3.3i %s",pgm+1,PresetData[row++]);

		strncpy(Program[pgm].Data,PresetData[row++],MAX_PROGRAM_LEN);
	
		++pgm;
	}

	for(chn=0;chn< MAX_SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].note = -1;
		SynthChannel[chn].freq = 0;
		SynthChannel[chn].freq_new = 0;
	}

	Compile();

	PortaSpeed = 100;
	Polyphony = MAX_SYNTH_CHANNELS;
	OutputGain = 100;

	MidiQueue.clear();

	MidiPitchBend = 0;
	MidiPitchBendRange = 2.0f;
	MidiModulationDepth = 0;

	MidiRPNLSB=0;
	MidiRPNMSB=0;
	MidiDataLSB=0;
	MidiDataMSB=0;

	SlideStep = 0;

	memset(MidiKeyState,0,sizeof(MidiKeyState));

	suspend();

	setEditor((AEffEditor*)new GUI(this));

	SetUpdateGUI();
}



Evalua::~Evalua()
{
	delete ev;
}



void Evalua::setParameter(VstInt32 index,float value)
{
}



float Evalua::getParameter(VstInt32 index)
{
	return 0;
} 



void Evalua::getParameterName(VstInt32 index,char *label)
{
} 



void Evalua::getParameterDisplay(VstInt32 index,char *text)
{
} 



void Evalua::getParameterLabel(VstInt32 index,char *label)
{
} 



VstInt32 Evalua::getProgram(void)
{
	UpdateGUI();	//force GUI update if current program has been changed while processReplacing is not called (plugin is added but not connected in Psycle)

	return ProgramIndex;
}



void Evalua::setProgram(VstInt32 program)
{
	if(program>NUM_PROGRAMS-1) program=NUM_PROGRAMS-1;
	
	ProgramIndex = program;
}



void Evalua::getProgramName(char* name)
{
	strcpy(name,Program[ProgramIndex].Name); 
}



void Evalua::setProgramName(char* name)
{
	strncpy(Program[ProgramIndex].Name,name,MAX_NAME_LEN);

	Program[ProgramIndex].Name[MAX_NAME_LEN-1]='\0';
} 



VstInt32 Evalua::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 Evalua::getNumMidiInputChannels(void)
{
	return MIDI_CHANNELS_ALL;
}



VstInt32 Evalua::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 Evalua::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)(strlen(str)+1);
}



VstInt32 Evalua::LoadStringChunk(char *str,int max_length,float *src)
{
	VstInt32 c,ptr;

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



VstInt32 Evalua::SavePresetChunk(float *chunk)
{
	VstInt32 ptr,pgm;

	ptr=0;

	chunk[ptr++] = DATA;
	chunk[ptr++] = POLY; chunk[ptr++] = (float)Polyphony;
	chunk[ptr++] = POSP; chunk[ptr++] = (float)PortaSpeed;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++] = PROG; chunk[ptr++] = (float)pgm;

		chunk[ptr++] = NAME; ptr += SaveStringChunk(Program[pgm].Name, &chunk[ptr]);

		chunk[ptr++] = DATA; ptr += SaveStringChunk(Program[pgm].Data, &chunk[ptr]);
	}

	chunk[ptr++] = DONE;

	return ptr*sizeof(float);//size in bytes
}



void Evalua::LoadPresetChunk(float *chunk)
{
	VstInt32 pgm;
	float tag;

	pgm=0;

	while(1)
	{
		tag = *chunk++;

		if (tag == DONE) break;

		if (tag == PROG) pgm = (int)*chunk++;

		if (tag == NAME) chunk += LoadStringChunk(Program[pgm].Name, MAX_NAME_LEN, chunk);

		if (tag == DATA) chunk += LoadStringChunk(Program[pgm].Data, MAX_PROGRAM_LEN, chunk);

		if (tag == POLY) Polyphony = (VstInt32)*chunk++;
		if (tag == POSP) PortaSpeed = (VstInt32)*chunk++;
	}

	Compile();
}



VstInt32 Evalua::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 Evalua::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	SetUpdateGUI();

	return 0;
}



void Evalua::SetUpdateGUI(void)
{
	UpdateGuiFlag=true;
}



void Evalua::UpdateGUI(void)
{
	//this is needed because setProgram repeatedly called by the host to obtain all program names

	if(ProgramIndexPrev!=ProgramIndex)
	{
		ProgramIndexPrev=ProgramIndex;

		Compile();

		SetUpdateGUI();
	}

	if(UpdateGuiFlag)
	{
		UpdateGuiFlag=false;

		if(editor) ((GUI*)editor)->SetUpdate(true);

		updateDisplay();
	}
}



const char* Evalua::Compile(void)
{
	const char* err;

	err = ev->Parse(Program[ProgramIndex].Data);

	return err;
}


void Evalua::MidiAddNote(VstInt32 delta,VstInt32 channel,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.channel =channel;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void Evalua::MidiAddPitchBend(VstInt32 delta,VstInt32 channel,float depth)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypePitchBend;
	entry.channel=channel;
	entry.delta  =delta;
	entry.depth  =depth;

	MidiQueue.push_back(entry);
}



void Evalua::MidiAddModulation(VstInt32 delta, float depth)
{
	MidiQueueStruct entry;

	entry.type = mEventTypeModulation;
	entry.delta = delta;
	entry.depth = depth;

	MidiQueue.push_back(entry);
}



bool Evalua::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



void Evalua::MidiAddProgramChange(VstInt32 delta,VstInt32 channel,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.channel=channel;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



void Evalua::MidiAddControl(VstInt32 delta,VstInt32 channel,VstInt32 value)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeControl;
	entry.channel=channel;
	entry.delta  =delta;
	entry.value  =value;

	MidiQueue.push_back(entry);
}



VstInt32 Evalua::processEvents(VstEvents* ev) 
{ 
	VstMidiEvent* event;
	VstInt32 i,channel,status,note,velocity,wheel;
	char* midiData;

	for(i=0;i<ev->numEvents;++i) 
	{ 
		event=(VstMidiEvent*)ev->events[i];

		if(event->type!=kVstMidiType) continue; 

		midiData=event->midiData;

		channel=midiData[0]&0x0f;
		status =midiData[0]&0xf0;

		switch(status)
		{
		case 0x80:	//note off
		case 0x90:	//note on
			{
				note=midiData[1]&0x7f; 

				velocity=midiData[2]&0x7f; 

				if(status==0x80) velocity=0; 

				MidiAddNote(event->deltaFrames,channel,note,velocity);
			} 
			break;

		case 0xb0:	//control change
			{ 
				if(midiData[1]==0x64) MidiRPNLSB =midiData[2]&0x7f;
				if(midiData[1]==0x65) MidiRPNMSB =midiData[2]&0x7f;
				if(midiData[1]==0x26) MidiDataLSB=midiData[2]&0x7f;

				if (midiData[1] == 0x01)
				{
					MidiAddModulation(event->deltaFrames, (float)(midiData[2] & 0x7f));
				}

				if(midiData[1]==0x06)
				{
					MidiDataMSB=midiData[2]&0x7f;

					if(MidiRPNLSB==0&&MidiRPNMSB==0) MidiPitchBendRange=(float)MidiDataMSB*.5f;
				}

				if(midiData[1]>=0x7b)//all notes off and mono/poly mode changes that also requires to do all notes off
				{
					MidiAddNote(event->deltaFrames,channel,-1,0);
				}
			} 
			break;

		case 0xc0:	//program change
			{
				MidiAddProgramChange(event->deltaFrames,channel,midiData[1]&0x7f);
			}
			break;

		case 0xe0:	//pitch bend change
			{
				wheel=(midiData[1]&0x7f)|((midiData[2]&0x7f)<<7);

				MidiAddPitchBend(event->deltaFrames, channel, (float)((wheel - 0x2000)*MidiPitchBendRange / 8192.0));
			}
			break;
		}
	} 

	return 1; 
} 



VstInt32 Evalua::SynthAllocateVoice(VstInt32 midi_ch,VstInt32 note)
{
	VstInt32 chn;

	if (Polyphony == 1) return 0;	//always use ch0 in mono mode

	for(chn=0;chn<Polyphony;++chn) if(SynthChannel[chn].note==note) return chn;
	for(chn=0;chn<Polyphony;++chn) if(SynthChannel[chn].note<0) return chn;

	return -1;
}



void Evalua::SynthChannelChangeNote(VstInt32 chn,VstInt32 midi_ch,VstInt32 note,VstInt32 velocity)
{
	ChannelStruct* ch;
	VstInt32 prev_note;

	ch = &SynthChannel[chn];

	prev_note = ch->note;

	ch->note = note;

	if (note >= 0)
	{
		ch->freq_new = ch->note - 69;

		if ((velocity >= 0 && prev_note < 0) || Polyphony > 1) ch->freq = ch->freq_new;

		SlideStep = (ch->freq - ch->freq_new)*20.0*log(1.0 - ((double)PortaSpeed / 100.0));
	}

	if (velocity >= 0)
	{
		ch->velocity = velocity;
		
		ch->time_cnt = 0;
		ch->pitch_cnt = 0;
		ch->time_acc = 0;
		ch->pitch_acc = 0;
	}
}



void Evalua::SynthChannelReleaseNote(VstInt32 midi_ch,VstInt32 note)
{
	int chn;

	if(note>=0)
	{
		for(chn=0;chn< MAX_SYNTH_CHANNELS;++chn)
		{
			if(SynthChannel[chn].note==note) SynthChannel[chn].note=-1;
		}
	}
	else
	{
		for(chn=0;chn< MAX_SYNTH_CHANNELS;++chn)
		{
			SynthChannel[chn].note=-1;
		}
	}
}



void Evalua::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	ChannelStruct* ch;
	float *outL=outputs[0];
	float *outR=outputs[1];
	float output, level;
	double freq, sample_rate, time_delta, pitch_delta;
	VstInt32 i, chn, note;
	VstInt64 n;

	sample_rate = updateSampleRate();

	time_delta = 65536.0 / sample_rate;

	while(--sampleFrames>=0)
	{
		for(i=0;i<(VstInt32)MidiQueue.size();++i)
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
								MidiKeyState[MidiQueue[i].note] = 1;

								chn = SynthAllocateVoice(MidiQueue[i].channel, MidiQueue[i].note);

								if(chn>=0)
								{
									SynthChannelChangeNote(chn, MidiQueue[i].channel, MidiQueue[i].note, MidiQueue[i].velocity);
								}
							}
							else//key off
							{
								if(MidiQueue[i].note>=0)
								{
									MidiKeyState[MidiQueue[i].note] = 0;

									SynthChannelReleaseNote(MidiQueue[i].channel, MidiQueue[i].note);

									if (Polyphony > 1)
									{
										for (chn = 0; chn < MAX_SYNTH_CHANNELS; ++chn) if (SynthChannel[chn].note == MidiQueue[i].note) SynthChannelReleaseNote(MidiQueue[i].channel, MidiQueue[i].note);
									}
									else
									{
										for (note = 127; note >= 0; --note)
										{
											if (MidiKeyState[note])
											{
												SynthChannelChangeNote(0, MidiQueue[i].channel, note, -1);

												break;
											}
										}
									}
								}
								else
								{
									memset(MidiKeyState,0,sizeof(MidiKeyState));	//stop all channels

									SynthChannelReleaseNote(-1,-1);
								}
							}
						}
						break;

					case mEventTypePitchBend:
						{
							MidiPitchBend = MidiQueue[i].depth;
						}
						break;

					case mEventTypeModulation:
						{
							MidiModulationDepth = (VstInt64)MidiQueue[i].depth;
						}
						break;

					case mEventTypeProgram:
						{
							if(ProgramIndex!=MidiQueue[i].program)
							{
								ProgramIndex=MidiQueue[i].program;

								Compile();

								SetUpdateGUI();
							}
						}
						break;

					case mEventTypeControl:
						{			
						}
						break;
					}
				}
			}
		}

		//generate sound, update channels if needed

		ev->ClearVars();

		level = 0;

		for (chn = 0; chn < MAX_SYNTH_CHANNELS; ++chn)
		{
			ch = &SynthChannel[chn];

			if (ch->note < 0) continue;

			if (PortaSpeed >= MAX_PORTA_SPEED)
			{
				ch->freq = ch->freq_new;
			}
			else
			{
				if (ch->freq < ch->freq_new)
				{
					ch->freq += SlideStep / sampleRate;

					if (ch->freq > ch->freq_new) ch->freq = ch->freq_new;
				}

				if (ch->freq > ch->freq_new)
				{
					ch->freq += SlideStep / sampleRate;

					if (ch->freq < ch->freq_new) ch->freq = ch->freq_new;
				}
			}

			freq = ch->freq + MidiPitchBend;

			pitch_delta = 440.0*pow(2.0, freq / 12.0) * 256.0 / sample_rate;	//256 considered a full waveform period

			ch->pitch_acc += pitch_delta;

			while (ch->pitch_acc >= 1.0)
			{
				ch->pitch_acc -= 1.0;
				ch->pitch_cnt += 1;
			}

			ch->time_acc += time_delta;

			while (ch->time_acc >= 1.0)
			{
				ch->time_acc -= 1.0;
				ch->time_cnt += 1;
			}

			ev->SetVar('T', ch->time_cnt);
			ev->SetVar('P', ch->pitch_cnt);
			ev->SetVar('V', ch->velocity);
			ev->SetVar('M', MidiModulationDepth);
			ev->SetVar('R', rand());

			n = ev->Solve();

			if (n < -256) n = -256;
			if (n > 256) n = 256;

			n = n * OutputGain / NORMAL_GAIN;

			output = (float)n / 256.0f;

			level += output;
		}

		level = level * .25f;

		if (level < -1.0f) level = -1.0f;
		if (level > 1.0f) level = 1.0f;

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();

	UpdateGUI();
}
