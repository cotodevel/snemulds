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

#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <nds/memory.h>
#include <stdarg.h>
#include <ctype.h>
#include "common_shared.h"

#include "fs.h"
#include "gui.h"
#include "ram.h"

#include "diskio.h"
#include "ff.h"

#include <unistd.h>
//#include <sys/dir.h>
#include <fcntl.h>

//fatfs
static FATFS dldiFs;

uint16	*g_extRAM = NULL;
int		g_UseExtRAM = 0;


int		FS_extram_init()
{
	g_extRAM = (u16*) ram_init();
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

char *_FS_getFileExtension(char *filename)
{
	static char ext[4];
	char	*ptr;
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

char *FS_getFileName(char *filename)
{
	static char name[100];
	char	*ptr;
	int		i;
	
	ptr = filename;
	ptr = strrchr(ptr, '.');
		
	for (i = 0; i < ptr-filename; i++)
		name[i] = filename[i]; 
	name[i] = 0;
	return name;
}

/* *********************** FSFAT ************************ */

FIL GLOBAL_FHANDLER;
static char currentFileName[100];

int		FS_init()
{
	return (f_mount(&dldiFs, "0:", 1));
}

int		FS_chdir(const char *path)
{
	FS_lock();
	//int ret = chdir(path);
	int ret =f_chdir (path);
	FS_unlock();
	return ret;
}


char	**FS_getDirectoryList(char *path, char *mask, int *cnt)
{	
	int			size;
		
	FS_lock();	
	DIR dir;
	*cnt = size = 0;
	
	FRESULT res;
	
	res = f_opendir (&dir, path);
	
	if( res == FR_OK )
	{
		while (1)
		{
			FILINFO fno;
			res = f_readdir (&dir, &fno);
			
			if(res != FR_OK || !(fno.fname[0])) break;
			
			if (!strcmp(fno.fname, "."))
				continue;		
			
			if (!strcmp(fno.fname, ".."))
				continue;		
			
			if (mask)
			{
				char *ext = _FS_getFileExtension(fno.fname);
				if (ext && strstr(mask, ext))
				{
					(*cnt)++;
					size += strlen(fno.fname)+1;
				}
			} else
			{
			  (*cnt)++;
			  size += strlen(fno.fname)+1;
			}
		}
	}
	
	res = f_rewinddir(&dir);
	
	char	**list = malloc((*cnt)*sizeof(char *)+size);
	char	*ptr = ((char *)list) + (*cnt)*sizeof(char *);
	
	int i = 0; 
	if( res == FR_OK )
	{
		while (1)
		{
			
			FILINFO fno;
			res = f_readdir (&dir, &fno);
			
			if(res != FR_OK || !fno.fname[0]) break;
			
			if (!strcmp(fno.fname, "."))
				continue;		
			
			if (!strcmp(fno.fname, ".."))
				continue;		
			
			
			if (mask)
			{
				char *ext = _FS_getFileExtension(fno.fname);
				if (ext && strstr(mask, ext))
				{
					strcpy(ptr, fno.fname);
					list[i++] = ptr;
					ptr += strlen(fno.fname)+1;  
				}
			} else
			{
				strcpy(ptr, fno.fname);
				list[i++] = ptr;
				ptr += strlen(fno.fname)+1;
			}
		}
	}
	f_closedir(&dir);
	FS_unlock();
	return list;
}

char logbuf[32000];
static int logcnt = 0;

void	FS_printlogBufOnly(char *buf)
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

void	FS_printlog(char *buf)
{
 
	FIL f_log;
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
		char name[30];
		sprintf(name,"snemul%d.log", logcnt%100);
		
		FS_lock();
		f_open(&f_log,name,FA_WRITE | FA_OPEN_ALWAYS);
		
		unsigned int written;
		f_write(&f_log, logbuf, strlen(logbuf), &written);
		f_truncate(&f_log);
		
		f_close(&f_log);
		FS_unlock();
		
		strcpy(logbuf, buf);
		logcnt++;
	}
	else
		strcat(logbuf, buf);
}

extern char g_printfbuf[100];

void	FS_flog(char *fmt, ...)
{
		
	va_list ap;	

    va_start(ap, fmt);
    vsnprintf(g_printfbuf, 100, fmt, ap);
    va_end(ap);

	FS_printlog(g_printfbuf);
}

int	FS_loadROM(char *ROM, char *filename)
{
	FIL	f;
	
	FS_lock();
	
	f_open(&f,filename,FA_READ);
	
	int size = FS_getFileSize(filename);
	
	//Prevent Cache problems.
	DC_FlushRange((u32*)ROM, (int)size);
	
	unsigned int read_so_far;
	f_read(&f, ROM, size, &read_so_far);
	
	GUI_printf("Read done\n");
	f_close(&f);
	
	FS_unlock();

	return 1;
}


int	FS_getFileSize(char *filename)
{	
	FIL FHANDLER;
	FS_lock();	
	
	int retval = 0;
	if(f_open(&FHANDLER, filename, FA_READ) != FR_OK){
		retval = -1;
	}
	
	retval = (int)f_size(&FHANDLER);
	f_close(&FHANDLER);
	return retval;
}

//tried size == file_size (rom size)
int	FS_loadFile(char *filename, char *buf, int size)
{
	FIL f;
	int file_size;
	
	FS_lock();	
	if(!(f_open(&f,filename,FA_READ) == FR_OK))
	{
		FS_unlock();
		return -1;
	}
	
	file_size = FS_getFileSize(filename);
	
	if (file_size < size)
	{
		f_close(&f);
		FS_unlock();
		return -1;
	}
	
	unsigned int read_so_far;
	f_read(&f, buf, size, &read_so_far);
	
    f_close(&f);
	FS_unlock();
	return 0;
}


int	FS_saveFile(char *filename, char *buf, int size)
{
	FIL f;
	
	FS_lock();
  	if(!(f_open(&f,filename,FA_WRITE ) == FR_OK))
	{
  		FS_unlock();
  		return 0;
  	}
	
	unsigned int written;
	f_write(&f, buf, size, &written);
	f_truncate(&f);
	
	f_close(&f);
	FS_unlock();
	return 0;
}


int	FS_loadROMForPaging(char *ROM, char *filename, int size)
{
	g_UseExtRAM = 0;
	FS_lock();
	
	if (f_open(&GLOBAL_FHANDLER, filename, FA_READ)!= FR_OK)
	{
		FS_unlock();
		return -1;
	}
	strcpy(currentFileName, filename);
	unsigned int read_so_far;
	f_read(&GLOBAL_FHANDLER, ROM, size, &read_so_far);

	FS_unlock();
	
	return 0;
}

//Deprecated. 4MB support only
int	FS_loadROMInExtRAM(char *ROM, char *filename, int size, int total_size)
{
	return -1;
}

int	FS_loadROMPage(char *buf, unsigned int pos, int size)
{	

	FS_lock();
	
	f_lseek (&GLOBAL_FHANDLER, pos);
	
	unsigned int read_so_far;
	f_read(&GLOBAL_FHANDLER, buf, size, &read_so_far);
	
	FS_unlock();	
	return (int)read_so_far;	
}

int	FS_shouldFreeROM()
{
	return 1;
}

