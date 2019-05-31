#pragma once

#include "dsp/digital.hpp"
#include "via_ui.hpp"
#include "via_virtual_module.hpp"
#include "dsp/decimator.hpp"

template<int OVERSAMPLE_AMOUNT, int OVERSAMPLE_QUALITY> 
struct Via : Module {
    
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
    
    Via() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

        dac1DecimatorBuffer = (float*) malloc(OVERSAMPLE_AMOUNT * sizeof(float));
        dac2DecimatorBuffer = (float*) malloc(OVERSAMPLE_AMOUNT * sizeof(float));
        dac3DecimatorBuffer = (float*) malloc(OVERSAMPLE_AMOUNT * sizeof(float));

    }

    ViaModule * virtualIO;

    uint32_t presetData[6];
    
    SchmittTrigger mainLogic;
    SchmittTrigger auxLogic;

    bool lastTrigState = false;
    bool lastAuxTrigState = false;
    
    int32_t lastTrigButton = 0;

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

    float * dac1DecimatorBuffer;
    float * dac2DecimatorBuffer;
    float * dac3DecimatorBuffer;

    Decimator<OVERSAMPLE_AMOUNT, OVERSAMPLE_QUALITY> dac1Decimator;
    Decimator<OVERSAMPLE_AMOUNT, OVERSAMPLE_QUALITY> dac2Decimator;
    Decimator<OVERSAMPLE_AMOUNT, OVERSAMPLE_QUALITY> dac3Decimator;

    void updateSlowIO(void) {

        virtualIO->button1Input = (int32_t) params[BUTTON1_PARAM].value;
        virtualIO->button2Input = (int32_t) params[BUTTON2_PARAM].value;
        virtualIO->button3Input = (int32_t) params[BUTTON3_PARAM].value;
        virtualIO->button4Input = (int32_t) params[BUTTON4_PARAM].value;
        virtualIO->button5Input = (int32_t) params[BUTTON5_PARAM].value;
        virtualIO->button6Input = (int32_t) params[BUTTON6_PARAM].value;

        // these have a janky array ordering to correspond with the DMA stream on the hardware
        virtualIO->controls.controlRateInputs[2] = clamp((int32_t) params[KNOB1_PARAM].value, 0, 4095);
        virtualIO->controls.controlRateInputs[3] = clamp((int32_t) params[KNOB2_PARAM].value, 0, 4095);
        virtualIO->controls.controlRateInputs[1] = clamp((int32_t) params[KNOB3_PARAM].value, 0, 4095);
        // model the the 1v/oct input, scale 10.6666666 volts 12 bit adc range
        // it the gain scaling stage is inverting
        float cv1Conversion = -inputs[CV1_INPUT].value;
        // ultimately we want a volt to be a chage of 384 in the adc reading
        cv1Conversion = cv1Conversion * 384.0;
        // offset to unipolar
        cv1Conversion += 2048.0;
        // clip at rails of the input opamp
        virtualIO->controls.controlRateInputs[0] = clamp((int32_t)cv1Conversion, 0, 4095);
    }

    // 2 sets the "GPIO" high, 1 sets it low, 0 is a no-op
    inline int32_t virtualLogicOut(int32_t logicOut, int32_t control) {
        return clamp(logicOut + (control & 2) - (control & 1), 0, 1);
    }

    void updateLEDs(void) {

        // the A B C D enumeration of the LEDs in the Via library makes little to no sense 
        // but its woven pretty deep so is a nagging style thing to fix

        ledAState = virtualLogicOut(ledAState, virtualIO->ledAOutput);
        ledBState = virtualLogicOut(ledBState, virtualIO->ledBOutput);
        ledCState = virtualLogicOut(ledCState, virtualIO->ledCOutput);
        ledDState = virtualLogicOut(ledDState, virtualIO->ledDOutput);

        lights[LED1_LIGHT].setBrightnessSmooth(ledAState, 2);
        lights[LED3_LIGHT].setBrightnessSmooth(ledBState, 2);
        lights[LED2_LIGHT].setBrightnessSmooth(ledCState, 2);
        lights[LED4_LIGHT].setBrightnessSmooth(ledDState, 2);

        lights[RED_LIGHT].setBrightnessSmooth(virtualIO->redLevelWrite/4095.0, 2);
        lights[GREEN_LIGHT].setBrightnessSmooth(virtualIO->greenLevelWrite/4095.0, 2);
        lights[BLUE_LIGHT].setBrightnessSmooth(virtualIO->blueLevelWrite/4095.0, 2);

        float output = outputs[MAIN_OUTPUT].value/8.0;
        lights[OUTPUT_RED_LIGHT].setBrightnessSmooth(clamp(-output, 0.0, 1.0));
        lights[OUTPUT_GREEN_LIGHT].setBrightnessSmooth(clamp(output, 0.0, 1.0));

    }

    void updateLogicOutputs(void) {
        logicAState = virtualLogicOut(logicAState, virtualIO->aLogicOutput);
        auxLogicState = virtualLogicOut(auxLogicState, virtualIO->auxLogicOutput);
        shAControl = virtualLogicOut(shAControl, virtualIO->shAOutput);
        shBControl = virtualLogicOut(shBControl, virtualIO->shBOutput);
    }

    void updateAudioRate(void) {

        // manage audio rate dacs and adcs

        // scale -5 - 5 V to -1 to 1 and then convert to 16 bit int;
        float cv2Scale = (32767.0 * clamp(-inputs[CV2_INPUT].value/5, -1.0, 1.0)) * params[CV2AMT_PARAM].value;
        float cv3Scale = (32767.0 * clamp(-inputs[CV3_INPUT].value/5, -1.0, 1.0)) * params[CV3AMT_PARAM].value;
        int16_t cv2Conversion = (int16_t) cv2Scale;
        int16_t cv3Conversion = (int16_t) cv3Scale;

        // no ADC buffer for now..
        virtualIO->inputs.cv2Samples[0] = cv2Conversion;
        virtualIO->inputs.cv3Samples[0] = cv3Conversion;

        // trigger handling

        mainLogic.process(rescale(inputs[MAIN_LOGIC_INPUT].value, .2, 1.2, 0.f, 1.f));
        bool trigState = mainLogic.isHigh();
        if (trigState && !lastTrigState) {
            virtualIO->mainRisingEdgeCallback();
        } else if (!trigState && lastTrigState) {
            virtualIO->mainFallingEdgeCallback();
        }
        lastTrigState = trigState; 

        auxLogic.process(rescale(inputs[AUX_LOGIC_INPUT].value, .2, 1.2, 0.f, 1.f));
        bool auxTrigState = auxLogic.isHigh();
        if (auxTrigState && !lastAuxTrigState) {
            virtualIO->auxRisingEdgeCallback();
        } else if (!auxTrigState && lastAuxTrigState) {
            virtualIO->auxFallingEdgeCallback();
        }
        lastAuxTrigState = auxTrigState; 

        int32_t samplesRemaining = OVERSAMPLE_AMOUNT;
        int32_t writeIndex = 0;

        // convert to float and downsample

        while (samplesRemaining) {

            dac1DecimatorBuffer[writeIndex] = (float) virtualIO->outputs.dac1Samples[writeIndex];
            dac2DecimatorBuffer[writeIndex] = (float) virtualIO->outputs.dac2Samples[writeIndex];
            dac3DecimatorBuffer[writeIndex] = (float) virtualIO->outputs.dac3Samples[writeIndex];

            samplesRemaining--;
            writeIndex ++;

        }

        float dac1Sample = dac1Decimator.process(dac1DecimatorBuffer);
        float dac2Sample = dac2Decimator.process(dac2DecimatorBuffer);
        float dac3Sample = dac3Decimator.process(dac3DecimatorBuffer);
        updateLogicOutputs();
        virtualIO->halfTransferCallback();

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

    };

};

