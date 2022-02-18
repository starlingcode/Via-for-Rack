#include "sync3xl.hpp"
#include "via_module.hpp"
#include "polyblamp.hpp"
#include "osdialog.h"

#include "trs.hpp"
#include "sync3xlexpand.hpp"

#define SYNC3_OVERSAMPLE_AMOUNT 24
#define SYNC3_OVERSAMPLE_QUALITY 6

struct Sync3XL : Via<SYNC3_OVERSAMPLE_AMOUNT, SYNC3_OVERSAMPLE_AMOUNT> {
    
    struct IRatioQuantity;
    struct IIRatioQuantity;
    struct IIIRatioQuantity;
    struct IButtonQuantity;
    struct RatioButtonQuantity;
    struct IIButtonQuantity;
    struct CVButtonQuantity;
    struct IIIButtonQuantity;

    Sync3XL() : Via(), virtualModule(asset::plugin(pluginInstance, "res/sync3scales.bin")) {

        virtualIO = &virtualModule;

		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam<IRatioQuantity>(KNOB1_PARAM, 0, 4095.0, 2048.0, "I Ratio");
        configParam<IIRatioQuantity>(KNOB2_PARAM, 0, 4095.0, 2048.0, "II Ratio");
        configParam<IIIRatioQuantity>(KNOB3_PARAM, 0, 4095.0, 2048.0, "III Ratio");
        configParam(CV1AMT_PARAM, -1.f, 1.0, 1.0, "I CV Scale");
        configParam(CV2AMT_PARAM, -1.f, 1.0, 1.0, "II CV Scale");
        configParam(CV3AMT_PARAM, -1.f, 1.0, 1.0, "III CV Scale");
        
        configParam<IButtonQuantity>(BUTTON1_PARAM, 0.0, 1.0, 0.0, "Oscillator I Shape");
        configParam<RatioButtonQuantity>(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Ratio Set");
        configParam<IIButtonQuantity>(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Oscillator II Shape");
        configParam<CVButtonQuantity>(BUTTON4_PARAM, 0.0, 1.0, 0.0, "CV Mapping");
        configParam<RatioButtonQuantity>(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Ratio Set");
        configParam<IIIButtonQuantity>(BUTTON6_PARAM, 0.0, 1.0, 0.0, "Oscillator III Shape");
        
		configParam(RES1_PARAM, -5.f, 5.f, 0.f, "I resonance");
		configParam(FREQ1_PARAM, -5.f, 5.f, 0.f, "I frequency");
		configParam(MODE1_PARAM, -5.f, 5.f, 0.f, "I filter type");
		configParam(RES2_PARAM, -5.f, 5.f, 0.f, "II resonance");
		configParam(FREQ2_PARAM, -5.f, 5.f, 0.f, "II frequency");
		configParam(MODE2_PARAM, -5.f, 5.f, 0.f, "II filter type");
		configParam(RES3_PARAM, -5.f, 5.f, 0.f, "III resonance");
		configParam(FREQ3_PARAM, -5.f, 5.f, 0.f, "III frequency");
		configParam(MODE3_PARAM, -5.f, 5.f, 0.f, "III filter type");
		configParam(RESAMT_PARAM, 0.f, 1.f, 0.f, "I/II/III resonance CV scale");
		configParam(FREQAMT_PARAM, 0.f, 1.f, 0.f, "I/II/III frequency CV scale");
		configParam(MODEAMT_PARAM, 0.f, 1.f, 0.f, "I/II/III filter type CV scale");
		configSwitch(RANGE_PARAM, 0.f, 1.f, 1.f, "I/II/III filter frequency range", {"Subaudio", "Audio"});
		configSwitch(TAP_TEMPO_PARAM, 0.f, 5.f, 0.f, "Tap tempo", {"Pressed", "Released"});
		configInput(CV1_INPUT, "I ratio CV");
		configInput(CV2_INPUT, "II ratio CV");
		configInput(CV3_INPUT, "III ratio CV");
		configInput(FREQ1CV_INPUT, "I cutoff CV");
		configInput(FREQ2CV_INPUT, "II cutoff CV");
		configInput(FREQ3CV_INPUT, "III cutoff CV");
		configInput(RESCV_INPUT, "I/II/III Resonance CV");
		configInput(MAIN_LOGIC_INPUT, "Sync");
		configInput(FREQCV_INPUT, "I/II/III resonance CV");
		configInput(MODECV_INPUT, "I/II/III resonance CV");
		configOutput(LOGIC1_OUTPUT, "I ratio change trigger");
		configOutput(OUT1_OUTPUT, "I");
		configOutput(LOGIC2_OUTPUT, "II ratio change trigger");
		configOutput(OUT2_OUTPUT, "II");
		configOutput(LOGIC3_OUTPUT, "III ratio change trigger");
		configOutput(OUT3_OUTPUT, "III");
		configOutput(LOGICMIX_OUTPUT, "I+II+III ratio change trigger");
		configOutput(OUTMIX_OUTPUT, "I+II+III");

        onSampleRateChange();
        virtualModule.displayRatio();

        rightExpander.producerMessage = new Sync3XLExpand; 
        rightExpander.consumerMessage = new Sync3XLExpand; 

    }

	enum ParamIds {
        KNOB1_PARAM,
        KNOB2_PARAM,
        KNOB3_PARAM,
		RANGE_PARAM,
		CV1AMT_PARAM,
        CV2AMT_PARAM,
        CV3AMT_PARAM,
        BUTTON1_PARAM,
        BUTTON2_PARAM,
        BUTTON3_PARAM,
        BUTTON4_PARAM,
        BUTTON5_PARAM,
        BUTTON6_PARAM,
        TRIG_BUTTON_PARAM,

		RES1_PARAM,
		FREQ1_PARAM,
		MODE1_PARAM,
		RES2_PARAM,
		FREQ2_PARAM,
		MODE2_PARAM,
		RES3_PARAM,
		FREQ3_PARAM,
		MODE3_PARAM,
		RESAMT_PARAM,
		FREQAMT_PARAM,
		MODEAMT_PARAM,
        TAP_TEMPO_PARAM,
		PARAMS_LEN
	};
	enum InputIds {
		FREQ1CV_INPUT,
		FREQ2CV_INPUT,
        CV1_INPUT,
        CV2_INPUT,
        CV3_INPUT,
        MAIN_LOGIC_INPUT,

		FREQ3CV_INPUT,
		RESCV_INPUT,
		FREQCV_INPUT,
		MODECV_INPUT,
		INPUTS_LEN
	};
	enum OutputIds {
		LOGIC1_OUTPUT,
		OUT1_OUTPUT,
		LOGIC2_OUTPUT,
		OUT2_OUTPUT,
		LOGIC3_OUTPUT,
		OUT3_OUTPUT,
		LOGICMIX_OUTPUT,
		OUTMIX_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightIds {
		LED1_LIGHT,
		LED2_LIGHT,
		LED3_LIGHT,
		LED4_LIGHT,
		LOGICLED1_LIGHT,
		LOGICLED2_LIGHT,
		LOGICLED3_LIGHT,
		SYNCLED_LIGHT,
		LOGICLEDMIX_LIGHT,
		RANGELOW_LIGHT,
        PM_LIGHT,
		LIGHTS_LEN
	};

    void process(const ProcessArgs &args) override;

    ViaSync3XL virtualModule;

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();

        ledDecay = 1.0/sampleRate;

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
        } else if (sampleRate == 352800.0) {
            divideAmount = 8;
        } else if (sampleRate == 384000.0) {
            divideAmount = 8;
        } else if (sampleRate == 705600.0) {
            divideAmount = 16;
        } else if (sampleRate == 768000.0) {
            divideAmount = 16;
        }
        
    }

    json_t *dataToJson() override {

        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "osc_modes", json_integer(virtualModule.sync3UI.modeStateBuffer));
        json_object_set_new(rootJ, "scale_file", json_string(scalePath.c_str()));
        
        return rootJ;
    }
    
    void dataFromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "osc_modes");
        virtualModule.sync3UI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.sync3UI.loadFromEEPROM(0);
        virtualModule.sync3UI.recallModuleState();
        virtualModule.sync3UI.defaultEnterMenuCallback();

        json_t *pathJ = json_object_get(rootJ, "scale_file");
        scalePath = json_string_value(pathJ);
        virtualModule.readScalesFromFile(scalePath);

    }

    std::string scalePath = asset::plugin(pluginInstance, "res/sync3scales.bin");

    void updateSlowIO(void) override {

        virtualIO->button1Input = (int32_t) params[BUTTON1_PARAM].getValue();
        virtualIO->button2Input = (int32_t) params[BUTTON2_PARAM].getValue();
        virtualIO->button3Input = (int32_t) params[BUTTON3_PARAM].getValue();
        virtualIO->button4Input = (int32_t) params[BUTTON4_PARAM].getValue();
        virtualIO->button5Input = (int32_t) params[BUTTON5_PARAM].getValue();
        virtualIO->button6Input = (int32_t) params[BUTTON6_PARAM].getValue();

        virtualIO->controls.controlRateInputs[2] = clamp((int32_t) (params[KNOB1_PARAM].getValue()), 0, 4095);
        virtualIO->controls.controlRateInputs[3] = clamp((int32_t) (params[KNOB2_PARAM].getValue()), 0, 4095);
        virtualIO->controls.controlRateInputs[1] = clamp((int32_t) (params[KNOB3_PARAM].getValue()), 0, 4095);
        float cv1Conversion = -inputs[CV1_INPUT].getVoltage() * params[CV1AMT_PARAM].getValue();
        cv1Conversion *= 2048.f/5.f;
        cv1Conversion += 2048;
        virtualIO->controls.controlRateInputs[0] = clamp((int32_t)cv1Conversion, 0, 4095);
        if (params[RANGE_PARAM].getValue() == 0.f) {
            subaudioFilter = 1;
        } else {
            subaudioFilter = 0;
        }
    }

    uint32_t logicOut1 = 0;
    uint32_t logicOut2 = 0;
    uint32_t logicOut3 = 0;

    void updateLEDs(void) override {

        lights[LED1_LIGHT].setSmoothBrightness((float) !virtualIO->ledAState, ledDecay);
        lights[LED2_LIGHT].setSmoothBrightness((float) !virtualIO->ledCState, ledDecay);
        lights[LED3_LIGHT].setSmoothBrightness((float) !virtualIO->ledDState, ledDecay);
        lights[LED4_LIGHT].setSmoothBrightness((float) !virtualIO->ledBState, ledDecay);

        lights[PM_LIGHT].setSmoothBrightness((float) virtualModule.tempPM, ledDecay);
        lights[SYNCLED_LIGHT].setSmoothBrightness((float) (virtualIO->greenLevelOut == 4095), ledDecay);
        lights[RANGELOW_LIGHT].setSmoothBrightness((float) subaudioFilter, ledDecay);
        lights[LOGICLED1_LIGHT].setSmoothBrightness((float) logicOut1, ledDecay);
        lights[LOGICLED2_LIGHT].setSmoothBrightness((float) logicOut2, ledDecay);
        lights[LOGICLED3_LIGHT].setSmoothBrightness((float) logicOut3, ledDecay);
        lights[LOGICLEDMIX_LIGHT].setSmoothBrightness((float) (logicOut1 | logicOut2 | logicOut3), ledDecay);
    }

    void processLogicInputs(void) override {

        float thisLogicIn = rescale(inputs[MAIN_LOGIC_INPUT].getVoltage() + params[TAP_TEMPO_PARAM].getValue(), .2, 1.2, 0.f, 1.f);
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

    }

    void updateOutputs(void) override {

        int32_t samplesRemaining = SYNC3_OVERSAMPLE_AMOUNT;
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
        
        virtualIO->halfTransferCallback();

        processOutputs(dac1Sample, dac2Sample, dac3Sample);

        updateLEDs();

    }

    int32_t optimize = 0;

    float lastDac1Phase = 0;
    float lastDac2Phase = 0;
    float lastDac3Phase = 0;

    dsp::MinBlepGenerator<8, 8, float> dac1MinBlep;
    dsp::MinBlepGenerator<8, 8, float> dac2MinBlep;
    dsp::MinBlepGenerator<8, 8, float> dac3MinBlep;

    PolyBlampGenerator<float> dac1PolyBlamp;
    PolyBlampGenerator<float> dac2PolyBlamp;
    PolyBlampGenerator<float> dac3PolyBlamp;

    float blampDelay1[3];
    float blampDelay2[3];
    float blampDelay3[3];

    void updateAudioRateEconomy(void) {

        acquireCVs();

        processLogicInputs();

        float dac1Sample = (float) virtualIO->outputs.dac1Samples[23];
        float dac2Sample = (float) virtualIO->outputs.dac2Samples[23];
        float dac3Sample = (float) virtualIO->outputs.dac3Samples[23];

        int32_t inc1 = (virtualModule.increment3 + virtualModule.phaseModIncrement2) * 24;
        int32_t inc2 = (virtualModule.increment4 + virtualModule.phaseModIncrement2) * 24;
        int32_t inc3 = virtualModule.increment2 * 24;


        if (virtualModule.sync3UI.button1Mode == 0) {

            int32_t crossingDirection = crossed0(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac3Phase) / (float) inc3;
                dac3MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            dac3Sample += dac3MinBlep.process();

        } else if (virtualModule.sync3UI.button1Mode == 1) {

            int32_t crossingDirection = crossed0(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac3Phase) / (float) inc3;
                dac3MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed2(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac3Phase) / (float) inc3;
                dac3MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac3Sample += dac3MinBlep.process();

        } else if (virtualModule.sync3UI.button1Mode == 2) {

            int32_t crossingDirection = crossed0(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - lastDac3Phase) / (float) inc3;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(inc3) / 4294967296.f));
            }

            crossingDirection = crossed2(lastDac3Phase, inc3);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac3Phase) / (float) inc3;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(inc3) / 4294967296.f));
            }

            float dac3Output = blampDelay3[0];
            blampDelay3[0] = blampDelay3[1];
            blampDelay3[1] = dac3Sample;
            dac3Sample = dac3Output + dac3PolyBlamp.process(); 

        }


        if (virtualModule.sync3UI.button3Mode == 0) {

            int32_t crossingDirection = crossed0(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac1Phase) / (float) inc1;
                dac1MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();

        } else if (virtualModule.sync3UI.button3Mode == 1) {

            int32_t crossingDirection = crossed0(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac1Phase) / (float) inc1;
                dac1MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed2(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac1Phase) / (float) inc1;
                dac1MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();

        } else if (virtualModule.sync3UI.button3Mode == 2) {

            int32_t crossingDirection = crossed0(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - lastDac1Phase) / (float) inc1;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(inc1) / 4294967296.f));
            }

            crossingDirection = crossed2(lastDac1Phase, inc1);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac1Phase) / (float) inc1;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(inc1) / 4294967296.f));
            }

            float dac1Output = blampDelay1[0];
            blampDelay1[0] = blampDelay1[1];
            blampDelay1[1] = dac1Sample;
            dac1Sample = dac1Output + dac1PolyBlamp.process(); 

        }


        if (virtualModule.sync3UI.button6Mode == 0) {

            int32_t crossingDirection = crossed0(lastDac2Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac2Phase) / (float) inc2;
                dac2MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();

        } else if (virtualModule.sync3UI.button6Mode == 1) {

            int32_t crossingDirection = crossed0(lastDac2Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac2Phase) / (float) inc2;
                dac2MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed2(lastDac1Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac2Phase) / (float) inc2;
                dac2MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();

        } else if (virtualModule.sync3UI.button6Mode == 2) {

            int32_t crossingDirection = crossed0(lastDac2Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - lastDac2Phase) / (float) inc2;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(inc2) / 4294967296.f));
            }

            crossingDirection = crossed2(lastDac2Phase, inc2);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac2Phase) / (float) inc2;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(inc2) / 4294967296.f));
            }

            float dac2Output = blampDelay2[0];
            blampDelay2[0] = blampDelay2[1];
            blampDelay2[1] = dac2Sample;
            dac2Sample = dac2Output + dac2PolyBlamp.process(); 

        }

        lastDac1Phase = virtualModule.phase3;
        lastDac2Phase = virtualModule.phase4;
        lastDac3Phase = virtualModule.phase2;

        
        virtualIO->halfTransferCallback();

        processOutputs(dac1Sample, dac2Sample, dac3Sample);

        updateLEDs();

        clockDivider = 0;

    }
    
    ZDFSVF<float> filters[3];

    bool subaudioFilter = 0;
    
    float filterVoltageToCoeff(float voltage) {

        float ts = APP->engine->getSampleTime();
        float freq = clamp(voltage, -10.f, 10.f);
        freq = float(480.f) * (dsp::approxExp2_taylor5(freq + 10.f)/float(1024.f)) * ts;
        if (subaudioFilter) {
            freq /= 1000;
        }
        return clamp(freq, 0.f, .49f);
    }

    float resVoltageToCoeff(float voltage) {
        float res = clamp(voltage, -10.f, 10.f);
        res /= 10.f;
        res += .5;
        res = clamp(res, 0.f, 1.f);
        res = dsp::approxExp2_taylor5((float(1.f) - res) * float(8.f)) / float(256.f);
        res = float(1.f) - res;
        return res;
    }

    float processFilterOuts(float modeVoltage, int filterIndex) { 
        modeVoltage /= 5.f;
        float lpScale = clamp(-modeVoltage, 0.f, 1.f);
        float bpScale =  1.f - clamp(abs(modeVoltage), 0.f, 1.f);
        float hpScale = clamp(modeVoltage, 0.f, 1.f);
        return filters[filterIndex].lpOut * lpScale + filters[filterIndex].bpOut * bpScale + filters[filterIndex].hpOut * hpScale; 
    }

    void processOutputs(float dac1, float dac2, float dac3) { 
 
        dac1 /= 4095.f;
        dac1 *= 10.f;
        dac1 -= 5.f;
        dac2 /= 4095.f;
        dac2 *= 10.f;
        dac2 -= 5.f;
        dac3 /= 4095.f;
        dac3 *= 10.f;
        dac3 -= 5.f;

        float out1 = dac3;
        float out2 = dac1;
        float out3 = dac2;

        float res = inputs[RESCV_INPUT].getVoltage() * params[RESAMT_PARAM].getValue();
        float freq = inputs[FREQCV_INPUT].getVoltage() * params[FREQAMT_PARAM].getValue();
        float mode = inputs[MODECV_INPUT].getVoltage() * params[MODEAMT_PARAM].getValue();

        float freq1 = freq + inputs[FREQ1CV_INPUT].getVoltage() + params[FREQ1_PARAM].getValue();
        float res1 = params[RES1_PARAM].getValue() + res;
        float mode1 = params[MODE1_PARAM].getValue() + mode;
        res1 = resVoltageToCoeff(res1);
        filters[0].setParams(filterVoltageToCoeff(freq1), res1);

        float freq2 = freq + inputs[FREQ2CV_INPUT].getVoltage() + params[FREQ2_PARAM].getValue();
        float res2 = params[RES2_PARAM].getValue() + res;
        float mode2 = params[MODE2_PARAM].getValue() + mode;
        res2 = resVoltageToCoeff(res2);
        filters[1].setParams(filterVoltageToCoeff(freq2), res2);

        float freq3 = freq + inputs[FREQ3CV_INPUT].getVoltage() + params[FREQ3_PARAM].getValue();
        float res3 = params[RES3_PARAM].getValue() + res;
        float mode3 = params[MODE3_PARAM].getValue() + mode;
        res3 = resVoltageToCoeff(res3);
        filters[2].setParams(filterVoltageToCoeff(freq3), res3);

        filters[0].process(out1 * (1.f - res1 * .9));
        filters[1].process(out2 * (1.f - res2 * .9));
        filters[2].process(out3 * (1.f - res3 * .9));

        out1 = processFilterOuts(mode1, 0);
        out2 = processFilterOuts(mode2, 1);
        out3 = processFilterOuts(mode3, 2);

        Sync3XLExpand* from_host = (Sync3XLExpand*) rightExpander.module->leftExpander.producerMessage;
        Sync3XLExpand* to_host = (Sync3XLExpand*) rightExpander.consumerMessage;

        float outMix;
        if (expanderAttached) {
            from_host->out1 = out1;
            from_host->out2 = out2;
            from_host->out3 = out3;
            out1 = to_host->out1;
            out2 = to_host->out2;
            out3 = to_host->out3;
            outMix = (out1 + out2 + out3)/3;
            from_host->mix = outMix;
            outMix = to_host->mix;
            rightExpander.module->leftExpander.messageFlipRequested = true;
        } else {
            outMix = (out1 + out2 + out3)/3;
        } 

        outputs[OUT1_OUTPUT].setVoltage(out1);
        outputs[OUT2_OUTPUT].setVoltage(out2);
        outputs[OUT3_OUTPUT].setVoltage(out3);
        outputs[OUTMIX_OUTPUT].setVoltage(outMix);
 
        logicOut1 = virtualIO->logicAState;
        logicOut2 = (virtualIO->redLevelOut == 4095);
        logicOut3 = (virtualIO->blueLevelOut == 4095);

        outputs[LOGIC1_OUTPUT].setVoltage(logicOut1 * 5);
        outputs[LOGIC2_OUTPUT].setVoltage(logicOut2 * 5);
        outputs[LOGIC3_OUTPUT].setVoltage(logicOut3 * 5);
        outputs[LOGICMIX_OUTPUT].setVoltage((logicOut1 | logicOut2 | logicOut3) * 5);
    }

    bool expanderAttached = false;

    void onExpanderChange(const ExpanderChangeEvent& e) override { 
        if (rightExpander.module && (rightExpander.module->model == modelSync3XLLevels)) {
            expanderAttached = true;
        } else {
            expanderAttached = false;
        }
    }

    ~Sync3XL() {
        free(rightExpander.producerMessage);
        free(rightExpander.consumerMessage);
    }   
};

void Sync3XL::process(const ProcessArgs &args) {

    clockDivider++;

    if (clockDivider >= divideAmount) {

        // update the "slow IO" (not audio rate) every 16 samples
        slowIOPrescaler++;
        if (slowIOPrescaler == 16) {
            slowIOPrescaler = 0;
            updateSlowIO();
            virtualModule.slowConversionCallback();
            virtualModule.ui_dispatch(SENSOR_EVENT_SIG);
            virtualModule.sync3UI.incrementTimer();
        }

        if (!optimize) {
            updateAudioRateEconomy();
        } else {
            updateAudioRate();
        }
        virtualModule.advanceMeasurementTimer();
        virtualModule.advanceTimer1();
        virtualModule.advanceTimer2();
    
        clockDivider = 0;

    }
    
}


struct Sync3XLWidget : ModuleWidget {
	Sync3XLWidget(Sync3XL* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/sync3xl.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		addParam(createParamCentered<TransparentButton>(mm2px(Vec(53.628,12.687)), module, Sync3XL::BUTTON1_PARAM));
		addParam(createParamCentered<ViaSifamBlack>(mm2px(Vec(43.315, 19.562)), module, Sync3XL::KNOB1_PARAM));
		addParam(createParamCentered<ViaSifamGrey>(mm2px(Vec(101.752,19.562)), module, Sync3XL::RES1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26.128,23.0)), module, Sync3XL::CV1AMT_PARAM));
		addParam(createParamCentered<ViaSifamGrey>(mm2px(Vec(77.69,26.437)), module, Sync3XL::FREQ1_PARAM));
		addParam(createParamCentered<ViaSifamGrey>(mm2px(Vec(125.815,26.437)), module, Sync3XL::MODE1_PARAM));
		addParam(createParamCentered<TransparentButton>(mm2px(Vec(53.628,40.188)), module, Sync3XL::BUTTON3_PARAM));
		addParam(createParamCentered<ViaSifamBlack>(mm2px(Vec(43.315, 47.062)), module, Sync3XL::KNOB2_PARAM));
		addParam(createParamCentered<ViaSifamGrey>(mm2px(Vec(101.752,47.062)), module, Sync3XL::RES2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26.128,50.5)), module, Sync3XL::CV2AMT_PARAM));
		addParam(createParamCentered<ViaSifamGrey>(mm2px(Vec(77.69,53.938)), module, Sync3XL::FREQ2_PARAM));
		addParam(createParamCentered<ViaSifamGrey>(mm2px(Vec(125.815,53.938)), module, Sync3XL::MODE2_PARAM));
		addParam(createParamCentered<TransparentButton>(mm2px(Vec(26.128,64.25)), module, Sync3XL::BUTTON4_PARAM));
		addParam(createParamCentered<TransparentButton>(mm2px(Vec(53.628,67.688)), module, Sync3XL::BUTTON6_PARAM));
		addParam(createParamCentered<ViaSifamBlack>(mm2px(Vec(43.315, 74.562)), module, Sync3XL::KNOB3_PARAM));
		addParam(createParamCentered<ViaSifamGrey>(mm2px(Vec(101.752,74.562)), module, Sync3XL::RES3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(26.128,78.0)), module, Sync3XL::CV3AMT_PARAM));
		addParam(createParamCentered<ViaSifamGrey>(mm2px(Vec(77.69,81.438)), module, Sync3XL::FREQ3_PARAM));
		addParam(createParamCentered<ViaSifamGrey>(mm2px(Vec(125.815,81.438)), module, Sync3XL::MODE3_PARAM));
		addParam(createParamCentered<TransparentButton>(mm2px(Vec(43.315,98.625)), module, Sync3XL::BUTTON2_PARAM));
		addParam(createParamCentered<ViaPushButton>(mm2px(Vec(19.253,98.625)), module, Sync3XL::TAP_TEMPO_PARAM));
		addParam(createParamCentered<NKK_2>(mm2px(Vec(63.94,105.5)), module, Sync3XL::RANGE_PARAM));
		addParam(createParamCentered<TransparentButton>(mm2px(Vec(43.315,112.375)), module, Sync3XL::BUTTON5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(77.69,98.625)), module, Sync3XL::FREQAMT_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(101.752,95.188)), module, Sync3XL::RESAMT_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(125.815,98.625)), module, Sync3XL::MODEAMT_PARAM));

		addInput(createInputCentered<ViaJack>(mm2px(Vec(8.94,26.437)), module, Sync3XL::CV1_INPUT));
		addInput(createInputCentered<ViaJack>(mm2px(Vec(63.94,26.57)), module, Sync3XL::FREQ1CV_INPUT));
		addInput(createInputCentered<ViaJack>(mm2px(Vec(8.94,53.938)), module, Sync3XL::CV2_INPUT));
		addInput(createInputCentered<ViaJack>(mm2px(Vec(63.94,53.839)), module, Sync3XL::FREQ2CV_INPUT));
		addInput(createInputCentered<ViaJack>(mm2px(Vec(8.94,81.438)), module, Sync3XL::CV3_INPUT));
		addInput(createInputCentered<ViaJack>(mm2px(Vec(63.94,81.339)), module, Sync3XL::FREQ3CV_INPUT));
		addInput(createInputCentered<ViaJack>(mm2px(Vec(101.752,108.938)), module, Sync3XL::RESCV_INPUT));
		addInput(createInputCentered<ViaJack>(mm2px(Vec(8.94, 108.938)), module, Sync3XL::MAIN_LOGIC_INPUT));
		addInput(createInputCentered<ViaJack>(mm2px(Vec(77.789,112.375)), module, Sync3XL::FREQCV_INPUT));
		addInput(createInputCentered<ViaJack>(mm2px(Vec(125.815,112.375)), module, Sync3XL::MODECV_INPUT));

		addOutput(createOutputCentered<ViaJack>(mm2px(Vec(143.002,16.125)), module, Sync3XL::LOGIC1_OUTPUT));
		addOutput(createOutputCentered<ViaJack>(mm2px(Vec(153.315,26.437)), module, Sync3XL::OUT1_OUTPUT));
		addOutput(createOutputCentered<ViaJack>(mm2px(Vec(143.002,43.625)), module, Sync3XL::LOGIC2_OUTPUT));
		addOutput(createOutputCentered<ViaJack>(mm2px(Vec(153.315,53.938)), module, Sync3XL::OUT2_OUTPUT));
		addOutput(createOutputCentered<ViaJack>(mm2px(Vec(143.002,71.125)), module, Sync3XL::LOGIC3_OUTPUT));
		addOutput(createOutputCentered<ViaJack>(mm2px(Vec(153.315,81.438)), module, Sync3XL::OUT3_OUTPUT));
		addOutput(createOutputCentered<ViaJack>(mm2px(Vec(143.002,98.625)), module, Sync3XL::LOGICMIX_OUTPUT));
		addOutput(createOutputCentered<ViaJack>(mm2px(Vec(153.315,108.938)), module, Sync3XL::OUTMIX_OUTPUT));

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(143.002,23.0)), module, Sync3XL::LOGICLED1_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(143.002,50.5)), module, Sync3XL::LOGICLED2_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(143.002,78.0)), module, Sync3XL::LOGICLED3_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(26.128,91.75)), module, Sync3XL::SYNCLED_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(36.44,98.625)), module, Sync3XL::LED1_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(36.44,112.375)), module, Sync3XL::LED4_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(50.19,98.625)), module, Sync3XL::LED2_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(50.19,112.375)), module, Sync3XL::LED3_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(143.002,105.5)), module, Sync3XL::LOGICLEDMIX_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(63.94,115.812)), module, Sync3XL::RANGELOW_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(19.253,71.125)), module, Sync3XL::PM_LIGHT));

	}

    void appendContextMenu(Menu *menu) override {
        Sync3XL *module = dynamic_cast<Sync3XL*>(this->module);

        struct PresetRecallItem : MenuItem {
            Sync3XL *module;
            int preset;
            void onAction(const event::Action &e) override {
                module->virtualModule.sync3UI.modeStateBuffer = preset;
                module->virtualModule.sync3UI.loadFromEEPROM(0);
                module->virtualModule.sync3UI.recallModuleState();
            }
        };

        struct OptimizationHandler : MenuItem {
            Sync3XL *module;
            int32_t optimization;
            void onAction(const event::Action &e) override {
                module->optimize = optimization;
            }
        };

        struct ScaleSetHandler : MenuItem {
            Sync3XL *module; 
            void onAction(const event::Action &e) override {
             
                char* pathC = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL); 
                if (!pathC) { 
                    // Fail silently 
                    return; 
                } 
                DEFER({ 
                    std::free(pathC); 
                }); 
             
                module->virtualModule.readScalesFromFile(pathC);
                module->scalePath = pathC;
            }
        };
        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("CPU Mode"));
        const std::string optimization[] = {
            "Optimized",
            "Direct Port",
        };
        for (int i = 0; i < (int) LENGTHOF(optimization); i++) {
            OptimizationHandler *menuItem = createMenuItem<OptimizationHandler>(optimization[i], CHECKMARK(module->optimize == i));
            menuItem->module = module;
            menuItem->optimization = i;
            menu->addChild(menuItem);
        }
        ScaleSetHandler *menuItem = createMenuItem<ScaleSetHandler>("Select Scale Set File");
        menuItem->module = module;
        menu->addChild(menuItem);

    }

};


Model* modelSync3XL = createModel<Sync3XL, Sync3XLWidget>("SYNC3XL");

// Tooltip definitions

struct Sync3XL::IRatioQuantity : ViaKnobQuantity {

    std::string getDisplayValueString(void) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        return string::f("%d", sync3Module->virtualModule.numerator1Alt) + "/" + 
                    string::f("%d", sync3Module->virtualModule.denominator1Select);

    }

};

struct Sync3XL::IIRatioQuantity : ViaKnobQuantity {

    std::string getDisplayValueString(void) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        return string::f("%i", sync3Module->virtualModule.numerator2Alt) + "/" + 
                    string::f("%i", sync3Module->virtualModule.denominator2Select);
    }

};

struct Sync3XL::IIIRatioQuantity : ViaKnobQuantity {

    std::string getDisplayValueString(void) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        return string::f("%i", sync3Module->virtualModule.numerator3Alt) + "/" + 
                    string::f("%i", sync3Module->virtualModule.denominator3Select);
    }

};

struct Sync3XL::IButtonQuantity : ViaButtonQuantity<3> {

    std::string buttonModes[3] = {"Saw", "Square", "Triangle"};

    IButtonQuantity() {
        for (int i = 0; i < 3; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        return sync3Module->virtualModule.sync3UI.button1Mode;

    }

    void setMode(int mode) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        sync3Module->virtualModule.sync3UI.button1Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button1Mode, BUTTON1_MASK, BUTTON1_SHIFT);
        sync3Module->virtualModule.handleButton1ModeChange(mode);

    }

};

struct Sync3XL::RatioButtonQuantity : ViaButtonQuantity<8> {

    std::string buttonModes[8] = {"Scale 1", "Scale 2", "Scale 3", "Scale 4", "Scale 5", "Scale 6", "Scale 7", "Scale 8"};

    RatioButtonQuantity() {
        for (int i = 0; i < 8; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        return sync3Module->virtualModule.sync3UI.button2Mode;

    }

    void setMode(int mode) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        sync3Module->virtualModule.sync3UI.button2Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button2Mode, BUTTON2_MASK, BUTTON2_SHIFT);
        sync3Module->virtualModule.handleButton2ModeChange(mode);

    }

};

struct Sync3XL::IIButtonQuantity : ViaButtonQuantity<3> {

    std::string buttonModes[3] = {"Saw", "Square", "Triangle"};

    IIButtonQuantity() {
        for (int i = 0; i < 3; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        return sync3Module->virtualModule.sync3UI.button3Mode;

    }

    void setMode(int mode) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        sync3Module->virtualModule.sync3UI.button3Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button3Mode, BUTTON3_MASK, BUTTON3_SHIFT);
        sync3Module->virtualModule.handleButton3ModeChange(mode);

    }

};

struct Sync3XL::CVButtonQuantity : ViaButtonQuantity<2> {

    std::string buttonModes[2] = {"Independent Ratios", "II + III and Phase Modulation"};

    CVButtonQuantity() {
        for (int i = 0; i < 2; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        return sync3Module->virtualModule.sync3UI.button4Mode;

    }

    void setMode(int mode) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        sync3Module->virtualModule.sync3UI.button4Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button4Mode, BUTTON4_MASK, BUTTON4_SHIFT);
        sync3Module->virtualModule.handleButton4ModeChange(mode);

    }

};

struct Sync3XL::IIIButtonQuantity : ViaButtonQuantity<3> {

    std::string buttonModes[3] = {"Saw", "Square", "Triangle"};

    IIIButtonQuantity() {
        for (int i = 0; i < 3; i++) {
            modes[i] = buttonModes[i];
        }
    }
    
    int getModeEnumeration(void) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        return sync3Module->virtualModule.sync3UI.button6Mode;

    }

    void setMode(int mode) override {

        Sync3XL * sync3Module = dynamic_cast<Sync3XL *>(this->module);

        sync3Module->virtualModule.sync3UI.button6Mode = mode;
        sync3Module->virtualModule.sync3UI.storeMode(sync3Module->virtualModule.sync3UI.button6Mode, BUTTON6_MASK, BUTTON6_SHIFT);
        sync3Module->virtualModule.handleButton6ModeChange(mode);

    }

};

