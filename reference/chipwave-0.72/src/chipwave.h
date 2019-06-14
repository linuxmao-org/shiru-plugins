#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"

#define PLUGIN_NAME					"ChipWave"
#define PLUGIN_VENDOR				"Shiru"
#define PLUGIN_PRODUCT				"v0.72 03.06.19"
#define PLUGIN_UID					'chwa'

#define NUM_INPUTS					0
#define NUM_OUTPUTS					2
#define NUM_PROGRAMS				128

#define SYNTH_CHANNELS				8

#define OSC_CUT_MAX_MS				1000

#define ENVELOPE_UPDATE_RATE_HZ		500

#define ENVELOPE_ATTACK_MAX_MS		10000
#define ENVELOPE_DECAY_MAX_MS		10000
#define ENVELOPE_RELEASE_MAX_MS		1000

#define MOD_DELAY_MAX_MS			10000
#define LFO_MAX_HZ					50

#define DETUNE_SEMITONES			7.0f

#define OVERSAMPLING				8

#define CHUNK_SIZE					65536

#define MAX_NAME_LEN				32

#define F_PI						((float)M_PI)

#define FILTER_CUTOFF_MIN_HZ		10
#define FILTER_CUTOFF_MAX_HZ		22050
#define FILTER_MIN_RESONANCE		1.0f
#define FILTER_MAX_RESONANCE		10.0f

#define OVERDRIVE_MAX				10.0f



enum {

	pIdOscAWave=0,
	pIdOscADuty,
	pIdOscAOver,
	pIdOscACut,
	pIdOscAMultiple,
	pIdOscASeed,

	pIdOscBWave,
	pIdOscBDuty,
	pIdOscBOver,
	pIdOscBCut,
	pIdOscBDetune,
	pIdOscBMultiple,
	pIdOscBSeed,

	pIdOscBalance,
	pIdOscMixMode,

	pIdFltCutoff,
	pIdFltReso,

	pIdSlideDelay,
	pIdSlideSpeed,
	pIdSlideRoute,

	pIdEnvAttack,
	pIdEnvDecay,
	pIdEnvSustain,
	pIdEnvRelease,
	pIdEnvOscADepth,
	pIdEnvOscBDepth,
	pIdEnvOscBDetuneDepth,
	pIdEnvOscMixDepth,
	pIdEnvFltDepth,
	pIdEnvLfoDepth,

	pIdLfoSpeed,
	pIdLfoPitchDepth,
	pIdLfoOscADepth,
	pIdLfoOscBDepth,
	pIdLfoOscMixDepth,
	pIdLfoFltDepth,

	pIdAmpAttack,
	pIdAmpDecay,
	pIdAmpSustain,
	pIdAmpRelease,

	pIdVelAmp,
	pIdVelOscADepth,
	pIdVelOscBDepth,
	pIdVelOscMixDepth,
	pIdVelFltCutoff,
	pIdVelFltReso,

	pIdPolyphony,
	pIdPortaSpeed,
	pIdOutputGain,

	NUM_PARAMS

};



const char* const oscWaveformNames[]={
	"Square/Pulse",
	"Tri/Saw",
	"Sine",
	"Digi Noise"
};



const char* const slideRouteNames[3]={
	"Both",
	"OscA only",
	"OscB only"
};



const char* const programDefaultNames[]={
	""
};



enum {
	mEventTypeNote=0,
	mEventTypeProgram,
	mEventTypePitchBend,
	mEventTypeModulation
};



struct MidiQueueStruct
{
	VstInt32 type;
	VstInt32 delta;
	VstInt32 note;
	VstInt32 velocity;
	VstInt32 program;

	float depth;
};



enum {
	eStageReset=0,
	eStageAttack,
	eStageDecay,
	eStageSustain,
	eStageRelease
};



struct SynthOscStruct {

	float acc;
	float add;
	float cut;

	VstInt32 noise;
	VstInt32 noise_seed;

};



struct SynthChannelStruct
{
	VstInt32 note;

	float velocity;

	float freq;
	float freq_new;

	SynthOscStruct osca;
	SynthOscStruct oscb;

	VstInt32 ev_stage;
	float ev_level;
	float ev_delta;

	VstInt32 ef_stage;
	float ef_level;
	float ef_delta;

	float volume;

	float slide_delay;
	float slide_osca;
	float slide_oscb;

	float lfo_count;
	float lfo_out;

	double filter_resofreq;
	double filter_amp;
	double filter_r;
	double filter_c;
	double filter_vibrapos;
	double filter_vibraspeed;
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")
#define PROG		TAG("PROG")
#define NAME		TAG("NAME")

#define OAWF		TAG("OAWF")
#define OADU		TAG("OADU")
#define OAOV		TAG("OAOV")
#define OACT		TAG("OACT")
#define OAMU		TAG("OAMU")
#define OASD		TAG("OASD")

#define OBWF		TAG("OBWF")
#define OBDU		TAG("OBDU")
#define OBOV		TAG("OBOV")
#define OBCT		TAG("OBCT")
#define OBDE		TAG("OBDE")
#define OBMU		TAG("OBMU")
#define OBSD		TAG("OBSD")

#define OBAL		TAG("OBAL")
#define OMIX		TAG("OMIX")

#define FLCU		TAG("FLCU")
#define FLRE		TAG("FLRE")

#define SLDE		TAG("SLDE")
#define SLSP		TAG("SLSP")
#define SLRO		TAG("SLRO")

#define ENAT		TAG("ENAT")
#define ENDC		TAG("ENDC")
#define ENSU		TAG("ENSU")
#define ENRE		TAG("ENRE")
#define ENFL		TAG("ENFL")
#define ENLF		TAG("ENLF")
#define ENOA		TAG("ENOA")
#define ENOB		TAG("ENOB")
#define ENOD		TAG("ENOD")
#define ENMX		TAG("ENMX")

#define LFSP		TAG("LFSP")
#define LFPI		TAG("LFPI")
#define LFOA		TAG("LFOA")
#define LFOB		TAG("LFOB")
#define LFMX		TAG("LFMX")
#define LFFL		TAG("LFFL")

#define VOAT		TAG("VOAT")
#define VODE		TAG("VODE")
#define VOSU		TAG("VOSU")
#define VORE		TAG("VORE")

#define VLAM		TAG("VLAM")
#define VLOA		TAG("VLOA")
#define VLOB		TAG("VLOB")
#define VLMX		TAG("VLMX")
#define VLFC		TAG("VLFC")
#define VLFR		TAG("VLFR")

#define POLY		TAG("POLY")
#define POSP		TAG("POSP")
#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const float ChunkPresetData[]=
{

	DATA,

	DONE

};



class ChipWave:public AudioEffectX
{
public:

	ChipWave(audioMasterCallback audioMaster);

	~ChipWave();

	virtual void setParameter(VstInt32 index,float value);
	virtual float getParameter(VstInt32 index);
	virtual void getParameterLabel(VstInt32 index,char *label);
	virtual void getParameterDisplay(VstInt32 index,char *text);
	virtual void getParameterName(VstInt32 index,char *text); 

	virtual VstInt32 getProgram();
	virtual void setProgram(VstInt32 program);
	virtual void getProgramName(char* name);
	virtual void setProgramName(char* name);

	virtual bool getEffectName(char *name) { strcpy(name,PLUGIN_NAME); return true; }
	virtual bool getVendorString(char *text) { strcpy(text,PLUGIN_VENDOR); return true; }
	virtual bool getProductString(char *text) { strcpy(text, PLUGIN_PRODUCT); return true; }
	virtual VstInt32 getVendorVersion() { return 1000; } 

	VstInt32 canDo(char* text);
	VstInt32 getNumMidiInputChannels(void);
	VstInt32 getNumMidiOutputChannels(void);

	virtual VstInt32 getChunk(void** data,bool isPreset=false);
	virtual VstInt32 setChunk(void* data,VstInt32 byteSize,bool isPreset=false);

	VstInt32 processEvents(VstEvents* ev);
	void processReplacing(float**inputs,float **outputs,VstInt32 sampleFrames);

	void UpdateGUI(bool display);

	VstInt32 Program;

	float pOscAWave		 [NUM_PROGRAMS];
	float pOscADuty		 [NUM_PROGRAMS];
	float pOscAOver		 [NUM_PROGRAMS];
	float pOscACut		 [NUM_PROGRAMS];
	float pOscAMultiple	 [NUM_PROGRAMS];
	float pOscASeed 	 [NUM_PROGRAMS];

	float pOscBWave		 [NUM_PROGRAMS];
	float pOscBDuty		 [NUM_PROGRAMS];
	float pOscBOver		 [NUM_PROGRAMS];
	float pOscBCut		 [NUM_PROGRAMS];
	float pOscBDetune	 [NUM_PROGRAMS];
	float pOscBMultiple	 [NUM_PROGRAMS];
	float pOscBSeed		 [NUM_PROGRAMS];

	float pOscBalance	 [NUM_PROGRAMS];
	float pOscMixMode	 [NUM_PROGRAMS];

	float pFltCutoff	 [NUM_PROGRAMS];
	float pFltReso		 [NUM_PROGRAMS];

	float pSlideDelay	 [NUM_PROGRAMS];
	float pSlideSpeed	 [NUM_PROGRAMS];
	float pSlideRoute	 [NUM_PROGRAMS];

	float pEnvAttack	 [NUM_PROGRAMS];
	float pEnvDecay		 [NUM_PROGRAMS];
	float pEnvSustain	 [NUM_PROGRAMS];
	float pEnvRelease	 [NUM_PROGRAMS];
	float pEnvOscADepth	 [NUM_PROGRAMS];
	float pEnvOscBDepth	 [NUM_PROGRAMS];
	float pEnvOscBDetuneDepth[NUM_PROGRAMS];
	float pEnvOscMixDepth[NUM_PROGRAMS];
	float pEnvFltDepth	 [NUM_PROGRAMS];
	float pEnvLfoDepth	 [NUM_PROGRAMS];

	float pLfoSpeed		 [NUM_PROGRAMS];
	float pLfoPitchDepth [NUM_PROGRAMS];
	float pLfoOscADepth	 [NUM_PROGRAMS];
	float pLfoOscBDepth	 [NUM_PROGRAMS];
	float pLfoOscMixDepth[NUM_PROGRAMS];
	float pLfoFltDepth	 [NUM_PROGRAMS];

	float pAmpAttack	 [NUM_PROGRAMS];
	float pAmpDecay		 [NUM_PROGRAMS];
	float pAmpSustain 	 [NUM_PROGRAMS];
	float pAmpRelease	 [NUM_PROGRAMS];

	float pVelAmp	     [NUM_PROGRAMS];
	float pVelOscADepth	 [NUM_PROGRAMS];
	float pVelOscBDepth	 [NUM_PROGRAMS];
	float pVelOscMixDepth[NUM_PROGRAMS];
	float pVelFltCutoff	 [NUM_PROGRAMS];
	float pVelFltReso	 [NUM_PROGRAMS];

	float pPolyphony	 [NUM_PROGRAMS];
	float pPortaSpeed	 [NUM_PROGRAMS];
	float pOutputGain	 [NUM_PROGRAMS];

	float SynthGetSample(SynthOscStruct *osc,float over,float duty,int wave);
	float SynthEnvelopeTimeToDelta(float value,float max_ms);
	float OverdriveValue(float value);

protected:

	char ProgramName[NUM_PROGRAMS][MAX_NAME_LEN];

	float FloatToMultiple(float value);

	float* getVarPtr(VstInt32 index);

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	VstInt32 SaveStringChunk(char *str,float *dst);
	VstInt32 LoadStringChunk(char *str,int max_length,float *src);

	VstInt32 SynthAllocateVoice(VstInt32 note);
	void SynthChannelChangeNote(VstInt32 chn,VstInt32 note);
	void SynthRestartEnvelope(VstInt32 chn);
	void SynthStopEnvelope(VstInt32 chn);
	void SynthAdvanceEnvelopes(void);

	void MidiAddNote(VstInt32 delta,VstInt32 note,VstInt32 velocity);
	void MidiAddProgramChange(VstInt32 delta,VstInt32 program);
	void MidiAddPitchBend(VstInt32 delta,float depth);
	void MidiAddModulation(VstInt32 delta,float depth);
	bool MidiIsAnyKeyDown(void);

	vector<MidiQueueStruct> MidiQueue;

	unsigned char MidiKeyState[128];

	float MidiPitchBend;
	float MidiPitchBendRange;
	float MidiModulationDepth;
	float MidiModulationCount;

	VstInt32 MidiRPNLSB;
	VstInt32 MidiRPNMSB;
	VstInt32 MidiDataLSB;
	VstInt32 MidiDataMSB;

	VstInt32 MidiModulationMSB;

	SynthChannelStruct SynthChannel[SYNTH_CHANNELS];

	float sEnvelopeDiv;
	float sSlideStep;

	float Chunk[CHUNK_SIZE];

	VstInt32 Noise[65536];

	bool UpdateGuiFlag;
};
