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

//Software BIOS modules by either replacement (bios logic replacement) or native BIOS support (use of the hardware bios rom vector, provided by ninty)

/////////////////////////////////////////////////// Shared BIOS ARM7/9 /////////////////////////////////////////////////////////////////
#include "biosTGDS.h"
#include "global_settings.h"
#include "dldi.h"
#include "dmaTGDS.h"
#include "InterruptsARMCores_h.h"
#include "posixHandleTGDS.h"

//NDS BIOS Routines C code

//problem kaputt docs say DS uses a rounded 4 byte copy, be it a fillvalue to dest or direct copy from src to dest, by size.
//Dont optimize as vram is 16 or 32bit, optimization can end up in 8bit writes.
//writes either a COPY_FIXED_SOURCE value = [r0], or plain copy from source to destination
void swiFastCopy(uint32 * source, uint32 * dest, int flags){
	#ifdef ARM9
	coherent_user_range_by_size((uint32)source, (int)((flags<<2)&0x1fffff));
	coherent_user_range_by_size((uint32)dest, (int)((flags<<2)&0x1fffff));
	#endif
	
	if(flags & COPY_FIXED_SOURCE){
		dmaFillWord(3, (uint32)(*(uint32*)source),(uint32)dest, (uint32)(((flags<<2)&0x1fffff)));
	}
	else //if(flags & COPY_SRCDEST_DMA)	//if not, just perform src to dest DMA copy
	{
		dmaTransferWord(3, (uint32)source, (uint32)dest, (uint32) (((flags<<2)&0x1fffff)) );
	}
}

#ifdef ARM9
//Once called, you consume the struct LZSSContext and then call free(struct LZSSContext.bufferSource)
struct LZSSContext LZS_DecodeFromBuffer(unsigned char *pak_buffer, unsigned int   pak_len){
	struct LZSSContext LZSSCtx;
	unsigned char *raw_buffer;
	unsigned int   raw_len, header;
	//printf("- decompressing from buffer... ");

	header = *pak_buffer;
	if (header != CMD_CODE_10) {
		//printf("ERROR: file is not LZSS encoded!");
		LZSSCtx.bufferSource = NULL;
		LZSSCtx.bufferSize = -1;
		return LZSSCtx;
	}

	raw_len = *(unsigned int *)pak_buffer >> 8;
	raw_buffer = (unsigned char *) TGDSARM9Malloc(raw_len * sizeof(char));

	swiDecompressLZSSWram((void *)pak_buffer, (void *)raw_buffer);

	LZSSCtx.bufferSource = raw_buffer;
	LZSSCtx.bufferSize = raw_len;

	//printf("LZS_Decode() end.");
	return LZSSCtx;
}
#endif

//Services Implementation

//These services run when a TGDS binary starts
#ifdef ARM7
void handleARM7InitSVC(){

}
#endif

#ifdef ARM9
void handleARM9InitSVC(){

}
#endif

//Services

#ifdef ARM7
bool isArm7ClosedLid = false;
inline __attribute__((always_inline)) 
void handleARM7SVC(){
	
	//Lid Closing + backlight events (ARM7)
	if(isArm7ClosedLid == false){
		if((REG_KEYXY & KEY_HINGE) == KEY_HINGE){
			SendFIFOWords(FIFO_IRQ_LIDHASCLOSED_SIGNAL, 0);
			screenLidHasClosedhandlerUser();
			isArm7ClosedLid = true;
		}
	}
	
	//Handles Sender FIFO overflows
	if(REG_IPC_FIFO_CR & IPC_FIFO_ERROR){
		REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	}
}

//ARM7 DLDI implementation
#ifdef ARM7_DLDI
void IPCIRQHandleDLDI(){
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	switch(fifomsg[7]){
		case((uint32)TGDS_DLDI_ARM7_READ):{
			struct DLDI_INTERFACE * dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
			u32 sector = (u32)fifomsg[0];
			u32 numSectors = (u32)fifomsg[1];
			u32 buffer = (u32)fifomsg[2];
			dldiInterface->ioInterface.readSectors(sector, numSectors, buffer);
			fifomsg[7] = fifomsg[2] = fifomsg[1] = fifomsg[0] = 0;
		}
		break;
	}


	switch(fifomsg[8]){
		case((uint32)TGDS_DLDI_ARM7_WRITE):{
			struct DLDI_INTERFACE * dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
			u32 sector = (u32)fifomsg[3];
			u32 numSectors = (u32)fifomsg[4];
			u32 buffer = (u32)fifomsg[5];
			dldiInterface->ioInterface.writeSectors(sector, numSectors, buffer);
			fifomsg[8] = fifomsg[5] = fifomsg[4] = fifomsg[3] = 0;
		}
		break;
	}	
}
#endif

#endif

#ifdef ARM9
__attribute__((section(".itcm")))
inline __attribute__((always_inline)) 
void handleARM9SVC(){
	//Handles Sender FIFO overflows
	if(REG_IPC_FIFO_CR & IPC_FIFO_ERROR){
		REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	}
}

#endif


