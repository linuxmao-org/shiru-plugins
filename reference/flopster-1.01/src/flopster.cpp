#include "flopster.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new Flopster(audioMaster);
}



Flopster::Flopster(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	HMODULE hm=NULL;
	WIN32_FIND_DATA ffd;
	HANDLE hFind;
	char directory[MAX_PATH];
	VstInt32 i,c1,c2,preset;

	setNumInputs(NUM_INPUTS);
	setNumOutputs(NUM_OUTPUTS);
	setUniqueID(PLUGIN_UID);

	isSynth();
	canProcessReplacing();

	//get plugin directory

	if(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCSTR)&handle_func,&hm))

	GetModuleFileNameA(hm,PluginDir,sizeof(PluginDir));

	i=(VstInt32)strlen(PluginDir);

	while(--i)
	{
		if(PluginDir[i]=='/'||PluginDir[i]=='\\')
		{
			PluginDir[i]=0;
			break;
		}
	}

	//scan subfolders to build preset list

	Program=0;

	for(i=0;i<NUM_PROGRAMS;++i) strcpy(ProgramName[i],"empty");

	strcpy(directory,PluginDir);
	strcat(directory,"\\samples\\*");

	hFind=FindFirstFile(directory,&ffd);

	if(INVALID_HANDLE_VALUE!=hFind) 
	{
		do
		{
			if(ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				c1=ffd.cFileName[0];
				c2=ffd.cFileName[1];

				if(c1>='0'&&c1<='9'&&c2>='0'&&c2<='9')
				{
					preset=((c1-'0')*10+(c2-'0'))-1;

					if(preset>=0&&preset<NUM_PROGRAMS)
					{
						strcpy(ProgramName[preset],ffd.cFileName);
					}
				}
			}
		}
		while(FindNextFile(hFind,&ffd)!=0);

		FindClose(hFind);
	}

	//load samples

	memset(&FDD,0,sizeof(FDD));

	memset(&SampleHeadStep,0,sizeof(SampleHeadStep));
	memset(&SampleHeadBuzz,0,sizeof(SampleHeadBuzz));
	memset(&SampleHeadSeek,0,sizeof(SampleHeadSeek));

	LoadAllSamples();

	//initialize variables

	pHeadStepGain=1.0f;
	pHeadSeekGain=1.0f;
	pHeadBuzzGain=1.0f;
	pSpindleGain =0.25f;
	pNoisesGain  =0.5f;
	pOutputGain  =1.0f;

	MidiQueue.clear();

	memset(MidiKeyState,0,sizeof(MidiKeyState));

	suspend();
}



Flopster::~Flopster()
{
	FreeAllSamples();
}



void Flopster::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdHeadStepGain: pHeadStepGain=value; break;
	case pIdHeadSeekGain: pHeadSeekGain=value; break;
	case pIdHeadBuzzGain: pHeadBuzzGain=value; break;
	case pIdSpindleGain:  pSpindleGain =value; break;
	case pIdNoisesGain:   pNoisesGain  =value; break;
	case pIdOutputGain:   pOutputGain  =value; break;
	}
}



float Flopster::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdHeadStepGain: return pHeadStepGain;
	case pIdHeadSeekGain: return pHeadSeekGain;
	case pIdHeadBuzzGain: return pHeadBuzzGain;
	case pIdSpindleGain:  return pSpindleGain;
	case pIdNoisesGain:   return pNoisesGain;
	case pIdOutputGain:   return pOutputGain;
	}

	return 0;
} 



void Flopster::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdHeadStepGain: strcpy(label,"Head step gain");	 break;
	case pIdHeadSeekGain: strcpy(label,"Head seek gain");	 break;
	case pIdHeadBuzzGain: strcpy(label,"Head buzz gain");	 break;
	case pIdSpindleGain:  strcpy(label,"Spindle gain");	 break;
	case pIdNoisesGain:  strcpy(label,"Noises gain");	 break;
	case pIdOutputGain:   strcpy(label,"Output gain");	 break;
	default:			  strcpy(label,"");
	}
} 



void Flopster::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdHeadStepGain: dB2string(pHeadStepGain,text,3); break;
	case pIdHeadSeekGain: dB2string(pHeadSeekGain,text,3); break;
	case pIdHeadBuzzGain: dB2string(pHeadBuzzGain,text,3); break;
	case pIdSpindleGain:  dB2string(pSpindleGain ,text,3); break;
	case pIdNoisesGain:   dB2string(pNoisesGain  ,text,3); break;
	case pIdOutputGain:   dB2string(pOutputGain  ,text,3); break;
	default:			  strcpy(text,"");
	}
} 



void Flopster::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdHeadStepGain:
	case pIdHeadSeekGain:
	case pIdHeadBuzzGain:
	case pIdSpindleGain:
	case pIdOutputGain: strcpy(label,"dB"); break;
	default:			strcpy(label,"");
	}
} 



VstInt32 Flopster::getProgram(void)
{
	return Program;
}



void Flopster::setProgram(VstInt32 program)
{
	if(Program!=program)
	{
		Program=program;

		LoadAllSamples();
	}
}



void Flopster::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void Flopster::setProgramName(char* name)
{
	//no renaming allowed, as names used to load samples from a folder
} 



VstInt32 Flopster::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 Flopster::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 Flopster::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



void Flopster::MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



bool Flopster::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



void Flopster::ResetPlayer(void)
{
	FDD.head_dir=1;
	FDD.spindle_sample_ptr=FDD.spindle_sample.length-1;
}



void Flopster::LoadAllSamples(void)
{
	char directory[MAX_PATH];
	char filename[MAX_PATH];
	int i;

	strcpy(directory,PluginDir);
	strcat(directory,"/samples/");
	strcat(directory,ProgramName[Program]);

	FreeAllSamples();
	ResetPlayer();

	for(i=0;i<STEP_SAMPLES_ALL;++i)
	{
		sprintf(filename,"%s/step_%2.2i.wav",directory,i);

		SampleLoad(&SampleHeadStep[i],filename);
	}

	for(i=0;i<HEAD_BUZZ_RANGE;++i)
	{
		sprintf(filename,"%s/buzz_%2.2i.wav",directory,i);

		SampleLoad(&SampleHeadBuzz[i],filename);
	}

	for(i=0;i<HEAD_SEEK_RANGE;++i)
	{
		sprintf(filename,"%s/seek_%2.2i.wav",directory,i);

		SampleLoad(&SampleHeadSeek[i],filename);
	}

	sprintf(filename,"%s/push.wav",directory);

	SampleLoad(&SampleDiskPush,filename);

	sprintf(filename,"%s/insert.wav",directory);

	SampleLoad(&SampleDiskInsert,filename);

	sprintf(filename,"%s/eject.wav",directory);

	SampleLoad(&SampleDiskEject,filename);

	sprintf(filename,"%s/pull.wav",directory);

	SampleLoad(&SampleDiskPull,filename);

	sprintf(filename,"%s/spindle.wav",directory);

	SampleLoad(&FDD.spindle_sample,filename);

	ResetPlayer();
}



void Flopster::FreeAllSamples(void)
{
	int i;

	SampleFree(&FDD.spindle_sample);

	SampleFree(&SampleDiskPush);
	SampleFree(&SampleDiskInsert);
	SampleFree(&SampleDiskEject);
	SampleFree(&SampleDiskPull);

	for(i=0;i<STEP_SAMPLES_ALL;++i) SampleFree(&SampleHeadStep[i]);
	for(i=0;i<HEAD_BUZZ_RANGE; ++i) SampleFree(&SampleHeadBuzz[i]);
	for(i=0;i<HEAD_SEEK_RANGE; ++i) SampleFree(&SampleHeadSeek[i]);
}



void Flopster::SampleLoad(sampleStruct *sample,char *filename)
{
	int ptr,size,bytes,align,filesize;
	FILE *file;

	file=fopen(filename,"rb");

	if(!file) return;

	fseek(file,0,SEEK_END);
	filesize=ftell(file);
	fseek(file,0,SEEK_SET);

	sample->src=new unsigned char[filesize+8];

	fread(sample->src,filesize,1,file);

	fclose(file);

	if(memcmp(sample->src,"RIFF",4)||memcmp(sample->src+8,"WAVEfmt ",8))
	{
		SampleFree(sample);
		return;
	}

	size=sample->src[4]+(sample->src[5]<<8)+(sample->src[6]<<16)+(sample->src[7]<<24);
	align=sample->src[32]+(sample->src[33]<<8);
	bytes=sample->src[40]+(sample->src[41]<<8)+(sample->src[42]<<16)+(sample->src[43]<<24);

	sample->wave=(signed short int*)(sample->src+44);
	sample->length=bytes/align;
	sample->loop_start=0;
	sample->loop_end=0;

	ptr=44+bytes;

	while(ptr<size)
	{
		if(!memcmp(&sample->src[ptr],"smpl",4))
		{
			if(sample->src[ptr+0x24]+sample->src[ptr+0x25]+sample->src[ptr+0x26]+sample->src[ptr+0x27])
			{
				sample->loop_start=sample->src[ptr+0x34]+(sample->src[ptr+0x35]<<8)+(sample->src[ptr+0x36]<<16)+(sample->src[ptr+0x37]<<24);
				sample->loop_end  =sample->src[ptr+0x38]+(sample->src[ptr+0x39]<<8)+(sample->src[ptr+0x3a]<<16)+(sample->src[ptr+0x3b]<<24);
			}

			break;
		}

		++ptr;
	}
}



void Flopster::SampleFree(sampleStruct *sample)
{
	if(sample&&sample->src)
	{
		delete[] sample->src;

		memset(sample,0,sizeof(sampleStruct));
	}
}



float Flopster::SampleRead(sampleStruct *sample,double pos)
{
	double s1,s2,fr;
	int ptr;

	if(!sample||!sample->wave) return 0;

	ptr=(int)pos;

	fr=(pos-(double)ptr);

	s1=sample->wave[ptr]/65536.0;

	++ptr;

	if(ptr<sample->length) s2=sample->wave[ptr]/65536.0; else s2=s1;

	return (float)(s1+(s2-s1)*fr);
}



void Flopster::FloppyStartHeadSample(sampleStruct *sample,float gain,bool loop,float relative)
{
	FDD.head_sample=sample;

	FDD.head_sample_loop=loop;
	FDD.head_gain=gain;

	if(relative==0)
	{
		FDD.head_sample_ptr=0;
	}
	else
	{
		FDD.head_sample_ptr=FDD.head_sample->loop_start+relative*(FDD.head_sample->loop_end-FDD.head_sample->loop_start);
	}
}



void Flopster::FloppyStep(int pos)
{
	if(pos>=0)
	{
		FDD.head_pos=pos;
		FDD.head_dir=1;
	}

	FloppyStartHeadSample(&SampleHeadStep[FDD.head_pos],pHeadStepGain,false,0);

	FDD.head_pos+=FDD.head_dir;

	if(FDD.head_pos<=0)
	{
		FDD.head_pos=0;
		FDD.head_dir=-FDD.head_dir;
	}

	if(FDD.head_pos>=(STEP_SAMPLES_ALL-1))
	{
		FDD.head_pos=(STEP_SAMPLES_ALL-1);
		FDD.head_dir=-FDD.head_dir;
	}
}



void Flopster::FloppySpindle(bool enable)
{
	if(FDD.spindle_enable==enable) return;

	FDD.spindle_enable=enable;

	if(enable&&FDD.spindle_sample_ptr>=FDD.spindle_sample.loop_end) FDD.spindle_sample_ptr=0;	//restart if it stopping
}



VstInt32 Flopster::processEvents(VstEvents* ev) 
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
				if(midiData[1]>=0x7b)//all notes off and mono/poly mode changes that also requires to do all notes off
				{
					MidiAddNote(event->deltaFrames,-1,0);
				}
			} 
			break;
		}
	} 

	return 1; 
} 



void Flopster::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float level;
	unsigned int i;
	int note,type;
	double sample_step;
	bool prev_any,spindle_stop,head_stop,reset_low_freq;

	sample_step=44100.0/updateSampleRate();

	while(--sampleFrames>=0)
	{
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
							prev_any=MidiIsAnyKeyDown();

							if(MidiQueue[i].velocity)//key on
							{
								MidiKeyState[MidiQueue[i].note]=MidiQueue[i].velocity;
							}
							else//key off
							{
								if(MidiQueue[i].note>=0)
								{
									MidiKeyState[MidiQueue[i].note]=0;
								}
								else
								{
									memset(MidiKeyState,0,sizeof(MidiKeyState));
								}
							}

							spindle_stop=true;
							head_stop=true;

							for(note=127;note>=0;--note)
							{
								if(MidiKeyState[note])
								{
									reset_low_freq=true;

									if(note>=SPECIAL_NOTE)
									{
										switch(note)
										{
										case SPINDLE_NOTE:
											{
												spindle_stop=false;
												reset_low_freq=false;
											}
											break;

										case SINGLE_STEP_NOTE: 
											{
												FloppyStep(-1);

												MidiKeyState[note]=0;
											}
											break;

										case DISK_PUSH_NOTE:
											{
												FloppyStartHeadSample(&SampleDiskPush,pNoisesGain,false,0);

												MidiKeyState[note]=0;
											}
											break;

										case DISK_INSERT_NOTE:
											{
												FloppyStartHeadSample(&SampleDiskInsert,pNoisesGain,false,0);

												MidiKeyState[note]=0;
											}
											break;

										case DISK_EJECT_NOTE:
											{
												FloppyStartHeadSample(&SampleDiskEject,pNoisesGain,false,0);
												
												MidiKeyState[note]=0;
											}
											break;

										case DISK_PULL_NOTE:
											{
												FloppyStartHeadSample(&SampleDiskPull,pNoisesGain,false,0);

												MidiKeyState[note]=0;
											}
											break;
										}
									}
									else
									{
										type=MidiKeyState[note]*5/128;

										if(note<HEAD_BASE_NOTE&&type>1) type=1;

										switch(type)
										{
										case 0:	//just head step, not pitched
											{
												FloppyStep(note%80);
											}
											break;

										case 1:	//repeating slow steps with a pitch
											{
												if(!prev_any) FDD.low_freq_acc=1.0f;	//trigger first step right away

												FDD.low_freq_add=(440.0f*pow(2.0f,(note-69-24)/12.0f))/sampleRate;

												reset_low_freq=false;
											}
											break;

										case 2:	//head buzz
											{
												if(note>=HEAD_BASE_NOTE&&note<HEAD_BASE_NOTE+HEAD_BUZZ_RANGE)
												{
													FloppyStartHeadSample(&SampleHeadBuzz[note-HEAD_BASE_NOTE],pHeadBuzzGain,true,0);
												}
											}
											break;

										case 3:	//head seek from last position
										case 4:	//head seek from initial position
											{
												if(note>=HEAD_BASE_NOTE&&note<HEAD_BASE_NOTE+HEAD_SEEK_RANGE)
												{
													FloppyStartHeadSample(&SampleHeadSeek[note-HEAD_BASE_NOTE],pHeadSeekGain,true,type==4?0:FDD.head_sample_relative_ptr);
												}
											}
											break;
										}

										head_stop=false;

										break;
									}
								}
							}

							FloppySpindle(!spindle_stop);

							if(reset_low_freq)
							{
								FDD.low_freq_acc=0;
								FDD.low_freq_add=0;
							}

							if(head_stop)
							{
								FDD.low_freq_acc=0;
								FDD.low_freq_add=0;

								FDD.head_sample_loop=false;
							}
						}
						break;
					}
				}
			}
		}
	
		FDD.low_freq_acc+=FDD.low_freq_add;

		if(FDD.low_freq_acc>=1.0f)
		{
			while(FDD.low_freq_acc>=1.0f) FDD.low_freq_acc-=1.0f;

			FloppyStep(-1);
		}

		level=0;

		if(FDD.spindle_sample.src)
		{
			level+=SampleRead(&FDD.spindle_sample,FDD.spindle_sample_ptr)*pSpindleGain;

			FDD.spindle_sample_ptr+=sample_step;

			if(FDD.spindle_enable)
			{
				if(FDD.spindle_sample_ptr>=FDD.spindle_sample.loop_end)
				{
					FDD.spindle_sample_ptr-=(FDD.spindle_sample.loop_end-FDD.spindle_sample.loop_start);
				}
			}
			else
			{
				if(FDD.spindle_sample_ptr<FDD.spindle_sample.loop_end)
				{
					FDD.spindle_sample_ptr=FDD.spindle_sample.loop_end;
				}
			}

			if(FDD.spindle_sample_ptr>FDD.spindle_sample.length-1) FDD.spindle_sample_ptr=FDD.spindle_sample.length-1;
		}

		if(FDD.head_sample)
		{
			level+=SampleRead(FDD.head_sample,FDD.head_sample_ptr)*FDD.head_gain;

			FDD.head_sample_ptr+=sample_step;

			if(FDD.head_sample_loop)
			{
				if(FDD.head_sample_ptr>=FDD.head_sample->loop_end)
				{
					FDD.head_sample_ptr-=(FDD.head_sample->loop_end-FDD.head_sample->loop_start);
				}

				if(FDD.head_sample_ptr<FDD.head_sample->loop_start)
				{
					FDD.head_sample_relative_ptr=.5f+.5f/FDD.head_sample->loop_start*(float)FDD.head_sample_ptr;
				}
				else
				{
					FDD.head_sample_relative_ptr=1.0f/(FDD.head_sample->loop_end-FDD.head_sample->loop_start)*(float)FDD.head_sample_ptr;
				}

				FDD.head_sample_relative_ptr=floorf(FDD.head_sample_relative_ptr*160.0f)/160.0f;	//attempt to make rough steps, somewhat aligned to head clicks
			}
			else
			{
				if(FDD.head_sample_ptr<FDD.head_sample->loop_end)
				{
					FDD.head_sample_ptr=FDD.head_sample->loop_end;
				}
			}

			if(FDD.head_sample_ptr>=FDD.head_sample->length)
			{
				FDD.head_sample=NULL;
			}
		}

		level=level*2.0f*pOutputGain;	//samples were too quiet comparing to other plugins, difficult to normalize without losing relative volumes

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
