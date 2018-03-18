//
//  Via_Sample_Generation.cpp
//  
//
//  Created by WIll Mitchell on 3/17/18.
//
//

#include "Via.hpp"

//////////////// SAMPLE GENERATION SECTION /////////////////


//this is called to write our last sample to the dacs and generate a new sample
void Via::dacISR(void) {
    
    uint32_t storePhase;
    float expoCalc;
    
    
    if ((OSCILLATOR_ACTIVE)) {
        
        
        //get our averages for t2 and morph cv
        
        //getAverages();
        
        //hold the phase state (attack or release) from the last sample
        
        storePhase = PHASE_STATE;
        
        //call the function to advance the phase of the contour generator
        
        getPhase();
        
        //determine whether our newly calculated phase value "position" is in the "attack" or "release" portion of the cycle
        //call the function that generates our next sample based upon the phase value "position"
        //pass that function a 1 or a 0 to indicate whether we are in attack or release
        
        if (position < span) {
            RESET_PHASE_STATE;
            getSample(0);
        }
        if (position >= span) {
            SET_PHASE_STATE;
            getSample(1);
        }
        
        //calculate our "morph" parameter as a function of the morph knob, CV, and our contour generator frequency
        
        //we use morphAverage, which is a running sum of the last 8 values (a running average without the division)
        
        //we basically check whether the morphCV is less than or greater than half of its full scale value
        //if it is less than half of full scale, we do a linear interpolation between our current knob value and 0 using the CV value as the interpolation fraction
        //if it is more than half of full scale, we do a linear interpolation between our current knob value and full scale using the CV value as the interpolation fraction
        //in both of these cases, we generate our interpolation fraction by simple bit shifting a sum involving the halfway scale value and the CV
        //basically, we figure out how far our morphCV is away from the halfway point and then scale that up to a 16 bit integer
        //this works because we assume that all of our ADC ranges are a power of two
        
        //we then scale back our morph value as frequency increases. with a table that exhibits a steadily increasing spectral content as morph increases, this serves as anti-aliasing
        
        //first we clamp our "inc" variable (which is analogous to our contour generator frequency) to max out at 2^20
        //this is our "maximum frequency" at which we find that our "morph" parameter is scaled all the way to 0
        
        if (inc > 1048575) {
            inc = 1048575;
        }
        
        
        //is our CV greater than half-scale (the big numbers are because we have a running sum of 8
        if (morphCV >= 2048) {
            //this first does the aforementioned interpolation between the knob value and full scale then scales back the value according to frequency
            fixMorph = myfix16_lerp(morphKnob, 4095, (morphCV - 2048) << 5);
        }
        else {
            //analogous to above except in this case, morphCV is less than halfway
            fixMorph = myfix16_lerp(0, morphKnob, morphCV << 5);
        }
        
        
        
        
        //if we are in high speed and not looping, activate drum mode
        
        if (DRUM_MODE_ON) {
            
            //this next bit generates our expo decay and scales amp
            //it gets the appropriate value for the expo table and scales into the range of the fix16 fractional component (16 bits)
            
            if (DRUM_ATTACK_ON) {
                
                //maintain a software-based counter to increment through a linear "attack" slope sample per sample
                attackCount = attackCount + 20;
                
                //if we get to our maximum value (this is the index where the value is 2^26)
                //this overflow value gives us a known range for our values from the lookup table
                if (attackCount >= 4094) {
                    //write to the flag word that we are done with our attack slope
                    RESET_DRUM_ATTACK_ON;
                    //since we use this to look up from the table, clamp it at our max value
                    attackCount = 4094;
                    //enable the timer that will generate our release slope
                    
                    //get our value from the lookup table, scale it, this will be 2^16
                    expoCalc = (powf(16, attackCount/4095) - 1) * 4369;
                    expoScale = (int) expoCalc;
                    //reset our counter to 0
                    attackCount = 0;
                    releaseCount = 4094;
                    //indicate that we are now in the "release" phase of our drum envelope
                    SET_DRUM_RELEASE_ON;
                } else {
                    //otherwise, use our counter to look up a value from the table
                    //that gets scaled to 0 - 2^16
                    expoCalc = (powf(16, attackCount/4095) - 1) * 4369;
                    expoScale = (int) expoCalc;
                }
                
            } else if (DRUM_RELEASE_ON) {
                
                releaseCount = releaseCount - (.01 + (1 - params[T2_PARAM].value) * (.5 + .1 * inputs[T2_INPUT].value));
                if (releaseCount <= 0) {
                    
                    releaseCount = 0;
                    
                    
                    SET_LAST_CYCLE;
                }
                
                expoCalc = (powf(16, releaseCount/4095) - 1) * 4369;
                expoScale = (int) expoCalc;
                
            }
            
            
            
            
            
            //scale the contour generator, an integer 0 - 2^16 is 0-1 in our 16 bit fixed point
            if (AMP_ON) {
                out = myfix16_mul(out, expoScale);
            }
            
            //apply the scaling value to the morph knob
            if (MORPH_ON) {
                fixMorph = myfix16_mul(fixMorph, expoScale);
            }
            
            
            
            
        }
        
        // if we transition from one phase state to another, enable the transition handler interrupt
        
        if ((PHASE_STATE) != storePhase) {
            
            EXTI15_10_IRQHandler();
            
        }
        
    }
    
    else {
        
        //turn off the display if the contour generator is inactive and we are not switching modes
        
        if (RGB_ON) {
            
            LEDC_OFF
            LEDD_OFF
            lights[RED_LIGHT].value = 0;
            lights[GREEN_LIGHT].value = 0;
            lights[BLUE_LIGHT].value = 0;
            
        }
        
        
    }
    
    
}

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
                
                incFromADCs = myfix16_mul(
                                          myfix16_mul(150000, lookuptable[time1CV] >> 5), lookuptable[(time1Knob >> 1) + 2047] >> 10) >> tableSizeCompensation;
                
                if (PITCH_ON) {incFromADCs = myfix16_mul(expoScale + 30000, incFromADCs);}
                
            }
            
            else {
                
                incFromADCs = myfix16_mul(myfix16_mul(myfix16_mul((3000 - (4095 -time2CV)) << 7, lookuptable[time1CV] >> 7), lookuptable[time1Knob] >> 4), lookuptable[time2Knob >> 4]) >> tableSizeCompensation;
                
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
    
    time1 = ((lookuptable[4095 - time1CV] >> 13) * (lookuptable[(4095 - time1Knob)] >> 13)) >> (5 + tableSizeCompensation);
    return time1;
    
}

int Via::calcTime2Env(void) {
    
    time2 = ((lookuptable[4095 - time2CV] >> 13) * (lookuptable[(4095 - time2Knob)] >> 13)) >> (7 + tableSizeCompensation);
    return time2;
    
}

int Via::calcTime1Seq(void) {
    
    time1 = ((lookuptable[4095 - time1CV] >> 13) * (lookuptable[(4095 - time1Knob)] >> 13)) >> (9 + tableSizeCompensation);
    return time1;
    
}

int Via::calcTime2Seq(void) {
    
    time2 = ((lookuptable[4095 - time2CV] >> 13) * (lookuptable[(4095 - time2Knob)] >> 13)) >> (9 + tableSizeCompensation);
    return time2;
    
}

void Via::getSample(uint32_t phase) {
    
    // in this function, we use our phase position to get the sample to give to our dacs using biinterpolation
    // essentially, we need to get 4 sample values and two "fractional arguments" (where are we at in between those sample values)
    // think of locating a position on a rectangular surface based upon how far you are between the bottom and top and how far you are between the left and right sides
    // that is basically what we are doing here
    
    uint32_t LnSample; // indicates the nearest neighbor to our position in the wavetable
    uint32_t LnFamily; // indicates the nearest neighbor (wavetable) to our morph value in the family
    uint32_t phaseFrac; // indicates the factional distance between our nearest neighbors in the wavetable
    uint32_t morphFrac; // indicates the factional distance between our nearest neighbors in the family
    uint32_t Lnvalue1; // sample values used by our two interpolations in
    uint32_t Rnvalue1;
    uint32_t Lnvalue2;
    uint32_t Rnvalue2;
    uint32_t interp1; // results of those two interpolations
    uint32_t interp2;
    
    // 0 means we are attacking from A to B, aka we are reading from the slopetable from left to right
    // 1 means we are releasing from B to A, aka we are reading from the slopetable from right to left
    if (phase == 0) {
        
        // we do a lot of tricky bitshifting to take advantage of the structure of a 16 bit fixed point number
        //truncate position then add one to find the relevant indices for our wavetables, first within the wavetable then the actual wavetables in the family
        LnSample = (position >> 16);
        
        //bit shifting to divide by the correct power of two takes a 12 bit number (our fixMorph) and returns the a quotient in the range of our family size
        LnFamily = fixMorph >> morphBitShiftRight;
        
        //determine the fractional part of our phase position by masking off the integer
        phaseFrac = 0x0000FFFF & position;
        // we have to calculate the fractional portion and get it up to full scale
        morphFrac = (fixMorph - (LnFamily << morphBitShiftRight))
        << morphBitShiftLeft;
        
        //get values from the relevant wavetables
        
        //        Lnvalue1 = Via::attackHoldArray[LnFamily][LnSample];
        //        Rnvalue1 = Via::attackHoldArray[LnFamily][LnSample + 1];
        //        Lnvalue2 = Via::attackHoldArray[LnFamily + 1][LnSample];
        //        Rnvalue2 = Via::attackHoldArray[LnFamily + 1][LnSample + 1];
        
        Lnvalue1 = currentFamily.attackFamily[LnFamily][LnSample];
        Rnvalue1 = currentFamily.attackFamily[LnFamily][LnSample + 1];
        Lnvalue2 = currentFamily.attackFamily[LnFamily + 1][LnSample];
        Rnvalue2 = currentFamily.attackFamily[LnFamily + 1][LnSample + 1];
        
        
        //find the interpolated values for the adjacent wavetables using an efficient fixed point linear interpolation
        interp1 = myfix16_lerp(Lnvalue1, Lnvalue2, morphFrac);
        interp2 = myfix16_lerp(Rnvalue1, Rnvalue2, morphFrac);
        
        //interpolate between those based upon the fractional part of our phase pointer
        
        out = myfix16_lerp(interp1, interp2, phaseFrac) >> 3;
        
        //we use the interpolated nearest neighbor samples to determine the sign of rate of change
        //aka, are we moving towrds a, or towards b
        //we use this to generate our gate output
        if (interp1 < interp2) {
            EOA_GATE_HIGH
            if (DELTAB) {
                EOA_JACK_HIGH
                if (RGB_ON) {
                    LEDC_ON
                }
            }
            if (DELTAA) {
                EOR_JACK_LOW
                if (RGB_ON) {
                    LEDD_OFF
                }
            }
        } else if (interp2 < interp1) {
            EOA_GATE_LOW
            if (DELTAB) {
                EOA_JACK_LOW
                if (RGB_ON) {
                    LEDC_OFF
                }
            }
            if (DELTAA) {
                EOR_JACK_HIGH
                if (RGB_ON) {
                    LEDD_ON
                }
            }
        }
        
        if (RGB_ON) {
                lights[BLUE_LIGHT].setBrightnessSmooth(out/4095.0);
                lights[GREEN_LIGHT].setBrightnessSmooth(fixMorph/4095.0);


        }
        
    }
    
    else {
        
        //this section is mostly the same as the "attack" above
        // however, we reflect position back over span to move backwards through our wavetable slope
        LnSample = ((spanx2 - position) >> 16);
        
        LnFamily = fixMorph >> morphBitShiftRight;
        
        // here, again, we use that reflected value
        phaseFrac = 0x0000FFFF & (spanx2 - position);
        
        morphFrac = (uint16_t) ((fixMorph - (LnFamily << morphBitShiftRight))
                                << morphBitShiftLeft);
        
        
        //        Lnvalue1 = Via::releaseHoldArray[LnFamily][LnSample];
        //        Rnvalue1 = Via::releaseHoldArray[LnFamily][LnSample + 1];
        //        Lnvalue2 = Via::releaseHoldArray[LnFamily + 1][LnSample];
        //        Rnvalue2 = Via::releaseHoldArray[LnFamily + 1][LnSample + 1];
        
        Lnvalue1 = currentFamily.releaseFamily[LnFamily][LnSample];
        Rnvalue1 = currentFamily.releaseFamily[LnFamily][LnSample + 1];
        Lnvalue2 = currentFamily.releaseFamily[LnFamily + 1][LnSample];
        Rnvalue2 = currentFamily.releaseFamily[LnFamily + 1][LnSample + 1];
        
        
        interp1 = myfix16_lerp(Lnvalue1, Lnvalue2, morphFrac);
        interp2 = myfix16_lerp(Rnvalue1, Rnvalue2, morphFrac);
        
        out = myfix16_lerp(interp1, interp2, phaseFrac) >> 3;
        
        if (interp1 < interp2) {
            EOA_GATE_HIGH
            if (DELTAB) {
                EOA_JACK_HIGH
                if (RGB_ON) {
                    LEDD_ON
                }
            }
            if (DELTAA) {
                EOR_JACK_LOW
                if (RGB_ON) {
                    LEDC_OFF
                }
            }
        } else if (interp2 < interp1) {
            EOA_GATE_LOW
            if (DELTAB) {
                EOA_JACK_LOW
                if (RGB_ON) {
                    LEDD_OFF
                }
            }
            if (DELTAA) {
                EOR_JACK_HIGH
                if (RGB_ON) {
                    LEDC_ON
                }
            }
        }
        
        //if the runtime display is on, show the current value of our contour generator in blue and morph in green
        if (RGB_ON) {
                lights[RED_LIGHT].setBrightnessSmooth(out/4095.0);
                lights[GREEN_LIGHT].setBrightnessSmooth(fixMorph/4095.0);

        }
    }
    
}

//helper functions to maintain and read from a circular buffer

void Via::write(buffer* buffer, int value) {
    buffer->buff[(buffer->writeIndex++) & BUFF_SIZE_MASK] = value;
}

int Via::readn(buffer* buffer, int Xn) {
    return buffer->buff[(buffer->writeIndex + (~Xn)) & BUFF_SIZE_MASK];
}

//noise at the morph CV is noise in the contour generator signal

void Via::getAverages(void) {
    
    static buffer morphCVBuffer;
    
    //keep a running sum of our last 8 morph CV readings
    
    morphAverage = (morphAverage + morphCV- readn(&morphCVBuffer, 31));
    
    write(&morphCVBuffer, morphCV);
    
}

//our 16 bit fixed point multiply and linear interpolate functions

int Via::myfix16_mul(int in0, int in1) {
    //taken from the fixmathlib library
    int64_t result = (uint64_t) in0 * in1;
    return result >> 16;
}

int Via::myfix16_lerp(int in0, int in1, uint16_t inFract) {
    //taken from the fixmathlib library
    int64_t tempOut = int64_mul_i32_i32(in0, (((int32_t) 1 << 16) - inFract));
    tempOut = int64_add(tempOut, int64_mul_i32_i32(in1, inFract));
    tempOut = int64_shift(tempOut, -16);
    return (int) int64_lo(tempOut);
}


