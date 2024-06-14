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

#include "ff.h"
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
#include "fileBrowse.h"	//generic template functions from TGDS: maintain 1 source, whose changes are globally accepted by all TGDS Projects.
#include "snemulds_memmap.h"
/* *********************** FAT ************************ */

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int	FS_loadROM(sint8 *ROM, sint8 *filename){
	FS_lock();
	f_close(&fPagingFD);
	int flags = charPosixToFlagPosix("r");
	BYTE mode = posixToFatfsAttrib(flags);
	FRESULT result = f_open(&fPagingFD, (const TCHAR*)filename, mode);
	if(result != FR_OK){
		FS_unlock();
		GUI_printf("FS_loadROM:epic fail :%s", filename);
		while(1==1){}
		return -1;
	}
	
	//Prevent Cache problems.
	f_lseek (
			&fPagingFD,   /* Pointer to the file object structure */
			(DWORD)0       /* File offset in unit of byte */
		);
	int size = f_size(&fPagingFD);
	int readSize;
	result = f_read(&fPagingFD, ROM, (int)size, (UINT*)&readSize);
	coherent_user_range_by_size((uint32)ROM, (int)size); //Prevent Cache problems.
	GUI_printf("Read done: %d bytes ", readSize);
	f_close(&fPagingFD);

	FS_unlock();
	return 1;
}


FIL fPagingFD;
int fPagingFDInternal = -1;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int	FS_loadROMForPaging(sint8 *ROM, sint8 *filename, int size)
{
	FS_lock();
	f_close(&fPagingFD);
	
	//Set up ROM paging initial state
	mem_init_paging();
	
	int flags = charPosixToFlagPosix("r");
	BYTE mode = posixToFatfsAttrib(flags);
	FRESULT result = f_open(&fPagingFD, (const TCHAR*)filename, mode);
	
	if(result != FR_OK){
		FS_unlock();
		GUI_printf("FS_loadROMForPaging:epic fail :%s", filename);
		while(1==1){}
		return -1;
	}
	
	//Prevent Cache problems.
	f_lseek (
			&fPagingFD,   /* Pointer to the file object structure */
			(DWORD)0       /* File offset in unit of byte */
		);

	int ret;
	result = f_read(&fPagingFD, ROM, (int)size, (UINT*)&ret);
	if(ret != size){
		return -1;
	}

	FS_unlock();
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)ROM, (int)size);
	return 0;
}


#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int	FS_loadROMPage(sint8 *buf, unsigned int pos, int size){
	FRESULT ret;	
	FS_lock();
	
	ret = f_lseek (
			&fPagingFD,   /* Pointer to the file object structure */
			(DWORD)pos       /* File offset in unit of byte */
		);

	int readSize;
	ret = f_read(&fPagingFD, (u8*)buf, size, (UINT*)&readSize);

	if (ret != FR_OK)
	{
		FS_unlock();
		return -1;
	}

	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)buf, (int)size);
	
	return readSize;
}

int	FS_shouldFreeROM()
{
	return 1;
}

//Reimplemented these so they're using fatfs directly and save RAM. 
//Note: FATFS barely works because SnemulDS has run out of memory. So snemul.cfg will likely corrupt.
int	FS_loadFileFatFS(sint8 *filename, sint8 *buf, int size){
	FS_lock();
	FIL thisFD;
	f_close(&thisFD);
	int flags = charPosixToFlagPosix("r");
	BYTE mode = posixToFatfsAttrib(flags);
	FRESULT result = f_open(&thisFD, (const TCHAR*)filename, mode);
	if(result != FR_OK){
		FS_unlock();
		return -1;
	}
	
	//Prevent Cache problems.
	f_lseek (
			&thisFD,   /* Pointer to the file object structure */
			(DWORD)0       /* File offset in unit of byte */
		);
	int readSize;
	result = f_read(&thisFD, buf, (int)size, (UINT*)&readSize);
	coherent_user_range_by_size((uint32)buf, (int)size); //Prevent Cache problems.
	f_close(&thisFD);

	FS_unlock();
	return 1;
}

int	FS_saveFileFatFS(sint8 *filename, sint8 *buf, int size,bool force_file_creation){
	FIL thisFD;
	f_close(&thisFD);
	char var[16];
	if(force_file_creation == true){
		sprintf((sint8*)var,"%s","w+");
	}
	else{
		sprintf((sint8*)var,"%s","w");
	}
	int flags = charPosixToFlagPosix(var);
	BYTE mode = posixToFatfsAttrib(flags);
	FRESULT result = f_open(&thisFD, (const TCHAR*)filename, mode);
	if(result != FR_OK){
		FS_unlock();
		return -1;
	}
	
	//Prevent Cache problems.
	f_lseek (
			&thisFD,   /* Pointer to the file object structure */
			(DWORD)0       /* File offset in unit of byte */
		);
	
	coherent_user_range_by_size((uint32)buf, (int)size); //Prevent Cache problems.
	int writtenSize;
	result = f_write(&thisFD, buf, (int)size, (UINT*)&writtenSize);
	//GUI_printf("Written: %d bytes ", readSize);
	f_close(&thisFD);

	if (result != FR_OK)
	{
		FS_unlock();
		return -1;
	}
	return 0;
}

int	FS_getFileSizeFatFS(sint8 *filename){
	FIL thisFD;
	f_close(&thisFD);
	int flags = charPosixToFlagPosix("r");
	BYTE mode = posixToFatfsAttrib(flags);
	FRESULT result = f_open(&thisFD, (const TCHAR*)filename, mode);
	if(result != FR_OK){
		FS_unlock();
		return -1;
	}
	int size = f_size(&thisFD);
	f_close(&thisFD);
	return size;
}