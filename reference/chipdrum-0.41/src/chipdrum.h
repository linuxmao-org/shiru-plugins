#include <math.h>
#include <vector>

using namespace std;

#include "audioeffectx.h"



#define PLUGIN_NAME				"ChipDrum"
#define PLUGIN_VENDOR			"Shiru"
#define PLUGIN_PRODUCT			"v0.41 03.06.19"
#define PLUGIN_UID				'cdru'

#define NUM_INPUTS				0
#define NUM_OUTPUTS				8
#define NUM_PROGRAMS			16

#define TONE_PITCH_MAX_HZ		16000.0f
#define NOISE_PITCH_MAX_HZ		64000.0f

#define DECAY_TIME_MAX_MS		500
#define RELEASE_TIME_MAX_MS		1000
#define NOISE_BURST_MAX_MS		250

#define SYNTH_NOTES				8
#define SYNTH_CHANNELS			9

#define OVERSAMPLING			8

#define CHUNK_SIZE				65536*2

#define MAX_NAME_LEN			32

#define PN_PTR(pgm,note)		((pgm)*SYNTH_NOTES+(note))

#define F_PI					((float)M_PI)

#define OVERDRIVE_MAX			10.0f

#define RETRIGGER_MAX_MS		50
#define RETRIGGER_MAX_COUNT		4

#define FILTER_CUTOFF_MIN_HZ	10
#define FILTER_CUTOFF_MAX_HZ	8000

#define MIN_UPDATE_RATE			10.0f
#define MAX_UPDATE_RATE			(250.0f-MIN_UPDATE_RATE)



enum {
	pIdNoteMapping=0,

	pIdToneLevel,
	pIdToneDecay,
	pIdToneSustain,
	pIdToneRelease,
	pIdTonePitch,
	pIdToneSlide,
	pIdToneWave,
	pIdToneOver,

	pIdNoiseLevel,
	pIdNoiseDecay,
	pIdNoiseSustain,
	pIdNoiseRelease,
	pIdNoisePitch1,
	pIdNoisePitch2,
	pIdNoisePitch2Off,
	pIdNoisePitch2Len,
	pIdNoisePeriod,
	pIdNoiseSeed,
	pIdNoiseType,

	pIdRetrigTime,
	pIdRetrigCount,
	pIdRetrigRoute,

	pIdFilterLP,
	pIdFilterHP,
	pIdFilterRoute,

	pIdDrumGroup,
	pIdDrumBitDepth,
	pIdDrumUpdateRate,
	pIdDrumVolume,
	pIdDrumPan,

	pIdVelDrumVolume,
	pIdVelTonePitch,
	pIdVelNoisePitch,
	pIdVelToneOver,

	pIdHat1Length,
	pIdHat2Length,
	pIdHat3Length,
	pIdHatPanWidth,

	pIdTom1Pitch,
	pIdTom2Pitch,
	pIdTom3Pitch,
	pIdTomPanWidth,

	pIdOutputGain,

	NUM_PARAMS
};



const char* const pNoteNames[SYNTH_NOTES]={
	"C (BD - Bass Drum)",
	"C# (CC - Crash Cymbal)",
	"D (SD - Snare Drum)",
	"D# (RD - Ride)",
	"E (CP - Clap)",
	"F (CW - Cowbell)",
	"F#,G#,A# (HH - Hi-Hat)",
	"G,A,B (TM - Tom)"
};



const char* const pWaveformNames[4]={
	"Square",
	"Saw",
	"Triangle",
	"Sine"
};



const char* const pRetrigRouteNames[3]={
	"Both",
	"Tone only",
	"Noise only"
};



const char* const pFilterRouteNames[4]={
	"Both",
	"Tone only",
	"Noise only",
	"None"
};



const float pBitDepth[]={2.0f,4.0f,8.0f,16.0f,32.0f,64.0f,256.0f,0};

const char* const pBitDepthNames[]={"1 bit","2 bit","3 bit","4 bit","5 bit","6 bit","8 bit","Analog"};



enum {
	mEventTypeNote=0,
	mEventTypeProgram
};



struct MidiQueueStruct
{
	VstInt32 type;
	VstInt32 delta;
	VstInt32 note;
	VstInt32 velocity;
	VstInt32 program;
};




struct SynthChannelStruct
{
	VstInt32 note;
	VstInt32 notem;

	float velocity;

	float tone_sample;

	float tone_level;
	float tone_decay;
	float tone_sustain;
	float tone_release;

	float tone_acc;
	float tone_add;
	float tone_adda;
	float tone_delta;
	float tone_over;

	float tone_env_acc;
	float tone_env_add1;
	float tone_env_add2;

	VstInt32 tone_wave;

	float noise_sample;

	float noise_level;
	float noise_decay;
	float noise_sustain;
	float noise_release;

	float noise_env_acc;
	float noise_env_add1;
	float noise_env_add2;

	float noise_acc;
	float noise_add1;
	float noise_add2;
	float noise_add1a;
	float noise_add2a;

	float noise_frame_acc;
	float noise_frame_add1;
	float noise_frame_add2;

	VstInt32 noise_mask;
	VstInt32 noise_ptr;
	VstInt32 noise_seed;
	VstInt32 noise_type;

	float retrigger_acc;
	float retrigger_add;
	VstInt32 retrigger_count;
	VstInt32 retrigger_route;

	VstInt32 group;

	float bit_depth;

	VstInt32 filter_route;

	double lpf_resofreq;
	double lpf_r;
	double lpf_c;
	double lpf_vibrapos;
	double lpf_vibraspeed;

	double hpf_resofreq;
	double hpf_r;
	double hpf_c;
	double hpf_vibrapos;
	double hpf_vibraspeed;

	float frame_acc;
	float frame_add;

	float volume_l;
	float volume_r;
};



#define TAG(s)		(float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)))

#define DATA		TAG("DATA")

#define PROG		TAG("PROG")
#define NAME		TAG("NAME")
#define NOTE		TAG("NOTE")

#define TLVL		TAG("TLVL")
#define TDC1		TAG("TDC1")
#define TDC2		TAG("TDC2")
#define TDCL		TAG("TDCL")
#define TPIT		TAG("TNPI")
#define TSLD		TAG("TNSL")
#define TWAV		TAG("TWAV")
#define TOVR		TAG("TOVR")

#define NLVL		TAG("LVLN")
#define NDC1		TAG("DC1N")
#define NDC2		TAG("DC2N")
#define NDCL		TAG("DCLN")
#define NPT1		TAG("PT1N")
#define NPT2		TAG("PT2N")
#define N2OF		TAG("2OFN")
#define N2LN		TAG("2LNN")
#define NSCA		TAG("SCAN")
#define NPRD		TAG("PRDN")
#define NTYP		TAG("TYPN")
#define NPSE		TAG("PSEN")

#define RTME		TAG("RTME")
#define RCNT		TAG("RCNT")
#define RRTE		TAG("RRTE")

#define LLPF		TAG("LLPF")
#define HHPF		TAG("HHPF")
#define FLTR		TAG("FLTR")

#define GRPO		TAG("GRPO")
#define BDPT		TAG("BDPT")
#define UPDR		TAG("UPDR")
#define VOLU		TAG("VOLU")
#define PANO		TAG("PANO")

#define VLDV		TAG("VDVL")
#define VLTP		TAG("VTPL")
#define VLNP		TAG("VNPL")
#define VLOD		TAG("VODL")

#define H1LN		TAG("1LNH")
#define H2LN		TAG("L2NH")
#define H3LN		TAG("LN3H")
#define HPAN		TAG("HPAN")

#define T1PT		TAG("1PTT")
#define T2PT		TAG("P2TT")
#define T3PT		TAG("PT3T")
#define TPAN		TAG("TPAN")

#define GAIN		TAG("GAIN")
#define DONE		TAG("DONE")



const float ChunkPresetData[]={

	DATA,

	DONE

};



class ChipDrum:public AudioEffectX
{
public:

	ChipDrum(audioMasterCallback audioMaster);

	~ChipDrum();

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
	bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
	VstInt32 getNumMidiInputChannels(void);
	VstInt32 getNumMidiOutputChannels(void);

	virtual VstInt32 getChunk(void** data,bool isPreset=false);
	virtual VstInt32 setChunk(void* data,VstInt32 byteSize,bool isPreset=false);

	VstInt32 processEvents(VstEvents* ev);
	void processReplacing(float**inputs,float **outputs,VstInt32 sampleFrames);

	void UpdateGUI(bool display);

	float OverdriveValue(float value);
	inline float SynthGetSample(VstInt32 wave,float acc,float over);

	void CopyDrum(void);
	void PasteDrum(void);

	VstInt32 Program;

	float pNoteMapping[NUM_PROGRAMS];
	int   pNoteMappingInt;

	float pToneLevel  [NUM_PROGRAMS*SYNTH_NOTES];
	float pToneDecay  [NUM_PROGRAMS*SYNTH_NOTES];
	float pToneSustain[NUM_PROGRAMS*SYNTH_NOTES];
	float pToneRelease[NUM_PROGRAMS*SYNTH_NOTES];
	float pTonePitch  [NUM_PROGRAMS*SYNTH_NOTES];
	float pToneSlide  [NUM_PROGRAMS*SYNTH_NOTES];
	float pToneWave   [NUM_PROGRAMS*SYNTH_NOTES];
	float pToneOver   [NUM_PROGRAMS*SYNTH_NOTES];

	float pNoiseLevel  [NUM_PROGRAMS*SYNTH_NOTES];
	float pNoiseDecay  [NUM_PROGRAMS*SYNTH_NOTES];
	float pNoiseSustain[NUM_PROGRAMS*SYNTH_NOTES];
	float pNoiseRelease[NUM_PROGRAMS*SYNTH_NOTES];
	float pNoisePitch1 [NUM_PROGRAMS*SYNTH_NOTES];
	float pNoisePitch2 [NUM_PROGRAMS*SYNTH_NOTES];
	float pNoisePitch2Off[NUM_PROGRAMS*SYNTH_NOTES];
	float pNoisePitch2Len[NUM_PROGRAMS*SYNTH_NOTES];
	float pNoisePeriod [NUM_PROGRAMS*SYNTH_NOTES];
	float pNoiseSeed   [NUM_PROGRAMS*SYNTH_NOTES];
	float pNoiseType   [NUM_PROGRAMS*SYNTH_NOTES];

	float pRetrigTime  [NUM_PROGRAMS*SYNTH_NOTES];
	float pRetrigCount [NUM_PROGRAMS*SYNTH_NOTES];
	float pRetrigRoute [NUM_PROGRAMS*SYNTH_NOTES];

	float pFilterLP    [NUM_PROGRAMS*SYNTH_NOTES];
	float pFilterHP    [NUM_PROGRAMS*SYNTH_NOTES];
	float pFilterRoute [NUM_PROGRAMS*SYNTH_NOTES];

	float pDrumGroup   [NUM_PROGRAMS*SYNTH_NOTES];
	float pDrumBitDepth[NUM_PROGRAMS*SYNTH_NOTES];
	float pDrumUpdateRate[NUM_PROGRAMS*SYNTH_NOTES];
	float pDrumVolume  [NUM_PROGRAMS*SYNTH_NOTES];
	float pDrumPan     [NUM_PROGRAMS*SYNTH_NOTES];

	float pVelDrumVolume[NUM_PROGRAMS*SYNTH_NOTES];
	float pVelTonePitch [NUM_PROGRAMS*SYNTH_NOTES];
	float pVelNoisePitch[NUM_PROGRAMS*SYNTH_NOTES];
	float pVelToneOver  [NUM_PROGRAMS*SYNTH_NOTES];

	float pHat1Length  [NUM_PROGRAMS];
	float pHat2Length  [NUM_PROGRAMS];
	float pHat3Length  [NUM_PROGRAMS];
	float pHatPanWidth [NUM_PROGRAMS];

	float pTom1Pitch   [NUM_PROGRAMS];
	float pTom2Pitch   [NUM_PROGRAMS];
	float pTom3Pitch   [NUM_PROGRAMS];
	float pTomPanWidth [NUM_PROGRAMS];

	float pOutputGain [NUM_PROGRAMS];

protected:

	char ProgramName[NUM_PROGRAMS][MAX_NAME_LEN];

	float Chunk[CHUNK_SIZE];

	float FloatToHz(float value,float range);
	int FloatToNoisePeriod(float value);

	VstInt32 SavePresetChunk(float *chunk);
	void LoadPresetChunk(float *chunk);

	VstInt32 SaveStringChunk(char *str,float *dst);
	VstInt32 LoadStringChunk(char *str,int max_length,float *src);

	void MidiAddNewNote(VstInt32 delta,VstInt32 note,VstInt32 velocity);
	void MidiAddProgramChange(VstInt32 delta,VstInt32 program);

	vector<MidiQueueStruct> MidiQueue;

	SynthChannelStruct SynthChannel[SYNTH_CHANNELS];

	void SynthRestartTone(VstInt32 chn);
	void SynthRestartNoise(VstInt32 chn);

	float* getVarPtr(VstInt32 index);

	bool isPatchLevelVar(VstInt32 index);

	unsigned char Noise[65536];

	float sampleRate;

	bool UpdateGuiFlag;

	float CopyBuf[64];

	bool CopyBufActive;
};
