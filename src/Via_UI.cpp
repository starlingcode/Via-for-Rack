//
//  Via_UI.cpp
//  
//
//  Created by WIll Mitchell on 3/17/18.
//
//

#include "Via.hpp"

///////////// USER INTERFACE SECTION /////////////////

void Via::readDetect() {
    
    //check to see if any of our touch sensors have gone into detect state
    
    if (params[FREQ_PARAM].value == 1) {
        RESET_RGB_ON; //turn off the runtime display
        modeFlag = 1; //indicate to the other mode change functions that we have pressed the freqMode button
        detectOn = 1; //indicate that a touch sensor was in detect state during this aquisition cycle
        clearLEDs(); //wipe the vestiges of our runtimme display
        pressCounter = 0; //reset the timer that we use for mode change timeout
        showMode(freqMode); //show our currentm mode
    }
    if (params[TRIG_PARAM].value == 1) {
        RESET_RGB_ON;
        modeFlag = 2; //indicate to the other mode change functions that we have pressed the trigger mode button
        detectOn = 1;
        clearLEDs();
        pressCounter = 0;
        showMode(trigMode);
    }
    if (params[LOOP_PARAM].value == 1) {
        RESET_RGB_ON;
        modeFlag = 3; //indicate to the other mode change functions that we have pressed the loop button
        detectOn = 1;
        clearLEDs();
        pressCounter = 0;
        showMode(loopMode);
    }
    if (params[SH_PARAM].value == 1) {
        RESET_RGB_ON;
        modeFlag = 4; //indicate to the other mode change functions that we have pressed the sample and hold mode button
        detectOn = 1;
        clearLEDs();
        pressCounter = 0;
        showMode(sampleHoldMode);
    }
    if (params[UP_PARAM].value == 1) {
        RESET_RGB_ON;
        modeFlag = 5; //indicate to the other mode change functions that we have pressed the family up button
        detectOn = 1;
        clearLEDs();
        pressCounter = 0;
        showMode(familyIndicator);
    }
    if (params[DOWN_PARAM].value == 1) {
        RESET_RGB_ON;
        modeFlag = 6; //indicate to the other mode change functions that we have pressed the family down button
        detectOn = 1;
        clearLEDs();
        pressCounter = 0;
        showMode(familyIndicator);
    }
    
}

void Via::readRelease(uint32_t modeFlagHolder) {
    
    // look for a change to release state on the button that was pressed (passed in with the function argument)
    
    pressCounter++;
    
    switch (modeFlagHolder) {
            
        case 1:
            
            if (params[FREQ_PARAM].value == 0) {
                detectOn = 0; // indicate that we no longer have a touch sensor in detect state
                clearLEDs(); // clear the display that showed the current mode
                handleRelease(modeFlagHolder); //take the appropriate action per the button that had been pressed
            }
            break;
            
        case 2:
            
            if (params[TRIG_PARAM].value == 0) {
                detectOn = 0;
                clearLEDs();
                handleRelease(modeFlagHolder);
            }
            break;
            
        case 3:
            
            if (params[LOOP_PARAM].value == 0) {
                detectOn = 0;
                clearLEDs();
                handleRelease(modeFlagHolder);
            }
            break;
            
        case 4:
            
            if (params[SH_PARAM].value == 0) {
                detectOn = 0;
                clearLEDs();
                handleRelease(modeFlagHolder);
            }
            break;
            
        case 5:
            
            if (params[UP_PARAM].value == 0) {
                detectOn = 0;
                clearLEDs();
                handleRelease(modeFlagHolder);
            }
            break;
            
        case 6:
            
            if (params[DOWN_PARAM].value == 0) {
                detectOn = 0;
                clearLEDs();
                handleRelease(modeFlagHolder);
            }
            break;
            
            
    }
    
}

void Via::handleRelease(uint32_t pinMode) {
    if (pressCounter < engineGetSampleRate()) {
        // if we havent exceeded the mode change timeout, change the appropriate mode and then display the new mode
        // current value is probably too short
        changeMode(pinMode);
        switch (pinMode) {
            case 1:
                showMode(freqMode);
                break;
            case 2:
                showMode(trigMode);
                break;
            case 3:
                showMode(loopMode);
                break;
            case 4:
                showMode(sampleHoldMode);
                break;
            case 5:
                showMode(familyIndicator);
                break;
            case 6:
                showMode(familyIndicator);
                break;
            case 7:
                modeFlag = 2;
                showMode(logicOutA);
                break;
            case 8:
                modeFlag = 2;
                showMode(logicOutB);
                break;
        }
        displayNewMode = 1;
        
    } else {
        
        clearLEDs();
        SET_RGB_ON;
    }
    
    pressCounter = 0;
}

void Via::changeMode(uint32_t mode) {
    
    int modeHolder;
    
    if (mode == 1) {
        // toggle through our 3 freqMode modes
        modeHolder = freqMode;
        modeHolder = (modeHolder + 1) % 3;
        freqMode = modeHolder;
        
        holdState = (holdState & 0b1111111111111001) | (freqMode << 1);
        
        switchFamily();
        
        if (freqMode == 0 && loopMode == noloop) {
            //since this parameter can throw us into drum mode, initialize the proper modulation flags per trigger mode
            SET_DRUM_MODE_ON;
            
            switch (trigMode) {
                case 0:
                    SET_AMP_ON;
                    SET_PITCH_ON;
                    SET_MORPH_ON;
                    break;
                case 1:
                    SET_AMP_ON;
                    RESET_PITCH_ON;
                    SET_MORPH_ON;
                    break;
                case 2:
                    SET_AMP_ON;
                    RESET_PITCH_ON;
                    RESET_MORPH_ON;
                    break;
                case 3:
                    RESET_AMP_ON;
                    RESET_PITCH_ON;
                    SET_MORPH_ON;
                    break;
                case 4:
                    RESET_AMP_ON;
                    SET_PITCH_ON;
                    SET_MORPH_ON;
                    break;
            }
            
        } else {
            // if we didnt just go into drum mode, make sure drum mode is off
            RESET_DRUM_MODE_ON;
            RESET_AMP_ON;
            RESET_PITCH_ON;
            RESET_MORPH_ON;
            
            // set the appropriate time calculation functions
            
        }
    }
    else if (mode == 2) {
        modeHolder = trigMode;
        modeHolder = (modeHolder + 1) % 5;
        trigMode = modeHolder;
        //initialize some essential retrigger variables
        
        holdState = (holdState & 0b1111111111000111) | (trigMode << 3);
        
        //incSign = 1;
        RESET_GATE_ON;
        //if drum mode is on, toggle through sets of modulation destinations
        switch (trigMode) {
            case 0:
                SET_AMP_ON;
                SET_PITCH_ON;
                SET_MORPH_ON;
                break;
            case 1:
                SET_AMP_ON;
                RESET_PITCH_ON;
                SET_MORPH_ON;
                break;
            case 2:
                SET_AMP_ON;
                RESET_PITCH_ON;
                RESET_MORPH_ON;
                break;
            case 3:
                RESET_AMP_ON;
                RESET_PITCH_ON;
                SET_MORPH_ON;
                break;
            case 4:
                RESET_AMP_ON;
                SET_PITCH_ON;
                SET_MORPH_ON;
                break;
                
        }
    }
    else if (mode == 3) {
        loopMode = (loopMode + 1) % 2;
        
        holdState = (holdState & 0b1111111111111110) | loopMode;
        
        if (loopMode == 0) {
            // signal to our oscillator that it should put itself to sleep
            SET_LAST_CYCLE;
            // switching to no loop when freqMode is at audio activates drum mode
            // this is about the same as what we do in the freqMode mode case above
            if (freqMode == 0) {
                SET_DRUM_MODE_ON;
                switch (trigMode) {
                    case 0:
                        SET_AMP_ON;
                        SET_PITCH_ON;
                        SET_MORPH_ON;
                        break;
                    case 1:
                        SET_AMP_ON;
                        RESET_PITCH_ON;
                        SET_MORPH_ON;
                        break;
                    case 2:
                        SET_AMP_ON;
                        RESET_PITCH_ON;
                        RESET_MORPH_ON;
                        break;
                    case 3:
                        RESET_AMP_ON;
                        RESET_PITCH_ON;
                        SET_MORPH_ON;
                        break;
                    case 4:
                        RESET_AMP_ON;
                        SET_PITCH_ON;
                        SET_MORPH_ON;
                        break;
                        
                }
            } else {
                RESET_DRUM_MODE_ON;
                RESET_AMP_ON;
                RESET_PITCH_ON;
                RESET_MORPH_ON;
            }
        } else {
            RESET_LAST_CYCLE;
            RESET_DRUM_MODE_ON;
            RESET_AMP_ON;
            RESET_PITCH_ON;
            RESET_MORPH_ON;
            //set our oscillator active flag so enabling loop starts playback
            //SET_OSCILLATOR_ACTIVE;
        }
        
    }
    else if (mode == 4) {
        sampleHoldMode = (sampleHoldMode + 1) % 6;
        
        holdState = (holdState & 0b1111111000111111) | (sampleHoldMode << 6);
        
        
    }
    else if (mode == 5) {
        // increment our family pointer and swap in the correct family
        
        familyIndicator = (familyIndicator + 1) % 8;
        switchFamily();
        holdState = (holdState & 0b1111000111111111) | (familyIndicator << 9);
    }
    else if (mode == 6) {
        // wrap back to the end of the array of families if we go back from the first entry
        // otherwise same as above
        if (familyIndicator == 0) {
            familyIndicator = 7;
        } else {
            familyIndicator = (familyIndicator - 1);
        }
        switchFamily();
        holdState = (holdState & 0b1111000111111111) | (familyIndicator << 9);
    }
    else if (mode == 7) {
        logicOutA = (logicOutA + 1) % 3;
        holdState = (holdState & 0b1100111111111111) | (logicOutA << 13);;
        switch (logicOutA) {
            case 0:
                SET_GATEA;
                RESET_TRIGA;
                RESET_DELTAA;
                break;
            case 1:
                RESET_GATEA;
                SET_TRIGA;
                RESET_DELTAA;
                break;
            case 2:
                RESET_GATEA;
                RESET_TRIGA;
                SET_DELTAA;
                break;
        }
        
        
    }
    else if (mode == 8) {
        logicOutB = (logicOutB + 1) % 3;
        holdState = (holdState & 0b0011111111111111) | (logicOutB << 15);
        switch (logicOutB) {
            case 0:
                SET_GATEB;
                RESET_TRIGB;
                RESET_DELTAB;
                break;
            case 1:
                RESET_GATEB;
                SET_TRIGB;
                RESET_DELTAB;
                break;
            case 2:
                RESET_GATEB;
                RESET_TRIGB;
                SET_DELTAB;
                break;
        }
        
    }
    
    
    
}

void Via::showMode(uint32_t currentmode) {
    
    // if we are switching families, show a color corresponding to that family
    if (modeFlag == 5 || modeFlag == 6) {
        familyRGB();
    }
    
    else {
        switch (currentmode) {
                // represent a 4 bit number with our LEDs
                // NEEDS WORK
            case 0:
                LEDA_ON
                break;
            case 1:
                LEDC_ON
                break;
            case 2:
                LEDB_ON
                break;
            case 3:
                LEDD_ON
                break;
            case 4:
                LEDA_ON
                LEDC_ON
                break;
            case 5:
                LEDB_ON
                LEDD_ON
                break;
        }
    }
    
    
}

void Via::familyRGB(void) {
    
    switch (freqMode) {
        case 0:
            lights[RED_LIGHT].value = 1;
            lights[GREEN_LIGHT].value = 0;
            lights[BLUE_LIGHT].value = 0;
            break;
            
        case 1:
            lights[RED_LIGHT].value = 0;
            lights[GREEN_LIGHT].value = 1;
            lights[BLUE_LIGHT].value = 0;
            break;
            
        case 2:
            lights[RED_LIGHT].value = 0;
            lights[GREEN_LIGHT].value = 0;
            lights[BLUE_LIGHT].value = 1;
    }
    
    switch (familyIndicator) {
        case 0:
            LEDA_ON
            break;
        case 1:
            LEDC_ON
            break;
        case 2:
            LEDB_ON
            break;
            
        case 3:
            LEDD_ON
            break;
        case 4:
            LEDA_ON
            LEDC_ON
            break;
        case 5:
            LEDB_ON
            LEDD_ON
            break;
        case 6:
            LEDA_ON;
            LEDB_ON;
            break;
        case 7:
            LEDC_ON;
            LEDD_ON;
            break;
            
    }
    
    
    
}
void Via::clearLEDs(void) {
    //pretty self explanatory
    
    LEDA_OFF
    LEDB_OFF
    LEDC_OFF
    LEDD_OFF
    
    //blank the LEDs
    lights[RED_LIGHT].value = 0;
    lights[GREEN_LIGHT].value = 0;
    lights[BLUE_LIGHT].value = 0;
    
}

// this sets the flags to be used in the interrupt and also fills the holding array on the heap

void Via::switchFamily(void) {
    
    currentFamily = familyArray[freqMode][familyIndicator];

    span = (currentFamily.tableLength) << 16;
    spanx2 = (currentFamily.tableLength) << 17;
    switch (currentFamily.familySize) {
            // these are values that properly allow us to select a family and interpolation fraction for our morph
        case 3:
            morphBitShiftRight = 11;
            morphBitShiftLeft = 5;
            break;
            
        case 5:
            morphBitShiftRight = 10;
            morphBitShiftLeft = 6;
            break;
            
        case 9:
            morphBitShiftRight = 9;
            morphBitShiftLeft = 7;
            break;
            
        case 17:
            morphBitShiftRight = 8;
            morphBitShiftLeft = 8;
            break;
            
        case 33:
            morphBitShiftRight = 7;
            morphBitShiftLeft = 9;
            break;
            
    }
    switch (currentFamily.tableLength) {
            // these are values that properly allow us to select a family and interpolation fraction for our morph
        case 4:
            tableSizeCompensation = 6;
            position = position >> 6;
            break;
            
        case 8:
            tableSizeCompensation = 5;
            position = position >> 5;
            break;
            
        case 16:
            tableSizeCompensation = 4;
            position = position >> 4;
            break;
            
        case 32:
            tableSizeCompensation = 3;
             position = position >> 3;
            break;
            
        case 64:
            tableSizeCompensation = 2;
            position = position >> 2;
            break;
            
        case 128:
            tableSizeCompensation = 1;
            position = position >> 1;
            break;
            
        case 256:
            tableSizeCompensation = 0;
            break;
            
    }
    
    loadSampleArray(currentFamily);
}


