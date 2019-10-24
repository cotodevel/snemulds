/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/


//Coto: add support title + compile version for easier debugging.

#include "about.h"

#include "utilsTGDS.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "cfg.h"
#include "gfx.h"
#include "core.h"
#include "apu.h"
#include "ppu.h"
#include "main.h"
#include "conf.h"
#include "fs.h"
#include "memmap.h"
#include "guiTGDS.h"
#include "opcodes.h"
#include "common.h"
#include "ipcfifoTGDSUser.h"

sint8*  SNEMULDS_TITLE[] = {
	"-= SNEmulDS",
	"by archeide =- "
};

sint8*  SNEMULDS_SUBTITLE[] = {
	"CPU: bubble2k Sound: gladius"
};

//fullpath	/rompath	/gamedir	/dirgame	/dirpath
sint8*  READ_GAME_DIR[] = {
	"snes"
};

volatile char versionBuf[0x100];
sint8 * RetTitleCompiledVersion(){
	sprintf((char*)&versionBuf[0],"%s %s %s",(sint8*)SNEMULDS_TITLE[0],(sint8*)"TGDS1.5",(sint8*)SNEMULDS_TITLE[1]);
	return (char*)&versionBuf[0];
}
