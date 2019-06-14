#define _GUI_ACTIVE_

#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include <vector>

using namespace std;


#include "aeffeditor.h"



#define COL_FRAME	RGB(80,70,110)
#define COL_STEPS	RGB(150,140,180)
#define COL_HANDLE	RGB(255,255,255)
#define COL_ACTIVE	RGB(190,180,210)
#define COL_GRAPH	RGB(120,160,220)
#define COL_WHITE	RGB(255,255,255)


enum {

	GUI_NONE=0,
	GUI_L_DOWN,
	GUI_R_DOWN,
	GUI_L_UP,
	GUI_R_UP

};



#define GUI_HANDLE_HEIGHT	14

#define GUI_MAX_CONTROLS	64



struct GUISliderStruct {

	VstInt32 x,y;
	VstInt32 w,h;

	VstInt32 param;
	VstInt32 steps;

	bool invert;

};	



class GUI:public AEffEditor
{
private:

	ChipWave *synth;
	
	char ClassName[32];

	HWND hWndForm;

	HBITMAP hBitmapBG;
	HBITMAP hBitmapBuf;

	VstInt32 guiWidth;
	VstInt32 guiHeight;

	ERect rect;

	GUISliderStruct guiControlList[GUI_MAX_CONTROLS];

	VstInt32 guiControlCount;

	VstInt32 guiHover;
	VstInt32 guiHoverX;
	VstInt32 guiHoverY;

	VstInt32 guiFocus;
	VstInt32 guiFocusOriginX;
	VstInt32 guiFocusOriginY;

	const char* GetClassName(void);

	static LRESULT WINAPI WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

	void SliderAdd(VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h,VstInt32 param,VstInt32 steps,bool invert);
	void SliderSet(GUISliderStruct *s,float value);
	float SliderGet(GUISliderStruct *s);
	void SliderRender(GUISliderStruct *s,HDC hdc,bool active);

	VstInt32 SliderCheckArea(GUISliderStruct *s,VstInt32 mx,VstInt32 my);
	void SliderCheckChange(GUISliderStruct *s,VstInt32 mx,VstInt32 my,VstInt32 click);

	void ProcessMouse(UINT message,WPARAM wParam,LPARAM lParam);

	void RenderWaveform(HDC hdc,VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h,VstInt32 osc);
	void RenderEnvelope(HDC hdc,VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h,VstInt32 env);
	void RenderAll(void);

	bool RenderActive;

public:

	GUI(AudioEffect* effect);

	~GUI();

	virtual bool open(void* ptr);

	virtual void close();

	virtual bool getRect(ERect **ppRect);

	void Update(void);
};
