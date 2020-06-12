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

#include "utilsTGDS.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "spifwTGDS.h"
#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include "reent.h"	//sbrk
#include "posixHandleTGDS.h"

#ifdef ARM9
#include "dswnifi_lib.h"
#endif

uint32 get_arm7_start_address(){
	return (uint32)TGDSIPC->DSHeader.arm7entryaddress;
}

uint32 get_arm7_end_address(){
	return (uint32)((u8*)TGDSIPC->DSHeader.arm7entryaddress + TGDSIPC->DSHeader.arm7size);
}

sint32 get_arm7_ext_size(){
	return (sint32)((uint8*)(uint32*)get_arm7_end_address() - (sint32)(get_arm7_start_address()));
}


uint32 get_arm9_start_address(){
	
	return (uint32)TGDSIPC->DSHeader.arm9entryaddress;
}

uint32 get_arm9_end_address(){
	
	return (uint32)((u8*)TGDSIPC->DSHeader.arm9entryaddress + TGDSIPC->DSHeader.arm9size);
}

sint32 get_arm9_ext_size(){
	return (sint32)((uint8*)(uint32*)get_arm9_end_address() - (sint32)(get_arm9_start_address()));
}

#ifdef ARM7
uint32 get_iwram_start(){
	return (uint32)(&_iwram_start);
}

sint32 get_iwram_size(){
	return (sint32)((uint8*)(uint32*)&_iwram_end - (sint32)(&_iwram_start));
}

uint32 get_iwram_end(){
	return (uint32)(&_iwram_end);
}
#endif

//Physical Memory Start: [ARM7/9 bin start ~ ARM7/9 bin end, 4 bytes alignment, get_lma_libend() <---- ] ------------------------------------------------------------ get_lma_wramend()
inline __attribute__((always_inline)) 
uint32 get_lma_libend(){
	u32 wram_start = (u32)&__lib__end__;
	return (uint32)((wram_start + (4 - 1)) & -4);  // Round up to 4-byte boundary // linear memory top (start)
}

//Physical Memory Start: [ARM7/9 bin start ~ ARM7/9 bin end, 4 bytes alignment, get_lma_libend()] ------------------------------------------------------------ get_lma_wramend() <----
inline __attribute__((always_inline)) 
uint32 get_lma_wramend(){
	#ifdef ARM7
	extern uint32 sp_USR;	//the farthest stack from the end memory is our free memory (in ARM7, shared stacks)
	u32 wram_end = ((uint32)&sp_USR - 0x400);
	return (uint32)((wram_end + (4 - 1)) & -4);
	#endif
	#ifdef ARM9
	u32 wram_end = (u32)&_ewram_end;
	return (uint32)((wram_end + (4 - 1)) & -4);  // Round up to 4-byte boundary // EWRAM has no stacks shared so we use the end memory
	#endif
}

#ifdef ARM9
inline __attribute__((always_inline)) 
uint32 get_ewram_start(){
	return (uint32)(&_ewram_start);
}

inline __attribute__((always_inline)) 
sint32 get_ewram_size(){
	return (sint32)((uint8*)(uint32*)get_lma_wramend() - (sint32)(&_ewram_start));
}

inline __attribute__((always_inline)) 
uint32 get_itcm_start(){
	return (uint32)(&_itcm_start);
}

inline __attribute__((always_inline)) 
sint32 get_itcm_size(){
	return (sint32)((uint8*)(uint32*)get_lma_wramend() - (sint32)(&_itcm_start));
}

inline __attribute__((always_inline)) 
uint32 get_dtcm_start(){
	return (uint32)(&_dtcm_start);
}

inline __attribute__((always_inline)) 
sint32 get_dtcm_size(){
	return (sint32)((uint8*)(uint32*)&_dtcm_end - (sint32)(&_dtcm_start));
}

#endif


/* ------------------------------------------------------------------------- */
/*!Increase program data space
   This is a minimal implementation.  Assuming the heap is growing upwards
   from __heap_start towards __heap_end.
   See linker file for definition.
   @param[in] incr  The number of bytes to increment the stack by.
   @return  A pointer to the start of the new block of memory                */
/* ------------------------------------------------------------------------- */

volatile char 		*heap_end = NULL;		/* Previous end of heap or 0 if none */
volatile char        *prev_heap_end = NULL;

inline __attribute__((always_inline)) 
void * _sbrk (int  incr)
{
	if (heap_end == NULL) {
		heap_end = (sint8*)get_lma_libend();			/* Initialize first time round */
	}

	prev_heap_end  = heap_end;
	heap_end      += incr;
	//check
	if( heap_end < (char*)get_lma_wramend()) {

	} else {
		#ifdef ARM9
		errno = ENOMEM;
		#endif
		return (char*)-1;
	}
	return (void *) prev_heap_end;

}	/* _sbrk () */

inline __attribute__((always_inline)) 
void * _sbrk_r (struct _reent * reent, int size){
	return _sbrk (size);
}

inline __attribute__((always_inline)) 
void Write8bitAddrExtArm(uint32 address, uint8 value){
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = address;
	fifomsg[1] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_8, (uint32)fifomsg);
}

inline __attribute__((always_inline)) 
void Write16bitAddrExtArm(uint32 address, uint16 value){
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = address;
	fifomsg[1] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_16, (uint32)fifomsg);
}

inline __attribute__((always_inline)) 
void Write32bitAddrExtArm(uint32 address, uint32 value){
	
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[0] = address;
	fifomsg[1] = (uint32)value;
	SendFIFOWords(WRITE_EXTARM_32, (uint32)fifomsg);
}

//NDS Memory Map (valid):
//todo: detect valid maps according to MPU settings
#ifdef ARM9
__attribute__((section(".itcm")))
inline __attribute__((always_inline)) 
bool isValidMap(uint32 addr){
	if(
		#ifdef ARM9
		((addr >= (uint32)0x06000000) && (addr < (uint32)0x06020000))	//NDS VRAM BG Engine Region protected	0x06000000 - 0x0601ffff
		||
		((addr >= (uint32)0x06020000) && (addr < (uint32)0x06040000))	//NDS VRAM BG Engine Region protected	0x06020000 - 0x0603ffff
		||
		((addr >= (uint32)0x06040000) && (addr < (uint32)0x06060000))	//NDS VRAM BG Engine Region protected	0x06040000 - 0x0605ffff
		||
		((addr >= (uint32)0x06060000) && (addr < (uint32)0x06080000))	//NDS VRAM BG Engine Region protected	0x06060000 - 0x0607ffff
		||
		((addr >= (uint32)0x06200000) && (addr < (uint32)(0x06200000 + 0x20000)))	//NDS VRAM Engine Region protected	//theoretical map
		||
		((addr >= (uint32)0x06400000) && (addr < (uint32)(0x06400000 + 0x14000)))	//NDS VRAM Engine Region protected	// actual map
		||
		((addr >= (uint32)0x06600000) && (addr < (uint32)(0x06600000 + 0x20000)))	//NDS VRAM Engine Region protected	// theoretical map
		||
		((addr >= (uint32)0x06800000) && (addr < (uint32)(0x06800000 + (656 * 1024) )))	//NDS VRAM Engine Region protected	// actual map
		||
		((addr >= (uint32)0x07000000) && (addr < (uint32)(0x07000000 + (2 * 1024) )))	//NDS VRAM OAM Region protected	// theoretical map?
		||
		((addr >= (uint32)get_ewram_start()) && (addr <= (uint32)(get_ewram_start() + get_ewram_size())))	//NDS EWRAM Region protected
		||
		((addr >= (uint32)(get_itcm_start())) && (addr <= (uint32)(get_itcm_start()+get_itcm_size())))	//NDS ITCM Region protected
		||
		((addr >= (uint32)(get_dtcm_start())) && (addr <= (uint32)(get_dtcm_start()+get_dtcm_size())))	//NDS DTCM Region protected
		||
		((addr >= (uint32)(0x05000000)) && (addr <= (uint32)(0x05000000 + 2*1024)))	//NDS Palette Region protected
		||
		( ((WRAM_CR & WRAM_32KARM9_0KARM7) == WRAM_32KARM9_0KARM7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 32*1024)) )	//NDS Shared WRAM 32K ARM9 / 0K ARM7 Region protected
		||
		( ((WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF9) == WRAM_16KARM9_16KARM7FIRSTHALF9) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 16*1024)) )	//NDS Shared WRAM 16K ARM9 (first half) / 16K ARM7 Region protected
		||
		( ((WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF7) == WRAM_16KARM9_16KARM7FIRSTHALF7) && (addr >= (uint32)(0x03000000 + (16*1024))) && (addr <= (uint32)(0x03000000 + (32*1024) )) )	//NDS Shared WRAM 16K ARM9 (second half) / 16K ARM7 Region protected
		#endif
		#ifdef ARM7
		((addr >= (uint32)get_iwram_start()) && (addr <= (uint32)(get_iwram_start() + get_iwram_size())))	//NDS IWRAM Region protected
		||
		( ((WRAM_CR & WRAM_0KARM9_32KARM7) == WRAM_0KARM9_32KARM7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 32*1024)) )	//NDS Shared WRAM 0K ARM9 / 32K ARM7 Region protected
		||
		( ((WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF7) == WRAM_16KARM9_16KARM7FIRSTHALF7) && (addr >= (uint32)(0x03000000)) && (addr <= (uint32)(0x03000000 + 16*1024)) )	//NDS Shared WRAM 16K ARM9 / 16K ARM7 (first half) Region protected
		||
		( ((WRAM_CR & WRAM_16KARM9_16KARM7FIRSTHALF9) == WRAM_16KARM9_16KARM7FIRSTHALF9) && (addr >= (uint32)(0x03000000 + (16*1024))) && (addr <= (uint32)(0x03000000 + (32*1024) )) )	//NDS Shared WRAM 16K ARM9 / 16K ARM7 (second half) Region 
		#endif
		||
		((addr >= (uint32)(0x04000000)) && (addr <= (uint32)(0x04000000 + 4*1024)))	//NDS IO Region protected
		||
		((addr >= (uint32)(0x08000000)) && (addr <= (uint32)(0x08000000 + (32*1024*1024))))	//GBA ROM MAP (allows to read GBA carts over GDB)
	){
		return true;
	}
	return false;
}
#endif


//usage:
//setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);	//both lit screens
//setBacklight(POWMAN_BACKLIGHT_TOP_BIT);								//top lit screen
//setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);							//bottom lit screen
//setBacklight(0);														//non-lit both LCD screens (poweroff)
	
int	setBacklight(int flags){
	#ifdef ARM7
		int PMBitsRead = PowerManagementDeviceRead((int)POWMAN_READ_BIT);
		PMBitsRead &= ~(POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT);
		PMBitsRead |= (int)(flags & (POWMAN_BACKLIGHT_BOTTOM_BIT|POWMAN_BACKLIGHT_TOP_BIT));
		PowerManagementDeviceWrite(POWMAN_WRITE_BIT, (int)PMBitsRead);
	#endif
	
	#ifdef ARM9
		
		uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
		fifomsg[0] = (uint32)FIFO_SCREENPOWER_WRITE;
		fifomsg[1] = (uint32)(flags);
		SendFIFOWords(FIFO_POWERMGMT_WRITE, (uint32)fifomsg);
	#endif
	return 0;
}


size_t ucs2tombs(uint8* dst, const unsigned short* src, size_t len) {
	size_t i=0,j=0;
	for (;src[i];i++){
		if(src[i] <= 0x007f){
			if(!dst)j++;else{
				if(len-j<2)break;
				dst[j++] = ((src[i] & 0x007f)      );
			}
		}else if(src[i] <= 0x07ff){
			if(!dst)j+=2;else{
				if(len-j<3)break;
				dst[j++] = ((src[i] & 0x07c0) >>  6) | 0xc0;
				dst[j++] = ((src[i] & 0x003f)      ) | 0x80;
			}
		}else if((src[i] & 0xdc00) == 0xd800 && (src[i+1] & 0xdc00) == 0xdc00){
			if(!dst)j+=4;else{
				unsigned short z = (src[i]&0x3ff)+0x40;
				if(len-j<5)break;
				dst[j++] = ((z      & 0x0300) >>  8) | 0xf0;   //2
				dst[j++] = ((z      & 0x00fc) >>  2) | 0x80;   //6
				dst[j++] = ((z      & 0x0003) <<  4)           //2
					  | ((src[i+1] & 0x03c0) >>  6) | 0x80; //4
				dst[j++] = ((src[i+1] & 0x003f)      ) | 0x80; //6
			}i++;
		}else{
			if(!dst)j+=3;else{
				if(len-j<4)break;
				dst[j++] = ((src[i] & 0xf000) >> 12) | 0xe0;
				dst[j++] = ((src[i] & 0x0fc0) >>  6) | 0x80;
				dst[j++] = ((src[i] & 0x003f)      ) | 0x80;
			}
		}
	}
	if(dst)dst[j]=0;
	return j;
}


#ifdef ARM9

#include "consoleTGDS.h"
#include "devoptab_devices.h"
#include "fatfslayerTGDS.h"
#include "posixHandleTGDS.h"
#include "biosTGDS.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/reent.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <_ansi.h>
#include <reent.h>
#include <sys/lock.h>
#include <fcntl.h>

int	FS_loadFile(sint8 *filename, sint8 *buf, int size)
{
	FILE *f;
	int file_size;
	
	f = fopen(filename, "r");	//old: f = fopen(filename, "rb");
	if (f == NULL)
	{
		return -1;
	}
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	if (file_size < size)
	{
		fclose(f);
		return -1;
	}
	
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, size, f);
	
    fclose(f);
	return 0;
}

//force_file_creation == true: create a savefile regardless. false otherwise
//returns: written data size
int	FS_saveFile(sint8 *filename, sint8 *buf, int size, bool force_file_creation){
	FILE * f;
	char var[16];
	int retWritten = -1;
	if(force_file_creation == true){
		sprintf(var,"%s","w+");
	}
	else{
		sprintf(var,"%s","w");
	}
	if( (f = fopen(filename, (sint8*)var)) != NULL){
		retWritten = fwrite(buf, 1, size, f);
		fclose(f);
  	}
	return retWritten;
}

int	FS_getFileSize(sint8 *filename){
	FILE * f = fopen(filename, "r");
	int size = -1;
	if (f != NULL){
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		fclose(f);
	}
	FS_unlock();
	return size;
}

int setSoundPower(int flags){
	return 0;
}

void	FS_lock()
{
	
}

void	FS_unlock()
{
	
}

//char * bufOut is returned so you can use the return function 
char * print_ip(uint32 ip, char * bufOut){
    uint8 bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;	
    sprintf(bufOut,"%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
	return bufOut;
}

//FileSystem utils
sint8 *_FS_getFileExtension(sint8 *filename){
	static sint8 ext[4];
	sint8	*ptr = filename;
	int		i = 0;
	
	do{
		ptr = strchr(ptr, '.');
		if (!ptr)
			return NULL;
		ptr++;
	}
	while (strlen(ptr) > 3);
	
	for (i = 0; i < (int)strlen(ptr); i++){
		ext[i] = toupper((int)(ptr[i])); 
	}
	ext[i] = 0;
	return ext;
}

void separateExtension(char *str, char *ext)
{
	int x = 0;
	int y = 0;
	for(y = strlen(str) - 1; y > 0; y--)
	{
		if(str[y] == '.')
		{
			// found last dot
			x = y;
			break;
		}
		if(str[y] == '/')
		{
			// found a slash before a dot, no ext
			ext[0] = 0;
			return;
		}
	}
	
	if(x > 0)
	{
		int y = 0;
		while(str[x] != 0)
		{
			ext[y] = str[x];
			str[x] = 0;
			x++;
			y++;
		}
		ext[y] = 0;
	}
	else
		ext[0] = 0;	
}

sint8 *FS_getFileName(sint8 *filename){
	static sint8 name[100];
	sint8	*ptr = filename;
	int		i = 0;
	ptr = strrchr(ptr, '.');	
	for (i = 0; i < ptr-filename; i++){
		name[i] = filename[i]; 
	}
	name[i] = 0;
	return name;
}

int		FS_chdir(const sint8 *path){
	return fatfs_chdir(path);
}


//taken from https://stackoverflow.com/questions/9052490/find-the-count-of-substring-in-string
//modified by Coto
static int indexParse = 0;
int count_substr(const char *str, const char* substr, bool overlap) {
  if (strlen(substr) == 0) return -1; // forbid empty substr

  int count = 0;
  int increment = overlap ? 1 : strlen(substr);
  char* s = NULL;
  for ( s =(char*)str; (s = strstr(s, substr)); s += increment)
    ++count;
  return count;
}

void splitCustom(const char *str, char sep, splitCustom_fn fun, char * outBuf, int indexToLeftOut)
{
    unsigned int start = 0, stop = 0;
    for (stop = 0; str[stop]; stop++) {
        if (str[stop] == sep) {
            fun(str + start, stop - start, outBuf, indexToLeftOut, &sep);
            start = stop + 1;
        }
    }
    fun(str + start, stop - start, outBuf, indexToLeftOut, &sep);
}

//this callback debugs every character separated from splitCustom()
/*
void print(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    if(indexParse != indexToLeftOut){
        char localBuf[MAX_TGDSFILENAME_LENGTH+1];
        snprintf(localBuf,len+1,"%s",str);
        printf(" %d:%s%s:%d\n", (int)len, localBuf, delim, indexParse);
        indexParse++;
    }
}
*/

//this callback builds an output path (outBuf) and filters out the desired index. (used as a trim last directory callback)
void buildPath(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    if(indexParse != indexToLeftOut){
        if(strlen(outBuf) == 0){
            snprintf(outBuf,len+2,"%s%s",str, delim);
        }
        else{
            char localBuf[MAX_TGDSFILENAME_LENGTH+1];
            sprintf(localBuf,"%s",outBuf);
            snprintf(outBuf,strlen(outBuf)+len+2,"%s%s%s",localBuf,str,delim);
        }
        indexParse++;
    }
}


//this callback splits the haystack found in a stream, in the outBuf
char * outSplitBuf[TOP_ITEMS_SPLIT][MAX_TGDSFILENAME_LENGTH+1];
void splitCallback(const char *str, size_t len, char * outBuf, int indexToLeftOut, char * delim){
    snprintf((char*)&outSplitBuf[indexParse][0],len+1,"%s",str);
    indexParse++;
} 

int getLastDirFromPath(char * stream, char * haystack, char * outBuf){
    indexParse = 0;
    //leading / always src stream
    int topval = strlen(stream); 
    if(stream[topval-1] != '/'){
        stream[topval-1] = '/';
    }
    int indexToLeftOut = count_substr(stream, haystack, false);
    int indexToLeftOutCopy = indexToLeftOut;
    if(indexToLeftOutCopy > 1){ //allow index 0 to exist, so it's always left the minimum directory
        indexToLeftOutCopy--;
    }
    splitCustom(stream, (char)*haystack, buildPath, outBuf, indexToLeftOutCopy);
    //remove 0: out stream
    topval = strlen(outBuf) + 1;
    if((outBuf[0] == '0') && (outBuf[1] == ':')){
        char temp[MAX_TGDSFILENAME_LENGTH+1];
        snprintf(temp,topval-2,"%s",(char*)&outBuf[2]);
        sprintf(outBuf,"%s",temp);
    }
	//remove leading / in out stream 
    topval = strlen(outBuf); 
    if(outBuf[topval-1] == '/'){
        outBuf[topval-1] = '\0';
    }
	//edge case: the only directory was the leading / and was just removed, if so restore item
    if(topval == 1){
        outBuf[topval-1] = '/';
    }
	//edge case: remove double leading / 
	if((outBuf[0] == '/') && (outBuf[1] == '/')){
		char temp[MAX_TGDSFILENAME_LENGTH+1];
		snprintf(temp,strlen(outBuf)+1-1,"%s",(char*)&outBuf[1]);	//strlen(charBuf) +1 ending char - current offset we start to copy from
		sprintf(outBuf,"%s",temp);
	}
    return indexToLeftOut;
}

int str_split(char * stream, char * haystack, char * outBuf){
	indexParse = 0;
    int indexToLeftOut = count_substr(stream, haystack, false);
    splitCustom(stream, (char)*haystack, splitCallback, outBuf, indexToLeftOut);
    return indexToLeftOut;
}

/*
* Copyright (C) 1996-2001  Internet Software Consortium.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM
* DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
* INTERNET SOFTWARE CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
* FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
* WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "socket.h"

#define NS_INT16SZ       2
#define NS_INADDRSZ      4
#define NS_IN6ADDRSZ    16

/* int
* inet_pton4(src, dst)
*      like inet_aton() but without all the hexadecimal and shorthand.
* return:
*      1 if `src' is a valid dotted quad, else 0.
* notice:
*      does not touch `dst' unless it's returning 1.
* author:
*      Paul Vixie, 1996.
*/
static int inet_pton4(src, dst)
const char *src;
unsigned char *dst;
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	unsigned char tmp[NS_INADDRSZ], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;
		if ((pch = strchr(digits, ch)) != NULL) {
			unsigned int new = *tp * 10 + (pch - digits);
			if (new > 255)
				return (0);
			*tp = new;
			if (! saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		}
		else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		} 
		else
			return (0);
	}
	if (octets < 4)
		return (0);
	memcpy(dst, tmp, NS_INADDRSZ);
	return (1);
}

/* int
* isc_net_pton(af, src, dst)
*      convert from presentation format (which usually means ASCII printable)
*      to network format (which is usually some kind of binary format).
* return:
*      1 if the address was valid for the specified address family
*      0 if the address wasn't valid (`dst' is untouched in this case)
*      -1 if some other error occurred (`dst' is untouched in this case, too)
* author:
*      Paul Vixie, 1996.
*/
int inet_pton(int af, const char *src, void *dst){
	switch(af){
		case AF_INET:{
			return (inet_pton4(src, dst));
		}
		break;
		default:{
			errno = EAFNOSUPPORT;
		}
		break;
	}
	return (-1);
}


void RenderTGDSLogoSubEngine(u8 * compressedLZSSBMP, int compressedLZSSBMPSize){
	initFBModeSubEngine0x06200000();
	struct LZSSContext LZSSCtx = LZS_DecodeFromBuffer(compressedLZSSBMP, (unsigned int)compressedLZSSBMPSize);
	//These are hardcoded: TGDSLogoLZSSCompressed.bin -> Size: 12.442 / CRC32: e7255f11
	#define TGDSLOGONDSSIZE_SIZE 98304
	#define TGDSLOGONDSSIZE_LENGTH 49152
	#define TGDSLOGONDSSIZE_WIDTH 256
	#define TGDSLOGONDSSIZE_HEIGHT 192
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)LZSSCtx.bufferSource, (int)LZSSCtx.bufferSize);
	renderFBMode3Engine((u16*)LZSSCtx.bufferSource, 0x06200000, (int)TGDSLOGONDSSIZE_WIDTH,(int)TGDSLOGONDSSIZE_HEIGHT);
	
	//used? discard
	TGDSARM9Free(LZSSCtx.bufferSource);
}

void RenderTGDSLogoMainEngine(u8 * compressedLZSSBMP, int compressedLZSSBMPSize){
	initFBModeMainEngine0x06000000();
	struct LZSSContext LZSSCtx = LZS_DecodeFromBuffer(compressedLZSSBMP, (unsigned int)compressedLZSSBMPSize);
	//These are hardcoded: TGDSLogoLZSSCompressed.bin -> Size: 12.442 / CRC32: e7255f11
	#define TGDSLOGONDSSIZE_SIZE 98304
	#define TGDSLOGONDSSIZE_LENGTH 49152
	#define TGDSLOGONDSSIZE_WIDTH 256
	#define TGDSLOGONDSSIZE_HEIGHT 192
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)LZSSCtx.bufferSource, (int)LZSSCtx.bufferSize);
	renderFBMode3Engine((u16*)LZSSCtx.bufferSource, 0x06000000, (int)TGDSLOGONDSSIZE_WIDTH,(int)TGDSLOGONDSSIZE_HEIGHT);
	
	//used? discard
	TGDSARM9Free(LZSSCtx.bufferSource);
}

#endif
