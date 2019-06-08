#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <math.h>
#include <vector>

using namespace std;


#include "aeffeditor.h"



#define GUI_WINDOW_WDT		595
#define GUI_WINDOW_HGT		240



class GUI:public AEffEditor
{
	Evalua *plug;

private:

	char ClassName[32];

	HWND hWndForm;
	HWND hWndStaticError; 
	HWND hWndStaticHint;
	HWND hWndButtonPoly;
	HWND hWndButtonPorta;
	HWND hWndButtonGain;
	HWND hWndEditData;

	HFONT hFontError;
	HFONT hFontData;
	HFONT hFontHint;

	ERect rect;

	HWND MouseActiveWnd;
	POINT MouseOrigin;

	const char* GetClassName(void);

	static LRESULT WINAPI WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

	bool NeedUpdate;
	bool NeedUpdateEdit;

public:

	GUI(AudioEffect* effect);

	~GUI();

	virtual bool open(void* ptr);

	virtual void close();

	virtual void idle();

	virtual bool getRect(ERect **ppRect);

	void SetUpdate(bool program);
};
