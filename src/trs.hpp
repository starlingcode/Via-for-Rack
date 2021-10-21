#include "starling.hpp"


using simd::float_4;
using simd::int32_4;

template <typename T = float>
struct ZenerClipperBL {

	T lastIn = T(0);

	T clip(T in) {
		T out = rack::simd::ifelse((in < T(-1.f)), T(-4.f / 5.f), (in - in * in * in * in * in / T(5.f)));
		return rack::simd::ifelse((in < T(1.f)), out, T(4.f / 5.f));
	}

	T integratedClip(T in) {
		T out = rack::simd::ifelse((in < T(-1.f)), T(-4.f / 5.f) * in - T(1.f/3.f), (in * in) / T(2.f) - (in * in * in * in * in * in) / T(30.f));
		return rack::simd::ifelse(in < T(1.f), out, T(4.f / 5.f) * in - T(1.f/3.f));
	}

	T process(T in) {

		T out = rack::simd::ifelse(abs(in - lastIn) < .000001f, clip((in + lastIn)/2), (integratedClip(in) - integratedClip(lastIn))/(in - lastIn));
		lastIn = in;
		return out;
	}

};

// This one can process a float_4 due to simple explicit formula for inverting 2 x 2 matrix
// Includes some trivial modifications to https://github.com/google/music-synthesizer-for-android/blob/master/lab/Second%20order%20sections%20in%20matrix%20form.ipynb
// for simultaineuous outputs from same state/transition matrix
// nice and stable and linear

template <typename T = float>
struct ZDFSVF {

	// storage for the discretized coefficients

	T Az[4] = {T(0.f), T(0.f),
			   T(0.f), T(0.f)};
	T Bz[2] = {T(0.f), T(0.f)};
	T CLPz[2] = {T(0.f), T(0.f)};
	T CBPz[2] = {T(0.f), T(0.f)};
	T CHPz[2] = {T(0.f), T(0.f)};
	T DLPz = T(0.f);
	T DBPz = T(0.f);
	T DHPz = T(0.f);

	T X[2] = {T(0.f), T(0.f)};

	T lpOut = T(0);
	T bpOut = T(0);
	T hpOut = T(0);

	// T diodeGain = T(1.f);

	void setParams(T freq, T res) {

		T g = tan(T(M_PI) * freq);
    	T k = T(2.f) - T(2) * res;
    	// k *= diodeGain;
   		T a1 = T(1.f)/(T(1.f) + g * (g + k));
    	T a2 = g * a1;
   		T a3 = g * a2;

   		Az[0] = T(2.f) * a1 - T(1.f);
   		Az[1] = T(-2.f) * a2;
   		Az[2] = T(2.f) * a2;
   		Az[3] = T(1.f) - T(2.f) * a3;

   		// printf("A : %4.4f, %4.4f \n", Az[0][0], Az[1][0]);
   		// printf("A : %4.4f, %4.4f \n\n", Az[2][0], Az[3][0]);

   		Bz[0] = T(2.f) * a2;
   		Bz[1] = T(2.f) * a3;

   		// printf("B : %4.4f, %4.4f \n\n", Bz[0][0], Bz[1][0]);

   		CLPz[0] = a2;
   		CLPz[1] = T(1.f) - a3;

   		// printf("CLP : %4.4f, %4.4f \n\n", CLPz[0][0], CLPz[1][0]);

   		CBPz[0] = a1;
   		CBPz[1] = -a2;

   		CHPz[0] = -k * a1 - a2;
   		CHPz[1] = k * a2 - (T(1.f) - a3);

   		DLPz = a3;
   		DBPz = a2;
   		DHPz = T(1.f) - k * a2 - a3;

	}

	ZenerClipperBL<T> clipper;

	void process(T in) {

		lpOut = in * DLPz + X[0] * CLPz[0] + X[1] * CLPz[1];
		bpOut = in * DBPz + X[0] * CBPz[0] + X[1] * CBPz[1];
		hpOut = in * DHPz + X[0] * CHPz[0] + X[1] * CHPz[1];

		T X0 = in * Bz[0] + X[0] * Az[0] + X[1] * Az[1];
		T X1 = in * Bz[1] + X[0] * Az[2] + X[1] * Az[3];

		X[0] = clipper.clip(X0 / T(10.f)) * T(10.f);
		X[0] = X0;
		X[1] = X1;

	}

};

