/*
 * xSF - Core Player
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-30
 *
 * Partially based on the vio*sf framework
 */

#include <cstring>
#include "XSFPlayer.h"
#include "XSFConfig.h"
#include "XSFCommon.h"

extern XSFConfig *xSFConfig;

XSFPlayer::XSFPlayer() : xSF(nullptr), sampleRate(0), detectedSilenceSample(0), detectedSilenceSec(0), skipSilenceOnStartSec(5), lengthSample(0), fadeSample(0), currentSample(0),
	prevSampleL(XSFPlayer::CHECK_SILENCE_BIAS), prevSampleR(XSFPlayer::CHECK_SILENCE_BIAS), lengthInMS(-1), fadeInMS(-1), volume(1.0), ignoreVolume(false)
{
}

XSFPlayer::XSFPlayer(const XSFPlayer &xSFPlayer) : xSF(new XSFFile()), sampleRate(xSFPlayer.sampleRate), detectedSilenceSample(xSFPlayer.detectedSilenceSample), detectedSilenceSec(xSFPlayer.detectedSilenceSec),
	skipSilenceOnStartSec(xSFPlayer.skipSilenceOnStartSec), lengthSample(xSFPlayer.lengthSample), fadeSample(xSFPlayer.fadeSample), currentSample(xSFPlayer.currentSample), prevSampleL(xSFPlayer.prevSampleL),
	prevSampleR(xSFPlayer.prevSampleR), lengthInMS(xSFPlayer.lengthInMS), fadeInMS(xSFPlayer.fadeInMS), volume(xSFPlayer.volume), ignoreVolume(xSFPlayer.ignoreVolume)
{
	*this->xSF = *xSFPlayer.xSF;
}

XSFPlayer &XSFPlayer::operator=(const XSFPlayer &xSFPlayer)
{
	if (this != &xSFPlayer)
	{
		if (!this->xSF)
			this->xSF = new XSFFile();
		*this->xSF = *xSFPlayer.xSF;
		this->sampleRate = xSFPlayer.sampleRate;
		this->detectedSilenceSample = xSFPlayer.detectedSilenceSample;
		this->detectedSilenceSec = xSFPlayer.detectedSilenceSec;
		this->skipSilenceOnStartSec = xSFPlayer.skipSilenceOnStartSec;
		this->lengthSample = xSFPlayer.lengthSample;
		this->fadeSample = xSFPlayer.fadeSample;
		this->currentSample = xSFPlayer.currentSample;
		this->prevSampleL = xSFPlayer.prevSampleL;
		this->prevSampleR = xSFPlayer.prevSampleR;
		this->lengthInMS = xSFPlayer.lengthInMS;
		this->fadeInMS = xSFPlayer.fadeInMS;
		this->volume = xSFPlayer.volume;
		this->ignoreVolume = xSFPlayer.ignoreVolume;
	}
	return *this;
}

bool XSFPlayer::FillBuffer(std::vector<uint8_t> &buf, unsigned &samplesWritten)
{
	bool endFlag = false;
	unsigned detectSilence = xSFConfig->GetDetectSilenceSec();
	unsigned pos = 0, bufsize = buf.size() >> 2;
	while (pos < bufsize)
	{
		unsigned remain = bufsize - pos, offset = pos;
		this->GenerateSamples(buf, pos << 1, remain);
		if (detectSilence || skipSilenceOnStartSec)
		{
			unsigned skipOffset = 0;
			for (unsigned ofs = 0; ofs < remain; ++ofs)
			{
				short *bufShort = reinterpret_cast<short *>(&buf[0]);
				unsigned long sampleL = bufShort[2 * (offset + ofs)], sampleR = bufShort[2 * (offset + ofs) + 1];
				bool silence = (sampleL + XSFPlayer::CHECK_SILENCE_BIAS + XSFPlayer::CHECK_SILENCE_LEVEL) - this->prevSampleL <= XSFPlayer::CHECK_SILENCE_LEVEL * 2 &&
					(sampleR + XSFPlayer::CHECK_SILENCE_BIAS + XSFPlayer::CHECK_SILENCE_LEVEL) - this->prevSampleR <= XSFPlayer::CHECK_SILENCE_LEVEL * 2;

				if (silence)
				{
					if (++this->detectedSilenceSample >= this->sampleRate)
					{
						this->detectedSilenceSample -= this->sampleRate;
						++this->detectedSilenceSec;
						if (this->skipSilenceOnStartSec && this->detectedSilenceSec >= this->skipSilenceOnStartSec)
						{
							this->skipSilenceOnStartSec = this->detectedSilenceSec = 0;
							if (ofs)
								skipOffset = ofs;
						}
					}
				}
				else
				{
					this->detectedSilenceSample = this->detectedSilenceSec = 0;
					if (this->skipSilenceOnStartSec)
					{
						this->skipSilenceOnStartSec = 0;
						if (ofs)
							skipOffset = ofs;
					}
				}

				prevSampleL = sampleL + XSFPlayer::CHECK_SILENCE_BIAS;
				prevSampleR = sampleR + XSFPlayer::CHECK_SILENCE_BIAS;
			}

			if (!this->skipSilenceOnStartSec)
			{
				if (skipOffset)
				{
					std::vector<uint8_t> tmpBuf = std::vector<uint8_t>(2 * skipOffset);
					memcpy(&tmpBuf[0], &buf[offset + 2 * skipOffset], 2 * skipOffset);
					memcpy(&buf[offset], &tmpBuf[0], 2 * skipOffset);
					pos += skipOffset;
				}
				else
					pos += remain;
			}
		}
		else
			pos += remain;
	}

	/* Detect end of song */
	if (!xSFConfig->GetPlayInfinitely())
	{
		if (this->currentSample >= this->lengthSample + this->fadeSample)
		{
			samplesWritten = 0;
			return true;
		}
		if (this->currentSample + bufsize >= this->lengthSample + this->fadeSample)
		{
			bufsize = this->lengthSample + this->fadeSample - this->currentSample;
			endFlag = true;
		}
	}

	/* Volume */
	if (!this->ignoreVolume && (!fEqual(this->volume, 1.0) || !fEqual(xSFConfig->GetVolume(), 1.0)))
	{
		double scale = this->volume * xSFConfig->GetVolume();
		short *bufShort = reinterpret_cast<short *>(&buf[0]);
		for (unsigned ofs = 0; ofs < bufsize; ++ofs)
		{
			double s1 = bufShort[2 * ofs] * scale, s2 = bufShort[2 * ofs + 1] * scale;
			if (s1 > 0x7FFF)
				s1 = 0x7FFF;
			else if (s1 < -0x8000)
				s1 = -0x8000;
			if (s2 > 0x7FFF)
				s2 = 0x7FFF;
			else if (s2 < -0x8000)
				s2 = -0x8000;
			bufShort[2 * ofs] = static_cast<short>(s1);
			bufShort[2 * ofs + 1] = static_cast<short>(s2);
		}
	}

	/* Fading */
	if (!xSFConfig->GetPlayInfinitely() && this->fadeSample && this->currentSample + bufsize >= this->lengthSample)
	{
		short *bufShort = reinterpret_cast<short *>(&buf[0]);
		for (unsigned ofs = 0; ofs < bufsize; ++ofs)
		{
			if (this->currentSample + ofs >= this->lengthSample && this->currentSample + ofs < this->lengthSample + this->fadeSample)
			{
				int scale = static_cast<uint64_t>(this->lengthSample + this->fadeSample - (this->currentSample + ofs)) * 0x10000 / this->fadeSample;
				bufShort[2 * ofs] = (bufShort[2 * ofs] * scale) >> 16;
				bufShort[2 * ofs + 1] = (bufShort[2 * ofs + 1] * scale) >> 16;
			}
			else if (this->currentSample + ofs >= this->lengthSample + this->fadeSample)
				bufShort[2 * ofs] = bufShort[2 * ofs + 1] = 0;
		}
	}

	this->currentSample += bufsize;
	samplesWritten = bufsize;
	return endFlag;
}

bool XSFPlayer::Load()
{
	this->lengthInMS = this->xSF->GetLengthMS(xSFConfig->GetDefaultLength());
	this->fadeInMS = this->xSF->GetFadeMS(xSFConfig->GetDefaultFade());
	this->lengthSample = static_cast<uint64_t>(this->lengthInMS) * this->sampleRate / 1000;
	this->fadeSample = static_cast<uint64_t>(this->fadeInMS) * this->sampleRate / 1000;
	this->volume = this->xSF->GetVolume(xSFConfig->GetVolumeType(), xSFConfig->GetPeakType());
	return true;
}

void XSFPlayer::SeekTop()
{
	this->skipSilenceOnStartSec = xSFConfig->GetSkipSilenceOnStartSec();
	this->currentSample = this->detectedSilenceSec = this->detectedSilenceSample = 0;
	this->prevSampleL = this->prevSampleR = XSFPlayer::CHECK_SILENCE_BIAS;
}

#ifdef WINAMP_PLUGIN

static inline DWORD TicksDiff(DWORD prev, DWORD cur) { return cur >= prev ? cur - prev : 0xFFFFFFFF - prev + cur; }

int XSFPlayer::Seek(unsigned seekPosition, volatile int *killswitch, std::vector<uint8_t> &buf, Out_Module *outMod)
{
	unsigned bufsize = buf.size() >> 2, seekSample = static_cast<uint64_t>(seekPosition) * this->sampleRate / 1000;
	DWORD prevTick = outMod ? GetTickCount() : 0;
	if (seekSample < this->currentSample)
	{
		this->Terminate();
		this->Load();
		this->SeekTop();
	}
	while (seekSample - this->currentSample > bufsize)
	{
		if (killswitch && *killswitch)
			return 1;
		if (outMod)
		{
			DWORD curTick = GetTickCount();
			if (TicksDiff(prevTick, curTick) >= 500)
			{
				prevTick = curTick;
				unsigned cur = static_cast<uint64_t>(this->currentSample) * 1000 / this->sampleRate;
				outMod->Flush(cur);
			}
		}
		this->GenerateSamples(buf, 0, bufsize);
		this->currentSample += bufsize;
	}
	if (seekSample - this->currentSample > 0)
	{
		this->GenerateSamples(buf, 0, seekSample - this->currentSample);
		this->currentSample = seekSample;
	}
	if (outMod)
		outMod->Flush(seekPosition);
	return 0;
}

#endif
