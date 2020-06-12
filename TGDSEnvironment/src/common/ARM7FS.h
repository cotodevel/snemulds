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

#ifndef __ARM7FS_h__
#define __ARM7FS_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "ipcfifoTGDS.h"

#ifdef ARM9
#include "fatfslayerTGDS.h"
#endif

#define IR_ARM7FS_Read (u32)(0xffffaaa0)		
#define IR_ARM7FS_Save (u32)(0xffffaab0)		

//TGDS ARM7FS Handle Method: Internal. Set at ARM7 and ARM9
#define TGDS_ARM7FS_INVALID (int)(-1)
#define TGDS_ARM7FS_FILEHANDLEPOSIX (int)(1)
#define TGDS_ARM7FS_TGDSFILEDESCRIPTOR (int)(2)

#ifdef ARM9
//typedef int (*ARM7FS_ReadBuffer_ARM9CallbackTGDSFD)(u8 * outBuffer, int fileOffset, struct fd * fdinstIn, int bufferSize);
typedef int (*ARM7FS_ReadBuffer_ARM9CallbackTGDSFD)(u8 *, int, struct fd *, int);

//typedef int (*ARM7FS_SaveBuffer_ARM9CallbackTGDSFD)(u8 * inBuffer, int fileOffset, struct fd * fdinstOut, int bufferSize);
typedef int (*ARM7FS_SaveBuffer_ARM9CallbackTGDSFD)(u8 *, int, struct fd *, int);

//typedef int (*ARM7FS_close_ARM9CallbackTGDSFD)(u8 * inBuffer, int fileOffset, struct fd * fdinstOut, int bufferSize);
typedef int (*ARM7FS_close_ARM9CallbackTGDSFD)(struct fd *);

#endif

static inline bool getARM7FSInitStatus(){
	
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->initStatus, sizeof(TGDSIPC->initStatus));
	#endif
	return TGDSIPC->initStatus;
}

static inline void setARM7FSInitStatus(bool ARM7FSInitStatus){
	
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->initStatus, sizeof(TGDSIPC->initStatus));
	#endif
	TGDSIPC->initStatus = ARM7FSInitStatus;
}

//IO
static inline int getARM7FSIOStatus(){
	
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->ARM7FSStatus, sizeof(TGDSIPC->ARM7FSStatus));
	#endif
	return TGDSIPC->ARM7FSStatus;
}

static inline void setARM7FSIOStatus(int ARM7FSStatus){
	
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->ARM7FSStatus, sizeof(TGDSIPC->ARM7FSStatus));
	#endif
	TGDSIPC->ARM7FSStatus = ARM7FSStatus;
}

//Global Transaction Status
static inline int getARM7FSTransactionStatus(){
	
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->ARM7FSTransactionStatus, sizeof(TGDSIPC->ARM7FSTransactionStatus));
	#endif
	return TGDSIPC->ARM7FSTransactionStatus;
}

static inline void setARM7FSTransactionStatus(int ARM7FSTransactionStatus){
	
	#ifdef ARM9
	coherent_user_range_by_size((uint32)&TGDSIPC->ARM7FSTransactionStatus, sizeof(TGDSIPC->ARM7FSTransactionStatus));
	#endif
	TGDSIPC->ARM7FSTransactionStatus = ARM7FSTransactionStatus;
}

#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM7
//Internal impl.
extern int ARM7FS_GetFileSize(void);
extern int ARM7FS_BufferReadByIRQ(void *OutBuffer, int fileOffset, int readBufferSize);
extern int ARM7FS_BufferSaveByIRQ(void *InBuffer, int fileOffset, int writeBufferSize);
extern void initARM7FS(char * ARM7FS_ARM9Filename, int curARM7FS_HandleMethod);	//ARM7

//Test case
extern void performARM7MP2FSTestCase(char * ARM7fsfname, int ARM7BuffSize, u32 * writtenDebug);	//ARM7
#endif

#ifdef ARM9
//Internal impl.
extern char * TGDSFileHandleARM7FSName;
extern void closeARM7FS();	//Must use this!! Instead deinitARM7FS(); directly when closing an ARM7FS session

//TGDS FS POSIX Implementation:
	extern bool initARM7FSPOSIX(char * inFilename, char * outFilename, int splitBufferSize, u32 * debugVar);	//ARM9.
	extern int ARM7FS_ReadBuffer_ARM9CallbackPOSIX(u8 * outBuffer, int fileOffset, struct fd * fdinstIn, int bufferSize);
	extern int ARM7FS_SaveBuffer_ARM9CallbackPOSIX(u8 * inBuffer, int fileOffset, struct fd * fdinstOut, int bufferSize);
	extern FILE * ARM7FS_FileHandleWrite;
	extern FILE * ARM7FS_FileHandleRead;

//TGDS FileDescriptor Implementation:
	extern bool initARM7FSTGDSFileHandle(struct fd * TGDSFileHandleIn, struct fd * TGDSFileHandleOut, int splitBufferSize, u32 * ARM7FS_ReadBuffer_ARM9ImplementationTGDSFDCall, u32 * ARM7FS_WriteBuffer_ARM9ImplementationTGDSFDCall, u32 * ARM7FS_close_ARM9ImplementationTGDSFDCall, u32 * debugVar);
	extern struct fd * ARM7FS_TGDSFileDescriptorWrite;	//same as FILE * ARM7FS_FileHandleWrite but the internal TGDS file descriptor handle
	extern struct fd * ARM7FS_TGDSFileDescriptorRead;	//same as FILE * ARM7FS_FileHandleRead but the internal TGDS file descriptor handle
	//Callbacks
	extern ARM7FS_ReadBuffer_ARM9CallbackTGDSFD 			ARM7FS_ReadBuffer_ARM9TGDSFD;
	extern ARM7FS_SaveBuffer_ARM9CallbackTGDSFD 			ARM7FS_SaveBuffer_ARM9TGDSFD;
	extern ARM7FS_close_ARM9CallbackTGDSFD 					ARM7FS_close_ARM9TGDSFD;
	
	
	//TGDS FileDescriptor implementation; weak symbols: The implementation of this is project-defined
	extern  __attribute__((weak))	int ARM7FS_ReadBuffer_ARM9ImplementationTGDSFD(u8 * outBuffer, int fileOffset, struct fd * fdinstIn, int bufferSize);
	extern  __attribute__((weak))	int ARM7FS_WriteBuffer_ARM9ImplementationTGDSFD(u8 * inBuffer, int fileOffset, struct fd * fdinstOut, int bufferSize);
	extern  __attribute__((weak))	int ARM7FS_close_ARM9ImplementationTGDSFD(struct fd * fdinstOut);
	
//Test Cases
//Posix
extern void performARM7MP2FSTestCasePOSIX(char * inFilename, char * outFilename, int splitBufferSize, u32 * debugVar);	//ARM9 
//TGDS FileDescriptor
extern void performARM7MP2FSTestCaseTGDSFileDescriptor(struct fd * TGDSFileHandleIn, struct fd * TGDSFileHandleOut, int splitBufferSize, u32 * ARM7FS_ReadBuffer_ARM9ImplementationTGDSFDCall, u32 * ARM7FS_WriteBuffer_ARM9ImplementationTGDSFDCall, u32 * ARM7FS_close_ARM9ImplementationTGDSFDCall, u32 * debugVar);

#endif

extern void deinitARM7FS();
extern int ARM7FS_HandleMethod;

#ifdef __cplusplus
}
#endif