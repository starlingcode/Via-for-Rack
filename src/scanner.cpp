#include "scanner.hpp"
#include "via_module.hpp"

#define SCANNER_OVERSAMPLE_AMOUNT 8
#define SCANNER_OVERSAMPLE_QUALITY 6

struct Scanner : Via<SCANNER_OVERSAMPLE_AMOUNT, SCANNER_OVERSAMPLE_QUALITY> {

        // Buttons

    struct JumpQuantity : ViaButtonQuantity<2> {

        std::string buttonModes[2] = {"Reverse", "Teleport"};

        JumpQuantity() {
            for (int i = 0; i < 2; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Scanner * scannerModule = dynamic_cast<Scanner *>(this->module);

            return scannerModule->virtualModule.scannerUI.button1Mode;

        }

        void setMode(int mode) override {

            Scanner * scannerModule = dynamic_cast<Scanner *>(this->module);

            scannerModule->virtualModule.scannerUI.button1Mode = mode;
            scannerModule->virtualModule.scannerUI.storeMode(scannerModule->virtualModule.scannerUI.button1Mode, BUTTON1_MASK, BUTTON1_SHIFT);
            scannerModule->virtualModule.handleButton1ModeChange(mode);

        }

    };

    struct YWorldQuantity : ViaButtonQuantity<8> {

        std::string buttonModes[8] = {"Slopes", "Hills", "Pyhisics World", "Shapeshifting Range", "Multiplier Mountains", "Synthville", "Steppes", "Blockland"};
        std::string descriptions[8] = {
            "Exponential to logarithmic shaping",
            "Evenly spaced half-sine peaks and valleys",
            "Samples of a vibrating string model",
            "A trio of peaks with changing shape",
            "Linear slopes with dropoffs",
            "Modeled lowpass filter with increasing cutoff",
            "Bitcrushing from 1-5 bits",
            "Ascending/descending 16 step patterns"
        };


        YWorldQuantity() {
            for (int i = 0; i < 8; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Scanner * scannerModule = dynamic_cast<Scanner *>(this->module);

            int mode = scannerModule->virtualModule.scannerUI.button2Mode;

            description = descriptions[mode];

            return mode;

        }

        void setMode(int mode) override {

            Scanner * scannerModule = dynamic_cast<Scanner *>(this->module);

            scannerModule->virtualModule.scannerUI.button2Mode = mode;
            scannerModule->virtualModule.scannerUI.storeMode(scannerModule->virtualModule.scannerUI.button2Mode, BUTTON2_MASK, BUTTON2_SHIFT);
            scannerModule->virtualModule.handleButton2ModeChange(mode);

        }

    };

    struct MapQuantity : ViaButtonQuantity<8> {

        std::string buttonModes[4] = {"Add", "Multiply", "Difference", "Lighten"};

        MapQuantity() {
            for (int i = 0; i < 4; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Scanner * scannerModule = dynamic_cast<Scanner *>(this->module);

            return scannerModule->virtualModule.scannerUI.button3Mode;

        }

        void setMode(int mode) override {

            Scanner * scannerModule = dynamic_cast<Scanner *>(this->module);

            scannerModule->virtualModule.scannerUI.button3Mode = mode;
            scannerModule->virtualModule.scannerUI.storeMode(scannerModule->virtualModule.scannerUI.button3Mode, BUTTON3_MASK, BUTTON3_SHIFT);
            scannerModule->virtualModule.handleButton3ModeChange(mode);

        }

    };

    struct XWorldQuantity : ViaButtonQuantity<8> {

        std::string buttonModes[8] = {"Slopes", "Hills", "Pyhisics World", "Shapeshifting Range", "Multiplier Mountains", "Synthville", "Steppes", "Blockland"};

        std::string descriptions[8] = {
            "Smooth tanh waveshaping",
            "A steep slope followed by gentler hills",
            "A bouncing ball trajectory",
            "Add peaks and valleys to a simple slope",
            "Sinusoidal slopes with steep dropoffs",
            "Waveforms from 2 op FM with increasing mod freq",
            "Staircases with 1-5 steps",
            "Ascending patterns of 8 steps"
        };

        XWorldQuantity() {
            for (int i = 0; i < 8; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Scanner * scannerModule = dynamic_cast<Scanner *>(this->module);

            int mode = scannerModule->virtualModule.scannerUI.button4Mode;

            description = descriptions[mode];

            return mode;

        }

        void setMode(int mode) override {

            Scanner * scannerModule = dynamic_cast<Scanner *>(this->module);

            scannerModule->virtualModule.scannerUI.button4Mode = mode;
            scannerModule->virtualModule.scannerUI.storeMode(scannerModule->virtualModule.scannerUI.button4Mode, BUTTON4_MASK, BUTTON4_SHIFT);
            scannerModule->virtualModule.handleButton4ModeChange(mode);

        }

    };
    
    Scanner() : Via() {

        virtualIO = &virtualModule;

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(KNOB1_PARAM, 0, 4095.0, 2048.0, "Surface shape control", "", 0.0, 4.0/4095.0);
        configParam(KNOB2_PARAM, 0, 4095.0, 2048.0, "X scan offset", "", 0.0, 2.0/4095.0, -1.0);
        configParam(KNOB3_PARAM, 0, 4095.0, 2048.0, "Y scan offset", "", 0.0, 2.0/4095.0, -1.0);
        configParam<BScaleQuantity>(B_PARAM, -1.0, 1.0, -1.0, "B level");
        paramQuantities[B_PARAM]->description = "Main scan out is bounded between A and B levels";
        configParam<CV2ScaleQuantity>(CV2AMT_PARAM, 0, 1.0, 1.0, "X input attenuation");
        configParam<ANormalQuantity>(A_PARAM, -5.0, 5.0, 5.0, "A level");
        paramQuantities[A_PARAM]->description = "Main scan out is bounded between A and B levels";
        configParam<CV3ScaleQuantity>(CV3AMT_PARAM, 0, 1.0, 1.0, "Y input attenuation");
        
        configParam<JumpQuantity>(BUTTON1_PARAM, 0.0, 1.0, 0.0, "JUMP input response");
        configParam<YWorldQuantity>(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Y scan world");
        configParam<MapQuantity>(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Map creation function");
        configParam<XWorldQuantity>(BUTTON4_PARAM, 0.0, 1.0, 0.0, "X scan world");
        configParam<YWorldQuantity>(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Y scan world");
        configParam<XWorldQuantity>(BUTTON6_PARAM, 0.0, 1.0, 0.0, "X scan world");
        
        configParam(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Unused");
        paramQuantities[TRIGBUTTON_PARAM]->description = "Enter preset menu on hardware module";


        onSampleRateChange();
        presetData[0] = virtualModule.scannerUI.stockPreset1;
        presetData[1] = virtualModule.scannerUI.stockPreset2;
        presetData[2] = virtualModule.scannerUI.stockPreset3;
        presetData[3] = virtualModule.scannerUI.stockPreset4;
        presetData[4] = virtualModule.scannerUI.stockPreset5;
        presetData[5] = virtualModule.scannerUI.stockPreset6;
        
    }
    void step() override;

    ViaScanner virtualModule;

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();

        ledDecay = 16.0/sampleRate;
        
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
        }  else if (sampleRate == 352800.0) {
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

        json_object_set_new(rootJ, "scanner_modes", json_integer(virtualModule.scannerUI.modeStateBuffer));
        
        return rootJ;
    }
    
    void dataFromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "scanner_modes");
        virtualModule.scannerUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.scannerUI.loadFromEEPROM(0);
        virtualModule.scannerUI.recallModuleState();


    }
    
};

void Scanner::step() {

    clockDivider++;

    if (clockDivider >= divideAmount) {

        // update the "slow IO" (not audio rate) every 16 samples
        // needs to scale with sample rate somehow
        slowIOPrescaler++;
        if (slowIOPrescaler == 16) {
            slowIOPrescaler = 0;
            updateSlowIO();
            virtualModule.slowConversionCallback();
            virtualModule.ui_dispatch(SENSOR_EVENT_SIG);
            virtualModule.scannerUI.incrementTimer();
            processTriggerButton();
            updateLEDs();

        }

        updateAudioRate();
    }
    
}

struct ScannerWidget : ModuleWidget  {

    ScannerWidget(Scanner *module) {

	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setModule(module);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scanner.svg")));


        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Scanner::KNOB1_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Scanner::KNOB2_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Scanner::KNOB3_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Scanner::B_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Scanner::CV2AMT_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Scanner::A_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Scanner::CV3AMT_PARAM));
        
        addParam(createParam<TransparentButton>(Vec(21 + .753, 105), module, Scanner::BUTTON4_PARAM));
        addParam(createParam<TransparentButton>(Vec(47 + .753, 77.5), module, Scanner::BUTTON2_PARAM));
        addParam(createParam<TransparentButton>(Vec(75 + .753, 105), module, Scanner::BUTTON6_PARAM));
        addParam(createParam<TransparentButton>(Vec(7 + .753, 142), module, Scanner::BUTTON1_PARAM));
        addParam(createParam<TransparentButton>(Vec(47 + .753, 131.5), module, Scanner::BUTTON5_PARAM));
        addParam(createParam<TransparentButton>(Vec(89 + .753, 142), module, Scanner::BUTTON3_PARAM));
        
        addParam(createParam<ViaPushButton>(Vec(132.7 + .753, 320), module, Scanner::TRIGBUTTON_PARAM));


        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 241.12), module, Scanner::A_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 282.62), module, Scanner::B_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 324.02), module, Scanner::MAIN_LOGIC_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 241.12), module, Scanner::CV1_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 282.62), module, Scanner::CV2_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 324.02), module, Scanner::CV3_INPUT));
        addInput(createInput<ViaJack>(Vec(135 + 1.053, 282.62), module, Scanner::AUX_LOGIC_INPUT));

        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 241.12), module, Scanner::LOGICA_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 282.62), module, Scanner::AUX_DAC_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 324.02), module, Scanner::MAIN_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(135 + 1.053, 241.12), module, Scanner::AUX_LOGIC_OUTPUT));

        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Scanner::LED1_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 268.5), module, Scanner::LED2_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Scanner::LED3_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 309.9), module, Scanner::LED4_LIGHT));
        addChild(createLight<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Scanner::OUTPUT_GREEN_LIGHT));
        addChild(createLight<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Scanner::RED_LIGHT));

    };

    void appendContextMenu(Menu *menu) override {
        Scanner *module = dynamic_cast<Scanner*>(this->module);

        struct PresetRecallItem : MenuItem {
            Scanner *module;
            int preset;
            void onAction(const event::Action &e) override {
                module->virtualModule.scannerUI.modeStateBuffer = preset;
                module->virtualModule.scannerUI.loadFromEEPROM(0);
                module->virtualModule.scannerUI.recallModuleState();
            }
        };

        struct StockPresetItem : MenuItem {
            Scanner *module;
            Menu *createChildMenu() override {
                Menu *menu = new Menu();
                const std::string presetLabels[] = {
                    "Slopes",
                    "Physics World",
                    "Multiplier Mountains",
                    "Synthville",
                    "Steppes",
                    "Blockland",
                };
                for (int i = 0; i < (int) LENGTHOF(presetLabels); i++) {
                    PresetRecallItem *item = createMenuItem<PresetRecallItem>(presetLabels[i], CHECKMARK(module->virtualModule.scannerUI.modeStateBuffer == (int32_t) module->presetData[i]));
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

Model *modelScanner = createModel<Scanner, ScannerWidget>("SCANNER");


