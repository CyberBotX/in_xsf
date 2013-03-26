/*
	Copyright 2006 yopyop
	Copyright 2007 shash
	Copyright 2007-2011 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FIFO_H
#define FIFO_H

#include "types.h"

//=================================================== IPC FIFO
struct IPC_FIFO
{
	uint32_t buf[16];

	uint8_t head;
	uint8_t tail;
	uint8_t size;
};

extern IPC_FIFO ipc_fifo[2];
extern void IPC_FIFOinit(uint8_t proc);
extern void IPC_FIFOsend(uint8_t proc, uint32_t val);
extern uint32_t IPC_FIFOrecv(uint8_t proc);
extern void IPC_FIFOcnt(uint8_t proc, uint16_t val);

//=================================================== GFX FIFO

//yeah, its oversize for now. thats a simpler solution
//moon seems to overdrive the fifo with immediate dmas
//i think this might be nintendo code too
//static const uint32_t HACK_GXIFO_SIZE = 200000;

/*struct GFX_FIFO
{
	uint8_t cmd[HACK_GXIFO_SIZE];
	uint32_t param[HACK_GXIFO_SIZE];

	uint32_t head; // start position
	uint32_t tail; // tail
	uint32_t size; // size FIFO buffer
};*/

/*struct GFX_PIPE
{
	uint8_t cmd[4];
	uint32_t param[4];

	uint8_t head;
	uint8_t tail;
	uint8_t size;
};*/

//extern GFX_PIPE gxPIPE;
//extern GFX_FIFO gxFIFO;
//extern void GFX_PIPEclear();
//extern void GFX_FIFOclear();
//extern void GFX_FIFOsend(uint8_t cmd, uint32_t param);
//extern bool GFX_PIPErecv(uint8_t *cmd, uint32_t *param);
//extern void GFX_FIFOcnt(uint32_t val);

//=================================================== Display memory FIFO
/*struct DISP_FIFO
{
	uint32_t buf[0x6000]; // 256x192 32K color
	uint32_t head; // head
	uint32_t tail; // tail
};*/

//extern DISP_FIFO disp_fifo;
//extern void DISP_FIFOinit();
//extern void DISP_FIFOsend(uint32_t val);
//extern uint32_t DISP_FIFOrecv();

#endif
