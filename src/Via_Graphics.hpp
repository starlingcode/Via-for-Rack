#include "Starling.hpp"
#include "util/color.hpp"
#include "dsp/digital.hpp"
#include "Via_Macros.hpp"

// Modified light widget for the white LED

struct WhiteLight : ModuleLightWidget {
    WhiteLight() {
        addBaseColor(COLOR_WHITE);
    }
};

// Adapted light object for the glowing triangle

struct RGBTriangle : ModuleLightWidget {
    RGBTriangle() {
        addBaseColor(nvgRGBAf(1.0, 0.0, 0.0, 1.0));
        addBaseColor(nvgRGBAf(0.0, 1.0, 0.0, 1.0));
        addBaseColor(nvgRGBAf(0.0, 0.0, 1.0, 1.0));
    }
    
    void drawLight(NVGcontext *vg) {
        
        nvgBeginPath(vg);
        nvgMoveTo(vg, .4,-22.3);
        nvgLineTo(vg, -17.1,11.7);
        nvgLineTo(vg, 17.1,11.7);
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
        float radius = 14;
        float oradius = radius + 13;
        
        nvgBeginPath(vg);
        nvgRect(vg, -25, -25, 50, 50);
        
        NVGpaint paint;
        NVGcolor icol = colorMult(color, 0.10);
        NVGcolor ocol = nvgRGB(0, 0, 0);
        paint = nvgRadialGradient(vg, 0, 0, radius, oradius, icol, ocol);
        nvgFillPaint(vg, paint);
        nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
        nvgFill(vg);
    }
};

// Davies knob for contrast against the black background, adapted from Rack component library
// Thanks Grayscale http://grayscale.info/ !

struct Davies1900hvia : Davies1900hKnob {
    Davies1900hvia() {
        setSVG(SVG::load(assetPlugin(plugin, "res/Davies1900hvia.svg")));
    }
};

struct ViaSifam : Davies1900hKnob {
    ViaSifam() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knob_sifam_blkcap.svg")));
    }
};

struct ViaJack : PJ301MPort {
    ViaJack() {
        setSVG(SVG::load(assetPlugin(plugin, "res/jack-nogradients.svg")));
    }
};

// Button skins for the manual trigger and touch sensors

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
