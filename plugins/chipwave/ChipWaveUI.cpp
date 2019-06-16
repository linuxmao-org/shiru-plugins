#include "ChipWaveUI.hpp"
#include "ChipWavePresets.hpp"
#include "ChipWaveShared.hpp"
#include "Window.hpp"
#include "Artwork.hpp"
#include "common/components/ValueFill.hpp"
#include "common/components/TextEdit.hpp"
#include "common/components/ColorPalette.hpp"
#include "common/components/Pango.hpp"

ChipWaveUI::ChipWaveUI()
    : UI(1120, 410),
      fControls(new std::unique_ptr<DGL::Widget>[Parameter_Count]),
      fControlNumSteps(new int[Parameter_Count]())
{
    InitNoise(fNoise);

    TextEdit *nameEdit = new TextEdit(this);
    fNameEdit.reset(nameEdit);
    nameEdit->setAbsolutePos(345, 15);
    nameEdit->setSize(350, 18);
    nameEdit->setFont("Monospace 10");
    nameEdit->ValueChangedCallback = [this](const std::string &text) { setState("ProgramName", text.c_str()); };

    ColorPalette &cp = ColorPalette::getDefault();
    cp.valuefill_fill = ColorRGBA{0xff, 0xff, 0xff, 0x40};
    cp.textedit_frame = ColorRGBA{80, 70, 110, 0xff};

    int sx, sy;

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
}

ChipWaveUI::~ChipWaveUI()
{
}

bool ChipWaveUI::onMotion(const MotionEvent &event)
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

void ChipWaveUI::onDisplay()
{
    const DGL::GraphicsContext &gc = getParentWindow().getGraphicsContext();
    cairo_t *cr = gc.cairo;

    if (!fGraphicsInitialized) {
        fBackgroundImage.loadFromPng(Artwork::backgroundData, Artwork::backgroundDataSize, &gc);
    }

    fBackgroundImage.draw();

    RenderWaveform(25, 112, 110, 80, 0);
    RenderWaveform(25, 268, 110, 80, 1);

    RenderEnvelope(565, 76, 194, 52, 0);
    RenderEnvelope(578, 154, 194, 52, 1);

    int controlHovered = fControlHovered;
    if (controlHovered != -1) {
        PangoLayout_u layout(pango_cairo_create_layout(cr));

        ParameterName pn = GetParameterName(controlHovered);

        std::string text = std::string(pn.name) + " : " + GetParameterDisplay(controlHovered, getControlValue(controlHovered));

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

        cairo_set_source_color(cr, ColorRGBA{80, 70, 110, 0xff});
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

void ChipWaveUI::parameterChanged(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, );

    setControlValue(index, value);
}

void ChipWaveUI::programLoaded(uint32_t index)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    for (unsigned p = 0; p < Parameter_Count; ++p)
        parameterChanged(p, PresetData[index].values[p]);

    stateChanged("ProgramName", PresetData[index].name);
}

void ChipWaveUI::stateChanged(const char *key, const char *value)
{
    if (!strcmp(key, "ProgramName"))
        fNameEdit->setText(value);
}

void ChipWaveUI::SliderAdd(int32_t x, int32_t y, int32_t w, int32_t h, int32_t param, int32_t steps, bool invert)
{
    ValueFill *control = new ValueFill(this);
    std::unique_ptr<ValueFill> control_ptr(control);

    control->setSize(w, h);
    control->setAbsolutePos(x, y);
    control->setOrientation(ValueFill::Vertical);

    fControlNumSteps[param] = steps;

    control->ValueChangedCallback =
        [this, param](double value) { setParameterValue(param, value); };

    if (!invert)
        control->setValueBounds(1, 0);
    else
        control->setValueBounds(0, 1);

    fControls[param] = std::move(control_ptr);
}

void ChipWaveUI::RenderWaveform(int32_t x, int32_t y, int32_t w, int32_t h, int32_t osc)
{
    const DGL::GraphicsContext &gc = getParentWindow().getGraphicsContext();
    cairo_t *cr = gc.cairo;

    cairo_save(cr);

    int32_t i,sx,sy,wave;
    float fh,fy,duty,over;
    SynthOscObject oscn;

    cairo_set_source_color(cr, ColorRGBA{120, 160, 220, 0xff});
    cairo_set_line_width(cr, 2);

    fh=(float)h;
    sx=x;

    switch(osc)
    {
    case 0:
        {
            duty=getControlValue(pIdOscADuty);

            over=OverdriveValue(getControlValue(pIdOscAOver));

            wave=(int32_t)(getControlValue(pIdOscAWave)*3.99f);

            oscn.noise_seed=(int32_t)(getControlValue(pIdOscASeed)*65535.99f);
        }
        break;

    case 1:
        {
            duty=getControlValue(pIdOscBDuty);

            over=OverdriveValue(getControlValue(pIdOscBOver));

            wave=(int32_t)(getControlValue(pIdOscBWave)*3.99f);

            oscn.noise_seed=(int32_t)(getControlValue(pIdOscBSeed)*65535.99f);
        }
        break;

    default:
        DISTRHO_SAFE_ASSERT_RETURN(false, );
    }

    oscn.acc=0;
    oscn.noise=0;
    oscn.noise_seed=0;

    cairo_new_path(cr);
    for(i=0;i<w;++i)
    {
        fy=y+fh/2.0f-fh*SynthGetSample(&oscn,fNoise,over,duty,wave)*.9f;

        sy=(int32_t)fy;

        if(!i) cairo_move_to(cr,sx,sy);

        cairo_line_to(cr,sx,sy);

        ++sx;

        oscn.acc+=1.0f/w*2.0f;    //show two periods

        while(oscn.acc>=1.0f) oscn.acc-=1.0f;

        ++oscn.noise;
    }
    cairo_stroke(cr);

    cairo_restore(cr);
}

void ChipWaveUI::RenderEnvelope(int32_t x, int32_t y, int32_t w, int32_t h, int32_t env)
{
    const DGL::GraphicsContext &gc = getParentWindow().getGraphicsContext();
    cairo_t *cr = gc.cairo;

    cairo_save(cr);

    int32_t i,sx,sy,stage,skip;
    float fh,fy,level,attack,decay,sustain,release,scale;
    bool makedotted;

    cairo_set_source_color(cr, ColorRGBA{120, 160, 220, 0xff});
    cairo_set_line_width(cr, 2);

    fh=(float)h;
    sx=x;

    scale=25;
    level=0;

    switch(env)
    {
    case 0://envelope
        {
            attack= SynthEnvelopeTimeToDelta(getControlValue(pIdEnvAttack),ENVELOPE_ATTACK_MAX_MS)*scale;
            decay=  SynthEnvelopeTimeToDelta(getControlValue(pIdEnvDecay),ENVELOPE_DECAY_MAX_MS )*scale;
            sustain=getControlValue(pIdEnvSustain);
            release=SynthEnvelopeTimeToDelta(getControlValue(pIdEnvRelease),ENVELOPE_RELEASE_MAX_MS)*scale;
        }
        break;

    case 1://amp
        {
            attack= SynthEnvelopeTimeToDelta(getControlValue(pIdAmpAttack),ENVELOPE_ATTACK_MAX_MS)*scale;
            decay=  SynthEnvelopeTimeToDelta(getControlValue(pIdAmpDecay),ENVELOPE_DECAY_MAX_MS )*scale;
            sustain=getControlValue(pIdAmpSustain);
            release=SynthEnvelopeTimeToDelta(getControlValue(pIdAmpRelease),ENVELOPE_RELEASE_MAX_MS)*scale;
        }
        break;

    default:
        DISTRHO_SAFE_ASSERT_RETURN(false, );
    }

    stage=0;
    skip=0;
    makedotted=false;

    cairo_new_path(cr);
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

        sy=(int32_t)fy;

        if(!i) cairo_move_to(cr,sx,sy);

        //make release dotted
        if (makedotted) {
            cairo_line_to(cr,sx,sy);
            cairo_stroke(cr);
            double dashes[]={2,2};
            cairo_set_dash(cr,dashes,2,0);
            cairo_move_to(cr,sx,sy);
            makedotted=false;
        }

        cairo_line_to(cr,sx,sy);

        if (stage==3&&level==0) {
            ++stage;
            makedotted=true;
        }

        ++sx;
    }
    cairo_stroke(cr);

    cairo_restore(cr);
}

float ChipWaveUI::getControlValue(uint32_t index) const
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, 0);

    DGL::Widget *control = fControls[index].get();

    if (!control)
        return 0;

    return static_cast<ValueFill *>(control)->value();
}

void ChipWaveUI::setControlValue(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, );

    DGL::Widget *control = fControls[index].get();

    if (!control)
        return;

    static_cast<ValueFill *>(control)->setValue(value);
}

///
namespace DISTRHO {

UI *createUI()
{
    return new ChipWaveUI;
}

} // namespace DISTRHO
