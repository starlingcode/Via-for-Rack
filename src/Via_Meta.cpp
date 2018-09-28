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
        RED_LIGHT,
        GREEN_LIGHT,
        BLUE_LIGHT,
        PURPLE_LIGHT,
        NUM_LIGHTS
    };
    
    Via_Meta() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
    
    SchmittTrigger trigButton;
    SchmittTrigger trigInput;
    SchmittTrigger freezeInput;

    ViaMeta virtualModule;

    int32_t dacReadIndex = 0;
    int32_t adcWriteIndex = 0;
    int32_t slowIOPrescaler = 0;

    float shALast;
    float shBLast;

    float aSample;
    float bSample;


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

    void updateLEDs(void) {

        // the A B C D enumeration of the LEDs in the Via library makes little to no sense 
        // but its woven pretty deep so is a nagging style thing to fix

        lights[LED1_LIGHT].setBrightnessSmooth(virtualModule.ledAOutput);
        lights[LED3_LIGHT].setBrightnessSmooth(virtualModule.ledBOutput);
        lights[LED2_LIGHT].setBrightnessSmooth(virtualModule.ledCOutput);
        lights[LED4_LIGHT].setBrightnessSmooth(virtualModule.ledDOutput);

        lights[RED_LIGHT].setBrightnessSmooth(virtualModule.redLevel/4095.0);
        lights[GREEN_LIGHT].setBrightnessSmooth(virtualModule.blueLevel/4095.0);
        lights[BLUE_LIGHT].setBrightnessSmooth(virtualModule.greenLevel/4095.0);

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
        updateLEDs();
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

    // trigger and button handling

    // .....

    //dacReadIndex++;
    float dac1Sample = (float) virtualModule.outputs.dac1Samples[0];
    float dac2Sample = (float) virtualModule.outputs.dac2Samples[0];
    float dac3Sample = (float) virtualModule.outputs.dac3Samples[0];
    // if (dacReadIndex == virtualModule.bufferSize - 1) {
        virtualModule.halfTransferCallback();
    // } else if (dacReadIndex == virtualModule.bufferSize*2 - 1) {
    //    virtualModule.transferCompleteCallback();
    //    dacReadIndex = 0;
    //}

    // "model" the circuit
    // A and B inputs with normalled reference voltages

    float aIn = inputs[A_INPUT].value + (!inputs[A_INPUT].active) * params[A_PARAM].value;
    float bIn = (inputs[B_INPUT].active) * ((inputs[B_INPUT].value) * (params[B_PARAM].value)) + (!inputs[B_INPUT].active) * (5* (params[B_PARAM].value));
    
    // sample and holds

    // // get a new sample on the rising edge at the sh control output
    // if (virtualModule.shAOutput > shALast) {
    //     aSample = aIn;
    // }

    // if (virtualModule.shBOutput > shBLast) {
    //     bSample = bIn;
    // }
    // shALast = virtualModule.shAOutput;
    // shBLast = virtualModule.shBOutput;

    // // either use the sample or track depending on the sh control output
    // aIn = virtualModule.shAOutput * aSample + !virtualModule.shAOutput * aIn;
    // bIn = virtualModule.shBOutput * bSample + !virtualModule.shBOutput * bIn;

    // VCA/mixing stage
    // normalize 12 bits to 0-1
    outputs[MAIN_OUTPUT].value = bIn*(dac2Sample/4095.0) + aIn*(dac1Sample/4095.0); 
    outputs[AUX_DAC_OUTPUT].value = (dac3Sample/4095.0 - .5) * 10.666666666;
    
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

	addParam(ParamWidget::create<ViaSifamBlack>(Vec(11.5, 33), module, Via_Meta::KNOB1_PARAM, 0, 4095.0, 0.0));
    addParam(ParamWidget::create<ViaSifamBlack>(Vec(70.5, 33), module, Via_Meta::KNOB2_PARAM, 0, 4095.0, 0.0));
    addParam(ParamWidget::create<ViaSifamBlack>(Vec(70.5, 173), module, Via_Meta::KNOB3_PARAM, 0, 4095.0, 0.0));
    addParam(ParamWidget::create<ViaSifamGrey>(Vec(11.5, 173), module, Via_Meta::B_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<ViaSifamBlack>(Vec(132.5, 33), module, Via_Meta::CV2AMT_PARAM, 0, 1.0, 0.0));
    addParam(ParamWidget::create<ViaSifamGrey>(Vec(132.5, 104), module, Via_Meta::A_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<ViaSifamBlack>(Vec(132.5, 173), module, Via_Meta::CV3AMT_PARAM, 0, 1.0, 0.0));
    
    addParam(ParamWidget::create<SH_Button>(Vec(8.5, 86), module, Via_Meta::BUTTON1_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Up_Button>(Vec(47, 82.5), module, Via_Meta::BUTTON2_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Freq_Button>(Vec(85, 86), module, Via_Meta::BUTTON3_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Trig_Button>(Vec(8.5, 137), module, Via_Meta::BUTTON4_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Down_Button>(Vec(46, 138), module, Via_Meta::BUTTON5_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Loop_Button>(Vec(85, 137), module, Via_Meta::BUTTON6_PARAM, 0.0, 1.0, 0.0));
    
    addParam(ParamWidget::create<VIA_manual_button>(Vec(133.5, 320), module, Via_Meta::TRIGBUTTON_PARAM, 0.0, 5.0, 0.0));


	addInput(Port::create<ViaJack>(Vec(9, 242), Port::INPUT, module, Via_Meta::A_INPUT));
    addInput(Port::create<ViaJack>(Vec(9, 283.5), Port::INPUT, module, Via_Meta::B_INPUT));
    addInput(Port::create<ViaJack>(Vec(9, 326), Port::INPUT, module, Via_Meta::MAIN_LOGIC_INPUT));
    addInput(Port::create<ViaJack>(Vec(47.0, 242), Port::INPUT, module, Via_Meta::CV1_INPUT));
    addInput(Port::create<ViaJack>(Vec(47.0, 283.5), Port::INPUT, module, Via_Meta::CV2_INPUT));
    addInput(Port::create<ViaJack>(Vec(47.0, 326), Port::INPUT, module, Via_Meta::CV3_INPUT));
    addInput(Port::create<ViaJack>(Vec(135.5, 283.5), Port::INPUT, module, Via_Meta::AUX_LOGIC_INPUT));

	addOutput(Port::create<ViaJack>(Vec(84.8, 242), Port::OUTPUT, module, Via_Meta::LOGICA_OUTPUT));
    addOutput(Port::create<ViaJack>(Vec(84.8, 283.5), Port::OUTPUT, module, Via_Meta::AUX_DAC_OUTPUT));
    addOutput(Port::create<ViaJack>(Vec(84.8, 326), Port::OUTPUT, module, Via_Meta::MAIN_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(135.5, 242), Port::OUTPUT, module, Via_Meta::AUX_LOGIC_OUTPUT));

	addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.3, 269.2), module, Via_Meta::LED1_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(74.8, 269.2), module, Via_Meta::LED2_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.3, 310.7), module, Via_Meta::LED3_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(74.8, 310.7), module, Via_Meta::LED4_LIGHT));
    addChild(ModuleLightWidget::create<LargeLight<RGBTriangle>>(Vec(59, 221), module, Via_Meta::RED_LIGHT));
}


Model *modelVia_Meta = Model::create<Via_Meta, Via_Meta_Widget>(
        "Starling", "Via_Meta", "Via_Meta", OSCILLATOR_TAG);



















///////////////////////// TABLE CLASS ///////////////////////////////




