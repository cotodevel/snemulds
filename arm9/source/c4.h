/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2003 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2003 Matthew Kendora and
                            Brad Jorsch (anomie@users.sourceforge.net)
 

                      
  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and
                            Nach (n-a-c-h@users.sourceforge.net)
                                          
  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2003 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman (jweidman@slip.net),
                            neviksti (neviksti@hotmail.com), and
                            Kris Bleakley (stinkfish@bigpond.com)
 
  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2003 zsKnight, pagefault (pagefault@zsnes.com)
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar and Gary Henderson.



 
  Specific ports contains the works of other authors. See headers in
  individual files.
 
  Snes9x homepage: http://www.snes9x.com
 
  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.
 
  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.
 
  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.
 
  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.
 
  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/
#ifndef _C4_H_
#define _C4_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef _MSC_VER
#include "port.h"
#endif

#ifdef ARM9
#include "typedefsTGDS.h"

typedef signed char int8;
typedef short int16;
typedef long int32;
typedef long long int64;

#define READ_WORD(s) ( *(uint8 *) (s) |\
		      (*((uint8 *) (s) + 1) << 8))

#define READ_DWORD(s) ( *(uint8 *) (s) |\
		       (*((uint8 *) (s) + 1) << 8) |\
		       (*((uint8 *) (s) + 2) << 16) |\
		       (*((uint8 *) (s) + 3) << 24))


#define WRITE_WORD(s, d) *(uint8 *) (s) = (d), \
                         *((uint8 *) (s) + 1) = (d) >> 8


#define WRITE_DWORD(s, d) *(uint8 *) (s) = (uint8) (d), \
                          *((uint8 *) (s) + 1) = (uint8) ((d) >> 8),\
                          *((uint8 *) (s) + 2) = (uint8) ((d) >> 16),\
                          *((uint8 *) (s) + 3) = (uint8) ((d) >> 24)


#define WRITE_3WORD(s, d) *(uint8 *) (s) = (uint8) (d), \
                          *((uint8 *) (s) + 1) = (uint8) ((d) >> 8),\
                          *((uint8 *) (s) + 2) = (uint8) ((d) >> 16)


#define READ_3WORD(s) ( *(uint8 *) (s) |\
                       (*((uint8 *) (s) + 1) << 8) |\
                       (*((uint8 *) (s) + 2) << 16))
			

#define SAR(b, n) ((b)>>(n))

//Snes9X specific
#define MEMMAP_BLOCK_SIZE (0x1000)
#define MEMMAP_NUM_BLOCKS (0x1000000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_BLOCKS_PER_BANK (0x10000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_SHIFT 12
#define MEMMAP_MASK (MEMMAP_BLOCK_SIZE - 1)

#endif

#ifdef ARM9
#ifdef __cplusplus
extern "C" {
#endif
#endif

extern int16 C4WFXVal;
extern int16 C4WFYVal;
extern int16 C4WFZVal;
extern int16 C4WFX2Val;
extern int16 C4WFY2Val;
extern int16 C4WFDist;
extern int16 C4WFScale;

void C4TransfWireFrame();
void C4TransfWireFrame2();
void C4CalcWireFrame();

extern int16 C41FXVal;
extern int16 C41FYVal;
extern int16 C41FAngleRes;
extern int16 C41FDist;
extern int16 C41FDistVal;

void C4Op1F();
void C4Op15();
void C4Op0D();

extern int16 C4CosTable[];
extern int16 C4SinTable[];

extern unsigned char CX4ROMBuffer[4 * 1024];
extern uint8 * currentCX4ROMPage;
extern int currentCX4ROMPagePtr;

extern uint8 readCX4ValueFromROM(uint32 SNESAddress);
extern void CX4CopyFromROM(uint32 SNESAddress, uint8 * targetBuffer, int targetBufferSize);
extern void S9xInitC4(char * C4ROMFilename);

#ifdef _MSC_VER
extern FILE * curC4FileHandle;
#endif

extern int curC4FileHandleSize;

#ifdef ARM9
//c4emu.c
extern uint8 S9xGetC4 (uint16 Address);
extern uint8 C4TestPattern [12 * 4];
extern void C4ConvOAM(void);
extern void C4DoScaleRotate(int row_padding);
extern void C4DrawLine(int32 X1, int32 Y1, int16 Z1, int32 X2, int32 Y2, int16 Z2, uint8 Color);
extern void C4DrawWireFrame(void);
extern void C4TransformLines(void);
extern void C4BitPlaneWave();
extern void C4SprDisintegrate();
extern void S9xC4ProcessSprites();
extern void S9xSetC4 (uint8 byte, uint16 Address);
extern int16 C4SinTable[512];
extern int16 C4CosTable[512];
#endif

#ifdef ARM9
#ifdef __cplusplus
}
#endif
#endif

#endif
