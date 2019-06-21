#pragma once

#include "via_ui.hpp"
#include "via_virtual_module.hpp"
#include "pow2decimate.hpp"

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
    
    Via() {

        dac1DecimatorBuffer = (float*) malloc(OVERSAMPLE_AMOUNT * sizeof(float));
        dac2DecimatorBuffer = (float*) malloc(OVERSAMPLE_AMOUNT * sizeof(float));
        dac3DecimatorBuffer = (float*) malloc(OVERSAMPLE_AMOUNT * sizeof(float));

    }

    ViaModule * virtualIO;

    uint32_t presetData[6];
    
    dsp::SchmittTrigger mainLogic;
    dsp::SchmittTrigger auxLogic;

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

    pow2Decimate<OVERSAMPLE_AMOUNT, OVERSAMPLE_QUALITY> dac1Decimator;
    pow2Decimate<OVERSAMPLE_AMOUNT, OVERSAMPLE_QUALITY> dac2Decimator;
    pow2Decimate<OVERSAMPLE_AMOUNT, OVERSAMPLE_QUALITY> dac3Decimator;


    // dsp::Decimator<OVERSAMPLE_AMOUNT, OVERSAMPLE_QUALITY> dac1Decimator;
    // dsp::Decimator<OVERSAMPLE_AMOUNT, OVERSAMPLE_QUALITY> dac2Decimator;
    // dsp::Decimator<OVERSAMPLE_AMOUNT, OVERSAMPLE_QUALITY> dac3Decimator;

    void updateSlowIO(void) {

        virtualIO->button1Input = (int32_t) params[BUTTON1_PARAM].getValue();
        virtualIO->button2Input = (int32_t) params[BUTTON2_PARAM].getValue();
        virtualIO->button3Input = (int32_t) params[BUTTON3_PARAM].getValue();
        virtualIO->button4Input = (int32_t) params[BUTTON4_PARAM].getValue();
        virtualIO->button5Input = (int32_t) params[BUTTON5_PARAM].getValue();
        virtualIO->button6Input = (int32_t) params[BUTTON6_PARAM].getValue();

        // these have a janky array ordering to correspond with the DMA stream on the hardware
        virtualIO->controls.controlRateInputs[2] = clamp((int32_t) (params[KNOB1_PARAM].getValue()), 0, 4095);
        virtualIO->controls.controlRateInputs[3] = clamp((int32_t) (params[KNOB2_PARAM].getValue()), 0, 4095);
        virtualIO->controls.controlRateInputs[1] = clamp((int32_t) (params[KNOB3_PARAM].getValue()), 0, 4095);
        // model the the 1v/oct input, scale 10.6666666 volts 12 bit adc range
        // it the gain scaling stage is inverting
        float cv1Conversion = -inputs[CV1_INPUT].getVoltage();
        // ultimately we want a volt to be a chage of 384 in the adc reading
        cv1Conversion = cv1Conversion * 384.0;
        // offset to unipolar
        cv1Conversion += 2048.0;
        // clip at rails of the input opamp
        virtualIO->controls.controlRateInputs[0] = clamp((int32_t)cv1Conversion, 0, 4095);
    }

    void processTriggerButton(void) {
        int32_t trigButton = clamp((int32_t)params[TRIGBUTTON_PARAM].getValue(), 0, 1);
        if (trigButton > lastTrigButton) {
            virtualIO->buttonPressedCallback();
        } else if (trigButton < lastTrigButton) {
            virtualIO->buttonReleasedCallback();
        } 
        lastTrigButton = trigButton;
    }

    // 2 sets the "GPIO" high, 1 sets it low, 0 is a no-op
    inline int32_t virtualLogicOut(int32_t logicOut, int32_t control) {
        return clamp(logicOut + (control & 2) - (control & 1), 0, 1);
    }

    float ledDecay = 1.0/48000.0;

    inline void updateLEDs(void) {

        // the A B C D enumeration of the LEDs in the Via library makes little to no sense 
        // but its woven pretty deep so is a nagging style thing to fix

        ledAState = virtualLogicOut(ledAState, virtualIO->ledAOutput);
        ledBState = virtualLogicOut(ledBState, virtualIO->ledBOutput);
        ledCState = virtualLogicOut(ledCState, virtualIO->ledCOutput);
        ledDState = virtualLogicOut(ledDState, virtualIO->ledDOutput);

        lights[LED1_LIGHT].setSmoothBrightness(ledAState, ledDecay);
        lights[LED3_LIGHT].setSmoothBrightness(ledBState, ledDecay);
        lights[LED2_LIGHT].setSmoothBrightness(ledCState, ledDecay);
        lights[LED4_LIGHT].setSmoothBrightness(ledDState, ledDecay);

        lights[RED_LIGHT].setSmoothBrightness(virtualIO->redLevelWrite/4095.0, ledDecay);
        lights[GREEN_LIGHT].setSmoothBrightness(virtualIO->greenLevelWrite/4095.0, ledDecay);
        lights[BLUE_LIGHT].setSmoothBrightness(virtualIO->blueLevelWrite/4095.0, ledDecay);

        float output = outputs[MAIN_OUTPUT].value/8.0;
        lights[OUTPUT_RED_LIGHT].setSmoothBrightness(clamp(-output, 0.0, 1.0), ledDecay);
        lights[OUTPUT_GREEN_LIGHT].setSmoothBrightness(clamp(output, 0.0, 1.0), ledDecay);

    }

    void updateLogicOutputs(void) {
        logicAState = virtualLogicOut(logicAState, virtualIO->aLogicOutput);
        auxLogicState = virtualLogicOut(auxLogicState, virtualIO->auxLogicOutput);
        shAControl = virtualLogicOut(shAControl, virtualIO->shAOutput);
        shBControl = virtualLogicOut(shBControl, virtualIO->shBOutput);
    }

    inline void acquireCVs(void) {
        // scale -5 - 5 V to -1 to 1 and then convert to 16 bit int;
        float cv2Scale = (32767.0 * clamp(-inputs[CV2_INPUT].getVoltage()/5, -1.0, 1.0)) * params[CV2AMT_PARAM].getValue();
        float cv3Scale = (32767.0 * clamp(-inputs[CV3_INPUT].getVoltage()/5, -1.0, 1.0)) * params[CV3AMT_PARAM].getValue();
        int16_t cv2Conversion = (int16_t) cv2Scale;
        int16_t cv3Conversion = (int16_t) cv3Scale;

        // no ADC buffer for now..
        virtualIO->inputs.cv2Samples[0] = cv2Conversion;
        virtualIO->inputs.cv3Samples[0] = cv3Conversion;
    }

    inline void processLogicInputs(void) {

        mainLogic.process(rescale(inputs[MAIN_LOGIC_INPUT].getVoltage(), .2, 1.2, 0.f, 1.f));
        bool trigState = mainLogic.isHigh();
        if (trigState && !lastTrigState) {
            virtualIO->mainRisingEdgeCallback();
        } else if (!trigState && lastTrigState) {
            virtualIO->mainFallingEdgeCallback();
        }
        lastTrigState = trigState; 

        auxLogic.process(rescale(inputs[AUX_LOGIC_INPUT].getVoltage(), .2, 1.2, 0.f, 1.f));
        bool auxTrigState = auxLogic.isHigh();
        if (auxTrigState && !lastAuxTrigState) {
            virtualIO->auxRisingEdgeCallback();
        } else if (!auxTrigState && lastAuxTrigState) {
            virtualIO->auxFallingEdgeCallback();
        }
        lastAuxTrigState = auxTrigState; 

    }

    inline void updateOutputs(void) {

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
        outputs[MAIN_OUTPUT].setVoltage(bIn*(dac2Sample/4095.0) + aIn*(dac1Sample/4095.0)); 
        outputs[AUX_DAC_OUTPUT].setVoltage((dac3Sample/4095.0 - .5) * -10.666666666);
        outputs[LOGICA_OUTPUT].setVoltage(logicAState * 5.0);
        outputs[AUX_LOGIC_OUTPUT].setVoltage(auxLogicState * 5.0);

    }

    void updateAudioRate(void) {

        acquireCVs();

        processLogicInputs();

        updateOutputs();

        clockDivider = 0;

    };

    struct BScaleQuantity : ParamQuantity {

        bool bConnected(void) {

            Via * module = dynamic_cast<Via *>(this->module);

            return module->inputs[B_INPUT].isConnected();

        } 

        std::string getDisplayValueString() override {

            if (!module)
                return "";

            float v = getSmoothValue();

            if (bConnected()) {
                return getLabel() + " scale: " + string::f("%.*g", 2, v);
            } else {
                return getLabel() + ": " + string::f("%.*g", 2, v * 5.0) + "V";                
            }

        }

        std::string getString() override {
            return getDisplayValueString();
        }

        void setDisplayValueString(std::string s) override {

            float v = 0.f;
            char suffix[2];
            int n = std::sscanf(s.c_str(), "%f%1s", &v, suffix);
            if (n >= 2) {
                // Parse SI prefixes
                switch (suffix[0]) {
                    case 'n': v *= 1e-9f; break;
                    case 'u': v *= 1e-6f; break;
                    case 'm': v *= 1e-3f; break;
                    case 'k': v *= 1e3f; break;
                    case 'M': v *= 1e6f; break;
                    case 'G': v *= 1e9f; break;
                    default: break;
                }
            }
            if (n >= 1) {
                if (bConnected()) {
                    setValue(v);
                } else {
                    setValue(v/5.0);
                }
            }
        }

    };

    struct ANormalQuantity : ParamQuantity {

        std::string getDisplayValueString() override {

            if (!module)
                return "";

            Via * module = dynamic_cast<Via *>(this->module);

            bool aConnected = module->inputs[A_INPUT].isConnected();

            float v = getSmoothValue();

            if (aConnected) {
                return "Overriden by A input";
            } else {
                return getLabel() + ": " + string::f("%.*g", 2, v) + "V";                
            }

        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct ButtonQuantity : ParamQuantity {

        std::string getString() override {
            return getLabel();
        }

        void setDisplayValueString(std::string s) override {}
        
    };

};

