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

#include "specific_shared.h"
#include "devoptab_devices.h"
#include "posix_hook_shared.h"
#include "toolchain_utils.h"

#include "fs.h"
#include "gui.h"
#include "nds_cp15_misc.h"

#include <unistd.h>
#include <sys/dir.h>
#include <fcntl.h>
#include "fsfat_layer.h"


#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "ram.h"
#include "conf.h"
#include "snemul_str.h"
#include "frontend.h"
#include "main.h"
#include "dldi.h"
#include "ppu.h"
#include "InterruptsARMCores_h.h"
#include "specific_shared.h"
#include "ff.h"
#include "mem_handler_shared.h"
#include "reent.h"
#include "sys/types.h"
#include "engine.h"
#include "core.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefs.h"
#include "console.h"
#include "api_wrapper.h"
#include "apu_jukebox.h"
#include "toolchain_utils.h"
#include "about.h"
#include "file.h"

uint16	*g_extRAM = NULL;
int		g_UseExtRAM = 0;

int		FS_extram_init()
{
	g_extRAM = (uint16*) ram_init();
	if (g_extRAM)
	{
		//g_extRAM = (uint16 *)ram_unlock();
		return ram_size();
	}
	return -1;
}

void	FS_lock()
{
	if (g_extRAM)
		ram_lock();
}

void	FS_unlock()
{
	if (g_extRAM)
		ram_unlock();
}

sint8 *_FS_getFileExtension(sint8 *filename)
{
	static sint8 ext[4];
	sint8	*ptr;
	int		i;
	
	ptr = filename;
	do
	{
		ptr = strchr(ptr, '.');
		if (!ptr)
			return NULL;
		ptr++;
	}
	while (strlen(ptr) > 3);
		
	for (i = 0; i < strlen(ptr); i++)
		ext[i] = toupper((int)(ptr[i])); 
	ext[i] = 0;
	return ext;
}

sint8 *FS_getFileName(sint8 *filename)
{
	static sint8 name[100];
	sint8	*ptr;
	int		i;
	
	ptr = filename;
	ptr = strrchr(ptr, '.');
		
	for (i = 0; i < ptr-filename; i++)
		name[i] = filename[i]; 
	name[i] = 0;
	return name;
}

/* *********************** FAT ************************ */

//sint32	currentfd = -1;	//for hooks with FS
//uint8 currentFileName[100];

int		FS_init()
{
	return fatfs_init();
}

int		FS_chdir(const sint8 *path)
{
	FS_lock();
	int ret = fatfs_chdir(path);	//old: chdir(path);	//could cause issues
	FS_unlock();
	return ret;
}


//ok so far. Rets OK dir listing
sint8	**FS_getDirectoryList(sint8 *path, sint8 *mask, int *cnt)
{	
	int			size;
		
	FS_lock();	
	DIR *dir = opendir (path); 
	*cnt = size = 0;
	if( NULL != dir )
	{
		while (1)
		{
			struct dirent* pent = readdir(dir);	//if NULL already not a dir.
			if(pent == NULL){
				break;
			}
			
			
			//if ( pent->d_type & DT_DIR ) continue;
			//posix standard
			struct fd * fdinst = fd_struct_get(pent->d_ino);
			if (!S_ISDIR(fdinst->stat.st_mode)) { 
				continue;
			}
			
			if (!strcmp(pent->d_name, ".")){
				continue;
			}
			
			if (mask)
			{
				sint8 *ext = _FS_getFileExtension(pent->d_name);
				
				//printf("fname:%s",pent->d_name);
				//printf("fname:%s",fdinst->cur_entry.d_name);
				//printf("ext:%s",ext);
				//sint8 * found = strstr(mask, ext);
				//printf("%s",found);
				//while(1);
				
				if (ext && strstr(mask, ext))
				{
					//filecount Increase
					(*cnt)++;
					size += strlen(pent->d_name)+1;
				}
			} 
			else
			{
				//filecount Increase
				(*cnt)++;
				size += strlen(pent->d_name)+1;
			}
		}
	}
	//printf("filecount:%d",*cnt);
	//while(1);
	rewinddir(dir);
	
	sint8	**list = malloc((*cnt)*sizeof(sint8 *)+size);
	sint8	*ptr = ((sint8 *)list) + (*cnt)*sizeof(sint8 *);
	
	int i = 0; 
	if( NULL != dir )
	{
		while (1)
		{
			struct dirent* pent = readdir(dir);	//if NULL already not a dir
			if(pent == NULL){
				break;
			}
			
			//if ( pent->d_type & DT_DIR ) continue;
			//posix standard
			
			struct fd * fdinst = fd_struct_get(pent->d_ino);
			if (!S_ISDIR(fdinst->stat.st_mode)) {
				continue;
			}
			
			if (!strcmp(pent->d_name, ".")){
				continue;		
			}
			
			if (mask)
			{
				sint8 *ext = _FS_getFileExtension(pent->d_name);
				if (ext && strstr(mask, ext))
				{
					strcpy(ptr, pent->d_name);
					list[i++] = ptr;
					ptr += strlen(pent->d_name)+1;  
				}
			} else
			{
				strcpy(ptr, pent->d_name);
				list[i++] = ptr;
				ptr += strlen(pent->d_name)+1;
			}
		}
	}
	
	closedir(dir);
	FS_unlock();
	return list;
}

sint8 logbuf[32000];
static int logcnt = 0;

void	FS_printlogBufOnly(sint8 *buf)
{
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
}

void	FS_printlog(sint8 *buf)
{
 
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
		
		f_log = fopen_fs(name, "w"); //fopen(getfatfsPath(name), "w");	//old: f_log = fopen(name, "w");
		//printf("trying to fopen_fs:%s",name);
		if(f_log){
			//printf("FILE handle ok");
		}
		else{
			//printf("FILE handle error");
		}
		fwrite_fs(logbuf, 1, strlen(logbuf), f_log); //old: fwrite(logbuf, 1, strlen(logbuf), f_log);
		fclose_fs(f_log);	//old: fclose(f_log);
		FS_unlock();
		
		strcpy(logbuf, buf);
		logcnt++;
	}
	else{
		strcat(logbuf, buf);
	}
	
}

//printf that instead stores on Filesystem
void	FS_flog(sint8 *fmt, ...)
{
		
	va_list ap;	

    va_start(ap, fmt);
    vsnprintf((sint8*)g_printfbuf, 100, fmt, ap);
    va_end(ap);

	FS_printlog((sint8*)g_printfbuf);
}

//works fine
int	FS_loadROM(sint8 *ROM, sint8 *filename)
{
	FILE	*f;
	FS_lock();
	f = fopen_fs(filename, "r"); //old: f = fopen(filename, "rb");
	fseek_fs(f, 0, SEEK_END); //old: fseek(f, 0, SEEK_END);
	int size = ftell_fs(f);	//old: int size = ftell(f);
	fseek_fs(f, 0, SEEK_SET);	//old: fseek(f, 0, SEEK_SET);

	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)ROM, (int)size);
	
	fread_fs(ROM, 1, size, f);	//old: fread(ROM, 1, size, f);
	GUI_printf("Read done\n");
	fclose_fs(f);	//old: fclose(f);
	FS_unlock();

	return 1;
}

//works
int	FS_getFileSize(sint8 *filename)
{	
	/* //implemented working... POSIX way (lower level calls)
	struct stat	st;	
	FS_lock();
	stat(filename, &st);
	FS_unlock();
	return st.st_size;
	*/
	
	//implemented working... POSIX way (high level calls)
	FILE * f = fopen_fs(filename, "r");	//old: f = fopen(filename, "rb");
	if (f == NULL)
	{
		FS_unlock();
		return -1;
	}
	fseek_fs(f, 0, SEEK_END); //old: fseek(f, 0, SEEK_END);
	int size = ftell_fs(f);	//old: int size = ftell(f);
	fseek_fs(f, 0, SEEK_SET);	//old: fseek(f, 0, SEEK_SET);
	fclose_fs(f);	//old:	fclose(f);	
	
	return size;
}




/*
int	FS_saveFile(sint8 *filename, sint8 *buf, int size)
{
	FILE *f;
	FS_lock();
  	if ((f = fopen_fs(filename, "w")) == NULL)	//old: if ((f = fopen(filename, "wb")) == NULL)
  	{
  		FS_unlock();
  		return -1;
  	}
	fwrite_fs(buf, 1, size, f);	//old: fwrite(buf, 1, size, f);
	fclose_fs(f);	//old: fclose(f);	
	FS_unlock();
	return 0;
}
*/



//ret 0 success, ret -1 on open problem
/*
int	FS_loadROMForPaging(sint8 *ROM, sint8 *filename, int size)
{
	g_UseExtRAM = 0;
	
	FS_lock();
	if (currentfd != -1)
		close(currentfd);
	
	currentfd = open(filename, O_RDONLY);
	if (currentfd < 0)
	{
		FS_unlock();
		return -1;
	}
	strcpy((sint8*)currentFileName, filename);

	int ret = read(currentfd, ROM, size);	//returns read size into buffer
	
	FS_unlock();
	
	if(ret != size){
		return -1;
	}
	
	return 0;
}
*/

FILE * fPaging ;

//use posix...
int	FS_loadROMForPaging(sint8 *ROM, sint8 *filename, int size)
{
	g_UseExtRAM = 0;
	
	FS_lock();
	
	if (fPaging){
		fclose_fs(fPaging);
	}
	fPaging = fopen_fs(filename, "r"); //old: f = fopen(filename, "rb");
	sint32 fd = fileno(fPaging);
	if(fd < 0){
		FS_unlock();
		return -1;
	}
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)ROM, (int)size);
	
	int ret = fread_fs(ROM, 1, size, fPaging);
	FS_unlock();
	
	if(ret != size){
		return -1;
	}
	
	return 0;
}


//use posix..
int	FS_loadROMPage(sint8 *buf, unsigned int pos, int size){
	
	int ret;	
	FS_lock();
	
	if(!fPaging){
		return -1;
	}
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)buf, (int)size);
	
	//ret = lseek(currentfd, pos, SEEK_SET);
	ret = fseek_fs(fPaging, pos, SEEK_SET);
	
	if (ret < 0)
	{
		FS_unlock();
		return -1;
	}
	
	
	return fread_fs(buf, 1, size, fPaging);
}

/*
int	FS_loadROMPage(sint8 *buf, unsigned int pos, int size)
{	
	int ret;	
	FS_lock();
	
	ret = lseek(currentfd, pos, SEEK_SET);
	if (ret < 0)
	{
		FS_unlock();
		return -1;
	}
		
	read(currentfd, buf, size);
	
	FS_unlock();	
	return ret;	
}
*/

int	FS_shouldFreeROM()
{
	return 1;
}

