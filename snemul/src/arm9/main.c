/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006-2007 archeide, All rights reserved. */
/***********************************************************/
/*
 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation; either version 2 of 
 the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, 
 but WITHOUT ANY WARRANTY; without even the implied warranty of 
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 GNU General Public License for more details.
 */

#include <nds.h>
#include <nds/registers_alt.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "gui/gui.h"

#include "fs.h"
#include "snes.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"

#include "ram.h"
#include "m3sd.h"

#include "conf.h"

#include "snemul_str.h"

//#define USE_EMUL


int _offsetY_tab[4] = { 16, 0, 32, 24 };

uint32 screen_mode;
int APU_MAX = 262;
//volatile int h_blank;

IN_ITCM
void Vblank()
{
	GFX.DSFrame++;
	GFX.v_blank=1;

	// FIX APU cycles
#if 0	
	if (/*CFG.Sound_output && */APU.counter > 100 && APU.counter < 261)
	*APU_ADDR_CNT += 261 - APU.counter;
#endif		
	//*APU_ADDR_CNT += 262;
	if (CFG.Sound_output)
	*APU_ADDR_CNT = APU_MAX;
	APU.counter = 0;

	VBLANK_INTR_WAIT_FLAGS |= IRQ_VBLANK;
	REG_IF = IRQ_VBLANK;
}

IN_ITCM
void Hblank()
{
	*APU_ADDR_CMD = 0xFFFFFFFF;

	if (DISP_Y >= 192)
	{
		if (DISP_Y == 192) // We are last scanline, update first line GFX

		{
			PPU_updateGFX(0);
		}
		goto end;
	}

	PPU_updateGFX(DISP_Y);

	//	h_blank=1;
	end:
	VBLANK_INTR_WAIT_FLAGS |= IRQ_HBLANK;
	REG_IF = IRQ_HBLANK;
	*APU_ADDR_CMD = 0;
}

u32 keys;

typedef struct s_Options
{
	uint8 BG3Squish :2;
	uint8 SoundOutput :1;
	uint8 LayersConf :6;
	uint8 TileMode :1;
	uint8 BG_Layer :8;
	uint8 YScroll :2;
	uint8 WaitVBlank :1;
	uint8 SpeedHack :3;
} t_Options;

void applyOptions()
{
	if (!CFG.Sound_output)
		APU_clear();

	if (CFG.LayersConf < 10)
		PPU_ChangeLayerConf(CFG.LayersConf);

	GFX.YScroll = _offsetY_tab[CFG.YScroll];
}

// FIXME: Move me...
uint8 LayersConf[10][4] =
{
{ 0, 1, 2, 3 },
{ 1, 2, 0, 3 },
{ 3, 3, 2, 3 },
{ 3, 3, 3, 3 },
{ 2, 2, 2, 2 },
{ 1, 1, 1, 1 },
{ 0, 0, 0, 0 },
{ 2, 3, 0, 1 },
{ 2, 0, 3, 1 },
{ 2, 1, 0, 3 }, 
};

void PPU_ChangeLayerConf(int i)
{
	CFG.LayersConf = i % 10;
	CFG.LayerPr[0] = LayersConf[CFG.LayersConf][0];
	CFG.LayerPr[1] = LayersConf[CFG.LayersConf][1];
	CFG.LayerPr[2] = LayersConf[CFG.LayersConf][2];
	CFG.LayerPr[3] = LayersConf[CFG.LayersConf][3];
}

void readOptionsFromConfig(char *section)
{
	CFG.BG3Squish = get_config_int(section, "BG3Squish", CFG.BG3Squish) & 3;
	// FIXME 
	GFX.YScroll = get_config_int(section, "YScroll", GFX.YScroll);
	if (GFX.YScroll == 16)
		CFG.YScroll = 0;
	if (GFX.YScroll == 0)
		CFG.YScroll = 1;
	if (GFX.YScroll == 32)
		CFG.YScroll = 2;
	if (GFX.YScroll == 24)
		CFG.YScroll = 3;	
	
	CFG.Scaled = get_config_int(section, "Scaled", CFG.Scaled);
	CFG.Sound_output = get_config_int(section, "Sound", CFG.Sound_output) & 1;
	CFG.BG_Layer = (get_config_int(section, "HDMA", 1)&1) << 7;

	int BG_Layer = get_config_oct(section, "BGLayers", 010111);
	if ((BG_Layer & 7) == 1)
		CFG.BG_Layer |= 1;
	if (((BG_Layer>>3) & 7) == 1)
		CFG.BG_Layer |= 2;
	if (((BG_Layer>>6) & 7) == 1)
		CFG.BG_Layer |= 4;
	if (((BG_Layer>>9) & 7) == 1)
		CFG.BG_Layer |= 8;
	if (((BG_Layer>>12) & 7) == 1)
		CFG.BG_Layer |= 0x10;

	CFG.LayersConf = get_config_int(section, "BGPriorities", CFG.LayersConf);
	if (CFG.LayersConf == 10)
	{
		int BGManualPriority = get_config_oct(section, "BGManualPriority",
				00123);
		CFG.LayerPr[0] = (BGManualPriority) & 3;
		CFG.LayerPr[1] = (BGManualPriority>>3) & 3;
		CFG.LayerPr[2] = (BGManualPriority>>6) & 3;
		CFG.LayerPr[3] = (BGManualPriority>>9) & 3;

		CFG.LayerPr[0] = get_config_int(section, "BG1Pr", CFG.LayerPr[0]) & 3;
		CFG.LayerPr[1] = get_config_int(section, "BG2Pr", CFG.LayerPr[1]) & 3;
		CFG.LayerPr[2] = get_config_int(section, "BG3Pr", CFG.LayerPr[2]) & 3;
		CFG.LayerPr[3] = get_config_int(section, "BG4Pr", CFG.LayerPr[3]) & 3;

	}
	else
		PPU_ChangeLayerConf(CFG.LayersConf);

	CFG.Transparency
			= get_config_int(section, "Transparency", CFG.Transparency);
	CFG.WaitVBlank = get_config_int(section, "Vblank", CFG.WaitVBlank);
	CFG.CPU_speedhack
			= get_config_int(section, "SpeedHacks", CFG.CPU_speedhack);
	CFG.FastDMA = get_config_int(section, "FastDMA", CFG.FastDMA);

	CFG.MouseXAddr = get_config_hex(section, "MouseXAddr", 0);
	CFG.MouseYAddr = get_config_hex(section, "MouseYAddr", 0);
	CFG.MouseMode = get_config_int(section, "MouseMode", 0);
	CFG.MouseXOffset = get_config_int(section, "MouseXOffset", 0);
	CFG.MouseYOffset = get_config_int(section, "MouseYOffset", 0);

	CFG.SoundPortSync = 0;

	int SoundPortSync = get_config_oct(section, "SoundPortSync",
			CFG.SoundPortSync);
	if ((SoundPortSync & 7) == 1)
		CFG.SoundPortSync |= 8;
	if (((SoundPortSync>>3) & 7) == 1)
		CFG.SoundPortSync |= 4;
	if (((SoundPortSync>>6) & 7) == 1)
		CFG.SoundPortSync |= 2;
	if (((SoundPortSync>>9) & 7) == 1)
		CFG.SoundPortSync |= 1;
	if (((SoundPortSync>>12) & 7) == 1)
		CFG.SoundPortSync |= 0x80;
	if (((SoundPortSync>>15) & 7) == 1)
		CFG.SoundPortSync |= 0x40;
	if (((SoundPortSync>>18) & 7) == 1)
		CFG.SoundPortSync |= 0x20;
	if (((SoundPortSync>>21) & 7) == 1)
		CFG.SoundPortSync |= 0x10;

	CFG.TilePriorityBG = get_config_int(section, "TilePriorityBG",
			CFG.TilePriorityBG);
	CFG.BG3TilePriority = get_config_int(section, "BG3TilePriority",
			CFG.BG3TilePriority);
	CFG.Debug2 = get_config_int(section, "BlankTileNumber", CFG.Debug2);
	int SpritePriority = get_config_oct(section, "SpritePriority", 01123);
	CFG.SpritePr[0] = (SpritePriority) & 3;
	CFG.SpritePr[1] = (SpritePriority>>3) & 3;
	CFG.SpritePr[2] = (SpritePriority>>6) & 3;
	CFG.SpritePr[3] = (SpritePriority>>9) & 3;
	
	CFG.MapExtMem = get_config_int(section, "MapExtMem", CFG.MapExtMem);
	
	CFG.AutoSRAM = get_config_int(section, "AutoSRAM", CFG.AutoSRAM);
}

void saveOptionsToConfig(char *section)
{
	set_config_int(section, "BG3Squish", CFG.BG3Squish);
	// FIXME 
	set_config_int(section, "YScroll", GFX.YScroll);
	set_config_int(section, "Sound", CFG.Sound_output);
	
	set_config_int(section, "Scaled", CFG.Scaled);
	//	set_config_int(section, "GFXEngine", CFG.TileMode);
	//	set_config_int(section, "HDMA", CFG.BG_Layer>>7);

	set_config_oct(section, "BGLayers", 5, (CFG.BG_Layer & 1)|((CFG.BG_Layer
			& 2)<<2)|((CFG.BG_Layer & 4)<<4)|((CFG.BG_Layer & 8)<<6)
			|((CFG.BG_Layer & 0x10)<<8));

	set_config_int(section, "BGPriorities", CFG.LayersConf);

	//	set_config_int(section, "Transparency", CFG.Transparency);
	set_config_int(section, "Vblank", CFG.WaitVBlank);
	set_config_int(section, "SpeedHacks", CFG.CPU_speedhack);
	//	set_config_int(section, "FastDMA", CFG.FastDMA);

	/*	set_config_hex(section, "MouseXAddr", 0);
	 set_config_hex(section, "MouseYAddr", 0);
	 set_config_int(section, "MouseMode", 0);
	 set_config_int(section, "MouseXOffset", 0);
	 set_config_int(section, "MouseYOffset", 0);*/

	//	set_config_oct(section, "SoundPortSync", CFG.SoundPortSync);

	/*	set_config_int(section, "TilePriorityBG", CFG.TilePriorityBG);
	 set_config_int(section, "BG3TilePriority", CFG.BG3TilePriority);
	 set_config_int(section, "BlankTileNumber", CFG.Debug2);
	 set_config_oct(section, "SpritePriority", 01123);*/
	
	set_config_int(section, "AutoSRAM", CFG.AutoSRAM);
	save_config_file();
}

// FIXME : fix layersconf

void packOptions(uint8 *ptr)
{
	t_Options *opt = (t_Options *)ptr;

	opt->BG3Squish = CFG.BG3Squish;
	opt->SoundOutput = CFG.Sound_output;
	if (CFG.LayersConf == 0)
		opt->LayersConf = 0x24; // 0/1/2
	else
		opt->LayersConf = CFG.LayerPr[0] | (CFG.LayerPr[1] << 2)
				| (CFG.LayerPr[2] << 4);
//	opt->TileMode = CFG.TileMode;
	opt->BG_Layer = CFG.BG_Layer;
	opt->YScroll = CFG.YScroll;
	opt->WaitVBlank = CFG.WaitVBlank;
	opt->SpeedHack = CFG.CPU_speedhack;
}

void unpackOptions(int version, uint8 *ptr)
{
	t_Options *opt = (t_Options *)ptr;

	if (version == 1)
		CFG.BG3Squish = 2-opt->BG3Squish;
	else
		CFG.BG3Squish = opt->BG3Squish;
	CFG.Sound_output = opt->SoundOutput;
	if (version == 1)
		CFG.LayersConf = opt->LayersConf;
	else
	{
		if (opt->LayersConf == 0x24) // 0/1/2 == automatic layer
		{
			CFG.LayersConf = 0;
		}
		else
		{
			CFG.LayersConf = 10;
			CFG.LayerPr[0] = opt->LayersConf&3;
			CFG.LayerPr[1] = (opt->LayersConf>>2)&3;
			CFG.LayerPr[2] = (opt->LayersConf>>4)&3;
			CFG.LayerPr[3] = 3;
		}
	}
/*	if (version == 1)
		CFG.TileMode = 0; // Force line by line mode
	else
		CFG.TileMode = opt->TileMode;*/
	CFG.BG_Layer = opt->BG_Layer;
	CFG.YScroll = opt->YScroll;
	CFG.WaitVBlank = opt->WaitVBlank;
	CFG.CPU_speedhack = opt->SpeedHack;

	applyOptions();
}

int checkConfiguration(char *name, int crc)
{
	// Check configuration file
	readOptionsFromConfig("Global");

	char *section= NULL;
	if (is_section_exists(SNES.ROM_info.title))
	{
		section = SNES.ROM_info.title;
	}
	else if (is_section_exists(FS_getFileName(name)))
	{
		section = FS_getFileName(name);
	}
	else if (section = find_config_section_with_hex("crc", crc))
	{
	}
	else if (section = find_config_section_with_string("title2", SNES.ROM_info.title))
	{
	}
	else if (section = find_config_section_with_hex("crc2", crc))
	{
	}
	else if (section = find_config_section_with_string("title3", SNES.ROM_info.title))
	{
	}
	else if (section = find_config_section_with_hex("crc3", crc))
	{
	}
	else if (section = find_config_section_with_string("title4", SNES.ROM_info.title))
	{
	}
	else if (section = find_config_section_with_hex("crc4", crc))
	{
	}

	if (section != NULL)
	{
		GUI_printf("Section : %s\n", section);
		readOptionsFromConfig(section);
	}
}

int loadROM(char *name, int confirm)
{
	int size;
	char romname[100];
	int ROMheader;
	char *ROM;
	int crc;

	// Save SRAM of previous game first
	saveSRAM();

	GUI_clear();
	/*	if (ROM && FS_shouldFreeROM())
	 free(ROM);*/
	CFG.LargeROM = 0;
	strcpy(romname, CFG.ROMPath);
	if (CFG.ROMPath[strlen(CFG.ROMPath)-1] != '/')
		strcat(romname, "/");
	strcat(romname, name);
	strcpy(CFG.ROMFile, romname);

	GUI_printf("Loading %s...\n", romname);

	void *ptr = malloc(4);
	GUI_printf("ptr=%p...\n", ptr);
	free(ptr);

	mem_clear_paging(); // FIXME: move me...

	ROM = (char *) SNES_ROM_ADDRESS;

	size = FS_getFileSize(romname);
	ROMheader = size & 8191;
	if (ROMheader != 0&& ROMheader != 512)
		ROMheader = 512;

#ifndef USE_GBFS	
	if (size-ROMheader > ROM_MAX_SIZE)
	{
		// Large ROM, memory pagging enabled
		if (size <= CFG.ExtRAMSize)
		{
			GUI_printf("Use External RAM\n");
			if (CFG.MapExtMem) // Use External RAM for mapping high addresses of Large ROM
				FS_loadROMInExtRAM(ROM-ROMheader, romname, ROM_MAX_SIZE+ROMheader, size);
			else // Use External RAM to load pages
				FS_loadROMInExtRAM(ROM-ROMheader, romname, ROM_STATIC_SIZE+ROMheader, size);
		}
		else
			FS_loadROMForPaging(ROM-ROMheader, romname, ROM_STATIC_SIZE+ROMheader);
		CFG.LargeROM = 1;
		crc = crc32(0, ROM, ROM_STATIC_SIZE);
		GUI_printf("Large ROM detected. CRC(1Mb) = %08x\n", crc);
	}
	else
#endif	
	{
		FS_loadROM(ROM-ROMheader, romname);
		CFG.LargeROM = 0;
		crc = crc32(0, ROM, size-ROMheader);
		GUI_printf("CRC = %08x\n", crc);
	}

	ROM += 0x400000; // Protected ROM

	changeROM(ROM-ROMheader, size);

	checkConfiguration(name, crc);

	return 0;
}

#define DEBUG_BUF ((char *)0x27FE200)

int selectSong(char *name)
{
	char spcname[100];

	strcpy(spcname, CFG.ROMPath);
	if (CFG.ROMPath[strlen(CFG.ROMPath)-1] != '/')
		strcat(spcname, "/");
	strcat(spcname, "/");
	strcat(spcname, name);
	strcpy(CFG.Playlist, spcname);
	CFG.Jukebox = 1;
	CFG.Sound_output = 0;
	APU_stop();
	if (FS_loadFile(spcname, APU_RAM_ADDRESS-0x100, 0x10200) < 0)
		return -1;
	APU_playSpc();
	// Wait APU init
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	//	iprintf("\nDBG: %s", DEBUG_BUF);	
	return 0;
}

extern void IntrHandlerAsm();

extern CPU_goto();
#ifdef ASM_OPCODES
extern CPU_init();
extern CPU_goto2();
#endif

extern char logbuf[];

extern uint32 AsmDebug[16];

void exception_handler()
{
	int i;
	u32 currentMode = getCPSR() & 0x1f;
	u32 thumbState = ((*(u32*)0x027FFD90) & 0x20);
	u32 savedPC = *(u32*)0x027FFD98;
	u32 exceptionAddress;

	static const char *registerNames[] =
	{ "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
			"r12", "sp", "lr", "pc", 
};

exceptionRegisters[15] = savedPC;

if (currentMode == 0x17)
{
	/* Data abort- actual faulting instruction was 8 bytes earlier */
	exceptionAddress = savedPC - 8;
}
else
{
	/*
	 * XXX: Assuming invalid instruction error?
	 * Place the fault at the previous instruction.
	 */
	exceptionAddress = savedPC - (thumbState ? 2 : 4);
}

iprintf(
		"r0=%08x r1=%08x r2=%08x r3=%08x\n",
		AsmDebug[0],AsmDebug[1],AsmDebug[2],AsmDebug[3] );

iprintf("\nException %02x @ %08x (%s)\n",
		currentMode, exceptionAddress,
		thumbState ? "Thumb" : "ARM");

/*    for (i = 0; i < 8; i++) {
 LOG(" %-03s %08x ", registerNames[i], exceptionRegisters[i]);
 LOG(" %-03s %08x \n", registerNames[i+8], exceptionRegisters[i+8]);
 }*/
}

/*
size_t _myFAT_directory_ucs2tombs(unsigned char* dst, const unsigned short* src, size_t len) {
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
#if 0
	_consolePrintf("---%02x%02x%02x%02x%02x%02x%02x%02x%02x %04x%04x\n",
dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],dst[6],dst[7],dst[8],src[0],src[1]);
#endif
	return j;
}
*/
//---------------------------------------------------------------------------------
int main(void)
{
//	setExceptionHandler(exception_handler);
	initMem();

	//---------------------------------------------------------------------------------
	touchPosition touchXY;

	*APU_ADDR_CNT = 0;
	*APU_ADDR_ANS = *APU_ADDR_CMD = 0;

	resetMemory2_ARM9();
	
	powerSET(POWER_ALL_2D | POWER_SWAP_LCDS);
	
#ifndef TIMER_Y	
	TIMER3_CR &= ~TIMER_ENABLE; // not strictly necessary if the timer hasn't been enabled before
	TIMER3_DATA = 0;
	TIMER3_CR = TIMER_ENABLE | TIMER_DIV_1;
#endif

	ARM7_fifo_init();

	screen_mode = 0;
	videoSetMode(0); //not using the main screen
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE); //sub bg 0 will be used to print text
	BG0_CR = BG1_CR = BG2_CR = BG3_CR = 0;

	/* 256Ko for Tiles (SNES: 32-64Ko) */
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);

	/* 128Ko (+48kb) for sub screen / GUI */
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);

	// Some memory for ARM7 (128 Ko!)
	vramSetBankD(VRAM_D_ARM7_0x06000000);

	/* 80Ko for Sprites (SNES : 32-64Ko) */
	vramSetBankE(VRAM_E_MAIN_SPRITE); // 0x6400000

	vramSetBankF(VRAM_F_MAIN_SPRITE);

	vramSetBankG(VRAM_G_BG_EXT_PALETTE);

	/* 48ko For CPU */

	//vramSetBankG(VRAM_G_LCD); 
	vramSetBankH(VRAM_H_LCD);
	vramSetBankI(VRAM_I_LCD);

	/* 32 first kilobytes for MAP */
	/* remaning memory for tiles */

	GUI_init();	
	
	GUI_setLanguage(PersonalData->_user_data.language);
	
#ifndef DSEMUL_BUILD	
	GUI.printfy = 32;
	GUI_align_printf(GUI_TEXT_ALIGN_CENTER, SNEMULDS_TITLE);
	GUI_align_printf(GUI_TEXT_ALIGN_CENTER, SNEMULDS_SUBTITLE);
    GUI.printfy += 32; // FIXME
	GUI_align_printf(GUI_TEXT_ALIGN_CENTER, _STR(4));
#endif	
	
	initSNESEmpty();

	// Clear "HDMA"
	int i;
	for (i = 0; i < 192; i++)
		GFX.lineInfo[i].mode = -1;

	IRQ_HANDLER = &IntrHandlerAsm;

	REG_IE = IRQ_VBLANK | IRQ_HBLANK;
	DISP_SR = DISP_VBLANK_IRQ | DISP_HBLANK_IRQ;

	REG_IF = ~0;
	REG_IME = 1;

	swiWaitForVBlank();
	swiWaitForVBlank();

	PrecalculateCalibrationData();

	GUI_printf(_STR(IDS_INITIALIZATION));
	if (FS_init())
		GUI_printf(_STR(IDS_FS_SUCCESS));
	else
		GUI_printf(_STR(IDS_FS_FAILED));	
	
#ifdef USE_EXTRAM	
/*	if (ram_init(DETECT_RAM))
	{
		GUI_printf("External RAM detected !!\n");
		CFG.ExtRAMSize = 32*1024*1024;
		
	} else*/
	CFG.ExtRAMSize = FS_extram_init(DETECT_RAM);
#else
	CFG.ExtRAMSize = 0;
#endif	
	//GUI_printf("zzz");
#if 0
	{	char *p = malloc(10);
		iprintf("RAM = %p last malloc = %p", SNESC.RAM, p);
	}
	/* TOUCH SCREEN TEST */
	while (1)
	{
		int i;

		mytouchPosition mytouchXY;

		scanKeys();
		keys = keysHeld();
		//		mytouchXY=mytouchReadXY();

		GUI_printf(0, 2, "keys = %x %d\n", keys, SNES.h_blank);
		if (keys & KEY_TOUCH)
		{
			touchXY=superTouchReadXY();
			GUI_printf(0, 3, "x = %d y = %d       ", touchXY.px, touchXY.py);
			//		waitReleaseTouch();	
		}

		if ((keys & KEY_START))
		break;
	}
#endif	


#ifndef	DSEMUL_BUILD	
	//for (i = 0; i < 100; i++)
	//	swiWaitForVBlank();
#endif	
	GUI_printf("Load conf1");
	// Load SNEMUL.CFG
	set_config_file("snemul.cfg");

	CFG.ROMPath = get_config_string(NULL, "ROMPath", GAMES_DIR);
	GUI_printf("Load conf2");
	readOptionsFromConfig("Global");
	GUI_printf("Load conf3");
	GUI_getConfig();	
	GUI_printf("Load conf4");

	char *ROMfile = GUI_getROM(CFG.ROMPath);

	loadROM(ROMfile, 0);

	//GUI_deleteROMSelector(); // Should also free ROMFile

#if 0
#ifdef ASM_OPCODES    
	iprintf("CPU_init=%p\n", CPU_init);
	iprintf("CPU_goto=%p\n", CPU_goto2);
#endif      
	iprintf("logbuf=%p\n", logbuf);

	while (1)
	{
		scanKeys();
		keys = keysHeld();
		if ((keys & KEY_START))
		break;
	}
#endif	
	GUI_printf("Start GUI");
	GUI_createMainMenu();

	/*		GUI.log = 1;		
	 consoleClear();*/

	while (1)
	{
		if (!SNES.Stopped)
			go();
		else
			swiWaitForVBlank();
		if (/*!CFG.mouse && */REG_POWERCNT & POWER_SWAP_LCDS)
			GUI_update();
		if (keys & KEY_LID)
		{
			saveSRAM();
			APU_pause();
			//			APU_stop();
			/* hinge is closed */
			/* power off everything not needed */
			powerOFF(POWER_ALL) ;
			/* set system into sleep */
			while (keys & KEY_LID)
			{
				swiWaitForVBlank();
				scanKeys();
				keys = keysHeld();
			}
			/* wait a bit until returning power */
			/* power on again */
			powerON(POWER_ALL_2D) ;
			/* set up old irqs again */

			APU_pause();
		}
	}

	return 0;
}
