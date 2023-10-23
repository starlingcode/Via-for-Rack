#pragma once

#include "starling-rack-ui.hpp"

// Custom Quantities

template<int NUM_MODES> 
struct ViaButtonQuantity : ParamQuantity {

    std::string modes[NUM_MODES];

    virtual int getModeEnumeration(void) {return 0;}
    virtual void setMode(int mode) {}

    float getDisplayValue() override {
        if (!module)
            return Quantity::getDisplayValue();

        return getModeEnumeration();
    }

    std::string getDisplayValueString() override {
        if (!module)
            return Quantity::getDisplayValueString();

        int mode = getDisplayValue();

        return modes[mode] + " (" + std::to_string(mode + 1) + ")";
    }

    void setDisplayValueString(std::string s) override {
        if (!module)
            return;

        for (int i = 0; i < NUM_MODES; i++) {
            if (s == modes[i] || s == std::to_string(i + 1)) {
                setMode(i);
            } 
        }

    }

};

struct ViaComplexButtonQuantity : ParamQuantity {

    std::string * modes;
    int numModes = 0;

    virtual int getModeEnumeration(void) {return 0;}
    virtual void getModeArray(void) {}
    virtual void setMode(int mode) {}

    float getDisplayValue() override {
        if (!module)
            return Quantity::getDisplayValue();

        return getModeEnumeration();
    }

    std::string getDisplayValueString() override {
        if (!module)
            return Quantity::getDisplayValueString();

        int mode = getDisplayValue();

        getModeArray();

        return modes[mode] + " (" + std::to_string(mode + 1) + ")";
    }

    void setDisplayValueString(std::string s) override {
        if (!module)
            return;

        for (int i = 0; i < numModes; i++) {
            if (s == modes[i] || s == std::to_string(i + 1)) {
                setMode(i);
            } 
        }

    }

};

struct ViaKnobQuantity : ParamQuantity {

    std::string stripScientificNotation(std::string s) {
        return s.substr(0, s.size() - 4);
    }

    virtual float translateParameter(float value) {return 0;}
    virtual float translateInput(float userInput) {return 0;}
    virtual void setLabel(void) {};

    float getDisplayValue() override {
        if (!module)
            return Quantity::getDisplayValue();

        setLabel();

        return translateParameter(getSmoothValue());
    }

    int getDisplayPrecision() override {
        return 3;
    }

    void setDisplayValue(float v) override {
        if (!module)
            return;

        setValue(translateInput(v));

    }

    std::string getDisplayValueString(void) override {

        std::string displayValueRaw = string::f("%.*g", getDisplayPrecision(), math::normalizeZero(getDisplayValue()));

        if (displayValueRaw.size() > 4) {

            if (string::endsWith(displayValueRaw, "e+03")) {
                return stripScientificNotation(displayValueRaw) + "k";
            } else if (string::startsWith(displayValueRaw.c_str(), "0.")) {
                return string::f("%.*g", getDisplayPrecision(), std::stof(displayValueRaw) * 1000.0) + "m";
            } else {
                return displayValueRaw;
            }

        } else if (displayValueRaw.size() > 2) {
            if (string::startsWith(displayValueRaw, "0.")) {
                return string::f("%.*g", getDisplayPrecision(), std::stof(displayValueRaw) * 1000.0) + "m";
            } else {
              return displayValueRaw;
            }
        } else {
            return displayValueRaw;
        }

    }

};
