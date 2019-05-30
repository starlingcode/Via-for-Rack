#include "meta.hpp"
#include "via_module.hpp"

#define META_OVERSAMPLE_AMOUNT 8
#define META_OVERSAMPLE_QUALITY 6

struct Meta : Via<META_OVERSAMPLE_AMOUNT, META_OVERSAMPLE_QUALITY> {
    
    Meta() : Via() {
        
        virtualIO = &virtualModule;

        onSampleRateChange();
        presetData[0] = virtualModule.metaUI.stockPreset1;
        presetData[1] = virtualModule.metaUI.stockPreset2;
        presetData[2] = virtualModule.metaUI.stockPreset3;
        presetData[3] = virtualModule.metaUI.stockPreset4;
        presetData[4] = virtualModule.metaUI.stockPreset5;
        presetData[5] = virtualModule.metaUI.stockPreset6;
    
    }

    void step() override;

    ViaMeta virtualModule;

    void onSampleRateChange() override {
        float sampleRate = engineGetSampleRate();

        if (sampleRate == 44100.0) {
            divideAmount = 1;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
        } else if (sampleRate == 48000.0) {
            divideAmount = 1;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
        } else if (sampleRate == 88200.0) {
            divideAmount = 2;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
        } else if (sampleRate == 96000.0) {
            divideAmount = 2;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
        } else if (sampleRate == 176400.0) {
            divideAmount = 4;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
        } else if (sampleRate == 192000.0) {
            divideAmount = 4;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
        }
        
    }

    json_t *toJson() override {

        json_t *rootJ = json_object();
        
        // freq
        json_object_set_new(rootJ, "meta_modes", json_integer(virtualModule.metaUI.modeStateBuffer));

        return rootJ;
    }

    int32_t testMode;
    
    void fromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "meta_modes");
        virtualModule.metaUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.metaUI.loadFromEEPROM(0);
        virtualModule.metaUI.recallModuleState();

    }
    
};

void Meta::step() {

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
            virtualModule.metaUI.incrementTimer();
            // trigger handling
            int32_t trigButton = clamp((int32_t)params[TRIGBUTTON_PARAM].value, 0, 1);
            if (trigButton > lastTrigButton) {
                virtualModule.buttonPressedCallback();
            } else if (trigButton < lastTrigButton) {
                virtualModule.buttonReleasedCallback();
            }
            virtualModule.metaUI.trigButton = trigButton; 
            lastTrigButton = trigButton;
            virtualModule.blinkTimerCount += virtualModule.blinkTimerEnable;
            virtualModule.blankTimerCount += virtualModule.blankTimerEnable;
            if (virtualModule.blinkTimerCount > virtualModule.blinkTimerOverflow) {
                virtualModule.blinkTimerCount = 0;
                virtualModule.blinkTimerEnable = 0;
                virtualModule.blankTimerEnable = 1;
                virtualModule.auxTimer1InterruptCallback();
            }
            if (virtualModule.blankTimerCount > virtualModule.blankTimerOverflow) {
                virtualModule.blankTimerCount = 0;
                virtualModule.blankTimerEnable = 0;
                virtualModule.auxTimer2InterruptCallback();
            }
        }

        updateAudioRate();

    }
    
}


struct MetaWidget : ModuleWidget  {

    MetaWidget(Meta *module) : ModuleWidget(module) {

    	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    	{
    		SVGPanel *panel = new SVGPanel();
    		panel->box.size = box.size;
    		panel->setBackground(SVG::load(assetPlugin(plugin, "res/meta.svg")));
    		addChild(panel);
    	}

        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(ParamWidget::create<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Meta::KNOB1_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Meta::KNOB2_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Meta::KNOB3_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Meta::B_PARAM, -1.0, 1.0, 1.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Meta::CV2AMT_PARAM, 0, 1.0, 1.0));
        addParam(ParamWidget::create<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Meta::A_PARAM, -5.0, 5.0, -5.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Meta::CV3AMT_PARAM, 0, 1.0, 1.0));
        
        addParam(ParamWidget::create<SH_Button>(Vec(10.5 + .753, 80), module, Meta::BUTTON1_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Up_Button>(Vec(47 + .753, 77.5), module, Meta::BUTTON2_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Freq_Button>(Vec(85 + .753, 80), module, Meta::BUTTON3_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Trig_Button>(Vec(10.5 + .753, 129), module, Meta::BUTTON4_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Down_Button>(Vec(46 + .753, 131.5), module, Meta::BUTTON5_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Loop_Button>(Vec(85 + .753, 129), module, Meta::BUTTON6_PARAM, 0.0, 1.0, 0.0));
        
        addParam(ParamWidget::create<VIA_manual_button>(Vec(132.7 + .753, 320), module, Meta::TRIGBUTTON_PARAM, 0.0, 5.0, 0.0));

        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 241.12), Port::INPUT, module, Meta::A_INPUT));
        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 282.62), Port::INPUT, module, Meta::B_INPUT));
        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 324.02), Port::INPUT, module, Meta::MAIN_LOGIC_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 241.12), Port::INPUT, module, Meta::CV1_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 282.62), Port::INPUT, module, Meta::CV2_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 324.02), Port::INPUT, module, Meta::CV3_INPUT));
        addInput(Port::create<ViaJack>(Vec(135 + 1.053, 282.62), Port::INPUT, module, Meta::AUX_LOGIC_INPUT));

        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 241.12), Port::OUTPUT, module, Meta::LOGICA_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 282.62), Port::OUTPUT, module, Meta::AUX_DAC_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 324.02), Port::OUTPUT, module, Meta::MAIN_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(135 + 1.053, 241.12), Port::OUTPUT, module, Meta::AUX_LOGIC_OUTPUT));

        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Meta::LED1_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.1 + .753, 268.5), module, Meta::LED2_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Meta::LED3_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.1 + .753, 309.9), module, Meta::LED4_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Meta::OUTPUT_GREEN_LIGHT));
        addChild(ModuleLightWidget::create<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Meta::RED_LIGHT));

        }

    void appendContextMenu(Menu *menu) override {
        Meta *module = dynamic_cast<Meta*>(this->module);

        struct MetaAux2ModeHandler : MenuItem {
            Meta *module;
            int32_t logicMode;
            void onAction(EventAction &e) override {
                module->virtualModule.metaUI.aux2Mode = logicMode;
                module->virtualModule.handleAux2ModeChange(logicMode);
                module->virtualModule.metaUI.storeMode(module->virtualModule.metaUI.aux2Mode, AUX_MODE2_MASK, AUX_MODE2_SHIFT);

            }
        };

        menu->addChild(MenuEntry::create());
        menu->addChild(MenuLabel::create("Logic out"));
        const std::string logicLabels[] = {
            "High during release",
            "High during attack",
        };
        for (int i = 0; i < (int) LENGTHOF(logicLabels); i++) {
            MetaAux2ModeHandler *aux2Item = MenuItem::create<MetaAux2ModeHandler>(logicLabels[i], CHECKMARK(module->virtualModule.metaUI.aux2Mode == i));
            aux2Item->module = module;
            aux2Item->logicMode = i;
            menu->addChild(aux2Item);
        }

        struct MetaAux4ModeHandler : MenuItem {
            Meta *module;
            int32_t signalMode;
            void onAction(EventAction &e) override {
                module->virtualModule.metaUI.aux4Mode = signalMode;
                module->virtualModule.handleAux4ModeChange(signalMode);
                module->virtualModule.metaUI.storeMode(module->virtualModule.metaUI.aux4Mode, AUX_MODE4_MASK, AUX_MODE4_SHIFT);
            }
        };

        menu->addChild(MenuLabel::create("Signal out"));
        const std::string signalLabels[] = {
            "Triangle",
            "Contour",
        };
        for (int i = 0; i < (int) LENGTHOF(signalLabels); i++) {
            MetaAux4ModeHandler *aux4Item = MenuItem::create<MetaAux4ModeHandler>(signalLabels[i], CHECKMARK(module->virtualModule.metaUI.aux4Mode == i));
            aux4Item->module = module;
            aux4Item->signalMode = i;
            menu->addChild(aux4Item);
        }

        struct MetaAux1ModeHandler : MenuItem {
            Meta *module;
            int32_t drumMode;
            void onAction(EventAction &e) override {
                module->virtualModule.metaUI.aux1Mode = drumMode;
                module->virtualModule.metaUI.storeMode(module->virtualModule.metaUI.aux1Mode, AUX_MODE1_MASK, AUX_MODE1_SHIFT);
                if ((module->virtualModule.metaUI.button3Mode | module->virtualModule.metaUI.button6Mode) == 0) {
                    module->virtualModule.handleAux1ModeChange(drumMode);
                }
            }
        };

        menu->addChild(MenuLabel::create("Drum signal out"));
        const std::string drumOutLabels[] = {
            "Triangle",
            "Contour",
            "Envelope",
            "Noise"
        };
        for (int i = 0; i < (int) LENGTHOF(drumOutLabels); i++) {
            MetaAux1ModeHandler *aux1Item = MenuItem::create<MetaAux1ModeHandler>(drumOutLabels[i], CHECKMARK(module->virtualModule.metaUI.aux1Mode == i));
            aux1Item->module = module;
            aux1Item->drumMode = i;
            menu->addChild(aux1Item);
        }


        struct MetaTuneC4 : MenuItem {
            Meta *module;
            ModuleWidget *moduleWidget;

            int32_t mode;
            void onAction(EventAction &e) override {

                int32_t audioOsc = !(module->virtualModule.metaUI.button3Mode) && module->virtualModule.metaUI.button6Mode;
                int32_t drumVoice = !(module->virtualModule.metaUI.button3Mode) && !(module->virtualModule.metaUI.button6Mode);

                if (audioOsc) {
                    moduleWidget->params[Meta::KNOB1_PARAM]->setValue(2048.f);
                    moduleWidget->params[Meta::KNOB2_PARAM]->setValue(0.f);
                } else if (drumVoice) {
                    moduleWidget->params[Meta::KNOB1_PARAM]->setValue(4095.f);
                }

            }
        };

        menu->addChild(MenuEntry::create());
        MetaTuneC4 *tuneC4 = new MetaTuneC4();
        tuneC4->text = "Tune to C4";
        tuneC4->module = module;
        tuneC4->moduleWidget = this;
        menu->addChild(tuneC4);

        struct PresetRecallItem : MenuItem {
            Meta *module;
            int preset;
            void onAction(EventAction &e) override {
                module->virtualModule.metaUI.modeStateBuffer = preset;
                module->virtualModule.metaUI.loadFromEEPROM(0);
                module->virtualModule.metaUI.recallModuleState();
            }
        };

        struct StockPresetItem : MenuItem {
            Meta *module;
            Menu *createChildMenu() override {
                Menu *menu = new Menu();
                const std::string presetLabels[] = {
                    "Drum",
                    "Oscillator",
                    "AR Envelope",
                    "Looping AR",
                    "Modulation Sequence",
                    "Complex LFO",
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


Model *modelMeta = Model::create<Meta, MetaWidget>(
        "Starling", "META", "META", OSCILLATOR_TAG, FUNCTION_GENERATOR_TAG, ENVELOPE_GENERATOR_TAG, DRUM_TAG, SYNTH_VOICE_TAG, LFO_TAG, RING_MODULATOR_TAG, DIGITAL_TAG);

