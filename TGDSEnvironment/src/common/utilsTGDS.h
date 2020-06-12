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


#ifndef __utilstgds_h__
#define __utilstgds_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "reent.h"

#ifdef ARM9
#include "limitsTGDS.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b))?(a):(b))
#endif

#define toolchain_generic_version ((char*)"1.5");

#define nds_ewram_mask (get_ewram_size()-1)

#endif

//IPC specific: Shared Work     027FF000h 4KB    -     -    -    R/W
#define IPCRegionSize	(sint32)(4*1024)

//splitCustom logic
typedef void(*splitCustom_fn)(const char *, size_t, char * ,int indexToLeftOut, char * delim);
#define TOP_ITEMS_SPLIT (int)(10)

#endif


#ifdef __cplusplus
extern "C"{
#endif

extern size_t ucs2tombs(uint8* dst, const unsigned short* src, size_t len);
extern int		setBacklight(int flags);
//top NDS EWRAM allocated by toolchain for libraries.
extern uint32 	__lib__end__;		//start free linear memory (malloc starts here)
extern uint32 	__vma_stub_end__;	//__lib__end__ + itcm + dtcm + any other sections

//CP15 MPU 
extern void MPUSet();
extern void * _sbrk (int size);
extern void * _sbrk_r (struct _reent * reent, int size);
extern void Write32bitAddrExtArm(uint32 address, uint32 value);
extern void Write16bitAddrExtArm(uint32 address, uint16 value);
extern void Write8bitAddrExtArm(uint32 address, uint8 value);
extern volatile char 		*heap_end;
extern volatile char        *prev_heap_end;

#ifdef ARM7
extern uint32 _iwram_start;
extern uint32 _iwram_end;
extern uint32 get_iwram_start();
extern sint32 get_iwram_size();
extern uint32 get_iwram_end();
#endif


#ifdef ARM9
//misc
extern int		FS_loadFile(sint8 *filename, sint8 *buf, int size);
extern int		FS_saveFile(sint8 *filename, sint8 *buf, int size,bool force_file_creation);
extern int		FS_getFileSize(sint8 *filename);
extern void		FS_lock();
extern void		FS_unlock();
extern char *	print_ip(uint32 ip, char * bufOut);

//FileSystem utils
extern sint8 	*_FS_getFileExtension(sint8 *filename);
extern sint8 	*FS_getFileName(sint8 *filename);
extern int		FS_chdir(const sint8 *path);

//splitCustom string by delimiter implementation in C, that does not use malloc/calloc and re-uses callbacks
extern char * outSplitBuf[TOP_ITEMS_SPLIT][MAX_TGDSFILENAME_LENGTH+1];
extern int count_substr(const char *str, const char* substr, bool overlap);
extern void splitCustom(const char *str, char sep, splitCustom_fn fun, char * outBuf, int indexToLeftOut);
extern void buildPath(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim);
extern int getLastDirFromPath(char * stream, char * haystack, char * outBuf);
extern int str_split(char * stream, char * haystack, char * outBuf);
extern int inet_pton(int af, const char *src, void *dst);

//linker script hardware address setup (get map addresses from linker file)
extern uint32 	_ewram_start;
extern uint32	_ewram_end;
extern uint32 	get_ewram_start();
extern sint32 	get_ewram_size();
extern uint32 	_dtcm_start;
extern uint32 	_dtcm_end;
extern uint32 	get_dtcm_start();
extern sint32 	get_dtcm_size();
extern uint32 	_gba_start;
extern uint32 	_gba_end;
extern uint32	_gbawram_start;
extern uint32	_gbawram_end;
extern uint32 	_itcm_start;
extern uint32 	_itcm_end;
extern uint32 	get_itcm_start();
extern sint32 	get_itcm_size();
extern uint32 	_vector_start;
extern uint32 	_vector_end;

//NDS Memory Map
extern bool isValidMap(uint32 addr);
extern void separateExtension(char *str, char *ext);
#endif


extern void RenderTGDSLogoSubEngine(u8 * compressedLZSSBMP, int compressedLZSSBMPSize);
extern void RenderTGDSLogoMainEngine(u8 * compressedLZSSBMP, int compressedLZSSBMPSize);

#ifdef __cplusplus
}
#endif
