//
//  Via_Logic.cpp
//  
//
//  Created by WIll Mitchell on 3/17/18.
//
//

#include "Via.hpp"

void Via::risingEdgeHandler(void) {
    if (!(OSCILLATOR_ACTIVE)) {     // contour generator at rest
        //this is how we properly wake up our contour generator
        SET_OSCILLATOR_ACTIVE;      // set the flag that our contour generator is active
        if (DRUM_MODE_ON) {         // perform the operations needed to initiate a drum sound
            SET_DRUM_ATTACK_ON;     //set global flag indicating we are using the timer to generate "attack"
            // logic to be used in the timer interrupt so we pass through and just load prescaler to shadow register
        }
        if (freqMode == env) {
            attackTime = &Via::calcTime1Env;  // set the function pointers for attack and release to the envelope time scale
            releaseTime = &Via::calcTime2Env; // this needs to be done here to ensure that we recover from retrigger behavior
        }
        else if (freqMode == seq) {
            attackTime = &Via::calcTime1Seq;  // set the function pointer for attack and release to the sequence time scale
            releaseTime = &Via::calcTime2Seq;
        }
        if (trigMode == gated) {
            SET_GATE_ON; // turn the gate flag on in gate mode
            incSign = 1;
        }
        sampHoldA();
    }
    else {
        if ((DRUM_MODE_ON) && !(DRUM_ATTACK_ON)) {
            RESET_DRUM_RELEASE_ON;
            SET_DRUM_ATTACK_ON;
            RESET_LAST_CYCLE;
            attackCount = releaseCount;
        }
        else {
            switch (trigMode) {
                    
                case hardsync:
                    position = 0; // hard reset to 0
                    holdPosition = 0;
                    break;
                    
                case gated:
                    if (position < span) { //look to see if we are backtracking, if so, reset the envelope behavior
                        if (attackTime == &Via::calcTime2Env) {
                            attackTime = &Via::calcTime1Env;
                        }
                        else if (attackTime == &Via::calcTime2Seq) {
                            attackTime = &Via::calcTime1Seq;
                        }
                        incSign = 1; // this reverts our direction
                        SET_GATE_ON; // signal that the gate is on
                    }
                    else { //if we are releasing and we get a new gate on, run back up the release slope at attack timescale
                        if (freqMode == env) {
                            releaseTime = &Via::calcTime1Env;
                        }
                        else if (freqMode == seq) {
                            releaseTime = &Via::calcTime1Seq;
                        }
                        incSign = -1; // this reverses the direction
                        SET_GATE_ON;
                    }
                    break;
                    
                case nongatedretrigger:
                    if (position >= span) { // if we are releasing and we get a new gate on, run back up the release slope at attack timescale
                        releaseTime = &Via::calcTime1Env;
                        incSign = -1;
                    }
                    break;
                    
                case pendulum:
                    incSign = incSign * -1;
                    break;
                    
                case pendulum2:
                    if (!(HOLD_AT_B)) { // if we aren't currently gated, reverse the direction of the contour generator
                        incSign = incSign * -1;
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
    if (trigMode == pendulum2 && loopMode == noloop && !(DRUM_MODE_ON)) { // regardless of whether the contour generator is at rest or not, toggle the gateOn every trigger with pendulum
        TOGGLE_GATE_ON;
    }
}


void Via::fallingEdgeHandler(void) {
    
    if (trigMode == gated && !(DRUM_MODE_ON)) { // aka, gate off when we aren't in drum mode
        if (position < span) { // if we release the gate before making it through attack, run back through attack at release speed
            if (freqMode == env) {
                attackTime = &Via::calcTime2Env;
            }
            if (freqMode == seq) {
                attackTime = &Via::calcTime2Seq;
            }
            if (!(HOLD_AT_B)) {incSign = -1;} // -1 in int
            RESET_GATE_ON;
            
        }
        else { // if we get a release when we are at or after span, reset the contour generator behavior and let it finish release
            
            if (freqMode == env) {
                releaseTime = &Via::calcTime2Env;
            };
            if (freqMode == seq) {
                releaseTime = &Via::calcTime2Seq;
            };
            incSign = 1;
            RESET_GATE_ON;
        }
    }
}
    
////////////functions from the interrupt c file//////////////////

void Via::EXTI15_10_IRQHandler(void) {
    
    switchARTimes = 0;
    
    if (!(PHASE_STATE)) {
        
        if (trigMode == nongatedretrigger) {
            incSign = 1;
            
             if (freqMode == 1) {
             releaseTime = &Via::calcTime2Env;
             } else if (freqMode == 2) {
             releaseTime = &Via::calcTime2Seq;
             }
        }
        
        
        
        ALOGIC_HIGH
        if (RGB_ON) {
            if (std::abs(inc) < 4000) {
                LEDC_ON
            } else {
                lights[LED2_LIGHT].value = ((inc >> 5)/65535.0) + .7;
            }
        }
        
        BLOGIC_LOW
        if (RGB_ON) {
            if (std::abs(inc) < 4000) {
                LEDD_OFF
            } else {
                lights[LED4_LIGHT].value = ((inc >> 5)/65535.0) + .7;
            }
        }
        
        
        
        if (inc < 0) {
            sampHoldB();
        } else if (OSCILLATOR_ACTIVE) {
            sampHoldA();
        }
        
        if (RGB_ON && (std::abs(inc) < 4000)) {
            lights[RED_LIGHT].value = 0;
        }
        
    } else {
        
        ALOGIC_LOW
        if (RGB_ON) {
            if (std::abs(inc) < 4000) {
                LEDD_ON
            } else {
                lights[LED4_LIGHT].value = ((inc >> 5)/65535.0) + .7;
            }
        }
        
        BLOGIC_HIGH
        if (RGB_ON) {
            if (std::abs(inc) < 4000) {
                LEDC_OFF
            } else {
                lights[LED2_LIGHT].value = ((inc >> 5)/65535.0) + .7;
            }
        }
        
        if (inc < 0) {
            sampHoldA();
        } else {
            sampHoldB();
        }
        
        if (RGB_ON && (std::abs(inc) < 4000)) {
            lights[BLUE_LIGHT].value = 0;
        }

    }
    
}

void Via::sampHoldB(void) {
    
    switch (sampleHoldMode) {
            
        case a:
            holdA = 0;
            if (RGB_ON) {
                LEDA_ON
            }
            break;
            
            // case b: b remains sampled
            
        case ab:
            holdA = 0;
            if (RGB_ON) {
                LEDA_OFF
            }
            // b remains sampled
            break;
            
        case antidecimate:
            sampleB = 1;
            holdA = 0;
            if (RGB_ON) {
                LEDB_OFF
                LEDA_ON
            }
            break;
            
        case decimate:
            sampleA = 1;
            if (RGB_ON) {
                LEDA_OFF
                LEDB_OFF
            }
            
            break;
            
        default:
            break;
            
    }
    
}

void Via::sampHoldA(void) {
    
    switch (sampleHoldMode) {
            
        case a:
            sampleA = 1;
            if (RGB_ON) {
                LEDA_OFF
            }
            break;
            
        case b:
            sampleB = 1;
            /*
             __HAL_TIM_SET_COUNTER(&htim8, 0);
             __HAL_TIM_ENABLE(&htim8);
             */
            if (RGB_ON) {
                LEDB_OFF
            }
            break;
            
        case ab:
            
            sampleA = 1;
            sampleB = 1;
            if (RGB_ON) {
                LEDB_OFF
                LEDA_ON
            }
            /*
             __HAL_TIM_SET_COUNTER(&htim8, 0);
             __HAL_TIM_ENABLE(&htim8);
             */
            break;
            
        case antidecimate:
            sampleA = 1;
            holdB = 0;
            if (RGB_ON) {
                LEDA_OFF
                LEDB_ON
            }
            break;
            
        case decimate:
            sampleB = 1;
            if (RGB_ON) {
                LEDA_OFF
                ;
                LEDB_OFF
                ;
            }
            /*
             __HAL_TIM_SET_COUNTER(&htim7, 0);
             __HAL_TIM_ENABLE(&htim7);
             */
            break;
            
        default:
            break;
            
    }
    
}





