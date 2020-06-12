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

#ifndef __nds_spitsc_h__
#define __nds_spitsc_h__

#include "typedefsTGDS.h"
#include <stdbool.h>

//TouchScreen
struct XYTscPos{
	uint16	rawx; 
	uint16	rawy; 
	uint16	px;   
	uint16	py;
	uint16	z1;   
	uint16	z2;
};

//XY / PenIRQ
#define KEY_XARM7 (1<<0)
#define KEY_YARM7 (1<<1)
#define KEY_PENIRQARM7 (1<<6)
#define KEY_HINGE (1<<7)	//0 open / 1 closed

//4000136h - NDS7 - EXTKEYIN - Key X/Y Input (R)
//  0      Button X     (0=Pressed, 1=Released)
//  1      Button Y     (0=Pressed, 1=Released)
//  3      DEBUG button (0=Pressed, 1=Released/None such)
//  6      Pen down     (0=Pressed, 1=Released/Disabled) (always 0 in DSi mode)
//  7      Hinge/folded (0=Open, 1=Closed)
//  2,4,5  Unknown / set
//  8..15  Unknown / zero
  
struct sEXTKEYIN{
	bool butX;	//true pressed/ false not
	bool butY;	//true pressed/ false not
	bool debugButton;	//ignored
	bool PenDown;	//true pressed	/ false not
	bool	hinge_folded;	//true folded / false open
} ;

//40001C0 | BIT_SPICLK_2MHZ | BIT_SPICNT_TSCCNT

//Control Byte (transferred MSB first)
//  0-1  Power Down Mode Select
#define BIT_TSCCNT_POWDOWN_MODE_SEL_DIFFERENTIAL (0 << 0)	//Differential Mode (Touchscreen, Penirq)
#define BIT_TSCCNT_POWDOWN_MODE_SEL_SINGLE (1 << 0)			//Single-Ended Mode (Temperature, Microphone)
#define BIT_TSCCNT_POWDOWN_MODE_SEL_UNUSED1 (2 << 0)
#define BIT_TSCCNT_POWDOWN_MODE_SEL_UNUSED2 (3 << 0)

//  2    Reference Select (0=Differential, 1=Single-Ended)
#define BIT_TSCCNT_REFSEL_DIFFERENTIAL (0 << 2)
#define BIT_TSCCNT_REFSEL_SINGLE (1 << 2)

//  3    Conversion Mode  (0=12bit, max CLK=2MHz, 1=8bit, max CLK=3MHz)
#define BIT_TSCCNT_CONVMODE_12bit (0 << 3)	//2MHZ
#define BIT_TSCCNT_CONVMODE_8bit (1 << 3)	//3MHZ

//  4-6  Channel Select   
//Channel
//  0 Temperature 0 (requires calibration, step 2.1mV per 1'C accuracy)
//  1 Touchscreen Y-Position  (somewhat 0B0h..F20h, or FFFh=released)
//  2 Battery Voltage         (not used, connected to GND in NDS, always 000h)
//  3 Touchscreen Z1-Position (diagonal position for pressure measurement)
//  4 Touchscreen Z2-Position (diagonal position for pressure measurement)
//  5 Touchscreen X-Position  (somewhat 100h..ED0h, or 000h=released)
//  6 AUX Input               (connected to Microphone in the NDS)
//  7 Temperature 1 (difference to Temp 0, without calibration, 2'C accuracy)
#define BIT_TSCCNT_TEMP0READ 	(0 << 4)
#define BIT_TSCCNT_TOUCHYPOS 	(1 << 4)
#define BIT_TSCCNT_BATTVOLTAGE 	(2 << 4)
#define BIT_TSCCNT_TOUCHZ1POS 	(3 << 4)
#define BIT_TSCCNT_TOUCHZ2POS 	(4 << 4)
#define BIT_TSCCNT_TOUCHXPOS 	(5 << 4)
#define BIT_TSCCNT_AUXINPUT 	(6 << 4)
#define BIT_TSCCNT_TEMP1READ 	(7 << 4)

//  7    Start Bit (Must be set to access Control Byte)
#define BIT_TSCCNT_START_CTRL (1 << 7)	//Must be set to 1 when using Control Byte in TSCNT

#endif


#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM7
void doSPIARM7IO();
#endif

//Shared
extern bool penIRQread();	//return true if TSC is touched, false if not

extern void XYReadScrPos(struct XYTscPos * StouchScrPosInst);	//User exposed
extern void XYReadPos();	//Internal, requires to be called on interrupts
#ifdef __cplusplus
}
#endif