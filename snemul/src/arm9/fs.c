/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* - GBFS integration by AlekMaul	 					   */
/* Free for non-commercial use                             */
/***********************************************************/

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
#else
#include "fat/gba_nds_fat.h"
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

#else

/* *********************** FAT LIB ************************ */

static FAT_FILE *currentFile;
static char *currentFileName = NULL;

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
		{
	       	list->items[cnt].info = type;
	       	strncpy(list->items[cnt].str, filename, 95);		
			cnt++;
		}
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
	currentFileName = strdup(filename);
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
#endif