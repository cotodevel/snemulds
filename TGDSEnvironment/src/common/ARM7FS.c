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

#include "ARM7FS.h"
#include "ipcfifoTGDS.h"
#include "typedefsTGDS.h"
#include "dmaTGDS.h"
#include "dldi.h"
#include "posixHandleTGDS.h"

#ifdef ARM7
#include "xmem.h"
#endif

#ifdef ARM9
#include "fatfslayerTGDS.h"
#endif

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
int ARM7FS_HandleMethod = 0;

/////////////////////////////////////////////////////////// [ARM7FS ARM7 User file IO start] ///////////////////////////////////////////////////////////

#ifdef ARM7
int ARM7FS_GetFileSize(void)
{
	
	return(TGDSIPC->IR_filesize);
}

//If void *OutBuffer is NULL, the internal buffer is refilled from the current read file handle
int ARM7FS_BufferReadByIRQ(void *OutBuffer, int fileOffset, int readBufferSize){
	if(readBufferSize <= 0){
		return 0;
	}
	if(fileOffset <= 0){
		fileOffset = 0;
	}
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	//Index 0 -- 4 used, do not use.
	fifomsg[5] = (uint32)TGDSIPC->IR_readbuf;
	fifomsg[6] = (uint32)readBufferSize;
	fifomsg[7] = (uint32)fileOffset;
	setARM7FSIOStatus(ARM7FS_IOSTATUS_BUSY);
	
	//Wait until ARM9 task done.
	SendFIFOWords(IR_ARM7FS_Read, (u32)fifomsg);
	while(getARM7FSIOStatus() == ARM7FS_IOSTATUS_BUSY){
		swiDelay(1);
	}
	if(OutBuffer != NULL){
		dmaTransferWord(0, (u32)((u8*)TGDSIPC->IR_readbuf), (uint32)OutBuffer, (uint32) readBufferSize);	//dmaTransferHalfWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count)
	}
	return readBufferSize;
}

//If void *InBuffer is NULL, the internal buffer is written to current write file handle
int ARM7FS_BufferSaveByIRQ(void *InBuffer, int fileOffset, int writeBufferSize){
	if(writeBufferSize <= 0){
		return 0;
	}
	if(fileOffset <= 0){
		fileOffset = 0;
	}
	
	
	if(InBuffer != NULL){
		dmaTransferWord(0, (uint32)InBuffer, (u32)TGDSIPC->IR_readbuf, (uint32)writeBufferSize);	//dmaTransferHalfWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count)
	}
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	//Index 0 -- 4 used, do not use.
	fifomsg[5] = (uint32)TGDSIPC->IR_readbuf;
	fifomsg[6] = (uint32)writeBufferSize;
	fifomsg[7] = (uint32)fileOffset;
	setARM7FSIOStatus(ARM7FS_IOSTATUS_BUSY);
	
	//Wait until ARM9 task done.
	SendFIFOWords(IR_ARM7FS_Save, (u32)fifomsg);
	while(getARM7FSIOStatus() == ARM7FS_IOSTATUS_BUSY){
		swiDelay(1);
	}
	
	return writeBufferSize;
}

/////////////////////////////////////////////////////////// [ARM7FS ARM7 User file IO end] ///////////////////////////////////////////////////////////

//Test case: reads a whole file into ARM9 and dumps it to fat fs
void performARM7MP2FSTestCase(char * ARM7fsfname, int ARM7BuffSize, u32 * writtenDebug){	//ARM7 impl.
	while(getARM7FSInitStatus() == false){	//Wait for ARM7Init
		swiDelay(1);
	}
	
	
	//ARM7 MP2 FS test case start
	u8* buffer =(u8*)TGDSARM7Malloc(ARM7BuffSize);
	TGDSIPC->IR_readbufsize = ARM7BuffSize;
	int filesize = TGDSIPC->IR_filesize;
	
	int i = 0;
	int fileOffst = 0;
	for(i = 0; i < filesize/ARM7BuffSize; i++){
		dmaFillHalfWord(0, 0, (u32)buffer, ARM7BuffSize);	//clear buffer to prevent false positive
		ARM7FS_BufferReadByIRQ(buffer, fileOffst, ARM7BuffSize);
		dmaFillHalfWord(0, 0, (u32)TGDSIPC->IR_readbuf, ARM7BuffSize);	//clear buffer to prevent false positive
		ARM7FS_BufferSaveByIRQ(buffer, fileOffst, ARM7BuffSize);
		fileOffst+= (ARM7BuffSize);
	}
	
	//left?
	int left = 0;
	if(fileOffst < filesize){
		left = filesize - fileOffst;	
		dmaFillHalfWord(0, 0, (u32)buffer, left);	//clear buffer to prevent false positive
		ARM7FS_BufferReadByIRQ(buffer, fileOffst, left);
		dmaFillHalfWord(0, 0, (u32)TGDSIPC->IR_readbuf, left);	//clear buffer to prevent false positive
		ARM7FS_BufferSaveByIRQ(buffer, fileOffst, left);
		fileOffst+=(left);
	}
	
	*writtenDebug = (u32)(TGDSIPC->IR_readbuf);
	TGDSARM7Free(buffer);
	
	//end testcase
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_IDLE);
}

void initARM7FS(char * ARM7FS_ARM9Filename, int curARM7FS_HandleMethod){	//ARM7 Impl.
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_BUSY);
	setARM7FSInitStatus(true); //finish ARM7FS 
	ARM7FS_HandleMethod = curARM7FS_HandleMethod;	//ARM7
	//ARM7 may continue with performARM7MP2FSTestCase(testcaseFilename, splitBufferSize) or async...
}
#endif

//Blocking. Will ensure ARM9 and ARM7'S ARM7FS context is discarded.
void deinitARM7FS(){
	#ifdef ARM7
	#endif
	
	#ifdef ARM9
	
	TGDSIPC->IR_readbufsize=0;
	TGDSIPC->IR_ReadOffset = 0;
	TGDSIPC->IR_WrittenOffset = 0;
	TGDSIPC->IR_filesize = 0;
	
	u8 * sharedBuffer = (u8 *)TGDSIPC->IR_readbuf;
	if(sharedBuffer != NULL){
		TGDSARM9Free(sharedBuffer);
	}
	TGDSIPC->IR_readbuf=0;
	
	
	switch(ARM7FS_HandleMethod){
		case(TGDS_ARM7FS_FILEHANDLEPOSIX):{
			if(ARM7FS_FileHandleRead != NULL){
				fclose(ARM7FS_FileHandleRead);
			}
			if(ARM7FS_FileHandleWrite != NULL){
				fclose(ARM7FS_FileHandleWrite);
			}
		}
		break;
		case(TGDS_ARM7FS_TGDSFILEDESCRIPTOR):{
			if(ARM7FS_TGDSFileDescriptorRead != NULL){
				ARM7FS_close_ARM9TGDSFD(ARM7FS_TGDSFileDescriptorRead);
			}
			if(ARM7FS_TGDSFileDescriptorWrite != NULL){
				ARM7FS_close_ARM9TGDSFD(ARM7FS_TGDSFileDescriptorWrite);
			}
		}
		break;
	}
	
	ARM7FS_HandleMethod = TGDS_ARM7FS_INVALID;
	ARM7FS_TGDSFileDescriptorRead = NULL;
	ARM7FS_TGDSFileDescriptorWrite = NULL;
	ARM7FS_ReadBuffer_ARM9TGDSFD = NULL;
	ARM7FS_SaveBuffer_ARM9TGDSFD = NULL;
	ARM7FS_close_ARM9TGDSFD = NULL;
	
	//ARM7 MP2 FS test case start.
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_BUSY);
	
	//Wait for ARM7FS de-init.
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (u32)IPC_ARM7DEINIT_ARM7FS;
	sendByteIPCIndirect(IPC_ARM7DEINIT_ARM7FS);sendIPCIRQOnly();
	while(fifomsg[0] == (u32)IPC_ARM7DEINIT_ARM7FS){
		swiDelay(1);
	}
	#endif
}

#ifdef ARM9

//If ret value: false, file not found or couldn't create the ARM7FS context
//ret value: true, success. ARM7 code is now able to use "ARM7FS ARM7 User file IO" section async. 
//When done, please close the ARM7FS through closeARM7FS(); in ARM9

//ARM7FS Example code: see https://bitbucket.org/Coto88/toolchaingenericds-ndstools/src/master/arm9/source/main.cpp, KEY_LEFT event.
bool initARM7FSPOSIX(char * inFilename, char * outFilename, int splitBufferSize, u32 * debugVar){	//ARM9. 
	if(inFilename == NULL){
		return false;
	}
	deinitARM7FS();
	
	TGDSIPC->IR_readbufsize=0;
	TGDSIPC->IR_readbuf=0;
	//setup vars
	TGDSIPC->IR_readbuf=(u32*)TGDSARM9Malloc(splitBufferSize);	//Must be EWRAM because then ARM7 can receive it into ARM7's 0x06000000 through DMA (hardware ring buffer)
	ARM7FS_FileHandleRead = fopen(inFilename, "r");
	
	if(outFilename != NULL){
		ARM7FS_FileHandleWrite = fopen(outFilename, "w+");
		int fdindexWrite = fileno(ARM7FS_FileHandleWrite);
		ARM7FS_TGDSFileDescriptorWrite = getStructFD(fdindexWrite);
	}
	
	ARM7FS_HandleMethod = TGDS_ARM7FS_FILEHANDLEPOSIX;	//ARM9
	int fdindexRead = fileno(ARM7FS_FileHandleRead);
	ARM7FS_TGDSFileDescriptorRead = getStructFD(fdindexRead);
	
	TGDSIPC->IR_ReadOffset = 0;
	TGDSIPC->IR_WrittenOffset = 0;
	TGDSIPC->IR_filesize = FS_getFileSizeFromOpenHandle(ARM7FS_FileHandleRead);
	
	if(ARM7FS_FileHandleRead != NULL){
	}
	else{
		printf("ARM9:initARM7FSPOSIX() failed");
		while(1==1){}
	}
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_BUSY);
	
	//Wait for ARM7FS init.
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (u32)inFilename;
	fifomsg[1] = (uint32)TGDSIPC->IR_filesize;
	fifomsg[2] = (uint32)splitBufferSize;
	fifomsg[3] = (uint32)IPC_ARM7INIT_ARM7FS;
	fifomsg[4] = (uint32)debugVar;
	fifomsg[5] = (uint32)0xFFFFFFFF;	//Test case disable
	fifomsg[6] = (uint32)ARM7FS_HandleMethod;
	
	sendByteIPCIndirect(IPC_ARM7INIT_ARM7FS);sendIPCIRQOnly();
	while(fifomsg[3] == IPC_ARM7INIT_ARM7FS){
		swiDelay(1);
	}
	return true;
}

char * TGDSFileHandleARM7FSName = "TGDSFileDescriptor";

//If ret value: false, file not found or couldn't create the ARM7FS context
//ret value: true, success. ARM7 code is now able to use "ARM7FS ARM7 User file IO" section async. 
//When done, please close the ARM7FS through closeARM7FS(); in ARM9

//ARM7FS Example code: see https://bitbucket.org/Coto88/toolchaingenericds-ndstools/src/master/arm9/source/main.cpp, KEY_LEFT event.
bool initARM7FSTGDSFileHandle(struct fd * TGDSFileHandleIn, struct fd * TGDSFileHandleOut, int splitBufferSize, u32 * ARM7FS_ReadBuffer_ARM9ImplementationTGDSFDCall, u32 * ARM7FS_WriteBuffer_ARM9ImplementationTGDSFDCall, u32 * ARM7FS_close_ARM9ImplementationTGDSFDCall, u32 * debugVar){	//ARM9 Impl. 
	deinitARM7FS();
	
	//setup vars
	if(TGDSFileHandleIn == NULL){
		deinitARM7FS();
		return false;
	}
	
	TGDSIPC->IR_readbuf=(u32*)TGDSARM9Malloc(splitBufferSize);	//Must be EWRAM because then ARM7 can receive it into ARM7's 0x06000000 through DMA (hardware ring buffer)
	if((u32*)TGDSIPC->IR_readbuf == (u32*)NULL){
		deinitARM7FS();
		return false;
	}
	
	ARM7FS_HandleMethod = TGDS_ARM7FS_TGDSFILEDESCRIPTOR;	//ARM9
	ARM7FS_TGDSFileDescriptorRead = TGDSFileHandleIn;
	ARM7FS_TGDSFileDescriptorWrite = TGDSFileHandleOut;
	TGDSIPC->IR_ReadOffset = 0;
	TGDSIPC->IR_WrittenOffset = 0;
	TGDSIPC->IR_filesize = FS_getFileSizeFromOpenStructFD(TGDSFileHandleIn);
	
	ARM7FS_ReadBuffer_ARM9TGDSFD = (ARM7FS_ReadBuffer_ARM9CallbackTGDSFD)ARM7FS_ReadBuffer_ARM9ImplementationTGDSFDCall;
	if(ARM7FS_TGDSFileDescriptorWrite != NULL){
		ARM7FS_SaveBuffer_ARM9TGDSFD = (ARM7FS_SaveBuffer_ARM9CallbackTGDSFD)ARM7FS_WriteBuffer_ARM9ImplementationTGDSFDCall;
	}
	ARM7FS_close_ARM9TGDSFD = (ARM7FS_close_ARM9CallbackTGDSFD)ARM7FS_close_ARM9ImplementationTGDSFDCall;
	
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_BUSY);
	
	//Wait for ARM7FS init.
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (u32)&TGDSFileHandleARM7FSName[0];
	fifomsg[1] = (uint32)TGDSIPC->IR_filesize;
	fifomsg[2] = (uint32)splitBufferSize;
	fifomsg[3] = (uint32)IPC_ARM7INIT_ARM7FS;
	fifomsg[4] = (uint32)debugVar;
	fifomsg[5] = (uint32)0xFFFFFFFF;	//Test case disable
	fifomsg[6] = (uint32)ARM7FS_HandleMethod;
	
	sendByteIPCIndirect(IPC_ARM7INIT_ARM7FS);sendIPCIRQOnly();
	while(fifomsg[3] == IPC_ARM7INIT_ARM7FS){
		swiDelay(1);
	}
	return true;
}

void closeARM7FS(){
	deinitARM7FS();
}

void performARM7MP2FSTestCasePOSIX(char * inFilename, char * outFilename, int splitBufferSize, u32 * debugVar){	//ARM9 Impl.
	deinitARM7FS();
	printf("performARM7MP2FSTestCasePOSIX() Test Case: start!");
	
	TGDSIPC->IR_readbufsize=0;
	TGDSIPC->IR_readbuf=0;
	//setup vars
	TGDSIPC->IR_readbuf=(u32*)TGDSARM9Malloc(splitBufferSize);	//Must be EWRAM because then ARM7 can receive it into ARM7's 0x06000000 through DMA (hardware ring buffer)
	ARM7FS_FileHandleRead = fopen(inFilename, "r");
	ARM7FS_FileHandleWrite = fopen(outFilename, "w+");
	
	ARM7FS_HandleMethod = TGDS_ARM7FS_FILEHANDLEPOSIX;	//ARM9
	int fdindexRead = fileno(ARM7FS_FileHandleRead);
	ARM7FS_TGDSFileDescriptorRead = getStructFD(fdindexRead);
	
	int fdindexWrite = fileno(ARM7FS_FileHandleWrite);
	ARM7FS_TGDSFileDescriptorWrite = getStructFD(fdindexWrite);
	
	TGDSIPC->IR_ReadOffset = 0;
	TGDSIPC->IR_WrittenOffset = 0;
	TGDSIPC->IR_filesize = FS_getFileSizeFromOpenHandle(ARM7FS_FileHandleRead);
	
	if((ARM7FS_FileHandleRead != NULL) && (ARM7FS_FileHandleWrite != NULL)){
		
	}
	else{
		printf("ARM9:performARM7MP2FSTestCasePOSIX() Test failed");
		while(1==1){}
	}
	//ARM7 MP2 FS test case start.
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_BUSY);
	
	//Wait for ARM7FS init.
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (u32)inFilename;
	fifomsg[1] = (uint32)TGDSIPC->IR_filesize;
	fifomsg[2] = (uint32)splitBufferSize;
	fifomsg[3] = (uint32)IPC_ARM7INIT_ARM7FS;
	fifomsg[4] = (uint32)debugVar;
	fifomsg[5] = (uint32)0xc070c070;	//Test case enable
	fifomsg[6] = (uint32)ARM7FS_HandleMethod;
	
	sendByteIPCIndirect(IPC_ARM7INIT_ARM7FS);sendIPCIRQOnly();
	while(fifomsg[3] == IPC_ARM7INIT_ARM7FS){
		swiDelay(1);
	}
	
	//Test Case: file read/writes are async (through interrupts)
	
	//wait until async file writes are done.
	while(getARM7FSTransactionStatus() == ARM7FS_TRANSACTIONSTATUS_BUSY){
		swiDelay(1);
	}
	
	fclose(ARM7FS_FileHandleRead);
	fclose(ARM7FS_FileHandleWrite);
	TGDSARM9Free((u8*)TGDSIPC->IR_readbuf);
	printf("performARM7MP2FSTestCasePOSIX() Test Case: end!");
}


void performARM7MP2FSTestCaseTGDSFileDescriptor(struct fd * TGDSFileHandleIn, struct fd * TGDSFileHandleOut, int splitBufferSize, u32 * ARM7FS_ReadBuffer_ARM9ImplementationTGDSFDCall, u32 * ARM7FS_WriteBuffer_ARM9ImplementationTGDSFDCall, u32 * ARM7FS_close_ARM9ImplementationTGDSFDCall, u32 * debugVar){	//ARM9 Impl.
	deinitARM7FS();
	printf("performARM7MP2FSTestCaseTGDSFileDescriptor()");
	printf("Test Case: start!");
	
	
	//setup vars
	if(TGDSFileHandleIn == NULL){
		deinitARM7FS();
		return false;
	}
	if(TGDSFileHandleOut == NULL){
		deinitARM7FS();
		return false;
	}
	TGDSIPC->IR_readbuf=(u32*)TGDSARM9Malloc(splitBufferSize);	//Must be EWRAM because then ARM7 can receive it into ARM7's 0x06000000 through DMA (hardware ring buffer)
	if((u32*)TGDSIPC->IR_readbuf == (u32*)NULL){
		deinitARM7FS();
		return false;
	}
	
	ARM7FS_HandleMethod = TGDS_ARM7FS_TGDSFILEDESCRIPTOR;	//ARM9
	ARM7FS_TGDSFileDescriptorRead = TGDSFileHandleIn;
	ARM7FS_TGDSFileDescriptorWrite = TGDSFileHandleOut;
	TGDSIPC->IR_ReadOffset = 0;
	TGDSIPC->IR_WrittenOffset = 0;
	TGDSIPC->IR_filesize = FS_getFileSizeFromOpenStructFD(TGDSFileHandleIn);
	
	ARM7FS_ReadBuffer_ARM9TGDSFD = (ARM7FS_ReadBuffer_ARM9CallbackTGDSFD)ARM7FS_ReadBuffer_ARM9ImplementationTGDSFDCall;
	ARM7FS_SaveBuffer_ARM9TGDSFD = (ARM7FS_SaveBuffer_ARM9CallbackTGDSFD)ARM7FS_WriteBuffer_ARM9ImplementationTGDSFDCall;
	ARM7FS_close_ARM9TGDSFD = (ARM7FS_close_ARM9CallbackTGDSFD)ARM7FS_close_ARM9ImplementationTGDSFDCall;
	
	//ARM7 MP2 FS test case start.
	setARM7FSTransactionStatus(ARM7FS_TRANSACTIONSTATUS_BUSY);
	
	//Wait for ARM7FS init.
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = (u32)&TGDSFileHandleARM7FSName[0];
	fifomsg[1] = (uint32)TGDSIPC->IR_filesize;
	fifomsg[2] = (uint32)splitBufferSize;
	fifomsg[3] = (uint32)IPC_ARM7INIT_ARM7FS;
	fifomsg[4] = (uint32)debugVar;
	fifomsg[5] = (uint32)0xc070c070;	//Test case enable
	fifomsg[6] = (uint32)ARM7FS_HandleMethod;
	
	sendByteIPCIndirect(IPC_ARM7INIT_ARM7FS);sendIPCIRQOnly();
	while(fifomsg[3] == IPC_ARM7INIT_ARM7FS){
		swiDelay(1);
	}
	
	//Test Case: file read/writes are async (through interrupts)
	
	//wait until async file writes are done.
	while(getARM7FSTransactionStatus() == ARM7FS_TRANSACTIONSTATUS_BUSY){
		swiDelay(1);
	}
	
	ARM7FS_close_ARM9TGDSFD(TGDSFileHandleIn);
	ARM7FS_close_ARM9TGDSFD(TGDSFileHandleOut);
	TGDSARM9Free((u8*)TGDSIPC->IR_readbuf);
	printf("performARM7MP2FSTestCaseTGDSFileDescriptor()");
	printf("Test Case: end!");
}

//Callbacks
ARM7FS_ReadBuffer_ARM9CallbackTGDSFD 			ARM7FS_ReadBuffer_ARM9TGDSFD;
ARM7FS_SaveBuffer_ARM9CallbackTGDSFD 			ARM7FS_SaveBuffer_ARM9TGDSFD;
ARM7FS_close_ARM9CallbackTGDSFD 				ARM7FS_close_ARM9TGDSFD;

FILE * ARM7FS_FileHandleWrite = NULL;
struct fd * ARM7FS_TGDSFileDescriptorWrite = NULL;

FILE * ARM7FS_FileHandleRead = NULL;
struct fd * ARM7FS_TGDSFileDescriptorRead = NULL;


///////////////////////////////////////////////TGDS FS POSIX Callbacks Implementation Start ///////////////////////////////////////////////
//												See ARM7FS.h, TGDS FileDescriptor Implementation)
//These callbacks are required when setting up initARM7FSPOSIX() or performARM7MP2FSTestCasePOSIX()

//ARM7 FS: Write from ARM7 to ARM9 POSIX filehandle
int ARM7FS_SaveBuffer_ARM9CallbackPOSIX(u8 * inBuffer, int fileOffset, struct fd * fdinstOut, int bufferSize){
	UINT written;
	f_lseek(fdinstOut->filPtr,(FSIZE_t)fileOffset);
	f_write(fdinstOut->filPtr, inBuffer, bufferSize, &written);
	return (int)(written);
}

//ARM7 FS: Read from ARM9 POSIX filehandle to ARM7
int ARM7FS_ReadBuffer_ARM9CallbackPOSIX(u8 * outBuffer, int fileOffset, struct fd * fdinstIn, int bufferSize){
	UINT read;
	f_lseek(fdinstIn->filPtr,(FSIZE_t)fileOffset);
	f_read(fdinstIn->filPtr, outBuffer, bufferSize, &read);
	return (int)(read);
}
///////////////////////////////////////////////TGDS FS POSIX Callbacks Implementation End ///////////////////////////////////////////////

#endif