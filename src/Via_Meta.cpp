#include "meta.hpp"
#include "Via_Graphics.hpp"


struct Via_Meta : Module {
    
    
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
    
    Via_Meta() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    ViaMeta virtualModule;
    
    int32_t lastTrigInput;
    int32_t lastAuxTrigInput;
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

    void updateSlowIO(void) {

        virtualModule.button1Input = (int32_t) params[BUTTON1_PARAM].value;
        virtualModule.button2Input = (int32_t) params[BUTTON2_PARAM].value;
        virtualModule.button3Input = (int32_t) params[BUTTON3_PARAM].value;
        virtualModule.button4Input = (int32_t) params[BUTTON4_PARAM].value;
        virtualModule.button5Input = (int32_t) params[BUTTON5_PARAM].value;
        virtualModule.button6Input = (int32_t) params[BUTTON6_PARAM].value;

        // these have a janky array ordering to correspond with the DMA stream on the hardware
        virtualModule.controls.controlRateInputs[2] = (int32_t) params[KNOB1_PARAM].value;
        virtualModule.controls.controlRateInputs[3] = (int32_t) params[KNOB2_PARAM].value;
        virtualModule.controls.controlRateInputs[1] = (int32_t) params[KNOB3_PARAM].value;
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

    // json_t *toJson() override {
    //     json_t *rootJ = json_object();
        
    //     // freq
    //     json_object_set_new(rootJ, "freq", json_integer(freqMode));
        
    //     // loop
    //     json_object_set_new(rootJ, "loop", json_integer(loopMode));
        
    //     // trig
    //     json_object_set_new(rootJ, "trig", json_integer(trigMode));
        
    //     // SH
    //     json_object_set_new(rootJ, "sampleHold", json_integer(sampleHoldMode));
        
    //     // familyIndicator
    //     json_object_set_new(rootJ, "family", json_integer(familyIndicator));
        
    //     // flagWord
    //     json_object_set_new(rootJ, "flagWord", json_integer(flagHolder));
        
    //     // flagWord
    //     json_object_set_new(rootJ, "position", json_integer(position));
        
    //     return rootJ;
    // }
    
    // void fromJson(json_t *rootJ) override {
    //     json_t *freqJ = json_object_get(rootJ, "freq");
    //     freqMode = json_integer_value(freqJ);
        
    //     json_t *loopJ = json_object_get(rootJ, "loop");
    //     loopMode = json_integer_value(loopJ);
        
    //     json_t *trigJ = json_object_get(rootJ, "trig");
    //     trigMode = json_integer_value(trigJ);
        
    //     json_t *sampleHoldJ = json_object_get(rootJ, "sampleHold");
    //     sampleHoldMode = json_integer_value(sampleHoldJ);
        
    //     json_t *familyJ = json_object_get(rootJ, "family");
    //     familyIndicator = json_integer_value(familyJ);
        
    //     json_t *flagWordJ = json_object_get(rootJ, "flagWord");
    //     flagHolder = json_integer_value(flagWordJ);
        
    //     json_t *positionJ = json_object_get(rootJ, "position");
    //     position = json_integer_value(positionJ);
        
    // }
    
};

void Via_Meta::step() {

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
            virtualModule.mainRisingEdgeCallback();
        } else if (trigButton < lastTrigButton) {
            virtualModule.mainFallingEdgeCallback();
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

    int32_t trigInput = clamp((int32_t)inputs[MAIN_LOGIC_INPUT].value, 0, 1);
    if (trigInput > lastTrigInput) {
        virtualModule.mainRisingEdgeCallback();
    } else if (trigInput < lastTrigInput) {
        virtualModule.mainFallingEdgeCallback();
    }
    lastTrigInput = trigInput; 

    int32_t auxTrigInput = clamp((int32_t)inputs[AUX_LOGIC_INPUT].value, 0, 1);
    if (auxTrigInput > lastAuxTrigInput) {
        virtualModule.auxRisingEdgeCallback();
    } else if (auxTrigInput < lastAuxTrigInput) {
        virtualModule.auxFallingEdgeCallback();
    }
    lastAuxTrigInput = auxTrigInput; 



    // buffer length of 1 ..
    float dac1Sample = (float) virtualModule.outputs.dac1Samples[0];
    float dac2Sample = (float) virtualModule.outputs.dac2Samples[0];
    float dac3Sample = (float) virtualModule.outputs.dac3Samples[0];
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
    
}


struct Via_Meta_Widget : ModuleWidget {
    Via_Meta_Widget(Via_Meta *module);
};


Via_Meta_Widget::Via_Meta_Widget(Via_Meta *module) : ModuleWidget(module) {
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

	addParam(ParamWidget::create<ViaSifamBlack>(Vec(9.022, 30.90), module, Via_Meta::KNOB1_PARAM, 0, 4095.0, 0.0));
    addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53, 30.90), module, Via_Meta::KNOB2_PARAM, 0, 4095.0, 0.0));
    addParam(ParamWidget::create<ViaSifamBlack>(Vec(68.53, 169.89), module, Via_Meta::KNOB3_PARAM, 0, 4095.0, 0.0));
    addParam(ParamWidget::create<ViaSifamGrey>(Vec(9.022, 169.89), module, Via_Meta::B_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<ViaSifamBlack>(Vec(128.04, 30.90), module, Via_Meta::CV2AMT_PARAM, 0, 1.0, 0.0));
    addParam(ParamWidget::create<ViaSifamGrey>(Vec(128.04, 100.4), module, Via_Meta::A_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<ViaSifamBlack>(Vec(132.5, 169.89), module, Via_Meta::CV3AMT_PARAM, 0, 1.0, 0.0));
    
    addParam(ParamWidget::create<SH_Button>(Vec(10.5, 80), module, Via_Meta::BUTTON1_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Up_Button>(Vec(47, 77.5), module, Via_Meta::BUTTON2_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Freq_Button>(Vec(85, 80), module, Via_Meta::BUTTON3_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Trig_Button>(Vec(10.5, 129), module, Via_Meta::BUTTON4_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Down_Button>(Vec(46, 131.5), module, Via_Meta::BUTTON5_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Loop_Button>(Vec(85, 129), module, Via_Meta::BUTTON6_PARAM, 0.0, 1.0, 0.0));
    
    addParam(ParamWidget::create<VIA_manual_button>(Vec(133.5, 320), module, Via_Meta::TRIGBUTTON_PARAM, 0.0, 5.0, 0.0));


	addInput(Port::create<ViaJack>(Vec(8.07, 241.22), Port::INPUT, module, Via_Meta::A_INPUT));
    addInput(Port::create<ViaJack>(Vec(8.07, 282.62), Port::INPUT, module, Via_Meta::B_INPUT));
    addInput(Port::create<ViaJack>(Vec(8.07, 324.02), Port::INPUT, module, Via_Meta::MAIN_LOGIC_INPUT));
    addInput(Port::create<ViaJack>(Vec(45.55, 241.22), Port::INPUT, module, Via_Meta::CV1_INPUT));
    addInput(Port::create<ViaJack>(Vec(45.55, 282.62), Port::INPUT, module, Via_Meta::CV2_INPUT));
    addInput(Port::create<ViaJack>(Vec(45.55, 324.02), Port::INPUT, module, Via_Meta::CV3_INPUT));
    addInput(Port::create<ViaJack>(Vec(134.8, 282.62), Port::INPUT, module, Via_Meta::AUX_LOGIC_INPUT));

	addOutput(Port::create<ViaJack>(Vec(83.28, 241.22), Port::OUTPUT, module, Via_Meta::LOGICA_OUTPUT));
    addOutput(Port::create<ViaJack>(Vec(83.28, 282.62), Port::OUTPUT, module, Via_Meta::AUX_DAC_OUTPUT));
    addOutput(Port::create<ViaJack>(Vec(83.28, 324.02), Port::OUTPUT, module, Via_Meta::MAIN_OUTPUT));
    addOutput(Port::create<ViaJack>(Vec(134.8, 241.22), Port::OUTPUT, module, Via_Meta::AUX_LOGIC_OUTPUT));

	addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.7, 268.6), module, Via_Meta::LED1_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.4, 268.6), module, Via_Meta::LED2_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.7, 309.9), module, Via_Meta::LED3_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(73.4, 309.9), module, Via_Meta::LED4_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(54.8, 179.6), module, Via_Meta::OUTPUT_GREEN_LIGHT));
    addChild(ModuleLightWidget::create<LargeLight<RGBTriangle>>(Vec(59, 221), module, Via_Meta::RED_LIGHT));
}


Model *modelVia_Meta = Model::create<Via_Meta, Via_Meta_Widget>(
        "Starling", "Via_Meta", "Via_Meta", OSCILLATOR_TAG);



















///////////////////////// TABLE CLASS ///////////////////////////////




