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

#ifndef posixHandleTGDS_h__
#define posixHandleTGDS_h__

#include "typedefsTGDS.h"
#include <ctype.h>
#include <stdlib.h>

#include <sys/reent.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <_ansi.h>
#include <reent.h>
#include <sys/lock.h>
#include <fcntl.h>

#ifdef ARM7
#include "xmem.h"
#endif

#ifdef ARM9
#include "ff.h"
#include "utilsTGDS.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "nds_cp15_misc.h"
#include "ipcfifoTGDS.h"
#endif

#define MAXPRINT7ARGVCOUNT (int)(20)

#ifdef ARM7
//TGDS Malloc implementation, before using them requires a call from ARM9: void initARM7Malloc(u32 ARM7MallocStartaddress, u32 memSizeBytes)
static inline u8* TGDSARM7Malloc(int size){
	return (u8*)Xmalloc((const int)size);
}

static inline u8 * TGDSARM7Calloc(int blockCount, int blockSize){
	return (u8*)Xcalloc((const int)blockSize, (const int)blockCount);
}

static inline void TGDSARM7Free(void *ptr){
	Xfree((const void *)ptr);
}

static inline u8 * TGDSARM7Realloc(void *ptr, int size){
	if(ptr != NULL){
		TGDSARM7Free(ptr);
	}
	return (u8*)TGDSARM7Malloc(size);
}

static inline u32 TGDSARM7MallocFreeMemory(){
	return (u32)XMEM_FreeMem();
}

#endif

#ifdef ARM9
//ARM9 Malloc
typedef u8* (*TGDSARM9MallocHandler)(int);
typedef u8* (*TGDSARM9CallocHandler)(int, int);
typedef void (*TGDSARM9FreeHandler)(void *);
typedef u32 (*TGDSARM9MallocFreeMemoryHandler)();

struct AllocatorInstance
{
	TGDSARM9MallocHandler 			CustomTGDSMalloc9;
	TGDSARM9CallocHandler 			CustomTGDSCalloc9;
	TGDSARM9FreeHandler				CustomTGDSFree9;
	TGDSARM9MallocFreeMemoryHandler	CustomTGDSMallocFreeMemory9;
	int memoryToAllocate;
	u32 ARM9MallocStartaddress;
};

#ifdef __cplusplus
extern "C"{
#endif
//POSIX Malloc implementation
extern int getMaxRam();

//TGDS Malloc implementation, before using them requires a call from ARM9: void initARM7Malloc(u32 ARM7MallocStartaddress, u32 memSizeBytes)
extern bool customMallocARM9;
//weak symbols : the implementation of this is project-defined
extern  __attribute__((weak))	struct AllocatorInstance * getProjectSpecificMemoryAllocatorSetup();

extern struct AllocatorInstance CustomAllocatorInstance;
extern TGDSARM9MallocHandler 			TGDSMalloc9;
extern TGDSARM9CallocHandler 			TGDSCalloc9;
extern TGDSARM9FreeHandler				TGDSFree9;
extern TGDSARM9MallocFreeMemoryHandler	TGDSMallocFreeMemory9;

extern u32 ARM9MallocBaseAddress;
extern void setTGDSARM9MallocBaseAddress(u32 address);
extern u32 getTGDSARM9MallocBaseAddress();
extern void initARM9Malloc(u32 ARM9MallocStartaddress, u32 memSizeBytes, u32 * mallocHandler, u32 * callocHandler, u32 * freeHandler, u32 * MallocFreeMemoryHandler, bool customAllocator);

#ifdef __cplusplus
}
#endif

static inline u8* TGDSARM9Malloc(int size){
	if(customMallocARM9 == false){
		return (u8*)malloc((const int)size);
	}
	else{
		return (u8*)TGDSMalloc9((const int)size);
	}
}

static inline u8 * TGDSARM9Calloc(int blockCount, int blockSize){
	if(customMallocARM9 == false){
		return (u8*)calloc(blockSize, blockCount);	//reverse order
	}
	else{
		return (u8*)TGDSCalloc9(blockCount, blockSize);	//same order
	}
}

static inline u8 * TGDSARM9Realloc(void *ptr, int size){
	if(customMallocARM9 == false){
		if(ptr != NULL){
			free(ptr);
		}
		return (u8*)malloc(size);
	}
	else{
		if(ptr != NULL){
			TGDSFree9(ptr);
		}
		return (u8*)TGDSMalloc9(size);
	}
}

static inline void TGDSARM9Free(void *ptr){
	if(customMallocARM9 == false){
		free(ptr);
	}
	else{
		TGDSFree9(ptr);
	}
}

static inline u32 TGDSARM9MallocFreeMemory(){
	if(customMallocARM9 == false){
		return (u32)getMaxRam();
	}
	else{
		return (u32)TGDSMallocFreeMemory9();
	}
}

#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM7
//ARM7 Malloc
extern u32 ARM7MallocBaseAddress;
extern void setTGDSARM7MallocBaseAddress(u32 address);
extern u32 getTGDSARM7MallocBaseAddress();

extern void printf7(char *chr, int argvCount, int * argv);	
extern void writeDebugBuffer7(char *chr, int argvCount, int * argv);
//shared
extern u8 * printfBufferShared;
extern uint8 * arm7debugBufferShared;
extern int * arm7ARGVBufferShared;

//args through ARM7 print debugger
extern int * arm7ARGVDebugBufferShared;
#endif

//Shared ARM7 Malloc
extern void initARM7Malloc(u32 ARM7MallocStartaddress, u32 memSizeBytes);

#ifdef ARM9
extern void setTGDSMemoryAllocator(struct AllocatorInstance * TGDSMemoryAllocator);
extern void printf7Setup();
extern void printf7(u8 * printfBufferShared, int * arm7ARGVBufferShared, int argvCount);
//shared
extern u8 printf7Buffer[MAX_TGDSFILENAME_LENGTH+1];
extern u8 arm7debugBuffer[MAX_TGDSFILENAME_LENGTH+1];
extern int arm7ARGVBuffer[MAXPRINT7ARGVCOUNT];

//args through ARM7 print debugger
extern int argvCount;
extern int arm7ARGVDebugBuffer[MAXPRINT7ARGVCOUNT];
#endif

#ifdef ARM9
//Requires first a call of:
//int argBuffer[MAXPRINT7ARGVCOUNT];
//memset((unsigned char *)&argBuffer[0], 0, sizeof(argBuffer));
//argBuffer[0] = 0xc0701111;
//argBuffer[1] = 0xc0702222;
//argBuffer[2] = 0xc0703333;
//writeDebugBuffer7("Write buffer 7 tests: Args: ", 3, (int*)&argBuffer[0]);

extern void printarm7DebugBuffer();
extern int _unlink(const sint8 *path);
extern void printfCoords(int x, int y, const char *fmt, ...);
#endif

extern int _vfprintf_r(struct _reent * reent, FILE *fp,const sint8 *fmt, va_list args);
extern int fork();
extern int isatty(int file);
extern int vfiprintf(FILE *fp,const sint8 *fmt, va_list list);
extern _off_t _lseek_r(struct _reent *ptr,int fd, _off_t offset, int whence );
extern int _gettimeofday(struct timeval *tv, struct timezone *tz);
extern int _end(int file);
extern _ssize_t _read_r ( struct _reent *ptr, int fd, void *buf, size_t cnt );
extern _ssize_t _write_r ( struct _reent *ptr, int fd, const void *buf, size_t cnt );
extern int _open_r ( struct _reent *ptr, const sint8 *file, int flags, int mode );
extern int _vfiprintf_r(struct _reent *reent, FILE *fp,const sint8 *fmt, va_list list);
extern int _link(const sint8 *path1, const sint8 *path2);
extern int	_stat_r ( struct _reent *_r, const char *file, struct stat *pstat );
extern void _exit (int status);
extern int _kill (pid_t pid, int sig);
extern pid_t _getpid (void);
extern int _close (int fd);
extern int close (int fd);
extern int _close_r ( struct _reent *ptr, int fd );

// High level POSIX functions:
// Alternate TGDS high level API if current posix implementation is missing or does not work. 
// Note: uses int filehandles (or StructFD index, being a TGDS internal file handle index)
extern int	open_tgds(const char * filepath, sint8 * args);
extern size_t	read_tgds(_PTR buf, size_t blocksize, size_t readsize, int fd);
extern size_t write_tgds(_PTR buf, size_t blocksize, size_t readsize, int fd);
extern int	close_tgds(int fd);
extern int	fseek_tgds(int fd, long offset, int whence);
extern long ftell_tgds(int fd);
extern int fputs_tgds(const char * s , int fd);
extern int fputc_tgds(int c, int fd);
extern int putc_tgds(int c, int fd);
extern int fprintf_tgds(int fd, const char *format, ...);
extern int fgetc_tgds(int fd);
extern char *fgets_tgds(char *s, int n, int fd);
extern int feof_tgds(int fd);
extern int ferror_tgds(int fd);
extern void TryToDefragmentMemory();

#ifdef __cplusplus
}
#endif

#ifdef ARM9
enum _flags {
    _READ = 01,     /* file open for reading */
    _WRITE = 02,    /* file open for writing */
    _UNBUF = 03,    /* file is unbuffered */
    _EOF    = 010,  /* EOF has occurred on this file */
    _ERR    = 020,  /* error occurred on this file */
};


//File Descriptor Semantics for Newlib
//Stream 0 is defined by convention to be the "standard input" stream, which newlib uses for the getc() and similar functions that don't otherwise specify an input stream. 
//Stream 1 is "standard output," the destination for printf() and puts(). 
//Stream 2 refers to "standard error," the destination conventionally reserved for messages of grave importance.

#define devoptab_fname_size 32

//devoptab_t that match file descriptors
struct devoptab_t{
   const sint8 name[devoptab_fname_size];
   int (*open_r )( struct _reent *r, const sint8 *path, int flags, int mode );
   int (*close_r )( struct _reent *r, int fd ); 
   _ssize_t (*write_r ) ( struct _reent *r, int fd, const sint8 *ptr, int len );
   _ssize_t (*read_r )( struct _reent *r, int fd, sint8 *ptr, int len );
};
#endif

static inline u8 * getarm7PrintfBuffer(){
	#ifdef ARM7
	return printfBufferShared;
	#endif
	#ifdef ARM9
	return (u8 *)&printf7Buffer[0];
	#endif
}
#endif

