/*
 * xSF - 2SF Player
 * By Naram Qashat (CyberBotX) [cyberbotx@cyberbotx.com]
 * Last modification on 2013-03-25
 *
 * Based on a modified vio2sf v0.22c
 *
 * Partially based on the vio*sf framework
 *
 * Utilizes a modified DeSmuME v0.9.8 for playback
 * http://desmume.org/
 */

#include <memory>
#include <zlib.h>
#include "convert.h"
#include "XSFPlayer.h"
#include "XSFCommon.h"
#include "desmume/armcpu.h"
#include "desmume/saves.h"
#include "desmume/NDSSystem.h"
#include "desmume/cp15.h"

class XSFPlayer_2SF : public XSFPlayer
{
public:
	XSFPlayer_2SF(const std::string &filename);
	XSFPlayer_2SF(const std::wstring &filename);
	~XSFPlayer_2SF() { this->Terminate(); }
	bool Load();
	void GenerateSamples(std::vector<uint8_t> &buf, unsigned offset, unsigned samples);
	void Terminate();
};

const char *XSFPlayer::WinampDescription = "2SF Decoder";
const char *XSFPlayer::WinampExts = "2sf;mini2sf\0DS Sound Format files (*.2sf;*.mini2sf)\0";

XSFPlayer *XSFPlayer::Create(const std::string &fn)
{
	return new XSFPlayer_2SF(fn);
}

XSFPlayer *XSFPlayer::Create(const std::wstring &fn)
{
	return new XSFPlayer_2SF(fn);
}

volatile bool execute = false;

static struct
{
	std::vector<uint8_t> rom, state;
	unsigned stateptr;
} loaderwork;

static inline uint16_t Get16BitsLE(const uint8_t *input)
{
	return input[0] | (input[1] << 8);
}

static void load_getstateinit(unsigned ptr)
{
	loaderwork.stateptr = ptr;
}

static void load_getsta(Status_Reg *ptr, unsigned l)
{
	unsigned s = l << 2;
	if (loaderwork.stateptr > loaderwork.state.size() || loaderwork.stateptr + s > loaderwork.state.size())
		return;
	if (ptr)
		for (unsigned i = 0; i < l; ++i)
		{
			uint32_t st = Get32BitsLE(&loaderwork.state[loaderwork.stateptr + (i << 2)]);
			ptr[i].bits.N = (st >> 31) & 1;
			ptr[i].bits.Z = (st >> 30) & 1;
			ptr[i].bits.C = (st >> 29) & 1;
			ptr[i].bits.V = (st >> 28) & 1;
			ptr[i].bits.Q = (st >> 27) & 1;
			ptr[i].bits.RAZ = (st >> 8) & ((1 << 19) - 1);
			ptr[i].bits.I = (st >> 7) & 1;
			ptr[i].bits.F = (st >> 6) & 1;
			ptr[i].bits.T = (st >> 5) & 1;
			ptr[i].bits.mode = (st >> 0) & 0x1f;
		}
	loaderwork.stateptr += s;
}

static void load_getbool(bool *ptr, unsigned l)
{
	unsigned s = l << 2;
	if (loaderwork.stateptr > loaderwork.state.size() || loaderwork.stateptr + s > loaderwork.state.size())
		return;
	if (ptr)
		for (unsigned i = 0; i < l; ++i)
			ptr[i] = !!Get32BitsLE(&loaderwork.state[loaderwork.stateptr + (i << 2)]);
	loaderwork.stateptr += s;
}

#ifdef SIGNED_IS_NOT_2S_COMPLEMENT
/* 2's complement */
static inline int32_t u32tos32(uint32_t v) { return static_cast<int32_t>((static_cast<int64_t>(v) ^ 0x8000) - 0x8000); }
#else
/* 2's complement */
static inline int32_t u32tos32(uint32_t v) { return static_cast<int32_t>(v); }
#endif

static void load_gets32(int32_t *ptr, unsigned l)
{
	unsigned s = l << 2;
	if (loaderwork.stateptr > loaderwork.state.size() || loaderwork.stateptr + s > loaderwork.state.size())
		return;
	if (ptr)
		for (unsigned i = 0; i < l; ++i)
			ptr[i] = u32tos32(Get32BitsLE(&loaderwork.state[loaderwork.stateptr + (i << 2)]));
	loaderwork.stateptr += s;
}

static void load_getu32(uint32_t *ptr, unsigned l)
{
	unsigned s = l << 2;
	if (loaderwork.stateptr > loaderwork.state.size() || loaderwork.stateptr + s > loaderwork.state.size())
		return;
	if (ptr)
		for (unsigned i = 0; i < l; ++i)
			ptr[i] = Get32BitsLE(&loaderwork.state[loaderwork.stateptr + (i << 2)]);
	loaderwork.stateptr += s;
}

static void load_getu16(uint16_t *ptr, unsigned l)
{
	unsigned s = l << 1;
	if (loaderwork.stateptr > loaderwork.state.size() || loaderwork.stateptr + s > loaderwork.state.size())
		return;
	if (ptr)
		for (unsigned i = 0; i < l; ++i)
			ptr[i] = Get16BitsLE(&loaderwork.state[loaderwork.stateptr + (i << 1)]);
	loaderwork.stateptr += s;
}

static void load_getu8(uint8_t *ptr, unsigned l)
{
	unsigned s = l;
	if (loaderwork.stateptr > loaderwork.state.size() || loaderwork.stateptr + s > loaderwork.state.size())
		return;
	if (ptr)
		for (unsigned i = 0; i < l; ++i)
			ptr[i] = loaderwork.state[loaderwork.stateptr + i];
	loaderwork.stateptr += s;
}

/*static void gdb_stub_fix(armcpu_t *armcpu)
{*/
	/* armcpu->R[15] = armcpu->instruct_adr; */
	/*armcpu->next_instruction = armcpu->instruct_adr;
	if(armcpu->CPSR.bits.T == 0)
	{
		armcpu->instruction = MMU_read32(armcpu->proc_ID, armcpu->next_instruction);
		armcpu->instruct_adr = armcpu->next_instruction;
		armcpu->next_instruction += 4;
		armcpu->R[15] = armcpu->next_instruction + 4;
	}
	else
	{
		armcpu->instruction = MMU_read16(armcpu->proc_ID, armcpu->next_instruction);
		armcpu->instruct_adr = armcpu->next_instruction;
		armcpu->next_instruction += 2;
		armcpu->R[15] = armcpu->next_instruction + 2;
	}
}*/

static void load_setstate()
{
	if (loaderwork.state.empty())
		return;

	//std::vector<uint8_t> savestate = std::vector<uint8_t>(&loaderwork.state[0], &loaderwork.state[loaderwork.state.size()]);
	EMUFILE_MEMORY f(&loaderwork.state);
	if (!savestate_load(&f))
	{
	//reset the emulator first to clean out the host's state

		//while the series of resets below should work,
		//we are testing the robustness of the savestate system with this full reset.
		//the full reset wipes more things, so we can make sure that they are being restored correctly
		//extern bool _HACK_DONT_STOPMOVIE;
		//_HACK_DONT_STOPMOVIE = true;
		NDS_Reset();
		//_HACK_DONT_STOPMOVIE = false;

		//reset some options to their old defaults which werent saved
		//nds.debugConsole = false;

		/* Skip over "Desmume Save File" crap */
		load_getstateinit(0x17);

		/* Read ARM7 cpu registers */
		//load_getu32(&NDS_ARM7.proc_ID, 1);
		load_getu32(NULL, 1);
		load_getu32(&NDS_ARM7.instruction, 1);
		load_getu32(&NDS_ARM7.instruct_adr, 1);
		load_getu32(&NDS_ARM7.next_instruction, 1);
		load_getu32(NDS_ARM7.R, 16);
		load_getsta(&NDS_ARM7.CPSR, 1);
		load_getsta(&NDS_ARM7.SPSR, 1);
		load_getu32(&NDS_ARM7.R13_usr, 1);
		load_getu32(&NDS_ARM7.R14_usr, 1);
		load_getu32(&NDS_ARM7.R13_svc, 1);
		load_getu32(&NDS_ARM7.R14_svc, 1);
		load_getu32(&NDS_ARM7.R13_abt, 1);
		load_getu32(&NDS_ARM7.R14_abt, 1);
		load_getu32(&NDS_ARM7.R13_und, 1);
		load_getu32(&NDS_ARM7.R14_und, 1);
		load_getu32(&NDS_ARM7.R13_irq, 1);
		load_getu32(&NDS_ARM7.R14_irq, 1);
		load_getu32(&NDS_ARM7.R8_fiq, 1);
		load_getu32(&NDS_ARM7.R9_fiq, 1);
		load_getu32(&NDS_ARM7.R10_fiq, 1);
		load_getu32(&NDS_ARM7.R11_fiq, 1);
		load_getu32(&NDS_ARM7.R12_fiq, 1);
		load_getu32(&NDS_ARM7.R13_fiq, 1);
		load_getu32(&NDS_ARM7.R14_fiq, 1);
		load_getsta(&NDS_ARM7.SPSR_svc, 1);
		load_getsta(&NDS_ARM7.SPSR_abt, 1);
		load_getsta(&NDS_ARM7.SPSR_und, 1);
		load_getsta(&NDS_ARM7.SPSR_irq, 1);
		load_getsta(&NDS_ARM7.SPSR_fiq, 1);
		load_getu32(&NDS_ARM7.intVector, 1);
		load_getu8(&NDS_ARM7.LDTBit, 1);
		load_getbool(&NDS_ARM7.waitIRQ, 1);
		bool tmpbool;
		//load_getbool(&NDS_ARM7.wIRQ, 1);
		//loaderwork.stateptr += 4;
		load_getbool(NULL, 1);
		//load_getbool(&NDS_ARM7.halt_IE_and_IF, 1);
		//load_getbool(&NDS_ARM7.wirq, 1);
		//loaderwork.stateptr += 4;
		//load_getbool(NULL, 1);
		//load_getbool(&NDS_ARM7.intrWaitARM_state, 1);
		load_getbool(&tmpbool, 1);
		//NDS_ARM7.intrWaitARM_state = tmpbool;

		/* Read ARM9 cpu registers */
		//load_getu32(&NDS_ARM9.proc_ID, 1);
		load_getu32(NULL, 1);
		load_getu32(&NDS_ARM9.instruction, 1);
		load_getu32(&NDS_ARM9.instruct_adr, 1);
		load_getu32(&NDS_ARM9.next_instruction, 1);
		load_getu32(NDS_ARM9.R, 16);
		load_getsta(&NDS_ARM9.CPSR, 1);
		load_getsta(&NDS_ARM9.SPSR, 1);
		load_getu32(&NDS_ARM9.R13_usr, 1);
		load_getu32(&NDS_ARM9.R14_usr, 1);
		load_getu32(&NDS_ARM9.R13_svc, 1);
		load_getu32(&NDS_ARM9.R14_svc, 1);
		load_getu32(&NDS_ARM9.R13_abt, 1);
		load_getu32(&NDS_ARM9.R14_abt, 1);
		load_getu32(&NDS_ARM9.R13_und, 1);
		load_getu32(&NDS_ARM9.R14_und, 1);
		load_getu32(&NDS_ARM9.R13_irq, 1);
		load_getu32(&NDS_ARM9.R14_irq, 1);
		load_getu32(&NDS_ARM9.R8_fiq, 1);
		load_getu32(&NDS_ARM9.R9_fiq, 1);
		load_getu32(&NDS_ARM9.R10_fiq, 1);
		load_getu32(&NDS_ARM9.R11_fiq, 1);
		load_getu32(&NDS_ARM9.R12_fiq, 1);
		load_getu32(&NDS_ARM9.R13_fiq, 1);
		load_getu32(&NDS_ARM9.R14_fiq, 1);
		load_getsta(&NDS_ARM9.SPSR_svc, 1);
		load_getsta(&NDS_ARM9.SPSR_abt, 1);
		load_getsta(&NDS_ARM9.SPSR_und, 1);
		load_getsta(&NDS_ARM9.SPSR_irq, 1);
		load_getsta(&NDS_ARM9.SPSR_fiq, 1);
		load_getu32(&NDS_ARM9.intVector, 1);
		load_getu8(&NDS_ARM9.LDTBit, 1);
		load_getbool(&NDS_ARM9.waitIRQ, 1);
		//load_getbool(&NDS_ARM9.wIRQ, 1);
		//loaderwork.stateptr += 4;
		load_getbool(NULL, 1);
		//load_getbool(&NDS_ARM9.halt_IE_and_IF, 1);
		//load_getbool(&NDS_ARM9.wirq, 1);
		//loaderwork.stateptr += 4;
		//load_getbool(NULL, 1);
		//load_getbool(&NDS_ARM9.intrWaitARM_state, 1);
		load_getbool(&tmpbool, 1);
		//NDS_ARM9.intrWaitARM_state = tmpbool;

		/* Read in other internal variables that are important */
		//int32_t tmps32;
		//load_gets32(&nds.ARM9Cycle, 1);
		load_gets32(NULL, 1);
		//load_gets32(&nds.ARM7Cycle, 1);
		load_gets32(NULL, 1);
		load_gets32(&nds.cycles, 1);
		int32_t tmps32_array[4];
		//load_getu64(nds.timerCycle[0], 4);
		load_gets32(tmps32_array, 4);
		for (int i = 0; i < 4; ++i)
			nds.timerCycle[0][i] = tmps32_array[i];
		//load_getu64(nds.timerCycle[1], 4);
		load_gets32(tmps32_array, 4);
		for (int i = 0; i < 4; ++i)
			nds.timerCycle[1][i] = tmps32_array[i];
		//bool tmpbool_array[4];
		//load_getbool(nds.timerOver[0], 4);
		//loaderwork.stateptr += 16;
		load_getbool(NULL, 4);
		//load_getbool(nds.timerOver[1], 4);
		//loaderwork.stateptr += 16;
		load_getbool(NULL, 4);
		//load_gets32(&nds.nextHBlank, 1);
		//loaderwork.stateptr += 4;
		load_gets32(NULL, 1);
		load_getu32(&nds.VCount, 1);
		load_getu32(&nds.old, 1);
		//load_gets32(&nds.diff, 1);
		//loaderwork.stateptr += 4;
		load_gets32(NULL, 1);
		//load_getbool(&nds.lignerendu, 1);
		//loaderwork.stateptr += 4;
		load_getbool(NULL, 1);
		load_getu16(NULL, 1);
		load_getu16(NULL, 1);

		/* Read in memory/registers specific to the ARM9 */
		//load_getu8 (MMU.ARM9_ITCM, 0x8000);
		load_getu8(NULL, 0x8000);
		load_getu8 (MMU.ARM9_DTCM, 0x4000);
		//load_getu8 (MMU.MAIN_MEM, 0x1000000);
		//load_getu8(MMU.MAIN_MEM, 0x800000);
		//load_getu8(NULL, 0x800000);
		//load_getu8(NULL, 0x1000000);
		load_getu8(NULL, 0xFF8000);
		load_getu8(MMU.ARM9_ITCM, 0x8000);
		load_getu8 (MMU.MAIN_MEM/*+0x400000*/, 0x400000);
		load_getu8 (MMU.ARM9_REG, 0x10000);
		load_getu8 (MMU.ARM9_VMEM, 0x800);
		load_getu8 (MMU.ARM9_OAM, 0x800);
		//uint8_t tmpu8_ABG[0x80000];
		//load_getu8 (ARM9Mem.ARM9_ABG, 0x80000);
		//loaderwork.stateptr += 0x80000;
		load_getu8(NULL, 0x80000);
		//uint8_t tmpu8_BBG[0x20000];
		//load_getu8 (ARM9Mem.ARM9_BBG, 0x20000);
		//loaderwork.stateptr += 0x20000;
		load_getu8(NULL, 0x20000);
		//uint8_t tmpu8_AOBJ[0x40000];
		//load_getu8 (ARM9Mem.ARM9_AOBJ, 0x40000);
		//loaderwork.stateptr += 0x40000;
		load_getu8(NULL, 0x40000);
		//uint8_t tmpu8_BOBJ[0x20000];
		//load_getu8 (ARM9Mem.ARM9_BOBJ, 0x20000);
		//loaderwork.stateptr += 0x20000;
		load_getu8(NULL, 0x20000);
		load_getu8 (MMU.ARM9_LCD, 0xA4000);
		//loaderwork.stateptr += 12;

		/* Read in memory/registers specific to the ARM7 */
		load_getu8 (MMU.ARM7_ERAM, 0x10000);
		load_getu8 (MMU.ARM7_REG, 0x10000);
		load_getu8 (MMU.ARM7_WIRAM, 0x10000);

		/* Read in shared memory */
		load_getu8 (MMU.SWIRAM, 0x8000);

		//gdb_stub_fix(&NDS_ARM9);
		//gdb_stub_fix(&NDS_ARM7);

		loadstate();
	}
}

static struct
{
	std::vector<uint8_t> buf;
	unsigned filled, used;
	uint32_t bufferbytes, cycles;
	int xfs_load, sync_type;
} sndifwork = {std::vector<uint8_t>(), 0, 0, 0, 0, 0, 0};

static void SNDIFDeInit() { }

static int SNDIFInit(int buffersize)
{
	uint32_t bufferbytes = buffersize * sizeof(int16_t);
	SNDIFDeInit();
	sndifwork.buf.resize(bufferbytes + 3);
	sndifwork.bufferbytes = bufferbytes;
	sndifwork.filled = sndifwork.used = 0;
	sndifwork.cycles = 0;
	return 0;
}

static void SNDIFMuteAudio() { }
static void SNDIFUnMuteAudio() { }
static void SNDIFSetVolume(int) { }

static uint32_t SNDIFGetAudioSpace()
{
	return sndifwork.bufferbytes >> 2; // bytes to samples
}

static void SNDIFUpdateAudio(int16_t *buffer, uint32_t num_samples)
{
	uint32_t num_bytes = num_samples << 2;
	if (num_bytes > sndifwork.bufferbytes)
		num_bytes = sndifwork.bufferbytes;
	memcpy(&sndifwork.buf[0], buffer, num_bytes);
	sndifwork.filled = num_bytes;
	sndifwork.used = 0;
}

static const int SNDIFID_2SF = 1;
static SoundInterface_struct SNDIF_2SF =
{
	SNDIFID_2SF,
	"2sf Sound Interface",
	SNDIFInit,
	SNDIFDeInit,
	SNDIFUpdateAudio,
	SNDIFGetAudioSpace,
	SNDIFMuteAudio,
	SNDIFUnMuteAudio,
	SNDIFSetVolume,
	NULL
};

SoundInterface_struct *SNDCoreList[] =
{
	&SNDIF_2SF,
	&SNDDummy,
	NULL
};

static void Map2SFSection(const std::vector<uint8_t> &section, bool isSave)
{
	auto &data = isSave ? loaderwork.state : loaderwork.rom;

	uint32_t offset = Get32BitsLE(&section[0]), size = Get32BitsLE(&section[4]), finalSize = size + offset;
	if (!isSave)
		finalSize = NextHighestPowerOf2(finalSize);
	if (data.empty())
		data.resize(finalSize + 10, 0);
	else if (data.size() < size + offset)
		data.resize(offset + finalSize + 10);
	memcpy(&data[offset], &section[8], size);
}

static bool Map2SF(XSFFile *xSF)
{
	if (!xSF->IsValidType(0x24))
		return false;

	auto &reservedSection = xSF->GetReservedSection(), &programSection = xSF->GetProgramSection();

	if (!reservedSection.empty())
	{
		size_t reservedPosition = 0, reservedSize = reservedSection.size();
		while (reservedPosition + 12 < reservedSize)
		{
			uint32_t saveSize = Get32BitsLE(&reservedSection[reservedPosition + 4]);
			if (Get32BitsLE(&reservedSection[reservedPosition]) == 0x45564153) // "SAVE"
			{
				if (reservedPosition + 12 + saveSize > reservedSize)
					return false;

				uint8_t tmpSaveUncompressed[8];
				unsigned long saveUncompressedSize = 8;
				uncompress(tmpSaveUncompressed, &saveUncompressedSize, &reservedSection[reservedPosition + 12], saveSize);
				saveUncompressedSize = Get32BitsLE(&tmpSaveUncompressed[4]) + 8;
				auto saveUncompressed = std::vector<uint8_t>(saveUncompressedSize);
				uncompress(&saveUncompressed[0], &saveUncompressedSize, &reservedSection[reservedPosition + 12], saveSize);

				Map2SFSection(saveUncompressed, true);
			}
			reservedPosition += saveSize + 12;
		}
	}

	if (!programSection.empty())
		Map2SFSection(programSection, false);

	return true;
}

static bool RecursiveLoad2SF(XSFFile *xSF, int level)
{
	if (level <= 10 && xSF->GetTagExists("_lib"))
	{
#ifdef _WIN32
		auto libxSF = std::auto_ptr<XSFFile>(new XSFFile(ExtractDirectoryFromPath(xSF->GetFilename().GetWStr()) + xSF->GetTagValue("_lib").GetWStr(), 4, 8));
#else
		auto libxSF = std::auto_ptr<XSFFile>(new XSFFile(ExtractDirectoryFromPath(xSF->GetFilename().GetAnsi()) + xSF->GetTagValue("_lib").GetAnsi(), 4, 8));
#endif
		if (!RecursiveLoad2SF(libxSF.get(), level + 1))
			return false;
	}

	if (!Map2SF(xSF))
		return false;

	unsigned n = 2;
	bool found;
	do
	{
		found = false;
		std::string libTag = "_lib" + stringify(n++);
		if (xSF->GetTagExists(libTag))
		{
			found = true;
#ifdef _WIN32
			auto libxSF = std::auto_ptr<XSFFile>(new XSFFile(ExtractDirectoryFromPath(xSF->GetFilename().GetWStr()) + xSF->GetTagValue(libTag).GetWStr(), 4, 8));
#else
			auto libxSF = std::auto_ptr<XSFFile>(new XSFFile(ExtractDirectoryFromPath(xSF->GetFilename().GetAnsi()) + xSF->GetTagValue(libTag).GetAnsi(), 4, 8));
#endif
			if (!RecursiveLoad2SF(libxSF.get(), level + 1))
				return false;
		}
	} while (found);

	return true;
}

static bool Load2SF(XSFFile *xSF)
{
	loaderwork.rom.clear();
	loaderwork.state.clear();
	loaderwork.stateptr = 0;

	return RecursiveLoad2SF(xSF, 1);
}

XSFPlayer_2SF::XSFPlayer_2SF(const std::string &filename) : XSFPlayer()
{
	this->xSF = new XSFFile(filename, 4, 8);
}

XSFPlayer_2SF::XSFPlayer_2SF(const std::wstring &filename) : XSFPlayer()
{
	this->xSF = new XSFFile(filename, 4, 8);
}

bool XSFPlayer_2SF::Load()
{
	int frames = xSF->GetTagValue("_frames", -1);
	sndifwork.sync_type = xSF->GetTagValue("_2sf_sync_type", 0);

	sndifwork.xfs_load = false;
	if (!Load2SF(this->xSF))
		return false;

	if (NDS_Init())
		return false;

	SPU_ChangeSoundCore(SNDIFID_2SF, 737);

	execute = false;

	MMU_unsetRom();
	if (!loaderwork.rom.empty())
	{
		NDS_SetROM(&loaderwork.rom[0], loaderwork.rom.size() - 1);
		gameInfo.loadData(reinterpret_cast<char *>(&loaderwork.rom[0]), loaderwork.rom.size() - 1);
	}

	NDS_Reset();

	execute = true;

	if (!loaderwork.state.empty())
	{
		armcp15_t *c9 = reinterpret_cast<armcp15_t *>(NDS_ARM9.coproc[15]);
		//int proc;
		if (frames == -1)
		{
			/* set initial ARM9 coprocessor state */

			armcp15_moveARM2CP(c9, 0x00000000, 0x01, 0x00, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000000, 0x07, 0x05, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000000, 0x07, 0x06, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000000, 0x07, 0x0a, 0, 4);
			armcp15_moveARM2CP(c9, 0x04000033, 0x06, 0x00, 0, 4);
			armcp15_moveARM2CP(c9, 0x0200002d, 0x06, 0x01, 0, 0);
			armcp15_moveARM2CP(c9, 0x027e0021, 0x06, 0x02, 0, 0);
			armcp15_moveARM2CP(c9, 0x08000035, 0x06, 0x03, 0, 0);
			armcp15_moveARM2CP(c9, 0x027e001b, 0x06, 0x04, 0, 0);
			armcp15_moveARM2CP(c9, 0x0100002f, 0x06, 0x05, 0, 0);
			armcp15_moveARM2CP(c9, 0xffff001d, 0x06, 0x06, 0, 0);
			armcp15_moveARM2CP(c9, 0x027ff017, 0x06, 0x07, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000020, 0x09, 0x01, 0, 1);

			armcp15_moveARM2CP(c9, 0x027e000a, 0x09, 0x01, 0, 0);

			armcp15_moveARM2CP(c9, 0x00000042, 0x02, 0x00, 0, 1);
			armcp15_moveARM2CP(c9, 0x00000042, 0x02, 0x00, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000002, 0x03, 0x00, 0, 0);
			armcp15_moveARM2CP(c9, 0x05100011, 0x05, 0x00, 0, 3);
			armcp15_moveARM2CP(c9, 0x15111011, 0x05, 0x00, 0, 2);
			armcp15_moveARM2CP(c9, 0x07dd1e10, 0x01, 0x00, 0, 0);
			armcp15_moveARM2CP(c9, 0x0005707d, 0x01, 0x00, 0, 0);

			armcp15_moveARM2CP(c9, 0x00000000, 0x07, 0x0a, 0, 4);
			armcp15_moveARM2CP(c9, 0x02004000, 0x07, 0x05, 0, 1);
			armcp15_moveARM2CP(c9, 0x02004000, 0x07, 0x0e, 0, 1);

			/* set initial timer state */

			/*MMU_write16(0, REG_TM0CNTL, 0x0000);
			MMU_write16(0, REG_TM0CNTH, 0x00C1);
			MMU_write16(1, REG_TM0CNTL, 0x0000);
			MMU_write16(1, REG_TM0CNTH, 0x00C1);
			MMU_write16(1, REG_TM1CNTL, 0xf7e7);
			MMU_write16(1, REG_TM1CNTH, 0x00C1);*/

			/* set initial interrupt state */

			MMU.reg_IME[0] = 0x00000001;
			MMU.reg_IE[0]  = 0x00042001;
			MMU.reg_IME[1] = 0x00000001;
			MMU.reg_IE[1]  = 0x0104009d;
		}
		else if (frames > 0)
		{
			/* execute boot code */
			for (int i = 0; i < frames; ++i)
				NDS_exec<true>();
		}

		/* load state */

		execute = false;
		SPU_Reset();

		load_setstate();
		loaderwork.state.clear();

		if (frames == -1)
			armcp15_moveARM2CP(c9, (NDS_ARM9.R13_irq & 0x0fff0000) | 0x0a, 0x09, 0x01, 0, 0);

		/* restore timer state */

		/*for (proc = 0; proc < 2; proc++)
		{
			MMU_write16(proc, REG_TM0CNTH, T1ReadWord(MMU.MMU_MEM[proc][0x40], REG_TM0CNTH & 0xFFF));
			MMU_write16(proc, REG_TM1CNTH, T1ReadWord(MMU.MMU_MEM[proc][0x40], REG_TM1CNTH & 0xFFF));
			MMU_write16(proc, REG_TM2CNTH, T1ReadWord(MMU.MMU_MEM[proc][0x40], REG_TM2CNTH & 0xFFF));
			MMU_write16(proc, REG_TM3CNTH, T1ReadWord(MMU.MMU_MEM[proc][0x40], REG_TM3CNTH & 0xFFF));
		}*/
	}
	else if (frames > 0)
	{
		/* skip 1 sec */
		for (int i = 0; i < frames; ++i)
			NDS_exec<false>();
	}
	execute = true;
	sndifwork.xfs_load = true;
	//CommonSettings.spu_advanced = true;
	//CommonSettings.advanced_timing = false;

	return XSFPlayer::Load();
}

void XSFPlayer_2SF::GenerateSamples(std::vector<uint8_t> &buf, unsigned offset, unsigned samples)
{
	static const double HBASE_CYCLES = 33509300.322234;
	static const int HLINE_CYCLES = 6 * (99 + 256);
	static const uint32_t HSAMPLES = static_cast<uint32_t>(static_cast<double>(this->sampleRate * HLINE_CYCLES) / HBASE_CYCLES);
	static const int VDIVISION = 100;
	static const int VLINES = 263;
	static const double VBASE_CYCLES = HBASE_CYCLES / VDIVISION;
	static const uint32_t VSAMPLES = static_cast<uint32_t>(static_cast<double>(this->sampleRate * HLINE_CYCLES * VLINES) / HBASE_CYCLES);

	if (!sndifwork.xfs_load)
		return;
	unsigned bytes = samples << 2;
	while (bytes)
	{
		unsigned remainbytes = sndifwork.filled - sndifwork.used;
		if (remainbytes > 0)
		{
			if (remainbytes > bytes)
			{
				memcpy(&buf[offset], &sndifwork.buf[sndifwork.used], bytes);
				sndifwork.used += bytes;
				offset += bytes;
				remainbytes -= bytes;
				bytes = 0;
				break;
			}
			else
			{
				memcpy(&buf[offset], &sndifwork.buf[sndifwork.used], remainbytes);
				sndifwork.used += remainbytes;
				offset += remainbytes;
				bytes -= remainbytes;
				remainbytes = 0;
			}
		}
		if (!remainbytes)
		{
			if (sndifwork.sync_type == 1)
			{
				/* vsync */
				sndifwork.cycles += (this->sampleRate / VDIVISION) * HLINE_CYCLES * VLINES;
				if (sndifwork.cycles >= static_cast<uint32_t>(VBASE_CYCLES * (VSAMPLES + 1)))
					sndifwork.cycles -= static_cast<uint32_t>(VBASE_CYCLES * (VSAMPLES + 1));
				else
					sndifwork.cycles -= static_cast<uint32_t>(VBASE_CYCLES * VSAMPLES);
			}
			else
			{
				/* hsync */
				sndifwork.cycles += this->sampleRate * HLINE_CYCLES;
				if (sndifwork.cycles >= static_cast<uint32_t>(HBASE_CYCLES * (HSAMPLES + 1)))
					sndifwork.cycles -= static_cast<uint32_t>(HBASE_CYCLES * (HSAMPLES + 1));
				else
					sndifwork.cycles -= static_cast<uint32_t>(HBASE_CYCLES * HSAMPLES);
			}
			NDS_exec<false>();
			SPU_Emulate_user();
		}
	}
}

void XSFPlayer_2SF::Terminate()
{
	MMU_unsetRom();
	NDS_DeInit();

	loaderwork.rom.clear();
	loaderwork.state.clear();
	loaderwork.stateptr = 0;
}
