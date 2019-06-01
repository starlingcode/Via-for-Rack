#include "sync.hpp"
#include "via_module.hpp"

#define SYNC_OVERSAMPLE_AMOUNT 8
#define SYNC_OVERSAMPLE_QUALITY 6

struct Sync : Via<SYNC_OVERSAMPLE_AMOUNT, SYNC_OVERSAMPLE_QUALITY> {

    enum ParamIds {
        KNOB1_PARAM,
        KNOB2_PARAM,
        KNOB3_PARAM,
        A_PARAM,
        B_PARAM,
        CV2AMT_PARAM,
        CV3AMT_PARAM,
        BUTTON1_PARAM,
        BUTTON2_PARAM,
        BUTTON3_PARAM,
        BUTTON4_PARAM,
        BUTTON5_PARAM,
        BUTTON6_PARAM,
        TRIGBUTTON_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        A_INPUT,
        B_INPUT,
        CV1_INPUT,
        CV2_INPUT,
        CV3_INPUT,
        MAIN_LOGIC_INPUT,
        AUX_LOGIC_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        MAIN_OUTPUT,
        LOGICA_OUTPUT,
        AUX_DAC_OUTPUT,
        AUX_LOGIC_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        LED1_LIGHT,
        LED2_LIGHT,
        LED3_LIGHT,
        LED4_LIGHT,
        OUTPUT_GREEN_LIGHT,
        OUTPUT_RED_LIGHT,
        RED_LIGHT,
        GREEN_LIGHT,
        BLUE_LIGHT,
        PURPLE_LIGHT,
        NUM_LIGHTS
    };
    
    Sync() : Via() {

        virtualIO = &virtualModule;

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(KNOB1_PARAM, 0, 4095.0, 2048.0, "Label Me!");
        configParam(KNOB2_PARAM, 0, 4095.0, 2048.0, "Label Me!");
        configParam(KNOB3_PARAM, 0, 4095.0, 2048.0, "Label Me!");
        configParam(B_PARAM, -1.0, 1.0, 0.5, "Label Me!");
        configParam(CV2AMT_PARAM, 0, 1.0, 1.0, "Label Me!");
        configParam(A_PARAM, -5.0, 5.0, 5.0, "Label Me!");
        configParam(CV3AMT_PARAM, 0, 1.0, 1.0, "Label Me!");
        
        configParam(BUTTON1_PARAM, 0.0, 1.0, 0.0, "Label Me!");
        configParam(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Label Me!");
        configParam(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Label Me!");
        configParam(BUTTON4_PARAM, 0.0, 1.0, 0.0, "Label Me!");
        configParam(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Label Me!");
        configParam(BUTTON6_PARAM, 0.0, 1.0, 0.0, "Label Me!");
        
        configParam(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Label Me!");

        onSampleRateChange();
        presetData[0] = virtualModule.syncUI.stockPreset1;
        presetData[1] = virtualModule.syncUI.stockPreset2;
        presetData[2] = virtualModule.syncUI.stockPreset3;
        presetData[3] = virtualModule.syncUI.stockPreset4;
        presetData[4] = virtualModule.syncUI.stockPreset5;
        presetData[5] = virtualModule.syncUI.stockPreset6;
    }

    ViaSync virtualModule;

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();

        if (sampleRate == 44100.0) {
            divideAmount = 1;
            virtualModule.virtualTimerOverflow = 44;
        } else if (sampleRate == 48000.0) {
            divideAmount = 1;
            virtualModule.virtualTimerOverflow = 48;
        } else if (sampleRate == 88200.0) {
            divideAmount = 2;
            virtualModule.virtualTimerOverflow = 44;
        } else if (sampleRate == 96000.0) {
            divideAmount = 2;
            virtualModule.virtualTimerOverflow = 48;

        } else if (sampleRate == 176400.0) {
            divideAmount = 4;
            virtualModule.virtualTimerOverflow = 44;
        } else if (sampleRate == 192000.0) {
            divideAmount = 4;
            virtualModule.virtualTimerOverflow = 48;
        }
        
    }

    json_t *dataToJson() override {

        json_t *rootJ = json_object();
        
        json_object_set_new(rootJ, "sync_modes", json_integer(virtualModule.syncUI.modeStateBuffer));
        
        return rootJ;
    }
    
    void dataFromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "sync_modes");
        virtualModule.syncUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.syncUI.loadFromEEPROM(0);
        virtualModule.syncUI.recallModuleState();


    }

    void process(const ProcessArgs &args) override {

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
            virtualModule.syncUI.incrementTimer();
            // trigger handling
            processTriggerButton();
            updateLEDs();
        }

        // manage the complex software timer

        virtualModule.virtualTimer += virtualModule.virtualTimerEnable;
        if (virtualModule.virtualTimer >= virtualModule.virtualTimerOverflow) {
            virtualModule.virtualTimer = 0;
            virtualModule.auxTimer2InterruptCallback();
        }

        updateAudioRate();

        virtualModule.incrementVirtualTimer();

        }
    
    }
    
};


struct Sync_Widget : ModuleWidget  {

    Sync_Widget(Sync *module) {

        box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setModule(module);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sync.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Sync::KNOB1_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Sync::KNOB2_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Sync::KNOB3_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Sync::B_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Sync::CV2AMT_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Sync::A_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Sync::CV3AMT_PARAM));
        
        addParam(createParam<TransparentButton>(Vec(7 + .753, 82), module, Sync::BUTTON1_PARAM));
        addParam(createParam<TransparentButton>(Vec(48 + .753, 79.5), module, Sync::BUTTON2_PARAM));
        addParam(createParam<TransparentButton>(Vec(88.5 + .753, 82), module, Sync::BUTTON3_PARAM));
        addParam(createParam<TransparentButton>(Vec(7 + .753, 136.5), module, Sync::BUTTON4_PARAM));
        addParam(createParam<TransparentButton>(Vec(48 + .753, 135.5), module, Sync::BUTTON5_PARAM));
        addParam(createParam<TransparentButton>(Vec(88.5 + .753, 136.5), module, Sync::BUTTON6_PARAM));
        
        addParam(createParam<ViaPushButton>(Vec(132.7 + .753, 320), module, Sync::TRIGBUTTON_PARAM));

        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 241.12), module, Sync::A_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 282.62), module, Sync::B_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 324.02), module, Sync::MAIN_LOGIC_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 241.12), module, Sync::CV1_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 282.62), module, Sync::CV2_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 324.02), module, Sync::CV3_INPUT));
        addInput(createInput<ViaJack>(Vec(135 + 1.053, 282.62), module, Sync::AUX_LOGIC_INPUT));

        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 241.12), module, Sync::LOGICA_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 282.62), module, Sync::AUX_DAC_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 324.02), module, Sync::MAIN_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(135 + 1.053, 241.12), module, Sync::AUX_LOGIC_OUTPUT));

        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Sync::LED1_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 268.5), module, Sync::LED2_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Sync::LED3_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 309.9), module, Sync::LED4_LIGHT));
        addChild(createLight<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Sync::OUTPUT_GREEN_LIGHT));
        addChild(createLight<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Sync::RED_LIGHT));

        }

    void appendContextMenu(Menu *menu) override {
        Sync *module = dynamic_cast<Sync*>(this->module);
        assert(module);

        struct SyncAux1ModeHandler : MenuItem {
            Sync *module;
            int32_t mode;
            void onAction(const event::Action &e) override {
                module->virtualModule.syncUI.aux1Mode = mode;
                module->virtualModule.handleAux1ModeChange(mode);
                module->virtualModule.syncUI.storeMode(module->virtualModule.syncUI.aux1Mode, AUX_MODE1_MASK, AUX_MODE1_SHIFT);
            }
        };

        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("Logic Out"));
        const std::string logicLabels[] = {
            "High during attack",
            "Delta"
        };
        for (int i = 0; i < (int) LENGTHOF(logicLabels); i++) {
            SyncAux1ModeHandler *aux1Item = createMenuItem<SyncAux1ModeHandler>(logicLabels[i], CHECKMARK(module->virtualModule.syncUI.aux1Mode == i));
            aux1Item->module = module;
            aux1Item->mode = i;
            menu->addChild(aux1Item);
        }

        struct SyncAux2ModeHandler : MenuItem {
            Sync *module;
            int32_t mode;
            void onAction(const event::Action &e) override {
                module->virtualModule.syncUI.aux2Mode = mode;
                module->virtualModule.handleAux2ModeChange(mode);
                module->virtualModule.syncUI.storeMode(module->virtualModule.syncUI.aux2Mode, AUX_MODE2_MASK, AUX_MODE2_SHIFT);
            }
        };

        menu->addChild(createMenuLabel("Signal Out"));
        const std::string signalLabels[] = {
            "Triangle",
            "Contour"
        };
        for (int i = 0; i < (int) LENGTHOF(signalLabels); i++) {
            SyncAux2ModeHandler *aux2Item = createMenuItem<SyncAux2ModeHandler>(signalLabels[i], CHECKMARK(module->virtualModule.syncUI.aux2Mode == i));
            aux2Item->module = module;
            aux2Item->mode = i;
            menu->addChild(aux2Item);
        }

        struct SyncAux3ModeHandler : MenuItem {
            Sync *module;
            int32_t mode;
            void onAction(const event::Action &e) override {
                module->virtualModule.syncUI.aux3Mode = mode;
                module->virtualModule.handleAux3ModeChange(mode);
                module->virtualModule.syncUI.storeMode(module->virtualModule.syncUI.aux3Mode, AUX_MODE3_MASK, AUX_MODE3_SHIFT);
            }
        };

        menu->addChild(createMenuLabel("Quadrature"));
        const std::string quadratureLabels[] = {
            "0 degrees",
            "90 degrees",
            "180 degrees",
            "270 degrees"
        };
        for (int i = 0; i < (int) LENGTHOF(quadratureLabels); i++) {
            SyncAux3ModeHandler *aux3Item = createMenuItem<SyncAux3ModeHandler>(quadratureLabels[i], CHECKMARK(module->virtualModule.syncUI.aux3Mode == i));
            aux3Item->module = module;
            aux3Item->mode = i;
            menu->addChild(aux3Item);
        }

        struct SyncAux4ModeHandler : MenuItem {
            Sync *module;
            int32_t mode;
            void onAction(const event::Action &e) override {
                module->virtualModule.syncUI.aux4Mode = mode;
                module->virtualModule.handleAux4ModeChange(mode);
                module->virtualModule.syncUI.storeMode(module->virtualModule.syncUI.aux4Mode, AUX_MODE4_MASK, AUX_MODE4_SHIFT);
            }

        };

        menu->addChild(createMenuLabel("Wave Options"));
        const std::string auxLabels[] = {
            "Group-specific",
            "Aux"
        };
        for (int i = 0; i < (int) LENGTHOF(auxLabels); i++) {
            SyncAux4ModeHandler *aux4Item = createMenuItem<SyncAux4ModeHandler>(auxLabels[i], CHECKMARK(module->virtualModule.syncUI.aux4Mode == i));
            aux4Item->module = module;
            aux4Item->mode = i;
            menu->addChild(aux4Item);
        }
        
        struct PresetRecallItem : MenuItem {
            Sync *module;
            int preset;
            void onAction(const event::Action &e) override {
                module->virtualModule.syncUI.modeStateBuffer = preset;
                module->virtualModule.syncUI.loadFromEEPROM(0);
                module->virtualModule.syncUI.recallModuleState();
            }
        };

        struct StockPresetItem : MenuItem {
            Sync *module;
            Menu *createChildMenu() override {
                Menu *menu = new Menu();
                const std::string presetLabels[] = {
                    "Harmonic Osc",
                    "Arpeggiated Osc",
                    "Arpeggiated Osc BP",
                    "V/oct Osc",
                    "Sequence",
                    "Tempo-Synced LFO",
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


Model *modelSync = createModel<Sync, Sync_Widget>("SYNC");


