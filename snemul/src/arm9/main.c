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

#include "gui.h"
#include "fs.h"
#include "snes.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"

#include "m3sd.h"

volatile int frame = 0;
int timer_0 = 55;

extern unsigned short PC;
volatile int v_blank;
//volatile int h_blank;

//---------------------------------------------------------------------------------
void Vblank() {
//---------------------------------------------------------------------------------
	frame++;
	v_blank=1;
	
	// FIX APU cycles
	if (CFG.Sound_output && APU.counter > 100 && APU.counter < 261)
		*APU_ADDR_CNT += 261 - APU.counter;  
	APU.counter = 0; 
	
	VBLANK_INTR_WAIT_FLAGS |= IRQ_VBLANK;
    REG_IF = IRQ_VBLANK;
}

static int _offsetY_tab[3] = { 16, 0, 32 };
//---------------------------------------------------------------------------------
void Hblank() {
//---------------------------------------------------------------------------------
	if (DISP_Y >= 192)
	{
		if (DISP_Y == 192)
		{
			/* FIXME: Vblank simulator */
	/*		if (CFG.Sound_output && APU.counter > 100 && APU.counter < 261)
				*APU_ADDR_CNT += 261 - APU.counter;  
			APU.counter = 0;*/
		}
		
		VBLANK_INTR_WAIT_FLAGS |= IRQ_HBLANK;	
    	REG_IF = IRQ_HBLANK;			
	}
	
	if (SNES.lineRegisters[DISP_Y+GFX.YScroll].Mode == 7)
	{
		switchToMode7();
		
		t_lineRegisters	*lr = &SNES.lineRegisters[DISP_Y+GFX.YScroll];
		
	   	BG3_XDX = lr->A; 
	   	BG3_XDY = lr->B;
	  	BG3_YDX = lr->C; 
	  	BG3_YDY = lr->D;
		BG3_CX = lr->CX;
		BG3_CY = lr->CY;
	}
	
//	h_blank=1;
	VBLANK_INTR_WAIT_FLAGS |= IRQ_HBLANK;	
    REG_IF = IRQ_HBLANK;	
}


int screen_mode;	

u32 keys;
		
char	*ROM;

t_list	*ROM_list;
t_list	*SPC_list;
t_list	*mainMenu;
t_list	*optionsMenu;

char *options_0[] = 
	{ "Mode 3 : squish more", "Mode 3 : squish", "Mode 3 : normal" };
char *YScroll_mode_0[] = 
	{ "YScroll : middle", "YScroll : top", "YScroll : bottom" };
char *SpeedHacksChoices[] =
	{ "No speed hacks", "Medium speed hacks", "Full speed hacks" };


void updateOptions()
{
	if (CFG.Sound_output)
		GUI_setItem(optionsMenu, 1, "Sound on", 0, 1);
    else		
    	GUI_setItem(optionsMenu, 1, "Sound off", 0, 1);
}

void initGUI()
{
	mainMenu = GUI_createList(8);
	GUI_setItem(mainMenu, 0, "Select ROM", 0, 0);
	GUI_setItem(mainMenu, 1, "Options", 0, 0);
#ifndef USE_GBFS	
	GUI_setItem(mainMenu, 2, "Load state", 0, 0);
	GUI_setItem(mainMenu, 3, "Save state", 0, 0);
#endif	
	GUI_setItem(mainMenu, 4, "Reset", 0, 0);
	GUI_setItem(mainMenu, 5, "Save SRAM", 0, 0);
	GUI_setItem(mainMenu, 6, "SPC Jukebox", 0, 0);
	GUI_setItem(mainMenu, 7, "Debug", 0, 0);
	
	//optionsMenu = GUI_createDialog(4);
	optionsMenu = GUI_createList(8);
	GUI_setItem(optionsMenu, 0, options_0[2], 0, 1);
	GUI_setItem(optionsMenu, 1, "Sound on", 0, 1);
//	GUI_setItem(optionsMenu, 1, "BG1 on", 0, 1);
	GUI_setItem(optionsMenu, 2, "BG2 on", 0, 1);
	GUI_setItem(optionsMenu, 3, "BG3 on,", 0, 1);
	GUI_setItem(optionsMenu, 4, "Sprites on", 0, 1);
	GUI_setItem(optionsMenu, 5, YScroll_mode_0[0], 0, 1);
	GUI_setItem(optionsMenu, 6, "No vblank", 0, 1);
	GUI_setItem(optionsMenu, 7, "Speed hack", 0, 1);
	updateOptions();
}

void	waitReleaseTouch()
{
	do
	{
		swiWaitForVBlank();
		scanKeys();
		keys = keysHeld();
	}
	while (keys & KEY_TOUCH);		
}

void	waitPressTouch()
{
	do
	{
		swiWaitForVBlank();
		scanKeys();
		keys = keysDown();
	}
	while (!(keys & KEY_TOUCH));		
}

void	waitHeldTouch()
{
	do
	{
		swiWaitForVBlank();
		scanKeys();
		keys = keysHeld();
	}
	while (!(keys & KEY_TOUCH));		
}

/************* TO PUT in include ********* */
typedef struct mytouchPosition {
	u8 error;
	u8 touched;
	int16	x;
	int16	y;
	int16	px;
	int16	py;
	int16	z1;
	int16	z2;
} mytouchPosition;

mytouchPosition mytouchReadXY();
/************* TO PUT in include ********* */

//#define USE_EMUL
touchPosition superTouchReadXY()
{
#ifdef USE_EMUL // How to detect Dualis emulator ???
	touchPosition touchXY;
	
//	touchXY = touchReadXY();
	touchXY = touchReadXY();
#else
	touchPosition touchXY;
	mytouchPosition mtouchXY;
		
    mtouchXY = mytouchReadXY();	// It seems first measure is not already updayed
    mtouchXY = mytouchReadXY();	
    mtouchXY = mytouchReadXY();	
	touchXY.px = mtouchXY.px;
	touchXY.py = mtouchXY.py;
#endif	
	return touchXY;
}

int	loadROM(char *name, int confirm)
{
	int	size;
	char romname[100];
	
	consoleClear();				
	if (ROM && FS_shouldFreeROM())
		free(ROM);
	CFG.LargeROM = 0;
	strcpy(romname, GAMES_DIR);
	strcat(romname, name);
	strcpy(CFG.ROMFile, romname);
iprintf("Name : %s", romname); 

   if (SNES.ROM_info)
   {
   		free(SNES.ROM_info);
   		SNES.ROM_info = NULL;
   }
	mem_clear_paging(); // FIXME: move me...
	ROM = FS_loadROM(romname, &size);
iprintf("ROM : %p %d", ROM, size);	
#ifndef USE_GBFS	
	if (!ROM)
	{
		ROM = FS_loadROMForPaging(romname, &size);
		CFG.LargeROM = 1;
	}
#endif	
	if (!ROM)
	{	
		return -1;
	}
	
	changeROM(ROM, size);
	
	if (confirm)
	while (1)
	{
		swiWaitForVBlank();		
		scanKeys();
		int held = keysHeld();
		if ((held & KEY_START)) 
		{
		    return 0;
		}
		if ((held & KEY_SELECT)) 
			return -1;
	}
	return 0;	
}

	#define DEBUG_BUF ((char *)0x27FE200)

int	selectSong(char *name)
{
	char spcname[100];
	
	strcpy(spcname, GAMES_DIR);
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

void	handle_saveState()
{
	t_list	*StatesMenu;
	int		i;
	touchPosition touchXY;
	int res;	
	char stateName[64];
	char stateFile[100];
	
	strcpy(stateFile, CFG.ROMFile);
	strcpy(strrchr(stateFile, '.'), ".sml");
	
	StatesMenu = GUI_createList(8);
	for (i = 0; i < 8; i++)
	{
	  	if (get_snapshot_name(stateFile, i, stateName))
	  		GUI_setItem(StatesMenu, i, stateName, 0, 0);
	  	else	  	
	  		GUI_setItem(StatesMenu, i, "--Empty--", 0, 1);
	}	

	GUI_displayMenuTitle("-= Save State =-", StatesMenu);

    waitReleaseTouch();			
	waitPressTouch();
	touchXY=superTouchReadXY();	
	
	res = GUI_checkMenu(StatesMenu, touchXY.px, touchXY.py);
	
	if (res >= 0)
	{
		sprintf(stateName, "%04d/%02d/%02d %02d:%02d",
			2000+IPC->time.rtc.year, IPC->time.rtc.month+1, IPC->time.rtc.day+1,
			IPC->time.rtc.hours, IPC->time.rtc.minutes);			
		
		CPU_pack();	
		write_snapshot(stateFile, res, stateName);
	}

	free(StatesMenu);
	waitReleaseTouch();	
	GUI_displayMenu(mainMenu);	
	
}

void	handle_loadState()
{
	t_list	*StatesMenu;
	int		i;
	touchPosition touchXY;
	int res;	
	char stateName[64];	
	char stateFile[100];
	
	strcpy(stateFile, CFG.ROMFile);
	strcpy(strrchr(stateFile, '.'), ".sml");
	
	StatesMenu = GUI_createList(8);
	for (i = 0; i < 8; i++)
	{
	  	if (get_snapshot_name(stateFile, i, stateName))
	  		GUI_setItem(StatesMenu, i, stateName, 0, 0);
	  	else	  		
	  		GUI_setItem(StatesMenu, i, "--Empty--", 0, 1);
	 }	

	GUI_displayMenuTitle("-= Load State =-", StatesMenu);

    waitReleaseTouch();			
	waitPressTouch();
	touchXY=superTouchReadXY();	
	
	res = GUI_checkMenu(StatesMenu, touchXY.px, touchXY.py);
	
	if (res >= 0 && StatesMenu->items[res].info == 0)
	{
		read_snapshot(stateFile, res);
		CPU_unpack();
		SNES_update();		
		PPU_update();		
//		PPU_reset();
	}

	free(StatesMenu);
	waitReleaseTouch();	
	GUI_displayMenu(mainMenu);	
	
}

void	SPC_jukebox_choose_real_time()
{
	touchPosition touchXY;
	int	res;
		
	touchXY=superTouchReadXY();
		
	res = GUI_checkList(SPC_list, touchXY.px, touchXY.py, 1);
	
	if (res == -2)
	{
		waitReleaseTouch();
		CFG.Jukebox = 0;
		GUI_displayMenu(mainMenu);
		return;
	}	
	if (res >= 0)
	{
		waitReleaseTouch();
		if (!selectSong(SPC_list->items[res].str))
		{
			GUI_displayList(SPC_list, 1);
			return;
		}
	}	
}

void	handleMenu()
{
	int res;
	touchPosition touchXY;

	scanKeys();
	keys = keysHeld();
	if (!(keys & KEY_TOUCH))
		return;

	touchXY=superTouchReadXY();
	
	if (GUI.log)
	{
		waitReleaseTouch();
		GUI_displayMenu(mainMenu);
		GUI.log = 0;
		return;
	}
	
	if (CFG.Jukebox == 1)
	{
		SPC_jukebox_choose_real_time();
		return;
	}
	
	res = GUI_checkMenu(mainMenu, touchXY.px, touchXY.py);
	if (res == 0)
	{
		APU_pause();		
		/* SELECT ROM */
		waitReleaseTouch();
	
		ROM_list->curs = 0;
		GUI_displayList(ROM_list, 1);
		while (1)
		{
			waitHeldTouch();
			touchXY=superTouchReadXY();
			
			res = GUI_checkList(ROM_list, touchXY.px, touchXY.py, 1);
			
			if (res == -2)
			{
				GUI_displayMenu(mainMenu);
				APU_pause();
				break;
			}
			
			if (res >= 0)
			{
				if (!loadROM(ROM_list->items[res].str, 1))
				{
					GUI_displayMenu(mainMenu);
//					APU_pause();
					return;
				}
				GUI_displayList(ROM_list, 1);
			}
		}
	}
	if (res == 1)
	{
		APU_pause();
		/* OPTIONS */
		updateOptions();
		GUI_displayMenuBack(optionsMenu);
		while (1)
		{
        waitReleaseTouch();			
		waitPressTouch();
		touchXY=superTouchReadXY();	
		
		res = GUI_checkMenu(optionsMenu, touchXY.px, touchXY.py);
		switch (res)
		{
		case 0:
			CFG.BG3Squish++;
			CFG.BG3Squish %= 3; 
			strcpy(optionsMenu->items[0].str, options_0[CFG.BG3Squish]);
			break;
		case 1:			
			CFG.Sound_output ^= 1;
			if (CFG.Sound_output)
				strcpy(optionsMenu->items[1].str, "Sound on");
			else			
			{
				APU_clear();
				strcpy(optionsMenu->items[1].str, "Sound off");
			}
			break;			
/*		case 1:			
			CFG.BG_Layer ^= 0x01;
			if (CFG.BG_Layer & 0x01)
				strcpy(optionsMenu->items[1].str, "BG1 on");
			else			
				strcpy(optionsMenu->items[1].str, "BG1 off");
			break;*/				
		case 2:								
			CFG.BG_Layer ^= 0x02;
			if (CFG.BG_Layer & 0x02)
				strcpy(optionsMenu->items[2].str, "BG2 on");
			else			
				strcpy(optionsMenu->items[2].str, "BG2 off");
			break;				
		case 3:								
			CFG.BG_Layer ^= 0x04;
			if (CFG.BG_Layer & 0x04)
				strcpy(optionsMenu->items[3].str, "BG3 on");
			else			
				strcpy(optionsMenu->items[3].str, "BG3 off");
			break;
		case 4:		
			CFG.BG_Layer ^= 0x10;
			if (CFG.BG_Layer & 0x10)
				strcpy(optionsMenu->items[4].str, "Sprites on");
			else			
				strcpy(optionsMenu->items[4].str, "Sprites off");
			break;								
		case 5:		
			CFG.YScroll++;
			CFG.YScroll %= 3; 
			strcpy(optionsMenu->items[5].str, YScroll_mode_0[CFG.YScroll]);
			GFX.YScroll = _offsetY_tab[CFG.YScroll];
			break;			
		case 6:		
			CFG.WaitVBlank ^= 1;
			if (CFG.WaitVBlank)
				strcpy(optionsMenu->items[6].str, "Wait vblank");
			else
				strcpy(optionsMenu->items[6].str, "No vblank");
			break;				
		case 7:		
			CFG.CPU_speedhack++;
			CFG.CPU_speedhack %= 3;
			strcpy(optionsMenu->items[7].str, SpeedHacksChoices[CFG.CPU_speedhack]);
			break;				
		default:
			waitReleaseTouch();
			APU_pause();			
			GUI_displayMenu(mainMenu);			
			return;
		}
		
		GUI_displayMenuBack(optionsMenu);		
		}

	}

#ifndef USE_GBFS
	if (res == 2)
	{
		APU_pause();
		handle_loadState();
		APU_pause();
		return;
	}

	if (res == 3)
	{
		APU_pause();
		handle_saveState();
		APU_pause();
		return;
	}
#endif	

	if (res == 4)
	{
		reset_SNES();
		loadSRAM();
		waitReleaseTouch();		
		GUI_displayMenu(mainMenu);
	}

	if (res == 5)
	{
		saveSRAM();
		waitReleaseTouch();
		return;
	}

	if (res == 6)
	{
//		APU_pause();
	
		waitReleaseTouch();
		GUI_displayList(SPC_list, 1);	
		CFG.Jukebox = 1;
	}


	if (res == 7)
	{
		GUI.log = 1;
		waitReleaseTouch();		
		consoleClear();
		LOG("Reset PPU...\n");
		PPU_reset();		
	}

		
//	iprintf("\x1b[22;0Hres = %d", res);

}
	
extern void IntrHandlerAsm();
	
 extern CPU_goto();
 #ifdef ASM_OPCODES
 extern CPU_init();	
 extern CPU_goto2();
 #endif
 
 extern char logbuf[];
	
//---------------------------------------------------------------------------------
int main(void) 
{
//---------------------------------------------------------------------------------
	touchPosition touchXY;

    *APU_ADDR_CNT = 0;
    *APU_ADDR_ANS = *APU_ADDR_CMD = 0; 
	resetMemory2_ARM9();

#ifndef TIMER_Y	
    TIMER3_CR &= ~TIMER_ENABLE; // not strictly necessary if the timer hasn't been enabled before
    TIMER3_DATA = 0;
    TIMER3_CR = TIMER_ENABLE | TIMER_DIV_1;
#endif

	ARM7_fifo_init();
    
//	screen_mode = DISPLAY_SCREEN_OFF;
	screen_mode = MODE_0_2D | DISPLAY_SPR_2D;
	videoSetMode(screen_mode);	//not using the main screen
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);	//sub bg 0 will be used to print text
	
#if 0	
	/* 256Ko for Tiles (SNES: 32Ko) */
	/* 128Ko for Sprites (SNES : 32Ko) */	
	vramSetBankA(VRAM_A_MAIN_BG_0x6000000);
	vramSetBankB(VRAM_B_MAIN_SPRITE | VRAM_OFFSET(0)); // 0x6400000
	vramSetBankC(VRAM_C_MAIN_BG_0x6020000);
	vramSetBankD(VRAM_D_ARM7); // Some memory for ARM7 (128 Ko!)

	/* 48 Ko For Sub screen */ 
	vramSetBankH(VRAM_H_SUB_BG); 
	vramSetBankI(VRAM_I_SUB_BG);

#else
	/* 256Ko for Tiles (SNES: 32Ko) */

	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	
	/* 128Ko (+48kb) for sub screen */ 
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	
	// Some memory for ARM7 (128 Ko!)
	vramSetBankD(VRAM_D_ARM7_0x06000000); 
	
	/* 96Ko for Sprites (SNES : 32Ko) */
	vramSetBankE(VRAM_E_MAIN_SPRITE);	// 0x6400000
	vramSetBankF(VRAM_F_MAIN_SPRITE);
	vramSetBankG(VRAM_G_MAIN_SPRITE);
	
	/* 48ko For CPU */
	
	//vramSetBankG(VRAM_G_LCD); 
	vramSetBankH(VRAM_H_LCD); 
	vramSetBankI(VRAM_I_LCD);
#endif	
	

	/* 32 first kilobytes for MAP */
	/* remaning memory for tiles */
	SUB_BG0_CR = BG_MAP_BASE(4);
	
	BG_PALETTE_SUB[255] = RGB15(31,31,31);	//by default font will be rendered with color 255

	//consoleInit() is a lot more flexible but this gets you up and running quick
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(4), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);

	IRQ_HANDLER = &IntrHandlerAsm;

    REG_IE = IRQ_VBLANK | IRQ_HBLANK;
    DISP_SR = DISP_VBLANK_IRQ | DISP_HBLANK_IRQ;
    
    REG_IF = ~0;
    REG_IME = 1;

	iprintf(SNEMULDS_TITLE);

	swiWaitForVBlank();
	swiWaitForVBlank();

    iprintf("%p", CPU_goto);
/*	iprintf("\n%x %x %x %x %x %x %x %x", 
	PersonalData->calX1, PersonalData->calY1,
	PersonalData->calX1px, PersonalData->calY1px,
		PersonalData->calX2, PersonalData->calY2,
	PersonalData->calX2px, PersonalData->calY2px
	
	);*/

	PrecalculateCalibrationData();

	initSNESEmpty();

	initGUI();
		
		
	// hack: Dualis detection 
	if (PersonalData->calY2 == 0xbf9 && PersonalData->calY1 == 0x343)
	{
		PersonalData->calY1 = 0x240;//0x1c0;
		PersonalData->calY2 = 0xB40;
		
		CFG.Sound_output = 0; // disable sound in dualis
	}
	
#if 0
	{ char *p = malloc(10);
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
	FS_init();
	
    ROM_list = FS_getDirectoryList(GAMES_DIR, "SMC|SFC|SWC|FIG");
    SPC_list = FS_getDirectoryList(GAMES_DIR, "SPC");    
    if (ROM_list->nb_items == 0)	 
	{
		iprintf("No files found.\n"
		        "Please put ROMs in SNES directory of your CF/SD card\n");
		return 0;
	}	
	
	if (ROM_list->nb_items > 1)
	{
		GUI_displayList(ROM_list, 0);		 
		while (1) 
		{
			swiWaitForVBlank();
			scanKeys();
			keys = keysHeld();
			touchXY=superTouchReadXY();
			if (keys & KEY_TOUCH) 
			{
				int res = GUI_checkList(ROM_list, touchXY.px, touchXY.py, 0);
				if (res >= 0)
				{
					if (loadROM(ROM_list->items[res].str, 1) < 0)
					{
						GUI_displayList(ROM_list, 0);
					}
					else
						break;
				}
			}
		}
	} else
		loadROM(ROM_list->items[0].str, 0);

    iprintf("Start!!!\n");
    
    
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
   
   GUI_displayMenu(mainMenu);

	while (1) 
	{
		go();
		handleMenu();	
	}
		
	return 0;
}
