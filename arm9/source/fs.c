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
#include "guiTGDS.h"
#include "nds_cp15_misc.h"

#include <unistd.h>
#include <sys/dir.h>
#include <fcntl.h>
#include "fatfslayerTGDS.h"
#include "xenofunzip.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "conf.h"
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

/* *********************** FAT ************************ */

//#define ENABLE_LOGGING

#ifdef ENABLE_LOGGING
sint8 logbuf[32000];
static int logcnt = 0;
#endif
void	FS_printlogBufOnly(sint8 *buf)
{
	#ifdef ENABLE_LOGGING
	if (logcnt == 0)
	{
		// first call
		strcpy(logbuf, buf);
		logcnt = 1;
		return;
	}	
	if( strlen(buf)+strlen(logbuf) >= 32000)
	{
		strcpy(logbuf, buf);
		logcnt++;
	}
	else
		strcat(logbuf, buf);
	#endif
}

void	FS_printlog(sint8 *buf)
{
	#ifdef ENABLE_LOGGING
	static FILE *f_log = NULL;
	if (logcnt == 0)
	{
		// first call
		strcpy(logbuf, buf);
		logcnt = 1;
		return;
	}	
	if( strlen(buf)+strlen(logbuf) >= 32000)
	{
		// Flush buffer
		sint8 name[30];
		sprintf(name,"snemul%d.log", logcnt%100);
		
		FS_lock();
		
		f_log = fopen(name, "w");
		if(f_log){
			//FILE handle ok
		}
		else{
			//FILE handle error
		}
		fwrite(logbuf, 1, strlen(logbuf), f_log);
		fclose(f_log);
		FS_unlock();
		
		strcpy(logbuf, buf);
		logcnt++;
	}
	else{
		strcat(logbuf, buf);
	}
	#endif
}

//printf that instead stores on Filesystem
void	FS_flog(sint8 *fmt, ...)
{
	#ifdef ENABLE_LOGGING
	char * printfBuf = (char*)&ConsolePrintfBuf[0];
	va_list ap;
    va_start(ap, fmt);
    vsnprintf((sint8*)printfBuf, 100, fmt, ap);
    va_end(ap);
	FS_printlog((sint8*)printfBuf);
	#endif
	#ifndef ENABLE_LOGGING
	return;
	#endif
}

int FS_loadROM(sint8 *ROM, sint8 *filename)
{
	FILE	*f;
	FS_lock();
	f = fopen(filename, "r");
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)ROM, (int)size);
	
	fread(ROM, 1, size, f);
	printf("Read done\n");
	fclose(f);
	FS_unlock();

	return 1;
}


FILE * fPaging ;

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
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)ROM, (int)size);
	
	int ret = fread(ROM, 1, size, fPaging);
	FS_unlock();
	
	if(ret != size){
		return -1;
	}
	
	return 0;
}


int	FS_loadROMPage(sint8 *buf, unsigned int pos, int size){
	int ret;	
	FS_lock();
	
	if(!fPaging){
		return -1;
	}
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)buf, (int)size);
	
	ret = fseek(fPaging, pos, SEEK_SET);
	
	if (ret < 0)
	{
		FS_unlock();
		return -1;
	}
	return fread(buf, 1, size, fPaging);
}

int	FS_shouldFreeROM()
{
	return 1;
}

//This method uses opendir(); and readdir(); to iterate through dir/file contents.
//if DIR is detected, the name is app
sint8	**FS_getDirectoryList(sint8 *path, sint8 *mask, int *cnt){
	int	size = 0;
	*cnt = size;
	
	// add ".." because fatfs removes it
	char * leaveDirDirectory = "..";
	(*cnt)++;
	size += strlen(leaveDirDirectory)+1;
	
	FS_lock();
	DIR *dir = opendir(path);
	if( NULL != dir ){
		while (1){
			struct dirent* pent = readdir(dir);
			if(pent != NULL){
				struct fd * fdinst = getStructFD(pent->d_ino);	//struct stat st is generated at the moment readdir(); is called, so get access to it through fdinst->stat
				if(fdinst){
					if(mask){
						sint8 *ext = _FS_getFileExtension(pent->d_name);
						if ((ext && strstr(mask, ext)) || (fdinst->StructFDType == FT_DIR)){
							//Count files and directories
							if(fdinst->StructFDType == FT_FILE){
								(*cnt)++;
								size += strlen(pent->d_name)+1;
							}
							else if(fdinst->StructFDType == FT_DIR){
								(*cnt)++;
								size += strlen(pent->d_name)+2;	//add trailing "/"
							}
						}
					}
					else{
						//Count files and directories
						if(fdinst->StructFDType == FT_FILE){
							(*cnt)++;
							size += strlen(pent->d_name)+1;
						}
						else if(fdinst->StructFDType == FT_DIR){
							(*cnt)++;
							size += strlen(pent->d_name)+2;	//add trailing "/"
						}
					}
				}
			}
			else{
				break;
			}
		}
	}
	rewinddir(dir);
	
	sint8	**list = (sint8	**)malloc((*cnt)*sizeof(sint8 *)+size);
	sint8	*ptr = ((sint8 *)list) + (*cnt)*sizeof(sint8 *);
	int i = 0; 
	
	// add ".." because fatfs removes it
	strcpy(ptr, leaveDirDirectory);
	list[i++] = ptr;
	ptr += strlen(leaveDirDirectory)+1;
	
	if(NULL != dir){
		while (1){
			struct dirent* pent = readdir(dir);	//if NULL already not a dir
			if(pent != NULL){
				struct fd * fdinst = getStructFD(pent->d_ino);
				if(fdinst){
					if(mask){
						sint8 *ext = _FS_getFileExtension(pent->d_name);
						if ((ext && strstr(mask, ext)) || (fdinst->StructFDType == FT_DIR)){
							if(fdinst->StructFDType == FT_FILE){
								strcpy(ptr, pent->d_name);
								list[i++] = ptr;
								ptr += strlen(pent->d_name)+1;
							}
							else if(fdinst->StructFDType == FT_DIR){
								char dirName[MAX_TGDSFILENAME_LENGTH+1];
								memset(dirName, 0, sizeof(dirName));
								strcpy(dirName, (pent->d_name));
								strcat(dirName, "/");
								
								strcpy(ptr, dirName);
								list[i++] = ptr;
								ptr += strlen(dirName)+1;
							}
						}
					}
					else{
						if(fdinst->StructFDType == FT_FILE){
							strcpy(ptr, pent->d_name);
							list[i++] = ptr;
							ptr += strlen(pent->d_name)+1;
						}
						else if(fdinst->StructFDType == FT_DIR){
							char dirName[MAX_TGDSFILENAME_LENGTH+1];
							memset(dirName, 0, sizeof(dirName));
							strcpy(dirName, (pent->d_name));
							strcat(dirName, "/");
							
							strcpy(ptr, dirName);
							list[i++] = ptr;
							ptr += strlen(dirName)+1;
						}
					}
				}
				
			}
			else{
				break;
			}
		}
	}
	closedir(dir);
	FS_unlock();
	return list;
}
