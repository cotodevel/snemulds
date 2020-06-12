// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>

typedef __int32 sint32;
typedef unsigned __int32 uint32;

typedef __int16 sint16;
typedef unsigned __int16 uint16;

typedef __int8 sint8;
typedef unsigned __int8 uint8;

//add vram alloc
#define vramSize (sint32)(128*1024)
#define vramBlockA (uint32)(0xa)
#define vramBlockB (uint32)(0xb)
#define vramBlockC (uint32)(0xc)
#define vramBlockD (uint32)(0xd)

#define HeapSize (uint32)(128*1024)
#define HeapBlock (uint32)(0xe)

//vram linear memory allocator 
sint32 vramABlockOfst	=	0;	//offset pointer to free memory, user alloced memory is (baseAddr + (sizeAlloced - vramBlockPtr))
sint32 vramBBlockOfst	=	0;
sint32 vramCBlockOfst	=	0;
sint32 vramDBlockOfst	=	0;

sint32 HeapBlockOfst	=	0;

//if ret ptr == NULL, invalid operation  not enough space
uint32 * vramHeapAlloc(uint32 vramBlock,uint32 StartAddr,int size){
	uint32 * BlockAssign = NULL;
	bool isVram = false;
	switch(vramBlock){
		case(vramBlockA):{
			BlockAssign = (uint32 *)&vramABlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockB):{
			BlockAssign = (uint32 *)&vramBBlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockC):{
			BlockAssign = (uint32 *)&vramCBlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockD):{
			BlockAssign = (uint32 *)&vramDBlockOfst;
			isVram = true;
		}
		break;
		case(HeapBlock):{
			BlockAssign = (uint32 *)&HeapBlockOfst;
		}
		break;
	}
	if(BlockAssign == NULL){
		return NULL;
	}
	sint32 heapDetected = (isVram == true) ? vramSize : HeapSize;
	if((StartAddr + (int)*BlockAssign + size) <= (StartAddr+heapDetected)){
		//memset((uint8*)(StartAddr + (int)*BlockAssign) , 0, size);
		*BlockAssign = (uint32)((int)*BlockAssign + size);
	}
	else{
		return NULL;
	}
	uint32 AllocBuf = (StartAddr + ((int)*BlockAssign - size));
	if(AllocBuf < StartAddr){
		AllocBuf = StartAddr;
	}
	return (uint32*)AllocBuf;
}

//if ret ptr == NULL, invalid operation  not enough space
uint32 * vramHeapFree(uint32 vramBlock,uint32 StartAddr,int size){
	uint32 * BlockAssign = NULL;
	bool isVram = false;
	switch(vramBlock){
		case(vramBlockA):{
			BlockAssign = (uint32 *)&vramABlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockB):{
			BlockAssign = (uint32 *)&vramBBlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockC):{
			BlockAssign = (uint32 *)&vramCBlockOfst;
			isVram = true;
		}
		break;
		case(vramBlockD):{
			BlockAssign = (uint32 *)&vramDBlockOfst;
			isVram = true;
		}
		break;
		case(HeapBlock):{
			BlockAssign = (uint32 *)&HeapBlockOfst;
		}
		break;
	}
	if(BlockAssign == NULL){
		return NULL;
	}
	if(((StartAddr + (int)*BlockAssign) - size) >= (StartAddr)){
		*BlockAssign = (uint32)((int)*BlockAssign - size);
	}
	else{
		return NULL;
	}
	return (uint32*)(StartAddr + ((int)*BlockAssign));
}

int _tmain(int argc, _TCHAR* argv[])
{
	uint32 startLinearVramAddr = 0x06000000;
	uint32 startLinearHeapAddr = 0x02040100;	//fake ewram address

	sint32 size = 1024 * 32;
	printf("vram-alloc%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapAlloc(vramBlockD,startLinearVramAddr,size));	//0x06000000
	printf("vram-alloc%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapAlloc(vramBlockD,startLinearVramAddr,size));	//0x06008000
	printf("vram-alloc%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapAlloc(vramBlockD,startLinearVramAddr,size));	//0x06010000
	printf("vram-alloc%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapAlloc(vramBlockD,startLinearVramAddr,size));	//0x06018000

	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0x02040100
	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0x02048100
	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0x02050100
	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0x02058100
	
	printf("heap-alloc%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapAlloc(HeapBlock,startLinearHeapAddr,size));	//0 (invalid)
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0x06018000
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0x02058100
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0x06010000
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0x02050100
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0x06008000
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0x02048100
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0x06000000
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0x02040100
	
	printf("vram-free%x:%x \n",startLinearVramAddr,(uint16 *)vramHeapFree(vramBlockD,startLinearVramAddr,size));	//0 (invalid)
	printf("heap-free%x:%x \n",startLinearHeapAddr,(uint16 *)vramHeapFree(HeapBlock,startLinearHeapAddr,size));		//0 (invalid)
	
	while(1==1){}
	return 0;
}

