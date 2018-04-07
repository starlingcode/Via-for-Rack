//
//  Via_Phase_Calculation.cpp
//  
//
//  Created by WIll Mitchell on 4/6/18.
//
//

#include "Via.hpp"


void Via::getPhase(void) {
    
    static int incFromADCs;
    
    //calculate our increment value in high speed mode
    
    
    
    //define increment for env and seq modes using function pointers to the appropriate knob/cv combo
    //these can be swapped around by the retrigger interrupt
    
    switch (freqMode) {
            
            
        case 0:
            /*
             contour generator frequency is a function of phase increment, wavetable size, and sample rate
             we multiply our ADC readings (knobs and CVs) to get a phase increment for our wavetable playback
             t1 knob and CV are mapped to an exponential curve with a lookup table
             this is scaled to 1v/oct for the t1CV
             t2 is linear FM
             we fiddled with scale using constant values and bitshifts to make the controls span the audio range
             if we are in drum mode, replace linear fm with the scaling factor from the drum envelope*/
            if (loopMode == 0) {
                
                incFromADCs = fix16_mul(
                                          fix16_mul(150000, lookupTable[time1CV] >> 5), lookupTable[(time1Knob >> 1) + 2047] >> 10) >> tableSizeCompensation;
                
                if (PITCH_ON) {incFromADCs = fix16_mul(expoScale + 30000, incFromADCs);}
                
            }
            
            else {
                
                incFromADCs = fix16_mul(fix16_mul(fix16_mul((3000 - (4095 -time2CV)) << 7, lookupTable[time1CV] >> 7), lookupTable[time1Knob] >> 4), lookupTable[time2Knob >> 4]) >> tableSizeCompensation;
                
            }
            break;
            
        case 1:
            if (!switchARTimes) {
                
                if ((position < span)) {
                    incFromADCs = calcTime1Env();
                } else {
                    incFromADCs = calcTime2Env();
                }
                
            } else {
                
                if ((position < span)) {
                    incFromADCs = calcTime2Env();
                } else {
                    incFromADCs = calcTime1Env();
                }
                
            }
            break;
            
        case 2:
            if (!switchARTimes) {
                
                if ((position < span)) {
                    incFromADCs = calcTime1Seq();
                } else {
                    incFromADCs = calcTime2Seq();
                }
                
            } else {
                
                if ((position < span)) {
                    incFromADCs = calcTime2Seq();
                } else {
                    incFromADCs = calcTime1Seq();
                }
            }
            break;
            
        default:
            break;
            
    }
    
    
    
    // apply the approrpiate signage to our inc per the retrigger behavior
    // this is how we get the contour generator to run backwards
    inc = incFromADCs * incSign;
    
    // if trigmode is gated and we arent in Drum Mode
    if (trigMode > 2 && !(DRUM_MODE_ON)) {
        
        // we look to see if we are about to increment past the attack->release transition
        
        if ((GATE_ON) && (std::abs(inc) > std::abs(span - position))) {
            
            // if so, we set a logic flag that we have frozen the contour generator in this transition
            SET_HOLD_AT_B;
            
            //and we hold it in place
            inc = span - position;
            
        }
        
        //if any of the above changes, we indicate that we are no longer frozen
        else {
            RESET_HOLD_AT_B;
        }
        
    }
    
    //this keeps us from asking the contour generator to jump all the way through the wavetable
    
    if (inc >= spanx2) {
        inc = spanx2 - 1;
    } else if (inc <= -spanx2) {
        inc = -spanx2 + 1;
    }
    
    //increment our phase pointer by the newly calculated increment value
    
    position = position + inc;
    
    // if we have incremented outside of our table, wrap back around to the other side and stop/reset if we are in LF 1 shot mode
    
    if (position >= spanx2) {
        
        position = position - spanx2;
        
        if ((loopMode == 0 && freqMode != 0) || (LAST_CYCLE)) {
            
            //this is the logic maintenance needed to properly put the contour generator to rest
            //this keeps behavior on the next trigger predictable
            
            RESET_LAST_CYCLE;
            RESET_OSCILLATOR_ACTIVE;
            RESET_DRUM_RELEASE_ON;
            incSign = 1;
            position = 0;
            SET_PHASE_STATE;
            if (RGB_ON) {
                LEDA_OFF
                LEDB_OFF
                LEDC_OFF
                LEDD_OFF
            }
            EXTI15_10_IRQHandler();
            
        }
        
    }
    // same as above but for when we are backtracking through the attack phase aka negative increment
    else if (position < 0) {
        
        position = position + spanx2;
        
        if ((loopMode == 0 && freqMode != 0) || (LAST_CYCLE)) {
            
            //same as above, we are putting our contour generator to rest
            
            RESET_LAST_CYCLE;
            RESET_OSCILLATOR_ACTIVE;
            RESET_DRUM_RELEASE_ON;
            incSign = 1;
            position = 0;
            RESET_PHASE_STATE;
            if (RGB_ON) {
                LEDA_OFF
                LEDB_OFF
                LEDC_OFF
                LEDD_OFF
            }
            EXTI15_10_IRQHandler();
            
        }
        
    }
    
}

//multiply our knobs with our CVs with the appropriate scaling per the current frequency mode

int Via::calcTime1Env(void) {
    
    time1 = ((lookupTable[4095 - time1CV] >> 13) * (lookupTable[(4095 - time1Knob)] >> 13)) >> (5 + tableSizeCompensation);
    return time1;
    
}

int Via::calcTime2Env(void) {
    
    time2 = ((lookupTable[4095 - time2CV] >> 13) * (lookupTable[(4095 - time2Knob)] >> 13)) >> (7 + tableSizeCompensation);
    return time2;
    
}

int Via::calcTime1Seq(void) {
    
    time1 = ((lookupTable[4095 - time1CV] >> 13) * (lookupTable[(4095 - time1Knob)] >> 13)) >> (9 + tableSizeCompensation);
    return time1;
    
}

int Via::calcTime2Seq(void) {
    
    time2 = ((lookupTable[4095 - time2CV] >> 13) * (lookupTable[(4095 - time2Knob)] >> 13)) >> (9 + tableSizeCompensation);
    return time2;
    
}
