#include "ChipDrumUI.hpp"
#include "ChipDrumPresets.hpp"
#include "Window.hpp"
#include "Artwork.hpp"
#include "common/components/ValueFill.hpp"
#include "common/components/ToggleButton.hpp"
#include "common/components/TriggerButton.hpp"
#include "common/components/TextEdit.hpp"
#include "common/components/ColorPalette.hpp"
#include "common/components/Pango.hpp"
#include <cstdio>

ChipDrumUI::ChipDrumUI()
    : UI(974, 412),
      fControls(new std::unique_ptr<DGL::Widget>[Parameter_Count]),
      fControlNumSteps(new int[Parameter_Count]())
{
    for (unsigned p = 0; p < Parameter_Count; ++p)
        InitParameter(p, fParameters[p]);

    TextEdit *nameEdit = new TextEdit(this);
    fNameEdit.reset(nameEdit);
    nameEdit->setAbsolutePos(300, 15);
    nameEdit->setSize(350, 18);
    nameEdit->setFont("Monospace 10");
    nameEdit->ValueChangedCallback = [this](const std::string &text) { setState("ProgramName", text.c_str()); };

    ColorPalette &cp = ColorPalette::getDefault();
    cp.valuefill_fill = ColorRGBA{0xff, 0xff, 0xff, 0x40};
    cp.textedit_frame = ColorRGBA{220, 90, 128, 0xff};

    int sx, sy;

    sx=28;
    sy=199;

    for(unsigned i = 0; i < 8; ++i) {
        //ToggleButtonAdd(sx,sy,50,22,true);
        ToggleButtonAdd(sx+28,sy,22,22,true);

        ToggleButton *button = fToggleButtons.back().get();
        fNoteSelectButton[i] = button;

        button->setValue(fSelectedNoteNumber == i);
        button->ValueChangedCallback =
            [this, i](bool value) {
                if (value)
                    selectNote(i);
                for (unsigned n = 0; n < SYNTH_NOTES; ++n)
                    fToggleButtons[n]->setValue(fSelectedNoteNumber == n);
            };

        sy+=24;
    }

    TriggerButtonAdd(91,345,54,18,false);
    fTriggerButtons.back()->TriggeredCallback = [this] { CopyDrum(); };
    TriggerButtonAdd(91,369,54,18,false);
    fTriggerButtons.back()->TriggeredCallback = [this] { PasteDrum(); };

    sx=165;
    sy=77;

    SliderAdd(sx+0*30,sy,20,128,pIdToneWave1,4,false);
    SliderAdd(sx+1*30,sy,20,128,pIdToneOver1,0,true);
    SliderAdd(sx+2*30,sy,20,128,pIdTonePitch1,0,true);
    SliderAdd(sx+3*30,sy,20,128,pIdToneSlide1,-1,true);

    sx=308;
    sy=77;

    SliderAdd(sx+0*30,sy,20,128,pIdToneLevel1,0,true);
    SliderAdd(sx+1*30,sy,20,128,pIdToneDecay1,0,true);
    SliderAdd(sx+2*30,sy,20,128,pIdToneSustain1,0,true);
    SliderAdd(sx+3*30,sy,20,128,pIdToneRelease1,0,true);

    sx=562;
    sy=77;

    SliderAdd(sx+0*30,sy,20,128,pIdNoiseLevel1,0,true);
    SliderAdd(sx+1*30,sy,20,128,pIdNoiseDecay1,0,true);
    SliderAdd(sx+2*30,sy,20,128,pIdNoiseSustain1,0,true);
    SliderAdd(sx+3*30,sy,20,128,pIdNoiseRelease1,0,true);

    sx=165;
    sy=250;

    SliderAdd(sx+0*30,sy,20,128,pIdNoiseType1,2,true);
    SliderAdd(sx+1*30,sy,20,128,pIdNoisePeriod1,0,true);
    SliderAdd(sx+2*30,sy,20,128,pIdNoiseSeed1,0,true);
    SliderAdd(sx+3*30,sy,20,128,pIdNoisePitch11,0,true);
    SliderAdd(sx+4*30,sy,20,128,pIdNoisePitch21,0,true);
    SliderAdd(sx+5*30,sy,20,128,pIdNoisePitch2Off1,0,true);
    SliderAdd(sx+6*30,sy,20,128,pIdNoisePitch2Len1,0,true);

    sx=396;
    sy=250;

    SliderAdd(sx+0*30,sy,20,128,pIdRetrigTime1,0,true);
    SliderAdd(sx+1*30,sy,20,128,pIdRetrigCount1,0,true);
    SliderAdd(sx+2*30,sy,20,128,pIdRetrigRoute1,3,true);

    sx=509;
    sy=250;

    SliderAdd(sx+0*30,sy,20,128,pIdFilterLP1,0,true);
    SliderAdd(sx+1*30,sy,20,128,pIdFilterHP1,0,true);
    SliderAdd(sx+2*30,sy,20,128,pIdFilterRoute1,4,true);

    sx=620;
    sy=250;

    SliderAdd(sx+0*30,sy,20,128,pIdVelDrumVolume1,0,true);
    SliderAdd(sx+1*30,sy,20,128,pIdVelTonePitch1,-1,true);
    SliderAdd(sx+2*30,sy,20,128,pIdVelNoisePitch1,-1,true);
    SliderAdd(sx+3*30,sy,20,128,pIdVelToneOver1,-1,true);

    sx=761;
    sy=250;

    SliderAdd(sx+0*30,sy,20,128,pIdDrumVolume1,0,true);
    SliderAdd(sx+1*30,sy,20,128,pIdDrumPan1,-1,true);
    SliderAdd(sx+2*30,sy,20,128,pIdDrumGroup1,4,true);
    SliderAdd(sx+3*30,sy,20,128,pIdDrumBitDepth1,8,true);
    SliderAdd(sx+4*30,sy,20,128,pIdDrumUpdateRate1,0,true);

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
}

ChipDrumUI::~ChipDrumUI()
{
}

bool ChipDrumUI::onMotion(const MotionEvent &event)
{
    int mx = event.pos.getX();
    int my = event.pos.getY();

    int controlHovered = -1;
    for (unsigned p = 0; p < Parameter_Count && controlHovered == -1; ++p) {
        DGL::Widget *control = fControls[p].get();

        if (!control)
            continue;

        int x = control->getAbsoluteX();
        int y = control->getAbsoluteY();
        int w = control->getWidth();
        int h = control->getHeight();

        bool inside = mx >= x && mx < x + w && my >= y && my < y + h;
        if (inside)
            controlHovered = p;
    }

    if (fControlHovered != controlHovered) {
        fControlHovered = controlHovered;
        repaint();
    }

    return false;
}

void ChipDrumUI::onDisplay()
{
    const DGL::GraphicsContext &gc = getParentWindow().getGraphicsContext();
    cairo_t *cr = gc.cairo;

    if (!fGraphicsInitialized) {
        fBackgroundImage.loadFromPng(Artwork::backgroundData, Artwork::backgroundDataSize, &gc);
    }

    fBackgroundImage.draw();

    RenderWaveform(39,76,110,80);

    RenderEnvelope(428,96,124,90);

    int controlHovered = fControlHovered;
    if (controlHovered != -1) {
        PangoLayout_u layout(pango_cairo_create_layout(cr));

        std::string paramName = fParameters[controlHovered].name.buffer();

        int controlParameter = controlHovered;
        if (ParameterNoteNumber(controlParameter) != -1) {
            controlParameter += fSelectedNoteNumber;
            // delete name prefix
            unsigned temp, count;
            if (sscanf(paramName.c_str(), "Note %u %n", &temp, &count) == 1)
                paramName = paramName.substr(count);
        }

        std::string text = paramName + " : " + GetParameterDisplay(controlHovered, getControlValue(controlParameter));

        pango_layout_set_font_description(layout.get(), pango_font_description_from_string("Monospace 9"));
        pango_layout_set_text(layout.get(), text.c_str(), (int)text.size());

        PangoRectangle text_rect = {};
        pango_layout_get_extents(layout.get(), nullptr, &text_rect);

        double textw = pango_units_to_double(text_rect.width);
        double texth = pango_units_to_double(text_rect.height);
        double textx = getWidth() - 20 - textw;
        double texty = 15;

        cairo_matrix_t mat;
        cairo_get_matrix(cr, &mat);
        cairo_translate(cr, textx, texty);

        cairo_set_source_color(cr, ColorRGBA{220, 90, 128, 0xff});
        cairo_rectangle(cr, 0, 0, textw, texth);
        cairo_fill(cr);

        cairo_set_source_color(cr, ColorRGBA{0xff, 0xff, 0xff, 0xff});
        pango_cairo_show_layout(cr, layout.get());
        cairo_set_matrix(cr, &mat);
    }

    for (unsigned p = 0; p < Parameter_Count; ++p) {
        unsigned steps = fControlNumSteps[p];
        DGL::Widget *control = fControls[p].get();

        if (steps == 0)
            continue;

        DISTRHO_SAFE_ASSERT_CONTINUE(control);

        double cx = control->getAbsoluteX();
        double cy = control->getAbsoluteY();
        double cw = control->getWidth();
        double ch = control->getHeight();

        cairo_set_source_color(cr, ColorRGBA{0xff, 0xff, 0xff, 0x80});
        cairo_set_line_width(cr, 1.0);

        if ((int)steps == -1) {
            cairo_move_to(cr, cx, cy + 0.5 * ch);
            cairo_line_to(cr, cx + cw, cy + 0.5 * ch);
            cairo_stroke(cr);
        }
        else if (steps > 1) {
            double sh = ch / steps;
            for (unsigned i = 1; i < steps; ++i) {
                cairo_move_to(cr, cx, cy + i * sh);
                cairo_line_to(cr, cx + cw, cy + i * sh);
                cairo_stroke(cr);
            }
        }
    }
}

void ChipDrumUI::parameterChanged(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, );

    setControlValue(index, value);
}

void ChipDrumUI::programLoaded(uint32_t index)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    for (unsigned p = 0; p < Parameter_Count; ++p)
        parameterChanged(p, PresetData[index].values[p]);

    stateChanged("ProgramName", PresetData[index].name);
}

void ChipDrumUI::stateChanged(const char *key, const char *value)
{
    if (!strcmp(key, "ProgramName"))
        fNameEdit->setText(value);
}

void ChipDrumUI::ToggleButtonAdd(int32_t x, int32_t y, int32_t w, int32_t h, bool hover)
{
    ToggleButton *button = new ToggleButton(this);
    std::unique_ptr<ToggleButton> button_ptr(button);

    button->setSize(w, h);
    button->setAbsolutePos(x, y);

    fToggleButtons.push_back(std::move(button_ptr));
}

void ChipDrumUI::TriggerButtonAdd(int32_t x, int32_t y, int32_t w, int32_t h, bool hover)
{
    TriggerButton *button = new TriggerButton(this);
    std::unique_ptr<TriggerButton> button_ptr(button);

    button->setSize(w, h);
    button->setAbsolutePos(x, y);

    fTriggerButtons.push_back(std::move(button_ptr));
}

void ChipDrumUI::SliderAdd(int32_t x, int32_t y, int32_t w, int32_t h, int32_t param, int32_t steps, bool invert)
{
    ValueFill *control = new ValueFill(this);
    std::unique_ptr<ValueFill> control_ptr(control);

    control->setSize(w, h);
    control->setAbsolutePos(x, y);
    control->setOrientation(ValueFill::Vertical);

    fControlNumSteps[param] = steps;

    control->ValueChangedCallback =
        [this, param](double value) {
            int note = ParameterNoteNumber(param);
            int p = (note == -1) ? param : (param + fSelectedNoteNumber);
            fParameterValues[p] = value;
            setParameterValue(p, value);
        };

    if (!invert)
        control->setValueBounds(fParameters[param].ranges.max, fParameters[param].ranges.min);
    else
        control->setValueBounds(fParameters[param].ranges.min, fParameters[param].ranges.max);

    fControls[param] = std::move(control_ptr);
}

void ChipDrumUI::RenderWaveform(int32_t x, int32_t y, int32_t w, int32_t h)
{
    const DGL::GraphicsContext &gc = getParentWindow().getGraphicsContext();
    cairo_t *cr = gc.cairo;

    cairo_save(cr);

    int32_t i,sx,sy,wave;
    float fh,fy,acc,over;

    cairo_set_source_color(cr, ColorRGBA{245, 200, 215, 0xff});
    cairo_set_line_width(cr, 2);

    fh=(float)h;
    sx=x;

    over=OverdriveValue(fParameterValues[pIdToneOver1 + fSelectedNoteNumber]);

    wave=(int32_t)(fParameterValues[pIdToneWave1 + fSelectedNoteNumber]*3.99f);

    acc=0;

    cairo_new_path(cr);
    for(i=0;i<w;++i)
    {
        fy=y+fh/2.0f-fh*SynthGetSample(wave,acc,over)*.5f*.9f;

        sy=(int32_t)fy;

        if(!i) cairo_move_to(cr,sx,sy);

        cairo_line_to(cr,sx,sy);

        ++sx;

        acc+=1.0f/w*2.0f;    //show two periods

        while(acc>=1.0f) acc-=1.0f;
    }
    cairo_stroke(cr);

    cairo_restore(cr);
}

void ChipDrumUI::RenderEnvelope(int32_t x, int32_t y, int32_t w, int32_t h)
{
    const DGL::GraphicsContext &gc = getParentWindow().getGraphicsContext();
    cairo_t *cr = gc.cairo;

    cairo_save(cr);

    int32_t i,sx,sy;
    float time,time_decay,time_release,time_start,time_end;
    float fh,fy,level,decay,sustain,release,scale;

    scale=.005f;

    //draw secondary noise burst

    cairo_set_source_color(cr, ColorRGBA{240, 110, 150, 0xff});
    cairo_set_line_width(cr, 2);

    fh=(float)h;
    sx=x;

    time=0;
    time_start=fParameterValues[pIdNoisePitch2Off1 + fSelectedNoteNumber]*DECAY_TIME_MAX_MS /1000.0f;
    time_end  =fParameterValues[pIdNoisePitch2Len1 + fSelectedNoteNumber]*NOISE_BURST_MAX_MS/1000.0f+time_start;

    cairo_new_path(cr);
    for(i=0;i<w;++i)
    {
        if(time>=time_start&&time<time_end)
        {
            cairo_move_to(cr,sx,y);

            cairo_line_to(cr,sx,y+h);
        }

        ++sx;

        time+=scale;
    }
    cairo_stroke(cr);

    //draw noise envelope

    cairo_set_source_color(cr, ColorRGBA{180, 50, 88, 0xff});
    cairo_set_line_width(cr, 2);

    fh=(float)h;
    sx=x;

    time=0;
    time_decay  =fParameterValues[pIdNoiseDecay1 + fSelectedNoteNumber]*DECAY_TIME_MAX_MS  /1000.0f;
    time_release=fParameterValues[pIdNoiseRelease1 + fSelectedNoteNumber]*RELEASE_TIME_MAX_MS/1000.0f;

    if(time_decay==0) time_decay=.01f;
    if(time_release==0) time_release=.01f;

    level  =fParameterValues[pIdNoiseLevel1 + fSelectedNoteNumber];
    sustain=fParameterValues[pIdNoiseSustain1 + fSelectedNoteNumber]*level;
    decay  =(level-sustain)*scale/time_decay;
    release=sustain*scale/time_release;

    cairo_new_path(cr);
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

        sy=(int32_t)fy;

        if(!i) cairo_move_to(cr,sx,sy);

        cairo_line_to(cr,sx,sy);

        ++sx;

        time+=scale;
    }
    cairo_stroke(cr);

    //draw tone envelope

    cairo_set_source_color(cr, ColorRGBA{245, 200, 215, 0xff});
    cairo_set_line_width(cr, 2);

    fh=(float)h;
    sx=x;

    time=0;
    time_decay  =fParameterValues[pIdToneDecay1 + fSelectedNoteNumber]*DECAY_TIME_MAX_MS  /1000.0f;
    time_release=fParameterValues[pIdToneRelease1 + fSelectedNoteNumber]*RELEASE_TIME_MAX_MS/1000.0f;

    if(time_decay==0) time_decay=.01f;
    if(time_release==0) time_release=.01f;

    level  =fParameterValues[pIdToneLevel1 + fSelectedNoteNumber];
    sustain=fParameterValues[pIdToneSustain1 + fSelectedNoteNumber]*level;
    decay  =(level-sustain)*scale/time_decay;
    release=sustain*scale/time_release;

    cairo_new_path(cr);
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

        sy=(int32_t)fy;

        if(!i) cairo_move_to(cr,sx,sy);

        cairo_line_to(cr,sx,sy);

        ++sx;

        time+=scale;
    }
    cairo_stroke(cr);

    cairo_restore(cr);
}

void ChipDrumUI::CopyDrum()
{
    unsigned note = fSelectedNoteNumber;
    float *ptr = fCopyBuf;

    for (unsigned p = 0; p < Parameter_Count; ++p) {
        if (ParameterNoteNumber(p) == (int)note)
            *ptr++ = getControlValue(p);
    }

    fCopyBufActive = true;
}

void ChipDrumUI::PasteDrum()
{
    if (!fCopyBufActive)
        return;

    unsigned note = fSelectedNoteNumber;
    const float *ptr = fCopyBuf;

    for (unsigned p = 0; p < Parameter_Count; ++p) {
        if (ParameterNoteNumber(p) == (int)note)
            setControlValue(p, *ptr++);
    }
}

float ChipDrumUI::getControlValue(uint32_t index) const
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, 0);

    return fParameterValues[index];
}

void ChipDrumUI::setControlValue(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, );

    fParameterValues[index] = value;

    DGL::Widget *control = fControls[ParameterFirstOfGroup(index)].get();

    if (!control)
        return;

    int note = ParameterNoteNumber(index);
    if (note == -1 || (unsigned)note == fSelectedNoteNumber)
        static_cast<ValueFill *>(control)->setValue(value);
}

void ChipDrumUI::selectNote(unsigned note)
{
    if (fSelectedNoteNumber == note)
        return;

    fSelectedNoteNumber = note;

    for (unsigned p = 0; p < Parameter_Count; ++p) {
        if (ParameterNoteNumber(p) == (int)note)
            setControlValue(p, fParameterValues[p]);
    }
}

///
namespace DISTRHO {

UI *createUI()
{
    return new ChipDrumUI;
}

} // namespace DISTRHO
