/*
 * SSEQ Player - Constants/Macros
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 *
 * Adapted from source code of FeOS Sound System
 * By fincs
 * https://github.com/fincs/FSS
 *
 * Some constants/macros also taken from libdns, part of the devkitARM portion of devkitPro
 * http://devkitpro.org/
 */

#pragma once

#include <cstdint>

inline constexpr std::uint32_t ARM7_CLOCK = 33513982;
inline constexpr double SecondsPerClockCycle = 64.0 * 2728.0 / ARM7_CLOCK;

inline std::uint32_t BIT(std::uint32_t n) { return 1 << n; }

enum class TrackState
{
	AllocateBit,
	NoteWait,
	PortamentoBit,
	TieBit,
	End,
	Bits
};

enum class TrackUpdateFlag
{
	Volume,
	Pan,
	Timer,
	Modulation,
	Length,
	Bits
};

enum class ChannelState
{
	None,
	Start,
	Attack,
	Decay,
	Sustain,
	Release
};

enum class ChannelFlag
{
	UpdateVolume,
	UpdatePan,
	UpdateTimer,
	Bits
};

enum class ChannelAllocateType
{
	PCM,
	PSG,
	Noise
};

inline constexpr int FSS_TRACKCOUNT = 16;
inline constexpr int FSS_MAXTRACKS = 32;
inline constexpr int FSS_TRACKSTACKSIZE = 3;
inline constexpr int AMPL_K = 723;
inline constexpr int AMPL_MIN = -AMPL_K;
inline constexpr int AMPL_THRESHOLD = AMPL_MIN * 128;

inline int SOUND_FREQ(int n) { return -0x1000000 / n; }

inline std::uint32_t SOUND_VOL(int n) { return n; }
inline std::uint32_t SOUND_VOLDIV(int n) { return n << 8; }
inline std::uint32_t SOUND_PAN(int n) { return n << 16; }
inline std::uint32_t SOUND_DUTY(int n) { return n << 24; }
inline const std::uint32_t SOUND_REPEAT = BIT(27);
inline const std::uint32_t SOUND_ONE_SHOT = BIT(28);
inline std::uint32_t SOUND_LOOP(bool a) { return a ? SOUND_REPEAT : SOUND_ONE_SHOT; }
inline constexpr std::uint32_t SOUND_FORMAT_PSG = 3 << 29;
inline std::uint32_t SOUND_FORMAT(int n) { return n << 29; }
inline const std::uint32_t SCHANNEL_ENABLE = BIT(31);

enum class Interpolation
{
	None,
	Linear,
	FourPointLegrange,
	SixPointLegrange,
	Sinc
};
