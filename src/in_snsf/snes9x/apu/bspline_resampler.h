/* Simple resampler based on bsnes's ruby audio library */

#ifndef __BSPLINE_RESAMPLER_H
#define __BSPLINE_RESAMPLER_H

#include <cmath>
#include "resampler.h"

#undef CLAMP
#undef SHORT_CLAMP
template<typename T1, typename T2> static inline T1 CLAMP(T1 x, T2 low, T2 high) { return x > high ? high : (x < low ? low : x); }
template<typename T> static inline short SHORT_CLAMP(T n) { return static_cast<short>(CLAMP(n, -32768, 32767)); }

class BsplineResampler : public Resampler
{
protected:
	double r_step;
	double r_frac;
	int r_left[4], r_right[4];

	double bspline(double x, double a, double b, double c, double d)
	{
		double ym1py1 = a + c;
		double c0 = 1 / 6.0 * ym1py1 + 2 / 3.0 * b;
		double c1 = 0.5 * (c - a);
		double c2 = 0.5 * ym1py1 - b;
		double c3 = 0.5 * (b - c) + 1 / 6.0 * (d - a);
		return ((c3 * x + c2) * x + c1) * x + c0;
	}

public:
	BsplineResampler(int num_samples) : Resampler(num_samples)
	{
		this->clear();
	}

	void time_ratio(double ratio)
	{
		this->r_step = ratio;
		this->clear();
	}

	void clear()
	{
		ring_buffer::clear ();
		this->r_frac = 1.0;
		this->r_left[0] = this->r_left[1] = this->r_left[2] = this->r_left[3] = 0;
		this->r_right[0] = this->r_right[1] = this->r_right[2] = this->r_right[3] = 0;
	}

	void read(short *data, int num_samples)
	{
		int i_position = this->start >> 1;
		short *internal_buffer = reinterpret_cast<short *>(this->buffer);
		int o_position = 0;
		int consumed = 0;

		while (o_position < num_samples && consumed < this->buffer_size)
		{
			int s_left = internal_buffer[i_position];
			int s_right = internal_buffer[i_position + 1];
			int max_samples = this->buffer_size >> 1;
			const double margin_of_error = 1.0e-10;

			if (std::abs(this->r_step - 1.0) < margin_of_error)
			{
				data[o_position] = static_cast<short>(s_left);
				data[o_position + 1] = static_cast<short>(s_right);

				o_position += 2;
				i_position += 2;
				if (i_position >= max_samples)
					i_position -= max_samples;
				consumed += 2;

				continue;
			}

			while (this->r_frac <= 1.0 && o_position < num_samples)
			{
				data[o_position] = SHORT_CLAMP(bspline(this->r_frac, this->r_left[0], this->r_left[1], this->r_left[2], this->r_left[3]));
				data[o_position + 1] = SHORT_CLAMP(bspline(this->r_frac, this->r_right[0], this->r_right[1], this->r_right[2], this->r_right[3]));

				o_position += 2;

				this->r_frac += this->r_step;
			}

			if (this->r_frac > 1.0)
			{
				this->r_left[0] = this->r_left[1];
				this->r_left[1] = this->r_left[2];
				this->r_left[2] = this->r_left[3];
				this->r_left[3] = s_left;

				this->r_right[0] = this->r_right[1];
				this->r_right[1] = this->r_right[2];
				this->r_right[2] = this->r_right[3];
				this->r_right[3] = s_right;

				this->r_frac -= 1.0;

				i_position += 2;
				if (i_position >= max_samples)
					i_position -= max_samples;
				consumed += 2;
			}
		}

		this->size -= consumed << 1;
		this->start += consumed << 1;
		if (this->start >= this->buffer_size)
			this->start -= this->buffer_size;
	}

	inline int avail()
	{
		return static_cast<int>(std::floor(((this->size >> 2) - this->r_frac) / this->r_step) * 2);
	}
};

#endif /* __BSPLINE_RESAMPLER_H */
