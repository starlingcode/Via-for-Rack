#include "gateseq.hpp"
#include "via_module.hpp"

#define GATESEQ_OVERSAMPLE_AMOUNT 1
#define GATESEQ_OVERSAMPLE_QUALITY 1

struct Gateseq : Via<GATESEQ_OVERSAMPLE_AMOUNT, GATESEQ_OVERSAMPLE_QUALITY>  {

    struct SHIButtonQuantity : ParamQuantity {

        std::string modes[3] = {"Off", "Resample A", "Sample and Track A"};

        float getDisplayValue() override {
            if (!module)
                return Quantity::getDisplayValue();

            Gateseq * gateseqModule = (Gateseq *) module;

            return gateseqModule->virtualModule.gateseqUI.button1Mode;
        }

        std::string getDisplayValueString() override {
            return modes[(int) getDisplayValue()];
        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct GateIButtonQuantity : ParamQuantity {

        std::string modes[3] = {"Open", "Logic Gate A", "Audio Gate A"};

        std::string getDisplayValueString() override {

            Gateseq * gateseqModule = (Gateseq *) module;

            return modes[gateseqModule->virtualModule.gateseqUI.button2Mode];
        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct SeqIButtonQuantity : ParamQuantity {

        std::string modes[4] = {"Length 16 Euclidean", "3 vs 2", "Shuffle-Swing", "Multiplier/Divider"};

        std::string getDisplayValueString() override {

            Gateseq * gateseqModule = (Gateseq *) module;

            return modes[gateseqModule->virtualModule.gateseqUI.button3Mode];
        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct SHIIButtonQuantity : ParamQuantity {

        std::string modes[3] = {"Off", "Resample B", "Sample and Track B"};
        std::string getDisplayValueString() override {

            Gateseq * gateseqModule = (Gateseq *) module;

            return modes[gateseqModule->virtualModule.gateseqUI.button4Mode];

        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct GateIIButtonQuantity : ParamQuantity {

        std::string modes[3] = {"Open", "Logic Gate B", "Audio Gate B"};

        std::string getDisplayValueString() override {

            Gateseq * gateseqModule = (Gateseq *) module;

            return modes[gateseqModule->virtualModule.gateseqUI.button5Mode];
        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct SeqIIButtonQuantity : ParamQuantity {

        std::string modes[4] = {"Length 16 Euclidean", "Odd vs Even", "2 or 3 Gates", "Rhythmic Clock Division"};

        std::string getDisplayValueString() override {

            Gateseq * gateseqModule = (Gateseq *) module;

            return modes[gateseqModule->virtualModule.gateseqUI.button6Mode];
        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct PtnIDensityQuantity : ParamQuantity {

        std::string getString() override {
            return "Pattern I Density";
        }

    };

    struct ModulationQuantity : ParamQuantity {

        std::string modes[4] = {"Offset", "Offset", "Shuffle to Swing", "Multiplier"};

        std::string getDisplayValueString() override {

            Gateseq * gateseqModule = (Gateseq *) module;

            return modes[gateseqModule->virtualModule.gateseqUI.button3Mode];                

        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct ModulationCVQuantity : ParamQuantity {

        std::string modes[4] = {"(Offset)", "(Offset)", "(Shufle to Swing)", "(Multiplier)"};

        std::string getDisplayValueString() override {

            Gateseq * gateseqModule = (Gateseq *) module;

            return "PTN I Modulation CV Attenuator " + modes[gateseqModule->virtualModule.gateseqUI.button3Mode];

        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct PtnIIDensityQuantity : ParamQuantity {

        std::string getString() override {
            return "Pattern II Density";
        }

    };

    struct PtnIICVQuantity : ParamQuantity {

        std::string getString() override {
            return "Pattern II Density CV Attenuator";
        }

    };

    struct BScaleQuantity : ParamQuantity {

        std::string getDisplayValueString() override {

            Gateseq * gateseqModule = (Gateseq *) module;

            bool bConnected = gateseqModule->inputs[B_INPUT].isConnected();

            float v = getSmoothValue();

            if (bConnected) {
                return "B scale: " + string::f("%.*g", 2, v);
            } else {
                return "B manual: " + string::f("%.*g", 2, v * 5.0) + "V";                
            }

        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct ANormalQuantity : ParamQuantity {

        std::string getDisplayValueString() override {

            Gateseq * gateseqModule = (Gateseq *) module;

            bool aConnected = gateseqModule->inputs[A_INPUT].isConnected();

            float v = getSmoothValue();

            if (aConnected) {
                return "Overriden by input patch";
            } else {
                return "A manual: " + string::f("%.*g", 2, v) + "V";                
            }

        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct ButtonQuantity : ParamQuantity {

        std::string getString() override {
            return "Manual Trigger";
        }

    };
    
    Gateseq() : Via() {

        virtualIO = &virtualModule;

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam<PtnIDensityQuantity>(KNOB1_PARAM, 0, 4095.0, 2048.0, "Pattern I density", "", 0.0, 1.0/4095.0);
        configParam<ModulationQuantity>(KNOB2_PARAM, 0, 4095.0, 2048.0, "Pattern I modulation", "", 0.0, 1.0/4095.0);
        configParam<PtnIIDensityQuantity>(KNOB3_PARAM, 0, 4095.0, 2048.0, "Pattern II density", "", 0.0, 1.0/4095.0);
        configParam<BScaleQuantity>(B_PARAM, -1.0, 1.0, 0.5, "Pattern II gate level");
        configParam<ModulationCVQuantity>(CV2AMT_PARAM, 0, 1.0, 1.0, "Pattern I modulation CV amount");
        configParam<ANormalQuantity>(A_PARAM, -5.0, 5.0, 5.0, "Pattern I gate level");
        configParam<PtnIICVQuantity>(CV3AMT_PARAM, 0, 1.0, 1.0, "Pattern II density CV amount");
        
        configParam<SHIButtonQuantity>(BUTTON1_PARAM, 0.0, 1.0, 0.0, "A channel/ PTN I S+H control");
        configParam<GateIButtonQuantity>(BUTTON2_PARAM, 0.0, 1.0, 0.0, "A channel/ PTN I gate control");
        configParam<SeqIButtonQuantity>(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Pattern I mode");
        configParam<SHIIButtonQuantity>(BUTTON4_PARAM, 0.0, 1.0, 0.0, "B channel/ PTN II S+H control");
        configParam<GateIIButtonQuantity>(BUTTON5_PARAM, 0.0, 1.0, 0.0, "B channel/ PTN II gate control");
        configParam<SeqIIButtonQuantity>(BUTTON6_PARAM, 0.0, 1.0, 0.0, "Pattern II bank");
        
        configParam<ButtonQuantity>(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Pattern reset");

        onSampleRateChange();

        presetData[0] = virtualModule.gateseqUI.stockPreset1;
        presetData[1] = virtualModule.gateseqUI.stockPreset2;
        presetData[2] = virtualModule.gateseqUI.stockPreset3;
        presetData[3] = virtualModule.gateseqUI.stockPreset4;
        presetData[4] = virtualModule.gateseqUI.stockPreset5;
        presetData[5] = virtualModule.gateseqUI.stockPreset6;
    }
    
    void process(const ProcessArgs &args) override;

    ViaGateseq virtualModule;

    void onSampleRateChange() override {

        float sampleRate = APP->engine->getSampleRate();

        ledDecay = 16.0/sampleRate;

        if (sampleRate == 44100.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 44;
        } else if (sampleRate == 48000.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 48;
        } else if (sampleRate == 88200.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 88;
        } else if (sampleRate == 96000.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 96;
        } else if (sampleRate == 176400.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 176;
        } else if (sampleRate == 192000.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 192;
        } else if (sampleRate == 352800.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 353;
        } else if (sampleRate == 384000.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 384;
        } else if (sampleRate == 705600.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 706;
        } else if (sampleRate == 768000.0) {
            virtualModule.sequencer.virtualTimer4Overflow = 768;
        }
        
    }

    json_t *dataToJson() override {

        json_t *rootJ = json_object();
        
        json_object_set_new(rootJ, "gateseq_modes", json_integer(virtualModule.gateseqUI.modeStateBuffer));
        json_object_set_new(rootJ, "logic_mode", json_integer((int) virtualModule.gateseqUI.aux2Mode));
     
        return rootJ;
    }
    
    void dataFromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "gateseq_modes");
        virtualModule.gateseqUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.gateseqUI.loadFromEEPROM(0);
        virtualModule.gateseqUI.recallModuleState();


    }
    
};

void Gateseq::process(const ProcessArgs &args) {

    // update the "slow IO" (not audio rate) every 16 samples
    // needs to scale with sample rate somehow
    slowIOPrescaler++;
    if (slowIOPrescaler == 16) {
        slowIOPrescaler = 0;
        updateSlowIO();
        virtualModule.slowConversionCallback();
        virtualModule.ui_dispatch(SENSOR_EVENT_SIG);
        virtualModule.gateseqUI.incrementTimer();
        processTriggerButton();
        updateLEDs();

    }

    // manage the software timers
    virtualModule.sequencer.virtualTimer1Count += virtualModule.sequencer.virtualTimer1Enable;
    virtualModule.sequencer.virtualTimer2Count += virtualModule.sequencer.virtualTimer2Enable;
    virtualModule.sequencer.virtualTimer3Count += virtualModule.sequencer.virtualTimer3Enable;
    virtualModule.sequencer.virtualTimer4Count += virtualModule.sequencer.virtualTimer4Enable;

    if (virtualModule.sequencer.virtualTimer2Count >= virtualModule.sequencer.virtualTimer2Overflow) {
        virtualModule.auxTimer1InterruptCallback();
        virtualModule.sequencer.virtualTimer2Count = 0;
    }
    if (virtualModule.sequencer.virtualTimer3Count >= virtualModule.sequencer.virtualTimer3Overflow) {
        virtualModule.auxTimer2InterruptCallback();
        virtualModule.sequencer.virtualTimer3Count = 0;
    }
    if (virtualModule.sequencer.virtualTimer4Count >= virtualModule.sequencer.virtualTimer4Overflow) {
        virtualModule.auxTimer3InterruptCallback();
        virtualModule.sequencer.virtualTimer4Count = 0;
    }

    acquireCVs();
    processLogicInputs();
    updateOutputs();

    updateLogicOutputs();
    virtualIO->halfTransferCallback();

    float dac1Sample = (float) virtualIO->outputs.dac1Samples[0];
    float dac2Sample = (float) virtualIO->outputs.dac2Samples[0];
    float dac3Sample = (float) virtualIO->outputs.dac3Samples[0];

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
    // normalize 12 bits to 0-1
    outputs[MAIN_OUTPUT].setVoltage(bIn*(dac2Sample/4095.0) + aIn*(dac1Sample/4095.0)); 
    outputs[AUX_DAC_OUTPUT].setVoltage((dac3Sample/4095.0 - .5) * -10.666666666);
    outputs[LOGICA_OUTPUT].setVoltage(logicAState * 5.0);
    outputs[AUX_LOGIC_OUTPUT].setVoltage(auxLogicState * 5.0);

    clockDivider = 0;
    
}

struct GateseqWidget : ModuleWidget  {

    GateseqWidget(Gateseq *module) {

        setModule(module);

        box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/gateseq.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Gateseq::KNOB1_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Gateseq::KNOB2_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Gateseq::KNOB3_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Gateseq::B_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Gateseq::CV2AMT_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Gateseq::A_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Gateseq::CV3AMT_PARAM));
        
        addParam(createParam<TransparentButton>(Vec(8 + .753, 85), module, Gateseq::BUTTON1_PARAM));
        addParam(createParam<TransparentButton>(Vec(48 + .753, 85), module, Gateseq::BUTTON2_PARAM));
        addParam(createParam<TransparentButton>(Vec(86 + .753, 85), module, Gateseq::BUTTON3_PARAM));
        addParam(createParam<TransparentButton>(Vec(8 + .753, 139), module, Gateseq::BUTTON4_PARAM));
        addParam(createParam<TransparentButton>(Vec(48 + .753, 139), module, Gateseq::BUTTON5_PARAM));
        addParam(createParam<TransparentButton>(Vec(86 + .753, 139), module, Gateseq::BUTTON6_PARAM));
        
        addParam(createParam<ViaPushButton>(Vec(132.7 + .753, 320), module, Gateseq::TRIGBUTTON_PARAM));

        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 241.12), module, Gateseq::A_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 282.62), module, Gateseq::B_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 324.02), module, Gateseq::MAIN_LOGIC_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 241.12), module, Gateseq::CV1_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 282.62), module, Gateseq::CV2_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 324.02), module, Gateseq::CV3_INPUT));
        addInput(createInput<ViaJack>(Vec(135 + 1.053, 282.62), module, Gateseq::AUX_LOGIC_INPUT));

        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 241.12), module, Gateseq::LOGICA_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 282.62), module, Gateseq::AUX_DAC_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 324.02), module, Gateseq::MAIN_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(135 + 1.053, 241.12), module, Gateseq::AUX_LOGIC_OUTPUT));

        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Gateseq::LED1_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 268.5), module, Gateseq::LED2_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Gateseq::LED3_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 309.9), module, Gateseq::LED4_LIGHT));
        addChild(createLight<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Gateseq::OUTPUT_GREEN_LIGHT));
        addChild(createLight<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Gateseq::RED_LIGHT));

        }

    void appendContextMenu(Menu *menu) override {
        Gateseq *module = dynamic_cast<Gateseq*>(this->module);
        assert(module);

        struct GateseqAux2ModeHandler : MenuItem {
            Gateseq *module;
            int32_t mode;
            void onAction(const event::Action &e) override {
                module->virtualModule.gateseqUI.aux2Mode = mode;
                module->virtualModule.gateseqUI.storeMode(module->virtualModule.gateseqUI.aux2Mode, AUX_MODE2_MASK, AUX_MODE2_SHIFT);
                module->virtualModule.handleAux2ModeChange(mode);
            }
        };

        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("Drum signal out"));
        const std::string logicLabels[] = {
            "And",
            "Or",
            "Xor",
            "Nor"
        };
        for (int i = 0; i < (int) LENGTHOF(logicLabels); i++) {
            GateseqAux2ModeHandler *aux2Item = createMenuItem<GateseqAux2ModeHandler>(logicLabels[i], CHECKMARK(module->virtualModule.gateseqUI.aux2Mode == i));
            aux2Item->module = module;
            aux2Item->mode = i;
            menu->addChild(aux2Item);
        }

        struct PresetRecallItem : MenuItem {
            Gateseq *module;
            int preset;
            void onAction(const event::Action &e) override {
                module->virtualModule.gateseqUI.modeStateBuffer = preset;
                module->virtualModule.gateseqUI.loadFromEEPROM(0);
                module->virtualModule.gateseqUI.recallModuleState();
            }
        };

        struct StockPresetItem : MenuItem {
            Gateseq *module;
            Menu *createChildMenu() override {
                Menu *menu = new Menu();
                const std::string presetLabels[] = {
                    "Euclidean",
                    "2 vs 3",
                    "Shuffle Swing",
                    "Multiplier",
                    "Logic",
                    "Resample",
                };
                for (int i = 0; i < (int) LENGTHOF(presetLabels); i++) {
                    PresetRecallItem *item = createMenuItem<PresetRecallItem>(presetLabels[i]);
                    item->module = module;
                    item->preset = module->presetData[i];
                    menu->addChild(item);
                }
                return menu;
            }
        };

        menu->addChild(new MenuEntry);
        StockPresetItem *stockPresets = createMenuItem<StockPresetItem>("Stock presets");
        stockPresets->module = module;
        menu->addChild(stockPresets);

        }

};



Model *modelGateseq = createModel<Gateseq, GateseqWidget>("GATESEQ");


