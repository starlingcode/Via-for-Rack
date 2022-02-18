#include "sync3.hpp"
#include "via_module.hpp"
#include "polyblamp.hpp"
#include "osdialog.h"

#define SYNC3_OVERSAMPLE_AMOUNT 24
#define SYNC3_OVERSAMPLE_QUALITY 6


struct Sync3 : Via<SYNC3_OVERSAMPLE_AMOUNT, SYNC3_OVERSAMPLE_AMOUNT> {

    struct IRatioQuantity;
    struct IIRatioQuantity;
    struct IIIRatioQuantity;
    struct IButtonQuantity;
    struct RatioButtonQuantity;
    struct IIButtonQuantity;
    struct CVButtonQuantity;
    struct IIIButtonQuantity;
    
    Sync3() : Via(), virtualModule(asset::plugin(pluginInstance, "res/sync3scales.bin")) {

        virtualIO = &virtualModule;

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam<IRatioQuantity>(KNOB1_PARAM, 0, 4095.0, 2048.0, "I Ratio");
        configParam<IIRatioQuantity>(KNOB2_PARAM, 0, 4095.0, 2048.0, "II Ratio");
        configParam<IIIRatioQuantity>(KNOB3_PARAM, 0, 4095.0, 2048.0, "III Ratio");
        configParam<BScaleQuantity>(B_PARAM, -1.0, 1.0, -1.0, "Oscillator III Level", "V");
        configParam<CV2ScaleQuantity>(CV2AMT_PARAM, 0, 1.0, 1.0, "II CV Scale");
        configParam<ANormalQuantity>(A_PARAM, -5.0, 5.0, 5.0, "Oscillator II Level", "V");
        configParam<CV3ScaleQuantity>(CV3AMT_PARAM, 0, 1.0, 1.0, "III CV Scale");
        
        configParam<IButtonQuantity>(BUTTON1_PARAM, 0.0, 1.0, 0.0, "Oscillator I Shape");
        configParam<RatioButtonQuantity>(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Ratio Set");
        configParam<IIButtonQuantity>(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Oscillator II Shape");
        configParam<CVButtonQuantity>(BUTTON4_PARAM, 0.0, 1.0, 0.0, "CV Mapping");
        configParam<RatioButtonQuantity>(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Ratio Set");
        configParam<IIIButtonQuantity>(BUTTON6_PARAM, 0.0, 1.0, 0.0, "Oscillator III Shape");
        
        configParam(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Tap Tempo");

        onSampleRateChange();

    }
    void process(const ProcessArgs &args) override;

    ViaSync3 virtualModule;

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();

        ledDecay = 1.0/sampleRate;

        if (sampleRate == 44100.0) {
            divideAmount = 1;
        } else if (sampleRate == 48000.0) {
            divideAmount = 1;
        } else if (sampleRate == 88200.0) {
            divideAmount = 2;
        } else if (sampleRate == 96000.0) {
            divideAmount = 2;
        } else if (sampleRate == 176400.0) {
            divideAmount = 4;
        } else if (sampleRate == 192000.0) {
            divideAmount = 4;
        } else if (sampleRate == 352800.0) {
            divideAmount = 8;
        } else if (sampleRate == 384000.0) {
            divideAmount = 8;
        } else if (sampleRate == 705600.0) {
            divideAmount = 16;
        } else if (sampleRate == 768000.0) {
            divideAmount = 16;
        }
        
    }

    json_t *dataToJson() override {

        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "osc_modes", json_integer(virtualModule.sync3UI.modeStateBuffer));
        json_object_set_new(rootJ, "scale_file", json_string(scalePath.c_str()));
        
        return rootJ;
    }
    
    void dataFromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "osc_modes");
        virtualModule.sync3UI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.sync3UI.loadFromEEPROM(0);
        virtualModule.sync3UI.recallModuleState();
        virtualModule.sync3UI.defaultEnterMenuCallback();

        json_t *pathJ = json_object_get(rootJ, "scale_file");
        scalePath = json_string_value(pathJ);
        virtualModule.readScalesFromFile(scalePath);

    }

    std::string scalePath = asset::plugin(pluginInstance, "res/sync3scales.bin");

    int32_t optimize = 0;

    float lastDac1Phase = 0;
    float lastDac2Phase = 0;
    float lastDac3Phase = 0;

    dsp::MinBlepGenerator<8, 8, float> dac1MinBlep;
    dsp::MinBlepGenerator<8, 8, float> dac2MinBlep;
    dsp::MinBlepGenerator<8, 8, float> dac3MinBlep;

    PolyBlampGenerator<float> dac1PolyBlamp;
    PolyBlampGenerator<float> dac2PolyBlamp;
    PolyBlampGenerator<float> dac3PolyBlamp;

    float blampDelay1[3];
    float blampDelay2[3];
    float blampDelay3[3];

    void updateAudioRateEconomy(void) {

        acquireCVs();

        processLogicInputs();

        float dac1Sample = (float) virtualIO->outputs.dac1Samples[23];
        float dac2Sample = (float) virtualIO->outputs.dac2Samples[23];
        float dac3Sample = (float) virtualIO->outputs.dac3Samples[23];

        int32_t inc1 = (virtualModule.increment3 + virtualModule.phaseModIncrement2) * 24;
        int32_t inc2 = (virtualModule.increment4 + virtualModule.phaseModIncrement2) * 24;
        int32_t inc3 = virtualModule.increment2 * 24;


        if (virtualModule.sync3UI.button1Mode == 0) {

            int32_t crossingDirection = crossed0(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac3Phase) / (float) inc3;
                dac3MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            dac3Sample += dac3MinBlep.process();

        } else if (virtualModule.sync3UI.button1Mode == 1) {

            int32_t crossingDirection = crossed0(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac3Phase) / (float) inc3;
                dac3MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed2(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac3Phase) / (float) inc3;
                dac3MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac3Sample += dac3MinBlep.process();

        } else if (virtualModule.sync3UI.button1Mode == 2) {

            int32_t crossingDirection = crossed0(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - lastDac3Phase) / (float) inc3;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(inc3) / 4294967296.f));
            }

            crossingDirection = crossed2(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac3Phase) / (float) inc3;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(inc3) / 4294967296.f));
            }

            float dac3Output = blampDelay3[0];
            blampDelay3[0] = blampDelay3[1];
            blampDelay3[1] = dac3Sample;
            dac3Sample = dac3Output + dac3PolyBlamp.process(); 

        }


        if (virtualModule.sync3UI.button3Mode == 0) {

            int32_t crossingDirection = crossed0(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac1Phase) / (float) inc1;
                dac1MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();

        } else if (virtualModule.sync3UI.button3Mode == 1) {

            int32_t crossingDirection = crossed0(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac1Phase) / (float) inc1;
                dac1MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed2(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac1Phase) / (float) inc1;
                dac1MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();

        } else if (virtualModule.sync3UI.button3Mode == 2) {

            int32_t crossingDirection = crossed0(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - lastDac1Phase) / (float) inc1;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(inc1) / 4294967296.f));
            }

            crossingDirection = crossed2(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac1Phase) / (float) inc1;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(inc1) / 4294967296.f));
            }

            float dac1Output = blampDelay1[0];
            blampDelay1[0] = blampDelay1[1];
            blampDelay1[1] = dac1Sample;
            dac1Sample = dac1Output + dac1PolyBlamp.process(); 

        }


        if (virtualModule.sync3UI.button6Mode == 0) {

            int32_t crossingDirection = crossed0(lastDac2Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac2Phase) / (float) inc2;
                dac2MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();

        } else if (virtualModule.sync3UI.button6Mode == 1) {

            int32_t crossingDirection = crossed0(lastDac2Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac2Phase) / (float) inc2;
                dac2MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed2(lastDac1Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac2Phase) / (float) inc2;
                dac2MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();

        } else if (virtualModule.sync3UI.button6Mode == 2) {

            int32_t crossingDirection = crossed0(lastDac2Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - lastDac2Phase) / (float) inc2;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(inc2) / 4294967296.f));
            }

            crossingDirection = crossed2(lastDac2Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac2Phase) / (float) inc2;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(inc2) / 4294967296.f));
            }

            float dac2Output = blampDelay2[0];
            blampDelay2[0] = blampDelay2[1];
            blampDelay2[1] = dac2Sample;
            dac2Sample = dac2Output + dac2PolyBlamp.process(); 

        }

        lastDac1Phase = virtualModule.phase3;
        lastDac2Phase = virtualModule.phase4;
        lastDac3Phase = virtualModule.phase2;

        
        virtualIO->halfTransferCallback();

        // "model" the circuit
        // A and B inputs with normalled reference voltages
        float aIn = inputs[A_INPUT].isConnected() ? inputs[A_INPUT].getVoltage() : params[A_PARAM].getValue();
        float bIn = inputs[B_INPUT].isConnected() ? inputs[B_INPUT].getVoltage() : 5.0;
        bIn *= params[B_PARAM].getValue();
        
        // sample and holds
        // get a new sample on the rising edge at the sh control output
        if (virtualIO->shAState > shALast) {
            aSample = aIn;
        }
        if (virtualIO->shBState > shBLast) {
            bSample = bIn;
        }

        shALast = virtualIO->shAState;
        shBLast = virtualIO->shBState;

        // either use the sample or track depending on the sh control output
        aIn = virtualIO->shAState ? aSample : aIn;
        bIn = virtualIO->shBState ? bSample : bIn;

        // VCA/mixing stage
        // normalize 12 bits to 0-1
        outputs[MAIN_OUTPUT].setVoltage(bIn*(dac2Sample/4095.0) + aIn*(dac1Sample/4095.0)); 
        outputs[AUX_DAC_OUTPUT].setVoltage((dac3Sample/4095.0 - .5) * -10.666666666);
        outputs[LOGICA_OUTPUT].setVoltage(virtualIO->logicAState * 5.0);
        outputs[AUX_LOGIC_OUTPUT].setVoltage(virtualIO->auxLogicState * 5.0);

        updateLEDs();

        clockDivider = 0;

    }
    
};

void Sync3::process(const ProcessArgs &args) {

    clockDivider++;

    if (clockDivider >= divideAmount) {

        // update the "slow IO" (not audio rate) every 16 samples
        slowIOPrescaler++;
        if (slowIOPrescaler == 16) {
            slowIOPrescaler = 0;
            updateSlowIO();
            virtualModule.slowConversionCallback();
            virtualModule.ui_dispatch(SENSOR_EVENT_SIG);
            virtualModule.sync3UI.incrementTimer();
            processTriggerButton();
        }

        if (!optimize) {
            updateAudioRateEconomy();
        } else {
            updateAudioRate();
        }
        virtualModule.advanceMeasurementTimer();
        virtualModule.advanceTimer1();
        virtualModule.advanceTimer2();

    }
    
}

struct Sync3Widget : ModuleWidget  {

    Sync3Widget(Sync3 *module) {

	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sync3.svg")));

        setModule(module);

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Sync3::KNOB1_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Sync3::KNOB2_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Sync3::KNOB3_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Sync3::B_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Sync3::CV2AMT_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Sync3::A_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Sync3::CV3AMT_PARAM));
        
        addParam(createParam<TransparentButton>(Vec(10.5 + .753, 83), module, Sync3::BUTTON1_PARAM));
        addParam(createParam<TransparentButton>(Vec(47 + .753, 90), module, Sync3::BUTTON2_PARAM));
        addParam(createParam<TransparentButton>(Vec(83 + .753, 83), module, Sync3::BUTTON3_PARAM));
        addParam(createParam<TransparentButton>(Vec(10.5 + .753, 133), module, Sync3::BUTTON4_PARAM));
        addParam(createParam<TransparentButton>(Vec(47 + .753, 129.5), module, Sync3::BUTTON5_PARAM));
        addParam(createParam<TransparentButton>(Vec(83 + .753, 133), module, Sync3::BUTTON6_PARAM));
        
        addParam(createParam<ViaPushButton>(Vec(132.7 + .753, 320), module, Sync3::TRIGBUTTON_PARAM));

        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 241.12), module, Sync3::A_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 282.62), module, Sync3::B_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 324.02), module, Sync3::MAIN_LOGIC_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 241.12), module, Sync3::CV1_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 282.62), module, Sync3::CV2_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 324.02), module, Sync3::CV3_INPUT));
        addInput(createInput<ViaJack>(Vec(135 + 1.053, 282.62), module, Sync3::AUX_LOGIC_INPUT));

        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 241.12), module, Sync3::LOGICA_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 282.62), module, Sync3::AUX_DAC_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 324.02), module, Sync3::MAIN_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(135 + 1.053, 241.12), module, Sync3::AUX_LOGIC_OUTPUT));

        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.9 + .753, 268.5), module, Sync3::LED1_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.8 + .753, 268.5), module, Sync3::LED2_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.9 + .753, 309.8), module, Sync3::LED3_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.8 + .753, 309.8), module, Sync3::LED4_LIGHT));
        addChild(createLight<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Sync3::OUTPUT_GREEN_LIGHT));
        addChild(createLight<LargeSimpleLight<RGBTriangle>>(Vec(59 + .753, 221), module, Sync3::RED_LIGHT));

    };

    void appendContextMenu(Menu *menu) override {
        Sync3 *module = dynamic_cast<Sync3*>(this->module);

        struct PresetRecallItem : MenuItem {
            Sync3 *module;
            int preset;
            void onAction(const event::Action &e) override {
                module->virtualModule.sync3UI.modeStateBuffer = preset;
                module->virtualModule.sync3UI.loadFromEEPROM(0);
                module->virtualModule.sync3UI.recallModuleState();
            }
        };

        struct OptimizationHandler : MenuItem {
            Sync3 *module;
            int32_t optimization;
            void onAction(const event::Action &e) override {
                module->optimize = optimization;
            }
        };

        struct ScaleSetHandler : MenuItem {
            Sync3 *module; 
            void onAction(const event::Action &e) override {
             
                char* pathC = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL); 
                if (!pathC) { 
                    // Fail silently 
                    return; 
                } 
                DEFER({ 
                    std::free(pathC); 
                }); 
             
                module->virtualModule.readScalesFromFile(pathC);
                module->scalePath = pathC;
            }
        };
        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("CPU Mode"));
        const std::string optimization[] = {
            "Optimized",
            "Direct Port",
        };
        for (int i = 0; i < (int) LENGTHOF(optimization); i++) {
            OptimizationHandler *menuItem = createMenuItem<OptimizationHandler>(optimization[i], CHECKMARK(module->optimize == i));
            menuItem->module = module;
            menuItem->optimization = i;
            menu->addChild(menuItem);
        }
        ScaleSetHandler *menuItem = createMenuItem<ScaleSetHandler>("Select Scale Set File");
        menuItem->module = module;
        menu->addChild(menuItem);

    }
    
};

Model *modelSync3 = createModel<Sync3, Sync3Widget>("SYNC3");

// Tooltip definitions

struct Sync3::IRatioQuantity : ViaKnobQuantity {

    std::string getDisplayValueString(void) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        return string::f("%d", sync3Module->virtualModule.numerator1Alt) + "/" + 
                    string::f("%d", sync3Module->virtualModule.denominator1Select);

    }

};

struct Sync3::IIRatioQuantity : ViaKnobQuantity {

    std::string getDisplayValueString(void) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        return string::f("%i", sync3Module->virtualModule.numerator2Alt) + "/" + 
                    string::f("%i", sync3Module->virtualModule.denominator2Select);
    }

};

struct Sync3::IIIRatioQuantity : ViaKnobQuantity {

    std::string getDisplayValueString(void) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        return string::f("%i", sync3Module->virtualModule.numerator3Alt) + "/" + 
                    string::f("%i", sync3Module->virtualModule.denominator3Select);
    }

};

struct Sync3::IButtonQuantity : ViaButtonQuantity<3> {

    std::string buttonModes[3] = {"Saw", "Square", "Triangle"};

    IButtonQuantity() {
        for (int i = 0; i < 3; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        return sync3Module->virtualModule.sync3UI.button1Mode;

    }

    void setMode(int mode) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        sync3Module->virtualModule.sync3UI.button1Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button1Mode, BUTTON1_MASK, BUTTON1_SHIFT);
        sync3Module->virtualModule.handleButton1ModeChange(mode);

    }

};

struct Sync3::RatioButtonQuantity : ViaButtonQuantity<8> {

    std::string buttonModes[8] = {"Rhythms", "Integers", "Open Intervals", "Circle of Fifths", "Major Arp", "Minor Arp", "Microtonal", "Bohlen-Pierce (Spooky)"};

    RatioButtonQuantity() {
        for (int i = 0; i < 8; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        return sync3Module->virtualModule.sync3UI.button2Mode;

    }

    void setMode(int mode) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        sync3Module->virtualModule.sync3UI.button2Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button2Mode, BUTTON2_MASK, BUTTON2_SHIFT);
        sync3Module->virtualModule.handleButton2ModeChange(mode);

    }

};

struct Sync3::IIButtonQuantity : ViaButtonQuantity<3> {

    std::string buttonModes[3] = {"Saw", "Square", "Triangle"};

    IIButtonQuantity() {
        for (int i = 0; i < 3; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        return sync3Module->virtualModule.sync3UI.button3Mode;

    }

    void setMode(int mode) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        sync3Module->virtualModule.sync3UI.button3Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button3Mode, BUTTON3_MASK, BUTTON3_SHIFT);
        sync3Module->virtualModule.handleButton3ModeChange(mode);

    }

};

struct Sync3::CVButtonQuantity : ViaButtonQuantity<2> {

    std::string buttonModes[2] = {"Independent Ratios", "II + III and Phase Modulation"};

    CVButtonQuantity() {
        for (int i = 0; i < 2; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        return sync3Module->virtualModule.sync3UI.button4Mode;

    }

    void setMode(int mode) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        sync3Module->virtualModule.sync3UI.button4Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button4Mode, BUTTON4_MASK, BUTTON4_SHIFT);
        sync3Module->virtualModule.handleButton4ModeChange(mode);

    }

};

struct Sync3::IIIButtonQuantity : ViaButtonQuantity<3> {

    std::string buttonModes[3] = {"Saw", "Square", "Triangle"};

    IIIButtonQuantity() {
        for (int i = 0; i < 3; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        return sync3Module->virtualModule.sync3UI.button6Mode;

    }

    void setMode(int mode) override {

        Sync3 * sync3Module = dynamic_cast<Sync3 *>(this->module);

        sync3Module->virtualModule.sync3UI.button6Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button6Mode, BUTTON6_MASK, BUTTON6_SHIFT);
        sync3Module->virtualModule.handleButton6ModeChange(mode);

    }

};
