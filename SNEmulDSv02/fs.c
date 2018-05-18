/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>

#include "fat/gba_nds_fat.h"
#include "fs.h"
#include "gui.h"

//#define EMULE_FS

#ifdef EMULE_FS
#include "cc_runme_smc.h"
#include "snes9x_smc.h"
#include "test_smc.h"



#define FAT_FindFirstFile(x) __FAT_FindFirstFile(x)
#define FAT_FindNextFile(x) __FAT_FindNextFile(x)
#define FAT_FindFirstFileLFN(x) __FAT_FindFirstFileLFN(x)
#define FAT_FindNextFileLFN(x) __FAT_FindNextFileLFN(x)
#define FAT_InitFiles() __FAT_InitFiles()

char	*__list[] = { "CC_RUNME.SMC", "SNES9X.SMC", "TEST.SMC" };
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
	if (!strcmp(filename, "SNES9X.SMC"))
	{
		ROM = (char *)snes9x_smc;
		*size = snes9x_smc_size;
	}
	if (!strcmp(filename, "CC_RUNME.SMC"))
	{
		ROM = (char *)cc_runme_smc;
		*size = cc_runme_smc_size;
	}	
	if (!strcmp(filename, "TEST.SMC"))
	{
		ROM = (char *)test_smc;
		*size = test_smc_size;
	}	
		
	return ROM; 
}

bool __FAT_InitFiles()
{
	return true;
}

#endif


void	FS_init()
{
	printf("Please restart if stuck...\n");
	while (1)
	{
		printf("Initialize FS...\n");
		if (FAT_InitFiles())
			break;
	}
	printf("Found FS!\n");
//	FAT_chdir("/SNES");
}

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
	 if (cnt == 0)
	 	return NULL;	 	

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
			cnt++;
	}
	return list;
}

void	FS_printlog(char *buf)
{
#if 1	
	static FAT_FILE *f_log = NULL;
	
	if (!f_log)
		f_log = FAT_fopen("snemul.log", "w");
	FAT_fwrite(buf, 1, strlen(buf), f_log);
//	FAT_fclose(f_log);
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

#ifndef EMULE_FS
char	*FS_loadROM(char *filename, int *size)
{
	FAT_FILE *f;
	char	*ROM;
	
	mem_clear_paging(); // FIXME: move me...
	f = FAT_fopen(filename, "r");
	*size = FAT_GetFileSize();
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
	f = FAT_fopen(filename, "w");
	FAT_fwrite(buf, 1, size, f);
	FAT_fclose(f);	
	return 0;
}


static FAT_FILE *currentFile;
static char *currentFileName = NULL;

char *FS_loadROMForPaging(char *filename, int *size)
{
	char	*ROM;
		
	mem_clear_paging(); // FIXME: move me...		
	currentFile = FAT_fopen(filename, "r");
	currentFileName = strdup(filename);
	*size = FAT_GetFileSize();
	iprintf("%s %d\n", filename, *size);
	// Read first 512 Kbytes
	if (!(ROM = malloc(1*1024*1024)))
	{
		iprintf("Couldn't allocate memory\n");
		FAT_fclose(currentFile);		
		return NULL;
	}
	FAT_fread(ROM, 1, 1*1024*1024, currentFile);
//	FAT_fclose(currentFile);

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
#endif