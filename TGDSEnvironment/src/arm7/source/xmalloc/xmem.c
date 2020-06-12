/*

	libriyo
	
	XMEM Module
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "xmem.h"
#include "posixHandleTGDS.h"
#include "InterruptsARMCores_h.h"

// default use 128K VRAM (ARM7 Mapped), may be overriden.
unsigned int XMEMTOTALSIZE = (128*1024);

// how many bytes will each of our blocks be?
unsigned short XMEM_BLOCKSIZE = 128;

// Number of blocks to create (mem/bs)
#define XMEM_BLOCKCOUNT (XMEMTOTALSIZE/XMEM_BLOCKSIZE)

// Size of Table in bytes
#define XMEM_TABLESIZE XMEM_BLOCKCOUNT

#define XMEM_STARTBLOCK 0x01
#define XMEM_ENDBLOCK 0x02
#define XMEM_USEDBLOCK 0x04


unsigned char *xmem_table;
//XMEM_BLOCK *xmem_blocks;
unsigned char *xmem_blocks;

void XmemSetup(unsigned int size, unsigned short blocks) {
	XMEMTOTALSIZE = size;
	XMEM_BLOCKSIZE = blocks;
}

void XmemInit() {
	// init XMEM
	//	void *XT;
	
	//XT =  malloc(1024*1024);
	xmem_table = (unsigned char *)getTGDSARM7MallocBaseAddress();	
	xmem_blocks = (unsigned char *)((u8*)getTGDSARM7MallocBaseAddress() + XMEM_TABLESIZE);	//XMEM_BLOCKSIZE*XMEM_BLOCKCOUNT should not exceed the end of getTGDSARM7MallocBaseAddress()
	
	if ((xmem_table == NULL) || (xmem_blocks == NULL)) {
		
		int argBuffer[MAXPRINT7ARGVCOUNT];
		memset((unsigned char *)&argBuffer[0], 0, sizeof(argBuffer));
		writeDebugBuffer7("XMEM: Could not allocate %d bytes of main ram for XMEM...", 0, (int*)&argBuffer[0]);
		while(1==1){
			IRQVBlankWait();
		}
		if (xmem_table) //free(xmem_table);
		if (xmem_blocks) //free(xmem_blocks);
		return;
	}
	
	//free(XT);
	
	//printf("***       XMEM       *** ");
	//printf("TABLE: %8.8X (%d) ",xmem_table,XMEM_TABLESIZE);
	//printf("BLOCK: %8.8X (%d) ",xmem_blocks,XMEM_BLOCKSIZE*XMEM_BLOCKCOUNT);
	//printf("***XMEM INIT COMPLETE*** ");
	
	xmem_table[0] = XMEM_STARTBLOCK | XMEM_ENDBLOCK | XMEM_USEDBLOCK; // reserved i suppose
	
}

void *Xmalloc(const int size) {
	int i, blocks, sblock, fbr;
	bool found;
	
	//printf("XMS");
	
	// find size amount of memory to give to 
	blocks = (size / XMEM_BLOCKSIZE) + 1;
	
	// we need *blocks* consecutive free blocks of mem
	sblock = 0;
	found = false;
	fbr = 0;
	
	//printf("XM: Blocks: %d (%d b) ",blocks,size);
	
	for (i=0;(unsigned)i<XMEM_TABLESIZE;i++) {
		if ((xmem_table[i] & XMEM_USEDBLOCK) == 0) {
			if (fbr == 0) {
				sblock = i;
			}
			fbr++;
			if (fbr >= blocks) {
				// found a good enough streak
				found = true;
				break;
			}
		}
		else {
			fbr = 0;
		}
	}

	if (!found) {
		// couldnt find enough free blocks!
		//printf("XM: Couldnt Find Mem: %d/%d ",size, XMEM_FreeMem());
		
		int argBuffer[MAXPRINT7ARGVCOUNT];
		memset((unsigned char *)&argBuffer[0], 0, sizeof(argBuffer));
		writeDebugBuffer7("XM: Couldnt Find Mem:", 0, (int*)&argBuffer[0]);
		
		return NULL;
	}
	//printf("XM: SBLOCK: %d ",sblock);

	// we found enough blocks!
	// allocate them...
	xmem_table[sblock] = XMEM_STARTBLOCK | XMEM_USEDBLOCK;
	if (blocks == 1) {
		xmem_table[sblock] |= XMEM_ENDBLOCK;
	}
	else {
		for (i=sblock+1;i<(blocks+sblock);i++) {
			xmem_table[i] = XMEM_USEDBLOCK;
		}
		xmem_table[sblock+(blocks-1)] |= XMEM_ENDBLOCK;
	}
	
	//printf("XM: %d %d %8.8X ", size, sblock, ((unsigned int) xmem_blocks + (sblock*XMEM_BLOCKSIZE))); 
	return (void *) ((unsigned int) xmem_blocks + (sblock*XMEM_BLOCKSIZE));

}
void *Xcalloc(const int size, const int count) {

	void *temp;
	unsigned int *temp2;
	int sizex,i;
	
	sizex = size * count;
	
	temp = Xmalloc(sizex);
	if (temp == NULL) return NULL;
	
	temp2 = (unsigned int *) temp;
	for (i=0;i<(sizex/4);i++) *temp2++ = 0;

	return temp;

}
void Xfree(const void *ptr) {

	int block,sblock;
	
	while (1) {
		if (ptr < xmem_blocks) {
			//printf("XM: Free: NXML %8.8X ",(unsigned int)ptr);
			break;
		}
		if (ptr > (xmem_blocks+(XMEM_BLOCKCOUNT*XMEM_BLOCKSIZE))) {
			//printf("XM: Free: NXMG %8.8X ",(u32)ptr);
			break;
		}
		
		block = ((unsigned int)ptr - (unsigned int)xmem_blocks) / XMEM_BLOCKSIZE;
		
		sblock = block;
		
		// clear table from block til we find an end block
		if (xmem_table[block] & XMEM_STARTBLOCK) {
			while(1) {
				if (xmem_table[block] & XMEM_ENDBLOCK) {
					// the last block
					xmem_table[block] = 0;
					break;
				}
				xmem_table[block] = 0;
				block++;
			}
		}
		else {
			//printf("XM: Free NSB: %8.8X ",(u32)ptr);
		}
		break;
	}

}

unsigned int XMEM_FreeMem(void) {

	int i,j;

	j = 0;
	for(i=0;(unsigned)i<XMEM_TABLESIZE;i++) {
		if (xmem_table[i] == 0) j++;
	}

	return j*XMEM_BLOCKSIZE;
}
