/*
	libriyo
	
	xmem.h
	
	I developed the XMEM system because I was having some serious overlapping
	issues and other problems with standard malloc/calloc/etc on the DS.
	
	This system simply uses a small table in ram to determine whats free and whats not.
	Its a little slower than the normal functions, but, at least its guaranteed to alloc/free
	memory correctly.
	
	It uses one call to the "real" malloc to allocate its full RAM space.
	
	Basic setup:
	XmemSetup(2048*1024,256);
	XmemInit();
	
	Would give 2MB of XMEM allocatable in 256 byte chunks. Make sure the second arg is
	a power of 2, and the first arg is divisable by your second arg.	
*/

//inherits what is defined in: ipcfifoTGDS.h
#ifndef __xmem_h__
#define __xmem_h__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define XMEM_BS 256

#ifdef __cplusplus
extern "C" {
#endif

// setup the amount of ram for the xmem system
extern void XmemSetup(unsigned int size, unsigned short blocks);

// starts the XMEM system
extern void XmemInit(unsigned int mallocLinearMemoryStart, unsigned int mallocLinearMemorySize);

// allocate ram from XMEM ram
// NULL if error/insufficient
extern void * Xmalloc(const int size);

// allocate ram from XMEM ram and 0 that ram
// NULL if error/insufficient
extern void * Xcalloc(const int size, const int count);

// free a block of XMEM
extern void Xfree(const void *ptr);

// return the amount of free mem
extern unsigned int XMEM_FreeMem(void);

extern unsigned int XMEMTOTALSIZE;

// Number of blocks to create (mem/bs)
extern unsigned int XMEM_BLOCKCOUNT;

// Size of Table in bytes
extern unsigned int XMEM_TABLESIZE;


#ifdef __cplusplus
}
#endif

#endif
