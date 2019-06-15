#include "ChipDrum.h"
#include "GUI.h"

#include "resource.h"



extern void* hInstance;

inline HINSTANCE GetInstance() { return (HINSTANCE)hInstance; }



const char* GUI::GetClassName(void)
{
	sprintf(ClassName, "CHDR%08x", (unsigned long)(LONG_PTR)this);

	return (const char*)ClassName;
}



GUI::GUI(AudioEffect* effect)
{
	WNDCLASS wc;

	synth=(ChipDrum*)effect;

	wc.style=CS_GLOBALCLASS;
	wc.lpfnWndProc=(WNDPROC)WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=GetInstance();
	wc.hIcon=0;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName=0;
	wc.lpszClassName= GetClassName();

	RegisterClass(&wc);
}



GUI::~GUI()
{
	UnregisterClass(GetClassName(),GetInstance());
}
	
	
	
bool GUI::open(void* ptr)
{
	HWND parent;
	HINSTANCE hinst;
	BITMAP bm;
	VstInt32 i,sx,sy;

	AEffEditor::open(ptr);

	parent=HWND(ptr);
	hinst=GetInstance();

	//load background image from resource

	hBitmapBG=(HBITMAP)LoadImage(GetInstance(),MAKEINTRESOURCE(IDI_BG_IMAGE),IMAGE_BITMAP,0,0,0);
	
	GetObject(hBitmapBG,sizeof(bm),&bm);

	guiWidth=bm.bmWidth;
	guiHeight=bm.bmHeight;

	//create image of the same size for double buffering
	//it needs to have the same format and palette, so just copy the background

	hBitmapBuf=(HBITMAP)CopyImage(hBitmapBG,IMAGE_BITMAP,0,0,0);

	//get window size from the background image and create window

	rect.left  =0;
	rect.top   =0;
	rect.right =guiWidth;
	rect.bottom=guiHeight;

	hWndForm=CreateWindow(GetClassName(),"ChipDrum",WS_CHILD|WS_VISIBLE,0,0,(rect.right-rect.left),(rect.bottom-rect.top),parent,0,hinst,this);

	SetWindowLongPtr(hWndForm,GWLP_USERDATA,(LONG_PTR)this);

	//create all controls

	guiHover=-1;
	guiFocus=-1;
	guiFocusOriginX=0;
	guiFocusOriginY=0;

	guiControlCount=0;

	sx=28;
	sy=199;

	for(i=0;i<8;++i)
	{
		ButtonAdd(sx,sy,50,22,true);

		sy+=24;
	}

	ButtonAdd(91,345,54,18,false);
	ButtonAdd(91,369,54,18,false);

	sx=165;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdToneWave,4,false);
	SliderAdd(sx+1*30,sy,20,128,pIdToneOver,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdTonePitch,0,true);
	SliderAdd(sx+3*30,sy,20,128,pIdToneSlide,-1,true);

	sx=308;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdToneLevel,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdToneDecay,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdToneSustain,0,true);
	SliderAdd(sx+3*30,sy,20,128,pIdToneRelease,0,true);

	sx=562;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdNoiseLevel,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdNoiseDecay,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdNoiseSustain,0,true);
	SliderAdd(sx+3*30,sy,20,128,pIdNoiseRelease,0,true);

	sx=165;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdNoiseType,2,true);
	SliderAdd(sx+1*30,sy,20,128,pIdNoisePeriod,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdNoiseSeed,0,true);
	SliderAdd(sx+3*30,sy,20,128,pIdNoisePitch1,0,true);
	SliderAdd(sx+4*30,sy,20,128,pIdNoisePitch2,0,true);
	SliderAdd(sx+5*30,sy,20,128,pIdNoisePitch2Off,0,true);
	SliderAdd(sx+6*30,sy,20,128,pIdNoisePitch2Len,0,true);

	sx=396;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdRetrigTime,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdRetrigCount,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdRetrigRoute,3,true);

	sx=509;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdFilterLP,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdFilterHP,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdFilterRoute,4,true);

	sx=620;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdVelDrumVolume,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdVelTonePitch,-1,true);
	SliderAdd(sx+2*30,sy,20,128,pIdVelNoisePitch,-1,true);
	SliderAdd(sx+3*30,sy,20,128,pIdVelToneOver,-1,true);

	sx=761;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdDrumVolume,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdDrumPan,-1,true);
	SliderAdd(sx+2*30,sy,20,128,pIdDrumGroup,4,true);
	SliderAdd(sx+3*30,sy,20,128,pIdDrumBitDepth,8,true);
	SliderAdd(sx+4*30,sy,20,128,pIdDrumUpdateRate,0,true);

	sx=702;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdHat1Length,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdHat2Length,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdHat3Length,0,true);
	SliderAdd(sx+3*30,sy,20,128,pIdHatPanWidth,-1,true);

	sx=843;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdTom1Pitch,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdTom2Pitch,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdTom3Pitch,0,true);
	SliderAdd(sx+3*30,sy,20,128,pIdTomPanWidth,-1,true);

	sx=933;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdOutputGain,0,true);

	RenderActive=false;

	synth->UpdateGUI(true);

	return true;
}



void GUI::close()
{
	DestroyWindow(hWndForm);

	DeleteObject(hBitmapBuf);
	DeleteObject(hBitmapBG);

	AEffEditor::close();
}



bool GUI::getRect(ERect **ppErect)
{
   *ppErect=&rect;

   return true;
}



void GUI::ButtonAdd(VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h,bool hover)
{
	GUIControlStruct *s;

	s=&guiControlList[guiControlCount++];

	s->type=GUI_CTL_BUTTON;

	s->x=x;
	s->y=y;
	s->w=w;
	s->h=h;

	s->param=-1;
	s->hover=hover;
}



//steps 0 no steps, 1..N steps, -1 zero at the middle

void GUI::SliderAdd(VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h,VstInt32 param,VstInt32 steps,bool invert)
{
	GUIControlStruct *s;

	s=&guiControlList[guiControlCount++];

	s->type=GUI_CTL_SLIDER;

	s->x=x;
	s->y=y;
	s->w=w;
	s->h=h;

	s->param=param;
	s->steps=steps;
	s->invert=invert;
}



void GUI::SliderSet(GUIControlStruct *s,float value)
{
	if(value<0.0f) value=0.0f;
	if(value>1.0f) value=1.0f;

	if(s->invert) value=1.0f-value;

	synth->setParameter(s->param,value);
} 



float GUI::SliderGet(GUIControlStruct *s)
{
	float value;

	value=synth->getParameter(s->param);

	if(s->invert) value=1.0f-value;

	return value;
}



void GUI::ControlRender(GUIControlStruct *s,HDC hdc,bool hover,bool focus)
{
	float fy;
	VstInt32 i,sx,sy;

	//SetDCBrushColor(hdc,COL_FRAME);
	//SetDCPenColor  (hdc,COL_FRAME);

	//Rectangle(hdc,s->x,s->y,s->x+s->w,s->y+s->h);

	switch(s->type)
	{
	case GUI_CTL_SLIDER:
		{
			SetDCBrushColor(hdc,COL_STEPS);
			SetDCPenColor  (hdc,COL_STEPS);

			if(s->steps>0)
			{	
				fy=(float)s->y+(GUI_HANDLE_HEIGHT/2+1);

				for(i=0;i<s->steps;++i)
				{
					Rectangle(hdc,s->x+1,(VstInt32)fy,s->x+s->w-1,(VstInt32)fy+1);

					fy+=((float)(s->h-(GUI_HANDLE_HEIGHT+2)))/(float)(s->steps-1);
				}
			}

			if(s->steps<0)
			{
				fy=(float)s->y+(GUI_HANDLE_HEIGHT/2+1)+((float)(s->h-(GUI_HANDLE_HEIGHT+2)))/2;

				Rectangle(hdc,s->x+1,(VstInt32)fy,s->x+s->w-1,(VstInt32)fy+1);
			}

			fy=s->y+(GUI_HANDLE_HEIGHT/2+1)+((float)(s->h-(GUI_HANDLE_HEIGHT+2))*SliderGet(s));

			sy=(VstInt32)fy;

			SetDCPenColor  (hdc,COL_FRAME);
			SetDCBrushColor(hdc,hover||focus?COL_ACTIVE:COL_HANDLE);

			Rectangle(hdc,s->x+1,sy-GUI_HANDLE_HEIGHT/2,s->x+s->w-1,sy+GUI_HANDLE_HEIGHT/2);
		}
		break;

	case GUI_CTL_BUTTON:
		{
			if(s->hover)
			{
				if(focus)
				{
					SetDCBrushColor(hdc,COL_FRAME);
					SetDCPenColor  (hdc,COL_FRAME);
				}
				else
				{
					if(hover)
					{
						SetDCBrushColor(hdc,COL_ACTIVE);
						SetDCPenColor  (hdc,COL_ACTIVE);
					}
					else
					{
						SetDCBrushColor(hdc,COL_WHITE);
						SetDCPenColor  (hdc,COL_WHITE);
					}
				}

				sx=s->x+30;
				sy=s->y+4;

				Rectangle(hdc,sx,sy,sx+s->h-8,sy+s->h-8);
			}
		}
		break;
	}
}



VstInt32 GUI::ControlCheckArea(GUIControlStruct *s,VstInt32 mx,VstInt32 my)
{
	if(mx<s->x||mx>=s->x+s->w||my<s->y||my>=s->y+s->h) return FALSE;

	return TRUE;
}



void GUI::SliderCheckChange(GUIControlStruct *s,VstInt32 mx,VstInt32 my,VstInt32 click)
{
	float fy,value;

	if(s->type!=GUI_CTL_SLIDER) return;

	if(my<s->y) my=s->y;
	if(my>=s->y+s->h) my=s->y+s->h;

	fy=(float)(my-(s->y+(GUI_HANDLE_HEIGHT/2+1)));

	value=fy/(float)(s->h-(GUI_HANDLE_HEIGHT+2));

	switch(click)
	{
	case GUI_BTN_L_DOWN:
		{
			if(GetKeyState(VK_SHIFT)&0x8000)
			{
				value=SliderGet(s);

				value+=(float)(my-guiFocusOriginY)/2000.0f;

				guiFocusOriginY=my;
			}

			SliderSet(s,value);
		}
		break;

	case GUI_BTN_R_DOWN:
		{
			SliderSet(s,.5f);
		}
		break;

	case GUI_BTN_L_UP:
		{
			if(s->steps>0)
			{
				value=value*((float)s->steps-.01f);
				value=floorf(value)/(float)(s->steps-1);

				SliderSet(s,value);
			}
		}
		break;

	case GUI_BTN_R_UP:
		{
			if(s->steps>0)
			{
				value=.5f*((float)s->steps-.01f);
				value=floorf(value)/(float)(s->steps-1);

				SliderSet(s,value);
			}
		}
		break;
	}
}



void GUI::RenderWaveform(HDC hdc,VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h)
{
	VstInt32 i,sx,sy,wave,pn_ptr;
	float fh,fy,acc,over;
	POINT pt;
	HPEN hPen;

	//SelectObject(hdc,GetStockObject(DC_PEN));

	//SetDCBrushColor(hdc,COL_FRAME);
	//SetDCPenColor  (hdc,COL_FRAME);

	//Rectangle(hdc,x,y,x+w,y+h);

	hPen=CreatePen(PS_SOLID,2,COL_GRAPH1);

	SelectObject(hdc,hPen);

	pn_ptr=PN_PTR(synth->Program,synth->pNoteMappingInt);

	fh=(float)h;
	sx=x;

	over=synth->OverdriveValue(synth->pToneOver[pn_ptr]);

	wave=(VstInt32)(synth->pToneWave[pn_ptr]*3.99f);

	acc=0;

	for(i=0;i<w;++i)
	{
		fy=y+fh/2.0f-fh*synth->SynthGetSample(wave,acc,over)*.5f*.9f;

		sy=(VstInt32)fy;

		if(!i) MoveToEx(hdc,sx,sy,&pt);

		LineTo(hdc,sx,sy);

		++sx;

		acc+=1.0f/w*2.0f;	//show two periods

		while(acc>=1.0f) acc-=1.0f;
	}

	DeleteObject(hPen);
}



void GUI::RenderEnvelope(HDC hdc,VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h)
{
	VstInt32 i,sx,sy,pn_ptr;
	float time,time_decay,time_release,time_start,time_end;
	float fh,fy,level,decay,sustain,release,scale;
	POINT pt;
	HPEN hPen;

	//SelectObject(hdc,GetStockObject(DC_PEN));

	//SetDCBrushColor(hdc,COL_FRAME);
	//SetDCPenColor  (hdc,COL_FRAME);

	//Rectangle(hdc,x,y,x+w,y+h);

	scale=.005f;
	
	pn_ptr=PN_PTR(synth->Program,synth->pNoteMappingInt);

	//draw secondary noise burst

	hPen=CreatePen(PS_SOLID,2,COL_GRAPH3);

	SelectObject(hdc,hPen);

	fh=(float)h;
	sx=x;

	time=0;
	time_start=synth->pNoisePitch2Off[pn_ptr]*DECAY_TIME_MAX_MS /1000.0f;
	time_end  =synth->pNoisePitch2Len[pn_ptr]*NOISE_BURST_MAX_MS/1000.0f+time_start;

	for(i=0;i<w;++i)
	{
		if(time>=time_start&&time<time_end)
		{
			MoveToEx(hdc,sx,y,&pt);

			LineTo(hdc,sx,y+h);
		}

		++sx;

		time+=scale;
	}

	DeleteObject(hPen);

	//draw noise envelope

	hPen=CreatePen(PS_SOLID,2,COL_GRAPH2);

	SelectObject(hdc,hPen);

	fh=(float)h;
	sx=x;

	time=0;
	time_decay  =synth->pNoiseDecay  [pn_ptr]*DECAY_TIME_MAX_MS  /1000.0f;
	time_release=synth->pNoiseRelease[pn_ptr]*RELEASE_TIME_MAX_MS/1000.0f;

	if(time_decay==0) time_decay=.01f;
	if(time_release==0) time_release=.01f;

	level  =synth->pNoiseLevel  [pn_ptr];
	sustain=synth->pNoiseSustain[pn_ptr]*level;
	decay  =(level-sustain)*scale/time_decay;
	release=sustain*scale/time_release;

	for(i=0;i<w;++i)
	{
		fy=y+h-fh*level;

		if(time<time_decay)
		{
			level-=decay;

			if(level<sustain) level=sustain;
		}
		else
		{
			level-=release;

			if(level<0) level=0;
		}

		sy=(VstInt32)fy;

		if(!i) MoveToEx(hdc,sx,sy,&pt);

		LineTo(hdc,sx,sy);

		++sx;

		time+=scale;
	}

	DeleteObject(hPen);

	//draw tone envelope

	hPen=CreatePen(PS_SOLID,2,COL_GRAPH1);

	SelectObject(hdc,hPen);

	fh=(float)h;
	sx=x;

	time=0;
	time_decay  =synth->pToneDecay  [pn_ptr]*DECAY_TIME_MAX_MS  /1000.0f;
	time_release=synth->pToneRelease[pn_ptr]*RELEASE_TIME_MAX_MS/1000.0f;

	if(time_decay==0) time_decay=.01f;
	if(time_release==0) time_release=.01f;

	level  =synth->pToneLevel  [pn_ptr];
	sustain=synth->pToneSustain[pn_ptr]*level;
	decay  =(level-sustain)*scale/time_decay;
	release=sustain*scale/time_release;

	for(i=0;i<w;++i)
	{
		fy=y-2+h-fh*level;

		if(time<time_decay)
		{
			level-=decay;

			if(level<sustain) level=sustain;
		}
		else
		{
			level-=release;

			if(level<0) level=0;
		}

		sy=(VstInt32)fy;

		if(!i) MoveToEx(hdc,sx,sy,&pt);

		LineTo(hdc,sx,sy);

		++sx;

		time+=scale;
	}

	DeleteObject(hPen);
}



void GUI::ProcessMouse(UINT message,WPARAM wParam,LPARAM lParam)
{
	VstInt32 i,mx,my,click;
	GUIControlStruct *s;

	click=0;

	mx=GET_X_LPARAM(lParam); 
	my=GET_Y_LPARAM(lParam); 

	if(message==WM_LBUTTONDOWN||message==WM_MOUSEMOVE) if(wParam&MK_LBUTTON) click=GUI_BTN_L_DOWN;
	if(message==WM_RBUTTONDOWN||message==WM_MOUSEMOVE) if(wParam&MK_RBUTTON) click=GUI_BTN_R_DOWN;
	if(message==WM_LBUTTONUP) click=GUI_BTN_L_UP;
	if(message==WM_RBUTTONUP) click=GUI_BTN_R_UP;

	guiHover=-1;

	for(i=0;i<guiControlCount;++i)
	{
		s=&guiControlList[i];

		if(ControlCheckArea(s,mx,my))
		{
			if(guiFocus<0)
			{
				guiHover=i;
				guiHoverX=mx;
				guiHoverY=my;
			}

			if(message==WM_LBUTTONDOWN||message==WM_RBUTTONDOWN)
			{
				guiFocus=guiHover;
				guiFocusOriginX=mx;
				guiFocusOriginY=my;
			}
		}
	}

	if(guiFocus>=0)
	{
		if(guiFocus<10)
		{
			if(message==WM_LBUTTONDOWN)
			{
				if(guiFocus<8)	//select drum
				{
					synth->setParameter(pIdNoteMapping,(float)(1+guiFocus)/SYNTH_NOTES);
				}
				else
				if(guiFocus==8)	//copy
				{
					synth->CopyDrum();
				}
				else
				if(guiFocus==9)	//paste
				{
					synth->PasteDrum();
				}
			}
		}
		else
		{
			SliderCheckChange(&guiControlList[guiFocus],mx,my,click);
		}
	}

	if(message==WM_LBUTTONUP||message==WM_RBUTTONUP)
	{
		guiFocus=-1;
	}
}



void GUI::Update(void)
{
	InvalidateRect(hWndForm,NULL,false);
}



void GUI::RenderControls(HDC hdc)
{
	GUIControlStruct *s;
	VstInt32 i;

	SelectObject(hdc,GetStockObject(DC_PEN));
	SelectObject(hdc,GetStockObject(DC_BRUSH));

	for(i=0;i<guiControlCount;++i)
	{
		s=&guiControlList[i];

		switch(s->type)
		{
		case GUI_CTL_SLIDER:
			ControlRender(s,hdc,(i==guiHover)?true:false,(i==guiFocus)?true:false);
			break;

		case GUI_CTL_BUTTON:
			ControlRender(s,hdc,(i==guiHover)?true:false,(i==guiFocus||i==synth->pNoteMappingInt)?true:false);
			break;
		}
	}
}



void GUI::RenderAll(void)
{
	HDC hdcWnd,hdcSrc,hdcDst;
	PAINTSTRUCT ps;
	BITMAP bm;
	SIZE psizl;
	VstInt32 x,y,hover,param;
	char str[1024],temp[1024];
	
	if(RenderActive) return;

	RenderActive=true;

	//copy the background image to the buffer

    hdcSrc=CreateCompatibleDC(NULL);
    hdcDst=CreateCompatibleDC(NULL);

    SelectObject(hdcSrc,hBitmapBG);
    SelectObject(hdcDst,hBitmapBuf);

    GetObject(hBitmapBG,sizeof(bm),&bm);

    BitBlt(hdcDst,0,0,bm.bmWidth,bm.bmHeight,hdcSrc,0,0,SRCCOPY);

    DeleteDC(hdcSrc);
    
	RenderControls(hdcDst);

	RenderWaveform(hdcDst,39,76,110,80);

	RenderEnvelope(hdcDst,428,96,124,90);

	//render info

	hover=guiHover;

	if(guiFocus>=0) hover=guiFocus;

	if(hover>=0)
	{
		param=guiControlList[hover].param;

		if(param>=0)
		{
			synth->getParameterName(param,temp);

			strcpy(str," ");
			strcat(str,temp);
			strcat(str," : ");

			synth->getParameterDisplay(param,temp);

			strcat(str,temp);

			synth->getParameterLabel(param,temp);

			strcat(str,temp);
			strcat(str," ");

			SetTextColor(hdcDst,COL_WHITE);
			SetBkColor  (hdcDst,COL_FRAME);

			GetTextExtentPoint32(hdcDst,str,(int)strlen(str),&psizl);
	/*
			x=guiHoverX;
			y=guiHoverY;

			if(x<guiWidth/2) x=x+20; else x=x-20-psizl.cx;
	*/
			x=guiWidth-15-psizl.cx;
			y=15;

			TextOut(hdcDst,x,y,str,(int)strlen(str));
		}
	}

	DeleteDC(hdcDst);

	//draw the buffer to the form

	hdcWnd=BeginPaint(hWndForm,&ps);

	hdcSrc=CreateCompatibleDC(hdcWnd);

	SelectObject(hdcSrc,hBitmapBuf);

	BitBlt(hdcWnd,0,0,guiWidth,guiHeight,hdcSrc,0,0,SRCCOPY);

	DeleteDC(hdcSrc);

	EndPaint(hWndForm,&ps);

	RenderActive=false;
}



LRESULT WINAPI GUI::WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	GUI *gui;

	gui=(GUI*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch(message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		{
			if(message==WM_LBUTTONDOWN) SetCapture(hWnd);
			if(message==WM_LBUTTONUP) ReleaseCapture();

			gui->ProcessMouse(message,wParam,lParam);
			
			InvalidateRect(hWnd,NULL,false);
		}
		return 0;

	case WM_PAINT:
		{
			gui->RenderAll();
		}
		return 0;
	}

	return DefWindowProc(hWnd,message,wParam,lParam);
}