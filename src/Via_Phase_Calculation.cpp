//
//  Via_Phase_Calculation.cpp
//  
//
//  Created by WIll Mitchell on 4/6/18.
//
//

#include "Via.hpp"


void Via::getPhaseOsc(void) {
    
    // calculate the product of the linar FM CV with a product of exponential lookup functions
    // the lookup functions are scaled with the CV input circuit to yield 1vOct at the T1 CV
    inc = fix16_mul(fix16_mul(fix16_mul((2100 - time2CV) << 9, lookupTable[4095 - time1CV] >> 5), lookupTable[time1Knob] >> 4), lookupTable[time2Knob >> 4]) >> tableSizeCompensation;
    
    //this keeps us from asking the contour generator to jump all the way through the wavetable
    if (inc >= span) {
        inc = span;
    } else if (inc <= -span) {
        inc = -span;
    }
    
    // apply the approrpiate signage to our inc per the retrigger behavior
    // this is how we get the contour generator to run backwards
    inc = inc * incSign;
    // if trigmode is gated and we arent in Drum Mode
    if (trigMode > 2) {
        // we look to see if we are about to increment past the attack->release transition
        if ((GATE_ON) && (abs(inc) > abs(span - position))) {
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
    
    // increment our phase pointer by the newly calculated increment value
    position = position + inc;
    
    // if we have incremented outside of our table, wrap back around to the other side and stop/reset if we are in LF 1 shot mode
    if (position >= spanx2) {
        position = position - spanx2;
    }
    // same as above but for when we are backtracking through the attack phase aka negative increment
    else if (position < 0) {
        position = position + spanx2;
    }
    
}

void Via::getPhaseDrum(void) {
    
    // calculate the product of exponential lookup functions
    // the lookup functions are scaled with the CV input circuit to yield 1vOct at the T1 CV
    // T2 is omitted from this calculation and determines drum decay time
    inc = fix16_mul(
                      fix16_mul(300000, lookupTable[4095 - time1CV] >> 6), lookupTable[(time1Knob >> 1) + 2047] >> 10) >> tableSizeCompensation;
    
    // scale with the drum envelope when specified by the trig control
    if (PITCH_ON) {inc = fix16_mul(expoScale + 30000, inc);}
    
    inc = inc * incSign;
    
    //this keeps us from asking the contour generator to jump all the way through the wavetable
    if (inc >= span) {
        inc = span;
    } else if (inc <= -span) {
        inc = -span;
    }
    
    // increment our phase pointer by the newly calculated increment value
    position = position + inc;
    
    // if we have incremented outside of our table, wrap back around to the other side and stop/reset if we are in LF 1 shot mode
    if (position >= spanx2) {
        position = position - spanx2;
        if (LAST_CYCLE) {
            
            // this is the logic maintenance needed to properly put the contour generator to rest
            // this keeps behavior on the next trigger predictable
            RESET_LAST_CYCLE;
            RESET_OSCILLATOR_ACTIVE;
            incSign = 1;
            position = 0;
            SET_PHASE_STATE;
            holdA = 0;
            holdB = 0;
            if (RGB_ON) {
                LEDA_OFF;
                LEDB_OFF;
                LEDC_OFF;
                LEDD_OFF;
            }
        }
    }
    // same as above but for when we are backtracking through the attack phase aka negative increment
    else if (position < 0) {
        
        position = position + spanx2;
        
        if (LAST_CYCLE) {
            
            // same as above, we are putting our contour generator to rest
            
            RESET_LAST_CYCLE;
            RESET_OSCILLATOR_ACTIVE;
            incSign = 1;
            position = 0;
            RESET_PHASE_STATE;
            holdA = 0;
            holdB = 0;
            if (RGB_ON) {
                LEDA_OFF;
                LEDB_OFF;
                LEDC_OFF;
                LEDD_OFF;
            }
            //HAL_NVIC_SetPendingIRQ(EXTI15_10_IRQn);
        }
    }
    
}

void Via::getPhaseSimpleEnv(void) {
    
    //call the appropriate time calculation for the phase state and retriggering behavior
    if (position < span) {
        inc = (this->*attackTime)();
    }
    else {
        inc = (this->*releaseTime)();
    }
    
    // apply the approrpiate signage to our inc per the retrigger behavior
    // this is how we get the contour generator to run backwards
    inc = inc * incSign;
    // if trigmode is gated
    if (trigMode > 2) {
        // we look to see if we are about to increment past the attack->release transition
        if ((GATE_ON) && (abs(inc) > abs(span - position))) {
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
    if (inc >= span) {
        inc = span;
    } else if (inc <= -span) {
        inc = -span;
    }
    
    // increment our phase pointer by the newly calculated increment value
    position = position + inc;
    
    // if we have incremented outside of our table, wrap back around to the other side and stop/reset if we are in LF 1 shot mode
    if (position >= spanx2) {
        
        position = position - spanx2;
        RESET_LAST_CYCLE;
        RESET_OSCILLATOR_ACTIVE;
        if (trigMode == pendulum)  {
            incSign = -1;
            position = spanx2;
            holdPosition = position;
        }
        else {
            incSign = 1;
            position = 0;
            holdPosition = position;
        }
        SET_PHASE_STATE;
        holdA = 0;
        holdB = 0;
        if (RGB_ON) {
            LEDA_OFF;
            LEDB_OFF;
            LEDC_OFF;
            LEDD_OFF;
        }
    }
    // same as above but for when we are backtracking through the attack phase aka negative increment
    else if (position < 0) {
        
        position = position + spanx2;
        
        RESET_LAST_CYCLE;
        RESET_OSCILLATOR_ACTIVE;
        incSign = 1;
        position = 0;
        holdPosition = 0;
        RESET_PHASE_STATE;
        holdA = 0;
        holdB = 0;
        if (RGB_ON) {
            LEDA_OFF;
            LEDB_OFF;
            LEDC_OFF;
            LEDD_OFF;
        }
    }
    
}

void Via::getPhaseSimpleLFO(void) {
    
    if (position < span) {
        inc = (this->*attackTime)();
    }
    else {
        inc = (this->*releaseTime)();
    }
    
    // apply the approrpiate signage to our inc per the retrigger behavior
    // this is how we get the contour generator to run backwards
    inc = inc * incSign;
    // if trigmode is gated and we arent in Drum Mode
    if (trigMode > 2) {
        // we look to see if we are about to increment past the attack->release transition
        if ((GATE_ON) && (abs(inc) > abs(span - position))) {
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
    if (inc >= span) {
        inc = span;
    } else if (inc <= -span) {
        inc = -span;
    }
    
    // increment our phase pointer by the newly calculated increment value
    position = position + inc;
    
    // if we have incremented outside of our table, wrap back around to the other side and stop/reset if we are in LF 1 shot mode
    if (position >= spanx2) {
        
        position = position - spanx2;
        
    }
    // same as above but for when we are backtracking through the attack phase aka negative increment
    else if (position < 0) {
        position = position + spanx2;
    }
    
}

void Via::getPhaseComplexEnv(void) {
    int attackTransferHolder;
    int releaseTransferHolder;
    int skewMod;
    
    inc = incSign*calcTime1Seq();
    
    if (!(HOLD_AT_B)) {
        holdPosition = inc + holdPosition;
    }
    if (holdPosition >= spanx2) {
        
        holdPosition = holdPosition - spanx2;
        
        RESET_OSCILLATOR_ACTIVE;
        if (trigMode == pendulum && !(DRUM_MODE_ON))  {
            incSign = -1;
            position = spanx2;
            holdPosition = spanx2;
        }
        else {
            incSign = 1;
            position = 0;
            holdPosition = 0;
        }
        out = 0;
        SET_PHASE_STATE;
        holdA = 0;
        holdB = 0;
        if (RGB_ON) {
            LEDA_OFF;
            LEDB_OFF;
            LEDC_OFF;
            LEDD_OFF;
        }
    }
    
    if (holdPosition < 0) {
        holdPosition = holdPosition + spanx2;
        
        RESET_OSCILLATOR_ACTIVE;
        incSign = 1;
        position = 0;
        holdPosition = 0;
        out = 0;
        RESET_PHASE_STATE;
        holdA = 0;
        holdB = 0;
        if (RGB_ON) {
            LEDA_OFF;
            LEDB_OFF;
            LEDC_OFF;
            LEDD_OFF;
        }
    }
    
    //combine the T2 CV and knob analogous to the morph knob
    
    if ((4095 - time2CV) >= 2047) {
        // this first does the aforementioned interpolation between the knob value and full scale then scales back the value according to frequency
        skewMod = fix16_lerp(time2Knob, 4095, ((4095 - time2CV) - 2048) << 4);
    }
    else {
        // analogous to above except in this case, morphCV is less than halfway
        skewMod = fix16_lerp(0, time2Knob, (4095 - time2CV) << 4);
    }
    
    if (holdPosition < (fix16_mul(spanx2, (4095 - skewMod) << 4))) {
        attackTransferHolder = (65535 << 11)/(4095 - skewMod); // 1/(T2*2)
        position = fix16_mul(holdPosition, attackTransferHolder);
        
    }
    else if (!(HOLD_AT_B)) {
        releaseTransferHolder = (65535 << 11)/(skewMod); // 1/((1-T2)*2)
        position = fix16_mul(holdPosition, releaseTransferHolder) + spanx2 - fix16_mul(spanx2, releaseTransferHolder);
        
    }
    
    if ((GATE_ON) && ((abs(inc) > abs(span - position)) || (HOLD_AT_B))) {
        // if so, we set a logic flag that we have frozen the contour generator in this transition
        SET_HOLD_AT_B;
    }
    //if any of the above changes, we indicate that we are no longer frozen
    else {
        RESET_HOLD_AT_B;
    }
    
    
    // if we have incremented outside of our table, wrap back around to the other side and stop/reset if we are in LF 1 shot mode
    if (position >= spanx2) {
        position = position - spanx2;
        
        RESET_OSCILLATOR_ACTIVE;
        if (trigMode == pendulum && !(DRUM_MODE_ON))  {
            incSign = -1;
            position = spanx2;
            holdPosition = position;
        }
        else {
            incSign = 1;
            position = 0;
            holdPosition = position;
        }
        SET_PHASE_STATE;
        holdA = 0;
        holdB = 0;
        if (RGB_ON) {
            LEDA_OFF;
            LEDB_OFF;
            LEDC_OFF;
            LEDD_OFF;
        }
    }
    // same as above but for when we are backtracking through the attack phase aka negative increment
    else if (position < 0) {
        
        position = position + spanx2;
        
        RESET_OSCILLATOR_ACTIVE;
        incSign = 1;
        position = 0;
        holdPosition = 0;
        RESET_PHASE_STATE;
        holdA = 0;
        holdB = 0;
        if (RGB_ON) {
            LEDA_OFF;
            LEDB_OFF;
            LEDC_OFF;
            LEDD_OFF;
        }
        
    }
    
}

void Via::getPhaseComplexLFO(void) {
    int attackTransferHolder;
    int releaseTransferHolder;
    int skewMod;

    
    inc = incSign*calcTime1Seq();
    
    if (!(HOLD_AT_B)) {
        holdPosition = inc + holdPosition;
    }
    if (holdPosition >= spanx2) {
        holdPosition = holdPosition - spanx2;
    }
    
    if (holdPosition < 0) {
        holdPosition = holdPosition + spanx2;
    }
    
    if ((4095 - time2CV) >= 2047) {
        // this first does the aforementioned interpolation between the knob value and full scale then scales back the value according to frequency
        skewMod = fix16_lerp(time2Knob, 4095, ((4095 - time2CV) - 2048) << 4);
    }
    else {
        // analogous to above except in this case, morphCV is less than halfway
        skewMod = fix16_lerp(0, time2Knob, (4095 - time2CV) << 4);
    }
    
    if (holdPosition < (fix16_mul(spanx2, (4095 - skewMod) << 4))) {
        attackTransferHolder = (65535 << 11)/(4095 - skewMod); // 1/(T2*2)
        position = fix16_mul(holdPosition, attackTransferHolder);
        
    }
    else if (!(HOLD_AT_B)) {
        releaseTransferHolder = (65535 << 11)/(skewMod); // 1/((1-T2)*2)
        position = fix16_mul(holdPosition, releaseTransferHolder) + spanx2 - fix16_mul(spanx2, releaseTransferHolder);
        
    }
    
    if ((GATE_ON) && ((abs(inc) > abs(span - position)) || (HOLD_AT_B))) {
        // if so, we set a logic flag that we have frozen the contour generator in this transition
        SET_HOLD_AT_B;
    }
    //if any of the above changes, we indicate that we are no longer frozen
    else {
        RESET_HOLD_AT_B;
    }
    
    // if we have incremented outside of our table, wrap back around to the other side and stop/reset if we are in LF 1 shot mode
    if (position >= spanx2) {
        position = position - spanx2;
    }
    // same as above but for when we are backtracking through the attack phase aka negative increment
    else if (position < 0) {
        position = position + spanx2;
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
