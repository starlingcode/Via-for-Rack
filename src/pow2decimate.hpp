

using simd::float_4;

template <typename T = float>
struct APPath1 {

	APPath1() {}

	T process(T input) {
		T out1 = (input - d2) * a0 + d1;
        d1 = input;
        d2 = out1;
        return out1;
	}

	void setCoefficients(float coeff0) {
		a0 = T(coeff0);
	}

	T d1 = T(0);
	T d2 = T(0);

	T a0 = T(0);

};

template <typename T = float>
struct APPath2 {

	APPath2() {}

	T process(T input) {
		T out1 = (input - d2) * a0 + d1;
        T out2 = (out1 - d3) * a1 + d2;
        d1 = input;
        d2 = out1;
        d3 = out2;
        return out2;
	}

	void setCoefficients(float coeff0, float coeff1) {
		a0 = T(coeff0);
		a1 = T(coeff1);
	}

	T d1 = T(0);
	T d2 = T(0);
	T d3 = T(0);

	T a0 = T(0);
	T a1 = T(0);

};

/** Decimate by a factor of 32 with cascaded half band filters. */
template <int OVERSAMPLE, typename T = float>
struct pow2Decimate {
	
	T in32Buffer[32];

	T in24Buffer[64];
	int in24Index;
	// T from24to8[6] = {0.04774302, 0.167565, 0.28518149, 0.28518149, 0.167565, 0.04774302};
	T from24to8[12] = {T(-0.00534053), T(-0.01919541), T(-0.01717899), T(0.0493416), T(0.18595231), T(0.30651506), T(0.30651506), T(0.18595231), T(0.0493416), T(-0.01717899), T(-0.01919541), T(-0.00534053)};
	
	T in16Buffer[16];
	
	T in8Buffer[8];
	
	T in4Buffer[4];

	T in2Buffer[2];

	APPath2<T> from2to1Path1;
	APPath2<T> from2to1Path2;
	APPath2<T> from4to2Path1;
	APPath2<T> from4to2Path2;
	APPath1<T> from8to4Path1;
	APPath1<T> from8to4Path2;
	APPath1<T> from16to8Path1;
	APPath1<T> from16to8Path2;
	APPath1<T> from32to16Path1;
	APPath1<T> from32to16Path2;

	pow2Decimate() {

		from2to1Path1.setCoefficients(0.0798664262025582, 0.5453236511825826);
		from2to1Path2.setCoefficients(0.283829344898100, 0.834411891201724);

		from4to2Path1.setCoefficients(0.0798664262025582, 0.5453236511825826);
		from4to2Path2.setCoefficients(0.283829344898100, 0.834411891201724);

		from8to4Path1.setCoefficients(0.11192);
		from8to4Path2.setCoefficients(0.53976);

		from16to8Path1.setCoefficients(0.11192);
		from16to8Path2.setCoefficients(0.53976);

		from32to16Path1.setCoefficients(0.11192);
		from32to16Path2.setCoefficients(0.53976);

		reset();

	}

	void reset() {
		in24Index = 0;
		std::memset(in32Buffer, 0, sizeof(in32Buffer));
		std::memset(in24Buffer, 0, sizeof(in24Buffer));
		std::memset(in16Buffer, 0, sizeof(in16Buffer));
		std::memset(in8Buffer, 0, sizeof(in8Buffer));
		std::memset(in4Buffer, 0, sizeof(in4Buffer));
		std::memset(in2Buffer, 0, sizeof(in2Buffer));
	}

	T process(T *in) {
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
	T process32x(T *in) {
		
		// copy in the data
		std::memcpy(&in32Buffer[0], in, 32*sizeof(T));		

		process32to16();
		process16to8();
		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 24 */
	inline T process24x(T *in) {
		
		std::memcpy(&in24Buffer[in24Index], in, 8*sizeof(T));
		in24Index = (in24Index + 8) & 63;

		// copy in the data
		std::memcpy(&in24Buffer[in24Index], &in[8], 8*sizeof(T));
		in24Index = (in24Index + 8) & 63;

		std::memcpy(&in24Buffer[in24Index], &in[16], 8*sizeof(T));
		in24Index = (in24Index + 8) & 63;

		process24to8();
		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 16 */
	T process16x(T *in) {
		
		// copy in the data
		std::memcpy(&in16Buffer[0], in, 16*sizeof(T));		

		process16to8();
		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 8 */
	T process8x(T *in) {
		
		// copy in the data
		std::memcpy(&in8Buffer[0], in, 8*sizeof(T));

		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 4 */
	T process4x(T *in) {
		
		// copy in the data
		std::memcpy(&in4Buffer[0], in, 4*sizeof(T));

		process4to2();
		return process2to1();

	}

	/** `in` must be length 2 */
	T process2x(T *in) {
		
		// copy in the data
		std::memcpy(&in2Buffer[0], in, 2*sizeof(T));

		return process2to1();

	}

	void inline process32to16(void) {

		int writeIndex = 0;

		// filter every other sample and write to the x8 buffer
		for (int i = 0; i < 32; i += 2) {
			
			in16Buffer[writeIndex] = (from32to16Path1.process(in32Buffer[i + 1]) + from32to16Path2.process(in32Buffer[i])) * 0.5;
			writeIndex++;

		}

	}

	void inline process24to8(void) {

		uint32_t workingIndex = (in24Index - 24) & 63;

		int writeIndex = 0;
		
		// filter every third sample and write to the x16 buffer
		for (int i = 0; i < 24; i += 3) {

			T accumulator = from24to8[0] * in24Buffer[workingIndex];
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

			in8Buffer[writeIndex] = accumulator;
			writeIndex++;
			workingIndex += 3;
			workingIndex &= 63;
		}

	}

	void inline process16to8(void) {

		int writeIndex = 0;

		// filter every other sample and write to the x8 buffer
		for (int i = 0; i < 16; i += 2) {
			
			in8Buffer[writeIndex] = (from16to8Path1.process(in16Buffer[i + 1]) + from16to8Path2.process(in16Buffer[i])) * 0.5;;
			writeIndex++;

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


	T inline process2to1(void) {

		return (from2to1Path1.process(in2Buffer[1]) + from2to1Path2.process(in2Buffer[0])) * 0.5;
	}
	
};
