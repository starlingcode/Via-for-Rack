#include "gateseq.hpp"
#include "via_module.hpp"

#define GATESEQ_OVERSAMPLE_AMOUNT 1
#define GATESEQ_OVERSAMPLE_QUALITY 1

struct Gateseq : Via<GATESEQ_OVERSAMPLE_AMOUNT, GATESEQ_OVERSAMPLE_QUALITY>  {
    
    Gateseq() : Via() {

        virtualIO = &virtualModule;

        onSampleRateChange();
        presetData[0] = virtualModule.gateseqUI.stockPreset1;
        presetData[1] = virtualModule.gateseqUI.stockPreset2;
        presetData[2] = virtualModule.gateseqUI.stockPreset3;
        presetData[3] = virtualModule.gateseqUI.stockPreset4;
        presetData[4] = virtualModule.gateseqUI.stockPreset5;
        presetData[5] = virtualModule.gateseqUI.stockPreset6;
    }
    void step() override;

    ViaGateseq virtualModule;

    void onSampleRateChange() override {

        float sampleRate = engineGetSampleRate();

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
        }
        
    }

    json_t *toJson() override {

        json_t *rootJ = json_object();
        
        json_object_set_new(rootJ, "gateseq_modes", json_integer(virtualModule.gateseqUI.modeStateBuffer));
        json_object_set_new(rootJ, "logic_mode", json_integer((int) virtualModule.gateseqUI.aux2Mode));
     
        return rootJ;
    }
    
    void fromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "gateseq_modes");
        virtualModule.gateseqUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.gateseqUI.loadFromEEPROM(0);
        virtualModule.gateseqUI.recallModuleState();


    }
    
};

void Gateseq::step() {

    // update the "slow IO" (not audio rate) every 16 samples
    // needs to scale with sample rate somehow
    slowIOPrescaler++;
    if (slowIOPrescaler == 16) {
        slowIOPrescaler = 0;
        updateSlowIO();
        virtualModule.slowConversionCallback();
        virtualModule.ui_dispatch(SENSOR_EVENT_SIG);
        virtualModule.gateseqUI.incrementTimer();
        // trigger handling
        int32_t trigButton = clamp((int32_t)params[TRIGBUTTON_PARAM].value, 0, 1);
        if (trigButton > lastTrigButton) {
            virtualModule.buttonPressedCallback();
        } else if (trigButton < lastTrigButton) {
            virtualModule.buttonReleasedCallback();
        } 
        lastTrigButton = trigButton;
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

    updateAudioRate();
    
}

struct GateseqWidget : ModuleWidget  {

    GateseqWidget(Gateseq *module) : ModuleWidget(module) {

        box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        {
            SVGPanel *panel = new SVGPanel();
            panel->box.size = box.size;
            panel->setBackground(SVG::load(assetPlugin(plugin, "res/gateseq.svg")));
            addChild(panel);
        }
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(ParamWidget::create<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Gateseq::KNOB1_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Gateseq::KNOB2_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Gateseq::KNOB3_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Gateseq::B_PARAM, -1.0, 1.0, 1.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Gateseq::CV2AMT_PARAM, 0, 1.0, 1.0));
        addParam(ParamWidget::create<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Gateseq::A_PARAM, -5.0, 5.0, -5.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Gateseq::CV3AMT_PARAM, 0, 1.0, 1.0));
        
        addParam(ParamWidget::create<SH_Button>(Vec(8 + .753, 85), module, Gateseq::BUTTON1_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Up_Button>(Vec(48 + .753, 85), module, Gateseq::BUTTON2_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Freq_Button>(Vec(86 + .753, 85), module, Gateseq::BUTTON3_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Trig_Button>(Vec(8 + .753, 139), module, Gateseq::BUTTON4_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Down_Button>(Vec(48 + .753, 139), module, Gateseq::BUTTON5_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Loop_Button>(Vec(86 + .753, 139), module, Gateseq::BUTTON6_PARAM, 0.0, 1.0, 0.0));
        
        addParam(ParamWidget::create<VIA_manual_button>(Vec(132.7 + .753, 320), module, Gateseq::TRIGBUTTON_PARAM, 0.0, 5.0, 0.0));

        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 241.12), Port::INPUT, module, Gateseq::A_INPUT));
        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 282.62), Port::INPUT, module, Gateseq::B_INPUT));
        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 324.02), Port::INPUT, module, Gateseq::MAIN_LOGIC_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 241.12), Port::INPUT, module, Gateseq::CV1_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 282.62), Port::INPUT, module, Gateseq::CV2_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 324.02), Port::INPUT, module, Gateseq::CV3_INPUT));
        addInput(Port::create<ViaJack>(Vec(135 + 1.053, 282.62), Port::INPUT, module, Gateseq::AUX_LOGIC_INPUT));

        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 241.12), Port::OUTPUT, module, Gateseq::LOGICA_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 282.62), Port::OUTPUT, module, Gateseq::AUX_DAC_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 324.02), Port::OUTPUT, module, Gateseq::MAIN_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(135 + 1.053, 241.12), Port::OUTPUT, module, Gateseq::AUX_LOGIC_OUTPUT));

        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Gateseq::LED1_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.7 + .753, 268.5), module, Gateseq::LED2_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Gateseq::LED3_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.7 + .753, 309.9), module, Gateseq::LED4_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Gateseq::OUTPUT_GREEN_LIGHT));
        addChild(ModuleLightWidget::create<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Gateseq::RED_LIGHT));

        }

    void appendContextMenu(Menu *menu) override {
        Gateseq *module = dynamic_cast<Gateseq*>(this->module);
        assert(module);

        struct GateseqAux2ModeHandler : MenuItem {
            Gateseq *module;
            int32_t mode;
            void onAction(EventAction &e) override {
                module->virtualModule.gateseqUI.aux2Mode = mode;
                module->virtualModule.gateseqUI.storeMode(module->virtualModule.gateseqUI.aux2Mode, AUX_MODE2_MASK, AUX_MODE2_SHIFT);
                module->virtualModule.handleAux2ModeChange(mode);
            }
        };

        menu->addChild(MenuEntry::create());
        menu->addChild(MenuLabel::create("Drum signal out"));
        const std::string logicLabels[] = {
            "And",
            "Or",
            "Xor",
            "Nor"
        };
        for (int i = 0; i < (int) LENGTHOF(logicLabels); i++) {
            GateseqAux2ModeHandler *aux2Item = MenuItem::create<GateseqAux2ModeHandler>(logicLabels[i], CHECKMARK(module->virtualModule.gateseqUI.aux2Mode == i));
            aux2Item->module = module;
            aux2Item->mode = i;
            menu->addChild(aux2Item);
        }

        struct PresetRecallItem : MenuItem {
            Gateseq *module;
            int preset;
            void onAction(EventAction &e) override {
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



Model *modelGateseq = Model::create<Gateseq, GateseqWidget>("GATESEQ");


