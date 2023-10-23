#include "osc3.hpp"
#include "via-module.hpp"
#include "starling-dsp.hpp"
#include "osdialog.h"

#define OSC3_OVERSAMPLE_AMOUNT 32
#define OSC3_OVERSAMPLE_QUALITY 6

struct Osc3 : Via<OSC3_OVERSAMPLE_AMOUNT, OSC3_OVERSAMPLE_AMOUNT> {

    struct FreqKnobQuantity;
    struct DetuneKnobQuantity;
    struct OctaveButtonQuantity;
    struct WaveshapeButtonQuantity;
    struct SHButtonQuantity;
    struct QuantizationButtonQuantity;
    struct DetuneButtonQuantity;

    float effectiveSR = 48000.0f;

    Osc3() : Via(), virtualModule(asset::plugin(pluginInstance, "res/original.osc3")) {

        virtualIO = &virtualModule;

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam<FreqKnobQuantity>(KNOB1_PARAM, 0, 4095.0, 2048.0, "Base frequency", "Hz");
        configParam<FreqKnobQuantity>(KNOB2_PARAM, 0, 4095.0, 2048.0, "Base frequency", "Hz");
        configParam<DetuneKnobQuantity>(KNOB3_PARAM, 0, 4095.0, 2048.0, "Detune");
        configParam<BScaleQuantity>(B_PARAM, -1.0, 1.0, -1.0, "Oscillator 3 level");
        configParam<CV2ScaleQuantity>(CV2AMT_PARAM, 0, 1.0, 1.0, "2 + 3 Phase CV scale");
        configParam<ANormalQuantity>(A_PARAM, -5.0, 5.0, 5.0, "Oscillator 2 level");
        configParam<CV3ScaleQuantity>(CV3AMT_PARAM, 0, 1.0, 1.0, "Detune CV scale");

        configParam<OctaveButtonQuantity>(BUTTON1_PARAM, 0.0, 1.0, 0.0, "Octave offset");
        configParam<WaveshapeButtonQuantity>(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Waveshape");
        configParam<SHButtonQuantity>(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Osc1 -> 2 and 3 level SH");
        configParam<OctaveButtonQuantity>(BUTTON4_PARAM, 0.0, 1.0, 0.0, "Octave offset");
        configParam<QuantizationButtonQuantity>(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Quantization");
        configParam<DetuneButtonQuantity>(BUTTON6_PARAM, 0.0, 1.0, 0.0, "Beat/detune mode");

        configParam(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Unity");

        configInput(A_INPUT, "Oscillator 2 level");
        configInput(B_INPUT, "Oscillator 3 level");
        configInput(CV1_INPUT, "V/oct");
        configInput(CV2_INPUT, "Phase");
        configInput(CV3_INPUT, "Detune");
        configInput(MAIN_LOGIC_INPUT, "Octave shift");
        configInput(AUX_LOGIC_INPUT, "Unity");

        configOutput(MAIN_OUTPUT, "Detuned output");
        configOutput(LOGICA_OUTPUT, "Beat tracking gate");
        configOutput(AUX_DAC_OUTPUT, "Oscillator 1 output");
        configOutput(AUX_LOGIC_OUTPUT, "Pitch change trigger");

        onSampleRateChange();

    }
    void process(const ProcessArgs &args) override;

    ViaOsc3 virtualModule;

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();

        ledDecay = 1.0/sampleRate;

        if (sampleRate == 44100.0) {
            divideAmount = 1;
            effectiveSR = 44100.0;
            virtualModule.absoluteTune = 49773;
        } else if (sampleRate == 48000.0) {
            divideAmount = 1;
            effectiveSR = 48000.0;
            virtualModule.absoluteTune = 45729;
        } else if (sampleRate == 88200.0) {
            divideAmount = 2;
            effectiveSR = 44100.0;
            virtualModule.absoluteTune = 49773;
        } else if (sampleRate == 96000.0) {
            divideAmount = 2;
            effectiveSR = 48000.0;
            virtualModule.absoluteTune = 45729;
        } else if (sampleRate == 176400.0) {
            divideAmount = 4;
            effectiveSR = 44100.0;
            virtualModule.absoluteTune = 49773;
        } else if (sampleRate == 192000.0) {
            divideAmount = 4;
            effectiveSR = 48000.0;
            virtualModule.absoluteTune = 45729;
        } else if (sampleRate == 352800.0) {
            divideAmount = 8;
            effectiveSR = 44100.0;
            virtualModule.absoluteTune = 49773;
        } else if (sampleRate == 384000.0) {
            divideAmount = 8;
            effectiveSR = 48000.0;
            virtualModule.absoluteTune = 45729;
        } else if (sampleRate == 705600.0) {
            divideAmount = 16;
            effectiveSR = 44100.0;
            virtualModule.absoluteTune = 49773;
        } else if (sampleRate == 768000.0) {
            divideAmount = 16;
            effectiveSR = 48000.0;
            virtualModule.absoluteTune = 45729;
        }

    }

    json_t *dataToJson() override {

        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "osc_modes", json_integer(virtualModule.osc3UI.modeStateBuffer));
        json_object_set_new(rootJ, "optimization", json_integer(optimize));
        json_object_set_new(rootJ, "scale_file", json_string(scalePath.c_str()));

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override {

        json_t *opt = json_object_get(rootJ, "optimization");
        if (opt) {
            optimize = json_integer_value(opt);
        }

        json_t *modesJ = json_object_get(rootJ, "osc_modes");
        if (modesJ) {
            virtualModule.osc3UI.modeStateBuffer = json_integer_value(modesJ);
            virtualModule.osc3UI.loadFromEEPROM(0);
            virtualModule.osc3UI.recallModuleState();
        }

        json_t *pathJ = json_object_get(rootJ, "scale_file");
        if (pathJ) {
            scalePath = json_string_value(pathJ);
            virtualModule.readScalesFromFile(scalePath);
        }
    }

    std::string scalePath = asset::plugin(pluginInstance, "res/original.osc3");

    int32_t optimize = 0;

    int32_t crossed1_8(uint32_t lastPhase, int32_t increment) {

        int64_t currentPhase = (int64_t) lastPhase + (int64_t) increment;

        if ((currentPhase >= ((uint32_t)1 << 29)) && (lastPhase < ((uint32_t)1 << 29))) {
            return 1;
        } else if ((currentPhase < ((uint32_t)1 << 29)) && (lastPhase >= ((uint32_t)1 << 29))) {
            return -1;
        } else {
            return 0;
        }

    }

    int32_t crossed3_8(uint32_t lastPhase, int32_t increment) {

        int64_t currentPhase = (int64_t) lastPhase + (int64_t) increment;

        if ((currentPhase >= ((uint32_t)3 << 29)) && (lastPhase < ((uint32_t)3 << 29))) {
            return 1;
        } else if ((currentPhase < ((uint32_t)3 << 29)) && (lastPhase >= ((uint32_t)3 << 29))) {
            return -1;
        } else {
            return 0;
        }

    }

    int32_t crossed5_8(uint32_t lastPhase, int32_t increment) {

        int64_t currentPhase = (int64_t) lastPhase + (int64_t) increment;

        if ((currentPhase >= ((uint32_t)5 << 29)) && (lastPhase < ((uint32_t)5 << 29))) {
            return 1;
        } else if ((currentPhase < ((uint32_t)5 << 29)) && (lastPhase >= ((uint32_t)5 << 29))) {
            return -1;
        } else {
            return 0;
        }

    }

    int32_t crossed7_8(uint32_t lastPhase, int32_t increment) {

        int64_t currentPhase = (int64_t) lastPhase + (int64_t) increment;

        if ((currentPhase >= ((uint32_t)7 << 29)) && (lastPhase < ((uint32_t)7 << 29))) {
            return 1;
        } else if ((currentPhase < ((uint32_t)7 << 29)) && (lastPhase >= ((uint32_t)7 << 29))) {
            return -1;
        } else {
            return 0;
        }

    }

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

        float dac1Sample = (float) virtualIO->outputs.dac1Samples[31];
        float dac2Sample = (float) virtualIO->outputs.dac2Samples[31];
        float dac3Sample = (float) virtualIO->outputs.dac3Samples[31];

        int32_t aInc = (virtualModule.aFreq + virtualModule.pm) * 32;
        int32_t bInc = (virtualModule.bFreq + virtualModule.pm) * 32;
        int32_t cInc = virtualModule.cFreq * 32;

        if (virtualModule.osc3UI.button2Mode == 0) {

            int32_t crossingDirection = crossed0(lastDac1Phase, aInc);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac1Phase) / (float) aInc;
                dac1MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed0(lastDac2Phase, bInc);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac2Phase) / (float) bInc;
                dac2MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed0(lastDac3Phase, cInc);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac3Phase) / (float) cInc;
                dac3MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();
            dac2Sample += dac2MinBlep.process();
            dac3Sample += dac3MinBlep.process();

        } else if (virtualModule.osc3UI.button2Mode == 1) {

            int32_t crossingDirection = crossed0(lastDac1Phase, aInc);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac1Phase) / (float) aInc;
                dac1MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed0(lastDac2Phase, bInc);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac2Phase) / (float) bInc;
                dac2MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed0(lastDac3Phase, cInc);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac3Phase) / (float) cInc;
                dac3MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed2(lastDac1Phase, aInc);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac1Phase) / (float) aInc;
                dac1MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed2(lastDac2Phase, bInc);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - (float)lastDac2Phase) / (float) bInc;
                dac2MinBlep.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * (float) crossingDirection);
            }

            crossingDirection = crossed2(lastDac3Phase, cInc);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - (float)lastDac3Phase) / (float) cInc;
                dac3MinBlep.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * (float) crossingDirection);
            }

            dac1Sample += dac1MinBlep.process();
            dac2Sample += dac2MinBlep.process();
            dac3Sample += dac3MinBlep.process();

        } else if (virtualModule.osc3UI.button2Mode == 2) {

            int32_t crossingDirection = crossed1_8(lastDac1Phase, aInc);
            if (crossingDirection) {
                float deltaPhase = (536870912.f - (float)lastDac1Phase) / (float) aInc;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(aInc) / 4294967296.f));
            }

            crossingDirection = crossed1_8(lastDac2Phase, bInc);
            if (crossingDirection) {
                float deltaPhase = (536870912.f - (float)lastDac2Phase) / (float) bInc;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(bInc) / 4294967296.f));
            }

            crossingDirection = crossed1_8(lastDac3Phase, cInc);
            if (crossingDirection) {
                float deltaPhase = (536870912.f - (float)lastDac3Phase) / (float) cInc;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(cInc) / 4294967296.f));
            }


            crossingDirection = crossed3_8(lastDac1Phase, aInc);
            if (crossingDirection) {
                float deltaPhase = (1610612736.f - (float)lastDac1Phase) / (float) aInc;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(aInc) / 4294967296.f));
            }

            crossingDirection = crossed3_8(lastDac2Phase, bInc);
            if (crossingDirection) {
                float deltaPhase = (1610612736.f - (float)lastDac2Phase) / (float) bInc;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(bInc) / 4294967296.f));
            }

            crossingDirection = crossed3_8(lastDac3Phase, cInc);
            if (crossingDirection) {
                float deltaPhase = (1610612736.f - (float)lastDac3Phase) / (float) cInc;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(cInc) / 4294967296.f));
            }


            crossingDirection = crossed5_8(lastDac1Phase, aInc);
            if (crossingDirection) {
                float deltaPhase = (2684354560.f - lastDac1Phase) / (float) aInc;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(aInc) / 4294967296.f));
            }

            crossingDirection = crossed5_8(lastDac2Phase, bInc);
            if (crossingDirection) {
                float deltaPhase = (2684354560.f - (float)lastDac2Phase) / (float) bInc;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(bInc) / 4294967296.f));
            }

            crossingDirection = crossed5_8(lastDac3Phase, cInc);
            if (crossingDirection) {
                float deltaPhase = (2684354560.f - (float)lastDac3Phase) / (float) cInc;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(cInc) / 4294967296.f));
            }


            crossingDirection = crossed7_8(lastDac1Phase, aInc);
            if (crossingDirection) {
                float deltaPhase = (3758096384.f - lastDac1Phase) / (float) aInc;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(aInc) / 4294967296.f));
            }

            crossingDirection = crossed7_8(lastDac2Phase, bInc);
            if (crossingDirection) {
                float deltaPhase = (3758096384.f - (float)lastDac2Phase) / (float) bInc;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(bInc) / 4294967296.f));
            }

            crossingDirection = crossed7_8(lastDac3Phase, cInc);
            if (crossingDirection) {
                float deltaPhase = (3758096384.f - (float)lastDac3Phase) / (float) cInc;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(cInc) / 4294967296.f));
            }

            float dac1Output = blampDelay1[0];
            float dac2Output = blampDelay2[0];
            float dac3Output = blampDelay3[0];

            blampDelay1[0] = blampDelay1[1];
            blampDelay2[0] = blampDelay2[1];
            blampDelay3[0] = blampDelay3[1];

            blampDelay1[1] = dac1Sample;
            blampDelay2[1] = dac2Sample;
            blampDelay3[1] = dac3Sample;

            dac1Sample = dac1Output + dac1PolyBlamp.process();
            dac2Sample = dac2Output + dac2PolyBlamp.process();
            dac3Sample = dac3Output + dac3PolyBlamp.process();

        } else if (virtualModule.osc3UI.button2Mode == 3) {

            int32_t crossingDirection = crossed0(lastDac1Phase, aInc);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - lastDac1Phase) / (float) aInc;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(aInc) / 4294967296.f));
            }

            crossingDirection = crossed0(lastDac2Phase, bInc);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac2Phase) / (float) bInc;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(bInc) / 4294967296.f));
            }

            crossingDirection = crossed0(lastDac3Phase, cInc);
            if (crossingDirection) {
                float deltaPhase = (4294967296.f * (crossingDirection == 1) - (float)lastDac3Phase) / (float) cInc;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(cInc) / 4294967296.f));
            }

            crossingDirection = crossed2(lastDac1Phase, aInc);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - lastDac1Phase) / (float) aInc;
                dac1PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(aInc) / 4294967296.f));
            }

            crossingDirection = crossed2(lastDac2Phase, bInc);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - (float)lastDac2Phase) / (float) bInc;
                dac2PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, -4095.0 * 4.0f * ((float) abs(bInc) / 4294967296.f));
            }

            crossingDirection = crossed2(lastDac3Phase, cInc);
            if (crossingDirection) {
                float deltaPhase = (2147483648.f - (float)lastDac3Phase) / (float) cInc;
                dac3PolyBlamp.insertDiscontinuity(deltaPhase - 1.0f, 4095.0 * 4.0f * ((float) abs(cInc) / 4294967296.f));
            }

            float dac1Output = blampDelay1[0];
            float dac2Output = blampDelay2[0];
            float dac3Output = blampDelay3[0];

            blampDelay1[0] = blampDelay1[1];
            blampDelay2[0] = blampDelay2[1];
            blampDelay3[0] = blampDelay3[1];

            blampDelay1[1] = dac1Sample;
            blampDelay2[1] = dac2Sample;
            blampDelay3[1] = dac3Sample;

            dac1Sample = dac1Output + dac1PolyBlamp.process();
            dac2Sample = dac2Output + dac2PolyBlamp.process();
            dac3Sample = dac3Output + dac3PolyBlamp.process();

        }

        lastDac1Phase = virtualModule.aPhase;
        lastDac2Phase = virtualModule.bPhase;
        lastDac3Phase = virtualModule.cPhase;


        // if (virtualModule.osc3UI.button2Mode == 0) {
        //     if (dac1Sample)
        // }

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

        updateLEDs();

        clockDivider = 0;

    }

};

void Osc3::process(const ProcessArgs &args) {

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
            virtualModule.osc3UI.incrementTimer();
            processTriggerButton();
            updateLEDs();
        }

        if (!optimize) {
            updateAudioRateEconomy();
        } else {
            updateAudioRate();
        }
        virtualModule.advanceMeasurementTimer();

    }

}

struct Osc3Widget : ModuleWidget  {

    Osc3Widget(Osc3 *module) {

	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/osc3.svg")));

        setModule(module);

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<SifamBlack>(Vec(9.022 + .753, 30.90), module, Osc3::KNOB1_PARAM));
        addParam(createParam<SifamBlack>(Vec(68.53 + .753, 30.90), module, Osc3::KNOB2_PARAM));
        addParam(createParam<SifamBlack>(Vec(68.53 + .753, 169.89), module, Osc3::KNOB3_PARAM));
        addParam(createParam<SifamGrey>(Vec(9.022 + .753, 169.89), module, Osc3::B_PARAM));
        addParam(createParam<SifamBlack>(Vec(128.04 + .753, 30.90), module, Osc3::CV2AMT_PARAM));
        addParam(createParam<SifamGrey>(Vec(128.04 + .753, 100.4), module, Osc3::A_PARAM));
        addParam(createParam<SifamBlack>(Vec(128.04 + .753, 169.89), module, Osc3::CV3AMT_PARAM));

        addParam(createParam<TransparentButton>(Vec(10.5 + .753, 88), module, Osc3::BUTTON1_PARAM));
        addParam(createParam<TransparentButton>(Vec(47 + .753, 77.5), module, Osc3::BUTTON2_PARAM));
        addParam(createParam<TransparentButton>(Vec(85 + .753, 90), module, Osc3::BUTTON3_PARAM));
        addParam(createParam<TransparentButton>(Vec(10.5 + .753, 129), module, Osc3::BUTTON4_PARAM));
        addParam(createParam<TransparentButton>(Vec(47 + .753, 133.5), module, Osc3::BUTTON5_PARAM));
        addParam(createParam<TransparentButton>(Vec(85 + .753, 129), module, Osc3::BUTTON6_PARAM));

        addParam(createParam<ViaPushButton>(Vec(132.7 + .753, 320), module, Osc3::TRIGBUTTON_PARAM));

        addInput(createInput<HexJack>(Vec(8.07 + 1.053, 241.12), module, Osc3::A_INPUT));
        addInput(createInput<HexJack>(Vec(8.07 + 1.053, 282.62), module, Osc3::B_INPUT));
        addInput(createInput<HexJack>(Vec(8.07 + 1.053, 324.02), module, Osc3::MAIN_LOGIC_INPUT));
        addInput(createInput<HexJack>(Vec(45.75 + 1.053, 241.12), module, Osc3::CV1_INPUT));
        addInput(createInput<HexJack>(Vec(45.75 + 1.053, 282.62), module, Osc3::CV2_INPUT));
        addInput(createInput<HexJack>(Vec(45.75 + 1.053, 324.02), module, Osc3::CV3_INPUT));
        addInput(createInput<HexJack>(Vec(135 + 1.053, 282.62), module, Osc3::AUX_LOGIC_INPUT));

        addOutput(createOutput<HexJack>(Vec(83.68 + 1.053, 241.12), module, Osc3::LOGICA_OUTPUT));
        addOutput(createOutput<HexJack>(Vec(83.68 + 1.053, 282.62), module, Osc3::AUX_DAC_OUTPUT));
        addOutput(createOutput<HexJack>(Vec(83.68 + 1.053, 324.02), module, Osc3::MAIN_OUTPUT));
        addOutput(createOutput<HexJack>(Vec(135 + 1.053, 241.12), module, Osc3::AUX_LOGIC_OUTPUT));

        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.9 + .753, 268.5), module, Osc3::LED1_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.8 + .753, 268.5), module, Osc3::LED2_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.9 + .753, 309.8), module, Osc3::LED3_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.8 + .753, 309.8), module, Osc3::LED4_LIGHT));
        addChild(createLight<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Osc3::OUTPUT_GREEN_LIGHT));
        addChild(createLight<LargeSimpleLight<RGBTriangle>>(Vec(59 + .753, 221), module, Osc3::RED_LIGHT));

    };

    void appendContextMenu(Menu *menu) override {
        Osc3 *module = dynamic_cast<Osc3*>(this->module);

        struct PresetRecallItem : MenuItem {
            Osc3 *module;
            int preset;
            void onAction(const event::Action &e) override {
                module->virtualModule.osc3UI.modeStateBuffer = preset;
                module->virtualModule.osc3UI.loadFromEEPROM(0);
                module->virtualModule.osc3UI.recallModuleState();
            }
        };

        struct OptimizationHandler : MenuItem {
            Osc3 *module;
            int32_t optimization;
            void onAction(const event::Action &e) override {
                module->optimize = optimization;
            }
        };

        struct ScaleSetHandler : MenuItem {
            Osc3 *module;
            void onAction(const event::Action &e) override {
#ifdef USING_CARDINAL_NOT_RACK
                Osc3* module = this->module;
                async_dialog_filebrowser(false, nullptr, nullptr, "Load Scale", [module](char* pathC) {
                    pathSelected(module, pathC);
                });
#else
                char* pathC = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
                pathSelected(module, pathC);
#endif
            }

            static void pathSelected(Osc3* module, char* pathC) {
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

Model *modelOsc3 = createModel<Osc3, Osc3Widget>("OSC3");

// Tooltip definitions

struct Osc3::FreqKnobQuantity: ViaKnobQuantity {

    void setDisplayValueString(std::string s) override {

        if (string::startsWith(s, "a#") || string::startsWith(s, "A#")) {
            setDisplayValue(29.14 * pow(2, std::stof(s.substr(2, 1))));
        } else if (string::startsWith(s, "a") || string::startsWith(s, "A")) {
            setDisplayValue(27.50 * pow(2, std::stof(s.substr(1, 1))));
        } else if (string::startsWith(s, "b") || string::startsWith(s, "B")) {
            setDisplayValue(30.87 * pow(2, std::stof(s.substr(1, 1))));
        } else if (string::startsWith(s, "c#") || string::startsWith(s, "C#")) {
            setDisplayValue(17.32 * pow(2, std::stof(s.substr(2, 1))));
        } else if (string::startsWith(s, "c") || string::startsWith(s, "C")) {
            setDisplayValue(16.35 * pow(2, std::stof(s.substr(1, 1))));
        } else if (string::startsWith(s, "d#") || string::startsWith(s, "D#")) {
            setDisplayValue(19.45 * pow(2, std::stof(s.substr(2, 1))));
        } else if (string::startsWith(s, "d") || string::startsWith(s, "D")) {
            setDisplayValue(18.35 * pow(2, std::stof(s.substr(1, 1))));
        } else if (string::startsWith(s, "e") || string::startsWith(s, "E")) {
           setDisplayValue(20.60 * pow(2, std::stof(s.substr(1, 1))));
        } else if (string::startsWith(s, "f#") || string::startsWith(s, "F#")) {
            setDisplayValue(23.12 * pow(2, std::stof(s.substr(2, 1))));
        } else if (string::startsWith(s, "f") || string::startsWith(s, "F")) {
            setDisplayValue(21.83 * pow(2, std::stof(s.substr(1, 1))));
        } else if (string::startsWith(s, "g#") || string::startsWith(s, "G#")) {
            setDisplayValue(25.96 * pow(2, std::stof(s.substr(2, 1))));
        } else if (string::startsWith(s, "g") || string::startsWith(s, "G")) {
            setDisplayValue(24.50 * pow(2, std::stof(s.substr(1, 1))));
        } else {
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
            if (n >= 1)
                setDisplayValue(v);
        }
    }

    float translateParameter(float value) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        int32_t rootIncrement = osc3Module->virtualModule.controls.knob1Value * 3;
        rootIncrement >>= 3;

        if (osc3Module->virtualModule.osc3UI.button5Mode) {
            rootIncrement &= 0xFE0;
        }

        rootIncrement = osc3Module->virtualModule.expo.convert(rootIncrement) >> 3;

        rootIncrement *= 8;
        rootIncrement = fix16_mul(rootIncrement, osc3Module->virtualModule.absoluteTune);
        rootIncrement = fix16_mul(rootIncrement, 65536 + (osc3Module->virtualModule.controls.knob2Value << 3));

        float frequency = osc3Module->effectiveSR * 32 * (rootIncrement/4294967296.0f);

        frequency *= pow(2, osc3Module->virtualModule.octaveRange);

        return frequency;

    }
    void setDisplayValue(float input) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        float target = input / 32.39;

        target = log2(target);

        float knob1Set = 0;
        float octaveSet = 0;

        printf("%9.6f Target \n", target);

        if (target <= 4.0f) {
            knob1Set = target/4.0 * 4095.0;
        } else if (target < 9.0f) {
            octaveSet = target - 4.0;
            octaveSet = (int32_t) octaveSet;
            octaveSet += 1;
            target -= octaveSet;
            knob1Set = target/4.0 * 4095.0;
            printf("%9.6f Target \n", target);
            printf("%9.6f Octave \n", octaveSet);

        } else {
            knob1Set = 4095;
            octaveSet = 5;
        }

        osc3Module->virtualModule.osc3UI.button1Mode = (int32_t) octaveSet;
        osc3Module->virtualModule.osc3UI.storeMode((int32_t) octaveSet, BUTTON1_MASK, BUTTON1_SHIFT);
        osc3Module->virtualModule.handleButton1ModeChange((int32_t) octaveSet);

        osc3Module->paramQuantities[Osc3::KNOB1_PARAM]->setValue(knob1Set);
        osc3Module->paramQuantities[Osc3::KNOB2_PARAM]->setValue(0);
    };

};

struct Osc3::DetuneKnobQuantity: ViaKnobQuantity {



    std::string getDisplayValueString(void) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        int32_t mode = osc3Module->virtualModule.osc3UI.button6Mode;

        if (mode == 0) {

            name = "Beat Frequency";

            unit = "Hz";

            float freqDiff = osc3Module->virtualModule.aFreq - osc3Module->virtualModule.bFreq;

            freqDiff = osc3Module->effectiveSR * 32 * (freqDiff/4294967296.0f);

            if (osc3Module->virtualModule.detuneBase == 0) {
                freqDiff = 0;
            }

            return string::f("%4.2f", freqDiff);

        } else if (mode == 1) {

            name = "Detune Amount";

            unit = "cents";

            float freqRatio = (osc3Module->virtualModule.bFreq - osc3Module->virtualModule.aFreq)/
                                (float) osc3Module->virtualModule.bFreq;
            freqRatio /= 0.06f;

            freqRatio *= 100.0;

            if (osc3Module->virtualModule.detuneBase == 0) {
                freqRatio = 0;
            }

            return string::f("%4.2f", freqRatio);

        } else if (mode == 2) {

            name = "Osc 2 and Osc 3 Chord Offsets";

            unit = "Notes";

            int32_t chord = __USAT(((osc3Module->virtualModule.controls.knob3Value << 4)) +
                (int32_t) -osc3Module->virtualModule.inputs.cv3Samples[0], 16);

            chord >>= 12;

            int32_t osc2Note = osc3Module->virtualModule.chords[chord*2 + 1];

            int32_t osc3Note = osc3Module->virtualModule.chords[chord*2];

            return "+" + string::f("%d", osc2Note) + ", " + string::f("%d", osc3Note) + " ";

        } else {

            name = "Unity Input Clock Multiplier";

            unit = "";

            return string::f("%i", osc3Module->virtualModule.beatMultiplier);

        }

    }

    // virtual float translateInput(float userInput) {

    //     Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

    //     return 11;

    // };

};

struct Osc3::OctaveButtonQuantity : ViaButtonQuantity<6> {

    std::string buttonModes[6] = {"+0 Octaves", "+1 Octaves", "+2 Octaves", "+3 Octaves", "+4 Octaves", "+5 Octaves"};

    OctaveButtonQuantity() {
        for (int i = 0; i < 6; i++) {
            modes[i] = buttonModes[i];
        }
    }

    int getModeEnumeration(void) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        return osc3Module->virtualModule.osc3UI.button1Mode;

    }

    void setMode(int mode) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        osc3Module->virtualModule.osc3UI.button1Mode = mode;
        osc3Module->virtualModule.osc3UI.storeMode(osc3Module->virtualModule.osc3UI.button1Mode, BUTTON1_MASK, BUTTON1_SHIFT);
        osc3Module->virtualModule.handleButton1ModeChange(mode);

    }

};

struct Osc3::WaveshapeButtonQuantity : ViaButtonQuantity<4> {

    std::string buttonModes[4] = {"Saw", "Square", "Trapezoid", "Triangle"};

    WaveshapeButtonQuantity() {
        for (int i = 0; i < 4; i++) {
            modes[i] = buttonModes[i];
        }
    }

    int getModeEnumeration(void) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        return osc3Module->virtualModule.osc3UI.button2Mode;

    }

    void setMode(int mode) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        osc3Module->virtualModule.osc3UI.button2Mode = mode;
        osc3Module->virtualModule.osc3UI.storeMode(osc3Module->virtualModule.osc3UI.button2Mode, BUTTON2_MASK, BUTTON2_SHIFT);
        osc3Module->virtualModule.handleButton2ModeChange(mode);

    }

};

struct Osc3::SHButtonQuantity : ViaButtonQuantity<2> {

    std::string buttonModes[2] = {"Off", "On"};

    SHButtonQuantity() {
        for (int i = 0; i < 2; i++) {
            modes[i] = buttonModes[i];
        }
    }

    int getModeEnumeration(void) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        return osc3Module->virtualModule.osc3UI.button3Mode;

    }

    void setMode(int mode) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        osc3Module->virtualModule.osc3UI.button3Mode = mode;
        osc3Module->virtualModule.osc3UI.storeMode(osc3Module->virtualModule.osc3UI.button3Mode, BUTTON3_MASK, BUTTON3_SHIFT);
        osc3Module->virtualModule.handleButton3ModeChange(mode);

    }

};

struct Osc3::QuantizationButtonQuantity : ViaButtonQuantity<4> {

    std::string buttonModes[4] = {"Off", "Semitone", "Major", "Minor"};

    QuantizationButtonQuantity() {
        for (int i = 0; i < 4; i++) {
            modes[i] = buttonModes[i];
        }
    }

    int getModeEnumeration(void) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        return osc3Module->virtualModule.osc3UI.button5Mode;

    }

    void setMode(int mode) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        osc3Module->virtualModule.osc3UI.button5Mode = mode;
        osc3Module->virtualModule.osc3UI.storeMode(osc3Module->virtualModule.osc3UI.button5Mode, BUTTON5_MASK, BUTTON5_SHIFT);
        osc3Module->virtualModule.handleButton5ModeChange(mode);

    }

};

struct Osc3::DetuneButtonQuantity : ViaButtonQuantity<4> {

    std::string buttonModes[4] = {"Even", "Scaled", "Chords", "Sync to Unity Input"};

    DetuneButtonQuantity() {
        for (int i = 0; i < 4; i++) {
            modes[i] = buttonModes[i];
        }
    }

    int getModeEnumeration(void) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        return osc3Module->virtualModule.osc3UI.button6Mode;

    }

    void setMode(int mode) override {

        Osc3 * osc3Module = dynamic_cast<Osc3 *>(this->module);

        osc3Module->virtualModule.osc3UI.button6Mode = mode;
        osc3Module->virtualModule.osc3UI.storeMode(osc3Module->virtualModule.osc3UI.button6Mode, BUTTON6_MASK, BUTTON6_SHIFT);
        osc3Module->virtualModule.handleButton6ModeChange(mode);

    }

};
