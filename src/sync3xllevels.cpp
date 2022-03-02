#include "starling.hpp"
#include "via-params.hpp"
#include "sync3xlexpand.hpp"


struct Sync3XLLevels : Module {
    enum ParamId {
        LVL1KNOB_PARAM,
        LVL2KNOB_PARAM,
        LVL3KNOB_PARAM,
        LVLMIXKNOB_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        LVL1CV_INPUT,
        LVL2CV_INPUT,
        LVL3CV_INPUT,
        LVLMIXCV_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        OUTPUTS_LEN
    };
    enum LightId {
        LVL1LED_LIGHT,
        LVL2LED_LIGHT,
        LVL3LED_LIGHT,
        LVLMIXLED_LIGHT,
        LVL1LED__LIGHT,
        LVL2LED__LIGHT,
        LVL3LED__LIGHT,
        LVLMIXLED__LIGHT,
        LIGHTS_LEN
    };

    Sync3XLLevels() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(LVL1KNOB_PARAM, 0.f, 1.f, 0.f, "Out 1 level");
        configParam(LVL2KNOB_PARAM, 0.f, 1.f, 0.f, "Out 2 level");
        configParam(LVL3KNOB_PARAM, 0.f, 1.f, 0.f, "Out 3 level");
        configParam(LVLMIXKNOB_PARAM, 0.f, 1.f, 0.f, "Mix level");
        configInput(LVL1CV_INPUT, "Out 1 level");
        configInput(LVL2CV_INPUT, "Out 2 level");
        configInput(LVL3CV_INPUT, "Out 3 level");
        configInput(LVLMIXCV_INPUT, "Mix level");

        leftExpander.producerMessage = new Sync3XLExpand; 
        leftExpander.consumerMessage = new Sync3XLExpand; 
    }

    ~Sync3XLLevels() {
        free(leftExpander.producerMessage);
        free(leftExpander.consumerMessage);
    }   

    bool expanderAttached = false;

    void process(const ProcessArgs& args) override {

        float voltage1 = inputs[LVL1CV_INPUT].getVoltage() + params[LVL1KNOB_PARAM].getValue() * 5.f;
        voltage1 = clamp(voltage1, 0.f, 10.f);
        voltage1 *= 0.2f;
        float voltage2 = inputs[LVL2CV_INPUT].getVoltage() + params[LVL2KNOB_PARAM].getValue() * 5.f;
        voltage2 = clamp(voltage2, 0.f, 10.f);
        voltage2 *= 0.2f;
        float voltage3 = inputs[LVL3CV_INPUT].getVoltage() + params[LVL3KNOB_PARAM].getValue() * 5.f;
        voltage3 = clamp(voltage3, 0.f, 10.f);
        voltage3 *= 0.2f;
        float voltageMix = inputs[LVLMIXCV_INPUT].getVoltage() + params[LVLMIXKNOB_PARAM].getValue() * 5.f;
        voltageMix = clamp(voltageMix, 0.f, 10.f);
        voltageMix *= 0.2f;

        if (expanderAttached && leftExpander.module) {

            Sync3XLExpand* from_host = (Sync3XLExpand*) leftExpander.consumerMessage;
            Sync3XLExpand* to_host = (Sync3XLExpand*) leftExpander.module->rightExpander.producerMessage;

            voltage1 *= from_host->out1;
            voltage2 *= from_host->out2;
            voltage3 *= from_host->out3;
            voltageMix *= from_host->mix;
            to_host->out1 = voltage1;
            to_host->out2 = voltage2;
            to_host->out3 = voltage3;
            to_host->mix = voltageMix;
            leftExpander.module->rightExpander.messageFlipRequested = true;
        }; 

        lights[LVL1LED_LIGHT].setSmoothBrightness(clamp(voltage1, 0.f, 5.f)/5.f, args.sampleTime);
        lights[LVL2LED_LIGHT].setSmoothBrightness(clamp(voltage2, 0.f, 5.f)/5.f, args.sampleTime);
        lights[LVL3LED_LIGHT].setSmoothBrightness(clamp(voltage3, 0.f, 5.f)/5.f, args.sampleTime);
        lights[LVLMIXLED_LIGHT].setSmoothBrightness(clamp(voltageMix, 0.f, 5.f)/5.f, args.sampleTime);
        lights[LVL1LED__LIGHT].setSmoothBrightness(-clamp(voltage1, -5.f, 0.f)/5.f, args.sampleTime);
        lights[LVL2LED__LIGHT].setSmoothBrightness(-clamp(voltage2, -5.f, 0.f)/5.f, args.sampleTime);
        lights[LVL3LED__LIGHT].setSmoothBrightness(-clamp(voltage3, -5.f, 0.f)/5.f, args.sampleTime);
        lights[LVLMIXLED__LIGHT].setSmoothBrightness(-clamp(voltageMix, -5.f, 0.f)/5.f, args.sampleTime);
    }
    
    void onExpanderChange(const ExpanderChangeEvent& e) override {
        if (leftExpander.module && (leftExpander.module->model == modelSync3XL)) {
            expanderAttached = true;
        } else {
            expanderAttached = false;
        }
    }
};


struct Sync3XLLevelsWidget : ModuleWidget {
    Sync3XLLevelsWidget(Sync3XLLevels* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/sync3xllevels.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamGrey>(mm2px(Vec(30.513, 19.562)), module, Sync3XLLevels::LVL1KNOB_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(30.513, 47.062)), module, Sync3XLLevels::LVL2KNOB_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(30.513, 74.562)), module, Sync3XLLevels::LVL3KNOB_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(30.513, 102.062)), module, Sync3XLLevels::LVLMIXKNOB_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(16.762, 26.438)), module, Sync3XLLevels::LVL1CV_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(16.762, 53.938)), module, Sync3XLLevels::LVL2CV_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(16.762, 81.438)), module, Sync3XLLevels::LVL3CV_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(16.762, 108.938)), module, Sync3XLLevels::LVLMIXCV_INPUT));

        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.45, 26.438)), module, Sync3XLLevels::LVL1LED_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(6.45, 26.438)), module, Sync3XLLevels::LVL1LED__LIGHT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.45, 53.938)), module, Sync3XLLevels::LVL2LED_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(6.45, 53.938)), module, Sync3XLLevels::LVL2LED__LIGHT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.45, 81.438)), module, Sync3XLLevels::LVL3LED_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(6.45, 81.438)), module, Sync3XLLevels::LVL3LED__LIGHT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(6.45, 108.938)), module, Sync3XLLevels::LVLMIXLED_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(6.45, 108.938)), module, Sync3XLLevels::LVLMIXLED__LIGHT));
    }
};


Model* modelSync3XLLevels = createModel<Sync3XLLevels, Sync3XLLevelsWidget>("SYNC3XLLEVELS");
