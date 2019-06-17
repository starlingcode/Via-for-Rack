#include "scanner.hpp"
#include "via_module.hpp"

#define SCANNER_OVERSAMPLE_AMOUNT 8
#define SCANNER_OVERSAMPLE_QUALITY 6

struct Scanner : Via<SCANNER_OVERSAMPLE_AMOUNT, SCANNER_OVERSAMPLE_QUALITY> {
    
    Scanner() : Via() {

        virtualIO = &virtualModule;

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(KNOB1_PARAM, 0, 4095.0, 2048.0, "Surface shape control", "", 0.0, 1.0/4095.0);
        configParam(KNOB2_PARAM, 0, 4095.0, 2048.0, "X scan input offset", "", 0.0, 1.0/4095.0);
        configParam(KNOB3_PARAM, 0, 4095.0, 2048.0, "Y scan input offset", "", 0.0, 1.0/4095.0);
        configParam(B_PARAM, -1.0, 1.0, 0.5, "B input attenuverter, main AXB output ranges from A to B");
        configParam(CV2AMT_PARAM, 0, 1.0, 1.0, "X scan input amount");
        configParam(A_PARAM, -5.0, 5.0, 5.0, "Manual A input overriden when patched");
        configParam(CV3AMT_PARAM, 0, 1.0, 1.0, "Y scan input amount");
        
        configParam(BUTTON1_PARAM, 0.0, 1.0, 0.0, "JUMP input: teleport or reverse");
        configParam(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Y scan world up");
        configParam(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Select map creation function");
        configParam(BUTTON4_PARAM, 0.0, 1.0, 0.0, "X scan world right");
        configParam(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Y scan world down");
        configParam(BUTTON6_PARAM, 0.0, 1.0, 0.0, "X scan world left");
        
        configParam(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Unused (preset select in hardware)");

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

Model *modelScanner = createModel<Scanner, ScannerWidget>("SCANNER");


