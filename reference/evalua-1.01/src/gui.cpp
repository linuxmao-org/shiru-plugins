#include "Evalua.h"
#include "GUI.h"



extern void* hInstance;

inline HINSTANCE GetInstance() { return (HINSTANCE)hInstance; }



const char* GUI::GetClassName(void)
{
	sprintf(ClassName,"EVALUA%08x",(unsigned long)(LONG_PTR)this);

	return (const char*)ClassName;
}



GUI::GUI(AudioEffect* effect)
{
	WNDCLASS wc;

	plug=(Evalua*)effect;

	rect.left  =0;
	rect.top   =0;
	rect.right =GUI_WINDOW_WDT;
	rect.bottom=GUI_WINDOW_HGT;

	wc.style=CS_GLOBALCLASS;
	wc.lpfnWndProc=(WNDPROC)WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=GetInstance();
	wc.hIcon=0;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName=0;
	wc.lpszClassName=GetClassName();

	RegisterClass(&wc);
}



GUI::~GUI()
{
	UnregisterClass(GetClassName(),GetInstance());
}
	
	
const char evalua_hint_text[] =
"DEC and HEX(with 0x prefix) integer numbers allowed\n"
"+ - * / % & | ^ << >> ( ) operators allowed (ADD,SUB,MUL,DIV,MOD,AND,OR,XOR,SHL,SHR)\n"
"P=pitch, depends on note, considers 256 per period; T=time since note start (65536 per second)\n"
"V=velocity 0..127; M=modulation wheel 0..127; R=random 0..32767";

bool GUI::open(void* ptr)
{
	HINSTANCE hinst;
	int x,y,w,h,style;

	AEffEditor::open(ptr);

	InitCommonControls();

	hinst=GetInstance();

	hWndForm=CreateWindow(GetClassName(),"Evalua",WS_CHILD|WS_VISIBLE,0,0,(rect.right-rect.left),(rect.bottom-rect.top),HWND(ptr),0,hinst,this);

	if(!hWndForm) return false;

	SetWindowLongPtr(hWndForm,GWLP_USERDATA,(LONG_PTR)this);

	style = WS_CHILD | WS_VISIBLE;

	x = 10;
	y = 10;
	w = GUI_WINDOW_WDT - x * 2;
	h = 25;

	hWndStaticError = CreateWindow("static", "ST_U", style, x, y, w, h, hWndForm, NULL, hinst, NULL);

	style = WS_VISIBLE | WS_CHILD | SS_OWNERDRAW;

	y = 5;
	w = 65;
	x = GUI_WINDOW_WDT - 10 - w;

	hWndButtonGain = CreateWindow("static", "ST_U", style, x, y, w, h, hWndForm, NULL, hinst, NULL);

	x -= w + 10;

	hWndButtonPorta = CreateWindow("static", "ST_U", style, x, y, w, h, hWndForm, NULL, hinst, NULL);

	x -= w + 10;

	hWndButtonPoly = CreateWindow("static", "ST_U", style, x, y, w, h, hWndForm, NULL, hinst, NULL);

	style = WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_UPPERCASE | ES_AUTOVSCROLL;

	x = 10;
	w = GUI_WINDOW_WDT - x * 2;
	y += h + 5;
	h = 25 * 5;

	hWndEditData = CreateWindow("edit", "0", style, x, y, w, h, hWndForm, NULL, hinst, NULL);

	SendMessage(hWndEditData, EM_SETLIMITTEXT, MAX_PROGRAM_LEN, 0);

	SetWindowLongPtr(hWndEditData, GWLP_USERDATA, 0xdeadf00b);	//Reaper hack to prevent hijacking Space button

	style = WS_CHILD | WS_VISIBLE;

	y += h + 10;
	h = 25 * 5;

	hWndStaticHint = CreateWindow("static", "ST_U", style, x, y, w, h, hWndForm, NULL, hinst, NULL);

	SetWindowText(hWndStaticHint, evalua_hint_text);

	hFontError = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));
	hFontData = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));
	hFontHint = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));

	SendMessage(hWndStaticError, WM_SETFONT, (WPARAM)hFontError, TRUE);
	SendMessage(hWndButtonPoly, WM_SETFONT, (WPARAM)hFontHint, TRUE);
	SendMessage(hWndButtonPorta, WM_SETFONT, (WPARAM)hFontHint, TRUE);
	SendMessage(hWndButtonGain, WM_SETFONT, (WPARAM)hFontHint, TRUE);
	SendMessage(hWndEditData, WM_SETFONT, (WPARAM)hFontData, TRUE);
	SendMessage(hWndStaticHint, WM_SETFONT, (WPARAM)hFontHint, TRUE);

	MouseActiveWnd = 0;

	SetUpdate(true);

	return true;
}



void GUI::close()
{
	DestroyWindow(hWndStaticError); 
	DestroyWindow(hWndStaticHint);
	DestroyWindow(hWndButtonPoly);
	DestroyWindow(hWndButtonPorta);
	DestroyWindow(hWndButtonGain);
	DestroyWindow(hWndEditData);
	DestroyWindow(hWndForm);

	DeleteObject(hFontError);
	DeleteObject(hFontData);
	DeleteObject(hFontHint);

	AEffEditor::close();
}



bool GUI::getRect(ERect **ppErect)
{
   *ppErect=&rect;

   return true;
}



void GUI::idle(void)
{
	int pgm;
	char buf[32];

	pgm=plug->ProgramIndex;

	if(NeedUpdateEdit)
	{
		NeedUpdateEdit=false;

		SetWindowText(hWndEditData, plug->Program[pgm].Data);
	}

	if(NeedUpdate)
	{
		NeedUpdate=false;

		const char* err = plug->Compile();

		SetWindowText(hWndStaticError, err ? err : "OK");

		if (plug->Polyphony > 1)
		{
			snprintf(buf, sizeof(buf), "Poly:%u", plug->Polyphony);
		}
		else
		{
			snprintf(buf, sizeof(buf), "Mono");
		}
		SetWindowText(hWndButtonPoly, buf);

		snprintf(buf, sizeof(buf), "Porta:%u", plug->PortaSpeed);
		SetWindowText(hWndButtonPorta, buf);

		snprintf(buf, sizeof(buf), "Gain:%u", plug->OutputGain);
		SetWindowText(hWndButtonGain, buf);

		InvalidateRect(hWndStaticError, NULL, true);
		InvalidateRect(hWndButtonPoly, NULL, true);
		InvalidateRect(hWndButtonPorta, NULL, true);
		InvalidateRect(hWndButtonGain, NULL, true);

		plug->updateDisplay();
	}
}



void GUI::SetUpdate(bool program)
{
	NeedUpdate=true;	
	NeedUpdateEdit=program;
}



LRESULT WINAPI GUI::WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpDIS;
	char tempstr[32];
	HWND phWnd;
	HDC phDC;
	GUI* gui;
	bool compile;
	int i, w, dx, len, val, max, pgm, color;
	RECT rect;
	POINT point;

	phWnd=(HWND)lParam;
	phDC =(HDC)wParam;
	
	gui=(GUI*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	if (!gui || !gui->plug || !phWnd) return DefWindowProc(hWnd, message, wParam, lParam);

	pgm = gui->plug->ProgramIndex;

	switch(message)
	{
		case WM_COMMAND:
		{
			if(HIWORD(wParam)==EN_CHANGE)	//any edit has been changed
			{
				compile = false;

				if (phWnd == gui->hWndEditData) { GetWindowText(phWnd, gui->plug->Program[pgm].Data, MAX_PROGRAM_LEN); compile = true; }

				if(compile)
				{
					gui->SetUpdate(false);
				}
			}
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			gui->MouseOrigin.x = GET_X_LPARAM(lParam);
			gui->MouseOrigin.y = GET_Y_LPARAM(lParam);

			ClientToScreen(hWnd, &gui->MouseOrigin);

			gui->MouseActiveWnd = 0;

			GetWindowRect(gui->hWndButtonPoly, &rect);

			if(PtInRect(&rect, gui->MouseOrigin)) gui->MouseActiveWnd = gui->hWndButtonPoly;

			GetWindowRect(gui->hWndButtonPorta, &rect);

			if (PtInRect(&rect, gui->MouseOrigin)) gui->MouseActiveWnd = gui->hWndButtonPorta;

			GetWindowRect(gui->hWndButtonGain, &rect);

			if (PtInRect(&rect, gui->MouseOrigin)) gui->MouseActiveWnd = gui->hWndButtonGain;

			if(gui->MouseActiveWnd)
			{
				SetCapture(hWnd);
			}
		}
		return 0;

		case WM_LBUTTONUP:
		{
			ReleaseCapture();

			gui->MouseActiveWnd = 0;
		}
		return 0;

		case WM_MOUSEMOVE:
		{
			point.x = GET_X_LPARAM(lParam);
			point.y = GET_Y_LPARAM(lParam);

			ClientToScreen(hWnd, &point);

			if (gui->MouseActiveWnd !=0)
			{
				dx = point.x - gui->MouseOrigin.x;
				
				if (gui->MouseActiveWnd == gui->hWndButtonPoly)
				{
					gui->plug->Polyphony += dx / 5;

					if (gui->plug->Polyphony < 1) gui->plug->Polyphony = 1;
					if (gui->plug->Polyphony > MAX_SYNTH_CHANNELS) gui->plug->Polyphony = MAX_SYNTH_CHANNELS;
				}
				
				if (gui->MouseActiveWnd == gui->hWndButtonPorta)
				{
					gui->plug->PortaSpeed += dx / 2;

					if (gui->plug->PortaSpeed < 1) gui->plug->PortaSpeed = 1;
					if (gui->plug->PortaSpeed > MAX_PORTA_SPEED) gui->plug->PortaSpeed = MAX_PORTA_SPEED;
				}

				if (gui->MouseActiveWnd == gui->hWndButtonGain)
				{
					gui->plug->OutputGain += dx / 2;

					if (gui->plug->OutputGain < 1) gui->plug->OutputGain = 1;
					if (gui->plug->OutputGain > MAX_GAIN) gui->plug->OutputGain = MAX_GAIN;
				}

				gui->MouseOrigin = point;

				gui->SetUpdate(false);
			}
		}
		return 0;

		case WM_DRAWITEM:
		{
			lpDIS = (LPDRAWITEMSTRUCT)lParam;

			SelectObject(lpDIS->hDC, GetStockObject(DC_BRUSH));
			SelectObject(lpDIS->hDC, GetStockObject(DC_PEN));

			SetBkMode(lpDIS->hDC, OPAQUE);

			SetDCBrushColor(lpDIS->hDC, GetSysColor(COLOR_BTNFACE));
			SetDCPenColor(lpDIS->hDC, GetSysColor(COLOR_WINDOWTEXT));

			RoundRect(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom, 5, 5);

			rect = lpDIS->rcItem;

			rect.top += 2;
			rect.left += 2;
			rect.right -= 2;
			rect.bottom -= 2;

			w = rect.right - rect.left; 
			
			max = w;
			val = 0;

			if (lpDIS->hwndItem == gui->hWndButtonGain)
			{
				max = MAX_GAIN;
				val = gui->plug->OutputGain;
			}

			if (lpDIS->hwndItem == gui->hWndButtonPoly)
			{
				max = MAX_SYNTH_CHANNELS;
				val = gui->plug->Polyphony;
			}

			if (lpDIS->hwndItem == gui->hWndButtonPorta)
			{
				max = MAX_PORTA_SPEED;
				val = gui->plug->PortaSpeed;
			}

			w = w * val / max;

			rect.right = rect.left + w;

			SetDCBrushColor(lpDIS->hDC, GetSysColor(COLOR_BTNSHADOW));
			SetDCPenColor(lpDIS->hDC, GetSysColor(COLOR_BTNSHADOW));

			RoundRect(lpDIS->hDC, rect.left, rect.top, rect.right, rect.bottom, 3, 3);

			len = GetWindowTextLength(lpDIS->hwndItem);

			GetWindowTextA(lpDIS->hwndItem, tempstr, len + 1);

			SetBkMode(lpDIS->hDC, TRANSPARENT);

			lpDIS->rcItem.top += 5;
			SetTextColor(lpDIS->hDC, GetSysColor(COLOR_BTNFACE));

			for (i = 0; i < 9; ++i)
			{
				rect = lpDIS->rcItem;
				rect.left = rect.left - 1 + (i % 3);
				rect.top = rect.top - 1 + ((i / 3) % 3);
				
				DrawTextA(lpDIS->hDC, tempstr, len, &rect, DT_CENTER);
			}

			SetTextColor(lpDIS->hDC, GetSysColor(COLOR_WINDOWTEXT));
			DrawTextA(lpDIS->hDC, tempstr, len, &lpDIS->rcItem, DT_CENTER);
		}
		return TRUE;

		case WM_CTLCOLORSTATIC:
		{
			color = GetSysColor(COLOR_BTNFACE);

			SetBkColor(phDC, color);
		}
		return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);

		case WM_ERASEBKGND:
		{

		}
		return TRUE;
	}

	return DefWindowProc(hWnd,message,wParam,lParam);
}