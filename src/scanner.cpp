#include "scanner.hpp"
#include "via_ui.hpp"
#include "dsp/decimator.hpp"
#include "dsp/digital.hpp"


struct Scanner : Module {
    
    
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
    
    Scanner() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
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

    uint32_t presetData[6];

    SchmittTrigger mainLogic;
    SchmittTrigger auxLogic;

    bool lastTrigState = false;
    bool lastAuxTrigState = false;

    int32_t lastTrigButton;

    int32_t dacReadIndex = 0;
    int32_t adcWriteIndex = 0;
    int32_t slowIOPrescaler = 0;

    int32_t ledAState = 0;
    int32_t ledBState = 0;
    int32_t ledCState = 0;
    int32_t ledDState = 0;

    int32_t logicAState = 0;
    int32_t auxLogicState = 0;

    int32_t shAControl = 0;
    int32_t shBControl = 0;

    float shALast = 0;
    float shBLast = 0;

    float aSample = 0;
    float bSample = 0;

    int32_t clockDivider = 0;

    int32_t divideAmount = 1;

    void onSampleRateChange() override {
        float sampleRate = engineGetSampleRate();

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
        }
        
    }

    #define SCANNER_OVERSAMPLE_AMOUNT 8
    #define SCANNER_OVERSAMPLE_QUALITY 6

    float dac1DecimatorBuffer[8];
    float dac2DecimatorBuffer[8];
    float dac3DecimatorBuffer[8];

    Decimator<SCANNER_OVERSAMPLE_AMOUNT, SCANNER_OVERSAMPLE_QUALITY> dac1Decimator;
    Decimator<SCANNER_OVERSAMPLE_AMOUNT, SCANNER_OVERSAMPLE_QUALITY> dac2Decimator;
    Decimator<SCANNER_OVERSAMPLE_AMOUNT, SCANNER_OVERSAMPLE_QUALITY> dac3Decimator;

    void updateSlowIO(void) {

        virtualModule.button1Input = (int32_t) params[BUTTON1_PARAM].value;
        virtualModule.button2Input = (int32_t) params[BUTTON2_PARAM].value;
        virtualModule.button3Input = (int32_t) params[BUTTON3_PARAM].value;
        virtualModule.button4Input = (int32_t) params[BUTTON4_PARAM].value;
        virtualModule.button5Input = (int32_t) params[BUTTON5_PARAM].value;
        virtualModule.button6Input = (int32_t) params[BUTTON6_PARAM].value;

        // these have a janky array ordering to correspond with the DMA stream on the hardware
        virtualModule.controls.controlRateInputs[2] = clamp((int32_t) params[KNOB1_PARAM].value, 1, 4095);
        virtualModule.controls.controlRateInputs[3] = clamp((int32_t) params[KNOB2_PARAM].value, 1, 4095);
        virtualModule.controls.controlRateInputs[1] = clamp((int32_t) params[KNOB3_PARAM].value, 1, 4095);
        // model the the 1v/oct input, scale 10.6666666 volts 12 bit adc range
        // it the gain scaling stage is inverting
        float cv1Conversion = -inputs[CV1_INPUT].value;
        // ultimately we want a volt to be a chage of 384 in the adc reading
        cv1Conversion = cv1Conversion * 384.0;
        // offset to unipolar
        cv1Conversion += 2048.0;
        // clip at rails of the input opamp
        virtualModule.controls.controlRateInputs[0] = clamp((int32_t)cv1Conversion, 0, 4095);
    }

    // 2 sets the "GPIO" high, 1 sets it low, 0 is a no-op
    inline int32_t virtualLogicOut(int32_t logicOut, int32_t control) {
        return clamp(logicOut + (control & 2) - (control & 1), 0, 1);
    }

    void updateLEDs(void) {

        // the A B C D enumeration of the LEDs in the Via library makes little to no sense 
        // but its woven pretty deep so is a nagging style thing to fix

        ledAState = virtualLogicOut(ledAState, virtualModule.ledAOutput);
        ledBState = virtualLogicOut(ledBState, virtualModule.ledBOutput);
        ledCState = virtualLogicOut(ledCState, virtualModule.ledCOutput);
        ledDState = virtualLogicOut(ledDState, virtualModule.ledDOutput);

        lights[LED1_LIGHT].setBrightnessSmooth(ledAState, 2);
        lights[LED3_LIGHT].setBrightnessSmooth(ledBState, 2);
        lights[LED2_LIGHT].setBrightnessSmooth(ledCState, 2);
        lights[LED4_LIGHT].setBrightnessSmooth(ledDState, 2);

        lights[RED_LIGHT].setBrightnessSmooth(virtualModule.redLevel/4095.0, 2);
        lights[GREEN_LIGHT].setBrightnessSmooth(virtualModule.greenLevel/4095.0, 2);
        lights[BLUE_LIGHT].setBrightnessSmooth(virtualModule.blueLevel/4095.0, 2);

        float output = outputs[MAIN_OUTPUT].value/8.0;
        lights[OUTPUT_RED_LIGHT].setBrightnessSmooth(clamp(-output, 0.0, 1.0));
        lights[OUTPUT_GREEN_LIGHT].setBrightnessSmooth(clamp(output, 0.0, 1.0));

    }

    void updateLogicOutputs(void) {
        logicAState = virtualLogicOut(logicAState, virtualModule.aLogicOutput);
        auxLogicState = virtualLogicOut(auxLogicState, virtualModule.auxLogicOutput);
        shAControl = virtualLogicOut(shAControl, virtualModule.shAOutput);
        shBControl = virtualLogicOut(shBControl, virtualModule.shBOutput);
    }

    json_t *toJson() override {

        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "scanner_modes", json_integer(virtualModule.scannerUI.modeStateBuffer));
        
        return rootJ;
    }
    
    void fromJson(json_t *rootJ) override {

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
            // trigger handling
            int32_t trigButton = clamp((int32_t)params[TRIGBUTTON_PARAM].value, 0, 1);
            if (trigButton > lastTrigButton) {
                virtualModule.buttonPressedCallback();
            } else if (trigButton < lastTrigButton) {
                virtualModule.buttonReleasedCallback();
            } 
            lastTrigButton = trigButton;
        }

        // manage audio rate dacs and adcs

        // scale -5 - 5 V to -1 to 1 and then convert to 16 bit int;
        float cv2Scale = (32767.0 * clamp(-inputs[CV2_INPUT].value/5, -1.0, 1.0)) * params[CV2AMT_PARAM].value;
        float cv3Scale = (32767.0 * clamp(-inputs[CV3_INPUT].value/5, -1.0, 1.0)) * params[CV3AMT_PARAM].value;
        int16_t cv2Conversion = (int16_t) cv2Scale;
        int16_t cv3Conversion = (int16_t) cv3Scale;

        // no ADC buffer for now..
        virtualModule.inputs.cv2Samples[0] = cv2Conversion;
        virtualModule.inputs.cv3Samples[0] = cv3Conversion;

        // trigger handling

        mainLogic.process(rescale(inputs[MAIN_LOGIC_INPUT].value, .2, 1.2, 0.f, 1.f));
        bool trigState = mainLogic.isHigh();
        if (trigState && !lastTrigState) {
            virtualModule.mainRisingEdgeCallback();
        } else if (!trigState && lastTrigState) {
            virtualModule.mainFallingEdgeCallback();
        }
        lastTrigState = trigState; 

        auxLogic.process(rescale(inputs[AUX_LOGIC_INPUT].value, .2, 1.2, 0.f, 1.f));
        bool auxTrigState = auxLogic.isHigh();
        if (auxTrigState && !lastAuxTrigState) {
            virtualModule.auxRisingEdgeCallback();
        } else if (!auxTrigState && lastAuxTrigState) {
            virtualModule.auxFallingEdgeCallback();
        }
        lastAuxTrigState = auxTrigState; 

        int32_t samplesRemaining = 8;
        int32_t writeIndex = 0;

        // convert to float and downsample

        while (samplesRemaining) {

            dac1DecimatorBuffer[writeIndex] = (float) virtualModule.outputs.dac1Samples[writeIndex];
            dac2DecimatorBuffer[writeIndex] = (float) virtualModule.outputs.dac2Samples[writeIndex];
            dac3DecimatorBuffer[writeIndex] = (float) virtualModule.outputs.dac3Samples[writeIndex];

            samplesRemaining--;
            writeIndex ++;

        }

        float dac1Sample = dac1Decimator.process(dac1DecimatorBuffer);
        float dac2Sample = dac2Decimator.process(dac2DecimatorBuffer);
        float dac3Sample = dac3Decimator.process(dac3DecimatorBuffer);
        updateLogicOutputs();
        virtualModule.halfTransferCallback();


        // "model" the circuit
        // A and B inputs with normalled reference voltages
        float aIn = inputs[A_INPUT].value + (!inputs[A_INPUT].active) * params[A_PARAM].value;
        float bIn = (inputs[B_INPUT].active) * ((inputs[B_INPUT].value) * (params[B_PARAM].value)) + (!inputs[B_INPUT].active) * (5* (params[B_PARAM].value));
        
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
        outputs[MAIN_OUTPUT].value = bIn*(dac2Sample/4095.0) + aIn*(dac1Sample/4095.0); 
        outputs[AUX_DAC_OUTPUT].value = (dac3Sample/4095.0 - .5) * -10.666666666;
        outputs[LOGICA_OUTPUT].value = logicAState * 5.0;
        outputs[AUX_LOGIC_OUTPUT].value = auxLogicState * 5.0;

        updateLEDs();

        clockDivider = 0;

    }
    
}

struct ScannerWidget : ModuleWidget  {

    ScannerWidget(Scanner *module) : ModuleWidget(module) {

	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/scanner.svg")));
		addChild(panel);
	}

        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(ParamWidget::create<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Scanner::KNOB1_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Scanner::KNOB2_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Scanner::KNOB3_PARAM, 0, 4095.0, 2048.0));
        addParam(ParamWidget::create<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Scanner::B_PARAM, -1.0, 1.0, 1.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Scanner::CV2AMT_PARAM, 0, 1.0, 1.0));
        addParam(ParamWidget::create<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Scanner::A_PARAM, -5.0, 5.0, -5.0));
        addParam(ParamWidget::create<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Scanner::CV3AMT_PARAM, 0, 1.0, 1.0));
        
        addParam(ParamWidget::create<SH_Button>(Vec(21 + .753, 105), module, Scanner::BUTTON4_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Up_Button>(Vec(47 + .753, 77.5), module, Scanner::BUTTON2_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Freq_Button>(Vec(75 + .753, 105), module, Scanner::BUTTON6_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Trig_Button>(Vec(7 + .753, 142), module, Scanner::BUTTON1_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Down_Button>(Vec(47 + .753, 131.5), module, Scanner::BUTTON5_PARAM, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<Loop_Button>(Vec(89 + .753, 142), module, Scanner::BUTTON3_PARAM, 0.0, 1.0, 0.0));
        
        addParam(ParamWidget::create<VIA_manual_button>(Vec(132.7 + .753, 320), module, Scanner::TRIGBUTTON_PARAM, 0.0, 5.0, 0.0));


        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 241.12), Port::INPUT, module, Scanner::A_INPUT));
        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 282.62), Port::INPUT, module, Scanner::B_INPUT));
        addInput(Port::create<ViaJack>(Vec(8.07 + 1.053, 324.02), Port::INPUT, module, Scanner::MAIN_LOGIC_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 241.12), Port::INPUT, module, Scanner::CV1_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 282.62), Port::INPUT, module, Scanner::CV2_INPUT));
        addInput(Port::create<ViaJack>(Vec(45.75 + 1.053, 324.02), Port::INPUT, module, Scanner::CV3_INPUT));
        addInput(Port::create<ViaJack>(Vec(135 + 1.053, 282.62), Port::INPUT, module, Scanner::AUX_LOGIC_INPUT));

        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 241.12), Port::OUTPUT, module, Scanner::LOGICA_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 282.62), Port::OUTPUT, module, Scanner::AUX_DAC_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(83.68 + 1.053, 324.02), Port::OUTPUT, module, Scanner::MAIN_OUTPUT));
        addOutput(Port::create<ViaJack>(Vec(135 + 1.053, 241.12), Port::OUTPUT, module, Scanner::AUX_LOGIC_OUTPUT));

        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Scanner::LED1_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.7 + .753, 268.5), module, Scanner::LED2_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Scanner::LED3_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.7 + .753, 309.9), module, Scanner::LED4_LIGHT));
        addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Scanner::OUTPUT_GREEN_LIGHT));
        addChild(ModuleLightWidget::create<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Scanner::RED_LIGHT));

    };

    void appendContextMenu(Menu *menu) override {
        Scanner *module = dynamic_cast<Scanner*>(this->module);

        struct PresetRecallItem : MenuItem {
            Scanner *module;
            int preset;
            void onAction(EventAction &e) override {
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




Model *modelScanner = Model::create<Scanner, ScannerWidget>(
        "Starling", "SCANNER", "SCANNER", WAVESHAPER_TAG, DISTORTION_TAG, SAMPLE_AND_HOLD_TAG);


