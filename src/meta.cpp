#include "meta.hpp"
#include "via_module.hpp"

#define META_OVERSAMPLE_AMOUNT 8
#define META_OVERSAMPLE_QUALITY 6


struct Meta : Via<META_OVERSAMPLE_AMOUNT, META_OVERSAMPLE_QUALITY> {

    #define META_WAVETABLE_LENGTH 33554432.0

    ExpoConverter expo;

    struct Time1Quantity : ViaKnobQuantity {

        std::string labels[3] = {"Coarse tune", "Attack time", "Cycle time"};
        std::string units[3] = {"Hz", "s", "s"};

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

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            int32_t drumMode = !metaModule->virtualModule.metaUI.button3Mode && !metaModule->virtualModule.metaUI.button6Mode;

            int32_t freqMode = metaModule->virtualModule.metaUI.button3Mode;

            if (drumMode) {

                int32_t timeBase1 = fix16_mul(fix16_mul(metaModule->expo.convert((metaModule->virtualModule.controls.knob1Value >> 3)*3 + 1024) >> 5, // 2 << 11
                    metaModule->expo.convert(2047)), // expoTable[2048]
                    metaModule->virtualModule.metaController.drumBaseIncrement);

                int32_t drumType = metaModule->virtualModule.metaUI.aux3Mode;

                if (drumType < 2) {
                    timeBase1 /= 32.0;
                } else {
                    timeBase1 /= 8.0 * 4.0;
                    timeBase1 *= 3.0;
                }

                return timeBase1/META_WAVETABLE_LENGTH * metaModule->sampleRateStore;  

            } else if (freqMode == 0) {

                int32_t timeBase1 = fix16_mul(fix16_mul(fix16_mul(metaModule->expo.convert((metaModule->virtualModule.controls.knob1Value >> 2)*3) >> 5, // 2 << 11
                    65535 + (metaModule->virtualModule.controls.knob2Value << 4)), // 2 << 16
                    metaModule->expo.convert(2047)), // expoTable[2048]
                metaModule->virtualModule.metaController.audioBaseIncrement);

                timeBase1 /= 4.0;

                return timeBase1/META_WAVETABLE_LENGTH * metaModule->sampleRateStore;       

            } else if (freqMode == 1) {

                int32_t timeBase1 = fix16_mul(metaModule->expo.convert(4095 - metaModule->virtualModule.controls.knob1Value) >> 7, // 2 << 11
                    metaModule->expo.convert(2048) >> 8);

                return 0.5/(timeBase1/META_WAVETABLE_LENGTH * metaModule->sampleRateStore);

            } else {

                int32_t timeBase1 = fix16_mul(metaModule->expo.convert(4095 - metaModule->virtualModule.controls.knob1Value) >> 9, // 2 << 11
                    metaModule->expo.convert(1024) >> 9);

                return 1.0/(timeBase1/META_WAVETABLE_LENGTH * metaModule->sampleRateStore);
            }        
        
        }

        float translateInput(float userInput) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            int32_t drumMode = !metaModule->virtualModule.metaUI.button3Mode && !metaModule->virtualModule.metaUI.button6Mode;

            int32_t freqMode = metaModule->virtualModule.metaUI.button3Mode;

            if (drumMode) {

                int32_t drumType = metaModule->virtualModule.metaUI.aux3Mode;

                float targetScale;

                if (drumType < 2) {

                    targetScale = userInput/21.797;                    

                } else {

                    targetScale = userInput/65.41;

                }

                targetScale = log2(targetScale * 2.0);                

                float time1 = roundf(targetScale * 384.0 * (8.0/3.0)) - 1024.0;
               
                return time1;  

            } else if (freqMode == 0) {

                float targetScale = userInput/16.34;

                targetScale = log2(targetScale);                

                float time1 = targetScale * 384.0 * (4.0/3.0);

                float timeBase1 = fix16_mul(fix16_mul(metaModule->expo.convert(((int)time1 >> 2)*3) >> 5, // 2 << 11
                    metaModule->expo.convert(2047)), // expoTable[2048]
                metaModule->virtualModule.metaController.audioBaseIncrement);

                timeBase1 /= 4.0;

                timeBase1 = timeBase1/META_WAVETABLE_LENGTH * metaModule->sampleRateStore;

                metaModule->paramQuantities[KNOB2_PARAM]->setValue((userInput/timeBase1 - 1.0) * 4095.0); 
               
                return time1;     

            } else if (freqMode == 1) {

                float lengthInSamples = userInput * metaModule->sampleRateStore;
                float desiredIncrement = 256.0/lengthInSamples;
                float normalizedCycleMod = ((float) (metaModule->expo.convert(2048)))/(65536.0 * 256.0);
                desiredIncrement = desiredIncrement/normalizedCycleMod;
                desiredIncrement *= 65536.0 * 128.0;

                return (4095.0 - (metaModule->reverseExpo(desiredIncrement)) * 384.0);

            } else {

                float lengthInSamples = userInput * metaModule->sampleRateStore;
                float desiredIncrement = 512.0/lengthInSamples;
                float normalizedCycleMod = ((float) (metaModule->expo.convert(1024)))/(65536.0 * 512.0);
                desiredIncrement = desiredIncrement/normalizedCycleMod;
                desiredIncrement *= 65536.0 * 512.0;

                return (4095.0 - (metaModule->reverseExpo(desiredIncrement)) * 384.0);

            }  

        };
        void setLabel(void) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            name = labels[metaModule->virtualModule.metaUI.button3Mode];
            unit = units[metaModule->virtualModule.metaUI.button3Mode];

        }
        int getDisplayPrecision(void) override {
            return 3;
        } 
 
    };
    
    struct Time2Quantity : ViaKnobQuantity {

        std::string labels[3] = {"Fine tune", "Release time", "Skew"};
        std::string units[3] = {"Hz", "s", "%"};

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

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            int32_t drumMode = !metaModule->virtualModule.metaUI.button3Mode && !metaModule->virtualModule.metaUI.button6Mode;

            int32_t freqMode = metaModule->virtualModule.metaUI.button3Mode;

            if (drumMode) {

                float release = (float) fix16_mul(metaModule->expo.convert(((4095 - metaModule->virtualModule.controls.knob2Value) >> 2) * 3) >> 11,
                    metaModule->expo.convert(1536));

                release /= 4.0;

                return 1.0/(release/META_WAVETABLE_LENGTH * metaModule->sampleRateStore);

            } else if (freqMode == 0) {

                float timeBase1 = fix16_mul(fix16_mul(fix16_mul(metaModule->expo.convert((metaModule->virtualModule.controls.knob1Value >> 2)*3) >> 5, // 2 << 11
                    65535 + (metaModule->virtualModule.controls.knob2Value << 4)), // 2 << 16
                    metaModule->expo.convert(2047)), // expoTable[2048]
                metaModule->virtualModule.metaController.audioBaseIncrement);

                timeBase1 /= 4.0;

                return timeBase1/META_WAVETABLE_LENGTH *  metaModule->sampleRateStore;       

            } else if (freqMode == 1) {

                int32_t timeBase1 = fix16_mul(metaModule->expo.convert(4095 - metaModule->virtualModule.controls.knob2Value) >> 9, // 2 << 11
                    metaModule->expo.convert(2048) >> 8);

                return 0.5/(timeBase1/META_WAVETABLE_LENGTH * metaModule->sampleRateStore);

            } else {

                return (((float) metaModule->virtualModule.controls.knob2Value) / 4095.0) * 60.0 + 20.0;

            }
                         
        
        }

        float translateInput(float userInput) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            int32_t drumMode = !metaModule->virtualModule.metaUI.button3Mode && !metaModule->virtualModule.metaUI.button6Mode;

            int32_t freqMode = metaModule->virtualModule.metaUI.button3Mode;

            if (drumMode) {

                float lengthInSamples = userInput * metaModule->sampleRateStore;
                float desiredIncrement = 256.0/lengthInSamples;
                float normalizedCycleMod = ((float) (metaModule->expo.convert(1536)))/65536.0;
                desiredIncrement = desiredIncrement/normalizedCycleMod;
                desiredIncrement *= 65536.0 * 2048 * 8.0;

                return (4095.0 - (metaModule->reverseExpo(desiredIncrement)/3.0) * 384.0 * 4.0);


            } else if (freqMode == 0) {

                float targetScale = userInput/16.34;

                targetScale = log2(targetScale);                

                float time1 = targetScale * 384.0 * (4.0/3.0);

                metaModule->paramQuantities[KNOB1_PARAM]->setValue(time1);

                float timeBase1 = fix16_mul(fix16_mul(metaModule->expo.convert(((int)time1 >> 2)*3) >> 5, // 2 << 11
                    metaModule->expo.convert(2047)), // expoTable[2048]
                metaModule->virtualModule.metaController.audioBaseIncrement);

                timeBase1 /= 4.0;

                timeBase1 = timeBase1/META_WAVETABLE_LENGTH * metaModule->sampleRateStore; 
               
                return (userInput/timeBase1 - 1.0) * 4095.0;     

            } else if (freqMode == 1) {

                float lengthInSamples = userInput * metaModule->sampleRateStore;
                float desiredIncrement = 256.0/lengthInSamples;
                float normalizedCycleMod = ((float) (metaModule->expo.convert(2048)))/(65536.0 * 256.0);
                desiredIncrement = desiredIncrement/normalizedCycleMod;
                desiredIncrement *= 65536.0 * 512.0;

                return (4095.0 - (metaModule->reverseExpo(desiredIncrement)) * 384.0);

            } else {

                return ((userInput - 20.0)/60.0) * 4095.0;

            }  

        };

        void setLabel(void) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            int32_t drumMode = !metaModule->virtualModule.metaUI.button3Mode && !metaModule->virtualModule.metaUI.button6Mode;

            if (drumMode) {
                name = "Drum decay";
                unit = "s";
            } else {
                name = labels[metaModule->virtualModule.metaUI.button3Mode];
                unit = units[metaModule->virtualModule.metaUI.button3Mode];
            }

        }

        int getDisplayPrecision(void) override {
            return 3;
        }
 
    };

    struct WaveshapeQuantity : ViaKnobQuantity {
        float translateParameter(float value) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            return metaModule->virtualModule.metaWavetable.tableSize * value/4095.0;            
        
        }
        float translateInput(float userInput) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            return (userInput * 4095.0)/((float) metaModule->virtualModule.metaWavetable.tableSize);

        };

        std::string getDisplayValueString(void) override {

            std::string displayValueRaw = string::f("%.*g", getDisplayPrecision(), math::normalizeZero(getDisplayValue()));

            return displayValueRaw;

        }

    };

    struct SHButtonQuantity : ViaButtonQuantity<6> {

        std::string buttonModes[6] = {"Off", "Track and hold A", "Resample B", "Track and hold A, resample B", "Track and hold A and B", "Resample A and B (Decimate)"};

        SHButtonQuantity() {
            for (int i = 0; i < 6; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            return metaModule->virtualModule.metaUI.button1Mode;

        }

        void setMode(int mode) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            metaModule->virtualModule.metaUI.button1Mode = mode;
            metaModule->virtualModule.metaUI.storeMode(metaModule->virtualModule.metaUI.button1Mode, BUTTON1_MASK, BUTTON1_SHIFT);
            metaModule->virtualModule.handleButton1ModeChange(mode);

        }

    };

    struct TableButtonQuantity : ViaComplexButtonQuantity {

        std::string buttonModes[3][8] = {{"Impulse", "Additive", "Linear Folds", "Skip Saw", "Perlin Noise", "Synthesized Vowels", "Sampled Vowels", "Trains"},
                                    {"Expo/Log Asymmetrical", "Expo/Log Symmetrical", "Circular Symmetrical", "Plateaus and Cliffs", "Moving Lump", "Fixed Lump", "Compressor", "Variable Sustain"},
                                    {"Ridges", "Euclidean Ridges", "Bounce", "Spring", "Ramps", "Sinusoids", "Sequences", "Steps"}};

        TableButtonQuantity() {
            modes = buttonModes[0];
            numModes = 8;
        }
        
        int getModeEnumeration(void) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            return metaModule->virtualModule.metaUI.button2Mode;

        }

        void getModeArray(void) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            modes = buttonModes[metaModule->virtualModule.metaUI.button3Mode];

            numModes = 8;

        }

        void setMode(int mode) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            metaModule->virtualModule.metaUI.button2Mode = mode;
            metaModule->virtualModule.metaUI.storeMode(metaModule->virtualModule.metaUI.button2Mode, BUTTON2_MASK, BUTTON2_SHIFT);
            metaModule->virtualModule.handleButton2ModeChange(mode);

        }

    };

    struct FreqButtonQuantity : ViaButtonQuantity<3> {

        std::string buttonModes[3] = {"Audio","Envelope","Sequence"};

        FreqButtonQuantity() {
            for (int i = 0; i < 3; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            return metaModule->virtualModule.metaUI.button3Mode;

        }

        void setMode(int mode) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            metaModule->virtualModule.metaUI.button3Mode = mode;
            metaModule->virtualModule.metaUI.storeMode(metaModule->virtualModule.metaUI.button3Mode, BUTTON3_MASK, BUTTON3_SHIFT);
            metaModule->virtualModule.handleButton3ModeChange(mode);

        }

    };

    struct TrigButtonQuantity : ViaComplexButtonQuantity {

        std::string trigModes[5] = {"No Retrigger","Hard Sync","A/R Model","Gated A/R Model","Pendulum"};
        std::string drumModes[4] = {"808 Kick","Tom","Pluck","Tone"};

        TrigButtonQuantity() {
            modes = trigModes;
            numModes = 0;
        }
        
        int getModeEnumeration(void) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            int32_t drumMode = !metaModule->virtualModule.metaUI.button3Mode && !metaModule->virtualModule.metaUI.button6Mode;

            if (drumMode) {
                return metaModule->virtualModule.metaUI.aux3Mode;
            } else {
                return metaModule->virtualModule.metaUI.button4Mode;
            }

        }

        void getModeArray(void) override {

            Meta * metaModule = dynamic_cast<Meta *>(module);

            int32_t drumMode = !metaModule->virtualModule.metaUI.button3Mode && !metaModule->virtualModule.metaUI.button6Mode;

            if (drumMode) {
                modes = drumModes;
                numModes = 4;
            } else {
                modes = trigModes;
                numModes = 5;
            }

        }

        void setMode(int mode) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            int32_t drumMode = !metaModule->virtualModule.metaUI.button3Mode && !metaModule->virtualModule.metaUI.button6Mode;

            if (drumMode) {
                metaModule->virtualModule.metaUI.aux3Mode = mode;
                metaModule->virtualModule.metaUI.storeMode(metaModule->virtualModule.metaUI.aux3Mode, AUX_MODE3_MASK, AUX_MODE3_SHIFT);
                metaModule->virtualModule.handleAux3ModeChange(mode);
            } else {
                metaModule->virtualModule.metaUI.button4Mode = mode;
                metaModule->virtualModule.metaUI.storeMode(metaModule->virtualModule.metaUI.button4Mode, BUTTON4_MASK, BUTTON4_SHIFT);
                metaModule->virtualModule.handleButton4ModeChange(mode);
            }

        }

    };

    struct LoopButtonQuantity : ViaButtonQuantity<2> {

        std::string buttonModes[2] = {"Off", "On"};

        LoopButtonQuantity() {
            for (int i = 0; i < 2; i++) {
                modes[i] = buttonModes[i];
            }
        }
        
        int getModeEnumeration(void) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            return metaModule->virtualModule.metaUI.button6Mode;

        }

        void setMode(int mode) override {

            Meta * metaModule = dynamic_cast<Meta *>(this->module);

            metaModule->virtualModule.metaUI.button6Mode = mode;
            metaModule->virtualModule.metaUI.storeMode(metaModule->virtualModule.metaUI.button6Mode, BUTTON6_MASK, BUTTON6_SHIFT);
            metaModule->virtualModule.handleButton6ModeChange(mode);

        }

    };

    Meta() : Via() {
        
        virtualIO = &virtualModule;

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam<Time1Quantity>(KNOB1_PARAM, 0, 4095.0, 2048.0);
        configParam<Time2Quantity>(KNOB2_PARAM, 0, 4095.0, 2048.0);
        configParam<WaveshapeQuantity>(KNOB3_PARAM, 0, 4095.0, 2048.0, "Wave shape");
        configParam<BScaleQuantity>(B_PARAM, -1.0, 1.0, 1.0, "B level");
        paramQuantities[B_PARAM]->description = "Main output is bounded between A and B levels";
        configParam<CV2ScaleQuantity>(CV2AMT_PARAM, 0, 1.0, 1.0, "Time 2 CV scale");
        configParam<ANormalQuantity>(A_PARAM, -5.0, 5.0, -5.0, "A level");
        paramQuantities[A_PARAM]->description = "Main output is bounded between A and B levels";
        configParam<CV3ScaleQuantity>(CV3AMT_PARAM, 0, 1.0, 1.0, "Wave shape CV scale");
        
        configParam<SHButtonQuantity>(BUTTON1_PARAM, 0.0, 1.0, 0.0, "S+H");
        configParam<TableButtonQuantity>(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Wavetable");
        configParam<FreqButtonQuantity>(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Frequency Range");
        configParam<TrigButtonQuantity>(BUTTON4_PARAM, 0.0, 1.0, 0.0, "TRIG response");
        configParam<TableButtonQuantity>(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Wavetable");
        configParam<LoopButtonQuantity>(BUTTON6_PARAM, 0.0, 1.0, 0.0, "Loop");
        
        configParam<ButtonQuantity>(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Manual Trigger");

        onSampleRateChange();
        presetData[0] = virtualModule.metaUI.stockPreset1;
        presetData[1] = virtualModule.metaUI.stockPreset2;
        presetData[2] = virtualModule.metaUI.stockPreset3;
        presetData[3] = virtualModule.metaUI.stockPreset4;
        presetData[4] = virtualModule.metaUI.stockPreset5;
        presetData[5] = virtualModule.metaUI.stockPreset6;
    
    }

    void process(const ProcessArgs &args) override;

    ViaMeta virtualModule;

    float sampleRateStore = 48000.0;

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();

        ledDecay = 16.0/sampleRate;

        if (sampleRate == 44100.0) {
            divideAmount = 1;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
            virtualModule.metaController.audioBaseIncrementStore = 39562;
            virtualModule.metaController.drumBaseIncrementStore = 66466;
        } else if (sampleRate == 48000.0) {
            divideAmount = 1;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
            virtualModule.metaController.audioBaseIncrementStore = 36347;
            virtualModule.metaController.drumBaseIncrementStore = 61065;
        } else if (sampleRate == 88200.0) {
            divideAmount = 2;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
            virtualModule.metaController.audioBaseIncrementStore = 39562;
            virtualModule.metaController.drumBaseIncrementStore = 66466;
        } else if (sampleRate == 96000.0) {
            divideAmount = 2;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
            virtualModule.metaController.audioBaseIncrementStore = 36347;
            virtualModule.metaController.drumBaseIncrementStore = 61065;
        } else if (sampleRate == 176400.0) {
            divideAmount = 4;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
            virtualModule.metaController.audioBaseIncrementStore = 39562;
            virtualModule.metaController.drumBaseIncrementStore = 66466;
        } else if (sampleRate == 192000.0) {
            divideAmount = 4;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
            virtualModule.metaController.audioBaseIncrementStore = 36347;
            virtualModule.metaController.drumBaseIncrementStore = 61065;
        } else if (sampleRate == 352800.0) {
            divideAmount = 8;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
            virtualModule.metaController.audioBaseIncrementStore = 39562;
            virtualModule.metaController.drumBaseIncrementStore = 66466;
        } else if (sampleRate == 384000.0) {
            divideAmount = 8;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
            virtualModule.metaController.audioBaseIncrementStore = 36347;
            virtualModule.metaController.drumBaseIncrementStore = 61065;
        } else if (sampleRate == 705600.0) {
            divideAmount = 16;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
            virtualModule.metaController.audioBaseIncrementStore = 39562;
            virtualModule.metaController.drumBaseIncrementStore = 66466;
        } else if (sampleRate == 768000.0) {
            divideAmount = 16;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
            virtualModule.metaController.audioBaseIncrementStore = 36347;
            virtualModule.metaController.drumBaseIncrementStore = 61065;
        }

        sampleRateStore = sampleRate/(float)divideAmount;
        
    }

    json_t *dataToJson() override {

        json_t *rootJ = json_object();
        
        // freq
        json_object_set_new(rootJ, "meta_modes", json_integer(virtualModule.metaUI.modeStateBuffer));

        return rootJ;
    }

    int32_t testMode;
    
    void dataFromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "meta_modes");
        virtualModule.metaUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.metaUI.loadFromEEPROM(0);
        virtualModule.metaUI.recallModuleState();

    }
    
};

void Meta::process(const ProcessArgs &args) {

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
            virtualModule.metaUI.incrementTimer();
            
            processTriggerButton();
            
            virtualModule.blinkTimerCount += virtualModule.blinkTimerEnable;
            virtualModule.blankTimerCount += virtualModule.blankTimerEnable;
            if (virtualModule.blinkTimerCount > virtualModule.blinkTimerOverflow) {
                virtualModule.blinkTimerCount = 0;
                virtualModule.blinkTimerEnable = 0;
                virtualModule.blankTimerEnable = 1;
                virtualModule.auxTimer1InterruptCallback();
            }
            if (virtualModule.blankTimerCount > virtualModule.blankTimerOverflow) {
                virtualModule.blankTimerCount = 0;
                virtualModule.blankTimerEnable = 0;
                virtualModule.auxTimer2InterruptCallback();
            }

            updateLEDs();

        }

        updateAudioRate();

    }
    
}

// Custom parameter widgets




struct MetaWidget : ModuleWidget  {

    MetaWidget(Meta *module) {

        setModule(module);

    	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/meta.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Meta::KNOB1_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Meta::KNOB2_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Meta::KNOB3_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Meta::B_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Meta::CV2AMT_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Meta::A_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Meta::CV3AMT_PARAM));
        
        addParam(createParam<TransparentButton>(Vec(10.5 + .753, 80), module, Meta::BUTTON1_PARAM));
        addParam(createParam<TransparentButton>(Vec(47 + .753, 77.5), module, Meta::BUTTON2_PARAM));
        addParam(createParam<TransparentButton>(Vec(85 + .753, 80), module, Meta::BUTTON3_PARAM));
        addParam(createParam<TransparentButton>(Vec(10.5 + .753, 129), module, Meta::BUTTON4_PARAM));
        addParam(createParam<TransparentButton>(Vec(46 + .753, 131.5), module, Meta::BUTTON5_PARAM));
        addParam(createParam<TransparentButton>(Vec(85 + .753, 129), module, Meta::BUTTON6_PARAM));
        
        addParam(createParam<ViaPushButton>(Vec(132.7 + .753, 320), module, Meta::TRIGBUTTON_PARAM));

        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 241.12), module, Meta::A_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 282.62), module, Meta::B_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 324.02), module, Meta::MAIN_LOGIC_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 241.12), module, Meta::CV1_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 282.62), module, Meta::CV2_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 324.02), module, Meta::CV3_INPUT));
        addInput(createInput<ViaJack>(Vec(135 + 1.053, 282.62), module, Meta::AUX_LOGIC_INPUT));

        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 241.12), module, Meta::LOGICA_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 282.62), module, Meta::AUX_DAC_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 324.02), module, Meta::MAIN_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(135 + 1.053, 241.12), module, Meta::AUX_LOGIC_OUTPUT));

        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Meta::LED1_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.1 + .753, 268.5), module, Meta::LED2_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Meta::LED3_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.1 + .753, 309.9), module, Meta::LED4_LIGHT));
        addChild(createLight<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Meta::OUTPUT_GREEN_LIGHT));
        addChild(createLight<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Meta::RED_LIGHT));

        }

    void appendContextMenu(Menu *menu) override {
        Meta *module = dynamic_cast<Meta*>(this->module);

        struct MetaAux2ModeHandler : MenuItem {
            Meta *module;
            int32_t logicMode;
            void onAction(const event::Action &e) override {
                module->virtualModule.metaUI.aux2Mode = logicMode;
                module->virtualModule.handleAux2ModeChange(logicMode);
                module->virtualModule.metaUI.storeMode(module->virtualModule.metaUI.aux2Mode, AUX_MODE2_MASK, AUX_MODE2_SHIFT);

            }
        };

        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("Logic out"));
        const std::string logicLabels[] = {
            "High during release",
            "High during attack",
        };
        for (int i = 0; i < (int) LENGTHOF(logicLabels); i++) {
            MetaAux2ModeHandler *aux2Item = createMenuItem<MetaAux2ModeHandler>(logicLabels[i], CHECKMARK(module->virtualModule.metaUI.aux2Mode == i));
            aux2Item->module = module;
            aux2Item->logicMode = i;
            menu->addChild(aux2Item);
        }

        struct MetaAux4ModeHandler : MenuItem {
            Meta *module;
            int32_t signalMode;
            void onAction(const event::Action &e) override {
                module->virtualModule.metaUI.aux4Mode = signalMode;
                module->virtualModule.handleAux4ModeChange(signalMode);
                module->virtualModule.metaUI.storeMode(module->virtualModule.metaUI.aux4Mode, AUX_MODE4_MASK, AUX_MODE4_SHIFT);
            }
        };

        struct MetaAux1ModeHandler : MenuItem {
            Meta *module;
            int32_t drumMode;
            void onAction(const event::Action &e) override {
                module->virtualModule.metaUI.aux1Mode = drumMode;
                module->virtualModule.metaUI.storeMode(module->virtualModule.metaUI.aux1Mode, AUX_MODE1_MASK, AUX_MODE1_SHIFT);
                if ((module->virtualModule.metaUI.button3Mode | module->virtualModule.metaUI.button6Mode) == 0) {
                    module->virtualModule.handleAux1ModeChange(drumMode);
                }
            }
        };

        if ((module->virtualModule.metaUI.button3Mode) || (module->virtualModule.metaUI.button6Mode)) {

            menu->addChild(createMenuLabel("Signal out"));
            const std::string signalLabels[] = {
                "Triangle",
                "Contour",
            };
            for (int i = 0; i < (int) LENGTHOF(signalLabels); i++) {
                MetaAux4ModeHandler *aux4Item = createMenuItem<MetaAux4ModeHandler>(signalLabels[i], CHECKMARK(module->virtualModule.metaUI.aux4Mode == i));
                aux4Item->module = module;
                aux4Item->signalMode = i;
                menu->addChild(aux4Item);
            }

        } else {

            menu->addChild(createMenuLabel("Drum signal out"));
            const std::string drumOutLabels[] = {
                "Triangle",
                "Contour",
                "Envelope",
                "Noise"
            };
            for (int i = 0; i < (int) LENGTHOF(drumOutLabels); i++) {
                MetaAux1ModeHandler *aux1Item = createMenuItem<MetaAux1ModeHandler>(drumOutLabels[i], CHECKMARK(module->virtualModule.metaUI.aux1Mode == i));
                aux1Item->module = module;
                aux1Item->drumMode = i;
                menu->addChild(aux1Item);
            }

        }

        struct PresetRecallItem : MenuItem {
            Meta *module;
            int preset;
            void onAction(const event::Action &e) override {
                module->virtualModule.metaUI.modeStateBuffer = preset;
                module->virtualModule.metaUI.loadFromEEPROM(0);
                module->virtualModule.metaUI.recallModuleState();
            }
        };

        struct StockPresetItem : MenuItem {
            Meta *module;
            Menu *createChildMenu() override {
                Menu *menu = new Menu();
                const std::string presetLabels[] = {
                    "Drum",
                    "Oscillator",
                    "AR Envelope",
                    "Looping AR",
                    "Modulation Sequence",
                    "Complex LFO",
                };
                for (int i = 0; i < (int) LENGTHOF(presetLabels); i++) {
                    PresetRecallItem *item = createMenuItem<PresetRecallItem>(presetLabels[i], CHECKMARK(module->virtualModule.metaUI.modeStateBuffer == (int32_t) module->presetData[i]));
                    item->module = module;
                    item->preset = module->presetData[i];
                    menu->addChild(item);
                }
                return menu;
            }
        };

        menu->addChild(new MenuEntry);
        StockPresetItem *stockPresets = createMenuItem<StockPresetItem>("Stock presets");
        stockPresets->module = module;
        menu->addChild(stockPresets);

    }

};


Model *modelMeta = createModel<Meta, MetaWidget>("META");

