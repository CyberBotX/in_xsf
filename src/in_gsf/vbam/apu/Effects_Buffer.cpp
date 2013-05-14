// Game_Music_Emu $vers. http://www.slack.net/~ant/

#include "Effects_Buffer.h"

#include <cmath>
#include <cstring>

/* Copyright (C) 2006-2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#include "blargg_source.h"

static const int fixed_shift = 12;
template<typename T> static inline Effects_Buffer::fixed_t TO_FIXED(const T &f) { return static_cast<Effects_Buffer::fixed_t>(f * (static_cast<Effects_Buffer::fixed_t>(1) << fixed_shift)); }
static inline Effects_Buffer::fixed_t FROM_FIXED(Effects_Buffer::fixed_t f) { return f >> fixed_shift; }

static const int max_read = 2560; // determines minimum delay

Effects_Buffer::Effects_Buffer(int max_bufs, long echo_size_) : Multi_Buffer(stereo)
{
	this->echo_size = std::max<long>(max_read * stereo, echo_size_ & ~1);
	this->clock_rate_ = 0;
	this->bass_freq_ = 90;
	this->bufs.clear();
	this->bufs_size = 0;
	this->bufs_max = std::max<int>(max_bufs, extra_chans);
	this->no_echo = this->no_effects  = true;

	// defaults
	this->config_.enabled = false;
	this->config_.delay[0] = 120;
	this->config_.delay[1] = 122;
	this->config_.feedback = 0.2f;
	this->config_.treble = 0.4f;

	static const float sep = 0.8f;
	this->config_.side_chans[0].pan = -sep;
	this->config_.side_chans[1].pan = sep;
	this->config_.side_chans[0].vol = this->config_.side_chans[1].vol = 1.0f;

	memset(&this->s, 0, sizeof(this->s));
	this->clear();
}

Effects_Buffer::~Effects_Buffer()
{
	this->delete_bufs();
}

// avoid using new []
blargg_err_t Effects_Buffer::new_bufs(int size)
{
	this->delete_bufs();
	this->bufs.resize(size);
	for (int i = 0; i < size; ++i)
		this->bufs[i].reset(new buf_t);
	this->bufs_size = size;
	return 0;
}

void Effects_Buffer::delete_bufs()
{
	this->bufs.clear();
	this->bufs_size = 0;
}

blargg_err_t Effects_Buffer::set_sample_rate(long rate, int msec)
{
	// extra to allow farther past-the-end pointers
	this->mixer.samples_read = 0;
	this->echo.resize(echo_size + stereo);
	return Multi_Buffer::set_sample_rate(rate, msec);
}

void Effects_Buffer::clock_rate(long rate)
{
	this->clock_rate_ = rate;
	for (int i = this->bufs_size; --i >= 0; )
		this->bufs[i]->clock_rate(this->clock_rate_);
}

void Effects_Buffer::bass_freq(int freq)
{
	this->bass_freq_ = freq;
	for (int i = this->bufs_size; --i >= 0; )
		this->bufs[i]->bass_freq(this->bass_freq_);
}

blargg_err_t Effects_Buffer::set_channel_count(int count, const int *types)
{
	Multi_Buffer::set_channel_count(count, types);

	this->delete_bufs();

	this->mixer.samples_read = 0;

	this->chans.resize(count + extra_chans);

	this->new_bufs(std::min(this->bufs_max, count + extra_chans));

	for (int i = this->bufs_size; --i >= 0; )
		RETURN_ERR(this->bufs[i]->set_sample_rate(this->sample_rate(), this->length()));

	for (int i = this->chans.size(); --i >= 0; )
	{
		auto &ch = this->chans[i];
		ch.cfg.vol = 1.0f;
		ch.cfg.pan = 0.0f;
		ch.cfg.surround = ch.cfg.echo = false;
	}
	// side channels with echo
	this->chans[2].cfg.echo = this->chans[3].cfg.echo = true;

	this->clock_rate(this->clock_rate_);
	this->bass_freq(this->bass_freq_);
	this->apply_config();
	this->clear();

	return 0;
}

void Effects_Buffer::clear_echo()
{
	if (!this->echo.empty())
		memset(&this->echo[0], 0, this->echo.size() * sizeof(echo[0]));
}

void Effects_Buffer::clear()
{
	this->echo_pos = 0;
	this->s.low_pass[0] = this->s.low_pass[1] = 0;
	this->mixer.samples_read = 0;

	for (int i = this->bufs_size; --i >= 0; )
		this->bufs[i]->clear();
	this->clear_echo();
}

auto Effects_Buffer::channel(int i) -> channel_t
{
	i += extra_chans;
	assert(extra_chans <= i && i < static_cast<int>(this->chans.size()));
	return this->chans[i].channel;
}

// Configuration

// 3 wave positions with/without surround, 2 multi (one with same config as wave)
static const int simple_bufs = 3 * 2 + 2 - 1;

Simple_Effects_Buffer::Simple_Effects_Buffer() : Effects_Buffer(extra_chans + simple_bufs, 18 * 1024L)
{
	this->config_.echo = 0.20f;
	this->config_.stereo = 0.20f;
	this->config_.surround = true;
	this->config_.enabled = false;
}

void Simple_Effects_Buffer::apply_config()
{
	auto &c = Effects_Buffer::config();

	c.enabled = this->config_.enabled;
	if (c.enabled)
	{
		c.delay[0] = 120;
		c.delay[1] = 122;
		c.feedback = this->config_.echo * 0.7f;
		c.treble = 0.6f - 0.3f * this->config_.echo;

		float sep = this->config_.stereo + 0.80f;
		if (sep > 1.0f)
			sep = 1.0f;

		c.side_chans[0].pan = -sep;
		c.side_chans[1].pan = sep;

		for (int i = this->channel_count(); --i >= 0; )
		{
			auto &ch = Effects_Buffer::chan_config(i);

			ch.pan = 0.0f;
			ch.surround = this->config_.surround;
			ch.echo = false;

			int type = this->channel_types() ? this->channel_types()[i] : 0;
			if (!(type & noise_type))
			{
				int index = (type & type_index_mask) % 6 - 3;
				if (index < 0)
				{
					index += 3;
					ch.surround = false;
					ch.echo = true;
				}
				if (index >= 1)
				{
					ch.pan = this->config_.stereo;
					if (index == 1)
						ch.pan = -ch.pan;
				}
			}
			else if (type & 1)
				ch.surround = false;
		}
	}

	Effects_Buffer::apply_config();
}

int Effects_Buffer::min_delay() const
{
	assert(this->sample_rate());
	return max_read * 1000L / this->sample_rate();
}

int Effects_Buffer::max_delay() const
{
	assert(this->sample_rate());
	return (this->echo_size / stereo - max_read) * 1000L / this->sample_rate();
}

void Effects_Buffer::apply_config()
{
	if (!this->bufs_size)
		return;

	this->s.treble = TO_FIXED(this->config_.treble);

	bool echo_dirty = false;

	fixed_t old_feedback = this->s.feedback;
	this->s.feedback = TO_FIXED(this->config_.feedback);
	if (!old_feedback && this->s.feedback)
		echo_dirty = true;

	// delays
	int i;
	for (i = stereo; --i >= 0;)
	{
		long delay = this->config_.delay[i] * this->sample_rate() / 1000 * stereo;
		delay = std::max<long>(delay, max_read * stereo);
		delay = std::min<long>(delay, this->echo_size - max_read * stereo);
		if (this->s.delay[i] != delay)
		{
			this->s.delay[i] = delay;
			echo_dirty = true;
		}
	}

	// side channels
	for (i = 2; --i >= 0; )
	{
		this->chans[i + 2].cfg.vol = this->chans[i].cfg.vol = this->config_.side_chans[i].vol * 0.5f;
		this->chans[i + 2].cfg.pan = this->chans[i].cfg.pan = this->config_.side_chans[i].pan;
	}

	// convert volumes
	for (i = this->chans.size(); --i >= 0; )
	{
		auto &ch = this->chans[i];
		ch.vol[0] = TO_FIXED(ch.cfg.vol - ch.cfg.vol * ch.cfg.pan);
		ch.vol[1] = TO_FIXED(ch.cfg.vol + ch.cfg.vol * ch.cfg.pan);
		if (ch.cfg.surround)
			ch.vol[0] = -ch.vol [0];
	}

	this->assign_buffers();

	// set side channels
	for (i = this->chans.size(); --i >= 0; )
	{
		auto &ch = chans[i];
		ch.channel.left = this->chans[ch.cfg.echo * 2].channel.center;
		ch.channel.right = this->chans[ch.cfg.echo * 2 + 1].channel.center;
	}

	bool old_echo = !this->no_echo && !this->no_effects;

	// determine whether effects and echo are needed at all
	this->no_effects = this->no_echo = true;
	for (i = this->chans.size(); --i >= extra_chans; )
	{
		auto &ch = this->chans[i];
		if (ch.cfg.echo && this->s.feedback)
			this->no_echo = false;

		if (ch.vol[0] != TO_FIXED(1) || ch.vol[1] != TO_FIXED(1))
			this->no_effects = false;
	}
	if (!this->no_echo)
		this->no_effects = false;

	if (this->chans[0].vol[0] != TO_FIXED(1) || this->chans[0].vol[1] != TO_FIXED(0) || this->chans[1].vol[0] != TO_FIXED(0) || this->chans[1].vol[1] != TO_FIXED(1))
		this->no_effects = false;

	if (!this->config_.enabled)
		this->no_effects = true;

	if (this->no_effects)
	{
		for (i = this->chans.size(); --i >= 0; )
		{
			auto &ch = this->chans[i];
			ch.channel.center = this->bufs[2].get();
			ch.channel.left = this->bufs[0].get();
			ch.channel.right = this->bufs[1].get();
		}
	}

	this->mixer.bufs[0] = this->bufs[0].get();
	this->mixer.bufs[1] = this->bufs[1].get();
	this->mixer.bufs[2] = this->bufs[2].get();

	if (echo_dirty || (!old_echo && (!this->no_echo && !this->no_effects)))
		this->clear_echo();

	this->channels_changed();
}

void Effects_Buffer::assign_buffers()
{
	// assign channels to buffers
	int buf_count = 0;
	for (int i = 0; i < static_cast<int>(this->chans.size()); ++i)
	{
		// put second two side channels at end to give priority to main channels
		// in case closest matching is necessary
		int x = i;
		if (i > 1)
			x += 2;
		if (x >= static_cast<int>(this->chans.size()))
			x -= this->chans.size() - 2;
		auto &ch = this->chans[x];

		int b = 0;
		for (; b < buf_count; ++b)
		{
			if (ch.vol[0] == this->bufs[b]->vol[0] && ch.vol[1] == this->bufs[b]->vol[1] && (ch.cfg.echo == this->bufs[b]->echo || !this->s.feedback))
				break;
		}

		if (b >= buf_count)
		{
			if (buf_count < this->bufs_max)
			{
				this->bufs[b]->vol[0] = ch.vol[0];
				this->bufs[b]->vol[1] = ch.vol[1];
				this->bufs[b]->echo = ch.cfg.echo;
				++buf_count;
			}
			else
			{
				// TODO: this is a mess, needs refinement
				b = 0;
				fixed_t best_dist = TO_FIXED(8);
				for (int h = buf_count; --h >= 0; )
				{
					auto CALC_LEVELS = [&](fixed_t vols[], fixed_t &sum, fixed_t &diff, bool &surround)
					{
						fixed_t vol_0 = vols[0];
						if (vol_0 < 0)
						{
							vol_0 = -vol_0;
							surround = true;
						}
						fixed_t vol_1 = vols[1];
						if (vol_1 < 0)
						{
							vol_1 = -vol_1;
							surround = true;
						}
						sum = vol_0 + vol_1;
						diff = vol_0 - vol_1;
					};
					fixed_t ch_sum, ch_diff, buf_sum, buf_diff;
					bool ch_surround, buf_surround;
					CALC_LEVELS(ch.vol, ch_sum, ch_diff, ch_surround);
					CALC_LEVELS(this->bufs[h]->vol, buf_sum, buf_diff, buf_surround);

					fixed_t dist = std::abs(ch_sum - buf_sum) + std::abs(ch_diff - buf_diff);

					if (ch_surround != buf_surround)
						dist += TO_FIXED(1) / 2;

					if (this->s.feedback && ch.cfg.echo != this->bufs[h]->echo)
						dist += TO_FIXED(1) / 2;

					if (best_dist > dist)
					{
						best_dist = dist;
						b = h;
					}
				}
			}
		}

		ch.channel.center = this->bufs[b].get();
	}
}

// Mixing

void Effects_Buffer::end_frame(blip_time_t time)
{
	for (int i = bufs_size; --i >= 0; )
		this->bufs[i]->end_frame(time);
}

long Effects_Buffer::read_samples(blip_sample_t *out, long out_size)
{
	out_size = std::min(out_size, this->samples_avail());

	int pair_count = static_cast<int>(out_size >> 1);
	assert(pair_count * stereo == out_size); // must read an even number of samples
	if (pair_count)
	{
		if (this->no_effects)
			this->mixer.read_pairs(out, pair_count);
		else
		{
			int pairs_remain = pair_count;
			do
			{
				// mix at most max_read pairs at a time
				int count = max_read;
				if (count > pairs_remain)
					count = pairs_remain;

				if (this->no_echo)
				{
					// optimization: clear echo here to keep mix_effects() a leaf function
					this->echo_pos = 0;
					memset(&this->echo[0], 0, count * stereo * sizeof(this->echo[0]));
				}
				this->mix_effects(out, count);

				int32_t new_echo_pos = this->echo_pos + count * stereo;
				if (new_echo_pos >= this->echo_size)
					new_echo_pos -= this->echo_size;
				this->echo_pos = new_echo_pos;
				assert(this->echo_pos < this->echo_size);

				out += count * stereo;
				this->mixer.samples_read += count;
				pairs_remain -= count;
			} while (pairs_remain);
		}

		if (this->samples_avail() <= 0 || this->immediate_removal())
		{
			for (int i = this->bufs_size; --i >= 0; )
			{
				auto &b = this->bufs[i];
				// TODO: might miss non-silence settling since it checks END of last read
				if (b->non_silent())
					b->remove_samples(this->mixer.samples_read);
				else
					b->remove_silence(this->mixer.samples_read);
			}
			this->mixer.samples_read = 0;
		}
	}
	return out_size;
}

void Effects_Buffer::mix_effects(blip_sample_t *out_, int pair_count)
{
	typedef fixed_t stereo_fixed_t[stereo];

	// add channels with echo, do echo, add channels without echo, then convert to 16-bit and output
	int echo_phase = 1;
	do
	{
		// mix any modified buffers
		{
			size_t bufNum = 0;
			int bufs_remain = this->bufs_size;
			do
			{
				auto &buf = this->bufs[bufNum++];
				if (buf->non_silent() && (buf->echo == !!echo_phase))
				{
					auto out = reinterpret_cast<stereo_fixed_t *>(&this->echo[this->echo_pos]);
					int bass = BLIP_READER_BASS(*buf);
					BLIP_READER_BEGIN(in, *buf);
					BLIP_READER_ADJ_(in, this->mixer.samples_read);
					fixed_t vol_0 = buf->vol[0];
					fixed_t vol_1 = buf->vol[1];

					int count = static_cast<unsigned>(echo_size - echo_pos) / stereo;
					int remain = pair_count;
					if (count > remain)
						count = remain;
					do
					{
						remain -= count;
						BLIP_READER_ADJ_(in, count);

						out += count;
						int offset = -count;
						do
						{
							fixed_t s = BLIP_READER_READ(in);
							BLIP_READER_NEXT_IDX_(in, bass, offset);

							out[offset][0] += s * vol_0;
							out[offset][1] += s * vol_1;
						} while ( ++offset );

						out = reinterpret_cast<stereo_fixed_t *>(&this->echo[0]);
						count = remain;
					} while (remain);

					BLIP_READER_END(in, *buf);
				}
			} while (--bufs_remain);
		}

		// add echo
		if (echo_phase && !this->no_echo)
		{
			fixed_t feedback = this->s.feedback;
			fixed_t treble = this->s.treble;

			int i = 1;
			do
			{
				fixed_t low_pass = this->s.low_pass[i];

				auto echo_end = &this->echo[this->echo_size + i];
				auto in_pos = &this->echo[this->echo_pos + i];
				int32_t out_offset = this->echo_pos + i + this->s.delay[i];
				if (out_offset >= this->echo_size)
					out_offset -= this->echo_size;
				assert(out_offset < this->echo_size);
				auto out_pos = &this->echo[out_offset];

				// break into up to three chunks to avoid having to handle wrap-around
				// in middle of core loop
				int remain = pair_count;
				do
				{
					auto pos = in_pos;
					if (pos < out_pos)
						pos = out_pos;
					int count = static_cast<uint32_t>(reinterpret_cast<char *>(echo_end) - reinterpret_cast<const char *>(pos)) / (stereo * sizeof(fixed_t));
					if (count > remain)
						count = remain;
					remain -= count;

					in_pos += count * stereo;
					out_pos += count * stereo;
					int offset = -count;
					do
					{
						low_pass += FROM_FIXED(in_pos[offset * stereo] - low_pass) * treble;
						out_pos[offset * stereo] = FROM_FIXED(low_pass) * feedback;
					} while (++offset);

					if (in_pos >= echo_end)
						in_pos -= echo_size;
					if (out_pos >= echo_end)
						out_pos -= echo_size;
				} while (remain);

				this->s.low_pass [i] = low_pass;
			} while (--i >= 0);
		}
	} while (--echo_phase >= 0);

	// clamp to 16 bits
	auto in = reinterpret_cast<stereo_fixed_t *>(&this->echo[this->echo_pos]);
	typedef blip_sample_t stereo_blip_sample_t[stereo];
	auto out = reinterpret_cast<stereo_blip_sample_t *>(out_);
	int count = static_cast<unsigned>(this->echo_size - this->echo_pos) / stereo;
	int remain = pair_count;
	if (count > remain)
		count = remain;
	do
	{
		remain -= count;
		in  += count;
		out += count;
		int offset = -count;
		do
		{
			fixed_t in_0 = FROM_FIXED(in[offset][0]);
			fixed_t in_1 = FROM_FIXED(in[offset][1]);

			BLIP_CLAMP(in_0, in_0);
			out[offset][0] = static_cast<blip_sample_t>(in_0);

			BLIP_CLAMP(in_1, in_1);
			out[offset][1] = static_cast<blip_sample_t>(in_1);
		} while (++offset);

		in = reinterpret_cast<stereo_fixed_t *>(&this->echo[0]);
		count = remain;
	} while (remain);
}
