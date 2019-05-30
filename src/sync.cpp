#include "sync.hpp"
#include "via_module.hpp"

#define SYNC_OVERSAMPLE_AMOUNT 8
#define SYNC_OVERSAMPLE_QUALITY 6

struct Sync : Via<SYNC_OVERSAMPLE_AMOUNT, SYNC_OVERSAMPLE_QUALITY> {
    
    Sync() : Via() {

        virtualIO = &virtualModule;

        onSampleRateChange();
        presetData[0] = virtualModule.syncUI.stockPreset1;
        presetData[1] = virtualModule.syncUI.stockPreset2;
        presetData[2] = virtualModule.syncUI.stockPreset3;
        presetData[3] = virtualModule.syncUI.stockPreset4;
        presetData[4] = virtualModule.syncUI.stockPreset5;
        presetData[5] = virtualModule.syncUI.stockPreset6;
    }

    void step() override;

    ViaSync virtualModule;

    void onSampleRateChange() override {
        float sampleRate = engineGetSampleRate();

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

    json_t *toJson() override {

        json_t *rootJ = json_object();
        
        json_object_set_new(rootJ, "sync_modes", json_integer(virtualModule.syncUI.modeStateBuffer));
        
        return rootJ;
    }
    
    void fromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "sync_modes");
        virtualModule.syncUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.syncUI.loadFromEEPROM(0);
        virtualModule.syncUI.recallModuleState();


    }
    
};

void Sync::step() {

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
            int32_t trigButton = clamp((int32_t)params[TRIGBUTTON_PARAM].value, 0, 1);
            if (trigButton > lastTrigButton) {
                virtualModule.buttonPressedCallback();
            } else if (trigButton < lastTrigButton) {
                virtualModule.buttonReleasedCallback();
            } 
            lastTrigButton = trigButton;
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


struct Sync_Widget : ModuleWidget  {

    Sync_Widget(Sync *module) : ModuleWidget(module) {

        box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        {
            SVGPanel *panel = new SVGPanel();
            panel->box.size = box.size;
            panel->setBackground(SVG::load(assetPlugin(plugin, "res/sync.svg")));
            addChild(panel);
        }

        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(ParamWidget::create<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Sync::KNOB1_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Sync::KNOB2_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Sync::KNOB3_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Sync::B_PARAM, -1.0, 1.0, 1.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Sync::CV2AMT_PARAM, 0, 1.0, 1.0));
        addParam(ParamWidget::create<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Sync::A_PARAM, -5.0, 5.0, -5.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Sync::CV3AMT_PARAM, 0, 1.0, 1.0));
        
        addParam(ParamWidget::create<SH_Button>(Vec(7 + .753, 82), module, Sync::BUTTON1_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Up_Button>(Vec(48 + .753, 79.5), module, Sync::BUTTON2_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Freq_Button>(Vec(88.5 + .753, 82), module, Sync::BUTTON3_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Trig_Button>(Vec(7 + .753, 136.5), module, Sync::BUTTON4_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Down_Button>(Vec(48 + .753, 135.5), module, Sync::BUTTON5_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Loop_Button>(Vec(88.5 + .753, 136.5), module, Sync::BUTTON6_PARAM, 0.0, 1.0, 0.0));
        
        addParam(ParamWidget::create<VIA_manual_button>(Vec(132.7 + .753, 320), module, Sync::TRIGBUTTON_PARAM, 0.0, 5.0, 0.0));

        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 241.12), Port::INPUT, module, Sync::A_INPUT));
        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 282.62), Port::INPUT, module, Sync::B_INPUT));
        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 324.02), Port::INPUT, module, Sync::MAIN_LOGIC_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 241.12), Port::INPUT, module, Sync::CV1_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 282.62), Port::INPUT, module, Sync::CV2_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 324.02), Port::INPUT, module, Sync::CV3_INPUT));
        addInput(Port::create<ViaJack>(Vec(135 + 1.053, 282.62), Port::INPUT, module, Sync::AUX_LOGIC_INPUT));

        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 241.12), Port::OUTPUT, module, Sync::LOGICA_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 282.62), Port::OUTPUT, module, Sync::AUX_DAC_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 324.02), Port::OUTPUT, module, Sync::MAIN_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(135 + 1.053, 241.12), Port::OUTPUT, module, Sync::AUX_LOGIC_OUTPUT));

        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Sync::LED1_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.7 + .753, 268.5), module, Sync::LED2_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Sync::LED3_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.7 + .753, 309.9), module, Sync::LED4_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Sync::OUTPUT_GREEN_LIGHT));
        addChild(ModuleLightWidget::create<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Sync::RED_LIGHT));

        }

    void appendContextMenu(Menu *menu) override {
        Sync *module = dynamic_cast<Sync*>(this->module);
        assert(module);

        struct SyncAux1ModeHandler : MenuItem {
            Sync *module;
            int32_t mode;
            void onAction(EventAction &e) override {
                module->virtualModule.syncUI.aux1Mode = mode;
                module->virtualModule.handleAux1ModeChange(mode);
                module->virtualModule.syncUI.storeMode(module->virtualModule.syncUI.aux1Mode, AUX_MODE1_MASK, AUX_MODE1_SHIFT);
            }
            void step() override {
                rightText = (module->virtualModule.syncUI.aux1Mode  == mode) ? "✔" : "";
                MenuItem::step();
            }
        };

        menu->addChild(MenuEntry::create());
        menu->addChild(MenuLabel::create("Logic Out"));
        const std::string logicLabels[] = {
            "High during attack",
            "Delta"
        };
        for (int i = 0; i < (int) LENGTHOF(logicLabels); i++) {
            SyncAux1ModeHandler *aux1Item = MenuItem::create<SyncAux1ModeHandler>(logicLabels[i], CHECKMARK(module->virtualModule.syncUI.aux1Mode == i));
            aux1Item->module = module;
            aux1Item->mode = i;
            menu->addChild(aux1Item);
        }

        struct SyncAux2ModeHandler : MenuItem {
            Sync *module;
            int32_t mode;
            void onAction(EventAction &e) override {
                module->virtualModule.syncUI.aux2Mode = mode;
                module->virtualModule.handleAux2ModeChange(mode);
                module->virtualModule.syncUI.storeMode(module->virtualModule.syncUI.aux2Mode, AUX_MODE2_MASK, AUX_MODE2_SHIFT);
            }
            void step() override {
                rightText = (module->virtualModule.syncUI.aux2Mode  == mode) ? "✔" : "";
                MenuItem::step();
            }
        };

        menu->addChild(MenuLabel::create("Signal Out"));
        const std::string signalLabels[] = {
            "Triangle",
            "Contour"
        };
        for (int i = 0; i < (int) LENGTHOF(signalLabels); i++) {
            SyncAux2ModeHandler *aux2Item = MenuItem::create<SyncAux2ModeHandler>(signalLabels[i], CHECKMARK(module->virtualModule.syncUI.aux2Mode == i));
            aux2Item->module = module;
            aux2Item->mode = i;
            menu->addChild(aux2Item);
        }

        struct SyncAux3ModeHandler : MenuItem {
            Sync *module;
            int32_t mode;
            void onAction(EventAction &e) override {
                module->virtualModule.syncUI.aux3Mode = mode;
                module->virtualModule.handleAux3ModeChange(mode);
                module->virtualModule.syncUI.storeMode(module->virtualModule.syncUI.aux3Mode, AUX_MODE3_MASK, AUX_MODE3_SHIFT);
            }
            void step() override {
                rightText = (module->virtualModule.syncUI.aux3Mode  == mode) ? "✔" : "";
                MenuItem::step();
            }
        };

        menu->addChild(MenuLabel::create("Quadrature"));
        const std::string quadratureLabels[] = {
            "0 degrees",
            "90 degrees",
            "180 degrees",
            "270 degrees"
        };
        for (int i = 0; i < (int) LENGTHOF(quadratureLabels); i++) {
            SyncAux3ModeHandler *aux3Item = MenuItem::create<SyncAux3ModeHandler>(quadratureLabels[i], CHECKMARK(module->virtualModule.syncUI.aux3Mode == i));
            aux3Item->module = module;
            aux3Item->mode = i;
            menu->addChild(aux3Item);
        }

        struct SyncAux4ModeHandler : MenuItem {
            Sync *module;
            int32_t mode;
            void onAction(EventAction &e) override {
                module->virtualModule.syncUI.aux4Mode = mode;
                module->virtualModule.handleAux4ModeChange(mode);
                module->virtualModule.syncUI.storeMode(module->virtualModule.syncUI.aux4Mode, AUX_MODE4_MASK, AUX_MODE4_SHIFT);
            }
            void step() override {
                rightText = (module->virtualModule.syncUI.aux4Mode  == mode) ? "✔" : "";
                MenuItem::step();
            }

        };

        menu->addChild(MenuLabel::create("Wave Options"));
        const std::string auxLabels[] = {
            "Group-specific",
            "Aux"
        };
        for (int i = 0; i < (int) LENGTHOF(auxLabels); i++) {
            SyncAux4ModeHandler *aux4Item = MenuItem::create<SyncAux4ModeHandler>(auxLabels[i], CHECKMARK(module->virtualModule.syncUI.aux4Mode == i));
            aux4Item->module = module;
            aux4Item->mode = i;
            menu->addChild(aux4Item);
        }
        
        struct PresetRecallItem : MenuItem {
            Sync *module;
            int preset;
            void onAction(EventAction &e) override {
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
                    PresetRecallItem *item = MenuItem::create<PresetRecallItem>(presetLabels[i]);
                    item->module = module;
                    item->preset = module->presetData[i];
                    menu->addChild(item);
                }
                return menu;
            }
        };

        menu->addChild(MenuEntry::create());
        StockPresetItem *stockPresets = MenuItem::create<StockPresetItem>("Stock presets");
        stockPresets->module = module;
        menu->addChild(stockPresets);

    }

};


Model *modelSync = Model::create<Sync, Sync_Widget>(
        "Starling", "SYNC", "SYNC", OSCILLATOR_TAG, CLOCK_MODULATOR_TAG, CLOCK_TAG, LFO_TAG, RING_MODULATOR_TAG, DIGITAL_TAG);


