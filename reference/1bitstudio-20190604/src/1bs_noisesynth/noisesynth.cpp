#include "NoiseSynth.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new NoiseSynth(audioMaster);
}



NoiseSynth::NoiseSynth(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 i,pgm,chn;

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
		pDurationA   [pgm]=0.2f;
		pOscBaseA    [pgm]=0.5f;
		pOscTypeA    [pgm]=0;
		pOscSlideA   [pgm]=0.5f;
		pPeriodA     [pgm]=1.0f;
		pSeedA       [pgm]=0;
		pDurationB   [pgm]=0.0f;
		pOscBaseB    [pgm]=0.5f;
		pOscTypeB    [pgm]=0;
		pOscSlideB   [pgm]=0.5f;
		pPeriodB     [pgm]=1.0f;
		pSeedB       [pgm]=0;
		pMixMode     [pgm]=0;
		pOscDetune	 [pgm]=0.5f;
		pPolyphony   [pgm]=.26f;
		pPortaSpeed  [pgm]=1.0f;
		pNoteCut     [pgm]=0;
		pVelTarget   [pgm]=0;
		pOutputGain  [pgm]=1.0f;
*/
	}

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].note=-1;
		SynthChannel[chn].freq=0;
		SynthChannel[chn].freq_new=0;
		SynthChannel[chn].accA=0;
		SynthChannel[chn].accB=0;
		SynthChannel[chn].outA=0;
		SynthChannel[chn].outB=0;
		SynthChannel[chn].ptrA=0;
		SynthChannel[chn].ptrB=0;
		SynthChannel[chn].periodA=0;
		SynthChannel[chn].periodB=0;
		SynthChannel[chn].durationA=0;
		SynthChannel[chn].durationB=0;
		SynthChannel[chn].slideA=0;
		SynthChannel[chn].slideB=0;
		SynthChannel[chn].slideDeltaA=0;
		SynthChannel[chn].slideDeltaB=0;
		SynthChannel[chn].detune=0;
		SynthChannel[chn].volume=1.0f/SYNTH_CHANNELS/OVERSAMPLING;
		SynthChannel[chn].duration=0;
	}

	srand(1);

	for(i=0;i<sizeof(Noise);++i) Noise[i]=rand();

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



NoiseSynth::~NoiseSynth()
{
}



int NoiseSynth::FloatToNoisePeriod(float value)
{
	const int period[]={8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536};

	return period[(int)(value*13.99f)];
}



void NoiseSynth::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdDurationA:		pDurationA   [Program]=value; updateDisplay(); break;
	case pIdDurationB:		pDurationB   [Program]=value; updateDisplay(); break;
	case pIdOscBaseA:		pOscBaseA    [Program]=value; break;
	case pIdOscBaseB:		pOscBaseB    [Program]=value; break;
	case pIdOscTypeA:		pOscTypeA    [Program]=value; updateDisplay(); break;
	case pIdOscTypeB:		pOscTypeB    [Program]=value; updateDisplay(); break;
	case pIdOscSlideA:		pOscSlideA   [Program]=value; break;
	case pIdOscSlideB:		pOscSlideB   [Program]=value; break;
	case pIdPeriodA:		pPeriodA     [Program]=value; break;
	case pIdPeriodB:		pPeriodB     [Program]=value; break;
	case pIdSeedA:			pSeedA       [Program]=value; break;
	case pIdSeedB:			pSeedB       [Program]=value; break;
	case pIdMixMode:		pMixMode     [Program]=value; break;
	case pIdOscDetune:		pOscDetune   [Program]=value; break;
	case pIdPolyphony:		pPolyphony   [Program]=value; break;
	case pIdPortaSpeed:		pPortaSpeed  [Program]=value; break;
	case pIdNoteCut:		pNoteCut     [Program]=value; updateDisplay(); break;
	case pIdVelTarget:		pVelTarget   [Program]=value; break;
	case pIdOutputGain:		pOutputGain  [Program]=value; break;
	}
}



float NoiseSynth::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdDurationA:		return pDurationA   [Program];
	case pIdDurationB:		return pDurationB   [Program];
	case pIdOscBaseA:		return pOscBaseA    [Program];
	case pIdOscBaseB:		return pOscBaseB    [Program];
	case pIdOscTypeA:		return pOscTypeA    [Program];
	case pIdOscTypeB:		return pOscTypeB    [Program];
	case pIdOscSlideA:		return pOscSlideA   [Program];
	case pIdOscSlideB:		return pOscSlideB   [Program];
	case pIdPeriodA:		return pPeriodA     [Program];
	case pIdPeriodB:		return pPeriodB     [Program];
	case pIdSeedA:			return pSeedA       [Program];
	case pIdSeedB:			return pSeedB       [Program];
	case pIdMixMode:		return pMixMode     [Program];
	case pIdOscDetune:		return pOscDetune   [Program];
	case pIdPolyphony:		return pPolyphony   [Program];
	case pIdPortaSpeed:		return pPortaSpeed  [Program];
	case pIdNoteCut:		return pNoteCut     [Program];
	case pIdVelTarget:		return pVelTarget   [Program];
	case pIdOutputGain:		return pOutputGain  [Program];
	}

	return 0;
} 



void NoiseSynth::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdDurationA:		strcpy(label,"Duration A");			break;
	case pIdDurationB:		strcpy(label,"Duration B");			break;
	case pIdOscBaseA:		strcpy(label,"Base pitch A");		break;
	case pIdOscBaseB:		strcpy(label,"Base pitch B");		break;
	case pIdOscTypeA:		strcpy(label,"Osc type A");			break;
	case pIdOscTypeB:		strcpy(label,"Osc type B");			break;
	case pIdOscSlideA:		strcpy(label,"Pitch slide A");		break;
	case pIdOscSlideB:		strcpy(label,"Pitch slide B");		break;
	case pIdPeriodA:		strcpy(label,"Noise period A");		break;
	case pIdPeriodB:		strcpy(label,"Noise period B");		break;
	case pIdSeedA:			strcpy(label,"Noise seed A");		break;
	case pIdSeedB:			strcpy(label,"Noise seed B");		break;
	case pIdMixMode:		strcpy(label,"Osc A/B mix");		break;
	case pIdOscDetune:		strcpy(label,"Osc A/B detune");		break;
	case pIdPolyphony:		strcpy(label,"Polyphony");			break;
	case pIdPortaSpeed:		strcpy(label,"Porta speed");		break;
	case pIdNoteCut:		strcpy(label,"Note cut");			break;
	case pIdVelTarget:		strcpy(label,"Velocity target");	break;
	case pIdOutputGain:		strcpy(label,"Output gain");		break;
	default:				strcpy(label,"");
	}
} 



void NoiseSynth::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdDurationA:		if(pDurationA[Program]<1.0f) float2string(NOISE_MAX_DURATION_MS*pDurationA[Program],text,6); else strcpy(text,"Infinite"); break;
	case pIdDurationB:		if(pDurationB[Program]<1.0f) float2string(NOISE_MAX_DURATION_MS*pDurationB[Program],text,6); else strcpy(text,"Infinite"); break;
	case pIdOscBaseA:		if(pOscTypeA[Program]*2.99f>=2.0f) float2string(pOscBaseA[Program]*LFO_MAX_FREQ_HZ,text,5); else sprintf(text,"%s",noteNames[(int)(pOscBaseA[Program]*96.0f)]); break;
	case pIdOscBaseB:		if(pOscTypeB[Program]*2.99f>=2.0f) float2string(pOscBaseB[Program]*LFO_MAX_FREQ_HZ,text,5); else sprintf(text,"%s",noteNames[(int)(pOscBaseB[Program]*96.0f)]); break;
	case pIdOscTypeA:		strcpy(text,pOscTypeNames[(int)(pOscTypeA[Program]*2.99f)]); break;
	case pIdOscTypeB:		strcpy(text,pOscTypeNames[(int)(pOscTypeB[Program]*2.99f)]); break;
	case pIdOscSlideA:		float2string(pOscSlideA[Program]*2.0f-1.0f,text,5); break;
	case pIdOscSlideB:		float2string(pOscSlideB[Program]*2.0f-1.0f,text,5); break;
	case pIdPeriodA:		sprintf(text,"%i",FloatToNoisePeriod(pPeriodA[Program])); break;
	case pIdPeriodB:		sprintf(text,"%i",FloatToNoisePeriod(pPeriodB[Program])); break;
	case pIdSeedA:			if(pSeedA[Program]>0) sprintf(text,"%i",(int)(pSeedA[Program]*65536.0f)); else strcpy(text,"Free running"); break;
	case pIdSeedB:			if(pSeedB[Program]>0) sprintf(text,"%i",(int)(pSeedB[Program]*65536.0f)); else strcpy(text,"Free running"); break;
	case pIdMixMode:		strcpy(text,pMixModeNames[(int)(pMixMode[Program]*2.99f)]); break;
	case pIdOscDetune:		float2string((pOscDetune[Program]-.5f)*DETUNE_RANGE_HZ,text,5); break;
	case pIdPolyphony:		strcpy(text,pPolyMixModeNames[(int)(pPolyphony[Program]*3.99f)]); break;
	case pIdPortaSpeed:		float2string(pPortaSpeed[Program],text,5); break;
	case pIdNoteCut:		if(pNoteCut[Program]==0) strcpy(text,"Infinite"); else float2string(1000.0f*pNoteCut[Program],text,5); break;
	case pIdVelTarget:		strcpy(text,pVelTargetNames[(int)(pVelTarget[Program]*2.99f)]); break;
	case pIdOutputGain:		dB2string(pOutputGain[Program],text,3); break;
	default:				strcpy(text,"");
	}
} 



void NoiseSynth::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdDurationA:  strcpy(label,(pDurationA[Program]<1.0f)?"ms":""); break;
	case pIdDurationB:  strcpy(label,(pDurationB[Program]<1.0f)?"ms":""); break;
	case pIdOscBaseA:	strcpy(label,(pOscTypeA[Program]*2.99f>=2.0f)?"Hz":""); break;
	case pIdOscBaseB:	strcpy(label,(pOscTypeB[Program]*2.99f>=2.0f)?"Hz":""); break;
	case pIdOscDetune:	strcpy(label,"Hz"); break;
	case pIdNoteCut:    strcpy(label,(pNoteCut[Program]>0)?"ms":""); break;
	case pIdOutputGain: strcpy(label,"dB"); break;
	default:            strcpy(label,"");
	}
} 



VstInt32 NoiseSynth::getProgram(void)
{
	return Program;
}



void NoiseSynth::setProgram(VstInt32 program)
{
	Program=program;
}



void NoiseSynth::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void NoiseSynth::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 NoiseSynth::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 NoiseSynth::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 NoiseSynth::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 NoiseSynth::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 NoiseSynth::LoadStringChunk(char *str,int max_length,float *src)
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



VstInt32 NoiseSynth::SavePresetChunk(float *chunk)
{
	int ptr,pgm;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		chunk[ptr++]=DURA; chunk[ptr++]=pDurationA [pgm];
		chunk[ptr++]=DURB; chunk[ptr++]=pDurationB [pgm];
		chunk[ptr++]=BASA; chunk[ptr++]=pOscBaseA  [pgm];
		chunk[ptr++]=BASB; chunk[ptr++]=pOscBaseB  [pgm];
		chunk[ptr++]=OFFA; chunk[ptr++]=pOscTypeA  [pgm];
		chunk[ptr++]=OFFB; chunk[ptr++]=pOscTypeB  [pgm];
		chunk[ptr++]=SLDA; chunk[ptr++]=pOscSlideA [pgm];
		chunk[ptr++]=SLDB; chunk[ptr++]=pOscSlideB [pgm];
		chunk[ptr++]=PRDA; chunk[ptr++]=pPeriodA   [pgm];
		chunk[ptr++]=PRDB; chunk[ptr++]=pPeriodB   [pgm];
		chunk[ptr++]=SEEA; chunk[ptr++]=pSeedA     [pgm];
		chunk[ptr++]=SEEB; chunk[ptr++]=pSeedB     [pgm];
		chunk[ptr++]=MODE; chunk[ptr++]=pMixMode   [pgm];
		chunk[ptr++]=DETU; chunk[ptr++]=pOscDetune [pgm];
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

		fprintf(file,"DURA,%1.3ff,",pDurationA   [pgm]);
		fprintf(file,"DURB,%1.3ff,",pDurationB   [pgm]);
		fprintf(file,"BASA,%1.3ff,",pOscBaseA    [pgm]);
		fprintf(file,"BASB,%1.3ff,",pOscBaseB    [pgm]);
		fprintf(file,"OFFA,%1.3ff,",pOscTypeA    [pgm]);
		fprintf(file,"OFFB,%1.3ff,",pOscTypeB    [pgm]);
		fprintf(file,"SLDA,%1.3ff,",pOscSlideA   [pgm]);
		fprintf(file,"SLDB,%1.3ff,",pOscSlideB   [pgm]);
		fprintf(file,"PRDA,%1.3ff,",pPeriodA     [pgm]);
		fprintf(file,"PRDB,%1.3ff,",pPeriodB     [pgm]);
		fprintf(file,"SEEA,%1.3ff,",pSeedA       [pgm]);
		fprintf(file,"SEEB,%1.3ff,",pSeedB       [pgm]);
		fprintf(file,"MODE,%1.3ff,",pMixMode     [pgm]);
		fprintf(file,"DETU,%1.3ff,",pOscDetune   [pgm]);
		fprintf(file,"POLY,%1.3ff,",pPolyphony   [pgm]);
		fprintf(file,"POSP,%1.3ff,",pPortaSpeed  [pgm]);
		fprintf(file,"NCUT,%1.3ff,",pNoteCut     [pgm]);
		fprintf(file,"VTGT,%1.3ff,",pVelTarget   [pgm]);
		fprintf(file,"GAIN,%1.3ff,",pOutputGain  [pgm]);

		fprintf(file,"\n");
	}

	fprintf(file,"\nDONE");
	fclose(file);
*/

	return ptr*sizeof(float);//size in bytes
}



void NoiseSynth::LoadPresetChunk(float *chunk)
{
	int pgm;
	float tag;

		while(1)
		{
			tag=*chunk++;

			if(tag==DONE) break;

			if(tag==PROG) pgm =(int)*chunk++;

			if(tag==NAME) chunk+=LoadStringChunk(ProgramName[pgm],MAX_NAME_LEN,chunk);

			if(tag==DURA) pDurationA [pgm]=*chunk++;
			if(tag==DURB) pDurationB [pgm]=*chunk++;
			if(tag==BASA) pOscBaseA  [pgm]=*chunk++;
			if(tag==BASB) pOscBaseB  [pgm]=*chunk++;
			if(tag==OFFA) pOscTypeA  [pgm]=*chunk++;
			if(tag==OFFB) pOscTypeB  [pgm]=*chunk++;
			if(tag==SLDA) pOscSlideA [pgm]=*chunk++;
			if(tag==SLDB) pOscSlideB [pgm]=*chunk++;
			if(tag==PRDA) pPeriodA   [pgm]=*chunk++;
			if(tag==PRDB) pPeriodB   [pgm]=*chunk++;
			if(tag==SEEA) pSeedA     [pgm]=*chunk++;
			if(tag==SEEB) pSeedB     [pgm]=*chunk++;
			if(tag==MODE) pMixMode   [pgm]=*chunk++;
			if(tag==DETU) pOscDetune [pgm]=*chunk++;
			if(tag==POLY) pPolyphony [pgm]=*chunk++;
			if(tag==POSP) pPortaSpeed[pgm]=*chunk++;
			if(tag==NCUT) pNoteCut   [pgm]=*chunk++;
			if(tag==VTGT) pVelTarget [pgm]=*chunk++;
			if(tag==GAIN) pOutputGain[pgm]=*chunk++;
		}
}



VstInt32 NoiseSynth::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 NoiseSynth::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	return 0;
}



void NoiseSynth::MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void NoiseSynth::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



void NoiseSynth::MidiAddPitchBend(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypePitchBend;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



void NoiseSynth::MidiAddModulation(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypeModulation;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



bool NoiseSynth::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



VstInt32 NoiseSynth::processEvents(VstEvents* ev) 
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



VstInt32 NoiseSynth::SynthAllocateVoice(VstInt32 note)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==note) return chn;
	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note<0) return chn;

	return -1;
}



void NoiseSynth::SynthChannelChangeNote(VstInt32 chn,VstInt32 note)
{
	SynthChannel[chn].note=note;

	if(note>=0)
	{
		SynthChannel[chn].freq_new=(float)SynthChannel[chn].note-69;

		sSlideStep=(SynthChannel[chn].freq-SynthChannel[chn].freq_new)*20.0f*log(1.0f-pPortaSpeed[Program]);
	}
}



void NoiseSynth::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float freq,level,velocity,modulation,mod_step,sampleRate;
	unsigned int i,s,out,outn,osc_mode,vtgt,poly_mode;
	int chn,note,prev_note;
	int osc_type_a,osc_type_b;

	sampleRate=(float)updateSampleRate();

	mod_step=12.0f/sampleRate*(float)M_PI;

	osc_mode  =(int)(pMixMode  [Program]*2.99f);
	poly_mode =(int)(pPolyphony[Program]*3.99f);
	osc_type_a=(int)(pOscTypeA [Program]*2.99f);
	osc_type_b=(int)(pOscTypeB [Program]*2.99f);

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
										SynthChannel[chn].accA=0;	//phase reset
										SynthChannel[chn].accB=0;	//phase reset

										SynthChannel[chn].ptrA=0;
										SynthChannel[chn].ptrB=0;

										if(pSeedA[Program]>0) SynthChannel[chn].seedA=(VstInt32)(pSeedA[Program]*65536.0f); else SynthChannel[chn].seedA=rand();
										if(pSeedB[Program]>0) SynthChannel[chn].seedB=(VstInt32)(pSeedB[Program]*65536.0f); else SynthChannel[chn].seedB=rand();

										SynthChannel[chn].outA=0;
										SynthChannel[chn].outB=0;

										SynthChannel[chn].periodA=FloatToNoisePeriod(pPeriodA[Program])-1;
										SynthChannel[chn].periodB=FloatToNoisePeriod(pPeriodB[Program])-1;
										SynthChannel[chn].freq=SynthChannel[chn].freq_new;
										SynthChannel[chn].volume=(vtgt==0?velocity:1.0f)/SYNTH_CHANNELS/OVERSAMPLING;
										SynthChannel[chn].duration=pNoteCut[Program]*(vtgt==2?velocity:1.0f)*sampleRate*OVERSAMPLING;
										SynthChannel[chn].durationA=pDurationA[Program]*sampleRate*OVERSAMPLING;
										SynthChannel[chn].durationB=pDurationB[Program]*sampleRate*OVERSAMPLING;

										SynthChannel[chn].slideA=0;
										SynthChannel[chn].slideB=0;
										SynthChannel[chn].slideDeltaA=(pOscSlideA[Program]-.5f)*100000.0f/sampleRate/OVERSAMPLING;
										SynthChannel[chn].slideDeltaB=(pOscSlideB[Program]-.5f)*100000.0f/sampleRate/OVERSAMPLING;
									}

									SynthChannel[chn].detune=(vtgt==1?velocity:1.0f)*(pOscDetune[Program]-.5f)*DETUNE_RANGE_HZ;
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

			switch(osc_type_a)
			{
			case 0: freq=440.0f*pow(2.0f,(SynthChannel[chn].freq+floorf(pOscBaseA[Program]*96.0f)+modulation+MidiPitchBend)/12.0f); break;
			case 1: freq=440.0f*pow(2.0f,floorf(pOscBaseA[Program]*96.0f)/12.0f); break;
			case 2: freq=pOscBaseA[Program]*LFO_MAX_FREQ_HZ;
			}

			SynthChannel[chn].addA=(freq+SynthChannel[chn].slideA)/sampleRate/OVERSAMPLING;

			switch(osc_type_b)
			{
			case 0: freq=440.0f*pow(2.0f,(SynthChannel[chn].freq+floorf(pOscBaseB[Program]*96.0f)+modulation+MidiPitchBend)/12.0f); break;
			case 1: freq=440.0f*pow(2.0f,floorf(pOscBaseB[Program]*96.0f)/12.0f); break;
			case 2: freq=pOscBaseB[Program]*LFO_MAX_FREQ_HZ;
			}

			SynthChannel[chn].addB=(freq+SynthChannel[chn].slideB+SynthChannel[chn].detune)/sampleRate/OVERSAMPLING;
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

				if(SynthChannel[chn].durationA>0||pDurationA[Program]>=1.0f)
				{
					SynthChannel[chn].durationA-=1.0f;

					if(SynthChannel[chn].durationA<0) SynthChannel[chn].durationA=0;

					SynthChannel[chn].accA+=SynthChannel[chn].addA;

					while(SynthChannel[chn].accA>=1.0f)
					{
						SynthChannel[chn].accA-=1.0f;

						SynthChannel[chn].outA=(Noise[(SynthChannel[chn].seedA+((SynthChannel[chn].ptrA>>3)&SynthChannel[chn].periodA))&(sizeof(Noise)-1)]>>((SynthChannel[chn].seedA+SynthChannel[chn].ptrA)&7))&1;

						++SynthChannel[chn].ptrA;
					}
				}
				else
				{
					SynthChannel[chn].outA=0;
				}

				if(SynthChannel[chn].durationB>0||pDurationB[Program]>=1.0f)
				{
					SynthChannel[chn].durationB-=1.0f;

					if(SynthChannel[chn].durationB<0) SynthChannel[chn].durationB=0;

					SynthChannel[chn].accB+=SynthChannel[chn].addB;

					while(SynthChannel[chn].accB>=1.0f)
					{
						SynthChannel[chn].accB-=1.0f;

						SynthChannel[chn].outB=(Noise[(SynthChannel[chn].seedB+((SynthChannel[chn].ptrB>>3)&SynthChannel[chn].periodB))&(sizeof(Noise)-1)]>>((SynthChannel[chn].seedB+SynthChannel[chn].ptrB)&7))&1;

						++SynthChannel[chn].ptrB;
					}
				}
				else
				{
					SynthChannel[chn].outB=0;
				}

				SynthChannel[chn].slideA+=SynthChannel[chn].slideDeltaA;
				SynthChannel[chn].slideB+=SynthChannel[chn].slideDeltaB;

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
