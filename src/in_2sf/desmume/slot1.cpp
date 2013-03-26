/*
	Copyright (C) 2010-2011 DeSmuME team

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

//#include <string>

#include "slot1.h"

//extern SLOT1INTERFACE slot1None;
extern SLOT1INTERFACE slot1Retail;
//extern SLOT1INTERFACE slot1R4;

SLOT1INTERFACE slot1List[NDS_SLOT1_COUNT] = {
		//slot1None,
		slot1Retail,
		//slot1R4
};

SLOT1INTERFACE	slot1_device = slot1Retail;			//default for frontends that dont even configure this
//uint8_t				slot1_device_type = NDS_SLOT1_RETAIL;

/*bool slot1Init()
{
	return slot1_device.init();
}*/

/*void slot1Close()
{
	slot1_device.close();
}*/

/*void slot1Reset()
{
	slot1_device.reset();
}*/

/*bool slot1Change(NDS_SLOT1_TYPE changeToType)
{
	printf("slot1Change to: %d\n", changeToType);
	if (changeToType > NDS_SLOT1_COUNT || changeToType < 0) return false;
	slot1_device.close();
	slot1_device_type = changeToType;
	slot1_device = slot1List[slot1_device_type];
	return slot1_device.init();
}*/
