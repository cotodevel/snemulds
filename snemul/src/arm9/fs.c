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
#include <malloc.h>
#include <stdarg.h>
#include "common.h"

#include "fs.h"
#include "gui.h"

#ifdef USE_GBFS
#include <gbfs.h>  
#elif defined(USE_GBA_FAT_LIB)
#include "fat/gba_nds_fat.h"
#elif defined(USE_LIBFAT)
#include <fat.h>
#include <unistd.h>
#include <sys/dir.h>
#endif

char *getFileExtension(char *filename)
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
		ext[i] = toupper(ptr[i]); 
	ext[i] = 0;
	return ext;
}

#ifdef USE_GBFS

/* *********************** GBFS code (by AlekMaul) **************** */

#include <gbfs.h>  

bool isFATSystem=false;   
GBFS_FILE  const* data_gbfs=0;


void	FS_init()
{
  printf("Initialize FS...\n");
    
  // Init at the beginning  of the GBFS rom
  WAIT_CR &= ~0x80;
  data_gbfs = find_first_gbfs_file((void *) 0x08000000);
  
}

int		FS_chdir(const char *path)
{
	return 0;
}

t_list 	*FS_getDirectoryList(char *mask)
{
	char	filename[250];
	int		cnt;
	t_list	*list;
	int		i;
	int		type;
    int 	cntGBFS=0;    
    const char * pFile;
		
    cntGBFS = gbfs_count_objs(data_gbfs);

    if (cntGBFS == 0)
		return NULL;	 	

	list = GUI_createList(cntGBFS);
	cnt = 0;
	for (i = 0; i < cntGBFS; i++)
	{
        pFile = gbfs_get_nth_obj(data_gbfs,i,filename,NULL);

		if (!strcmp(filename, "."))
			continue;		
				 				
  	   	if (mask)
		{
	        char *ext = getFileExtension(filename);
	        if (ext && strstr(mask, ext))
	        {
	        	list->items[cnt].info = type;
	        	strncpy(list->items[cnt].str, filename, 95);
	        	cnt++;
	        }
		} else
		{
        	list->items[cnt].info = type;
        	strncpy(list->items[cnt].str, filename, 95);			
			cnt++;
		}
	}
	list->nb_items = cnt;
	return list;
}

void	FS_printlog(char *buf)
{
}

void	FS_flog(char *fmt, ...)
{
}

char	*FS_loadROM(char *filename, int *size)
{
	char	*ROM;
  	char szFile[250];  
  	const char * pFile;
	
	mem_clear_paging(); // FIXME: move me...
    ROM  =  gbfs_get_obj(data_gbfs, strrchr(filename,'/')+1, (u32 *)size);
/*     
    int i=0;
    while (1) {
      pFile = gbfs_get_nth_obj(data_gbfs,i++,szFile,(u32 *) size);
      if (!strcmp(szFile,strrchr(filename,'/')+1))
        break;
    }
*/    
    iprintf("%s %d\n", strrchr(filename,'/')+1, *size);
		
	return ROM; 
}

int	FS_loadFile(char *filename, char *buf, int size)
{
	char	*ptr;
  	char szFile[250];  
  	const char * pFile;
	
    ptr  =  gbfs_get_obj(data_gbfs, strrchr(filename,'/')+1, NULL);
    memcpy(buf, ptr, size); 
	return 0;
}

int	FS_saveFile(char *filename, char *buf, int size)
{
	return 0;
}

// ROM paging is useless with GBFS (we have contiguous memory)

char *FS_loadROMForPaging(char *filename, int *size)
{
	return NULL;
}

int	FS_loadROMPage(char *buf, unsigned int pos, int size)
{
	return -1;
}

int	FS_shouldFreeROM()
{
	return 0;
}

#elif defined(USE_GBA_FAT_LIB)

/* *********************** FAT LIB ************************ */

static FAT_FILE *currentFile;
static char *currentFileName[100];

void	FS_init()
{
	int i;
	printf("Please restart if stuck...\n");
	for (i = 0; i < 3; i++)
	{
		printf("Initialize FS...\n");
		if (FAT_InitFiles())
		{
			printf("Found FS!\n");
			break;
		}
	}
	printf("FAT_InitFiles returned error.\nTrying anyway...\n");
//	FAT_chdir("/SNES");
}

int		FS_chdir(const char *path)
{
	return FAT_chdir(path);
}

t_list 	*FS_getDirectoryList(char *mask)
{
	char	filename[250];
	int		cnt;
	t_list	*list;
	int		i;
	int		type;
		
	cnt = i = 0;
    while (1)
	 {
		if (i == 0)
			type = (int)FAT_FindFirstFile(filename);
		else
			type =(int)FAT_FindNextFile(filename);	
		if (!type)
			break;	 	
		i++;				
		if (!strcmp(filename, "."))
			continue;		
						
		if (mask)
		{
			char *ext = getFileExtension(filename);
			if (ext && strstr(mask, ext))
				cnt++;						
		} else
		  cnt++;
	 }

	list = GUI_createList(cnt);
	cnt = i = 0; 
	while (1)
	{
		if (i == 0)
			type = (int)FAT_FindFirstFileLFN(filename);
		else
			type =(int)FAT_FindNextFileLFN(filename);	
		if (!type)
			break;
		i++;				
		if (!strcmp(filename, "."))
			continue;		
				 				
  	   	if (mask)
		{
	        char *ext = getFileExtension(filename);
	        if (ext && strstr(mask, ext))
	        {
	        	list->items[cnt].info = type;
	        	strncpy(list->items[cnt].str, filename, 95);
	        	cnt++;
	        }
		} else
		{
	       	list->items[cnt].info = type;
	       	strncpy(list->items[cnt].str, filename, 95);		
			cnt++;
		}
	}
	return list;
}

static char logbuf[32000];

void	FS_printlog(char *buf)
{
#if 1	
	static FAT_FILE *f_log = NULL;
	static int i = 0;
	
	if (i == 0)
	{
		// first call
		strcpy(logbuf, buf);
		i = 1;
		return;
	}	
	if( strlen(buf)+strlen(logbuf) >= 32000)
	{
		// Flush buffer
		char name[30];
		sprintf(name,"snemul%d.log", i);
		f_log = FAT_fopen(name, "w");	
		FAT_fwrite(logbuf, 1, strlen(logbuf), f_log);
		FAT_fclose(f_log);
		
		strcpy(logbuf, buf);
		i++;
	}
	else
		strcat(logbuf, buf); 
#endif	
}

void	FS_flog(char *fmt, ...)
{
	va_list ap;	
	FAT_FILE *f_log;
	static char	*buf = NULL;

	if (!buf)
	{
		buf = malloc(255);
	}
    va_start(ap, fmt);
    vsnprintf(buf, 255, fmt, ap);
    va_end(ap);

	FS_printlog(buf);
}

char	*FS_loadROM(char *filename, int *size)
{
	FAT_FILE *f;
	char	*ROM;
	
	mem_clear_paging(); // FIXME: move me...
	f = FAT_fopen(filename, "r");
	*size = 524800;
//	*size = FAT_GetFileSize();
	iprintf("%s %d\n", filename, *size);
	if (!(ROM = malloc(*size)))
	{
		iprintf("Couldn't allocate memory\n");
		FAT_fclose(f);		
		return NULL;
	}
	FAT_fread(ROM, 1, *size, f);
	iprintf("Read done\n");	
	FAT_fclose(f);
	
		
	return ROM; 
}

int	FS_loadFile(char *filename, char *buf, int size)
{
	FAT_FILE *f;
	f = FAT_fopen(filename, "r");
	if (f == NULL)
		return -1;
	if (FAT_GetFileSize() < size)
	{
		FAT_fclose(f);		
		return -1;
	}
	FAT_fread(buf, 1, size, f);
	FAT_fclose(f);	
	return 0;
}

int	FS_saveFile(char *filename, char *buf, int size)
{
	FAT_FILE *f;
 	// 3 retries for my buggy M3 slim  
  	if ((f = FAT_fopen(filename, "w")) == NULL)
	  if ((f = FAT_fopen(filename, "w")) == NULL)
	  	if ((f = FAT_fopen(filename, "w")) == NULL)	  
  			return 0;
	FAT_fwrite(buf, 1, size, f);
	FAT_fclose(f);	
	return 0;
}

char *FS_loadROMForPaging(char *filename, int *size)
{
	char	*ROM;
		
	mem_clear_paging(); // FIXME: move me...		
	currentFile = FAT_fopen(filename, "r");
	strcpy(currentFileName, filename);
	*size = FAT_GetFileSize();
	iprintf("%s %d\n", filename, *size);
	// Read first 1024 Kbytes
	if (!(ROM = malloc(1*1024*1024)))
	{
		iprintf("Couldn't allocate memory\n");
		FAT_fclose(currentFile);		
		return NULL;
	}
	FAT_fread(ROM, 1, 1*1024*1024, currentFile);
	FAT_fclose(currentFile);

	return ROM;
}

int	FS_loadROMPage(char *buf, unsigned int pos, int size)
{
	int ret;
	currentFile = FAT_fopen(currentFileName, "r");
	if (currentFile == NULL)
		LOG("currentFile is NULL\n");
	
	ret = FAT_fseek(currentFile, pos, SEEK_SET);
	if (ret < 0)
		LOG("first fseek is NULL\n");
			
	ret = FAT_fread(buf, 1, size, currentFile);
	FAT_fclose(currentFile);
	return ret;	
}

int	FS_shouldFreeROM()
{
	return 1;
}
#elif defined(USE_LIBFAT)


/* *********************** LIBFAT ************************ */

static FILE *currentFile;
static char *currentFileName[100];

void	FS_init()
{
	iprintf("Initialize FS...\n");
	iprintf("Please restart if stuck...\n");
	if (fatInitDefault())
//	if (fatInitialise(4, true)) 
	{
		iprintf("FS OK!\n");
    	// Success
	} else {
    	// Failure
    	iprintf("FS Failure, continue anyway...\n");
	} 
}

int		FS_chdir(const char *path)
{
	return chdir(path);
}

t_list 	*FS_getDirectoryList(char *path, char *mask)
{
	char	filename[250];
	int		cnt;
	t_list	*list;
	int		ret;	
	DIR_ITER	*dir;
	struct stat	st;
		
	dir = diropen(path);
	cnt = 0;
    while (1)
	 {
	 	ret = dirnext(dir, filename, &st);
		if (ret < 0)
			break;	 
		if (!S_ISREG(st.st_mode))
			continue;
		if (!strcmp(filename, "."))
			continue;		
iprintf("%s %o\n", filename, st.st_mode);			
						
		if (mask)
		{
			char *ext = getFileExtension(filename);
			if (ext && strstr(mask, ext))
				cnt++;						
		} else
		  cnt++;
	}
	 
	dirreset(dir);
	list = GUI_createList(cnt);
	cnt =  0; 
	while (1)
	{
		ret = dirnext(dir, filename, &st);
		
		if (ret < 0)
			break;	 
		if (!S_ISREG(st.st_mode))
			continue;
		if (!strcmp(filename, "."))
			continue;		
				 				
  	   	if (mask)
		{
	        char *ext = getFileExtension(filename);
	        if (ext && strstr(mask, ext))
	        {
	        	list->items[cnt].info = 1;
	        	strncpy(list->items[cnt].str, filename, 95);
	        	cnt++;
	        }
		} else
		{
	       	list->items[cnt].info = 1;
	       	strncpy(list->items[cnt].str, filename, 95);		
			cnt++;
		}
	}
	dirclose(dir);
	return list;
}

char logbuf[32000];
static int logcnt = 0;

void	FS_printlogBufOnly(char *buf)
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
		f_log = fopen(name, "w");	
		fwrite(logbuf, 1, strlen(logbuf), f_log);
		fclose(f_log);
		
		strcpy(logbuf, buf);
		logcnt++;
	}
	else
		strcat(logbuf, buf);
#endif	
}

static char flog_buf[255];

void	FS_flog(char *fmt, ...)
{
		
	va_list ap;	

    va_start(ap, fmt);
    vsnprintf(flog_buf, 255, fmt, ap);
    va_end(ap);

	FS_printlog(flog_buf);
}

char	*FS_loadROM(char *filename, int *size)
{
	FILE	*f;
	char	*ROM;
	struct stat	st;
	
	stat(filename, &st);
	*size = st.st_size;
//	*size = 524800;
		
	f = fopen(filename, "r");

	iprintf("%s %d\n", filename, *size);
	if (!(ROM = malloc(*size)))
	{
		iprintf("Couldn't allocate memory\n");
		fclose(f);		
		return NULL;
	}
	fread(ROM, 1, *size, f);
	iprintf("Read done\n");	
	fclose(f);
	
		
	return ROM; 
}

int	FS_loadFile(char *filename, char *buf, int size)
{
	FILE *f;
	struct stat	st;
	
	stat(filename, &st);
	f = fopen(filename, "r");
	if (f == NULL)
		return -1;
	if (st.st_size < size)
	{
		fclose(f);		
		return -1;
	}
	fread(buf, 1, size, f);
	fclose(f);	
	return 0;
}

int	FS_saveFile(char *filename, char *buf, int size)
{
	FILE *f;
 	// 3 retries for my buggy M3 slim  
  	if ((f = fopen(filename, "w")) == NULL)
/*	  if ((f = fopen(filename, "w")) == NULL)
	  	if ((f = fopen(filename, "w")) == NULL)*/	  
  			return 0;
	fwrite(buf, 1, size, f);
	fclose(f);	
	return 0;
}

char *FS_loadROMForPaging(char *filename, int *size)
{
	char	*ROM;
	struct stat	st;
	
	stat(filename, &st);
	currentFile = fopen(filename, "r");
	strcpy(currentFileName, filename);
	*size = st.st_size;
	iprintf("%s %d\n", filename, *size);
	// Read first 1024 Kbytes
	if (!(ROM = malloc(1*1024*1024)))
	{
		iprintf("Couldn't allocate memory\n");
		fclose(currentFile);		
		return NULL;
	}
	fread(ROM, 1, 1*1024*1024, currentFile);
	fclose(currentFile);

	return ROM;
}

int	FS_loadROMPage(char *buf, unsigned int pos, int size)
{
	int ret;
	currentFile = fopen(currentFileName, "r");
	if (currentFile == NULL)
		LOG("currentFile is NULL\n");
	
	ret = fseek(currentFile, pos, SEEK_SET);
	if (ret < 0)
		LOG("first fseek is NULL\n");
			
	ret = fread(buf, 1, size, currentFile);
	fclose(currentFile);
	return ret;	
}

int	FS_shouldFreeROM()
{
	return 1;
}

#endif


#ifdef FAKE_FS

/* *********************** FAKE FS ************************ */

#include "cc_runme_smc.h"
#include "snes9x_smc.h"
//#include "test_smc.h"
#include "mario_smc.h"


#define FAT_FindFirstFile(x) __FAT_FindFirstFile(x)
#define FAT_FindNextFile(x) __FAT_FindNextFile(x)
#define FAT_FindFirstFileLFN(x) __FAT_FindFirstFileLFN(x)
#define FAT_FindNextFileLFN(x) __FAT_FindNextFileLFN(x)
#define FAT_InitFiles() __FAT_InitFiles()

//char	*__list[] = { "SNES9X.SMC", "MARIO.SMC" };
char	*__list[] = { "SNES9X.SMC" };
//char	*__list[] = { "CC-RUNME.SMC" };
//char	*__list[] = { "MARIO.SMC" };
int		__cnt = 0;

int __FAT_FindFirstFile(char *filename)
{
	__cnt = 0;
	strcpy(filename, __list[0]);
	__cnt++;	
	return 1;
}
int __FAT_FindNextFile(char *filename)
{
	if (__cnt >= sizeof(__list)/sizeof(char *))
		return 0;
	strcpy(filename, __list[__cnt]);
	__cnt++;	
	return 1;

}
int __FAT_FindFirstFileLFN(char *filename)
{
	__cnt = 0;
	strcpy(filename, __list[0]);
	__cnt++;	
	return 1;
}

int __FAT_FindNextFileLFN(char *filename)
{
	if (__cnt >= sizeof(__list)/sizeof(char *))
		return 0;
	strcpy(filename, __list[__cnt]);
	__cnt++;	
	return 1;
}

char	*FS_loadROM(char *filename, int *size)
{
	char	*ROM;
	
	ROM = NULL;
	if (!strcmp(filename, "/MARIO.SMC") ||
		!strcmp(filename, "/SNES/MARIO.SMC"))
	{
		ROM = (char *)mario_smc;
		*size = mario_smc_size;
	}
	if (!strcmp(filename, "/SNES9X.SMC") ||
	!strcmp(filename, "/SNES/SNES9X.SMC"))
	{
		ROM = (char *)snes9x_smc;
		*size = snes9x_smc_size;
	}	
	if (!strcmp(filename, "/CC-RUNME.SMC"))
	{
		ROM = (char *)cc_runme_smc;
		*size = cc_runme_smc_size;
	}	
		
	return ROM; 
}

void	FS_init()
{
}

int		FS_chdir(const char *path)
{
}

t_list 	*FS_getDirectoryList(char *mask)
{
	char	filename[250];
	int		cnt;
	t_list	*list;
	int		i;
	int		type;
		
	cnt = i = 0;
    while (1)
	 {
		if (i == 0)
			type = (int)FAT_FindFirstFile(filename);
		else
			type =(int)FAT_FindNextFile(filename);	
		if (!type)
			break;	 	
		i++;				
		if (!strcmp(filename, "."))
			continue;		
						
		if (mask)
		{
			char *ext = getFileExtension(filename);
			if (ext && strstr(mask, ext))
				cnt++;						
		} else
		  cnt++;
	 }

	list = GUI_createList(cnt);
	cnt = i = 0; 
	while (1)
	{
		if (i == 0)
			type = (int)FAT_FindFirstFileLFN(filename);
		else
			type =(int)FAT_FindNextFileLFN(filename);	
		if (!type)
			break;
		i++;				
		if (!strcmp(filename, "."))
			continue;		
				 				
  	   	if (mask)
		{
	        char *ext = getFileExtension(filename);
	        if (ext && strstr(mask, ext))
	        {
	        	list->items[cnt].info = type;
	        	strncpy(list->items[cnt].str, filename, 95);
	        	cnt++;
	        }
		} else
		{
	       	list->items[cnt].info = type;
	       	strncpy(list->items[cnt].str, filename, 95);		
			cnt++;
		}
	}
	return list;
}

char logbuf[32000];

void	DesMuMeDebug(void *a, void *b, void *c, void *d); 

void	FS_printlog(char *buf)
{
#if 0
	static int i = 0;
	
	if (i == 0)
	{
		// first call
		strcpy(logbuf, buf);
		i = 1;
		return;
	}	
	if( strlen(buf)+strlen(logbuf) >= 32000)
	{
		// Flush buffer
		char name[30];
		strcpy(logbuf, buf);
		i++;
	}
	else
		strcat(logbuf, buf); 

#else
	DesMuMeDebug(buf, NULL, NULL, NULL);
#endif	

}

static char flog_buf[255];

void	FS_flog(char *fmt, ...)
{
		
	va_list ap;	

    va_start(ap, fmt);
    vsnprintf(flog_buf, 255, fmt, ap);
    va_end(ap);

	FS_printlog(buf);
}


int	FS_loadFile(char *filename, char *buf, int size)
{
	return -1;
}

int	FS_saveFile(char *filename, char *buf, int size)
{
	return -1;
}

char *FS_loadROMForPaging(char *filename, int *size)
{
	return NULL;
}

int	FS_loadROMPage(char *buf, unsigned int pos, int size)
{
	return 0;	
}

int	FS_shouldFreeROM()
{
	return 0;
}

#endif
