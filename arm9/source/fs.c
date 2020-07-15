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
#include "xenofunzip.h"
#include "fileBrowse.h"	//generic template functions from TGDS: maintain 1 source, whose changes are globally accepted by all TGDS Projects.

/* *********************** FAT ************************ */

//This method uses opendir(); and readdir(); to iterate through dir/file contents.
//if DIR is detected, the name is app
sint8	**FS_getDirectoryList(sint8 *path, sint8 *mask, int *cnt){
	int	size = 0;
	*cnt = size;
	
	// add ".." because fatfs removes it
	char * leaveDirDirectory = "..";
	char * baseDirectory = "/";
	int pathLen = strlen(path);
	
	//remove leading "0:" 
	if (
		(path[0] == '0')
		&&
		(path[1] == ':')
	){
		char savePath[256+1];
		memset(savePath, 0, sizeof(savePath));
		strncpy(savePath, (char*)&path[2], pathLen - 2);
		strcpy(path, savePath);
	}
	
	//If directory is not root, add ".."
	char tempPath[256+1];
	memset(tempPath, 0, sizeof(tempPath));
	strcpy(tempPath, path);
	if(
		(pathLen >= 2)
	)
	{
		(*cnt)++;
		size += strlen(baseDirectory)+1;
	}
	//we reach base path, add "/"
	else{
		(*cnt)++;
		size += strlen(leaveDirDirectory)+1;
	}
	
	//Create TGDS Dir API context
	struct FileClassList * fileClassListCtx = initFileList();
	cleanFileList(fileClassListCtx);

	//Use TGDS Dir API context
	int startFromIndex = 0;
	struct FileClass * fileClassInst = NULL;
	fileClassInst = FAT_FindFirstFile(tempPath, fileClassListCtx, startFromIndex);
	
	//Generate Dir
	int curdirCount = getCurrentDirectoryCount(fileClassListCtx);
	
	//Iterate and fill
	int dirIter = 0;
	for(dirIter = 0; dirIter < curdirCount; dirIter++){
		struct FileClass * fileClassInst = getFileClassFromList(dirIter, fileClassListCtx);
		char curFileDirName[256+1];
		strcpy(curFileDirName, fileClassInst->fd_namefullPath);
		if(mask){
			sint8 *ext = _FS_getFileExtension(curFileDirName);
			if ((ext && strstr(mask, ext)) || (fileClassInst->type == FT_DIR)){
				//Count files and directories
				if(fileClassInst->type == FT_FILE){
					(*cnt)++;
					parsefileNameTGDS(curFileDirName);
					strcpy(fileClassInst->fd_namefullPath, curFileDirName);
					size += strlen(fileClassInst->fd_namefullPath)+1;
				}
				else if(fileClassInst->type == FT_DIR){
					(*cnt)++;
					parseDirNameTGDS(curFileDirName);
					strcpy(fileClassInst->fd_namefullPath, curFileDirName);
					size += strlen(fileClassInst->fd_namefullPath)+2;	//add trailing "/"
				}
			}
		}//so far requires to remove last leading "/" from file/dirs
		else{
			//Count files and directories
			if(fileClassInst->type == FT_FILE){
				(*cnt)++;
				size += strlen(curFileDirName)+1;
			}
			else if(fileClassInst->type == FT_DIR){
				(*cnt)++;
				size += strlen(curFileDirName)+2;	//add trailing "/"
			}
		}
	}
	
	sint8	**list = (sint8	**)malloc((*cnt)*sizeof(sint8 *)+size);
	sint8	*ptr = ((sint8 *)list) + (*cnt)*sizeof(sint8 *);
	int i = 0; 
	
	//If directory is not root, add ".."
	if(
		(pathLen > 2)
	)
	{
		strcpy(ptr, leaveDirDirectory);
		list[i++] = ptr;
		ptr += strlen(leaveDirDirectory)+1;
	}
	//we reach base path, add "/"
	else{
		strcpy(ptr, baseDirectory);
		list[i++] = ptr;
		ptr += strlen(baseDirectory)+1;
	}
	
	//Iterate and fill
	for(dirIter = 0; dirIter < curdirCount; dirIter++){
		struct FileClass * fileClassInst = getFileClassFromList(dirIter, fileClassListCtx);
		char curFileDirName[256+1];
		strcpy(curFileDirName, fileClassInst->fd_namefullPath);
		if(mask){
			sint8 *ext = _FS_getFileExtension(curFileDirName);
			if ((ext && strstr(mask, ext)) || (fileClassInst->type == FT_DIR)){
				if(fileClassInst->type == FT_FILE){
					strcpy(ptr, curFileDirName);
					list[i++] = ptr;
					ptr += strlen(curFileDirName)+1;
				}
				else if(fileClassInst->type == FT_DIR){
					char dirName[MAX_TGDSFILENAME_LENGTH+1];
					memset(dirName, 0, sizeof(dirName));
					strcpy(dirName, (curFileDirName));
					strcat(dirName, "/");
					
					strcpy(ptr, dirName);
					list[i++] = ptr;
					ptr += strlen(dirName)+1;
				}
			}
		}
		else{
			if(fileClassInst->type == FT_FILE){
				strcpy(ptr, curFileDirName);
				list[i++] = ptr;
				ptr += strlen(curFileDirName)+1;
			}
			else if(fileClassInst->type == FT_DIR){
				char dirName[MAX_TGDSFILENAME_LENGTH+1];
				memset(dirName, 0, sizeof(dirName));
				strcpy(dirName, (curFileDirName));
				strcat(dirName, "/");
				
				strcpy(ptr, dirName);
				list[i++] = ptr;
				ptr += strlen(dirName)+1;
			}
		}
	}
	
	//Free TGDS Dir API context
	freeFileList(fileClassListCtx);
	return list;
}

int	FS_loadROM(sint8 *ROM, sint8 *filename)
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
	GUI_printf("Read done\n");
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
