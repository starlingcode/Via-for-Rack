#include "atsr.hpp"
#include "via_module.hpp"

#define ATSR_OVERSAMPLE_AMOUNT 1
#define ATSR_OVERSAMPLE_QUALITY 1

struct Atsr : Via<ATSR_OVERSAMPLE_AMOUNT, ATSR_OVERSAMPLE_QUALITY> {
    
    Atsr() : Via() {

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        virtualIO = &virtualModule;

        configParam(KNOB1_PARAM, 0, 4095.0, 2048.0, "Attack time");
        configParam(KNOB2_PARAM, 0, 4095.0, 2048.0, "Transition time");
        configParam(KNOB3_PARAM, 0, 4095.0, 2048.0, "Release time");
        configParam(B_PARAM, -1.0, 1.0, 0.5, "Sustain level scale");
        configParam(CV2AMT_PARAM, 0, 1.0, 1.0, "Transition time CV amount");
        configParam(A_PARAM, -5.0, 5.0, 5.0, "Attack level normal");
        configParam(CV3AMT_PARAM, 0, 1.0, 1.0, "Release time CV amount");
        
        configParam(BUTTON1_PARAM, 0.0, 1.0, 0.0, "Attack shape");
        configParam(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Transition shape");
        configParam(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Segment gate select");
        configParam(BUTTON4_PARAM, 0.0, 1.0, 0.0, "Toggle A time CV attack or all");
        configParam(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Toggle Level CV sample and hold");
        configParam(BUTTON6_PARAM, 0.0, 1.0, 0.0, "Release shape");
        
        configParam(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Manual gate");

        onSampleRateChange();
    }

    ViaAtsr virtualModule;

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();

        if (sampleRate == 44100.0) {
            virtualModule.incScale = 71332;
        } else if (sampleRate == 48000.0) {
            virtualModule.incScale = 65536;
        } else if (sampleRate == 88200.0) {
            virtualModule.incScale = 35666;
        } else if (sampleRate == 96000.0) {
            virtualModule.incScale = 32768;
        } else if (sampleRate == 176400.0) {
            virtualModule.incScale = 17833;
        } else if (sampleRate == 192000.0) {
            virtualModule.incScale = 16383;
        }
        
    }

    void updateLEDs(void) {

        // the A B C D enumeration of the LEDs in the Via library makes little to no sense 
        // but its woven pretty deep so is a nagging style thing to fix

        if (virtualModule.runtimeDisplay & !virtualModule.shOn) {
            lights[LED1_LIGHT].setSmoothBrightness(virtualModule.blueLevelWrite/4095.0, 1);
            lights[LED3_LIGHT].setSmoothBrightness(virtualModule.redLevelWrite/4095.0, 1);
        } else {
            ledAState = virtualLogicOut(ledAState, virtualModule.ledAOutput);
            ledBState = virtualLogicOut(ledBState, virtualModule.ledBOutput);
            lights[LED1_LIGHT].setSmoothBrightness(ledAState, 1);
            lights[LED3_LIGHT].setSmoothBrightness(ledBState, 1);
        }
        ledCState = virtualLogicOut(ledCState, virtualModule.ledCOutput);
        ledDState = virtualLogicOut(ledDState, virtualModule.ledDOutput);


        lights[LED2_LIGHT].setSmoothBrightness(ledCState, 1);
        lights[LED4_LIGHT].setSmoothBrightness(ledDState, 1);

        lights[RED_LIGHT].setSmoothBrightness(virtualModule.redLevelWrite/4095.0, 1);
        lights[GREEN_LIGHT].setSmoothBrightness(virtualModule.greenLevelWrite/4095.0, 1);
        lights[BLUE_LIGHT].setSmoothBrightness(virtualModule.blueLevelWrite/4095.0, 1);

        float output = outputs[MAIN_OUTPUT].value/8.0;
        lights[OUTPUT_RED_LIGHT].setSmoothBrightness(clamp(-output, 0.0, 1.0), 1);
        lights[OUTPUT_GREEN_LIGHT].setSmoothBrightness(clamp(output, 0.0, 1.0), 1);

    }

    json_t *dataToJson() override {

        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "atsr_modes", json_integer(virtualModule.atsrUI.modeStateBuffer));
    
     
        return rootJ;
    }
    
    void dataFromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "atsr_modes");
        virtualModule.atsrUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.atsrUI.loadFromEEPROM(0);
        virtualModule.atsrUI.recallModuleState();


    }

    void process(const ProcessArgs &args) override;
    
};

void Atsr::process(const ProcessArgs &args) {

    // update the "slow IO" (not audio rate) every 16 samples
    // needs to scale with sample rate somehow
    slowIOPrescaler++;
    if (slowIOPrescaler == 16) {
        slowIOPrescaler = 0;
        updateSlowIO();
        virtualModule.slowConversionCallback();
        virtualModule.ui_dispatch(SENSOR_EVENT_SIG);
        virtualModule.atsrUI.incrementTimer();
        processTriggerButton();
        updateLEDs();
    }

    acquireCVs();

    processLogicInputs();

    virtualModule.halfTransferCallback();
    float dac1Sample = (float) virtualModule.outputs.dac1Samples[0];
    float dac2Sample = (float) virtualModule.outputs.dac2Samples[0];
    float dac3Sample = (float) virtualModule.outputs.dac3Samples[0];
    updateLogicOutputs();

    // "model" the circuit
    // A and B inputs with normalled reference voltages
    float aIn = inputs[A_INPUT].getVoltage() + (!inputs[A_INPUT].isConnected()) * params[A_PARAM].getValue();
    float bIn = (inputs[B_INPUT].isConnected()) * ((inputs[B_INPUT].getVoltage()) * (params[B_PARAM].getValue())) + (!inputs[B_INPUT].isConnected()) * (5* (params[B_PARAM].getValue()));
    
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
    outputs[MAIN_OUTPUT].setVoltage(bIn*(dac2Sample/32767.0) + aIn*(dac1Sample/32767.0)); 
    outputs[AUX_DAC_OUTPUT].setVoltage((dac3Sample/4095.0 - .5) * -10.666666666);
    outputs[LOGICA_OUTPUT].setVoltage(logicAState * 5.0);
    outputs[AUX_LOGIC_OUTPUT].setVoltage(auxLogicState * 5.0);
    
}

struct AtsrWidget : ModuleWidget  {

    AtsrWidget(Atsr *module) {

        setModule(module);

        box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/atsr.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Atsr::KNOB1_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Atsr::KNOB2_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Atsr::KNOB3_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Atsr::B_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Atsr::CV2AMT_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Atsr::A_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Atsr::CV3AMT_PARAM));
        
        addParam(createParam<TransparentButton>(Vec(9 + .753, 85), module, Atsr::BUTTON1_PARAM));
        addParam(createParam<TransparentButton>(Vec(48 + .753, 75), module, Atsr::BUTTON2_PARAM));
        addParam(createParam<TransparentButton>(Vec(87 + .753, 86), module, Atsr::BUTTON3_PARAM));
        addParam(createParam<TransparentButton>(Vec(7 + .753, 132), module, Atsr::BUTTON4_PARAM));
        addParam(createParam<TransparentButton>(Vec(48 + .753, 142), module, Atsr::BUTTON5_PARAM));
        addParam(createParam<TransparentButton>(Vec(88 + .753, 135), module, Atsr::BUTTON6_PARAM));
        
        addParam(createParam<ViaPushButton>(Vec(132.7 + .753, 320), module, Atsr::TRIGBUTTON_PARAM));

        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 241.12), module, Atsr::A_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 282.62), module, Atsr::B_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 324.02), module, Atsr::MAIN_LOGIC_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 241.12), module, Atsr::CV1_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 282.62), module, Atsr::CV2_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 324.02), module, Atsr::CV3_INPUT));
        addInput(createInput<ViaJack>(Vec(135 + 1.053, 282.62), module, Atsr::AUX_LOGIC_INPUT));

        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 241.12), module, Atsr::LOGICA_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 282.62), module, Atsr::AUX_DAC_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 324.02), module, Atsr::MAIN_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(135 + 1.053, 241.12), module, Atsr::AUX_LOGIC_OUTPUT));

        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Atsr::LED1_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 268.5), module, Atsr::LED2_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Atsr::LED3_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.7 + .753, 309.9), module, Atsr::LED4_LIGHT));
        addChild(createLight<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Atsr::OUTPUT_GREEN_LIGHT));
        addChild(createLight<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Atsr::RED_LIGHT));

        }

};



Model *modelAtsr = createModel<Atsr, AtsrWidget>("ATSR");


