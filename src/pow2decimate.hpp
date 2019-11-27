

using simd::float_4;

template <typename T = float>
struct APPath1 {

	T d1 = T(0);
	T d2 = T(0);
	T a0 = T(0);


	APPath1(float coeff0, float coeff1) {
		a0 = T(coeff0);
	}

	APPath1() {}

	T process(T input) {
		T out1 = (input - d2) * a0 + d1;
        d1 = input;
        d2 = out1;
        return out1;
	}
};

template <typename T = float>
struct APPath2 {

	T d1 = T(0);
	T d2 = T(0);
	T d3 = T(0);

	T a0 = T(0);
	T a1 = T(0);


	APPath2(float coeff0, float coeff1) {
		a0 = T(coeff0);
		a1 = T(coeff1);
	}

	APPath2() {}

	T process(T input) {
		T out1 = (input - d2) * a0 + d1;
        T out2 = (out1 - d3) * a1 + d2;
        d1 = input;
        d2 = out1;
        d3 = out2;
        return out2;
	}
};

/** Decimate by a factor of 32 with cascaded half band filters. */
template <int OVERSAMPLE, int QUALITY>
struct pow2Decimate {
	
	float in32Buffer[64];
	int in32Index;
	float from32to16[7] = {-0.03147689, 0.0, 0.28147607, 0.5, 0.28147607, 0.0, -0.03147689};
	// float_4 from32to16Kernel = {-0.03147689f, 0.28147607f, 0.28147607f, -0.03147689f};

	float in24Buffer[64];
	int in24Index;
	// float from24to8[6] = {0.04774302, 0.167565, 0.28518149, 0.28518149, 0.167565, 0.04774302};
	float from24to8[12] = {-0.00534053, -0.01919541, -0.01717899, 0.0493416, 0.18595231, 0.30651506, 0.30651506, 0.18595231, 0.0493416, -0.01717899, -0.01919541, -0.00534053};
	
	float in16Buffer[32];
	int in16Index;
	float from16to8[7] = {-0.03216907, 0.0, 0.28215172, 0.5, 0.28215172, 0.0, -0.03216907};
	// float_4 from16to8Kernel = {-0.03216907, 0.28215172, 0.28215172, -0.03216907};
	
	float in8Buffer[8];
	
	float in4Buffer[4];

	float in2Buffer[2];

	APPath2<float> from2to1Path1;
	APPath2<float> from2to1Path2;
	APPath2<float> from4to2Path1;
	APPath2<float> from4to2Path2;
	APPath1<float> from8to4Path1;
	APPath2<float> from8to4Path2;

	pow2Decimate(float cutoff = 0.9f) {
		from2to1Path1.a0 = 0.0798664262025582;
		from2to1Path1.a1 = 0.5453236511825826;
		from2to1Path2.a0 = 0.283829344898100;
		from2to1Path2.a1 = 0.834411891201724;
		from4to2Path1.a0 = 0.0798664262025582;
		from4to2Path1.a1 = 0.5453236511825826;
		from4to2Path2.a0 = 0.283829344898100;
		from4to2Path2.a1 = 0.834411891201724;
		from8to4Path1.a0 = 0.11192;
		from8to4Path2.a0 = 0.53976;
		reset();
	}
	void reset() {
		in32Index = 0;
		in24Index = 0;
		in16Index = 0;
		std::memset(in32Buffer, 0, sizeof(in32Buffer));
		std::memset(in24Buffer, 0, sizeof(in24Buffer));
		std::memset(in16Buffer, 0, sizeof(in16Buffer));
		std::memset(in8Buffer, 0, sizeof(in8Buffer));
		std::memset(in4Buffer, 0, sizeof(in4Buffer));
		std::memset(in2Buffer, 0, sizeof(in2Buffer));
	}

	float process(float *in) {
		if (OVERSAMPLE == 2) {
			return process2x(in);
		} else if (OVERSAMPLE == 4) {
			return process4x(in);
		} else if (OVERSAMPLE == 8) {
			return process8x(in);
		} else if (OVERSAMPLE == 16) {
			return process16x(in);
		} else if (OVERSAMPLE == 24) {
			return process24x(in);
		} else if (OVERSAMPLE == 32) {
			return process32x(in);
		}  else {
			return in[0];
		}
	}

	/** `in` must be length 32 */
	float process32x(float *in) {
		
		// copy in the data
		std::memcpy(&in32Buffer[in32Index], in, 32*sizeof(float));		
		// update the write index
		in32Index += 32;
		in32Index &= 63;

		process32to16();
		process16to8();
		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 24 */
	inline float process24x(float *in) {
		
		std::memcpy(&in24Buffer[in24Index], in, 8*sizeof(float));
		in24Index = (in24Index + 8) & 63;

		// copy in the data
		std::memcpy(&in24Buffer[in24Index], &in[8], 8*sizeof(float));
		in24Index = (in24Index + 8) & 63;

		std::memcpy(&in24Buffer[in24Index], &in[16], 8*sizeof(float));
		in24Index = (in24Index + 8) & 63;

		process24to8();
		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 16 */
	float process16x(float *in) {
		
		// copy in the data
		std::memcpy(&in16Buffer[in16Index], in, 16*sizeof(float));		
		// update the write index
		in16Index += 16;
		in16Index &= 31;

		process16to8();
		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 8 */
	float process8x(float *in) {
		
		// copy in the data
		std::memcpy(&in8Buffer[0], in, 8*sizeof(float));

		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 4 */
	float process4x(float *in) {
		
		// copy in the data
		std::memcpy(&in4Buffer[0], in, 4*sizeof(float));

		process4to2();
		return process2to1();

	}

	/** `in` must be length 2 */
	float process2x(float *in) {
		
		// copy in the data
		std::memcpy(&in2Buffer[0], in, 2*sizeof(float));

		return process2to1();

	}

	void inline process32to16(void) {

		int32_t workingIndex = (in32Index - 32) & 63;
		
		// filter every other sample and write to the x16 buffer
		for (int i = 0; i < 32; i += 2) {

			float accumulator = 0.5 * in32Buffer[(workingIndex - 3) & 63];
			// float vectorRead[4];
			
			// float_4 work = {in32Buffer[workingIndex], in32Buffer[(workingIndex - 2) & 63], 
			// 	in32Buffer[(workingIndex - 4) & 63], in32Buffer[(workingIndex - 6) & 63]};
			// work *= from32to16Kernel;

			// work.store(vectorRead);

			// accumulator += vectorRead[0] + vectorRead[1] + vectorRead[2] + vectorRead[3];

			accumulator += from32to16[0] * in32Buffer[workingIndex];
			accumulator += from32to16[2] * in32Buffer[(workingIndex - 2) & 63];
			accumulator += from32to16[4] * in32Buffer[(workingIndex - 4) & 63];
			accumulator += from32to16[6] * in32Buffer[(workingIndex - 6) & 63];
			
			in16Buffer[in16Index] = accumulator;
			in16Index = (in16Index + 1) & 31;
			workingIndex += 2;    
		}
	}

	void inline process24to8(void) {

		uint32_t workingIndex = (in24Index - 24) & 63;

		int writeIndex = 0;
		
		// filter every third sample and write to the x16 buffer
		for (int i = 0; i < 24; i += 3) {

			float accumulator = from24to8[0] * in24Buffer[workingIndex];
			accumulator += from24to8[1] * in24Buffer[(workingIndex - 1) & 63];
			accumulator += from24to8[2] * in24Buffer[(workingIndex - 2) & 63];
			accumulator += from24to8[3] * in24Buffer[(workingIndex - 3) & 63];
			accumulator += from24to8[4] * in24Buffer[(workingIndex - 4) & 63];
			accumulator += from24to8[5] * in24Buffer[(workingIndex - 5) & 63];
			accumulator += from24to8[6] * in24Buffer[(workingIndex - 6) & 63];
			accumulator += from24to8[7] * in24Buffer[(workingIndex - 7) & 63];
			accumulator += from24to8[8] * in24Buffer[(workingIndex - 8) & 63];
			accumulator += from24to8[9] * in24Buffer[(workingIndex - 9) & 63];
			accumulator += from24to8[10] * in24Buffer[(workingIndex - 10) & 63];
			accumulator += from24to8[11] * in24Buffer[(workingIndex - 11) & 63];

			in8Buffer[writeIndex] = in24Buffer[workingIndex];
			writeIndex++;
			workingIndex += 3;
			workingIndex &= 63;
		}

	}

	void inline process16to8(void) {

		int workingIndex = (in16Index - 16) & 31;

		int writeIndex = 0;

		// filter every other sample and write to the x8 buffer
		for (int i = 0; i < 16; i += 2) {

			float accumulator = 0.5 * in16Buffer[(workingIndex - 3) & 31];

			// float vectorRead[4];
			
			// float_4 work = {in16Buffer[workingIndex], in16Buffer[(workingIndex - 2) & 31], 
			// 	in16Buffer[(workingIndex - 4) & 31], in16Buffer[(workingIndex - 6) & 31]};
			// work *= from16to8Kernel;

			// work.store(vectorRead);

			// accumulator += vectorRead[0] + vectorRead[1] + vectorRead[2] + vectorRead[3];

			accumulator += from16to8[0] * in16Buffer[workingIndex];
			accumulator += from16to8[2] * in16Buffer[(workingIndex - 2) & 31];
			accumulator += from16to8[4] * in16Buffer[(workingIndex - 4) & 31];
			accumulator += from16to8[6] * in16Buffer[(workingIndex - 6) & 31];
			
			in8Buffer[writeIndex] = accumulator;
			writeIndex++;
			workingIndex += 2;      
		}

	}

	void inline process8to4(void) {

		in4Buffer[0] = (from8to4Path1.process(in8Buffer[1]) + from8to4Path2.process(in8Buffer[0])) * 0.5;

		in4Buffer[1] = (from8to4Path1.process(in8Buffer[3]) + from8to4Path2.process(in8Buffer[2])) * 0.5;

		in4Buffer[2] = (from8to4Path1.process(in8Buffer[5]) + from8to4Path2.process(in8Buffer[4])) * 0.5;

		in4Buffer[3] = (from8to4Path1.process(in8Buffer[7]) + from8to4Path2.process(in8Buffer[6])) * 0.5;

	}

	void inline  process4to2(void) {

		in2Buffer[0] = (from4to2Path1.process(in4Buffer[1]) + from4to2Path2.process(in4Buffer[0])) * 0.5;

		in2Buffer[1] = (from4to2Path1.process(in4Buffer[3]) + from4to2Path2.process(in4Buffer[2])) * 0.5;


	}


	float inline process2to1(void) {

		return (from2to1Path1.process(in2Buffer[1]) + from2to1Path2.process(in2Buffer[0])) * 0.5;
	}
	
};
