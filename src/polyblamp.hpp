#pragma once
#include <dsp/common.hpp>

template <typename T = float>
struct PolyBlampGenerator {
	T buf[4] = {};
	int pos = 0;

	PolyBlampGenerator() {
		buf[0] = T(0);
		buf[1] = T(0);
		buf[2] = T(0);
		buf[3] = T(0);
	}

	/** Places a discontinuity at -1 < p <= 0 relative to the current frame */
	void insertDiscontinuity(float p, T x) {
		if (!(-1 < p && p <= 0))
			return;

		float d = -p;
		float d_2 = d * d;
		float d_3 = d_2 * d;
		float d_4 = d_3 * d;
		float d_5 = d_4 * d;

		int index = (pos + 0) % 4;
		buf[index] += (d_5 / 120.f) * x;

		index = (pos + 1) % 4;
		buf[index] += (-d_5 / 40.f + d_4 / 24.0f + d_3 / 12.0f + d_2 / 12.0f + d / 24.0f + 1.0f / 120.0f) * x;

		index = (pos + 2) % 4;
		buf[index] += (d_5 / 40.f - d_4 / 12.0f + d_2 / 3.0f - d / 2.0f + 7.0f / 30.0f) * x;
		
		index = (pos + 3) % 4;
		buf[index] += (-d_5 / 120.f + d_4 / 24.0f - d_3 / 12.0f + d_2 / 12.0f - d / 24.0f + 1.0f / 120.0f) * x;

	}

	T process() {
		T v = buf[pos];
		buf[pos] = T(0);
		pos = (pos + 1) % (4);
		return v;
	}
};