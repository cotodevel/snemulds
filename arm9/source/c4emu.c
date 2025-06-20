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
#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <string.h>
#include "snes.h"
#include <math.h>
#include "snemulds_memmap.h"
#include "ppu.h"
#include "c4.h"

//uint32 Address = SNES Address . Example: D B92A (Bank D, offset 0xB92A)
uint8 *CX4FetchROMFromSNESAddress(uint32 Address){
    return ((SNESC.ROM + (((Address>>16)-1) * 0x8000)) +  (Address & 0xffff));
}

void S9xInitC4 (){
    // Stupid zsnes code, we can't do the logical thing without breaking
    // savestates
    //SNESC.C4RAM = &Memory.FillRAM [0x6000];
    memset(SNESC.C4RAM, 0, 0x2000);
}

uint8 S9xGetC4 (uint16 Address)
{
#ifdef DEBUGGER
    if(Settings.BGLayering) printf("%02x from %04x\n", SNESC.C4RAM[Address-0x6000], Address);
#endif
    if(Address >= 0x6000){
        return (SNESC.C4RAM [Address-0x6000]);
    }
    else
        return (SNESC.C4RAM [Address&0x1FFF]);
}

uint8 C4TestPattern [12 * 4] =
{
    0x00, 0x00, 0x00, 0xff,
    0xff, 0xff, 0x00, 0xff,
    0x00, 0x00, 0x00, 0xff,
    0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0x00, 0x00,
    0x80, 0xff, 0xff, 0x7f,
    0x00, 0x80, 0x00, 0xff,
    0x7f, 0x00, 0xff, 0x7f,
    0xff, 0x7f, 0xff, 0xff,
    0x00, 0x00, 0x01, 0xff,
    0xff, 0xfe, 0x00, 0x01,
    0x00, 0xff, 0xfe, 0x00
};


void C4ConvOAM(void){
    uint8 *OAMptr=SNESC.C4RAM+(SNESC.C4RAM[0x626]<<2);
    uint8 *i=0;
	for(i=SNESC.C4RAM+0x1fd; i>OAMptr; i-=4){
        // Clear OAM-to-be
        *i=0xe0;
    }

    uint16 globalX, globalY;
    uint8 *OAMptr2;
    int16 SprX, SprY;
    uint8 SprName, SprAttr;
    uint8 SprCount;
    
    globalX=READ_WORD(SNESC.C4RAM+0x0621);
    globalY=READ_WORD(SNESC.C4RAM+0x0623);
    OAMptr2=SNESC.C4RAM+0x200+(SNESC.C4RAM[0x626]>>2);

    if(SNESC.C4RAM[0x0620]!=0){
        SprCount=128-SNESC.C4RAM[0x626];
        uint8 offset=(SNESC.C4RAM[0x626]&3)*2;
        int prio=0;
		for(prio=0x30; prio>=0; prio-=0x10){
            uint8 *srcptr=SNESC.C4RAM+0x220;
            int i=0;
			for(i=SNESC.C4RAM[0x0620]; i>0 && SprCount>0; i--, srcptr+=16){
                if((srcptr[4]&0x30)!=prio) continue;
                SprX=READ_WORD(srcptr)-globalX;
                SprY=READ_WORD(srcptr+2)-globalY;
                SprName=srcptr[5];
                SprAttr=srcptr[4] | srcptr[0x06]; // XXX: mask bits?

                unsigned int val = READ_3WORD(srcptr + 7); 
				uint8 *sprptr=CX4FetchROMFromSNESAddress(val);				
                if(*sprptr!=0){
                    int16 X, Y;
					int SprCnt=0;
                    for(SprCnt=*sprptr++; SprCnt>0 && SprCount>0; SprCnt--, sprptr+=4){
                        X=(int8)sprptr[1];
                        if(SprAttr&0x40){ // flip X
                            X=-X-((sprptr[0]&0x20)?16:8);
                        }
                        X+=SprX;
                        if(X>=-16 && X<=272){
                            Y=(int8)sprptr[2];
                            if(SprAttr&0x80){
                                Y=-Y-((sprptr[0]&0x20)?16:8);
                            }
                            Y+=SprY;
                            if(Y>=-16 && Y<=224){
                                OAMptr[0]=X&0xff;
                                OAMptr[1]=(uint8)Y;
                                OAMptr[2]=SprName+sprptr[3];
                                OAMptr[3]=SprAttr^(sprptr[0]&0xc0); // XXX: Carry from SprName addition?
                                *OAMptr2 &= ~(3<<offset);
                                if(X&0x100) *OAMptr2 |= 1<<offset;
                                if(sprptr[0]&0x20) *OAMptr2 |= 2<<offset;
                                OAMptr+=4;
                                SprCount--;
                                offset=(offset+2)&6;
                                if(offset==0) OAMptr2++;
                            }
                        }
                    }
                } else if(SprCount>0){
                    OAMptr[0]=(uint8)SprX;
                    OAMptr[1]=(uint8)SprY;
                    OAMptr[2]=SprName;
                    OAMptr[3]=SprAttr;
                    *OAMptr2 &= ~(3<<offset);
                    if(SprX&0x100) *OAMptr2 |= 3<<offset;
                    else *OAMptr2 |= 2<<offset;
                    OAMptr+=4;
                    SprCount--;
                    offset=(offset+2)&6;
                    if(offset==0) OAMptr2++;
                }
            }
        }
    }
    // XXX: Copy to OAM? I doubt it.
}

void C4DoScaleRotate(int row_padding){
    int16 A, B, C, D;

    // Calculate matrix
    int32 XScale=READ_WORD(SNESC.C4RAM+0x1f8f);
    if(XScale&0x8000) XScale=0x7fff;
    int32 YScale=READ_WORD(SNESC.C4RAM+0x1f92);
    if(YScale&0x8000) YScale=0x7fff;

    if(READ_WORD(SNESC.C4RAM+0x1f80)==0)
	{ // no rotation
        // XXX: only do this for C and D?
        // XXX: and then only when YScale is 0x1000?
        A=(int16)XScale;
		B=0;
        C=0;
		D=(int16)YScale;
    }
	else if(READ_WORD(SNESC.C4RAM+0x1f80)==128){ // 90 degree rotation
        // XXX: Really do this?
        A=0;
		B=(int16)(-YScale);
        C=(int16)XScale;
		D=0;
    } else if(READ_WORD(SNESC.C4RAM+0x1f80)==256){ // 180 degree rotation
        // XXX: Really do this?
        A=(int16)(-XScale);
		B=0;
        C=0;
		D=(int16)(-YScale);
    } else if(READ_WORD(SNESC.C4RAM+0x1f80)==384){ // 270 degree rotation
        // XXX: Really do this?
        A=0;
		B=(int16)YScale;
        C=(int16)(-XScale);
		D=0;
    } else {
        A=(int16)SAR(C4CosTable[READ_WORD(SNESC.C4RAM+0x1f80)&0x1ff]*XScale, 15);
        B=(int16)(-SAR(C4SinTable[READ_WORD(SNESC.C4RAM+0x1f80)&0x1ff]*YScale, 15));
        C=(int16)SAR(C4SinTable[READ_WORD(SNESC.C4RAM+0x1f80)&0x1ff]*XScale, 15);
        D=(int16)SAR(C4CosTable[READ_WORD(SNESC.C4RAM+0x1f80)&0x1ff]*YScale, 15);
    }

    // Calculate Pixel Resolution
    uint8 w=SNESC.C4RAM[0x1f89]&~7;
    uint8 h=SNESC.C4RAM[0x1f8c]&~7;

//    printf("%dx%d XScale=%04x YScale=%04x angle=%03x\n", w, h, XScale, YScale, READ_WORD(SNESC.C4RAM+0x1f80)&0x1ff);
//    printf("Matrix: [%10g %10g]  [%04x %04x]\n", A/4096.0, B/4096.0, A&0xffff, B&0xffff);
//    printf("        [%10g %10g]  [%04x %04x]\n", C/4096.0, D/4096.0, C&0xffff, D&0xffff);

    // Clear the output RAM
    memset(SNESC.C4RAM, 0, (w+row_padding/4)*h/2);

    int32 Cx=(int16)READ_WORD(SNESC.C4RAM+0x1f83);
    int32 Cy=(int16)READ_WORD(SNESC.C4RAM+0x1f86);

    // Calculate start position (i.e. (Ox, Oy) = (0, 0))
    // The low 12 bits are fractional, so (Cx<<12) gives us the Cx we want in
    // the function. We do Cx*A etc normally because the matrix parameters
    // already have the fractional parts.
    int32 LineX=(Cx<<12) - Cx*A - Cx*B;
    int32 LineY=(Cy<<12) - Cy*C - Cy*D;
    
    // Start loop
    uint32 X, Y;
    uint8 byte;
    int outidx=0;
    uint8 bit=0x80;
    int y=0;
	for(y=0; y<h; y++){
        X=LineX;
        Y=LineY;
        int x=0; 
		for(x=0; x<w; x++){
            if((X>>12)>=w || (Y>>12)>=h){
                byte=0;
            } else {
                uint32 addr=(Y>>12)*w+(X>>12);
                byte=SNESC.C4RAM[0x600+(addr>>1)];
                if(addr&1) byte>>=4;
            }

            // De-bitplanify
            if(byte&1) SNESC.C4RAM[outidx]|=bit;
            if(byte&2) SNESC.C4RAM[outidx+1]|=bit;
            if(byte&4) SNESC.C4RAM[outidx+16]|=bit;
            if(byte&8) SNESC.C4RAM[outidx+17]|=bit;

            bit>>=1;
            if(bit==0){
                bit=0x80;
                outidx+=32;
            }
            
            X+=A; // Add 1 to output x => add an A and a C
            Y+=C;
        }
        outidx+=2+row_padding;
        if(outidx&0x10){
            outidx&=~0x10;
        } else {
            outidx-=w*4+row_padding;
        }
        LineX+=B; // Add 1 to output y => add a B and a D
        LineY+=D;
    }
}

void C4DrawLine(int32 X1, int32 Y1, int16 Z1, int32 X2, int32 Y2, int16 Z2, uint8 Color){
    // Transform coordinates
    C4WFXVal=(short)X1;
    C4WFYVal=(short)Y1;
    C4WFZVal=Z1;
    C4WFScale=SNESC.C4RAM[0x1f90];
    C4WFX2Val=SNESC.C4RAM[0x1f86];
    C4WFY2Val=SNESC.C4RAM[0x1f87];
    C4WFDist=SNESC.C4RAM[0x1f88];
    C4TransfWireFrame2();
    X1=(C4WFXVal+48)<<8;
    Y1=(C4WFYVal+48)<<8;

    C4WFXVal=(short)X2;
    C4WFYVal=(short)Y2;
    C4WFZVal=Z2;
    C4TransfWireFrame2();
    X2=(C4WFXVal+48)<<8;
    Y2=(C4WFYVal+48)<<8;

    // get line info
    C4WFXVal=(short)(X1>>8);
    C4WFYVal=(short)(Y1>>8);
    C4WFX2Val=(short)(X2>>8);
    C4WFY2Val=(short)(Y2>>8);
    C4CalcWireFrame();
    X2=(int16)C4WFXVal;
    Y2=(int16)C4WFYVal;

    // render line
    int i=0;
	for(i=C4WFDist?C4WFDist:1; i>0; i--)
	{ //.loop
        if(X1>0xff && Y1>0xff && X1<0x6000 && Y1<0x6000)
		{
            uint16 addr=((X1&~0x7ff) + (Y1&~0x7ff)*12 + (Y1&0x700))>>7;
            addr=(((Y1>>8)>>3)<<8)-(((Y1>>8)>>3)<<6)+(((X1>>8)>>3)<<4)+((Y1>>8)&7)*2;
            uint8 bit=0x80>>((X1>>8)&7);
            SNESC.C4RAM[addr+0x300]&=~bit;
            SNESC.C4RAM[addr+0x301]&=~bit;
            if(Color&1) SNESC.C4RAM[addr+0x300]|=bit;
            if(Color&2) SNESC.C4RAM[addr+0x301]|=bit;
        }
        X1+=X2;
        Y1+=Y2;
    }
}

void C4DrawWireFrame(void){
    uint8 *line=CX4FetchROMFromSNESAddress(READ_3WORD(SNESC.C4RAM+0x1f80));
    uint8 *point1, *point2;
    int16 X1, Y1, Z1;
    int16 X2, Y2, Z2;
    uint8 Color;

#ifdef DEBUGGER
    if(READ_3WORD(SNESC.C4RAM+0x1f8f)&0xff00ff) printf("wireframe: Unexpected value in $7f8f: %06x\n", READ_3WORD(SNESC.C4RAM+0x1f8f));
    if(READ_3WORD(SNESC.C4RAM+0x1fa4)!=0x001000) printf("wireframe: Unexpected value in $7fa4: %06x\n", READ_3WORD(SNESC.C4RAM+0x1fa4));
#endif
	int i=0;
    for(i=SNESC.C4RAM[0x0295]; i>0; i--, line+=5){
        if(line[0]==0xff && line[1]==0xff){
            uint8 *tmp=line-5;
            while(line[2]==0xff && line[3]==0xff) tmp-=5;
            point1=CX4FetchROMFromSNESAddress((SNESC.C4RAM[0x1f82]<<16) | (tmp[2]<<8) | tmp[3]);
        } else {
            point1=CX4FetchROMFromSNESAddress((SNESC.C4RAM[0x1f82]<<16) | (line[0]<<8) | line[1]);
        }
        point2=CX4FetchROMFromSNESAddress((SNESC.C4RAM[0x1f82]<<16) | (line[2]<<8) | line[3]);

        X1=(point1[0]<<8) | point1[1];
        Y1=(point1[2]<<8) | point1[3];
        Z1=(point1[4]<<8) | point1[5];
        X2=(point2[0]<<8) | point2[1];
        Y2=(point2[2]<<8) | point2[3];
        Z2=(point2[4]<<8) | point2[5];
        Color=line[4];
        C4DrawLine(X1, Y1, Z1, X2, Y2, Z2, Color);
    }
}

void C4TransformLines(void){
    C4WFX2Val=SNESC.C4RAM[0x1f83];
    C4WFY2Val=SNESC.C4RAM[0x1f86];
    C4WFDist=SNESC.C4RAM[0x1f89];
    C4WFScale=SNESC.C4RAM[0x1f8c];

    // transform vertices
    uint8 *ptr=SNESC.C4RAM;
	{
		int i=0;
		for(i=READ_WORD(SNESC.C4RAM+0x1f80); i>0; i--, ptr+=0x10)
		{
			C4WFXVal=READ_WORD(ptr+1);
	        C4WFYVal=READ_WORD(ptr+5);
		    C4WFZVal=READ_WORD(ptr+9);
			C4TransfWireFrame();

	        // displace
		    WRITE_WORD(ptr+1, C4WFXVal+0x80);
			WRITE_WORD(ptr+5, C4WFYVal+0x50);
		}
	}
    WRITE_WORD(SNESC.C4RAM+0x600, 23);
    WRITE_WORD(SNESC.C4RAM+0x602, 0x60);
    WRITE_WORD(SNESC.C4RAM+0x605, 0x40);
    WRITE_WORD(SNESC.C4RAM+0x600+8, 23);
    WRITE_WORD(SNESC.C4RAM+0x602+8, 0x60);
    WRITE_WORD(SNESC.C4RAM+0x605+8, 0x40);

    ptr=SNESC.C4RAM+0xb02;
    uint8 *ptr2=SNESC.C4RAM;
	{
		int i=0;
	    for(i=READ_WORD(SNESC.C4RAM+0xb00); i>0; i--, ptr+=2, ptr2+=8)
		{
		    C4WFXVal=READ_WORD(SNESC.C4RAM+(ptr[0]<<4)+1);
			C4WFYVal=READ_WORD(SNESC.C4RAM+(ptr[0]<<4)+5);
			C4WFX2Val=READ_WORD(SNESC.C4RAM+(ptr[1]<<4)+1);
			C4WFY2Val=READ_WORD(SNESC.C4RAM+(ptr[1]<<4)+5);
			C4CalcWireFrame();
			WRITE_WORD(ptr2+0x600, C4WFDist?C4WFDist:1);
			WRITE_WORD(ptr2+0x602, C4WFXVal);
			WRITE_WORD(ptr2+0x605, C4WFYVal);
		}
    }
}

void C4BitPlaneWave(){
    static uint16 bmpdata[]={
        0x0000, 0x0002, 0x0004, 0x0006, 0x0008, 0x000A, 0x000C, 0x000E,
        0x0200, 0x0202, 0x0204, 0x0206, 0x0208, 0x020A, 0x020C, 0x020E,
        0x0400, 0x0402, 0x0404, 0x0406, 0x0408, 0x040A, 0x040C, 0x040E,
        0x0600, 0x0602, 0x0604, 0x0606, 0x0608, 0x060A, 0x060C, 0x060E,
        0x0800, 0x0802, 0x0804, 0x0806, 0x0808, 0x080A, 0x080C, 0x080E
    };
    
    uint8 *dst=SNESC.C4RAM;
    uint32 waveptr=SNESC.C4RAM[0x1f83];
    uint16 mask1=0xc0c0;
    uint16 mask2=0x3f3f;
	int j=0;
    for(j=0; j<0x10; j++){
        do {
            int16 height=-((int8)SNESC.C4RAM[waveptr+0xb00])-16;
            int i=0;
			for(i=0; i<40; i++){
                uint16 tmp=READ_WORD(dst+bmpdata[i]) & mask2;
                if(height>=0){
                    if(height<8){
                        tmp|=mask1&READ_WORD(SNESC.C4RAM+0xa00+height*2);
                    } else {
                        tmp|=mask1&0xff00;
                    }
                }
                WRITE_WORD(dst+bmpdata[i], tmp);
                height++;
            }
            waveptr=(waveptr+1)&0x7f;
            mask1=(mask1>>2)|(mask1<<6);
            mask2=(mask2>>2)|(mask2<<6);
        } while(mask1!=0xc0c0);
        dst+=16;

        do {
            int16 height=-((int8)SNESC.C4RAM[waveptr+0xb00])-16;
            int i=0;
			for(i=0; i<40; i++){
                uint16 tmp=READ_WORD(dst+bmpdata[i]) & mask2;
                if(height>=0){
                    if(height<8){
                        tmp|=mask1&READ_WORD(SNESC.C4RAM+0xa10+height*2);
                    } else {
                        tmp|=mask1&0xff00;
                    }
                }
                WRITE_WORD(dst+bmpdata[i], tmp);
                height++;
            }
            waveptr=(waveptr+1)&0x7f;
            mask1=(mask1>>2)|(mask1<<6);
            mask2=(mask2>>2)|(mask2<<6);
        } while(mask1!=0xc0c0);
        dst+=16;
    }
}

void C4SprDisintegrate(){
    uint8 width, height;
    uint32 StartX, StartY;
    uint8 *src;
    int32 scaleX, scaleY;
    int32 Cx, Cy;

    width=SNESC.C4RAM[0x1f89];
    height=SNESC.C4RAM[0x1f8c];
    Cx=(int16)READ_WORD(SNESC.C4RAM+0x1f80);
    Cy=(int16)READ_WORD(SNESC.C4RAM+0x1f83);

#ifdef DEBUGGER
    if((Cx&~1)!=width/2 || (Cy&~1)!=height/2) printf("Center is not middle of image for disintegrate! (%d, %d) != (%d, %d)\n", Cx, Cy, width/2, height/2);
#endif
    
    scaleX=(int16)READ_WORD(SNESC.C4RAM+0x1f86);
    scaleY=(int16)READ_WORD(SNESC.C4RAM+0x1f8f);
    StartX=-Cx*scaleX+(Cx<<8);
    StartY=-Cy*scaleY+(Cy<<8);
    src=SNESC.C4RAM+0x600;

    memset(SNESC.C4RAM, 0, width*height/2);
    uint32 y=0,i=0;
    for(y=StartY, i=0; i<height; i++, y+=scaleY)
	{
		uint32 x=0,j=0;
        for(x=StartX, j=0; j<width; j++, x+=scaleX)
		{
            if((x>>8)<width && (y>>8)<height && (y>>8)*width+(x>>8)<0x2000)
			{
                uint8 pixel=(j&1)?(*src>>4):*src;
                int idx=(y>>11)*width*4+(x>>11)*32+((y>>8)&7)*2;
                uint8 mask=0x80>>((x>>8)&7);
                if(pixel&1) SNESC.C4RAM[idx]|=mask;
                if(pixel&2) SNESC.C4RAM[idx+1]|=mask;
                if(pixel&4) SNESC.C4RAM[idx+16]|=mask;
                if(pixel&8) SNESC.C4RAM[idx+17]|=mask;
            }
            if(j&1) src++;
        }
    }
}

void S9xC4ProcessSprites(){
    switch(SNESC.C4RAM[0x1f4d])
	{
      case 0x00: // Build OAM
#ifdef DEBUGGER
//        printf("00 00 Build OAM!\n");
#endif
        C4ConvOAM();
        break;

      case 0x03: // Scale/Rotate
#ifdef DEBUGGER
//        printf("00 03 Scale/Rotate!\n");
#endif
        C4DoScaleRotate(0);
        break;

      case 0x05: // Transform Lines
#ifdef DEBUGGER
//        printf("00 05 Transform Lines!\n");
#endif
        C4TransformLines();
        break;

      case 0x07: // Scale/Rotate
#ifdef DEBUGGER
//        printf("00 07 Scale/Rotate!\n");
#endif
        C4DoScaleRotate(64);
        break;

      case 0x08: // Draw wireframe
#ifdef DEBUGGER
//        printf("00 08 Draw wireframe!\n");
#endif
        C4DrawWireFrame();
        break;

      case 0x0b: // Disintegrate
#ifdef DEBUGGER
        printf("00 0b Disintegrate!\n");
#endif
        C4SprDisintegrate();
        break;

      case 0x0c: // Wave
#ifdef DEBUGGER
//        printf("00 0b Wave!\n");
#endif
        C4BitPlaneWave();
        break;
        
      default:
        break;
    }
}

void S9xSetC4 (uint8 byte, uint16 Address){
    int i;
    SNESC.C4RAM [Address-0x6000] = byte;
    if (Address == 0x7f4f){
        if(SNESC.C4RAM[0x1f4d]==0x0e && byte<0x40 && (byte&3)==0){
            SNESC.C4RAM[0x1f80]=byte>>2;
        }
		else
		{
            switch (byte)
			{
              case 0x00: // Sprite
                S9xC4ProcessSprites();
                break;

              case 0x01: // Draw wireframe
                memset(SNESC.C4RAM+0x300, 0, 16*12*3*4);
                C4DrawWireFrame();
                break;

              case 0x05: // Propulsion (?)
                {
                    int32 tmp=0x10000;
                    if(READ_WORD(SNESC.C4RAM+0x1f83)){
                        tmp=SAR((tmp/READ_WORD(SNESC.C4RAM+0x1f83))*READ_WORD(SNESC.C4RAM+0x1f81), 8);
                    }
                    WRITE_WORD(SNESC.C4RAM+0x1f80, (uint16)tmp);
                }
                break;

              case 0x0d: // Set vector length
                C41FXVal=READ_WORD(SNESC.C4RAM+0x1f80);
                C41FYVal=READ_WORD(SNESC.C4RAM+0x1f83);
                C41FDistVal=READ_WORD(SNESC.C4RAM+0x1f86);
                C4Op0D();
                WRITE_WORD(SNESC.C4RAM+0x1f89, C41FXVal);
                WRITE_WORD(SNESC.C4RAM+0x1f8c, C41FYVal);
                break;

              case 0x10: // Polar to rectangluar
                {
                    int32 tmp=SAR((int32)READ_WORD(SNESC.C4RAM+0x1f83)*C4CosTable[READ_WORD(SNESC.C4RAM+0x1f80)&0x1ff]*2, 16);
                    WRITE_3WORD(SNESC.C4RAM+0x1f86, tmp);
                    tmp=SAR((int32)READ_WORD(SNESC.C4RAM+0x1f83)*C4SinTable[READ_WORD(SNESC.C4RAM+0x1f80)&0x1ff]*2, 16);
                    WRITE_3WORD(SNESC.C4RAM+0x1f89, (tmp-SAR(tmp, 6)));
                }
                break;

              case 0x13: // Polar to rectangluar
                {
                    int32 tmp=SAR((int32)READ_WORD(SNESC.C4RAM+0x1f83)*C4CosTable[READ_WORD(SNESC.C4RAM+0x1f80)&0x1ff]*2, 8);
                    WRITE_3WORD(SNESC.C4RAM+0x1f86, tmp);
                    tmp=SAR((int32)READ_WORD(SNESC.C4RAM+0x1f83)*C4SinTable[READ_WORD(SNESC.C4RAM+0x1f80)&0x1ff]*2, 8);
                    WRITE_3WORD(SNESC.C4RAM+0x1f89, tmp);
                }
                break;

              case 0x15: // Pythagorean
                C41FXVal=READ_WORD(SNESC.C4RAM+0x1f80);
                C41FYVal=READ_WORD(SNESC.C4RAM+0x1f83);
                C41FDist=(int16)sqrt((double)C41FXVal*C41FXVal + (double)C41FYVal*C41FYVal);
                WRITE_WORD(SNESC.C4RAM+0x1f80, C41FDist);
                break;

              case 0x1f: // atan
                C41FXVal=READ_WORD(SNESC.C4RAM+0x1f80);
                C41FYVal=READ_WORD(SNESC.C4RAM+0x1f83);
                C4Op1F();
                WRITE_WORD(SNESC.C4RAM+0x1f86, C41FAngleRes);
                break;

              case 0x22: // Trapezoid
                {
                    int16 angle1=READ_WORD(SNESC.C4RAM+0x1f8c)&0x1ff;
                    int16 angle2=READ_WORD(SNESC.C4RAM+0x1f8f)&0x1ff;
#ifdef DEBUGGER
                    if(C4CosTable[angle1]==0) fprintf(stderr, "22 Trapezoid: Invalid tangent! angle1=%d\n", angle1);
                    if(C4CosTable[angle2]==0) fprintf(stderr, "22 Trapezoid: Invalid tangent! angle2=%d\n", angle2);
#endif
                    int32 tan1=(C4CosTable[angle1]!=0)?((((int32)C4SinTable[angle1])<<16)/C4CosTable[angle1]):0x80000000;
                    int32 tan2=(C4CosTable[angle2]!=0)?((((int32)C4SinTable[angle2])<<16)/C4CosTable[angle2]):0x80000000;
                    int16 y = READ_WORD(SNESC.C4RAM+0x1f83) - READ_WORD(SNESC.C4RAM+0x1f89);
                    int16 left, right;
                    int j=0;
					for(j=0; j<225; j++)
                    {
                        if(y>=0)
                        {
                            left = SAR((int32)tan1*y, 16) -
                                READ_WORD(SNESC.C4RAM+0x1f80) + 
                                READ_WORD(SNESC.C4RAM+0x1f86);
                            right = SAR((int32)tan2*y, 16) -
                                READ_WORD(SNESC.C4RAM+0x1f80) + 
                                READ_WORD(SNESC.C4RAM+0x1f86) +
                                READ_WORD(SNESC.C4RAM+0x1f93);

                            if(left<0 && right<0){
                                left=1;
                                right=0;
                            } else if(left<0){
                                left=0;
                            } else if(right<0){
                                right=0;
                            }
                            if(left>255 && right>255){
                                left=255;
                                right=254;
                            } else if(left>255){
                                left=255;
                            } else if(right>255){
                                right=255;
                            }
                        }
                        else
                        {
                            left=1;
                            right=0;
                        }
                        SNESC.C4RAM[j+0x800] = (uint8)left;
                        SNESC.C4RAM[j+0x900] = (uint8)right;
                        y++;
                    }
                }
                break;

              case 0x25: // Multiply
                { 
                    int32 foo=READ_3WORD(SNESC.C4RAM+0x1f80);
                    int32 bar=READ_3WORD(SNESC.C4RAM+0x1f83);
                    foo*=bar;
                    WRITE_3WORD(SNESC.C4RAM+0x1f80, foo);
                }
                break;

              case 0x2d: // Transform Coords
                C4WFXVal=READ_WORD(SNESC.C4RAM+0x1f81);
                C4WFYVal=READ_WORD(SNESC.C4RAM+0x1f84);
                C4WFZVal=READ_WORD(SNESC.C4RAM+0x1f87);
                C4WFX2Val=SNESC.C4RAM[0x1f89];
                C4WFY2Val=SNESC.C4RAM[0x1f8a];
                C4WFDist=SNESC.C4RAM[0x1f8b];
                C4WFScale=READ_WORD(SNESC.C4RAM+0x1f90);
                C4TransfWireFrame2();
                WRITE_WORD(SNESC.C4RAM+0x1f80, C4WFXVal);
                WRITE_WORD(SNESC.C4RAM+0x1f83, C4WFYVal);
                break;

              case 0x40: // Sum
                {
                    uint16 sum=0;
					int i=0;
                    for(i=0; i<0x800; sum+=SNESC.C4RAM[i++]);
                    WRITE_WORD(SNESC.C4RAM+0x1f80, sum);
                }
                break;

              case 0x54: // Square
                {
                    int64 a=SAR((int64)READ_3WORD(SNESC.C4RAM+0x1f80)<<40, 40);
				//	printf("%08X%08X\n", (uint32)(a>>32), (uint32)(a&0xFFFFFFFF));
                    a*=a;
				//	printf("%08X%08X\n", (uint32)(a>>32), (uint32)(a&0xFFFFFFFF));
                    WRITE_3WORD(SNESC.C4RAM+0x1f83, a);
                    WRITE_3WORD(SNESC.C4RAM+0x1f86, (a>>24));
                }
                break;

              case 0x5c: // Immediate Reg
                for (i = 0; i < 12 * 4; i++)
                    SNESC.C4RAM[i] = C4TestPattern [i];
                break;

              case 0x89: // Immediate ROM
                SNESC.C4RAM[0x1f80] = 0x36;
                SNESC.C4RAM[0x1f81] = 0x43;
                SNESC.C4RAM[0x1f82] = 0x05;
                break;

              default:
#ifdef DEBUGGER
                printf ("Unknown C4 command (%02x)\n", byte);
#endif
                break;
            }
        }
    } else if (Address == 0x7f47) {
        memmove(SNESC.C4RAM+(READ_WORD(SNESC.C4RAM+0x1f45)&0x1fff), 
                CX4FetchROMFromSNESAddress(READ_3WORD(SNESC.C4RAM+0x1f40)),
                READ_WORD(SNESC.C4RAM+0x1f43));
    }
}

int16 C4SinTable[512] = {
	    0,    402,    804,   1206,   1607,   2009,   2410,   2811,
      3211,   3611,   4011,   4409,   4808,   5205,   5602,   5997,
      6392,   6786,   7179,   7571,   7961,   8351,   8739,   9126,
      9512,   9896,  10278,  10659,  11039,  11416,  11793,  12167,
     12539,  12910,  13278,  13645,  14010,  14372,  14732,  15090,
     15446,  15800,  16151,  16499,  16846,  17189,  17530,  17869,
     18204,  18537,  18868,  19195,  19519,  19841,  20159,  20475,
     20787,  21097,  21403,  21706,  22005,  22301,  22594,  22884,
     23170,  23453,  23732,  24007,  24279,  24547,  24812,  25073,
     25330,  25583,  25832,  26077,  26319,  26557,  26790,  27020,
     27245,  27466,  27684,  27897,  28106,  28310,  28511,  28707,
     28898,  29086,  29269,  29447,  29621,  29791,  29956,  30117,
     30273,  30425,  30572,  30714,  30852,  30985,  31114,  31237,
     31357,  31471,  31581,  31685,  31785,  31881,  31971,  32057,
     32138,  32214,  32285,  32351,  32413,  32469,  32521,  32568,
     32610,  32647,  32679,  32706,  32728,  32745,  32758,  32765,
     32767,  32765,  32758,  32745,  32728,  32706,  32679,  32647,
     32610,  32568,  32521,  32469,  32413,  32351,  32285,  32214,
     32138,  32057,  31971,  31881,  31785,  31685,  31581,  31471,
     31357,  31237,  31114,  30985,  30852,  30714,  30572,  30425,
     30273,  30117,  29956,  29791,  29621,  29447,  29269,  29086,
     28898,  28707,  28511,  28310,  28106,  27897,  27684,  27466,
     27245,  27020,  26790,  26557,  26319,  26077,  25832,  25583,
     25330,  25073,  24812,  24547,  24279,  24007,  23732,  23453,
     23170,  22884,  22594,  22301,  22005,  21706,  21403,  21097,
     20787,  20475,  20159,  19841,  19519,  19195,  18868,  18537,
     18204,  17869,  17530,  17189,  16846,  16499,  16151,  15800,
     15446,  15090,  14732,  14372,  14010,  13645,  13278,  12910,
     12539,  12167,  11793,  11416,  11039,  10659,  10278,   9896,
      9512,   9126,   8739,   8351,   7961,   7571,   7179,   6786,
      6392,   5997,   5602,   5205,   4808,   4409,   4011,   3611,
      3211,   2811,   2410,   2009,   1607,   1206,    804,    402,
         0,   -402,   -804,  -1206,  -1607,  -2009,  -2410,  -2811,
     -3211,  -3611,  -4011,  -4409,  -4808,  -5205,  -5602,  -5997,
     -6392,  -6786,  -7179,  -7571,  -7961,  -8351,  -8739,  -9126,
     -9512,  -9896, -10278, -10659, -11039, -11416, -11793, -12167,
    -12539, -12910, -13278, -13645, -14010, -14372, -14732, -15090,
    -15446, -15800, -16151, -16499, -16846, -17189, -17530, -17869,
    -18204, -18537, -18868, -19195, -19519, -19841, -20159, -20475,
    -20787, -21097, -21403, -21706, -22005, -22301, -22594, -22884,
    -23170, -23453, -23732, -24007, -24279, -24547, -24812, -25073,
    -25330, -25583, -25832, -26077, -26319, -26557, -26790, -27020,
    -27245, -27466, -27684, -27897, -28106, -28310, -28511, -28707,
    -28898, -29086, -29269, -29447, -29621, -29791, -29956, -30117,
    -30273, -30425, -30572, -30714, -30852, -30985, -31114, -31237,
    -31357, -31471, -31581, -31685, -31785, -31881, -31971, -32057,
    -32138, -32214, -32285, -32351, -32413, -32469, -32521, -32568,
    -32610, -32647, -32679, -32706, -32728, -32745, -32758, -32765,
    -32767, -32765, -32758, -32745, -32728, -32706, -32679, -32647,
    -32610, -32568, -32521, -32469, -32413, -32351, -32285, -32214,
    -32138, -32057, -31971, -31881, -31785, -31685, -31581, -31471,
    -31357, -31237, -31114, -30985, -30852, -30714, -30572, -30425,
    -30273, -30117, -29956, -29791, -29621, -29447, -29269, -29086,
    -28898, -28707, -28511, -28310, -28106, -27897, -27684, -27466,
    -27245, -27020, -26790, -26557, -26319, -26077, -25832, -25583,
    -25330, -25073, -24812, -24547, -24279, -24007, -23732, -23453,
    -23170, -22884, -22594, -22301, -22005, -21706, -21403, -21097,
    -20787, -20475, -20159, -19841, -19519, -19195, -18868, -18537,
    -18204, -17869, -17530, -17189, -16846, -16499, -16151, -15800,
    -15446, -15090, -14732, -14372, -14010, -13645, -13278, -12910,
    -12539, -12167, -11793, -11416, -11039, -10659, -10278,  -9896,
     -9512,  -9126,  -8739,  -8351,  -7961,  -7571,  -7179,  -6786,
     -6392,  -5997,  -5602,  -5205,  -4808,  -4409,  -4011,  -3611,
     -3211,  -2811,  -2410,  -2009,  -1607,  -1206,   -804,   -402
};

int16 C4CosTable[512] = {
	     32767,  32765,  32758,  32745,  32728,  32706,  32679,  32647,
     32610,  32568,  32521,  32469,  32413,  32351,  32285,  32214,
     32138,  32057,  31971,  31881,  31785,  31685,  31581,  31471,
     31357,  31237,  31114,  30985,  30852,  30714,  30572,  30425,
     30273,  30117,  29956,  29791,  29621,  29447,  29269,  29086,
     28898,  28707,  28511,  28310,  28106,  27897,  27684,  27466,
     27245,  27020,  26790,  26557,  26319,  26077,  25832,  25583,
     25330,  25073,  24812,  24547,  24279,  24007,  23732,  23453,
     23170,  22884,  22594,  22301,  22005,  21706,  21403,  21097,
     20787,  20475,  20159,  19841,  19519,  19195,  18868,  18537,
     18204,  17869,  17530,  17189,  16846,  16499,  16151,  15800,
     15446,  15090,  14732,  14372,  14010,  13645,  13278,  12910,
     12539,  12167,  11793,  11416,  11039,  10659,  10278,   9896,
      9512,   9126,   8739,   8351,   7961,   7571,   7179,   6786,
      6392,   5997,   5602,   5205,   4808,   4409,   4011,   3611,
      3211,   2811,   2410,   2009,   1607,   1206,    804,    402,
         0,   -402,   -804,  -1206,  -1607,  -2009,  -2410,  -2811,
     -3211,  -3611,  -4011,  -4409,  -4808,  -5205,  -5602,  -5997,
     -6392,  -6786,  -7179,  -7571,  -7961,  -8351,  -8739,  -9126,
     -9512,  -9896, -10278, -10659, -11039, -11416, -11793, -12167,
    -12539, -12910, -13278, -13645, -14010, -14372, -14732, -15090,
    -15446, -15800, -16151, -16499, -16846, -17189, -17530, -17869,
    -18204, -18537, -18868, -19195, -19519, -19841, -20159, -20475,
    -20787, -21097, -21403, -21706, -22005, -22301, -22594, -22884,
    -23170, -23453, -23732, -24007, -24279, -24547, -24812, -25073,
    -25330, -25583, -25832, -26077, -26319, -26557, -26790, -27020,
    -27245, -27466, -27684, -27897, -28106, -28310, -28511, -28707,
    -28898, -29086, -29269, -29447, -29621, -29791, -29956, -30117,
    -30273, -30425, -30572, -30714, -30852, -30985, -31114, -31237,
    -31357, -31471, -31581, -31685, -31785, -31881, -31971, -32057,
    -32138, -32214, -32285, -32351, -32413, -32469, -32521, -32568,
    -32610, -32647, -32679, -32706, -32728, -32745, -32758, -32765,
    -32767, -32765, -32758, -32745, -32728, -32706, -32679, -32647,
    -32610, -32568, -32521, -32469, -32413, -32351, -32285, -32214,
    -32138, -32057, -31971, -31881, -31785, -31685, -31581, -31471,
    -31357, -31237, -31114, -30985, -30852, -30714, -30572, -30425,
    -30273, -30117, -29956, -29791, -29621, -29447, -29269, -29086,
    -28898, -28707, -28511, -28310, -28106, -27897, -27684, -27466,
    -27245, -27020, -26790, -26557, -26319, -26077, -25832, -25583,
    -25330, -25073, -24812, -24547, -24279, -24007, -23732, -23453,
    -23170, -22884, -22594, -22301, -22005, -21706, -21403, -21097,
    -20787, -20475, -20159, -19841, -19519, -19195, -18868, -18537,
    -18204, -17869, -17530, -17189, -16846, -16499, -16151, -15800,
    -15446, -15090, -14732, -14372, -14010, -13645, -13278, -12910,
    -12539, -12167, -11793, -11416, -11039, -10659, -10278,  -9896,
	 -9512,  -9126,  -8739,  -8351,  -7961,  -7571,  -7179,  -6786,
     -6392,  -5997,  -5602,  -5205,  -4808,  -4409,  -4011,  -3611,
     -3211,  -2811,  -2410,  -2009,  -1607,  -1206,   -804,   -402,
         0,    402,    804,   1206,   1607,   2009,   2410,   2811,
      3211,   3611,   4011,   4409,   4808,   5205,   5602,   5997,
      6392,   6786,   7179,   7571,   7961,   8351,   8739,   9126,
      9512,   9896,  10278,  10659,  11039,  11416,  11793,  12167,
     12539,  12910,  13278,  13645,  14010,  14372,  14732,  15090,
     15446,  15800,  16151,  16499,  16846,  17189,  17530,  17869,
     18204,  18537,  18868,  19195,  19519,  19841,  20159,  20475,
     20787,  21097,  21403,  21706,  22005,  22301,  22594,  22884,
     23170,  23453,  23732,  24007,  24279,  24547,  24812,  25073,
     25330,  25583,  25832,  26077,  26319,  26557,  26790,  27020,
     27245,  27466,  27684,  27897,  28106,  28310,  28511,  28707,
     28898,  29086,  29269,  29447,  29621,  29791,  29956,  30117,
     30273,  30425,  30572,  30714,  30852,  30985,  31114,  31237,
     31357,  31471,  31581,  31685,  31785,  31881,  31971,  32057,
     32138,  32214,  32285,  32351,  32413,  32469,  32521,  32568,
     32610,  32647,  32679,  32706,  32728,  32745,  32758,  32765
};
