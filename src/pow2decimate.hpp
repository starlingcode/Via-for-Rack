

using simd::float_4;

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
	
	float in8Buffer[16];
	int in8Index;
	float from8to4[7] = {-0.03512255, 0.0, 0.28483447, 0.5, 0.28483447,  0.0, -0.03512255};
	// float_4 from8to4Kernel = {-0.03512255, 0.28483447, 0.28483447, -0.03512255};
	
	float in4Buffer[32];
	int in4Index;
	float from4to2[15] = {-0.00373676, 0.0, 0.02056883, 0.0, -0.0723208, 0.0, 0.30536937, 0.5, 0.30536937, 0.0, -0.0723208, 0.0, 0.02056883, 0.0, -0.00373676};
	// float_4 from4to2Kernel1 = {-0.00373676, 0.02056883, -0.0723208, 0.30536937};
	// float_4 from4to2Kernel2 = {0.30536937, -0.0723208, 0.02056883, -0.00373676};

	float in2Buffer[64];
	int in2Index;
	float from2to1[47] = {-0.00050667, 0.0, 0.00109178, 0.0, -0.00218994, 0.0, 0.00393102, 0.0, -0.00654965, 0.0, 0.0103604, 0.0, -0.01583172, 0.0, 0.02377447, 0.0, 
		-0.03586712, 0.0, 0.05646437, 0.0, -0.1016438, 0.0, 0.31679927, 0.49998081, 0.31679927, 0.0, -0.1016438, 0.0, 0.05646437, 0.0, -0.03586712, 0.0, 0.02377447, 0.0, -0.01583172, 0.0, 0.0103604, 0.0, -0.00654965, 0.0, 0.00393102, 0.0, -0.00218994, 0.0, 0.00109178, 0.0, -0.00050667};
	float_4 from2to1Kernel1 = {-0.00050667, 0.00109178, -0.00218994, 0.00393102};
	float_4 from2to1Kernel2 = {-0.00654965, 0.0103604, -0.01583172, 0.02377447};
	float_4 from2to1Kernel3 = {-0.03586712, 0.05646437, -0.1016438, 0.31679927};
	float_4 from2to1Kernel4 = {0.31679927, -0.1016438, 0.05646437, -0.03586712};
	float_4 from2to1Kernel5 = {0.02377447, -0.01583172, 0.0103604, -0.00654965};
	float_4 from2to1Kernel6 = {0.00393102, -0.00218994, 0.00109178, -0.00050667};

	pow2Decimate(float cutoff = 0.9f) {
		// boxcarLowpassIR(kernel, OVERSAMPLE*QUALITY, cutoff * 0.5f / OVERSAMPLE);
		// blackmanHarrisWindow(kernel, OVERSAMPLE*QUALITY);
		reset();
	}
	void reset() {
		in32Index = 0;
		in24Index = 0;
		in16Index = 0;
		in8Index = 0;
		in4Index = 0;
		in2Index = 0;
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
		std::memcpy(&in8Buffer[in8Index], in, 8*sizeof(float));
		// update the write index
		in8Index += 8;
		in8Index &= 15;

		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 4 */
	float process4x(float *in) {
		
		// copy in the data
		std::memcpy(&in4Buffer[in4Index], in, 4*sizeof(float));
		// update the write index
		in4Index += 4;
		in4Index &= 31;

		process4to2();
		return process2to1();

	}

	/** `in` must be length 2 */
	float process2x(float *in) {
		
		// copy in the data
		std::memcpy(&in2Buffer[in2Index], in, 2*sizeof(float));
		// update the write index
		in2Index += 2;
		in2Index &= 63;

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

			in8Buffer[in8Index] = in24Buffer[workingIndex];
			in8Index = (in8Index + 1) & 15;
			workingIndex += 3;
			workingIndex &= 63;
		}

	}

	void inline process16to8(void) {

		int workingIndex = (in16Index - 16) & 31;

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
			
			in8Buffer[in8Index] = accumulator;
			in8Index = (in8Index + 1) & 15;
			workingIndex += 2;      
		}

	}

	void inline process8to4(void) {

		int workingIndex = (in8Index - 8) & 15;

		// filter every other sample and write to the x4 buffer
		for (int i = 0; i < 8; i += 2) {
			
			float accumulator = 0.5 * in8Buffer[(workingIndex - 3) & 15];

			// float vectorRead[4];
			
			// float_4 work = {in8Buffer[workingIndex], in8Buffer[(workingIndex - 2) & 15], 
			// 	in8Buffer[(workingIndex - 4) & 15], in8Buffer[(workingIndex - 6) & 15]};
			// work *= from8to4Kernel;

			// work.store(vectorRead);

			// accumulator += vectorRead[0] + vectorRead[1] + vectorRead[2] + vectorRead[3];

			accumulator += from8to4[0] * in8Buffer[workingIndex];
			accumulator += from8to4[2] * in8Buffer[(workingIndex - 2) & 15];
			accumulator += from8to4[4] * in8Buffer[(workingIndex - 4) & 15];
			accumulator += from8to4[6] * in8Buffer[(workingIndex - 6) & 15];			
			
			in4Buffer[in4Index] = accumulator;
			in4Index = (in4Index + 1) & 31;
			
			workingIndex += 2;    
		}

	}

	void inline  process4to2(void) {

		int workingIndex = (in4Index - 4) & 31;

		// filter every other sample and write to the x2 buffer
		for (int i = 0; i < 4; i += 2) {
			
			float accumulator = 0.5 * in4Buffer[(workingIndex - 7) & 31]; 

			// float vectorRead[4];

			// float_4 work = {in4Buffer[workingIndex], in4Buffer[(workingIndex - 2) & 31], 
			// 	in4Buffer[(workingIndex - 4) & 31], in4Buffer[(workingIndex - 6) & 31]};
			// work *= from4to2Kernel1;
			// float_4 accumulateVector = work;

			// work = {in4Buffer[(workingIndex - 8) & 31], in4Buffer[(workingIndex - 10) & 31], 
			// 	in4Buffer[(workingIndex - 12) & 31], in4Buffer[(workingIndex - 14) & 31]};
			// work *= from4to2Kernel2;
			// accumulateVector += work;

			// accumulateVector.store(vectorRead);

			// accumulator += vectorRead[0] + vectorRead[1] + vectorRead[2] + vectorRead[3];

			accumulator += from4to2[0] * in4Buffer[workingIndex];
			accumulator += from4to2[2] * in4Buffer[(workingIndex - 2) & 31];
			accumulator += from4to2[4] * in4Buffer[(workingIndex - 4) & 31];
			accumulator += from4to2[6] * in4Buffer[(workingIndex - 6) & 31];
			accumulator += from4to2[8] * in4Buffer[(workingIndex - 8) & 31];
			accumulator += from4to2[10] * in4Buffer[(workingIndex - 10) & 31];
			accumulator += from4to2[12] * in4Buffer[(workingIndex - 12) & 31];
			accumulator += from4to2[14] * in4Buffer[(workingIndex - 14) & 31];

			in2Buffer[in2Index] = accumulator;
			in2Index = (in2Index + 1) & 63;
			workingIndex += 2;    
		}

	}

	// Elliptic filter will make this faster with more passband ripple
	// Higher order butterworth might still be faster

	float inline process2to1(void) {

		int workingIndex = (in2Index - 2) & 63;

		// this time, we only need one sample, so the for loop does the FIR

		float accumulator = 0.49998081 * in2Buffer[(workingIndex - 23) & 63];

		float vectorRead[4];

		float_4 work = {in2Buffer[workingIndex], in2Buffer[(workingIndex - 2) & 63], 
			in2Buffer[(workingIndex - 4) & 63], in2Buffer[(workingIndex - 6) & 63]};
		work *= from2to1Kernel1;
		float_4 accumulateVector = work;

		work = {in2Buffer[(workingIndex - 8) & 63], in2Buffer[(workingIndex - 10) & 63], 
			in2Buffer[(workingIndex - 12) & 63], in2Buffer[(workingIndex - 14) & 63]};
		work *= from2to1Kernel2;
		accumulateVector += work;

		work = {in2Buffer[(workingIndex - 16) & 63], in2Buffer[(workingIndex - 18) & 63], 
			in2Buffer[(workingIndex - 20) & 63], in2Buffer[(workingIndex - 22) & 63]};
		work *= from2to1Kernel3;
		accumulateVector += work;

		work = {in2Buffer[(workingIndex - 24) & 63], in2Buffer[(workingIndex - 26) & 63], 
			in2Buffer[(workingIndex - 28) & 63], in2Buffer[(workingIndex - 30) & 63]};
		work *= from2to1Kernel4;
		accumulateVector += work;

		work = {in2Buffer[(workingIndex - 32) & 63], in2Buffer[(workingIndex - 34) & 63], 
			in2Buffer[(workingIndex - 36) & 63], in2Buffer[(workingIndex - 38) & 63]};
		work *= from2to1Kernel5;
		accumulateVector += work;

		work = {in2Buffer[(workingIndex - 40) & 63], in2Buffer[(workingIndex - 42) & 63], 
			in2Buffer[(workingIndex - 44) & 63], in2Buffer[(workingIndex - 46) & 63]};
		work *= from2to1Kernel6;
		accumulateVector += work;

		accumulateVector.store(vectorRead);

		accumulator += vectorRead[0] + vectorRead[1] + vectorRead[2] + vectorRead[3];

		// for (int i = 0; i < 47; i += 2) {
		// 	accumulator += from2to1[i] * in2Buffer[(workingIndex - i) & 63];
		// }

		return accumulator;
	}
	
};
