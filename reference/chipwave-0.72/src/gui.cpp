#include "ChipWave.h"
#include "GUI.h"

#include "resource.h"



extern void* hInstance;

inline HINSTANCE GetInstance() { return (HINSTANCE)hInstance; }



const char* GUI::GetClassName(void)
{
	sprintf(ClassName, "CHWA%08x", (unsigned long)(LONG_PTR)this);

	return (const char*)ClassName;
}



GUI::GUI(AudioEffect* effect)
{
	WNDCLASS wc;

	synth=(ChipWave*)effect;

	wc.style=CS_GLOBALCLASS;
	wc.lpfnWndProc=WndProc;
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
	
	
	
bool GUI::open(void* ptr)
{
	HWND parent;
	HINSTANCE hinst;
	BITMAP bm;
	VstInt32 sx,sy;

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

	hWndForm=CreateWindow(GetClassName(),"ChipWave",WS_CHILD|WS_VISIBLE,0,0,(rect.right-rect.left),(rect.bottom-rect.top),parent,0,hinst,this);

	SetWindowLongPtr(hWndForm,GWLP_USERDATA,(LONG_PTR)this);

	//create all controls

	guiHover=-1;
	guiFocus=-1;
	guiFocusOriginX=0;
	guiFocusOriginY=0;

	guiControlCount=0;

	sx=150;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdOscAWave,4,false);
	SliderAdd(sx+1*30,sy,20,128,pIdOscADuty,-1,true);
	SliderAdd(sx+2*30,sy,20,128,pIdOscAOver,-1,true);
	SliderAdd(sx+3*30,sy,20,128,pIdOscACut,0,true);
	SliderAdd(sx+4*30,sy,20,128,pIdOscAMultiple,15,true);
	SliderAdd(sx+5*30,sy,20,128,pIdOscASeed,0,true);

	sx=150;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdOscBWave    , 4,false);
	SliderAdd(sx+1*30,sy,20,128,pIdOscBDuty    ,-1,true);
	SliderAdd(sx+2*30,sy,20,128,pIdOscBOver    ,-1,true);
	SliderAdd(sx+3*30,sy,20,128,pIdOscBCut     , 0,true);
	SliderAdd(sx+4*30,sy,20,128,pIdOscBMultiple,15,true);
	SliderAdd(sx+5*30,sy,20,128,pIdOscBSeed    ,0,true);
	SliderAdd(sx+6*30,sy,20,128,pIdOscBDetune  ,0,true);

	sx=354;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdOscBalance,-1,false);
	SliderAdd(sx+1*30,sy,20,128,pIdOscMixMode, 2,true);

	sx=439;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdEnvAttack ,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdEnvDecay  ,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdEnvSustain,0,true);
	SliderAdd(sx+3*30,sy,20,128,pIdEnvRelease,0,true);

	sx=786;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdAmpAttack ,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdAmpDecay  ,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdAmpSustain,0,true);
	SliderAdd(sx+3*30,sy,20,128,pIdAmpRelease,0,true);

	sx=409;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdEnvOscADepth      ,-1,true);
	SliderAdd(sx+1*30,sy,20,128,pIdEnvOscBDepth      ,-1,true);
	SliderAdd(sx+2*30,sy,20,128,pIdEnvOscBDetuneDepth, 0,true);
	SliderAdd(sx+3*30,sy,20,128,pIdEnvOscMixDepth    ,-1,true);
	SliderAdd(sx+4*30,sy,20,128,pIdEnvFltDepth       ,-1,true);
	SliderAdd(sx+5*30,sy,20,128,pIdEnvLfoDepth       , 0,true);

	sx=611;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdLfoSpeed      , 0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdLfoPitchDepth , 0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdLfoOscADepth  ,-1,true);
	SliderAdd(sx+3*30,sy,20,128,pIdLfoOscBDepth  ,-1,true);
	SliderAdd(sx+4*30,sy,20,128,pIdLfoOscMixDepth,-1,true);
	SliderAdd(sx+5*30,sy,20,128,pIdLfoFltDepth   ,-1,true);

	sx=813;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdVelAmp        , 0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdVelOscADepth  ,-1,true);
	SliderAdd(sx+2*30,sy,20,128,pIdVelOscBDepth  ,-1,true);
	SliderAdd(sx+3*30,sy,20,128,pIdVelOscMixDepth,-1,true);
	SliderAdd(sx+4*30,sy,20,128,pIdVelFltCutoff  ,-1,true);
	SliderAdd(sx+5*30,sy,20,128,pIdVelFltReso    ,-1,true);

	sx=930;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdFltCutoff,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdFltReso  ,0,true);

	sx=1015;
	sy=77;

	SliderAdd(sx+0*30,sy,20,128,pIdSlideDelay,0,true);
	SliderAdd(sx+1*30,sy,20,128,pIdSlideSpeed,-1,true);
	SliderAdd(sx+2*30,sy,20,128,pIdSlideRoute,3,true);

	sx=1015;
	sy=250;

	SliderAdd(sx+0*30,sy,20,128,pIdPolyphony ,2,true);
	SliderAdd(sx+1*30,sy,20,128,pIdPortaSpeed,0,true);
	SliderAdd(sx+2*30,sy,20,128,pIdOutputGain,0,true);

	synth->UpdateGUI(true);

	RenderActive=false;

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



//steps 0 no steps, 1..N steps, -1 zero at the middle

void GUI::SliderAdd(VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h,VstInt32 param,VstInt32 steps,bool invert)
{
	GUISliderStruct *s;

	s=&guiControlList[guiControlCount++];

	s->x=x;
	s->y=y;
	s->w=w;
	s->h=h;

	s->param=param;
	s->steps=steps;
	s->invert=invert;
}



void GUI::SliderSet(GUISliderStruct *s,float value)
{
	if(value<0.0f) value=0.0f;
	if(value>1.0f) value=1.0f;

	if(s->invert) value=1.0f-value;

	synth->setParameter(s->param,value);
} 



float GUI::SliderGet(GUISliderStruct *s)
{
	float value;

	value=synth->getParameter(s->param);

	if(s->invert) value=1.0f-value;

	return value;
}



void GUI::SliderRender(GUISliderStruct *s,HDC hdc,bool active)
{
	float fy;
	VstInt32 i,sy;

	//SetDCBrushColor(hdc,COL_FRAME);
	//SetDCPenColor  (hdc,COL_FRAME);

	//Rectangle(hdc,s->x,s->y,s->x+s->w,s->y+s->h);

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
	SetDCBrushColor(hdc,active?COL_ACTIVE:COL_HANDLE);

	Rectangle(hdc,s->x+1,sy-GUI_HANDLE_HEIGHT/2,s->x+s->w-1,sy+GUI_HANDLE_HEIGHT/2);
}



VstInt32 GUI::SliderCheckArea(GUISliderStruct *s,VstInt32 mx,VstInt32 my)
{
	if(mx<s->x||mx>=s->x+s->w||my<s->y||my>=s->y+s->h) return FALSE;

	return TRUE;
}



void GUI::SliderCheckChange(GUISliderStruct *s,VstInt32 mx,VstInt32 my,VstInt32 click)
{
	float fy,value;

	if(my<s->y) my=s->y;
	if(my>=s->y+s->h) my=s->y+s->h;

	fy=(float)(my-(s->y+(GUI_HANDLE_HEIGHT/2+1)));

	value=fy/(float)(s->h-(GUI_HANDLE_HEIGHT+2));

	switch(click)
	{
	case GUI_L_DOWN:
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

	case GUI_R_DOWN:
		{
			SliderSet(s,.5f);
		}
		break;

	case GUI_L_UP:
		{
			if(s->steps>0)
			{
				value=value*((float)s->steps-.01f);
				value=floorf(value)/(float)(s->steps-1);

				SliderSet(s,value);
			}
		}
		break;

	case GUI_R_UP:
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



void GUI::RenderWaveform(HDC hdc,VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h,VstInt32 osc)
{
	VstInt32 i,sx,sy,wave;
	float fh,fy,duty,over;
	POINT pt;
	HPEN hPen;
	SynthOscStruct oscn;

	//SelectObject(hdc,GetStockObject(DC_PEN));

	//SetDCBrushColor(hdc,COL_FRAME);
	//SetDCPenColor  (hdc,COL_FRAME);

	//Rectangle(hdc,x,y,x+w,y+h);

	hPen=CreatePen(PS_SOLID,2,COL_GRAPH);

	SelectObject(hdc,hPen);

	fh=(float)h;
	sx=x;
	
	switch(osc)
	{
	case 0:
		{
			duty=synth->pOscADuty[synth->Program];

			over=synth->OverdriveValue(synth->pOscAOver[synth->Program]);

			wave=(VstInt32)(synth->pOscAWave[synth->Program]*3.99f);
		
			oscn.noise_seed=(VstInt32)(synth->pOscASeed[synth->Program]*65535.99f);
		}
		break;

	case 1:
		{
			duty=synth->pOscBDuty[synth->Program];

			over=synth->OverdriveValue(synth->pOscBOver[synth->Program]);

			wave=(VstInt32)(synth->pOscBWave[synth->Program]*3.99f);
			
			oscn.noise_seed=(VstInt32)(synth->pOscBSeed[synth->Program]*65535.99f);
		}
		break;
	}

	oscn.acc=0;
	oscn.noise=0;
	oscn.noise_seed=0;

	for(i=0;i<w;++i)
	{
		fy=y+fh/2.0f-fh*synth->SynthGetSample(&oscn,over,duty,wave)*.9f;

		sy=(VstInt32)fy;

		if(!i) MoveToEx(hdc,sx,sy,&pt);

		LineTo(hdc,sx,sy);

		++sx;

		oscn.acc+=1.0f/w*2.0f;	//show two periods

		while(oscn.acc>=1.0f) oscn.acc-=1.0f;

		++oscn.noise;
	}

	DeleteObject(hPen);
}



void GUI::RenderEnvelope(HDC hdc,VstInt32 x,VstInt32 y,VstInt32 w,VstInt32 h,VstInt32 env)
{
	VstInt32 i,sx,sy,dx,stage,skip;
	float fh,fy,level,attack,decay,sustain,release,scale;
	POINT pt;
	HPEN hPen;

	//SelectObject(hdc,GetStockObject(DC_PEN));

	//SetDCBrushColor(hdc,COL_FRAME);
	//SetDCPenColor  (hdc,COL_FRAME);

	//Rectangle(hdc,x,y,x+w,y+h);

	hPen=CreatePen(PS_SOLID,2,COL_GRAPH);

	SelectObject(hdc,hPen);

	fh=(float)h;
	sx=x;

	scale=25;
	level=0;

	switch(env)
	{
	case 0://envelope
		{
			attack= synth->SynthEnvelopeTimeToDelta(synth->pEnvAttack[synth->Program],ENVELOPE_ATTACK_MAX_MS)*scale;
			decay=  synth->SynthEnvelopeTimeToDelta(synth->pEnvDecay [synth->Program],ENVELOPE_DECAY_MAX_MS )*scale;
			sustain=synth->pEnvSustain[synth->Program];
			release=synth->SynthEnvelopeTimeToDelta(synth->pEnvRelease[synth->Program],ENVELOPE_RELEASE_MAX_MS)*scale;
		}
		break;

	case 1://amp
		{
			attack= synth->SynthEnvelopeTimeToDelta(synth->pAmpAttack[synth->Program],ENVELOPE_ATTACK_MAX_MS)*scale;
			decay=  synth->SynthEnvelopeTimeToDelta(synth->pAmpDecay [synth->Program],ENVELOPE_DECAY_MAX_MS )*scale;
			sustain=synth->pAmpSustain[synth->Program];
			release=synth->SynthEnvelopeTimeToDelta(synth->pAmpRelease[synth->Program],ENVELOPE_RELEASE_MAX_MS)*scale;
		}
		break;
	}

	stage=0;
	skip=0;
	dx=0;

	for(i=0;i<w;++i)
	{
		fy=y+h-fh*level;

		switch(stage)
		{
		case 0://attack
			{
				level+=attack;

				if(level>=1.0f)
				{
					level=1.0f;

					++stage;
				}
			}
			break;

		case 1://decay
			{
				level-=decay;

				if(level<sustain)
				{
					level=sustain;

					release*=level;

					++stage;
				}
			}
			break;

		case 2://sustain
			{
				++skip;

				if(skip>=50)
				{
					++stage;
					dx=sx;
				}
			}
			break;

		case 3://release
			{
				level-=release;

				if(level<0) level=0;
			}
			break;
		}

		sy=(VstInt32)fy;

		if(!i) MoveToEx(hdc,sx,sy,&pt);

		LineTo(hdc,sx,sy);

		++sx;
	}

	DeleteObject(hPen);

	//make release dotted

	SelectObject(hdc,GetStockObject(DC_PEN));
	SelectObject(hdc,GetStockObject(DC_BRUSH));

	SetDCBrushColor(hdc,COL_FRAME);
	SetDCPenColor  (hdc,COL_FRAME);

	while(dx<x+w)
	{
		if(stage==3&&!(dx&3)) Rectangle(hdc,dx,y,dx+2,y+h+2);

		++dx;
	}
}



void GUI::ProcessMouse(UINT message,WPARAM wParam,LPARAM lParam)
{
	VstInt32 i,mx,my,click;

	click=0;

	mx=GET_X_LPARAM(lParam); 
	my=GET_Y_LPARAM(lParam); 

	if(message==WM_LBUTTONDOWN||message==WM_MOUSEMOVE) if(wParam&MK_LBUTTON) click=GUI_L_DOWN;
	if(message==WM_RBUTTONDOWN||message==WM_MOUSEMOVE) if(wParam&MK_RBUTTON) click=GUI_R_DOWN;
	if(message==WM_LBUTTONUP) click=GUI_L_UP;
	if(message==WM_RBUTTONUP) click=GUI_R_UP;

	guiHover=-1;

	for(i=0;i<guiControlCount;++i)
	{
		if(SliderCheckArea(&guiControlList[i],mx,my))
		{
			if(guiFocus<0)
			{
				guiHover=i;
				guiHoverX=mx;
				guiHoverY=my;
			}

			if(message==WM_LBUTTONDOWN||message==WM_RBUTTONDOWN)
			{
				if(guiFocus<0)
				{
					guiFocus=guiHover;
					guiFocusOriginX=mx;
					guiFocusOriginY=my;
				}
			}
		}
	}

	if(guiFocus>=0)
	{
		SliderCheckChange(&guiControlList[guiFocus],mx,my,click);
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



void GUI::RenderAll(void)
{
	HDC hdcWnd,hdcSrc,hdcDst;
	PAINTSTRUCT ps;
	BITMAP bm;
	SIZE psizl;
	VstInt32 i,x,y,hover,param;
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
    
	//render controls

	SelectObject(hdcDst,GetStockObject(DC_PEN));
	SelectObject(hdcDst,GetStockObject(DC_BRUSH));

	for(i=0;i<guiControlCount;++i) SliderRender(&guiControlList[i],hdcDst,(i==guiHover||i==guiFocus)?true:false);

	RenderWaveform(hdcDst,25,112,110,80,0);
	RenderWaveform(hdcDst,25,268,110,80,1);

	RenderEnvelope(hdcDst,565,76,194,52,0);
	RenderEnvelope(hdcDst,578,154,194,52,1);

	//render info

	hover=guiHover;

	if(guiFocus>=0) hover=guiFocus;

	if(hover>=0)
	{
		param=guiControlList[hover].param;

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
		x=guiWidth-20-psizl.cx;
		y=15;

		TextOut(hdcDst,x,y,str,(int)strlen(str));
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