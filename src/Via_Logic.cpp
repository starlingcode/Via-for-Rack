//
//  Via_Logic.cpp
//  
//
//  Created by WIll Mitchell on 3/17/18.
//
//

#include "Via.hpp"

void Via::risingEdgeHandler(void) {
    
    if (!(OSCILLATOR_ACTIVE)) {
        
        SET_OSCILLATOR_ACTIVE;
        incSign = 1;
        if (DRUM_MODE_ON) {
            SET_DRUM_ATTACK_ON;
        }
        else if (trigMode == 3) {
            SET_GATE_ON;
        }
        
    } else {
        
        if ((DRUM_MODE_ON) && !(DRUM_ATTACK_ON)) {
            
            RESET_DRUM_RELEASE_ON;
            SET_DRUM_ATTACK_ON;
            RESET_LAST_CYCLE;
            attackCount = releaseCount;
            
        } else {
            
            switch (trigMode) {
                case 1:
                    
                    position = 0;
                    break;
                    
                case 2:
                    
                    if ((PHASE_STATE) && (!switchARTimes)) {
                        switchARTimes = 1;
                        incSign = -1;
                    }
                    break;
                    
                case 3:
                    
                    SET_GATE_ON;
                    
                    if (!(PHASE_STATE) && switchARTimes) {
                        switchARTimes = 0;
                        incSign = 1;
                    }
                    
                    
                    if ((PHASE_STATE) && !switchARTimes) {
                        switchARTimes = 1;
                        incSign = -1;
                    }
                    break;
                    
                case 4:
                    
                    
                    if (!(HOLD_AT_B)) {
                        if (incSign == 1) {
                            incSign = -1;
                        } else {
                            incSign = 1;
                        }
                    }
                    break;
                    
                default:
                    break;
                    
            }
        }
    }
    if ((trigMode == 4) && (loopMode == 0)) { // regardless of whether the contour generator is at rest or not, toggle the gateOn every trigger with pendulum
        
        TOGGLE_GATE_ON;
        
    }
    
    
}


void Via::fallingEdgeHandler(void) {
    
    if (trigMode == gated && !(DRUM_MODE_ON)) { //aka, gate off when we aren't in drum mode
        
        if (position < span) { //if we release the gate before making it through attack, run back through attack at release speed
            
            switchARTimes = 1;
            incSign = -1; // -1 in int
            RESET_GATE_ON;
            
        } else { //if we get a release when we are at or after span, reset the contour generator behavior and let it finish release
            
            switchARTimes = 0;
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
            /*
             if (freqMode == 1) {
             releaseTime = calcTime2Env;
             } else if (freqMode == 2) {
             releaseTime = calcTime2Seq;
             }*/
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





