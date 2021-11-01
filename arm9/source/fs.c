/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006-2007 archeide, All rights reserved. */
/***********************************************************/
/*
This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License as 
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include "ipcfifoTGDSUser.h"
#include "devoptab_devices.h"
#include "posixHandleTGDS.h"
#include "utilsTGDS.h"

#include "fs.h"
#include "consoleTGDS.h"
#include "nds_cp15_misc.h"

#include <unistd.h>
#include <sys/dir.h>
#include <fcntl.h>
#include "fatfslayerTGDS.h"

#include "gfx.h"
#include "cfg.h"
#include "apu.h"
//#include "ram.h"
#include "conf.h"
//#include "frontend.h"
#include "main.h"
#include "ppu.h"
#include "InterruptsARMCores_h.h"
#include "ff.h"
#include "reent.h"
#include "sys/types.h"
#include "engine.h"
#include "core.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"
#include "consoleTGDS.h"
#include "about.h"
#include "fileBrowse.h"	//generic template functions from TGDS: maintain 1 source, whose changes are globally accepted by all TGDS Projects.

/* *********************** FAT ************************ */

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int	FS_loadROM(sint8 *ROM, sint8 *filename)
{
	FILE	*f;
	FS_lock();
	f = fopen(filename, "r");
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	fread(ROM, 1, size, f);
	GUI_printf("Read done: %s:%d", filename, size);
	fclose(f);
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)ROM, (int)size);
	
	FS_unlock();

	return 1;
}


FILE * fPaging ;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int	FS_loadROMForPaging(sint8 *ROM, sint8 *filename, int size)
{
	FS_lock();
	
	if (fPaging){
		fclose(fPaging);
	}
	fPaging = fopen(filename, "r");
	sint32 fd = fileno(fPaging);
	if(fd < 0){
		FS_unlock();
		return -1;
	}
	
	int ret = fread(ROM, 1, size, fPaging);
	FS_unlock();
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)ROM, (int)size);
	
	if(ret != size){
		return -1;
	}
	
	return 0;
}


#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int	FS_loadROMPage(sint8 *buf, unsigned int pos, int size){
	
	int ret;	
	FS_lock();
	
	if(!fPaging){
		return -1;
	}
	
	ret = fseek(fPaging, pos, SEEK_SET);
	
	if (ret < 0)
	{
		FS_unlock();
		return -1;
	}
	
	
	ret = fread(buf, 1, size, fPaging);
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)buf, (int)size);
	
	return ret;
}

int	FS_shouldFreeROM()
{
	return 1;
}
