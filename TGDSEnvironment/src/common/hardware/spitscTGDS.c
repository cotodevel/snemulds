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

#include <stdbool.h>
#include "ipcfifoTGDS.h"
#include "spitscTGDS.h"

#ifdef ARM7
#include "spiTGDS.h"
#include "clockTGDS.h"
#include "eventsTGDS.h"



//Source http://problemkaputt.de/gbatek.htm
inline __attribute__((always_inline)) 
void doSPIARM7IO(){
	struct sEXTKEYIN * sEXTKEYINInst = (struct sEXTKEYIN *)&TGDSIPC->EXTKEYINInst;
	
	//Read is pen down
	sEXTKEYINInst->PenDown = penIRQread();
	
	//External X Y Struct
	uint8 readEXTKEYIN = (uint8)REG_KEYXY;
	if(!(readEXTKEYIN & KEY_XARM7)){
		sEXTKEYINInst->butX = true;
	}
	else{
		sEXTKEYINInst->butX = false;
	}
	
	if(!(readEXTKEYIN & KEY_YARM7)){
		sEXTKEYINInst->butY = true;
	}
	else{
		sEXTKEYINInst->butY = false;
	}
	
	//Folding
	if(!(readEXTKEYIN & KEY_HINGE)){
		sEXTKEYINInst->hinge_folded = true;
	}
	else{
		sEXTKEYINInst->hinge_folded = false;
	}
	
	//NDS Format ARM7 XY/PENIRQ/HINGE buttons
	TGDSIPC->buttons7 = REG_KEYXY;
	
	//REG_KEYINPUT ARM7 
	TGDSIPC->KEYINPUT7	=	(uint16)REG_KEYINPUT;
	
	//Do touchscreen process
	XYReadPos();
	
	//read clock
	TGDSIPC->ndsRTCSeconds = nds_get_time7();
	
	//Handle Sleep-wakeup events
	uint16 buttonsARM7 = REG_KEYXY;
	uint32 readKeys = (uint32)(( ((~KEYINPUT)&0x3ff) | (((~buttonsARM7)&3)<<10) | (((~buttonsARM7)<<6) & (KEY_TOUCH|KEY_LID) ))^KEY_LID);
	if(sleepModeEnabled == true){
		if (readKeys & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN | KEY_A | KEY_B | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_TOUCH | KEY_SELECT | KEY_START)){
			TurnOnScreens();
		}
	}
	
	//Custom Button Mapping Handler
	CustomInputMappingHandler(readKeys);

}
#endif

bool penIRQread(){

	#ifdef ARM7
	if(!(REG_KEYXY & KEY_PENIRQARM7)){
		return true;
	}
	else{
		return false;
	}
	#endif
	
	#ifdef ARM9
	return (TGDSIPC->EXTKEYINInst.PenDown);
	#endif
}

#ifdef ARM7

static int LastTSCPosX = 0;
static int LastTSCPosY = 0;

//Internal
void XYReadPos(){
	
	if(TGDSIPC->EXTKEYINInst.PenDown == true){
		//Set Chip Select LOW to invoke the command & Transmit the instruction byte: TSC CNT Differential Mode: X Raw TSC 
		REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_CSHOLDENABLE | BIT_SPICNT_TSCCNT | BIT_SPICLK_2MHZ;
		RWSPICNT(BIT_TSCCNT_START_CTRL|BIT_TSCCNT_POWDOWN_MODE_SEL_DIFFERENTIAL| BIT_TSCCNT_REFSEL_DIFFERENTIAL | BIT_TSCCNT_CONVMODE_12bit | BIT_TSCCNT_TOUCHXPOS);
		uint8 resultx11to5 =RWSPICNT(0);	//0-11-10-9-8-7-6-5
		uint8 resultx4to0 = RWSPICNT(0);	//4-3-2-1-0-0-0-0
		uint16 read_raw_x = ((resultx11to5 & 0x7F) << 5) | ((resultx4to0 & 0xF8)>>3);
		
		//required
		SPICSHIGH();
		SPIWAITCNT();
		
		//Set Chip Select LOW to invoke the command & Transmit the instruction byte: TSC CNT Differential Mode: Y Raw TSC 
		REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_CSHOLDENABLE | BIT_SPICNT_TSCCNT | BIT_SPICLK_2MHZ;
		RWSPICNT(BIT_TSCCNT_START_CTRL|BIT_TSCCNT_POWDOWN_MODE_SEL_DIFFERENTIAL| BIT_TSCCNT_REFSEL_DIFFERENTIAL | BIT_TSCCNT_CONVMODE_12bit | BIT_TSCCNT_TOUCHYPOS);
		uint8 resulty11to5 =RWSPICNT(0);	//0-11-10-9-8-7-6-5
		uint8 resulty4to0 = RWSPICNT(0);	//4-3-2-1-0-0-0-0
		uint16 read_raw_y = ((resulty11to5 & 0x7F) << 5) | ((resulty4to0 & 0xF8)>>3);
		
		//required
		SPICSHIGH();
		SPIWAITCNT();
		
		//Touchscreen Position (pixel TFT X Y Coordinates conversion)
		//Read the X and Y positions in 12bit differential mode, then convert the touchscreen values (adc) to screen/pixel positions (scr), as such:
		//scr.x = (adc.x-adc.x1) * (scr.x2-scr.x1) / (adc.x2-adc.x1) + (scr.x1-1)
		//scr.y = (adc.y-adc.y1) * (scr.y2-scr.y1) / (adc.y2-adc.y1) + (scr.y1-1)
		
		struct sDSFWSETTINGS * DSFWSettingsInst = (struct sDSFWSETTINGS *)&TGDSIPC->DSFWSETTINGSInst;
		
		uint16 adc_x1 = (((DSFWSettingsInst->tsc_adcposx1y112bit[1] << 8) & 0x0f00)) | DSFWSettingsInst->tsc_adcposx1y112bit[0];
		uint16 adc_y1 = (((DSFWSettingsInst->tsc_adcposx1y112bit[3] << 8) & 0x0f00)) | DSFWSettingsInst->tsc_adcposx1y112bit[2];
		
		uint8 scr_x1  = (DSFWSettingsInst->tsc_tsccalx1y18bit[0]);
		uint8 scr_y1  = (DSFWSettingsInst->tsc_tsccalx1y18bit[1]);
		
		uint16 adc_x2 = (((DSFWSettingsInst->tsc_adcposx2y212bit[1]<<8) & 0x0f00)) | DSFWSettingsInst->tsc_adcposx2y212bit[0];
		uint16 adc_y2 = (((DSFWSettingsInst->tsc_adcposx2y212bit[3]<<8) & 0x0f00)) | DSFWSettingsInst->tsc_adcposx2y212bit[2];
		
		uint8 scr_x2  = (DSFWSettingsInst->tsc_tsccalx2y28bit[0]);
		uint8 scr_y2  = (DSFWSettingsInst->tsc_tsccalx2y28bit[1]);
		
		sint32 scrx = (read_raw_x-adc_x1) * (scr_x2-scr_x1) / (adc_x2-adc_x1) + (scr_x1-1);
		sint32 scry = (read_raw_y-adc_y1) * (scr_y2-scr_y1) / (adc_y2-adc_y1) + (scr_y1-1);
		
		TGDSIPC->touchX    = read_raw_x;
		TGDSIPC->touchXpx = scrx;
		TGDSIPC->touchY    = read_raw_y;
		TGDSIPC->touchYpx = scry;
		
		LastTSCPosX = scrx;
		LastTSCPosY = scry;
	}
	else{
		TGDSIPC->touchY    = 0;
		TGDSIPC->touchYpx = 0;
		TGDSIPC->touchX    = 0;
		TGDSIPC->touchXpx = 0;
		LastTSCPosX = 0;
		LastTSCPosY = 0;	
	}
}

#endif
//External 
//relies on doSPIARM7IO() XY Readings
void XYReadScrPos(struct XYTscPos * StouchScrPosInst){
    
    StouchScrPosInst->rawx =   TGDSIPC->touchX;
    StouchScrPosInst->rawy =   TGDSIPC->touchY;
    
    //TFT x/y pixel
    StouchScrPosInst->px   =   TGDSIPC->touchXpx;
    StouchScrPosInst->py   =   TGDSIPC->touchYpx;
    
    StouchScrPosInst->z1   =   TGDSIPC->touchZ1;
    StouchScrPosInst->z2   =   TGDSIPC->touchZ2;
	
}