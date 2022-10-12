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

#ifndef _DSP1_H_
#define _DSP1_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "typedefsTGDS.h"

typedef signed char int8;
typedef short int16;
typedef long int32;
typedef long long int64;

// Simple vector and matrix types
typedef double MATRIX[3][3];
typedef double VECTOR[3];

enum AttitudeMatrix { MatrixA, MatrixB, MatrixC };

struct SDSP1 {
    bool8 waiting4command;
    bool8 first_parameter;
    uint8 command;
    uint32 in_count;
    uint32 in_index;
    uint32 out_count;
    uint32 out_index;
    uint8 parameters [512];
//output was 512 for DSP-2 work, updated to reflect current thinking on DSP-3
    uint8 output [512];

    // Attitude matrices
    MATRIX vMa;
    MATRIX vMb;
    MATRIX vMc;
    
    // Matrix and translaton vector for
    // transforming a 3D position into the global coordinate system,
    // from the view space coordinate system.
    MATRIX vM;
    VECTOR vT;

    // Focal distance
    double vFov;

    // A precalculated value for optimization
    double vPlaneD;
    
    // Raster position of horizon
    double vHorizon;

    // Convert a 2D screen coordinate to a 3D ground coordinate in global coordinate system.
    //void ScreenToGround(VECTOR &v, double X2d, double Y2d);

	//MATRIX &GetMatrix( AttitudeMatrix Matrix );
};

///////////////// DSP Commands ////////////////////

// DSP1 Command 02h
struct DSP1_Parameter
{
    /*
	DSP1_Parameter( int16 Fx, int16 Fy, int16 Fz,
		      uint16 Lfe, uint16 Les,
		      int8 Aas, int8 Azs );
	*/

    // Raster number of imaginary center
    int16 Vof;	// -32768 ~ +32767

    // Raster number representing
    // horizontal line.
    int16 Vva;	// -32768 ~ +32767

    // X,Y coordinate of the point
    // projected on the center of the screen
    // (ground coordinate)
    int16 Cx;	// -32768 ~ +32767
    int16 Cy;	// -32768 ~ +32767
};

// DSP1 Command 0Ah
struct DSP1_Raster
{
    //DSP1_Raster( int16 Vs );

    // Linear transformation matrix elements
    // for each raster
    int16 An;
    int16 Bn;
    int16 Cn;
    int16 Dn;
};

// DSP1 Command 06h
struct DSP1_Project
{
    //DSP1_Project( int16 x, int16 y, int16 z );

    int16 H;
    int16 V;
    int16 M;
};

// DSP1 Command 0Eh
struct DSP1_Target
{
    //DSP1_Target( int16 h, int16 v );

    int16 X;
    int16 Y;
};

// DSP1 Command 04h
struct DSP1_Triangle
{
	//DSP1_Triangle (int16 Theta, int16 r );
    int16 S;
    int16 C;
};

// DSP1 Command 08h
struct DSP1_Radius
{
	//DSP1_Radius( int16 x, int16 y, int16 z );
    int16 Ll;
    int16 Lh;
};

// DSP1 Command 18h
int16 DSP1_Range( int16 x, int16 y, int16 z, int16 r );

// DSP1 Command 28h
int16 DSP1_Distance( int16 x, int16 y, int16 z );

// DSP1 Command 0Ch
struct DSP1_Rotate
{
	//DSP1_Rotate (int16 A, int16 x1, int16 y1);

    int16 x2;
    int16 y2;
};

// DSP1 Command 1Ch
struct DSP1_Polar
{
	//DSP1_Polar( int8 Za, int8 Xa, int8 Ya, int16 x, int16 y, int16 z );

    int16 X;
    int16 Y;
    int16 Z;
};

// DSP1 Command 01h, 11h and 21h
//void DSP1_Attitude( int16 m, int8 Za, int8 Xa, int8 Ya, AttitudeMatrix Matrix );

// DSP1 Command 0Dh, 1Dh and 2Dh
struct DSP1_Objective
{
	//DSP1_Objective( int16 x, int16 y, int16 z, AttitudeMatrix Matrix );

    int16 F;
    int16 L;
    int16 U;
};

// DSP1 Command 03h, 13h and 23h
struct DSP1_Subjective
{
	//DSP1_Subjective( int16 F, int16 L, int16 U, AttitudeMatrix Matrix );

    int16 X;
    int16 Y;
    int16 Z;
};

// DSP1 Command 0Bh, 1Bh and 2Bh
//int16 DSP1_Scalar( int16 x, int16 y, int16 z, AttitudeMatrix Matrix );

// DSP1 Command 14h
struct DSP1_Gyrate
{
	//DSP1_Gyrate( int8 Zi, int8 Xi, int8 Yi, int8 dU, int8 dF, int8 dL );

    int8 Z0;
    int8 X0;
    int8 Y0;
};

// DSP1 Command 00h
//int16 DSP1_Multiply( int16 k, int16 I );

// DSP1 Command 10h
struct DSP1_Inverse
{
	//DSP1_Inverse( int16 a, int16 b );

    int16 A;
    int16 B;
};

#define INCR 2048
#define Angle(x) (((x)/(65536/INCR)) & (INCR-1))
#define Cos(x) ((double) CosTable2[x])
#define Sin(x) ((double) SinTable2[x])
#ifdef PI
#undef PI
#endif
#define PI 3.1415926535897932384626433832795

#define VofAngle 0x3880

#endif


#ifdef __cplusplus
extern "C"{
#endif

extern struct SDSP1 DSP1;

extern void S9xResetDSP1 ();
extern uint8 S9xGetDSP (uint16 Address);
extern void S9xSetDSP (uint8 Byte, uint16 Address);

extern void (*SetDSP)(uint8, uint16);
extern uint8 (*GetDSP)(uint16);

extern void DSP1SetByte(uint8 byte, uint16 address);
extern uint8 DSP1GetByte(uint16 address);

extern void DSP2SetByte(uint8 byte, uint16 address);
extern uint8 DSP2GetByte(uint16 address);

extern void DSP3SetByte(uint8 byte, uint16 address);
extern uint8 DSP3GetByte(uint16 address);

extern void DSP4SetByte(uint8 byte, uint16 address);
extern uint8 DSP4GetByte(uint16 address);


extern const unsigned short DSP1ROM[1024];
extern double CosTable2[INCR];
extern double SinTable2[INCR];
extern short Op00Multiplicand;
extern short Op00Multiplier;
extern short Op00Result;
extern short Op20Multiplicand;
extern short Op20Multiplier;
extern short Op20Result;

extern signed short Op10Coefficient;
extern signed short Op10Exponent;
extern signed short Op10CoefficientR;
extern signed short Op10ExponentR;

extern short Op04Angle;
extern short Op04Radius;
extern short Op04Sin;
extern short Op04Cos;

extern const short DSP1_MulTable[256];
extern const short DSP1_SinTable[256];
extern short DSP1_Sin(short Angle);

extern short Op0CA;
extern short Op0CX1;
extern short Op0CY1;
extern short Op0CX2;
extern short Op0CY2;

extern short Op02FX;
extern short Op02FY;
extern short Op02FZ;
extern short Op02LFE;
extern short Op02LES;
extern unsigned short Op02AAS;
extern unsigned short Op02AZS;
extern unsigned short Op02VOF;
extern unsigned short Op02VVA;

extern short Op02CX;
extern short Op02CY;
extern double Op02CXF;
extern double Op02CYF;
extern double ViewerX0;
extern double ViewerY0;
extern double ViewerZ0;
extern double ViewerX1;
extern double ViewerY1;
extern double ViewerZ1;
extern double ViewerX;
extern double ViewerY;
extern double ViewerZ;
extern int ViewerAX;
extern int ViewerAY;
extern int ViewerAZ;
extern double NumberOfSlope;
extern double ScreenX;
extern double ScreenY;
extern double ScreenZ;
extern double TopLeftScreenX;
extern double TopLeftScreenY;
extern double TopLeftScreenZ;
extern double BottomRightScreenX;
extern double BottomRightScreenY;
extern double BottomRightScreenZ;
extern double Ready;
extern double RasterLX;
extern double RasterLY;
extern double RasterLZ;
extern double ScreenLX1;
extern double ScreenLY1;
extern double ScreenLZ1;
extern int    ReversedLES;
extern short Op02LESb;
extern double NAzsB, NAasB;
extern double ViewerXc;
extern double ViewerYc;
extern double ViewerZc;
extern double CenterX, CenterY;
extern short Op02CYSup, Op02CXSup;
extern double CXdistance;
extern short TValDebug, TValDebug2;
extern short ScrDispl;

extern short Op0AVS;
extern short Op0AA;
extern short Op0AB;
extern short Op0AC;
extern short Op0AD;

extern double RasterRX;
extern double RasterRY;
extern double RasterRZ;
extern double RasterLSlopeX;
extern double RasterLSlopeY;
extern double RasterLSlopeZ;
extern double RasterRSlopeX;
extern double RasterRSlopeY;
extern double RasterRSlopeZ;
extern double GroundLX;
extern double GroundLY;
extern double GroundRX;
extern double GroundRY;
extern double Distance;

extern double NAzs, NAas;
extern double RVPos, RHPos, RXRes, RYRes;
extern short Op06X;
extern short Op06Y;
extern short Op06Z;
extern short Op06H;
extern short Op06V;
extern unsigned short Op06S;

extern double ObjPX;
extern double ObjPY;
extern double ObjPZ;
extern double ObjPX1;
extern double ObjPY1;
extern double ObjPZ1;
extern double ObjPX2;
extern double ObjPY2;
extern double ObjPZ2;
extern double DivideOp06;
extern int Temp;
extern int tanval2;

extern short matrixC[3][3];
extern short matrixB[3][3];
extern short matrixA[3][3];

extern short Op01m;
extern short Op01Zr;
extern short Op01Xr;
extern short Op01Yr;
extern short Op11m;
extern short Op11Zr;
extern short Op11Xr;
extern short Op11Yr;
extern short Op21m;
extern short Op21Zr;
extern short Op21Xr;
extern short Op21Yr;

extern short Op0DX;
extern short Op0DY;
extern short Op0DZ;
extern short Op0DF;
extern short Op0DL;
extern short Op0DU;
extern short Op1DX;
extern short Op1DY;
extern short Op1DZ;
extern short Op1DF;
extern short Op1DL;
extern short Op1DU;
extern short Op2DX;
extern short Op2DY;
extern short Op2DZ;
extern short Op2DF;
extern short Op2DL;
extern short Op2DU;

extern short Op03F;
extern short Op03L;
extern short Op03U;
extern short Op03X;
extern short Op03Y;
extern short Op03Z;
extern short Op13F;
extern short Op13L;
extern short Op13U;
extern short Op13X;
extern short Op13Y;
extern short Op13Z;
extern short Op23F;
extern short Op23L;
extern short Op23U;
extern short Op23X;
extern short Op23Y;
extern short Op23Z;

extern short Op14Zr;
extern short Op14Xr;
extern short Op14Yr;
extern short Op14U;
extern short Op14F;
extern short Op14L;
extern short Op14Zrr;
extern short Op14Xrr;
extern short Op14Yrr;

extern short Op0EH;
extern short Op0EV;
extern short Op0EX;
extern short Op0EY;

extern short Op0BX;
extern short Op0BY;
extern short Op0BZ;
extern short Op0BS;
extern short Op1BX;
extern short Op1BY;
extern short Op1BZ;
extern short Op1BS;
extern short Op2BX;
extern short Op2BY;
extern short Op2BZ;
extern short Op2BS;

extern short Op08X, Op08Y, Op08Z, Op08Ll, Op08Lh;
extern short Op18X, Op18Y, Op18Z, Op18R, Op18D;
extern short Op38X, Op38Y, Op38Z, Op38R, Op38D;
extern short Op28X;
extern short Op28Y;
extern short Op28Z;
extern short Op28R;

extern short Op1CX, Op1CY, Op1CZ;
extern short Op1CXBR, Op1CYBR, Op1CZBR, Op1CXAR, Op1CYAR, Op1CZAR;
extern short Op1CX1;
extern short Op1CY1;
extern short Op1CZ1;
extern short Op1CX2;
extern short Op1CY2;
extern short Op1CZ2;

extern unsigned short Op0FRamsize;
extern unsigned short Op0FPass;

extern short Op2FUnknown;
extern short Op2FSize;

//dspemu2.c
extern uint16 DSP2Op09Word1;
extern uint16 DSP2Op09Word2;
extern bool DSP2Op05HasLen;
extern int DSP2Op05Len;
extern bool DSP2Op06HasLen;
extern int DSP2Op06Len;
extern uint8 DSP2Op05Transparent;

extern bool DSP2Op0DHasLen;
extern int DSP2Op0DOutLen;
extern int DSP2Op0DInLen;


#ifdef __cplusplus
}
#endif