//
//  Via_Sample_Generation.cpp
//  
//
//  Created by WIll Mitchell on 3/17/18.
//
//

#include "Via.hpp"

//////////////// SAMPLE GENERATION SECTION /////////////////


// this mirrors the dac timer interrupt handler in the hardware implementation
// it parses the controls and flags from the retriggering logic and generates the next sample

void Via::dacISR(void) {
    
    uint32_t storePhase;
    
    if ((OSCILLATOR_ACTIVE)) {
        
        //hold the phase state (attack or release) from the last sample
        storePhase = PHASE_STATE;
        
        //call the function to advance the phase of the contour generator
        (this->*getPhase)();
        
        //call the function that generates our next sample based upon the phase value "position"
        getSampleQuinticSpline();
        
        //determine whether phase value "position" is in the "attack" or "release" portion of the cycle
        
        if (position < span) {
            RESET_PHASE_STATE;
            if (RGB_ON) {
                lights[BLUE_LIGHT].setBrightnessSmooth(out/4095.0);
                lights[GREEN_LIGHT].setBrightnessSmooth(fixMorph/4095.0);
            }
        }
        if (position >= span) {
            SET_PHASE_STATE;
            if (RGB_ON) {
                lights[RED_LIGHT].setBrightnessSmooth(out/4095.0);
                lights[GREEN_LIGHT].setBrightnessSmooth(fixMorph/4095.0);
            }
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
            fixMorph = fix16_lerp(morphKnob, 4095, (morphCV - 2048) << 5);
        }
        else {
            //analogous to above except in this case, morphCV is less than halfway
            fixMorph = fix16_lerp(0, morphKnob, morphCV << 5);
        }
        
        //if we are in high speed and not looping, activate drum mode
        
        if (DRUM_MODE_ON) {
            generateDrumEnvelope();
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

void Via::getSampleQuinticSpline(void) {
    
    // adapted from code shared by Josh Scholar on musicDSP.org
    // https://web.archive.org/web/20170705065209/http://www.musicdsp.org/showArchiveComment.php?ArchiveID=60
    
    /* in this function, we use our phase position to get the sample to give to our dacs using a quintic spline interpolation technique
    essentially, we need to get 6 pairs of sample values and two "fractional arguments" (where are we at in between those sample values)
    one fractional argument determines the relative position in a linear interpolation between the sample values (morph)
    the other is the fractional phase argument used in the interpolation calculation
    */
    
    uint32_t LnSample;  // indicates the left most sample in the neighborhood of sample values around the phase pointer
    uint32_t LnFamily;  // indicates the nearest neighbor (wavetable) to our morph value in the family
    uint32_t waveFrac;  // indicates the fractional distance between the nearest sample values in terms of phase
    uint32_t morphFrac; // indicates the fractional distance between our nearest neighbors in the family
    uint32_t lFvalue0; // samples
    uint32_t rFvalue0;
    uint32_t lFvalue1;
    uint32_t rFvalue1;
    uint32_t lFvalue2;
    uint32_t rFvalue2;
    uint32_t lFvalue3;
    uint32_t rFvalue3;
    uint32_t lFvalue4;
    uint32_t rFvalue4;
    uint32_t lFvalue5;
    uint32_t rFvalue5;
    uint32_t interp0;  // results of the interpolations between sample pairs
    uint32_t interp1;
    uint32_t interp2;
    uint32_t interp3;
    uint32_t interp4;
    uint32_t interp5;

    // we do a lot of tricky bitshifting to take advantage of the structure of a 16 bit fixed point number
    // truncate position then add one to find the relevant indices for our wavetables, first within the wavetable then the actual wavetables in the family
    LnSample = (position >> 16) + 2;
    
    // bit shifting to divide by the correct power of two takes a 12 bit number (our fixMorph) and returns the a quotient in the range of our family size
    LnFamily = fixMorph >> morphBitShiftRight;
    
    // determine the fractional part of our phase position by masking off the integer
    waveFrac = 0x0000FFFF & position;
    
    // we have to calculate the fractional portion and get it up to full scale q31_t
    morphFrac = (fixMorph - (LnFamily << morphBitShiftRight)) << morphBitShiftLeft;
    
    lFvalue0 = tableHoldArray[LnFamily][LnSample - 2];
    lFvalue1 = tableHoldArray[LnFamily][LnSample - 1];
    lFvalue2 = tableHoldArray[LnFamily][LnSample];
    lFvalue3 = tableHoldArray[LnFamily][LnSample + 1];
    lFvalue4 = tableHoldArray[LnFamily][LnSample + 2];
    lFvalue5 = tableHoldArray[LnFamily][LnSample + 3];
    rFvalue0 = tableHoldArray[LnFamily + 1][LnSample- 2];
    rFvalue1 = tableHoldArray[LnFamily + 1][LnSample - 1];
    rFvalue2 = tableHoldArray[LnFamily + 1][LnSample];
    rFvalue3 = tableHoldArray[LnFamily + 1][LnSample + 1];
    rFvalue4 = tableHoldArray[LnFamily + 1][LnSample + 2];
    rFvalue5 = tableHoldArray[LnFamily + 1][LnSample + 3];
    
    // find the interpolated values for the adjacent wavetables using an efficient fixed point linear interpolation
    interp0 = fix16_lerp(lFvalue0, rFvalue0, morphFrac);
    interp1 = fix16_lerp(lFvalue1, rFvalue1, morphFrac);
    interp2 = fix16_lerp(lFvalue2, rFvalue2, morphFrac);
    interp3 = fix16_lerp(lFvalue3, rFvalue3, morphFrac);
    interp4 = fix16_lerp(lFvalue4, rFvalue4, morphFrac);
    interp5 = fix16_lerp(lFvalue5, rFvalue5, morphFrac);
    
    out = (interp2
           + fix24_mul(699051, fix16_mul(waveFrac, ((interp3-interp1)*16 + (interp0-interp4)*2
               + fix16_mul(waveFrac, ((interp3+interp1)*16 - interp0 - interp2*30 - interp4
                   + fix16_mul(waveFrac, (interp3*66 - interp2*70 - interp4*33 + interp1*39 + interp5*7 - interp0*9
                       + fix16_mul(waveFrac, ( interp2*126 - interp3*124 + interp4*61 - interp1*64 - interp5*12 + interp0*13
                            + fix16_mul(waveFrac, ((interp3-interp2)*50 + (interp1-interp4)*25 + (interp5-interp0) * 5))
                       ))
                   ))
               ))
           ))
           ));
    
    out = out >> 3;
    
    if (out > 4095){out = 4095;}
    else if (out < 0){out = 0;}
    
    // we use the interpolated nearest neighbor samples to determine the sign of rate of change
    // aka, are we moving towrds a, or towards b
    // we use this to generate our gate output
    if (interp2 < interp3) {
        EXPAND_GATE_HIGH;
    } else if (interp3 < interp2) {
        EXPAND_GATE_LOW;
    }
}

void Via::generateDrumEnvelope(void) {

    //this  generates our expo decay and scales amp
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
        out = fix16_mul(out, expoScale);
    }
    //apply the scaling value to the morph knob
    if (MORPH_ON) {
        fixMorph = fix16_mul(fixMorph, expoScale);
    }
}

// fixed point multiply and linear interpolate functions from libfixmath

int Via::fix16_mul(int in0, int in1) {
    //taken from the fixmathlib library
    int64_t result = (uint64_t) in0 * in1;
    return result >> 16;
}

int Via::fix24_mul(int in0, int in1) {
    //taken from the fixmathlib library
    int64_t result = (uint64_t) in0 * in1;
    return result >> 24;
}

int Via::fix16_lerp(int in0, int in1, uint16_t inFract) {
    //taken from the fixmathlib library
    int64_t tempOut = int64_mul_i32_i32(in0, (((int32_t) 1 << 16) - inFract));
    tempOut = int64_add(tempOut, int64_mul_i32_i32(in1, inFract));
    tempOut = int64_shift(tempOut, -16);
    return (int) int64_lo(tempOut);
}

// this shuttles the sample data from the holding object to an array padded for spline interpolation
void Via::loadSampleArray(Family family) {
    
    uint32_t numSamples = family.tableLength;
    
    //for each table in the family
    for (uint32_t i = 0; i < family.familySize; i++) {
        //include the "last two" samples from release
        Via::tableHoldArray[i][0] = *(*(family.releaseFamily + i) + 0);
        tableHoldArray[i][1] = *(*(family.releaseFamily + i) + 0);
        //fill in a full cycle's worth of samples
        //the release gets reversed
        //we drop the last sample from attack and the first from releas
        for (uint32_t j = 0;j < numSamples; j++) {
            tableHoldArray[i][2 + j] = *(*(family.attackFamily + i) + j);
            tableHoldArray[i][2 + numSamples + j] = *(*(family.releaseFamily + i) + family.tableLength - j);
        }
        //pad out the "first two" samples from attack
        tableHoldArray[i][(numSamples << 1) + 2] = *(*(family.attackFamily + i) + 0);
        tableHoldArray[i][(numSamples << 1) + 3] = *(*(family.attackFamily + i) + 0);
        tableHoldArray[i][(numSamples << 1) + 4] = *(*(family.attackFamily + i) + 0);
    }
}

