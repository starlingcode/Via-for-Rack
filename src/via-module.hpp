#pragma once

#include "via-ui.hpp"
#include "via-virtual-module.hpp"
#include "starling-dsp.hpp"

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

        dacDecimatorBuffer = (float_4*) malloc(OVERSAMPLE_AMOUNT * sizeof(float_4));

    }

    TARGET_VIA * virtualIO;

    uint32_t presetData[6];
    
    dsp::SchmittTrigger mainLogic;
    dsp::SchmittTrigger auxLogic;

    bool lastTrigState = false;
    bool lastAuxTrigState = false;
    
    int32_t lastTrigButton = 0;

    int32_t dacReadIndex = 0;
    int32_t adcWriteIndex = 0;
    int32_t slowIOPrescaler = 0;

    float shALast = 0;
    float shBLast = 0;

    float aSample = 0;
    float bSample = 0;

    int32_t clockDivider = 0;

    int32_t divideAmount = 1;

    float_4 * dacDecimatorBuffer;

    DecimatePow2<OVERSAMPLE_AMOUNT, float_4> dacDecimator;

    virtual void updateSlowIO(void) {

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

    virtual void processTriggerButton(void) {
        int32_t trigButton = clamp((int32_t)params[TRIGBUTTON_PARAM].getValue(), 0, 1);
        if (trigButton > lastTrigButton) {
            virtualIO->buttonPressedCallback();
        } else if (trigButton < lastTrigButton) {
            virtualIO->buttonReleasedCallback();
        } 
        lastTrigButton = trigButton;
    }

    float ledDecay = 4.f/(48000.f);

    virtual void updateLEDs(void) {

        lights[LED1_LIGHT].setSmoothBrightness((float) !virtualIO->ledAState, ledDecay);
        lights[LED3_LIGHT].setSmoothBrightness((float) !virtualIO->ledBState, ledDecay);
        lights[LED2_LIGHT].setSmoothBrightness((float) !virtualIO->ledCState, ledDecay);
        lights[LED4_LIGHT].setSmoothBrightness((float) !virtualIO->ledDState, ledDecay);

        lights[RED_LIGHT].setSmoothBrightness(virtualIO->redLevelOut/4095.0, ledDecay);
        lights[GREEN_LIGHT].setSmoothBrightness(virtualIO->greenLevelOut/4095.0, ledDecay);
        lights[BLUE_LIGHT].setSmoothBrightness(virtualIO->blueLevelOut/4095.0, ledDecay);

        float output = outputs[MAIN_OUTPUT].value/8.0;
        lights[OUTPUT_RED_LIGHT].setSmoothBrightness(clamp(-output, 0.0, 1.0), ledDecay);
        lights[OUTPUT_GREEN_LIGHT].setSmoothBrightness(clamp(output, 0.0, 1.0), ledDecay);

    }

    virtual void acquireCVs(void) {
        // scale -5 - 5 V to -1 to 1 and then convert to 16 bit int;
        float cv2Scale = (32767.0 * clamp(-inputs[CV2_INPUT].getVoltage()/5, -1.0, 1.0)) * params[CV2AMT_PARAM].getValue();
        float cv3Scale = (32767.0 * clamp(-inputs[CV3_INPUT].getVoltage()/5, -1.0, 1.0)) * params[CV3AMT_PARAM].getValue();
        int16_t cv2Conversion = (int16_t) cv2Scale;
        int16_t cv3Conversion = (int16_t) cv3Scale;

        // no ADC buffer for now..
        virtualIO->inputs.cv2Samples[0] = cv2Conversion;
        virtualIO->inputs.cv3Samples[0] = cv3Conversion;
    }

    float lastLogicIn = 0.0;

    virtual void processLogicInputs(void) {

        float thisLogicIn = rescale(inputs[MAIN_LOGIC_INPUT].getVoltage(), .2, 1.2, 0.f, 1.f);
        mainLogic.process(thisLogicIn);
        bool trigState = mainLogic.isHigh();
        if (trigState && !lastTrigState) {

            float difference = thisLogicIn - lastLogicIn;
            float fractionalPhase = (1.0f - lastLogicIn) / difference;

            fractionalPhase *= 1439.0f;
            virtualIO->measurementTimerFractional = fractionalPhase;

            virtualIO->mainRisingEdgeCallback();
        } else if (!trigState && lastTrigState) {
            virtualIO->mainFallingEdgeCallback();
        }
        lastTrigState = trigState; 

        lastLogicIn = thisLogicIn;

        auxLogic.process(rescale(inputs[AUX_LOGIC_INPUT].getVoltage(), .2, 1.2, 0.f, 1.f));
        bool auxTrigState = auxLogic.isHigh();
        if (auxTrigState && !lastAuxTrigState) {
            virtualIO->auxRisingEdgeCallback();
        } else if (!auxTrigState && lastAuxTrigState) {
            virtualIO->auxFallingEdgeCallback();
        }
        lastAuxTrigState = auxTrigState; 

    }

    virtual void updateOutputs(void) {

        // i know this isn't the right way to vectorize but it was the easiest way to test the vectorized implementation

        int32_t samplesRemaining = OVERSAMPLE_AMOUNT;
        int32_t writeIndex = 0;

        while (samplesRemaining) {

            dacDecimatorBuffer[writeIndex] = float_4((float) virtualIO->outputs.dac1Samples[writeIndex],
                                                        (float) virtualIO->outputs.dac2Samples[writeIndex],
                                                            (float) virtualIO->outputs.dac3Samples[writeIndex], 0);

            samplesRemaining--;
            writeIndex ++;

        }

        float_4 result = dacDecimator.process(dacDecimatorBuffer);
        
        float dac1Sample = result[0];
        float dac2Sample = result[1];
        float dac3Sample = result[2];
        
        // float dac1Sample = dac1Decimator.process(dac1DecimatorBuffer);
        // float dac2Sample = dac2Decimator.process(dac2DecimatorBuffer);
        // float dac3Sample = dac3Decimator.process(dac3DecimatorBuffer);
        
        virtualIO->halfTransferCallback();

        // "model" the circuit
        // A and B inputs with normalled reference voltages
        float aIn = inputs[A_INPUT].isConnected() ? inputs[A_INPUT].getVoltage() : params[A_PARAM].getValue();
        float bIn = inputs[B_INPUT].isConnected() ? inputs[B_INPUT].getVoltage() : 5.0;
        bIn *= params[B_PARAM].getValue();
        
        // sample and holds
        // get a new sample on the rising edge at the sh control output
        if (virtualIO->shAState > shALast) {
            aSample = aIn;
        }
        if (virtualIO->shBState > shBLast) {
            bSample = bIn;
        }

        shALast = virtualIO->shAState;
        shBLast = virtualIO->shBState;

        // either use the sample or track depending on the sh control output
        aIn = virtualIO->shAState ? aSample : aIn;
        bIn = virtualIO->shBState ? bSample : bIn;

        // VCA/mixing stage
        // normalize 12 bits to 0-1
        outputs[MAIN_OUTPUT].setVoltage(bIn*(dac2Sample/4095.0) + aIn*(dac1Sample/4095.0)); 
        outputs[AUX_DAC_OUTPUT].setVoltage((dac3Sample/4095.0 - .5) * -10.666666666);
        outputs[LOGICA_OUTPUT].setVoltage(virtualIO->logicAState * 5.0);
        outputs[AUX_LOGIC_OUTPUT].setVoltage(virtualIO->auxLogicState * 5.0);

    }

    void updateAudioRate(void) {

        acquireCVs();

        processLogicInputs();

        updateOutputs();

        // updateLEDs();

        clockDivider = 0;

    };

    // minblep helpers

    int32_t crossed0(uint32_t lastPhase, int32_t increment) {

        int64_t currentPhase = (int64_t) lastPhase + (int64_t) increment;

        // printf("Current Phase: %llu \n", currentPhase);

        if (currentPhase >= ((int64_t)1 <<32)) {
            return 1;
        } else if (currentPhase < 0) {
            return -1;
        } else {
            return 0;
        }

    }

    int32_t crossed2(uint32_t lastPhase, int32_t increment) {

        int64_t currentPhase = (int64_t) lastPhase + (int64_t) increment;

        if ((currentPhase >= ((uint32_t)2 << 30)) && (lastPhase < ((uint32_t)2 << 30))) {
            return 1;
        } else if ((currentPhase < ((uint32_t)2 << 30)) && (lastPhase >= ((uint32_t)2 << 30))) {
            return -1;
        } else {
            return 0;
        }

    }


    // Parameter quantity stuff

    float reverseExpo(float expoScaled) {

        return log2(expoScaled/65536.0);

    }


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
                return string::f("%.*g", 2, v);
            } else {
                return string::f("%.*g", 2, v * 5.0);                
            }

        }

        std::string getString() override {

            if (!module)
                return "";

            if (bConnected()) {
                return getLabel() + " scale: " + getDisplayValueString();
            } else {
                return getLabel() + ": " + getDisplayValueString() + "V";                
            }

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

            float v = getSmoothValue();

            return string::f("%.*g", 2, v);                

        }

        std::string getString() override {

            if (!module)
                return "";

            Via * module = dynamic_cast<Via *>(this->module);

            bool aConnected = module->inputs[A_INPUT].isConnected();

            if (aConnected) {
                return "Overriden by A input";
            } else {
                return getLabel() + ": " + getDisplayValueString() + "V";                
            }
        }

    };

    struct ButtonQuantity : ParamQuantity {

        std::string getString() override {
            return getLabel();
        }

        void setDisplayValueString(std::string s) override {}
        
    };

    struct CV2ScaleQuantity : ParamQuantity {

        virtual void setLabel(void) {};

        std::string getDisplayValueString() override {

            float v = getSmoothValue();

            return string::f("%.*g", 3, v);                

        }

        std::string getString() override {

            if (!module)
                return "";

            Via * module = dynamic_cast<Via *>(this->module);

            bool connected = module->inputs[CV2_INPUT].isConnected();

            setLabel();

            if (!connected) {
                return "CV input unpatched";
            } else {
                return getLabel() + ": " + getDisplayValueString();              
            }

        }

    };

    struct CV3ScaleQuantity : ParamQuantity {

        virtual void setLabel(void) {};

        std::string getDisplayValueString() override {

            float v = getSmoothValue();

            return string::f("%.*g", 3, v);                

        }

        std::string getString() override {

            if (!module)
                return "";

            Via * module = dynamic_cast<Via *>(this->module);

            setLabel();

            bool connected = module->inputs[CV3_INPUT].isConnected();

            if (!connected) {
                return "CV input unpatched";
            } else {
                return getLabel() + ": " + getDisplayValueString();              
            }

        }

    };

};

