#include "CompanyName.hpp"
#include "util/color.hpp"

struct WhiteLight : ModuleLightWidget {
    WhiteLight() {
        addBaseColor(COLOR_WHITE);
    }
};

struct RGBTriangle : ModuleLightWidget {
    RGBTriangle() {
        addBaseColor(COLOR_RED);
        addBaseColor(COLOR_GREEN);
        addBaseColor(COLOR_BLUE);
        addBaseColor(nvgRGBf(1.0, 0, 1.0));
    }
    
    void drawLight(NVGcontext *vg) {
        float r = box.size.x / 0.7;
        float ax,ay, bx,by;
        
        nvgRotate(vg, nvgDegToRad(30));
        //nvgTransform(vg, 1.0, 1.1, 1.0, 1.0, 1.0, 1.0);
        ax = cosf(120.0f/180.0f*NVG_PI) * r;
        ay = sinf(120.0f/180.0f*NVG_PI) * r;
        bx = cosf(-120.0f/180.0f*NVG_PI) * r;
        by = sinf(-120.0f/180.0f*NVG_PI) * r;
        nvgBeginPath(vg);
        nvgMoveTo(vg, r,0);
        nvgLineTo(vg, ax,ay);
        nvgLineTo(vg, bx,by);
        nvgClosePath(vg);
        
        
        // Solid color

        nvgFillColor(vg, color);
        nvgTransRGBAf(color, 1.0);
        nvgFill(vg);
        
        // Border
        nvgStrokeWidth(vg, 0.5);
        nvgStrokeColor(vg, borderColor);
        nvgStroke(vg);
        nvgRotate(vg, (30.0/120.0)*NVG_PI*2);
    }
    
    void drawHalo(NVGcontext *vg) {
        float radius = box.size.x / 0.7;
        float oradius = radius + 15.0;
        
        nvgBeginPath(vg);
        nvgRect(vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);
        
        NVGpaint paint;
        NVGcolor icol = colorMult(color, 0.15);
        NVGcolor ocol = nvgRGB(0, 0, 0);
        paint = nvgRadialGradient(vg, radius, radius, radius, oradius, icol, ocol);
        nvgFillPaint(vg, paint);
        nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
        nvgFill(vg);
    }
};

struct Davies1900hvia : SVGKnob {
    Davies1900hvia() {
        minAngle = -0.83*M_PI;
        maxAngle = 0.83*M_PI;
        sw->svg = SVG::load(assetPlugin(plugin, "res/Davies1900hvia.svg"));
        sw->wrap();
        box.size = sw->box.size;
    }
};

struct SH_Button : SVGSwitch, MomentarySwitch {
    SH_Button() {
        addFrame(SVG::load(assetPlugin(plugin, "res/S+H_button.svg")));
    }
};


struct Trig_Button : SVGSwitch, MomentarySwitch {
    Trig_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/trig_button.svg")));
    }
};


struct Freq_Button : SVGSwitch, MomentarySwitch {
    Freq_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/freq_button.svg")));
    }
};


struct Loop_Button : SVGSwitch, MomentarySwitch {
    Loop_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/loop_button.svg")));
    }
};


struct Up_Button : SVGSwitch, MomentarySwitch {
    Up_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/up_button.svg")));
    }
};


struct Down_Button : SVGSwitch, MomentarySwitch {
    Down_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/down_button.svg")));
    }
};

struct VIA_manual_button : SVGSwitch, MomentarySwitch {
    VIA_manual_button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/manual_trig.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/manual_trig_down.svg")));
    }
};



#define time1Knob ((int) (params[T1_PARAM].value * 4095))
#define time2Knob ((int) (params[T2_PARAM].value * 4095))
#define morphKnob ((int) (params[MORPH_PARAM].value * 4095))
#define BKnob params[B_PARAM].value
#define AKnob params[A_PARAM].value
#define time1CV ((int) (((clamp(inputs[T1_INPUT].value, -5.0, 5.0)/10) + .5) * 4095))
#define time2CV ((int)((((clamp(inputs[T2_INPUT].value, -5.0, 5.0) * params[T2AMT_PARAM].value)/10) + .5) * 4095))
#define morphCV ((int)((((clamp(inputs[MORPH_INPUT].value, -5.0, 5.0) * params[MORPHAMT_PARAM].value)/10) + .5) * 4095))




#define EOA_JACK_HIGH outputs[LOGICB_OUTPUT].value = 5.0;
#define EOA_JACK_LOW outputs[LOGICB_OUTPUT].value = 0;

#define EOA_GATE_HIGH outputs[DELTA_OUTPUT].value = 5.0;
#define EOA_GATE_LOW outputs[DELTA_OUTPUT].value = 0;

#define EOR_JACK_HIGH outputs[LOGICA_OUTPUT].value = 5.0;
#define EOR_JACK_LOW outputs[LOGICA_OUTPUT].value = 0;



#define LEDA_ON lights[LED1_LIGHT].setBrightnessSmooth(1.0);
#define LEDA_OFF lights[LED1_LIGHT].value = 0;

#define LEDB_ON lights[LED3_LIGHT].setBrightnessSmooth(1.0);
#define LEDB_OFF lights[LED3_LIGHT].value = 0;

#define LEDC_ON lights[LED2_LIGHT].setBrightnessSmooth(1.0);
#define LEDC_OFF lights[LED2_LIGHT].value = 0;

#define LEDD_ON lights[LED4_LIGHT].setBrightnessSmooth(1.0);
#define LEDD_OFF lights[LED4_LIGHT].value = 0;



#define BUFF_SIZE 32
#define BUFF_SIZE_MASK (BUFF_SIZE-1)







/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/




#define PHASE_STATE 		flagHolder & 0b00000000000000000000000000000001
#define LAST_PHASE_STATE 	flagHolder & 0b00000000000000000000000000000010
#define GATE_ON 			flagHolder & 0b00000000000000000000000000000100
#define RGB_ON 				flagHolder & 0b00000000000000000000000000001000
#define UPDATE_PRESCALER 	flagHolder & 0b00000000000000000000000000010000
#define DRUM_MODE_ON 		flagHolder & 0b00000000000000000000000000100000
#define DRUM_ATTACK_ON 		flagHolder & 0b00000000000000000000000001000000
#define DRUM_RELEASE_ON 	flagHolder & 0b00000000000000000000000010000000
#define PITCH_ON 			flagHolder & 0b00000000000000000000000100000000
#define MORPH_ON 			flagHolder & 0b00000000000000000000001000000000
#define AMP_ON 				flagHolder & 0b00000000000000000000010000000000
#define DRUM_OFF 			flagHolder & 0b00000000000000000000100000000000
#define LAST_CYCLE 			flagHolder & 0b00000000000000000001000000000000
#define HOLD_AT_B 			flagHolder & 0b00000000000000000010000000000000
#define OSCILLATOR_ACTIVE 	flagHolder & 0b00000000000000000100000000000000
#define TRIGGER_BUTTON	 	flagHolder & 0b00000000000000001000000000000000
#define DETECT_ON	 		flagHolder & 0b00000000000000010000000000000000
#define AUX_MENU	 		flagHolder & 0b00000000000000100000000000000000
#define GATEA		 		flagHolder & 0b00000000000001000000000000000000
#define TRIGA		 		flagHolder & 0b00000000000010000000000000000000
#define DELTAA		 		flagHolder & 0b00000000000100000000000000000000
#define GATEB		 		flagHolder & 0b00000000001000000000000000000000
#define TRIGB		 		flagHolder & 0b00000000010000000000000000000000
#define DELTAB		 		flagHolder & 0b00000000100000000000000000000000


#define SET_PHASE_STATE 		flagHolder |= 0b00000000000000000000000000000001
#define SET_LAST_PHASE_STATE 	flagHolder |= 0b00000000000000000000000000000010
#define SET_GATE_ON 			flagHolder |= 0b00000000000000000000000000000100
#define SET_RGB_ON 				flagHolder |= 0b00000000000000000000000000001000
#define SET_UPDATE_PRESCALER 	flagHolder |= 0b00000000000000000000000000010000
#define SET_DRUM_MODE_ON 		flagHolder |= 0b00000000000000000000000000100000
#define SET_DRUM_ATTACK_ON 		flagHolder |= 0b00000000000000000000000001000000
#define SET_DRUM_RELEASE_ON 	flagHolder |= 0b00000000000000000000000010000000
#define SET_PITCH_ON 			flagHolder |= 0b00000000000000000000000100000000
#define SET_MORPH_ON 			flagHolder |= 0b00000000000000000000001000000000
#define SET_AMP_ON 				flagHolder |= 0b00000000000000000000010000000000
#define SET_DRUM_OFF 			flagHolder |= 0b00000000000000000000100000000000
#define SET_LAST_CYCLE 			flagHolder |= 0b00000000000000000001000000000000
#define SET_HOLD_AT_B 			flagHolder |= 0b00000000000000000010000000000000
#define SET_OSCILLATOR_ACTIVE 	flagHolder |= 0b00000000000000000100000000000000
#define SET_TRIGGER_BUTTON	 	flagHolder |= 0b00000000000000001000000000000000
#define SET_DETECT_ON	 		flagHolder |= 0b00000000000000010000000000000000
#define SET_AUX_MENU			flagHolder |= 0b00000000000000100000000000000000
#define SET_GATEA		 		flagHolder |= 0b00000000000001000000000000000000
#define SET_TRIGA		 		flagHolder |= 0b00000000000010000000000000000000
#define SET_DELTAA		 		flagHolder |= 0b00000000000100000000000000000000
#define SET_GATEB		 		flagHolder |= 0b00000000001000000000000000000000
#define SET_TRIGB		 		flagHolder |= 0b00000000010000000000000000000000
#define SET_DELTAB		 		flagHolder |= 0b00000000100000000000000000000000




#define RESET_PHASE_STATE 		flagHolder &= 0b11111111111111111111111111111110
#define RESET_LAST_PHASE_STATE 	flagHolder &= 0b11111111111111111111111111111101
#define RESET_GATE_ON 			flagHolder &= 0b11111111111111111111111111111011
#define RESET_RGB_ON 			flagHolder &= 0b11111111111111111111111111110111
#define RESET_UPDATE_PRESCALER 	flagHolder &= 0b11111111111111111111111111101111
#define RESET_DRUM_MODE_ON 		flagHolder &= 0b11111111111111111111111111011111
#define RESET_DRUM_ATTACK_ON 	flagHolder &= 0b11111111111111111111111110111111
#define RESET_DRUM_RELEASE_ON 	flagHolder &= 0b11111111111111111111111101111111
#define RESET_PITCH_ON 			flagHolder &= 0b11111111111111111111111011111111
#define RESET_MORPH_ON 			flagHolder &= 0b11111111111111111111110111111111
#define RESET_AMP_ON 			flagHolder &= 0b11111111111111111111101111111111
#define RESET_DRUM_OFF 			flagHolder &= 0b11111111111111111111011111111111
#define RESET_LAST_CYCLE 		flagHolder &= 0b11111111111111111110111111111111
#define RESET_HOLD_AT_B 		flagHolder &= 0b11111111111111111101111111111111
#define RESET_OSCILLATOR_ACTIVE flagHolder &= 0b11111111111111111011111111111111
#define RESET_TRIGGER_BUTTON	flagHolder &= 0b11111111111111110111111111111111
#define RESET_DETECT_ON			flagHolder &= 0b11111111111111101111111111111111
#define RESET_AUX_MENU			flagHolder &= 0b11111111111111011111111111111111
#define RESET_GATEA				flagHolder &= 0b11111111111110111111111111111111
#define RESET_TRIGA				flagHolder &= 0b11111111111101111111111111111111
#define RESET_DELTAA			flagHolder &= 0b11111111111011111111111111111111
#define RESET_GATEB				flagHolder &= 0b11111111110111111111111111111111
#define RESET_TRIGB				flagHolder &= 0b11111111101111111111111111111111
#define RESET_DELTAB			flagHolder &= 0b11111111011111111111111111111111



#define TOGGLE_GATE_ON 			flagHolder ^= 0b00000000000000000000000000000100

#define REMEMBER_PHASE_STATE	flagHolder ^= (-((PHASE_STATE) ^ flagHolder) & (1UL << 1))

