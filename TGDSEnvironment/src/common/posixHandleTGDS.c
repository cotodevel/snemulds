/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

//Coto: this was rewritten by me so it could fit the following setup:
//The overriden stock POSIX calls are specifically targeted to newlib libc nano ARM Toolchain

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <_ansi.h>
#include <reent.h>
#include "typedefsTGDS.h"
#include "ipcfifoTGDS.h"
#include "posixHandleTGDS.h"

#ifdef ARM7
#include "xmem.h"
#endif

#ifdef ARM9
#include "dsregs_asm.h"
#include "devoptab_devices.h"
#include "errno.h"
#include "sys/stat.h"
#include "dirent.h"
#include "consoleTGDS.h"
#include "clockTGDS.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "limitsTGDS.h"
#include "dswnifi_lib.h"
#endif

//basic print ARM7 support
#ifdef ARM7
uint8 * arm7debugBufferShared = NULL;
uint8 * printfBufferShared = NULL;
int * arm7ARGVBufferShared = NULL;

//args through ARM7 print debugger
int * arm7ARGVDebugBufferShared = NULL;

void printf7(char *chr, int argvCount, int * argv){
	u8* printf7Buf = getarm7PrintfBuffer();
	if(printf7Buf != NULL){
		int strSize = strlen(chr) + 1;
		memset(printf7Buf, 0, 256+1);	//MAX_TGDSFILENAME_LENGTH
		memcpy((u8*)printf7Buf, (u8*)chr, strSize);
		printf7Buf[strSize] = 0;
		if((argvCount > 0) && (argvCount < MAXPRINT7ARGVCOUNT) && (arm7ARGVBufferShared != NULL)){
			memset((u8*)arm7ARGVBufferShared, 0, MAXPRINT7ARGVCOUNT*sizeof(int));
			int i = 0;
			for(i = 0; i < argvCount; i++){
				arm7ARGVBufferShared[i] = argv[i];
			}
		}
		
		uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
		fifomsg[0] = (uint32)printf7Buf;
		fifomsg[1] = (uint32)arm7ARGVBufferShared;
		fifomsg[2] = (uint32)argvCount;
		SendFIFOWords(TGDS_ARM7_PRINTF7, (u32)fifomsg);
	}
}

void writeDebugBuffer7(char *chr, int argvCount, int * argv){
	u8* debugBuf = arm7debugBufferShared;
	if(debugBuf != NULL){
		 //Actually used here because the ARM7 Debug buffer can be read asynchronously
		TGDSIPC->argvCount = argvCount;
		
		int strSize = strlen(chr)+1;
		memset(debugBuf, 0, 256+1);	//MAX_TGDSFILENAME_LENGTH
		memcpy((u8*)debugBuf, (u8*)chr, strSize);
		debugBuf[strSize] = 0;
		if((argvCount > 0) && (argvCount < MAXPRINT7ARGVCOUNT) && (arm7ARGVDebugBufferShared != NULL)){
			memset((u8*)arm7ARGVDebugBufferShared, 0, MAXPRINT7ARGVCOUNT*sizeof(int));
			int i = 0;
			for(i = 0; i < argvCount; i++){
				arm7ARGVDebugBufferShared[i] = argv[i];
			}
		}
		//
		//uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
		//fifomsg[0] = (uint32)debugBuf;
		//fifomsg[1] = (uint32)arm7ARGVBufferShared;
		//fifomsg[2] = (uint32)argvCount;
		//SendFIFOWords(TGDS_ARM7_PRINTF7, (u32)fifomsg);
	}
}
#endif

//ARM7 malloc support. Will depend on the current memory mapped (can be either IWRAM(very scarse), EWRAM, VRAM or maybe other)
#ifdef ARM7
u32 ARM7MallocBaseAddress = 0;
void setTGDSARM7MallocBaseAddress(u32 address){
	ARM7MallocBaseAddress = address;
}

u32 getTGDSARM7MallocBaseAddress(){
	return ARM7MallocBaseAddress;
}

//example: u32 ARM7MallocStartaddress = 0x06000000, u32 memSize = 128*1024
void initARM7Malloc(u32 ARM7MallocStartaddress, u32 memSizeBytes){
	setTGDSARM7MallocBaseAddress(ARM7MallocStartaddress);
	XMEMTOTALSIZE = memSizeBytes;
	//Init XMEM (let's see how good this one behaves...)
	u32 xmemsize = XMEMTOTALSIZE;
	xmemsize = xmemsize - (xmemsize/XMEM_BS) - 1024;
	xmemsize = xmemsize - (xmemsize%1024);
	XmemSetup(xmemsize, XMEM_BS);
	XmemInit();
}

#endif

#ifdef ARM9
//ARM7 Malloc implementation: Blocking, because several processes depend on ARM7 having a proper malloc impl.
void initARM7Malloc(u32 ARM7MallocStartaddress, u32 memSizeBytes){
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (uint32)ARM7MallocStartaddress;
	fifomsg[1] = (uint32)memSizeBytes;
	fifomsg[2] = (uint32)TGDS_ARM7_SETUPARM7MALLOC;
	SendFIFOWords(TGDS_ARM7_SETUPARM7MALLOC, (u32)fifomsg);
	while(fifomsg[2] == TGDS_ARM7_SETUPARM7MALLOC){
		swiDelay(2);
	}
}

//ARM9 Malloc implementation: Blocking, because several processes running on ARM7 may require ARM9 having a proper malloc impl.
u32 ARM9MallocBaseAddress = 0;
void setTGDSARM9MallocBaseAddress(u32 address){
	ARM9MallocBaseAddress = address;
}

u32 getTGDSARM9MallocBaseAddress(){
	return ARM9MallocBaseAddress;
}
//Global
bool customMallocARM9 = false;
TGDSARM9MallocHandler 			TGDSMalloc9;
TGDSARM9CallocHandler 			TGDSCalloc9;
TGDSARM9FreeHandler				TGDSFree9;
TGDSARM9MallocFreeMemoryHandler	TGDSMallocFreeMemory9;

//Prototype
struct AllocatorInstance CustomAllocatorInstance;

void initARM9Malloc(u32 ARM9MallocStartaddress, u32 memSizeBytes, u32 * mallocHandler, u32 * callocHandler, u32 * freeHandler, u32 * MallocFreeMemoryHandler, bool customAllocator){
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (uint32)ARM9MallocStartaddress;
	fifomsg[1] = (uint32)memSizeBytes;
	fifomsg[2] = (uint32)customAllocator;
	fifomsg[3] = (uint32)TGDS_ARM7_SETUPARM9MALLOC;
	setTGDSARM9MallocBaseAddress(ARM9MallocStartaddress);
	if(customAllocator == true){
		if(mallocHandler != NULL){
			TGDSMalloc9 = (TGDSARM9MallocHandler)mallocHandler;
		}
		if(callocHandler != NULL){
			TGDSCalloc9 = (TGDSARM9CallocHandler)callocHandler;
		}
		if(freeHandler != NULL){
			TGDSFree9 = (TGDSARM9FreeHandler)freeHandler;
		}
		if(MallocFreeMemoryHandler != NULL){
			TGDSMallocFreeMemory9 = (TGDSARM9MallocFreeMemoryHandler)MallocFreeMemoryHandler;
		}
	}
	
	SendFIFOWords(TGDS_ARM7_SETUPARM9MALLOC, (u32)fifomsg);
	while(fifomsg[3] == TGDS_ARM7_SETUPARM9MALLOC){
		swiDelay(2);
	}
}

void setTGDSMemoryAllocator(struct AllocatorInstance * TGDSMemoryAllocator){
	//Enable Default/Custom TGDS Memory Allocator
	u32 ARM9MallocStartaddress = (u32)sbrk(0);
	if(TGDSMemoryAllocator == NULL){
		customMallocARM9 = false;
		initARM9Malloc(ARM9MallocStartaddress, getMaxRam(), NULL, NULL, NULL, NULL, customMallocARM9);
	}
	else{
		customMallocARM9 = true;
		initARM9Malloc(
			(u32)TGDSMemoryAllocator->ARM9MallocStartaddress, 
			(u32)TGDSMemoryAllocator->memoryToAllocate, 
			(u32 *)TGDSMemoryAllocator->CustomTGDSMalloc9, 
			(u32 *)TGDSMemoryAllocator->CustomTGDSCalloc9, 
			(u32 *)TGDSMemoryAllocator->CustomTGDSFree9, 
			(u32 *)TGDSMemoryAllocator->CustomTGDSMallocFreeMemory9, 
			customMallocARM9
		);
	}
}
#endif


#ifdef ARM9
u8 printf7Buffer[MAX_TGDSFILENAME_LENGTH+1];
u8 arm7debugBuffer[MAX_TGDSFILENAME_LENGTH+1];
int arm7ARGVBuffer[MAXPRINT7ARGVCOUNT];

//args through ARM7 print debugger
int arm7ARGVDebugBuffer[MAXPRINT7ARGVCOUNT];

void printf7Setup(){
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (uint32)&printf7Buffer[0];
	fifomsg[1] = (uint32)&arm7debugBuffer[0];
	fifomsg[2] = (uint32)&arm7ARGVBuffer[0];
	
	//ARM7 print debugger
	fifomsg[3] = (uint32)&arm7ARGVDebugBuffer[0];
	
	SendFIFOWords(TGDS_ARM7_PRINTF7SETUP, (u32)fifomsg);
}

void printf7(u8 * printfBufferShared, int * arm7ARGVBufferShared, int argvCount){
	//argvCount can't be retrieved from here because this call comes from FIFO hardware (and with it, the argvCount value)
	coherent_user_range_by_size((uint32)printfBufferShared, strlen((char*)printfBufferShared) + 1);
	coherent_user_range_by_size((uint32)arm7ARGVBufferShared, sizeof(int) * MAXPRINT7ARGVCOUNT);
	
	char argChar[MAX_TGDSFILENAME_LENGTH+1];
	memset(argChar, 0, sizeof(argChar)); //Big note!!!! All buffers (strings, binary, etc) must be initialized like this! Otherwise you will get undefined behaviour!!
	int i = 0;
	for(i = 0; i < argvCount; i++){
		sprintf((char*)argChar, "%s %x", argChar, arm7ARGVBufferShared[i]);
	}
	argChar[strlen(argChar) + 1] = '\0';
	
	char printfTemp[MAX_TGDSFILENAME_LENGTH+1];
	memset(printfTemp, 0, sizeof(argChar));
	strcpy(printfTemp, (char*)printfBufferShared);
	strcat(printfTemp, argChar);
	printfTemp[strlen(printfTemp)+1] = '\0';
	printf(printfTemp);
}

void printarm7DebugBuffer(){
	//add args parsing
	u8 * arm7debugBufferShared = (u8 *)&arm7debugBuffer[0];
	int * arm7ARGVBufferShared = (int *)&arm7ARGVDebugBuffer[0];
	 
	
	coherent_user_range_by_size((uint32)arm7debugBufferShared, sizeof(arm7debugBuffer));
	coherent_user_range_by_size((uint32)arm7ARGVBufferShared, sizeof(int) * MAXPRINT7ARGVCOUNT);
	coherent_user_range_by_size((uint32)&TGDSIPC->argvCount, sizeof(int));
	
	int argvCount = TGDSIPC->argvCount;
	
	char argChar[MAX_TGDSFILENAME_LENGTH+1];
	memset(argChar, 0, sizeof(argChar)); //Big note!!!! All buffers (strings, binary, etc) must be initialized like this! Otherwise you will get undefined behaviour!!
	int i = 0;
	for(i = 0; i < argvCount; i++){
		sprintf((char*)argChar, "%s %x", argChar, arm7ARGVBufferShared[i]);
	}
	argChar[strlen(argChar) + 1] = '\0';
	
	char printfTemp[MAX_TGDSFILENAME_LENGTH+1];
	memset(printfTemp, 0, sizeof(argChar));
	strcpy(printfTemp, (char*)arm7debugBufferShared);
	strcat(printfTemp, argChar);
	
	strcat(printfTemp, ">6");	//TGDS Console Font Color: TGDSPrintfColor_Yellow
	
	printfTemp[strlen(printfTemp)+1] = '\0';
	printf(printfTemp); 
}

int printf(const char *fmt, ...){
	coherent_user_range_by_size((uint32)fmt, strlen(fmt)+1);
	char * stringBuf = (char*)&ConsolePrintfBuf[0];
	va_list args;
    va_start(args, fmt);
	//merge any "..." special arguments where sint8 * ftm requires then store in output printf buffer
	vsnprintf ((sint8*)stringBuf, (int)sizeof(ConsolePrintfBuf), fmt, args);
	va_end(args);
	coherent_user_range_by_size((uint32)ConsolePrintfBuf, sizeof(ConsolePrintfBuf));
	
	t_GUIZone * zoneInst = getDefaultZoneConsole();
	bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
	
	int color = (int)TGDSPrintfColor_LightGrey;	//default color
	int stringSize = (int)strlen(stringBuf);
	
	//Separate the TGDS Console font color if exists
	char cpyBuf[256+1] = {0};
	strcpy(cpyBuf, stringBuf);
	char * colorChar = (char*)&outSplitBuf[1][0];
	int matchCount = str_split((char*)cpyBuf, ">", NULL);
	if(matchCount > 0){
		color = atoi(colorChar);
		stringBuf[strlen(stringBuf) - (strlen(colorChar)+1) ] = '\0';
	}
	
	GUI_drawText(zoneInst, 0, GUI.printfy, color, stringBuf, readAndBlendFromVRAM);
	GUI.printfy += getFontHeightFromZone(zoneInst);	//skip to next line
	coherent_user_range_by_size((uint32)0x06200000, 128*1024);
	return stringSize;
}

//same as printf but having X, Y coords (relative to char width and height)
void printfCoords(int x, int y, const char *fmt, ...){
	coherent_user_range_by_size((uint32)fmt, strlen(fmt)+1);
	char * stringBuf = (char*)&ConsolePrintfBuf[0];
	va_list args;
    va_start(args, fmt);
	//merge any "..." special arguments where sint8 * ftm requires then store in output printf buffer
	vsnprintf ((sint8*)stringBuf, (int)sizeof(ConsolePrintfBuf), fmt, args);
	va_end(args);
	coherent_user_range_by_size((uint32)ConsolePrintfBuf, sizeof(ConsolePrintfBuf));
	
	int stringSize = (int)strlen(stringBuf);
	t_GUIZone * zoneInst = getDefaultZoneConsole();
	GUI.printfy = y * getFontHeightFromZone(zoneInst);
	x = x * zoneInst->font->height;
	bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
	int color = 0xff;	//white
	GUI_drawText(zoneInst, x, GUI.printfy, color, stringBuf, readAndBlendFromVRAM);
	GUI.printfy += getFontHeightFromZone(zoneInst);
	coherent_user_range_by_size((uint32)0x06200000, 128*1024);
}

int _vfprintf_r(struct _reent * reent, FILE *fp,const sint8 *fmt, va_list args){
	coherent_user_range_by_size((uint32)fmt, strlen(fmt)+1);
	char * stringBuf = (char*)&ConsolePrintfBuf[0];
	vsnprintf ((sint8*)stringBuf, (int)sizeof(ConsolePrintfBuf), fmt, args);
	coherent_user_range_by_size((uint32)ConsolePrintfBuf, sizeof(ConsolePrintfBuf));
	
	//if the fprintf points to stdout (unix shell C/C++ source code), redirect to DS printf.
	if(fp == stdout){
		int stringSize = (int)strlen(stringBuf);
		t_GUIZone * zoneInst = getDefaultZoneConsole();
		bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM, it is discarded.
		int color = 0xff;	//white
		GUI_drawText(zoneInst, 0, GUI.printfy, color, stringBuf, readAndBlendFromVRAM);
		GUI.printfy += getFontHeightFromZone(zoneInst);
		coherent_user_range_by_size((uint32)0x06200000, 128*1024);
		return stringSize;
	}
	return fputs (stringBuf, fp);
}


//Notes:
//	- 	Before you get confused, the layer order is: POSIX file operations-> newlib POSIX fd assign-> devoptab filesystem -> fatfs_layer (n file descriptors for each file op) -> fatfs driver -> dldi.
//		So we can have a portable/compatible filesystem with multiple file operations.
//
//	-	Newlib dictates to override reentrant weak functions, overriding non reentrant is undefined behaviour.

//TGDS Filesystem <- unix <- posix and/or linux implementation

int fork(){
	return -1;
}

//C++ requires this
void _exit (int status){
	
	//todo: add some exception handlers to notify ARM cores program has ran	
	while(1);
}

int _kill (pid_t pid, int sig){
	return -1;
}

pid_t _getpid (void){
	return 0;	//shall always return valid (0)
}
//C++ requires this end


//read (get struct FD index from FILE * handle)

//ok _read_r reentrant called
_ssize_t _read_r ( struct _reent *ptr, int fd, void *buf, size_t cnt ){
	//Conversion here 
	struct fd * fdinst = getStructFD(fd);
	if((fdinst != NULL) && (fdinst->devoptabFileDescriptor != NULL) && (fdinst->devoptabFileDescriptor != (struct devoptab_t *)&devoptab_stub)){
		return (_ssize_t)fdinst->devoptabFileDescriptor->read_r( NULL, fdinst->cur_entry.d_ino, buf, cnt );
	}
	return -1;
}

//ok _write_r reentrant called
//write (get struct FD index from FILE * handle)
_ssize_t _write_r ( struct _reent *ptr, int fd, const void *buf, size_t cnt ){
	//Conversion here 
	struct fd * fdinst = getStructFD(fd);
	if((fdinst != NULL) && (fdinst->devoptabFileDescriptor != NULL) && (fdinst->devoptabFileDescriptor != (struct devoptab_t *)&devoptab_stub)){
		return (_ssize_t)fdinst->devoptabFileDescriptor->write_r( NULL, fdinst->cur_entry.d_ino, buf, cnt );
	}
	return -1;
}

int _open_r ( struct _reent *ptr, const sint8 *file, int flags, int mode ){
	if(file != NULL){	
		int i = 0;
		char * token_rootpath = (char*)&outSplitBuf[0][0];
		str_split((char*)file, "/", NULL);
		sint8 token_str[MAX_TGDSFILENAME_LENGTH+1] = {0};
		sint32 countPosixFDescOpen = open_posix_filedescriptor_devices() + 1;
		/* search for "file:/" in "file:/folder1/folder.../file.test" in dotab_list[].name */
		for (i = 0; i < countPosixFDescOpen ; i++){
			if(strlen(token_rootpath) > 0){
				sprintf((sint8*)token_str,"%s%s",(char*)token_rootpath,"/");	//format properly
				if (strcmp((sint8*)token_str,devoptab_struct[i]->name) == 0){
					return devoptab_struct[i]->open_r( NULL, file, flags, mode ); //returns / allocates a new struct fd index with either DIR or FIL structure allocated
				}
			}
		}
	}
	return -1;
}

//POSIX Logic: hook devoptab descriptor into devoptab functions
int _close (int fd){
	return _close_r(NULL, fd);
}

int close (int fd){
	return _close(fd);
}

//allocates a new struct fd index with either DIR or FIL structure allocated
//not overriden, we force the call from fd_close
int _close_r ( struct _reent *ptr, int fd ){
	//Conversion here 
	struct fd * fdinst = getStructFD(fd);	
	if((fdinst != NULL) && (fdinst->devoptabFileDescriptor != NULL) && (fdinst->devoptabFileDescriptor != (struct devoptab_t *)&devoptab_stub)){
		return (_ssize_t)fdinst->devoptabFileDescriptor->close_r( NULL, fdinst->cur_entry.d_ino );
	}	
	return -1;
}

//isatty: Query whether output stream is a terminal.
int _isatty(int file){
	return  1;
}

int _end(int file)
{
	return  1;
}

//	-	All below high level posix calls for fatfs access must use the function getfatfsPath("file_or_dir_path") for file (dldi sd) handling
_off_t _lseek_r(struct _reent *ptr,int fd, _off_t offset, int whence ){	//(FileDescriptor :struct fd index)
	return fatfs_lseek(fd, offset, whence);	
}

int _link(const sint8 *path1, const sint8 *path2){
    return fatfs_link(path1, path2);
}

int _unlink(const sint8 *path){
    return f_unlink(path);
}

int	_stat_r ( struct _reent *_r, const char *file, struct stat *pstat ){
	return fatfs_stat(file, pstat);
}

int _gettimeofday(struct timeval *tv, struct timezone *tz){
	if (tv == NULL)
    {
		__set_errno (EINVAL);
		return -1;
    }
    const struct tm *tmp = getTime();
    if(tmp == NULL){
		return -1;
    }
	tv->tv_sec = (long int)getNDSRTCInSeconds();
	tv->tv_usec = 0L;
	if (tz != NULL)
    {
		const time_t timer = tv->tv_sec;
		struct tm tm;
		const long int save_timezone = 0; //__timezone;
		const long int save_daylight = 0; //__daylight;
		char *save_tzname[2];
		save_tzname[0] = ""; //__tzname[0];
		save_tzname[1] = "";//__tzname[1];
		//tmp = localtime_r (&timer, &tm);
		//tz->tz_minuteswest = 0; //__timezone / 60;
		//tz->tz_dsttime = 0; //__daylight;
		//__timezone = save_timezone;
		//__daylight = save_daylight;
		//__tzname[0] = save_tzname[0];
		//__tzname[1] = save_tzname[1];
    }
	return 0;
}

mode_t umask(mode_t mask)
{
  __set_errno (ENOSYS);
  return -1;
}

int readlink(const char *path, char *buf, size_t bufsize){
	__set_errno(ENOSYS);
	return -1;
}

int lstat(const char * path, struct stat *buf){
	return fatfs_stat((const sint8 *)path,buf);
}

int getMaxRam(){
	int maxRam = ((int)(&_ewram_end)  - (int)sbrk(0) );
	return (int)maxRam;
}

//Memory is too fragmented up to this point, causing to have VERY little memory left. 
//Luckily for us this memory hack allows dmalloc to re-arrange and free more memory for us! Also fixing malloc memory fragmentation!! WTF Dude.
void TryToDefragmentMemory(){
	int freeRam = getMaxRam();
	//I'm not kidding, this allows to de-fragment memory. Relative to how much memory we have and re-allocate it
	char * defragMalloc[1024];	//4M / 4096. DS Mem can't be higher than this
	int memSteps = (freeRam / 4096);
	int i = 0;
	for(i = 0; i < memSteps; i++){
		defragMalloc[i] = malloc(4);	//dmalloc blocks are 4K each 
	}
	for(i = 0; i < memSteps; i++){
		if(defragMalloc[i] != NULL){
			free(defragMalloc[i]);
		}
	}
}

// High level POSIX functions:
// Alternate TGDS high level API if current posix implementation is missing or does not work. 
// Note: uses int filehandles (or StructFD index, being a TGDS internal file handle index)
int	open_tgds(const char * filepath, sint8 * args){
	sint32 posix_flags = 0;	
	//"r"	read: Open file for input operations. The file must exist.
	//"w"	write: Create an empty file for output operations. If a file with the same name already exists, its contents are discarded and the file is treated as a new empty file.
	//"a"	append: Open file for output at the end of a file. Output operations always write data at the end of the file, expanding it. Repositioning operations (fseek, fsetpos, rewind) are ignored. The file is created if it does not exist.
	//"r+"	read/update: Open a file for update (both for input and output). The file must exist.
	//"w+"	write/update: Create an empty file and open it for update (both for input and output). If a file with the same name already exists its contents are discarded and the file is treated as a new empty file.
	//"a+"	append/update: Open a file for update (both for input and output) with all output operations writing data at the end of the file. Repositioning operations (fseek, fsetpos, rewind) affects the next input operations, but output operations move the position back to the end of file. The file is created if it does not exist.
	// rw / rw+ are used by some linux distros so we add that as well.
	
	//args to POSIX flags conversion:
	//http://pubs.opengroup.org/onlinepubs/9699919799/functions/fopen.html
	if ( (strcmp(args,"r") == 0) || (strcmp(args,"rb") == 0)){
		posix_flags |= O_RDONLY;
	}
	
	else if ( (strcmp(args,"w") == 0) || (strcmp(args,"wb") == 0)){
		posix_flags |= O_WRONLY|O_CREAT|O_TRUNC;
	}
	
	else if ( (strcmp(args,"a") == 0) || (strcmp(args,"ab") == 0)){
		posix_flags |= O_WRONLY|O_CREAT|O_APPEND;
	}
	
	else if ( (strcmp(args,"r+") == 0) || (strcmp(args,"rb+") == 0) || (strcmp(args,"r+b") == 0)){
		posix_flags |= O_RDWR;
	}
	
	else if ( (strcmp(args,"w+") == 0) || (strcmp(args,"wb+") == 0) || (strcmp(args,"w+b") == 0)){
		posix_flags |= O_RDWR|O_CREAT|O_TRUNC;
	}
	
	else if ( (strcmp(args,"a+") == 0) || (strcmp(args,"ab+") == 0) || (strcmp(args,"a+b") == 0)){
		posix_flags |= O_RDWR|O_CREAT|O_APPEND;
	}
	return _open_r(NULL , filepath, posix_flags,0);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
}

size_t	read_tgds(_PTR buf, size_t blocksize, size_t readsize, int fd){
	return (size_t)_read_r(NULL, fd, buf, readsize);	//returns read size from file to buffer
}


size_t write_tgds(_PTR buf, size_t blocksize, size_t readsize, int fd){
	return (size_t)_write_r(NULL, fd, buf, (sint32)readsize);	//returns written size from buf to file
}

int	close_tgds(int fd){
	return _close(fd);	//no reentrancy so we don't care. Close handle
}

int	fseek_tgds(int fd, long offset, int whence){
	return _lseek_r(NULL,fd,(_off_t)offset,whence);	//call fseek so _lseek_r (overriden) is called
}

long ftell_tgds(int fd){
	return fatfs_lseek(fd, 0, SEEK_CUR);
}

int fputs_tgds(const char * s , int fd){
	int length = strlen(s);
	int wlen = 0;
	int res = 0;
	wlen = write_tgds((void*)s, 0, (sint32)length, fd);	//returns written size from buf to file
	wlen += write_tgds("\n", 0, (sint32)1, fd);	//returns written size from buf to file 
	if (wlen == (length+1))
	{
		res = 0;
	}
	else
	{
		res = 0;	// EOF
	}
	return res;
}

int fputc_tgds(int c, int fd){
	char ch = (char) c;
	if (write_tgds(&ch, 0, (sint32)1, fd) != 1){
		c = 0;	//EOF
	}
	return c;
}

int putc_tgds(int c, int fd){
	return fputc_tgds(c,fd);
}

int fprintf_tgds(int fd, const char *format, ...){
	va_list arg;
	volatile sint8 printfbuf[MAX_TGDSFILENAME_LENGTH+1];
	va_start (arg, format);
	vsnprintf((sint8*)printfbuf, 100, format, arg);
	va_end (arg);
	return write_tgds((char*)printfbuf, 1, strlen((char*)printfbuf), fd);
}

int fgetc_tgds(int fd){
	unsigned char ch;
    return (read_tgds(&ch, 1, 1, fd) == 1) ? (int)ch : 0; // EOF
}

char *fgets_tgds(char *s, int n, int fd){
    int ch;
    char *p = s;
    while (n > 1){
		ch = fgetc_tgds(fd);
		if (ch == 0) {	// EOF
			*p = '\0';
			return (p == s) ? NULL : s;
		}
		*p++ = ch;
		if (ch == '\n'){
			break;
		}
		n--;
    }
    if (n){
		*p = '\0';
	}
    return s;
}

int feof_tgds(int fd){
	int offset = -1;
	struct fd * fdinst = getStructFD(fd);
	offset = ftell_tgds(fd);
	if(fdinst->stat.st_size <= offset){
		//stream->_flags &= ~_EOF;
	}
	else{
		//stream->_flags |= _EOF;
		offset = 0;
	}
	return offset;
}

//stub
int ferror_tgds(int fd){
	if(fd == structfd_posixInvalidFileDirOrBufferHandle){
		return 1;
	}
	return 0;
}

#endif