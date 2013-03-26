/*
	Copyright (C) 2006 yopyop
	Copyright (C) 2006-2010 DeSmuME team

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

#ifndef __CP15_H__
#define __CP15_H__

#include "armcpu.h"

struct armcp15_t
{
	uint32_t IDCode;
	uint32_t cacheType;
	uint32_t TCMSize;
	uint32_t ctrl;
	uint32_t DCConfig;
	uint32_t ICConfig;
	uint32_t writeBuffCtrl;
	uint32_t und;
	uint32_t DaccessPerm;
	uint32_t IaccessPerm;
	uint32_t protectBaseSize0;
	uint32_t protectBaseSize1;
	uint32_t protectBaseSize2;
	uint32_t protectBaseSize3;
	uint32_t protectBaseSize4;
	uint32_t protectBaseSize5;
	uint32_t protectBaseSize6;
	uint32_t protectBaseSize7;
	uint32_t cacheOp;
	uint32_t DcacheLock;
	uint32_t IcacheLock;
	uint32_t ITCMRegion;
	uint32_t DTCMRegion;
	uint32_t processID;
	uint32_t RAM_TAG;
	uint32_t testState;
	uint32_t cacheDbg;
	/* calculated bitmasks for the regions to decide rights uppon */
	/* calculation is done in the MCR instead of on mem access for performance */
	uint32_t regionWriteMask_USR[8];
	uint32_t regionWriteMask_SYS[8];
	uint32_t regionReadMask_USR[8];
	uint32_t regionReadMask_SYS[8];
	uint32_t regionExecuteMask_USR[8];
	uint32_t regionExecuteMask_SYS[8];
	uint32_t regionWriteSet_USR[8];
	uint32_t regionWriteSet_SYS[8];
	uint32_t regionReadSet_USR[8];
	uint32_t regionReadSet_SYS[8];
	uint32_t regionExecuteSet_USR[8];
	uint32_t regionExecuteSet_SYS[8];

	armcpu_t *cpu;
};

armcp15_t *armcp15_new(armcpu_t *c);
//bool armcp15_dataProcess(armcp15_t *armcp15, uint8_t CRd, uint8_t CRn, uint8_t CRm, uint8_t opcode1, uint8_t opcode2);
//bool armcp15_load(armcp15_t *armcp15, uint8_t CRd, uint8_t adr);
//bool armcp15_store(armcp15_t *armcp15, uint8_t CRd, uint8_t adr);
bool armcp15_moveCP2ARM(armcp15_t *armcp15, uint32_t * R, uint8_t CRn, uint8_t CRm, uint8_t opcode1, uint8_t opcode2);
bool armcp15_moveARM2CP(armcp15_t *armcp15, uint32_t val, uint8_t CRn, uint8_t CRm, uint8_t opcode1, uint8_t opcode2);
//bool armcp15_isAccessAllowed(armcp15_t *armcp15,uint32_t address,uint32_t access);

static const uint32_t CP15_ACCESS_WRITE = 0;
static const uint32_t CP15_ACCESS_READ = 2;
static const uint32_t CP15_ACCESS_EXECUTE = 4;
static const uint32_t CP15_ACCESS_WRITEUSR = CP15_ACCESS_WRITE;
static const uint32_t CP15_ACCESS_WRITESYS = 1;
static const uint32_t CP15_ACCESS_READUSR = CP15_ACCESS_READ;
static const uint32_t CP15_ACCESS_READSYS = 3;
static const uint32_t CP15_ACCESS_EXECUSR = CP15_ACCESS_EXECUTE;
static const uint32_t CP15_ACCESS_EXECSYS = 5;

#endif /* __CP15_H__*/
