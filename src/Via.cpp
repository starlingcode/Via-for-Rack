#include "Via.hpp"



void Via::step() {
    
    if (oneTime == 0) {
        
        if (loopMode == noloop) {
            switch (freqMode) {
                case audio:
                    getPhase = &Via::getPhaseDrum;
                    break;
                case env:
                    getPhase = &Via::getPhaseSimpleEnv;
                    break;
                case seq:
                    getPhase = &Via::getPhaseComplexEnv;
                    break;
            }
        } else {
            switch (freqMode) {
                case audio:
                    getPhase = &Via::getPhaseOsc;
                    break;
                case env:
                    getPhase = &Via::getPhaseSimpleLFO;
                    break;
                case seq:
                    getPhase = &Via::getPhaseComplexLFO;
                    break;
            }
        }
        
        attackTime = &Via::calcTime1Env;
        releaseTime = &Via::calcTime2Env;
        
        incSign = 1;
        oneTime = 1;
        currentFamily = familyArray[0][0];
        switchFamily();
        SET_GATEA;
        SET_GATEB;
        SET_RGB_ON;

    }
    
    
    
    if (detectOn == 0) {
        readDetect();
    } else {
        readRelease(modeFlag);
    }
    
    if (displayNewMode == 1) {
        pressCounter ++;
        if (pressCounter > engineGetSampleRate()) {
            clearLEDs(); // get rid of our last mode display
            SET_RGB_ON;
            pressCounter = 0;
            displayNewMode = 0;
        }
    }
    
    //trigButton.process(params[TRIGBUTTON_PARAM].value);
    //trigInput.process(inputs[TRIG_INPUT].value);
    if ((inputs[TRIG_INPUT].value >= 1.0) || (params[TRIGBUTTON_PARAM].value >= 1.0)) {
        triggerState = 1;
    } else {
        triggerState = 0;
    }
    
    

    if(triggerState > lastTriggerState) {
        risingEdgeHandler();
    }
    else if (triggerState < lastTriggerState) {
        fallingEdgeHandler();
    }
        

    lastTriggerState = triggerState;
     



    if (inputs[FREEZE_INPUT].value < 1.0) {
        dacISR();
    }

    
    float aIn = inputs[A_INPUT].value + (!inputs[A_INPUT].active) * params[A_PARAM].value;
    float bIn = (inputs[B_INPUT].active) * ((inputs[B_INPUT].value) * (params[B_PARAM].value)) + (!inputs[B_INPUT].active) * (5* (params[B_PARAM].value));
    
    if (sampleA) {
        aSample = aIn;
        sampleA = 0;
        holdA = 1;
    }
    
    if (sampleB) {
        bSample = bIn;
        sampleB = 0;
        holdB = 1;
    }
    
    if (holdA) {aIn = aSample;}
    if (holdB) {bIn = bSample;}
    
    outputs[MAIN_OUTPUT].value = bIn*(out/4095.0) + aIn*((4095-out)/4095.0);
    
    
}


struct ViaWidget : ModuleWidget {
    ViaWidget(Via *module);
};


ViaWidget::ViaWidget(Via *module) : ModuleWidget(module) {
	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/via_freerunning_no_margin.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(ParamWidget::create<Davies1900hvia>(Vec(11.5, 33), module, Via::T1_PARAM, 0, 1.0, 0.0));
    addParam(ParamWidget::create<Davies1900hvia>(Vec(70.5, 33), module, Via::T2_PARAM, 0, 1.0, 0.0));
    addParam(ParamWidget::create<Davies1900hvia>(Vec(70.5, 173), module, Via::MORPH_PARAM, 0, 1.0, 0.0));
    addParam(ParamWidget::create<Davies1900hvia>(Vec(11.5, 173), module, Via::B_PARAM, -1.0, 1.0, 0.0));
    addParam(ParamWidget::create<Davies1900hvia>(Vec(132.5, 33), module, Via::T2AMT_PARAM, 0, 1.0, 0.0));
    addParam(ParamWidget::create<Davies1900hvia>(Vec(132.5, 104), module, Via::A_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Davies1900hvia>(Vec(132.5, 173), module, Via::MORPHAMT_PARAM, 0, 1.0, 0.0));
    
    addParam(ParamWidget::create<SH_Button>(Vec(8.5, 86), module, Via::SH_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Up_Button>(Vec(47, 82.5), module, Via::UP_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Freq_Button>(Vec(85, 86), module, Via::FREQ_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Trig_Button>(Vec(8.5, 137), module, Via::TRIG_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Down_Button>(Vec(46, 138), module, Via::DOWN_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Loop_Button>(Vec(85, 137), module, Via::LOOP_PARAM, 0.0, 1.0, 0.0));
    
    addParam(ParamWidget::create<VIA_manual_button>(Vec(133.5, 320), module, Via::TRIGBUTTON_PARAM, 0.0, 5.0, 0.0));


	addInput(Port::create<PJ301MPort>(Vec(9, 242), Port::INPUT, module, Via::A_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(9, 283.5), Port::INPUT, module, Via::B_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(9, 326), Port::INPUT, module, Via::TRIG_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(47.0, 242), Port::INPUT, module, Via::T1_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(47.0, 283.5), Port::INPUT, module, Via::T2_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(47.0, 326), Port::INPUT, module, Via::MORPH_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(135.5, 283.5), Port::INPUT, module, Via::FREEZE_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(84.8, 242), Port::OUTPUT, module, Via::LOGICA_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(84.8, 283.5), Port::OUTPUT, module, Via::LOGICB_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(84.8, 326), Port::OUTPUT, module, Via::MAIN_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(135.5, 242), Port::OUTPUT, module, Via::DELTA_OUTPUT));

	addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.3, 269.2), module, Via::LED1_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(74.8, 269.2), module, Via::LED2_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(35.3, 310.7), module, Via::LED3_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<WhiteLight>>(Vec(74.8, 310.7), module, Via::LED4_LIGHT));
    addChild(ModuleLightWidget::create<LargeLight<RGBTriangle>>(Vec(59, 221), module, Via::RED_LIGHT));
}


Model *modelVia = Model::create<Via, ViaWidget>(
        "CompanyName", "Via", "Via", OSCILLATOR_TAG);



















///////////////////////// TABLE CLASS ///////////////////////////////




