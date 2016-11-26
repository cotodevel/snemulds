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

//#include <malloc.h>
#include <stdarg.h>
#include <ctype.h>
#include "common.h"

#include "fs.h"

#include "gui_draw/gui.h"

#include "ram.h"

#include <fat.h>
#include <unistd.h>
#include <sys/dir.h>
#include <fcntl.h>

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

/* *********************** LIBFAT ************************ */

static int	currentfd = -1;
static char currentFileName[100];
//static struct stat	st;
//static char tfilename[260];

int		FS_init()
{
	//return fatInit(8, true);
	return (fatInitDefault());
}

int		FS_chdir(const char *path)
{
	FS_lock();
	int ret = chdir(path);
	FS_unlock();
	return ret;
}



char	**FS_getDirectoryList(char *path, char *mask, int *cnt)
{	
	int			size;
		
	FS_lock();	
	DIR *dir = opendir (path); 
	*cnt = size = 0;
	if( NULL != dir )
	{
		while (1)
		{
			struct dirent* pent = readdir(dir);
			if(pent == NULL) break;
			
			if ( pent->d_type & DT_DIR ) continue;
			
			if (!strcmp(pent->d_name, "."))
				continue;		
							
			if (mask)
			{
				char *ext = _FS_getFileExtension(pent->d_name);
				if (ext && strstr(mask, ext))
				{
					(*cnt)++;
					size += strlen(pent->d_name)+1;
				}
			} else
			{
			  (*cnt)++;
			  size += strlen(pent->d_name)+1;
			}
		}
	}
	rewinddir(dir);

	char	**list = malloc((*cnt)*sizeof(char *)+size);
	char	*ptr = ((char *)list) + (*cnt)*sizeof(char *);
	
	int i = 0; 
	if( NULL != dir )
	{
		while (1)
		{
			struct dirent* pent = readdir(dir);
			if(pent == NULL) break;
				
			if ( pent->d_type & DT_DIR ) continue;
			
			if (!strcmp(pent->d_name, "."))
				continue;		
									
			if (mask)
			{
				char *ext = _FS_getFileExtension(pent->d_name);
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

char logbuf[32000];
static int logcnt = 0;

void	FS_printlogBufOnly(char *buf)
{
	//static FILE *f_log = NULL;

	
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
#if 0
		static FILE *f_log = NULL;

		if (!f_log)
			f_log = fopen("/SNEMUL.LOG", "w");	
		fwrite(buf, 1, strlen(buf), f_log);
		fflush(f_log);
#else
 
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
		char name[30];
		sprintf(name,"snemul%d.log", logcnt%100);
		
		FS_lock();
		f_log = fopen(name, "w");	
		fwrite(logbuf, 1, strlen(logbuf), f_log);
		fclose(f_log);
		FS_unlock();
		
		strcpy(logbuf, buf);
		logcnt++;
	}
	else
		strcat(logbuf, buf);
#endif	
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
	FILE	*f;
//	struct stat	st;
	
	FS_lock();
//	stat(filename, &st);
	
	f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	fread(ROM, 1, size, f);
	GUI_printf("Read done\n");
	fclose(f);
	FS_unlock();

	return 1;
}


int	FS_getFileSize(char *filename)
{
	struct stat	st;	
	FS_lock();	
	stat(filename, &st);
	FS_unlock();
	return st.st_size;
}


/*int FS_getFileSize(char *filename)
{
	FS_lock();
	FILE *f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	long l = ftell(f);
	fclose(f);
	FS_unlock();
	return (int)l;
}*/

int	FS_loadFile(char *filename, char *buf, int size)
{
	FILE *f;
	int file_size;
	
	FS_lock();	
	f = fopen(filename, "rb");
	if (f == NULL)
	{
		FS_unlock();
		return -1;
	}
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	if (file_size < size)
	{
		fclose(f);
		FS_unlock();
		return -1;
	}
	fseek(f, 0, SEEK_SET);	
	fread(buf, 1, size, f);
	fclose(f);	
	FS_unlock();
	return 0;
}

int	FS_saveFile(char *filename, char *buf, int size)
{
	FILE *f;
	FS_lock();
  	if ((f = fopen(filename, "wb")) == NULL)
  	{
  		FS_unlock();
  		return 0;
  	}
	fwrite(buf, 1, size, f);
	fclose(f);	
	FS_unlock();
	return 0;
}

int	FS_loadROMForPaging(char *ROM, char *filename, int size)
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
	strcpy(currentFileName, filename);

	read(currentfd, ROM, size);

	FS_unlock();
	
	return 0;
}

int	FS_loadROMInExtRAM(char *ROM, char *filename, int size, int total_size)
{
#ifdef USE_EXTRAM
	if (g_extRAM == NULL)
		return -1;
	
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
	strcpy(currentFileName, filename);

	// Load all ROM by block of size, starting from the end
	
	// First read the last part
	int i = total_size - (total_size % size);
	GUI_printf("Load at %d, %d\n", i, total_size % size);
	FS_loadROMPage(ROM, i, total_size % size);
	// copy it in the external ram
	swiFastCopy(ROM, (uint8 *)g_extRAM+i, (total_size % size) / 4);

	i -= size;
	
	while (i >= 0)
	{
		// Read one piece of ROM into DS RAM
		GUI_printf("Load at %d, %d\n", i, size);
		FS_loadROMPage(ROM, i, size);
		
		// Than copy it in Ext RAM
		swiFastCopy(ROM, (uint8 *)g_extRAM+i, size / 4);
		
		i -= size;
	}
	g_UseExtRAM = 1;
	close(currentfd);
#endif	
	return -1;
}

int	FS_loadROMPage(char *buf, unsigned int pos, int size)
{	
#ifdef USE_EXTRAM
	if (g_UseExtRAM)
	{
		//swiFastCopy((uint8 *)extRAM+pos, buf, size / 4);
		memcpy(buf, (uint8 *)g_extRAM+pos, size);				
		return 0;
	}
#endif	

	int ret;	
	FS_lock();

	//REG_IE &= ~(IRQ_VBLANK);
	
	ret = lseek(currentfd, pos, SEEK_SET);
	if (ret < 0)
	{
		FS_unlock();
		return -1;
	}
		
	read(currentfd, buf, size);
	
	//REG_IE |= (IRQ_VBLANK);	
	FS_unlock();	
	return ret;	
}

int	FS_shouldFreeROM()
{
	return 1;
}


