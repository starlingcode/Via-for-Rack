#include "starling.hpp"
#include "color.hpp"

// Modified light widget for the white LED

struct WhiteLight : ModuleLightWidget {
    WhiteLight() {
        addBaseColor(SCHEME_WHITE);
    }
};

// Adapted light object for the glowing triangle

struct RGBTriangle : ModuleLightWidget {
    RGBTriangle() {
        addBaseColor(nvgRGBAf(1.0, 0.0, 0.0, 1.0));
        addBaseColor(nvgRGBAf(0.0, 1.0, 0.0, 1.0));
        addBaseColor(nvgRGBAf(0.0, 0.0, 1.0, 1.0));
    }
    
    void drawLight(const DrawArgs &args) override {
        
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, .5,-17.8);
        nvgLineTo(args.vg, -12,9.6);
        nvgLineTo(args.vg, 12.7,9.6);
        nvgClosePath(args.vg);
        
        
        
        // Solid color
        
        nvgFillColor(args.vg, color);
        nvgTransRGBAf(color, 1.0);
        nvgFill(args.vg);
        
        // Border
        nvgStrokeWidth(args.vg, 0.5);
        nvgStrokeColor(args.vg, borderColor);
        nvgStroke(args.vg);
        nvgRotate(args.vg, (30.0/120.0)*NVG_PI*2);
    }
    
    void drawHalo(const DrawArgs &args) override {
        float radius = 14;
        float oradius = radius + 13;
        
        nvgBeginPath(args.vg);
        nvgRect(args.vg, -25, -25, 50, 50);
        
        NVGpaint paint;
        NVGcolor icol = color::mult(color, 0.10);
        NVGcolor ocol = nvgRGB(0, 0, 0);
        paint = nvgRadialGradient(args.vg, 0, 0, radius, oradius, icol, ocol);
        nvgFillPaint(args.vg, paint);
        nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
        nvgFill(args.vg);
    }
};


struct ViaSifamBlack : RoundKnob {
    ViaSifamBlack() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_sifam_blkcap.svg")));
    }
};

struct ViaSifamGrey : RoundKnob {
    ViaSifamGrey() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_sifam_grycap.svg")));
    }
};

struct ViaJack : SvgPort {
    ViaJack() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/jack-nogradients.svg")));
    }
};

// Button skins for the manual trigger and touch sensors

struct TransparentButton : SvgSwitch {
    TransparentButton() {
        momentary = true;
        shadow->opacity = 0.0;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/transparent_button.svg")));
    }
};

struct ViaPushButton : SvgSwitch {
    ViaPushButton() {
        momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/manual_trig.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/manual_trig_down.svg")));
    }
};
