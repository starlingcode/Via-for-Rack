#include "rack.hpp"


using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct MyModuleWidget : ModuleWidget {
	MyModuleWidget();
};



struct SH_Button : SVGSwitch, MomentarySwitch {
    SH_Button() {
        addFrame(SVG::load(assetPlugin(plugin, "res/S+H_button.svg")));
    }
};


struct Trig_Button : SVGSwitch, MomentarySwitch {
    Trig_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/trig_button.svg")));
    }
};


struct Freq_Button : SVGSwitch, MomentarySwitch {
    Freq_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/freq_button.svg")));
    }
};


struct Loop_Button : SVGSwitch, MomentarySwitch {
    Loop_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/loop_button.svg")));
    }
};


struct Up_Button : SVGSwitch, MomentarySwitch {
    Up_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/up_button.svg")));
    }
};


struct Down_Button : SVGSwitch, MomentarySwitch {
    Down_Button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/down_button.svg")));
    }
};

struct VIA_manual_button : SVGSwitch, MomentarySwitch {
    VIA_manual_button() {
        addFrame(SVG::load(assetPlugin(plugin,"res/manual_trig.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/manual_trig_down.svg")));
    }
};


#ifndef __libfixmath_int64_h__
#define __libfixmath_int64_h__

#ifdef __cplusplus
extern "C"
{
#endif
    
#ifndef FIXMATH_NO_64BIT
    static inline  int64_t int64_const(int32_t hi, uint32_t lo) { return (((int64_t)hi << 32) | lo); }
    static inline  int64_t int64_from_int32(int32_t x) { return (int64_t)x; }
    static inline  int32_t int64_hi(int64_t x) { return (x >> 32); }
    static inline uint32_t int64_lo(int64_t x) { return (x & ((1ULL << 32) - 1)); }
    
    static inline int64_t int64_add(int64_t x, int64_t y)   { return (x + y);  }
    static inline int64_t int64_neg(int64_t x)              { return (-x);     }
    static inline int64_t int64_sub(int64_t x, int64_t y)   { return (x - y);  }
    static inline int64_t int64_shift(int64_t x, int8_t y)  { return (y < 0 ? (x >> -y) : (x << y)); }
    
    static inline int64_t int64_mul_i32_i32(int32_t x, int32_t y) { return (x * y);  }
    static inline int64_t int64_mul_i64_i32(int64_t x, int32_t y) { return (x * y);  }
    
    static inline int64_t int64_div_i64_i32(int64_t x, int32_t y) { return (x / y);  }
    
    static inline int int64_cmp_eq(int64_t x, int64_t y) { return (x == y); }
    static inline int int64_cmp_ne(int64_t x, int64_t y) { return (x != y); }
    static inline int int64_cmp_gt(int64_t x, int64_t y) { return (x >  y); }
    static inline int int64_cmp_ge(int64_t x, int64_t y) { return (x >= y); }
    static inline int int64_cmp_lt(int64_t x, int64_t y) { return (x <  y); }
    static inline int int64_cmp_le(int64_t x, int64_t y) { return (x <= y); }
#else
    
    typedef struct {
        int32_t hi;
        uint32_t lo;
    } __int64_t;
    
    static inline __int64_t int64_const(int32_t hi, uint32_t lo) { return (__int64_t){ hi, lo }; }
    static inline __int64_t int64_from_int32(int32_t x) { return (__int64_t){ (x < 0 ? -1 : 0), x }; }
    static inline   int32_t int64_hi(__int64_t x) { return x.hi; }
    static inline  uint32_t int64_lo(__int64_t x) { return x.lo; }
    
    static inline int int64_cmp_eq(__int64_t x, __int64_t y) { return ((x.hi == y.hi) && (x.lo == y.lo)); }
    static inline int int64_cmp_ne(__int64_t x, __int64_t y) { return ((x.hi != y.hi) || (x.lo != y.lo)); }
    static inline int int64_cmp_gt(__int64_t x, __int64_t y) { return ((x.hi > y.hi) || ((x.hi == y.hi) && (x.lo >  y.lo))); }
    static inline int int64_cmp_ge(__int64_t x, __int64_t y) { return ((x.hi > y.hi) || ((x.hi == y.hi) && (x.lo >= y.lo))); }
    static inline int int64_cmp_lt(__int64_t x, __int64_t y) { return ((x.hi < y.hi) || ((x.hi == y.hi) && (x.lo <  y.lo))); }
    static inline int int64_cmp_le(__int64_t x, __int64_t y) { return ((x.hi < y.hi) || ((x.hi == y.hi) && (x.lo <= y.lo))); }
    
    static inline __int64_t int64_add(__int64_t x, __int64_t y) {
        __int64_t ret;
        ret.hi = x.hi + y.hi;
        ret.lo = x.lo + y.lo;
        if((ret.lo < x.lo) || (ret.hi < y.hi))
            ret.hi++;
        return ret;
    }
    
    static inline __int64_t int64_neg(__int64_t x) {
        __int64_t ret;
        ret.hi = ~x.hi;
        ret.lo = ~x.lo + 1;
        if(ret.lo == 0)
            ret.hi++;
        return ret;
    }
    
    static inline __int64_t int64_sub(__int64_t x, __int64_t y) {
        return int64_add(x, int64_neg(y));
    }
    
    static inline __int64_t int64_shift(__int64_t x, int8_t y) {
        __int64_t ret;
        if(y > 0) {
            if(y >= 32)
                return (__int64_t){ 0, 0 };
            ret.hi = (x.hi << y) | (x.lo >> (32 - y));
            ret.lo = (x.lo << y);
        } else {
            y = -y;
            if(y >= 32)
                return (__int64_t){ 0, 0 };
            ret.lo = (x.lo >> y) | (x.hi << (32 - y));
            ret.hi = (x.hi >> y);
        }
        return ret;
    }
    
    static inline __int64_t int64_mul_i32_i32(int32_t x, int32_t y) {
        int16_t hi[2] = { (x >> 16), (y >> 16) };
        uint16_t lo[2] = { (x & 0xFFFF), (y & 0xFFFF) };
        
        int32_t r_hi = hi[0] * hi[1];
        int32_t r_md = (hi[0] * lo[1]) + (hi[1] * lo[0]);
        uint32_t r_lo = lo[0] * lo[1];
        
        r_hi += (r_md >> 16);
        r_lo += (r_md << 16);
        
        return (__int64_t){ r_hi, r_lo };
    }
    
    static inline __int64_t int64_mul_i64_i32(__int64_t x, int32_t y) {
        int neg = ((x.hi ^ y) < 0);
        if(x.hi < 0)
            x = int64_neg(x);
        if(y < 0)
            y = -y;
        
        uint32_t _x[4] = { (x.hi >> 16), (x.hi & 0xFFFF), (x.lo >> 16), (x.lo & 0xFFFF) };
        uint32_t _y[2] = { (y >> 16), (y & 0xFFFF) };
        
        uint32_t r[4];
        r[0] = (_x[0] * _y[0]);
        r[1] = (_x[1] * _y[0]) + (_x[0] * _y[1]);
        r[2] = (_x[1] * _y[1]) + (_x[2] * _y[0]);
        r[3] = (_x[2] * _y[0]) + (_x[1] * _y[1]);
        
        __int64_t ret;
        ret.lo = r[0] + (r[1] << 16);
        ret.hi = (r[3] << 16) + r[2] + (r[1] >> 16);
        return (neg ? int64_neg(ret) : ret);
    }
    
    static inline __int64_t int64_div_i64_i32(__int64_t x, int32_t y) {
        int neg = ((x.hi ^ y) < 0);
        if(x.hi < 0)
            x = int64_neg(x);
        if(y < 0)
            y = -y;
        
        __int64_t ret = { (x.hi / y) , (x.lo / y) };
        x.hi = x.hi % y;
        x.lo = x.lo % y;
        
        __int64_t _y = int64_from_int32(y);
        
        __int64_t i;
        for(i = int64_from_int32(1); int64_cmp_lt(_y, x); _y = int64_shift(_y, 1), i = int64_shift(i, 1));
        
        while(x.hi) {
            _y = int64_shift(_y, -1);
            i = int64_shift(i, -1);
            if(int64_cmp_ge(x, _y)) {
                x = int64_sub(x, _y);
                ret = int64_add(ret, i);
            }
        }
        
        ret = int64_add(ret, int64_from_int32(x.lo / y));
        return (neg ? int64_neg(ret) : ret);
    }
    
#define int64_t __int64_t
    
#endif
    
#ifdef __cplusplus
}
#endif

#endif





#define time1Knob ((int) (params[T1_PARAM].value * 4095))
#define time2Knob ((int) (params[T2_PARAM].value * 4095))
#define morphKnob ((int) (params[MORPH_PARAM].value * 4095))
#define BKnob params[B_PARAM].value
#define AKnob params[A_PARAM].value
#define time1CV ((int) (((inputs[T1_INPUT].value/10) + .5) * 4095))
#define time2CV ((int)((((inputs[T2_INPUT].value * params[T2AMT_PARAM].value)/10) + .5) * 4095))
#define morphCV ((int)((((inputs[MORPH_INPUT].value * params[MORPHAMT_PARAM].value)/10) + .5) * 4095))




#define EOA_JACK_HIGH outputs[LOGICB_OUTPUT].value = 5.0;
#define EOA_JACK_LOW outputs[LOGICB_OUTPUT].value = 0;

#define EOA_GATE_HIGH outputs[DELTA_OUTPUT].value = 5.0;
#define EOA_GATE_LOW outputs[DELTA_OUTPUT].value = 0;

#define EOR_JACK_HIGH outputs[LOGICA_OUTPUT].value = 5.0;
#define EOR_JACK_LOW outputs[LOGICA_OUTPUT].value = 0;



#define LEDA_ON lights[LED1_LIGHT].value = 1;
#define LEDA_OFF lights[LED1_LIGHT].value = 0;

#define LEDB_ON lights[LED3_LIGHT].value = 1;
#define LEDB_OFF lights[LED3_LIGHT].value = 0;

#define LEDC_ON lights[LED2_LIGHT].value = 1;
#define LEDC_OFF lights[LED2_LIGHT].value = 0;

#define LEDD_ON lights[LED4_LIGHT].value = 1;
#define LEDD_OFF lights[LED4_LIGHT].value = 0;










#define BUFF_SIZE 32
#define BUFF_SIZE_MASK (BUFF_SIZE-1)







/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/




#define PHASE_STATE 		flagHolder & 0b00000000000000000000000000000001
#define LAST_PHASE_STATE 	flagHolder & 0b00000000000000000000000000000010
#define GATE_ON 			flagHolder & 0b00000000000000000000000000000100
#define RGB_ON 				flagHolder & 0b00000000000000000000000000001000
#define UPDATE_PRESCALER 	flagHolder & 0b00000000000000000000000000010000
#define DRUM_MODE_ON 		flagHolder & 0b00000000000000000000000000100000
#define DRUM_ATTACK_ON 		flagHolder & 0b00000000000000000000000001000000
#define DRUM_RELEASE_ON 	flagHolder & 0b00000000000000000000000010000000
#define PITCH_ON 			flagHolder & 0b00000000000000000000000100000000
#define MORPH_ON 			flagHolder & 0b00000000000000000000001000000000
#define AMP_ON 				flagHolder & 0b00000000000000000000010000000000
#define DRUM_OFF 			flagHolder & 0b00000000000000000000100000000000
#define LAST_CYCLE 			flagHolder & 0b00000000000000000001000000000000
#define HOLD_AT_B 			flagHolder & 0b00000000000000000010000000000000
#define OSCILLATOR_ACTIVE 	flagHolder & 0b00000000000000000100000000000000
#define TRIGGER_BUTTON	 	flagHolder & 0b00000000000000001000000000000000
#define DETECT_ON	 		flagHolder & 0b00000000000000010000000000000000
#define AUX_MENU	 		flagHolder & 0b00000000000000100000000000000000
#define GATEA		 		flagHolder & 0b00000000000001000000000000000000
#define TRIGA		 		flagHolder & 0b00000000000010000000000000000000
#define DELTAA		 		flagHolder & 0b00000000000100000000000000000000
#define GATEB		 		flagHolder & 0b00000000001000000000000000000000
#define TRIGB		 		flagHolder & 0b00000000010000000000000000000000
#define DELTAB		 		flagHolder & 0b00000000100000000000000000000000


#define SET_PHASE_STATE 		flagHolder |= 0b00000000000000000000000000000001
#define SET_LAST_PHASE_STATE 	flagHolder |= 0b00000000000000000000000000000010
#define SET_GATE_ON 			flagHolder |= 0b00000000000000000000000000000100
#define SET_RGB_ON 				flagHolder |= 0b00000000000000000000000000001000
#define SET_UPDATE_PRESCALER 	flagHolder |= 0b00000000000000000000000000010000
#define SET_DRUM_MODE_ON 		flagHolder |= 0b00000000000000000000000000100000
#define SET_DRUM_ATTACK_ON 		flagHolder |= 0b00000000000000000000000001000000
#define SET_DRUM_RELEASE_ON 	flagHolder |= 0b00000000000000000000000010000000
#define SET_PITCH_ON 			flagHolder |= 0b00000000000000000000000100000000
#define SET_MORPH_ON 			flagHolder |= 0b00000000000000000000001000000000
#define SET_AMP_ON 				flagHolder |= 0b00000000000000000000010000000000
#define SET_DRUM_OFF 			flagHolder |= 0b00000000000000000000100000000000
#define SET_LAST_CYCLE 			flagHolder |= 0b00000000000000000001000000000000
#define SET_HOLD_AT_B 			flagHolder |= 0b00000000000000000010000000000000
#define SET_OSCILLATOR_ACTIVE 	flagHolder |= 0b00000000000000000100000000000000
#define SET_TRIGGER_BUTTON	 	flagHolder |= 0b00000000000000001000000000000000
#define SET_DETECT_ON	 		flagHolder |= 0b00000000000000010000000000000000
#define SET_AUX_MENU			flagHolder |= 0b00000000000000100000000000000000
#define SET_GATEA		 		flagHolder |= 0b00000000000001000000000000000000
#define SET_TRIGA		 		flagHolder |= 0b00000000000010000000000000000000
#define SET_DELTAA		 		flagHolder |= 0b00000000000100000000000000000000
#define SET_GATEB		 		flagHolder |= 0b00000000001000000000000000000000
#define SET_TRIGB		 		flagHolder |= 0b00000000010000000000000000000000
#define SET_DELTAB		 		flagHolder |= 0b00000000100000000000000000000000




#define RESET_PHASE_STATE 		flagHolder &= 0b11111111111111111111111111111110
#define RESET_LAST_PHASE_STATE 	flagHolder &= 0b11111111111111111111111111111101
#define RESET_GATE_ON 			flagHolder &= 0b11111111111111111111111111111011
#define RESET_RGB_ON 			flagHolder &= 0b11111111111111111111111111110111
#define RESET_UPDATE_PRESCALER 	flagHolder &= 0b11111111111111111111111111101111
#define RESET_DRUM_MODE_ON 		flagHolder &= 0b11111111111111111111111111011111
#define RESET_DRUM_ATTACK_ON 	flagHolder &= 0b11111111111111111111111110111111
#define RESET_DRUM_RELEASE_ON 	flagHolder &= 0b11111111111111111111111101111111
#define RESET_PITCH_ON 			flagHolder &= 0b11111111111111111111111011111111
#define RESET_MORPH_ON 			flagHolder &= 0b11111111111111111111110111111111
#define RESET_AMP_ON 			flagHolder &= 0b11111111111111111111101111111111
#define RESET_DRUM_OFF 			flagHolder &= 0b11111111111111111111011111111111
#define RESET_LAST_CYCLE 		flagHolder &= 0b11111111111111111110111111111111
#define RESET_HOLD_AT_B 		flagHolder &= 0b11111111111111111101111111111111
#define RESET_OSCILLATOR_ACTIVE flagHolder &= 0b11111111111111111011111111111111
#define RESET_TRIGGER_BUTTON	flagHolder &= 0b11111111111111110111111111111111
#define RESET_DETECT_ON			flagHolder &= 0b11111111111111101111111111111111
#define RESET_AUX_MENU			flagHolder &= 0b11111111111111011111111111111111
#define RESET_GATEA				flagHolder &= 0b11111111111110111111111111111111
#define RESET_TRIGA				flagHolder &= 0b11111111111101111111111111111111
#define RESET_DELTAA			flagHolder &= 0b11111111111011111111111111111111
#define RESET_GATEB				flagHolder &= 0b11111111110111111111111111111111
#define RESET_TRIGB				flagHolder &= 0b11111111101111111111111111111111
#define RESET_DELTAB			flagHolder &= 0b11111111011111111111111111111111



#define TOGGLE_GATE_ON 			flagHolder ^= 0b00000000000000000000000000000100

#define REMEMBER_PHASE_STATE	flagHolder ^= (-((PHASE_STATE) ^ flagHolder) & (1UL << 1))

