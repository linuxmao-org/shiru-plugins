#include "SerialSynth.h"



AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new SerialSynth(audioMaster);
}



SerialSynth::SerialSynth(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
	VstInt32 pgm,chn;

	setNumInputs(NUM_INPUTS);
	setNumOutputs(NUM_OUTPUTS);
	setUniqueID(PLUGIN_UID);

	isSynth();
	canProcessReplacing();
	programsAreChunks(true);

	Program=0;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		strcpy(ProgramName[pgm],programDefaultNames[pgm]);

		pOsc1Active[pgm]=0;
		pOsc2Active[pgm]=0;
		pOsc3Active[pgm]=0;
		pOsc1Mode  [pgm]=1.0f;
		pOsc2Mode  [pgm]=1.0f;
		pOsc3Mode  [pgm]=1.0f;
		pOsc1Note  [pgm]=.5f;
		pOsc2Note  [pgm]=.5f;
		pOsc3Note  [pgm]=.5f;
		pOsc1Detune[pgm]=.5f;
		pOsc2Detune[pgm]=.5f;
		pOsc3Detune[pgm]=.5f;
		pOsc1Phase [pgm]=0;
		pOsc2Phase [pgm]=0;
		pOsc3Phase [pgm]=0;
		pOsc1Output[pgm]=1.0f;
		pOsc2Output[pgm]=1.0f;
		pOsc3Output[pgm]=1.0f;
		pOscMixMode[pgm]=0;
		pPolyphony [pgm]=1.0f;
		pPortaSpeed[pgm]=1.0f;
		pNoteCut   [pgm]=0;
		pVelTarget [pgm]=0;
		pOutputGain[pgm]=1.0f;
	}

	LoadPresetChunk((float*)ChunkPresetData);

	for(chn=0;chn<SYNTH_CHANNELS;++chn)
	{
		SynthChannel[chn].note=-1;
		SynthChannel[chn].freq=0;
		SynthChannel[chn].freq_new=0;
		SynthChannel[chn].acc1=0;
		SynthChannel[chn].acc2=0;
		SynthChannel[chn].acc3=0;
		SynthChannel[chn].out1=0;
		SynthChannel[chn].out2=0;
		SynthChannel[chn].out3=0;
		SynthChannel[chn].volume=1.0f/SYNTH_CHANNELS/OVERSAMPLING;
		SynthChannel[chn].duration=0;
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



SerialSynth::~SerialSynth()
{
}



float SerialSynth::FloatToRelativeNote(float n)
{
	return floor((n-.5f)*96.0f);
}



float SerialSynth::FloatToAbsoluteNote(float n)
{
	return floor(n*96.0f);
}



void SerialSynth::setParameter(VstInt32 index,float value)
{
	switch(index)
	{
	case pIdOsc1Active: pOsc1Active[Program]=value; break;
	case pIdOsc2Active: pOsc2Active[Program]=value; break;
	case pIdOsc3Active: pOsc3Active[Program]=value; break;
	case pIdOsc1Mode:   pOsc1Mode  [Program]=value; updateDisplay(); break;
	case pIdOsc2Mode:   pOsc2Mode  [Program]=value; updateDisplay(); break;
	case pIdOsc3Mode:   pOsc3Mode  [Program]=value; updateDisplay(); break;
	case pIdOsc1Note:   pOsc1Note  [Program]=value; break;
	case pIdOsc2Note:   pOsc2Note  [Program]=value; break;
	case pIdOsc3Note:   pOsc3Note  [Program]=value; break;
	case pIdOsc1Detune: pOsc1Detune[Program]=value; break;
	case pIdOsc2Detune: pOsc2Detune[Program]=value; break;
	case pIdOsc3Detune: pOsc3Detune[Program]=value; break;
	case pIdOsc1Phase:  pOsc1Phase [Program]=value; break;
	case pIdOsc2Phase:  pOsc2Phase [Program]=value; break;
	case pIdOsc3Phase:  pOsc3Phase [Program]=value; break;
	case pIdOsc1Output: pOsc1Output[Program]=value; break;
	case pIdOsc2Output: pOsc2Output[Program]=value; break;
	case pIdOsc3Output: pOsc3Output[Program]=value; break;
	case pIdOscMixMode: pOscMixMode[Program]=value; break;
	case pIdPolyphony:  pPolyphony [Program]=value; break;
	case pIdPortaSpeed: pPortaSpeed[Program]=value; break;
	case pIdNoteCut:    pNoteCut   [Program]=value; updateDisplay(); break;
	case pIdVelTarget:  pVelTarget [Program]=value; break;
	case pIdOutputGain: pOutputGain[Program]=value; break;
	}
}



float SerialSynth::getParameter(VstInt32 index)
{
	switch(index)
	{
	case pIdOsc1Active: return pOsc1Active[Program];
	case pIdOsc2Active: return pOsc2Active[Program];
	case pIdOsc3Active: return pOsc3Active[Program];
	case pIdOsc1Mode:   return pOsc1Mode  [Program];
	case pIdOsc2Mode:   return pOsc2Mode  [Program];
	case pIdOsc3Mode:   return pOsc3Mode  [Program];
	case pIdOsc1Note:   return pOsc1Note  [Program];
	case pIdOsc2Note:   return pOsc2Note  [Program];
	case pIdOsc3Note:   return pOsc3Note  [Program];
	case pIdOsc1Detune: return pOsc1Detune[Program];
	case pIdOsc2Detune: return pOsc2Detune[Program];
	case pIdOsc3Detune: return pOsc3Detune[Program];
	case pIdOsc1Phase:  return pOsc1Phase [Program];
	case pIdOsc2Phase:  return pOsc2Phase [Program];
	case pIdOsc3Phase:  return pOsc3Phase [Program];
	case pIdOsc1Output: return pOsc1Output[Program];
	case pIdOsc2Output: return pOsc2Output[Program];
	case pIdOsc3Output: return pOsc3Output[Program];
	case pIdOscMixMode: return pOscMixMode[Program];
	case pIdPolyphony:  return pPolyphony [Program];
	case pIdPortaSpeed: return pPortaSpeed[Program];
	case pIdNoteCut:    return pNoteCut   [Program];
	case pIdVelTarget:  return pVelTarget [Program];
	case pIdOutputGain: return pOutputGain[Program];
	}

	return 0;
} 



void SerialSynth::getParameterName(VstInt32 index,char *label)
{
	switch(index)
	{
	case pIdOsc1Active: strcpy(label,"Osc 1 enable");    break;
	case pIdOsc2Active: strcpy(label,"Osc 2 enable");    break;
	case pIdOsc3Active: strcpy(label,"Osc 3 enable");    break;
	case pIdOsc1Mode:   strcpy(label,"Osc 1 mode");      break;
	case pIdOsc2Mode:   strcpy(label,"Osc 2 mode");      break;
	case pIdOsc3Mode:   strcpy(label,"Osc 3 mode");      break;
	case pIdOsc1Note:   strcpy(label,"Osc 1 note");      break;
	case pIdOsc2Note:   strcpy(label,"Osc 2 note");      break;
	case pIdOsc3Note:   strcpy(label,"Osc 3 note");      break;
	case pIdOsc1Detune: strcpy(label,"Osc 1 detune");    break;
	case pIdOsc2Detune: strcpy(label,"Osc 2 detune");    break;
	case pIdOsc3Detune: strcpy(label,"Osc 3 detune");    break;
	case pIdOsc1Phase:  strcpy(label,"Osc 1 phase");     break;
	case pIdOsc2Phase:  strcpy(label,"Osc 2 phase");     break;
	case pIdOsc3Phase:  strcpy(label,"Osc 3 phase");     break;
	case pIdOsc1Output: strcpy(label,"Osc 1 output");    break;
	case pIdOsc2Output: strcpy(label,"Osc 2 output");    break;
	case pIdOsc3Output: strcpy(label,"Osc 3 output");    break;
	case pIdOscMixMode: strcpy(label,"Osc mix mode");    break;
	case pIdPolyphony:  strcpy(label,"Polyphony");       break;
	case pIdPortaSpeed: strcpy(label,"Porta speed");     break;
	case pIdNoteCut:    strcpy(label,"Note cut");        break;
	case pIdVelTarget:  strcpy(label,"Velocity target"); break;
	case pIdOutputGain: strcpy(label,"Output gain");     break;
	default:            strcpy(label,"");
	}
} 



void SerialSynth::getParameterDisplay(VstInt32 index,char *text)
{
	switch(index)
	{
	case pIdOsc1Active: strcpy(text,pOsc1ActiveNames[(int)(pOsc1Active[Program]*2.99f)]); break;
	case pIdOsc2Active: strcpy(text,pOsc2ActiveNames[(int)(pOsc2Active[Program]*2.99f)]); break;
	case pIdOsc3Active: strcpy(text,pOsc3ActiveNames[(int)(pOsc3Active[Program]*2.99f)]); break;
	case pIdOsc1Mode:   strcpy(text,(pOsc1Mode[Program]<.5f)?"Fixed":"Relative"); break;
	case pIdOsc2Mode:   strcpy(text,(pOsc2Mode[Program]<.5f)?"Fixed":"Relative"); break;
	case pIdOsc3Mode:   strcpy(text,(pOsc3Mode[Program]<.5f)?"Fixed":"Relative"); break;
	case pIdOsc1Note:   if(pOsc1Mode[Program]>=.5f) sprintf(text,"%i",(int)FloatToRelativeNote(pOsc1Note[Program])); else strcpy(text,noteNames[(int)FloatToAbsoluteNote(pOsc1Note[Program])]); break;
	case pIdOsc2Note:   if(pOsc2Mode[Program]>=.5f) sprintf(text,"%i",(int)FloatToRelativeNote(pOsc2Note[Program])); else strcpy(text,noteNames[(int)FloatToAbsoluteNote(pOsc2Note[Program])]); break;
	case pIdOsc3Note:   if(pOsc3Mode[Program]>=.5f) sprintf(text,"%i",(int)FloatToRelativeNote(pOsc3Note[Program])); else strcpy(text,noteNames[(int)FloatToAbsoluteNote(pOsc3Note[Program])]); break;
	case pIdOsc1Detune: float2string((pOsc1Detune[Program]-.5f)*2.0f,text,5); break;
	case pIdOsc2Detune: float2string((pOsc2Detune[Program]-.5f)*2.0f,text,5); break;
	case pIdOsc3Detune: float2string((pOsc3Detune[Program]-.5f)*2.0f,text,5); break;
	case pIdOsc1Phase:  if(pOsc1Phase[Program]>0) float2string(pOsc1Phase[Program],text,5); else strcpy(text,"Free"); break;
	case pIdOsc2Phase:  if(pOsc2Phase[Program]>0) float2string(pOsc2Phase[Program],text,5); else strcpy(text,"Free"); break;
	case pIdOsc3Phase:  if(pOsc3Phase[Program]>0) float2string(pOsc3Phase[Program],text,5); else strcpy(text,"Free"); break;
	case pIdOsc1Output: strcpy(text,(pOsc1Output[Program]<.5f)?"Disabled":"Enabled"); break;
	case pIdOsc2Output: strcpy(text,(pOsc2Output[Program]<.5f)?"Disabled":"Enabled"); break;
	case pIdOsc3Output: strcpy(text,(pOsc3Output[Program]<.5f)?"Disabled":"Enabled"); break;
	case pIdOscMixMode:	strcpy(text,pOscMixModeNames[(int)(pOscMixMode[Program]*4.99f)]); break;
	case pIdPolyphony:	strcpy(text,(pPolyphony[Program]<.5f)?"Mono":"Poly"); break;
	case pIdPortaSpeed: float2string(pPortaSpeed[Program],text,5); break;
	case pIdNoteCut:    if(pNoteCut[Program]==0) strcpy(text,"Infinite"); else float2string(1000.0f*pNoteCut[Program],text,5); break;
	case pIdVelTarget:  strcpy(text,pVelTargetNames[(int)(pVelTarget[Program]*2.99f)]); break;
	case pIdOutputGain: dB2string(pOutputGain[Program],text,3); break;
	default:            strcpy(text,"");
	}
} 



void SerialSynth::getParameterLabel(VstInt32 index,char *label)
{
	switch(index)
	{ 
	case pIdNoteCut:    strcpy(label,(pNoteCut[Program]>0)?"ms":""); break;
	case pIdOutputGain: strcpy(label,"dB"); break;
	default:            strcpy(label,"");
	}
} 



VstInt32 SerialSynth::getProgram(void)
{
	return Program;
}



void SerialSynth::setProgram(VstInt32 program)
{
	Program=program;
}



void SerialSynth::getProgramName(char* name)
{
	strcpy(name,ProgramName[Program]); 
}



void SerialSynth::setProgramName(char* name)
{
	strncpy(ProgramName[Program],name,MAX_NAME_LEN);

	ProgramName[Program][MAX_NAME_LEN-1]='\0';
} 



VstInt32 SerialSynth::canDo(char* text)
{
	if(!strcmp(text,"receiveVstEvents"   )) return 1;
	if(!strcmp(text,"receiveVstMidiEvent")) return 1;

	return -1;
}



VstInt32 SerialSynth::getNumMidiInputChannels(void)
{
	return 1;//monophonic
}



VstInt32 SerialSynth::getNumMidiOutputChannels(void)
{
	return 0;//no MIDI output
}



VstInt32 SerialSynth::SaveStringChunk(char *str,float *dst)
{
	VstInt32 i;

	for(i=0;i<(VstInt32)strlen(str);++i) *dst++=str[i];

	*dst++=0;

	return (VstInt32)strlen(str)+1;
}



VstInt32 SerialSynth::LoadStringChunk(char *str,int max_length,float *src)
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



VstInt32 SerialSynth::SavePresetChunk(float *chunk)
{
	int ptr,pgm;

	ptr=0;

	chunk[ptr++]=DATA;

	for(pgm=0;pgm<NUM_PROGRAMS;++pgm)
	{
		chunk[ptr++]=PROG; chunk[ptr++]=(float)pgm;
		chunk[ptr++]=NAME; ptr+=SaveStringChunk(ProgramName[pgm],&chunk[ptr]);

		chunk[ptr++]=O1AC; chunk[ptr++]=pOsc1Active[pgm];
		chunk[ptr++]=O1MD; chunk[ptr++]=pOsc1Mode  [pgm];
		chunk[ptr++]=O1NT; chunk[ptr++]=pOsc1Note  [pgm];
		chunk[ptr++]=O1DE; chunk[ptr++]=pOsc1Detune[pgm];
		chunk[ptr++]=O1PH; chunk[ptr++]=pOsc1Phase [pgm];
		chunk[ptr++]=O1UT; chunk[ptr++]=pOsc1Output[pgm];

		chunk[ptr++]=O2AC; chunk[ptr++]=pOsc2Active[pgm];
		chunk[ptr++]=O2MD; chunk[ptr++]=pOsc2Mode  [pgm];
		chunk[ptr++]=O2NT; chunk[ptr++]=pOsc2Note  [pgm];
		chunk[ptr++]=O2DE; chunk[ptr++]=pOsc2Detune[pgm];
		chunk[ptr++]=O2PH; chunk[ptr++]=pOsc2Phase [pgm];
		chunk[ptr++]=O2UT; chunk[ptr++]=pOsc2Output[pgm];

		chunk[ptr++]=O3AC; chunk[ptr++]=pOsc3Active[pgm];
		chunk[ptr++]=O3MD; chunk[ptr++]=pOsc3Mode  [pgm];
		chunk[ptr++]=O3NT; chunk[ptr++]=pOsc3Note  [pgm];
		chunk[ptr++]=O3DE; chunk[ptr++]=pOsc3Detune[pgm];
		chunk[ptr++]=O3PH; chunk[ptr++]=pOsc3Phase [pgm];
		chunk[ptr++]=O3UT; chunk[ptr++]=pOsc3Output[pgm];

		chunk[ptr++]=OMIX; chunk[ptr++]=pOscMixMode[pgm];
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

		fprintf(file,"O1AC,%1.3ff,",pOsc1Active[pgm]);
		fprintf(file,"O1MD,%1.3ff,",pOsc1Mode  [pgm]);
		fprintf(file,"O1NT,%1.3ff,",pOsc1Note  [pgm]);
		fprintf(file,"O1DE,%1.3ff,",pOsc1Detune[pgm]);
		fprintf(file,"O1PH,%1.3ff,",pOsc1Phase [pgm]);
		fprintf(file,"O1UT,%1.3ff,",pOsc1Output[pgm]);
		fprintf(file,"\n");
		fprintf(file,"O2AC,%1.3ff,",pOsc2Active[pgm]);
		fprintf(file,"O2MD,%1.3ff,",pOsc2Mode  [pgm]);
		fprintf(file,"O2NT,%1.3ff,",pOsc2Note  [pgm]);
		fprintf(file,"O2DE,%1.3ff,",pOsc2Detune[pgm]);
		fprintf(file,"O2PH,%1.3ff,",pOsc2Phase [pgm]);
		fprintf(file,"O2UT,%1.3ff,",pOsc2Output[pgm]);
		fprintf(file,"\n");
		fprintf(file,"O3AC,%1.3ff,",pOsc3Active[pgm]);
		fprintf(file,"O3MD,%1.3ff,",pOsc3Mode  [pgm]);
		fprintf(file,"O3NT,%1.3ff,",pOsc3Note  [pgm]);
		fprintf(file,"O3DE,%1.3ff,",pOsc3Detune[pgm]);
		fprintf(file,"O3PH,%1.3ff,",pOsc3Phase [pgm]);
		fprintf(file,"O3UT,%1.3ff,",pOsc3Output[pgm]);
		fprintf(file,"\n");
		fprintf(file,"OMIX,%1.3ff,",pOscMixMode[pgm]);
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



void SerialSynth::LoadPresetChunk(float *chunk)
{
	int pgm;
	float tag;

	while(1)
	{
		tag=*chunk++;

		if(tag==DONE) break;

		if(tag==PROG) pgm =(int)*chunk++;

		if(tag==NAME) chunk+=LoadStringChunk(ProgramName[pgm],MAX_NAME_LEN,chunk);

		if(tag==O1AC) pOsc1Active[pgm]=*chunk++;
		if(tag==O1MD) pOsc1Mode  [pgm]=*chunk++;
		if(tag==O1NT) pOsc1Note  [pgm]=*chunk++;
		if(tag==O1DE) pOsc1Detune[pgm]=*chunk++;
		if(tag==O1PH) pOsc1Phase [pgm]=*chunk++;
		if(tag==O1UT) pOsc1Output[pgm]=*chunk++;

		if(tag==O2AC) pOsc2Active[pgm]=*chunk++;
		if(tag==O2MD) pOsc2Mode  [pgm]=*chunk++;
		if(tag==O2NT) pOsc2Note  [pgm]=*chunk++;
		if(tag==O2DE) pOsc2Detune[pgm]=*chunk++;
		if(tag==O2PH) pOsc2Phase [pgm]=*chunk++;
		if(tag==O2UT) pOsc2Output[pgm]=*chunk++;

		if(tag==O3AC) pOsc3Active[pgm]=*chunk++;
		if(tag==O3MD) pOsc3Mode  [pgm]=*chunk++;
		if(tag==O3NT) pOsc3Note  [pgm]=*chunk++;
		if(tag==O3DE) pOsc3Detune[pgm]=*chunk++;
		if(tag==O3PH) pOsc3Phase [pgm]=*chunk++;
		if(tag==O3UT) pOsc3Output[pgm]=*chunk++;

		if(tag==OMIX) pOscMixMode[pgm]=*chunk++;
		if(tag==POLY) pPolyphony [pgm]=*chunk++;
		if(tag==POSP) pPortaSpeed[pgm]=*chunk++;
		if(tag==NCUT) pNoteCut   [pgm]=*chunk++;
		if(tag==VTGT) pVelTarget [pgm]=*chunk++;
		if(tag==GAIN) pOutputGain[pgm]=*chunk++;
	}
}



VstInt32 SerialSynth::getChunk(void** data,bool isPreset)
{
	int size;

	size=SavePresetChunk(Chunk);

	*data=Chunk;

	return size;
}



VstInt32 SerialSynth::setChunk(void* data,VstInt32 byteSize,bool isPreset)
{
	if(byteSize>sizeof(Chunk)) return 0;

	memcpy(Chunk,data,byteSize);

	LoadPresetChunk(Chunk);

	return 0;
}



void SerialSynth::MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity)
{
	MidiQueueStruct entry;

	entry.type    =mEventTypeNote;
	entry.delta   =delta;
	entry.note    =note;
	entry.velocity=velocity;//0 for key off

	MidiQueue.push_back(entry);
}



void SerialSynth::MidiAddProgramChange(VstInt32 delta,VstInt32 program)
{
	MidiQueueStruct entry;

	entry.type   =mEventTypeProgram;
	entry.delta  =delta;
	entry.program=program;

	MidiQueue.push_back(entry);
}



void SerialSynth::MidiAddPitchBend(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypePitchBend;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



void SerialSynth::MidiAddModulation(VstInt32 delta,float depth)
{
	MidiQueueStruct entry;

	entry.type =mEventTypeModulation;
	entry.delta=delta;
	entry.depth=depth;

	MidiQueue.push_back(entry);
}



bool SerialSynth::MidiIsAnyKeyDown(void)
{
	int i;

	for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

	return false;
}



VstInt32 SerialSynth::processEvents(VstEvents* ev) 
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



VstInt32 SerialSynth::SynthAllocateVoice(VstInt32 note)
{
	VstInt32 chn;

	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note==note) return chn;
	for(chn=0;chn<SYNTH_CHANNELS;++chn) if(SynthChannel[chn].note<0) return chn;

	return -1;
}



void SerialSynth::SynthChannelChangeNote(VstInt32 chn,VstInt32 note)
{
	SynthChannel[chn].note=note;

	if(note>=0)
	{
		SynthChannel[chn].freq_new=(float)SynthChannel[chn].note-69;

		sSlideStep=(SynthChannel[chn].freq-SynthChannel[chn].freq_new)*20.0f*log(1.0f-pPortaSpeed[Program]);
	}
}



void SerialSynth::processReplacing(float **inputs,float **outputs,VstInt32 sampleFrames)
{
	float *outL=outputs[0];
	float *outR=outputs[1];
	float outf,base,note1,note2,note3,level,velocity,modulation,mod_step,sampleRate;
	VstInt32 i,s,vtgt,poly_mode,chn,note,prev_note;
	VstInt32 osc_mix,osc1_src,osc2_src,osc3_src;
	VstInt32 mode1,mode2,mode3,gate1,gate2,gate3,out1,out2,out3;

	sampleRate=(float)updateSampleRate();

	mod_step=12.0f/sampleRate*(float)M_PI;

	poly_mode=pPolyphony[Program]<.5f?0:1;

	osc_mix =(int)(pOscMixMode[Program]*4.99f);
	osc1_src=(int)(pOsc1Active[Program]*2.99f);
	osc2_src=(int)(pOsc2Active[Program]*2.99f);
	osc3_src=(int)(pOsc3Active[Program]*2.99f);

	mode1=pOsc1Mode[Program]<.5f?0:1;
	mode2=pOsc2Mode[Program]<.5f?0:1;
	mode3=pOsc3Mode[Program]<.5f?0:1;

	vtgt=pVelTarget[Program]<.5f?0:1;

	while(--sampleFrames>=0)
	{
		if(MidiModulationDepth>=.01f) modulation=sinf(MidiModulationCount)*MidiModulationDepth; else modulation=0;

		MidiModulationCount+=mod_step;

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
								MidiKeyState[MidiQueue[i].note]=1;

								if(!poly_mode) chn=0; else chn=SynthAllocateVoice(MidiQueue[i].note);

								if(chn>=0)
								{
									prev_note=SynthChannel[chn].note;

									SynthChannelChangeNote(chn,MidiQueue[i].note);

									velocity=(float)MidiQueue[i].velocity/100.0f;

									if(prev_note<0||poly_mode)
									{
										SynthChannel[chn].freq=SynthChannel[chn].freq_new;
										SynthChannel[chn].volume=(vtgt==0?velocity:1.0f)/SYNTH_CHANNELS/OVERSAMPLING;
										SynthChannel[chn].duration=pNoteCut[Program]*(vtgt==1?velocity:1.0f)*sampleRate*OVERSAMPLING;

										if(pOsc1Phase[Program]>0) SynthChannel[chn].acc1=pOsc1Phase[Program];
										if(pOsc2Phase[Program]>0) SynthChannel[chn].acc2=pOsc2Phase[Program];
										if(pOsc3Phase[Program]>0) SynthChannel[chn].acc3=pOsc3Phase[Program];
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

			base=SynthChannel[chn].freq+modulation+MidiPitchBend*12.0f;

			if(mode1) note1=base+FloatToRelativeNote(pOsc1Note[Program]); else note1=FloatToAbsoluteNote(pOsc1Note[Program]);
			if(mode2) note2=base+FloatToRelativeNote(pOsc2Note[Program]); else note2=FloatToAbsoluteNote(pOsc2Note[Program]);
			if(mode3) note3=base+FloatToRelativeNote(pOsc3Note[Program]); else note3=FloatToAbsoluteNote(pOsc3Note[Program]);

			SynthChannel[chn].add1=440.0f*pow(2.0f,(note1+pOsc1Detune[Program])/12.0f)/sampleRate/OVERSAMPLING;
			SynthChannel[chn].add2=440.0f*pow(2.0f,(note2+pOsc2Detune[Program])/12.0f)/sampleRate/OVERSAMPLING;
			SynthChannel[chn].add3=440.0f*pow(2.0f,(note3+pOsc3Detune[Program])/12.0f)/sampleRate/OVERSAMPLING;
		}

		level=0;

		for(s=0;s<OVERSAMPLING;++s)
		{
			outf=0;

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

				switch(osc1_src)
				{
				case 1:  gate1=SynthChannel[chn].out2; break;
				case 2:  gate1=SynthChannel[chn].out3; break;
				default: gate1=1;
				}
	
				switch(osc2_src)
				{
				case 1:  gate2=SynthChannel[chn].out1; break;
				case 2:  gate2=SynthChannel[chn].out3; break;
				default: gate2=1;
				}

				switch(osc3_src)
				{
				case 1:  gate3=SynthChannel[chn].out1; break;
				case 2:  gate3=SynthChannel[chn].out2; break;
				default: gate3=1;
				}

				if(gate1)
				{
					SynthChannel[chn].acc1+=SynthChannel[chn].add1;

					while(SynthChannel[chn].acc1>=1.0f) SynthChannel[chn].acc1-=1.0f;

					if(SynthChannel[chn].acc1>=.5f) SynthChannel[chn].out1=1; else SynthChannel[chn].out1=0;
				}

				if(gate2)
				{
					SynthChannel[chn].acc2+=SynthChannel[chn].add2;

					while(SynthChannel[chn].acc2>=1.0f) SynthChannel[chn].acc2-=1.0f;

					if(SynthChannel[chn].acc2>=.5f) SynthChannel[chn].out2=1; else SynthChannel[chn].out2=0;
				}

				if(gate3)
				{
					SynthChannel[chn].acc3+=SynthChannel[chn].add3;

					while(SynthChannel[chn].acc3>=1.0f) SynthChannel[chn].acc3-=1.0f;

					if(SynthChannel[chn].acc3>=.5f) SynthChannel[chn].out3=1; else SynthChannel[chn].out3=0;
				}
			
				gate1=(pOsc1Output[Program]>=.5f)?1:0;
				gate2=(pOsc2Output[Program]>=.5f)?1:0;
				gate3=(pOsc3Output[Program]>=.5f)?1:0;

				switch(osc_mix)
				{
				case 0:	//ADD 1:1:1
					{
						if(gate1&&SynthChannel[chn].out1) outf+=SynthChannel[chn].volume;
						if(gate2&&SynthChannel[chn].out2) outf+=SynthChannel[chn].volume;
						if(gate3&&SynthChannel[chn].out3) outf+=SynthChannel[chn].volume;
					}
					break;

				case 1:	//ADD 1:.5:.25
					{
						if(gate1&&SynthChannel[chn].out1) outf+=SynthChannel[chn].volume*0.25f;
						if(gate2&&SynthChannel[chn].out2) outf+=SynthChannel[chn].volume*0.50f;
						if(gate3&&SynthChannel[chn].out3) outf+=SynthChannel[chn].volume*1.00f;
					}
					break;

				case 2:	//AND
					{
						out1=gate1?SynthChannel[chn].out1:1;
						out2=gate2?SynthChannel[chn].out2:1;
						out3=gate3?SynthChannel[chn].out3:1;

						if(out1&out2&out3) outf+=SynthChannel[chn].volume;
					}
					break;

				case 3:	//OR
					{
						out1=gate1?SynthChannel[chn].out1:0;
						out2=gate2?SynthChannel[chn].out2:0;
						out3=gate3?SynthChannel[chn].out3:0;

						if(out1|out2|out3) outf+=SynthChannel[chn].volume;
					}
					break;

				case 4:	//XOR
					{
						out1=gate1?SynthChannel[chn].out1:0;
						out2=gate2?SynthChannel[chn].out2:0;
						out3=gate3?SynthChannel[chn].out3:0;

						if(out1^out2^out3) outf+=SynthChannel[chn].volume;
					}
					break;
				}
			}

			level+=outf;
		}

		level=level*pOutputGain[Program];

		(*outL++)=level;
		(*outR++)=level;
	}

	MidiQueue.clear();
}
