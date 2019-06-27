#include "atsr.hpp"
#include "via_module.hpp"

#define ATSR_OVERSAMPLE_AMOUNT 1
#define ATSR_OVERSAMPLE_QUALITY 1

struct Atsr : Via<ATSR_OVERSAMPLE_AMOUNT, ATSR_OVERSAMPLE_QUALITY> {

#define PHASE_LENGTH ((float) 0xFFFFFFF)
#define NUM_SAMPLES 4096.0

    struct ATimeQuantity : ViaKnobQuantity {

        virtual float translateParameter(float value) {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            int32_t cycleMod = atsrModule->virtualModule.expo.convert(2048) >> 5;

            float timeBase = __USAT(fix16_mul(cycleMod,
                atsrModule->virtualModule.expo.convert(4095 - atsrModule->virtualModule.controls.knob1Value) >> 6), 25);

            return 1/(((timeBase)/PHASE_LENGTH) * atsrModule->sampleRateStore);            
        
        }
        virtual float translateInput(float userInput) {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            float lengthInSamples = userInput * atsrModule->sampleRateStore;
            float desiredIncrement = NUM_SAMPLES/lengthInSamples;
            float normalizedCycleMod = ((float) (atsrModule->virtualModule.expo.convert(2048) >> 5))/65536.0;
            desiredIncrement = desiredIncrement/normalizedCycleMod;
            desiredIncrement *= 65536.0 * 64.0;

            return (4095 - (atsrModule->reverseExpo(desiredIncrement))*384);

        };

    };

    struct TTimeQuantity : ViaKnobQuantity {

        virtual float translateParameter(float value) {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            int32_t cycleMod = atsrModule->virtualModule.expo.convert(2048) >> 5;

            if (atsrModule->virtualModule.cycleTime) {
                cycleMod = fix16_mul(cycleMod, cycleMod);
            }

            float timeBase = __USAT(fix16_mul(cycleMod,
                atsrModule->virtualModule.expo.convert(4095 - atsrModule->virtualModule.controls.knob2Value) >> 6), 25);

            return 1/(((timeBase)/PHASE_LENGTH) * atsrModule->sampleRateStore);            
        
        }
        virtual float translateInput(float userInput) {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            float lengthInSamples = userInput * atsrModule->sampleRateStore;
            float desiredIncrement = NUM_SAMPLES/lengthInSamples;
            float normalizedCycleMod = ((float) (atsrModule->virtualModule.expo.convert(2048) >> 5))/65536.0;
            if (atsrModule->virtualModule.cycleTime) {
                normalizedCycleMod *= normalizedCycleMod;
            }
            desiredIncrement = desiredIncrement/normalizedCycleMod;
            desiredIncrement *= 65536.0 * 64.0;

            return (4095 - (atsrModule->reverseExpo(desiredIncrement))*384);

        };

    };

    struct RTimeQuantity : ViaKnobQuantity {

        virtual float translateParameter(float value) {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            int32_t cycleMod = atsrModule->virtualModule.expo.convert(2048) >> 5;

            if (atsrModule->virtualModule.cycleTime) {
                cycleMod = fix16_mul(cycleMod, cycleMod);
            }

            float timeBase = __USAT(fix16_mul(cycleMod,
                atsrModule->virtualModule.expo.convert(4095 - atsrModule->virtualModule.controls.knob3Value) >> 6), 25);

            return 1/(((timeBase)/PHASE_LENGTH) * atsrModule->sampleRateStore);            
        
        }
        virtual float translateInput(float userInput) {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            float lengthInSamples = userInput * atsrModule->sampleRateStore;
            float desiredIncrement = NUM_SAMPLES/lengthInSamples;
            float normalizedCycleMod = ((float) (atsrModule->virtualModule.expo.convert(2048) >> 5))/65536.0;
            if (atsrModule->virtualModule.cycleTime) {
                normalizedCycleMod *= normalizedCycleMod;
            }
            desiredIncrement = desiredIncrement/normalizedCycleMod;
            desiredIncrement *= 65536.0 * 64.0;

            return (4095 - (atsrModule->reverseExpo(desiredIncrement))*384);

        };

    };

    // Buttons

    struct ASlopeButtonQuantity : ViaButtonQuantity<4> {

        std::string buttonModes[4] = {"Expo", "Linear", "Sigmoid", "Log"};

        ASlopeButtonQuantity() {
            for (int i = 0; i < 4; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            return atsrModule->virtualModule.atsrUI.button1Mode;

        }

        void setMode(int mode) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            atsrModule->virtualModule.atsrUI.button1Mode = mode;
            atsrModule->virtualModule.atsrUI.storeMode(atsrModule->virtualModule.atsrUI.button1Mode, BUTTON1_MASK, BUTTON1_SHIFT);
            atsrModule->virtualModule.handleButton1ModeChange(mode);

        }

    };

    struct TSlopeButtonQuantity : ViaButtonQuantity<4> {

        std::string buttonModes[4] = {"Expo", "Linear", "Sigmoid", "Log"};

        TSlopeButtonQuantity() {
            for (int i = 0; i < 4; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            return atsrModule->virtualModule.atsrUI.button2Mode;

        }

        void setMode(int mode) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            atsrModule->virtualModule.atsrUI.button2Mode = mode;
            atsrModule->virtualModule.atsrUI.storeMode(atsrModule->virtualModule.atsrUI.button2Mode, BUTTON2_MASK, BUTTON2_SHIFT);
            atsrModule->virtualModule.handleButton2ModeChange(mode);

        }

    };

    struct StageButtonQuantity : ViaButtonQuantity<4> {

        std::string buttonModes[4] = {"Attack", "Transition", "Sustain", "Release"};

        StageButtonQuantity() {
            for (int i = 0; i < 4; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            return atsrModule->virtualModule.atsrUI.button3Mode;

        }

        void setMode(int mode) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            atsrModule->virtualModule.atsrUI.button3Mode = mode;
            atsrModule->virtualModule.atsrUI.storeMode(atsrModule->virtualModule.atsrUI.button3Mode, BUTTON3_MASK, BUTTON3_SHIFT);
            atsrModule->virtualModule.handleButton3ModeChange(mode);

        }

    };


    struct AtkAllButtonQuantity : ViaButtonQuantity<2> {

        std::string buttonModes[2] = {"Attack Time", "All Slopes (V/oct)"};

        AtkAllButtonQuantity() {
            for (int i = 0; i < 2; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            return atsrModule->virtualModule.atsrUI.button4Mode;

        }

        void setMode(int mode) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            atsrModule->virtualModule.atsrUI.button4Mode = mode;
            atsrModule->virtualModule.atsrUI.storeMode(atsrModule->virtualModule.atsrUI.button4Mode, BUTTON4_MASK, BUTTON4_SHIFT);
            atsrModule->virtualModule.handleButton4ModeChange(mode);

        }

    };

    struct SHButtonQuantity : ViaButtonQuantity<2> {

        std::string buttonModes[2] = {"Enabled", "Disabled"};

        SHButtonQuantity() {
            for (int i = 0; i < 2; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            return atsrModule->virtualModule.atsrUI.button5Mode;

        }

        void setMode(int mode) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            atsrModule->virtualModule.atsrUI.button5Mode = mode;
            atsrModule->virtualModule.atsrUI.storeMode(atsrModule->virtualModule.atsrUI.button5Mode, BUTTON5_MASK, BUTTON5_SHIFT);
            atsrModule->virtualModule.handleButton5ModeChange(mode);

        }

    };

    struct RSlopeButtonQuantity : ViaButtonQuantity<4> {

        std::string buttonModes[4] = {"Expo", "Linear", "Sigmoid", "Log"};

        RSlopeButtonQuantity() {
            for (int i = 0; i < 4; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            return atsrModule->virtualModule.atsrUI.button6Mode;

        }

        void setMode(int mode) override {

            Atsr * atsrModule = dynamic_cast<Atsr *>(this->module);

            atsrModule->virtualModule.atsrUI.button6Mode = mode;
            atsrModule->virtualModule.atsrUI.storeMode(atsrModule->virtualModule.atsrUI.button6Mode, BUTTON6_MASK, BUTTON6_SHIFT);
            atsrModule->virtualModule.handleButton6ModeChange(mode);

        }

    };
    
    Atsr() : Via() {

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        virtualIO = &virtualModule;

        configParam<ATimeQuantity>(KNOB1_PARAM, 0, 4095.0, 2048.0, "Attack time", "s", 0.0, 1.0/4095.0);
        configParam<TTimeQuantity>(KNOB2_PARAM, 0, 4095.0, 2048.0, "Transition time", "s", 0.0, 1.0/4095.0);
        configParam<RTimeQuantity>(KNOB3_PARAM, 0, 4095.0, 2048.0, "Release time", "s", 0.0, 1.0/4095.0);
        configParam<BScaleQuantity>(B_PARAM, -1.0, 1.0, 0.5, "Sustain level");
        configParam(CV2AMT_PARAM, 0, 1.0, 1.0, "Transition time CV amount");
        configParam<ANormalQuantity>(A_PARAM, -5.0, 5.0, 5.0, "Attack level");
        configParam(CV3AMT_PARAM, 0, 1.0, 1.0, "Release time CV amount");
        
        configParam<ASlopeButtonQuantity>(BUTTON1_PARAM, 0.0, 1.0, 0.0, "Attack slope shape");
        configParam<TSlopeButtonQuantity>(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Transition slope shape");
        configParam<StageButtonQuantity>(BUTTON3_PARAM, 0.0, 1.0, 0.0, "SEG gate high during");
        configParam<AtkAllButtonQuantity>(BUTTON4_PARAM, 0.0, 1.0, 0.0, "A time CV destination");
        configParam<SHButtonQuantity>(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Level CV S+H");
        configParam<RSlopeButtonQuantity>(BUTTON6_PARAM, 0.0, 1.0, 0.0, "Release slope shape");
        
        configParam<ButtonQuantity>(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Manual gate");

        onSampleRateChange();
    }

    ViaAtsr virtualModule;

    float sampleRateStore = 48000.0;

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();

        ledDecay = 16.0/sampleRate;

        sampleRateStore = sampleRate;

        if (sampleRate == 44100.0) {
            virtualModule.incScale = 71332;
        } else if (sampleRate == 48000.0) {
            virtualModule.incScale = 65536;
        } else if (sampleRate == 88200.0) {
            virtualModule.incScale = 35666;
        } else if (sampleRate == 96000.0) {
            virtualModule.incScale = 32768;
        } else if (sampleRate == 176400.0) {
            virtualModule.incScale = 17833;
        } else if (sampleRate == 192000.0) {
            virtualModule.incScale = 16383;
        } else if (sampleRate == 352800.0) {
            virtualModule.incScale = 16383;
        } else if (sampleRate == 384000.0) {
            virtualModule.incScale = 16383;
        } else if (sampleRate == 705600.0) {
            virtualModule.incScale = 16383;
        } else if (sampleRate == 768000.0) {
            virtualModule.incScale = 16383;
        }
        
    }

    void updateLEDs(void) {

        // the A B C D enumeration of the LEDs in the Via library makes little to no sense 
        // but its woven pretty deep so is a nagging style thing to fix

        if (virtualModule.runtimeDisplay & !virtualModule.shOn) {
            lights[LED1_LIGHT].setSmoothBrightness(virtualModule.blueLevelWrite/4095.0, ledDecay);
            lights[LED3_LIGHT].setSmoothBrightness(virtualModule.redLevelWrite/4095.0, ledDecay);
        } else {
            ledAState = virtualLogicOut(ledAState, virtualModule.ledAOutput);
            ledBState = virtualLogicOut(ledBState, virtualModule.ledBOutput);
            lights[LED1_LIGHT].setSmoothBrightness(ledAState, ledDecay);
            lights[LED3_LIGHT].setSmoothBrightness(ledBState, ledDecay);
        }
        ledCState = virtualLogicOut(ledCState, virtualModule.ledCOutput);
        ledDState = virtualLogicOut(ledDState, virtualModule.ledDOutput);


        lights[LED2_LIGHT].setSmoothBrightness(ledCState, ledDecay);
        lights[LED4_LIGHT].setSmoothBrightness(ledDState, ledDecay);

        lights[RED_LIGHT].setSmoothBrightness(virtualModule.redLevelWrite/4095.0, ledDecay);
        lights[GREEN_LIGHT].setSmoothBrightness(virtualModule.greenLevelWrite/4095.0, ledDecay);
        lights[BLUE_LIGHT].setSmoothBrightness(virtualModule.blueLevelWrite/4095.0, ledDecay);

        float output = outputs[MAIN_OUTPUT].value/8.0;
        lights[OUTPUT_RED_LIGHT].setSmoothBrightness(clamp(-output, 0.0, 1.0), ledDecay);
        lights[OUTPUT_GREEN_LIGHT].setSmoothBrightness(clamp(output, 0.0, 1.0), ledDecay);

    }

    json_t *dataToJson() override {

        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "atsr_modes", json_integer(virtualModule.atsrUI.modeStateBuffer));
    
     
        return rootJ;
    }
    
    void dataFromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "atsr_modes");
        virtualModule.atsrUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.atsrUI.loadFromEEPROM(0);
        virtualModule.atsrUI.recallModuleState();


    }

    void process(const ProcessArgs &args) override;
    
};

void Atsr::process(const ProcessArgs &args) {

    // update the "slow IO" (not audio rate) every 16 samples
    // needs to scale with sample rate somehow
    slowIOPrescaler++;
    if (slowIOPrescaler == 16) {
        slowIOPrescaler = 0;
        updateSlowIO();
        virtualModule.slowConversionCallback();
        virtualModule.ui_dispatch(SENSOR_EVENT_SIG);
        virtualModule.atsrUI.incrementTimer();
        processTriggerButton();
        updateLEDs();
    }

    acquireCVs();

    processLogicInputs();

    virtualModule.halfTransferCallback();
    float dac1Sample = (float) virtualModule.outputs.dac1Samples[0];
    float dac2Sample = (float) virtualModule.outputs.dac2Samples[0];
    float dac3Sample = (float) virtualModule.outputs.dac3Samples[0];
    updateLogicOutputs();

    // "model" the circuit
    // A and B inputs with normalled reference voltages
    float aIn = inputs[A_INPUT].getVoltage() + (!inputs[A_INPUT].isConnected()) * params[A_PARAM].getValue();
    float bIn = (inputs[B_INPUT].isConnected()) * ((inputs[B_INPUT].getVoltage()) * (params[B_PARAM].getValue())) + (!inputs[B_INPUT].isConnected()) * (5* (params[B_PARAM].getValue()));
    
    // sample and holds
    // get a new sample on the rising edge at the sh control output
    if (shAControl > shALast) {
        aSample = aIn;
    }
    if (shBControl > shBLast) {
        bSample = bIn;
    }

    shALast = shAControl;
    shBLast = shBControl;

    // either use the sample or track depending on the sh control output
    aIn = shAControl * aSample + !shAControl * aIn;
    bIn = shBControl * bSample + !shBControl * bIn;

    // VCA/mixing stage
    // normalize 15 bits to 0-1
    outputs[MAIN_OUTPUT].setVoltage(bIn*(dac2Sample/32767.0) + aIn*(dac1Sample/32767.0)); 
    outputs[AUX_DAC_OUTPUT].setVoltage((dac3Sample/4095.0 - .5) * -10.666666666);
    outputs[LOGICA_OUTPUT].setVoltage(logicAState * 5.0);
    outputs[AUX_LOGIC_OUTPUT].setVoltage(auxLogicState * 5.0);
    
}

struct AtsrWidget : ModuleWidget  {

    AtsrWidget(Atsr *module) {

        setModule(module);

        box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/atsr.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Atsr::KNOB1_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Atsr::KNOB2_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Atsr::KNOB3_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Atsr::B_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Atsr::CV2AMT_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Atsr::A_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Atsr::CV3AMT_PARAM));
        
        addParam(createParam<TransparentButton>(Vec(9 + .753, 85), module, Atsr::BUTTON1_PARAM));
        addParam(createParam<TransparentButton>(Vec(48 + .753, 75), module, Atsr::BUTTON2_PARAM));
        addParam(createParam<TransparentButton>(Vec(87 + .753, 86), module, Atsr::BUTTON3_PARAM));
        addParam(createParam<TransparentButton>(Vec(7 + .753, 132), module, Atsr::BUTTON4_PARAM));
        addParam(createParam<TransparentButton>(Vec(48 + .753, 142), module, Atsr::BUTTON5_PARAM));
        addParam(createParam<TransparentButton>(Vec(88 + .753, 135), module, Atsr::BUTTON6_PARAM));
        
        addParam(createParam<ViaPushButton>(Vec(132.7 + .753, 320), module, Atsr::TRIGBUTTON_PARAM));

        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 241.12), module, Atsr::A_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 282.62), module, Atsr::B_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 324.02), module, Atsr::MAIN_LOGIC_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 241.12), module, Atsr::CV1_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 282.62), module, Atsr::CV2_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 324.02), module, Atsr::CV3_INPUT));
        addInput(createInput<ViaJack>(Vec(135 + 1.053, 282.62), module, Atsr::AUX_LOGIC_INPUT));

        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 241.12), module, Atsr::LOGICA_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 282.62), module, Atsr::AUX_DAC_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 324.02), module, Atsr::MAIN_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(135 + 1.053, 241.12), module, Atsr::AUX_LOGIC_OUTPUT));

        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Atsr::LED1_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 268.5), module, Atsr::LED2_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Atsr::LED3_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 309.9), module, Atsr::LED4_LIGHT));
        addChild(createLight<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Atsr::OUTPUT_GREEN_LIGHT));
        addChild(createLight<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Atsr::RED_LIGHT));

        }

};



Model *modelAtsr = createModel<Atsr, AtsrWidget>("ATSR");


