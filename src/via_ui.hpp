#include "starling.hpp"
#include "color.hpp"
#include <app/ParamWidget.hpp>
#include <iomanip> // setprecision
#include <sstream> // stringstream

// Modified light widget for the white LED

// struct WhiteLight : ModuleLightWidget {
//     WhiteLight() {
//         addBaseColor(SCHEME_WHITE);
//     }
// };

// Adapted light object for the glowing triangle

struct RGBTriangle : ModuleLightWidget {
    RGBTriangle() {
        addBaseColor(nvgRGBAf(1.0, 0.0, 0.0, 1.0));
        addBaseColor(nvgRGBAf(0.0, 1.0, 0.0, 1.0));
        addBaseColor(nvgRGBAf(0.0, 0.0, 1.0, 1.0));
    }
    
    void drawBackground(const DrawArgs &args) override {}

    void drawLight(const DrawArgs &args) override {
        
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, .5,-17.8);
        nvgLineTo(args.vg, -12,9.6);
        nvgLineTo(args.vg, 12.7,9.6);
        nvgClosePath(args.vg);
        
        // Solid color
        
        nvgFillColor(args.vg, color);
        //nvgTransRGBAf(color, 1.0);
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

struct ButtonLight : YellowLight {
    ButtonLight() {
        box.size = Vec(28 - 6, 28 - 6);
        bgColor = color::BLACK_TRANSPARENT;
    }
};

struct NKK_2 : app::SvgSwitch {
	NKK_2() {
		addFrame(Svg::load(asset::system("res/ComponentLibrary/NKK_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/NKK_2.svg")));
	}
};


// Custom Quantities

template<int NUM_MODES> 
struct ViaButtonQuantity : ParamQuantity {

    std::string modes[NUM_MODES];

    virtual int getModeEnumeration(void) {return 0;}
    virtual void setMode(int mode) {}

    float getDisplayValue() override {
        if (!module)
            return Quantity::getDisplayValue();

        return getModeEnumeration();
    }

    std::string getDisplayValueString() override {
        if (!module)
            return Quantity::getDisplayValueString();

        int mode = getDisplayValue();

        return modes[mode] + " (" + std::to_string(mode + 1) + ")";
    }

    void setDisplayValueString(std::string s) override {
        if (!module)
            return;

        for (int i = 0; i < NUM_MODES; i++) {
            if (s == modes[i] || s == std::to_string(i + 1)) {
                setMode(i);
            } 
        }

    }

};

struct ViaComplexButtonQuantity : ParamQuantity {

    std::string * modes;
    int numModes = 0;

    virtual int getModeEnumeration(void) {return 0;}
    virtual void getModeArray(void) {}
    virtual void setMode(int mode) {}

    float getDisplayValue() override {
        if (!module)
            return Quantity::getDisplayValue();

        return getModeEnumeration();
    }

    std::string getDisplayValueString() override {
        if (!module)
            return Quantity::getDisplayValueString();

        int mode = getDisplayValue();

        getModeArray();

        return modes[mode] + " (" + std::to_string(mode + 1) + ")";
    }

    void setDisplayValueString(std::string s) override {
        if (!module)
            return;

        for (int i = 0; i < numModes; i++) {
            if (s == modes[i] || s == std::to_string(i + 1)) {
                setMode(i);
            } 
        }

    }

};

struct ViaKnobQuantity : ParamQuantity {

    std::string stripScientificNotation(std::string s) {
        return s.substr(0, s.size() - 4);
    }

    virtual float translateParameter(float value) {return 0;}
    virtual float translateInput(float userInput) {return 0;}
    virtual void setLabel(void) {};

    float getDisplayValue() override {
        if (!module)
            return Quantity::getDisplayValue();

        setLabel();

        return translateParameter(getSmoothValue());
    }

    int getDisplayPrecision() override {
        return 3;
    }

    void setDisplayValue(float v) override {
        if (!module)
            return;

        setValue(translateInput(v));

    }

    std::string getDisplayValueString(void) override {

        std::string displayValueRaw = string::f("%.*g", getDisplayPrecision(), math::normalizeZero(getDisplayValue()));

        if (displayValueRaw.size() > 4) {

            if (string::endsWith(displayValueRaw, "e+03")) {
                return stripScientificNotation(displayValueRaw) + "k";
            } else if (string::startsWith(displayValueRaw.c_str(), "0.")) {
                return string::f("%.*g", getDisplayPrecision(), std::stof(displayValueRaw) * 1000.0) + "m";
            } else {
                return displayValueRaw;
            }

        } else if (displayValueRaw.size() > 2) {
            if (string::startsWith(displayValueRaw, "0.")) {
                return string::f("%.*g", getDisplayPrecision(), std::stof(displayValueRaw) * 1000.0) + "m";
            } else {
              return displayValueRaw;
            }
        } else {
            return displayValueRaw;
        }

    }

};
