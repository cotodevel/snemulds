/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include <nds.h>
#include <video.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "gui.h"
#include "fs.h"
#include "snes.h"
#include "gfx.h"
#include "cfg.h"

#include "m3sd.h"

volatile int frame = 0;
int timer_0 = 0;

extern unsigned short PC;
volatile int v_blank;

//---------------------------------------------------------------------------------
void Vblank() {
//---------------------------------------------------------------------------------
	frame++;
	v_blank=1;
}



int screen_mode;	

u32 keys;
		
char	*ROM;

t_list	*list;
t_list	*mainMenu;
t_list	*optionsMenu;

char *options_0[] = 
	{ "Mode 3 : squish more", "Mode 3 : squish", "Mode 3 : normal" };
char *YScroll_mode_0[] = 
	{ "YScroll : middle", "YScroll : top", "YScroll : bottom" };


void initGUI()
{
	mainMenu = GUI_createList(7);
	GUI_setItem(mainMenu, 0, "Select ROM", 0, 0);
	GUI_setItem(mainMenu, 1, "Options", 0, 0);
	GUI_setItem(mainMenu, 2, "Load state", 0, 0);
	GUI_setItem(mainMenu, 3, "Write state", 0, 0);
	GUI_setItem(mainMenu, 4, "Reset", 0, 0);
	GUI_setItem(mainMenu, 5, "Save SRAM", 0, 0);
	GUI_setItem(mainMenu, 6, "Debug", 0, 0);
	
	//optionsMenu = GUI_createDialog(4);
	optionsMenu = GUI_createList(8);
	GUI_setItem(optionsMenu, 0, options_0[2], 0, 1);
	GUI_setItem(optionsMenu, 1, "BG1 on", 0, 1);
	GUI_setItem(optionsMenu, 2, "BG2 on", 0, 1);
	GUI_setItem(optionsMenu, 3, "BG3 on,", 0, 1);
	GUI_setItem(optionsMenu, 4, "Sprites on", 0, 1);
	GUI_setItem(optionsMenu, 5, YScroll_mode_0[0], 0, 1);
	GUI_setItem(optionsMenu, 6, "No vblank", 0, 1);
	GUI_setItem(optionsMenu, 7, "Speed hack", 0, 1);
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

touchPosition superTouchReadXY()
{
	touchPosition touchXY1;
	touchPosition touchXY2;
	touchPosition touchXY3;
	touchPosition touchXY;
	
	touchXY1 = touchReadXY();
//	swiWaitForVBlank();
	touchXY2 = touchReadXY();
//	swiWaitForVBlank();
	touchXY3 = touchReadXY();
	
	touchXY.px = (touchXY1.px + touchXY2.px + touchXY3.px) / 3;
	touchXY.py = (touchXY1.py + touchXY2.py + touchXY3.py) / 3;
	return touchXY;
}

int	loadROM(t_list *list, int res)
{
	int	size;
	char *romname;
	char *ptr;
	
	consoleClear();				
	if (ROM)
		free(ROM);
	CFG.LargeROM = 0;
	romname = malloc(200);
	strcpy(romname, "/SNES/");
	strcat(romname, list->items[res].str);
	CFG.ROMFile = romname;
//	iprintf("Name : %s", romname); 
	ROM = FS_loadROM(romname, &size);
	if (!ROM)
	{
		ROM = FS_loadROMForPaging(romname, &size);
		CFG.LargeROM = 1;
	}
	if (!ROM)
	{	
/*		GUI_displayList(list);
		if (CFG.LargeROM)	
			iprintf("\x1b[23;0HError 2");
		else
			iprintf("\x1b[23;0HError 1S");*/
		return -1;
	}
/*	GUI.log = 1;
	consoleClear();*/
	changeROM(ROM, size);
	
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

void	handle_saveState()
{
	t_list	*StatesMenu;
	int		i;
	touchPosition touchXY;
	int res;	
	char *stateName;
	char *stateFile;
	
	stateFile = strdup(CFG.ROMFile);
	strcpy(strrchr(stateFile, '.'), ".sml");
	
	StatesMenu = GUI_createList(8);
	for (i = 0; i < 8; i++)
	{
		char *state_name;
	  	state_name = get_snapshot_name(stateFile, i);
	  	if (state_name == NULL)
	  		GUI_setItem(StatesMenu, i, "--Empty--", 0, 1);
	  	else
	  		GUI_setItem(StatesMenu, i, state_name, 0, 0);
	  	free(state_name);
	}	

	GUI_displayMenuTitle("-= Save State =-", StatesMenu);

    waitReleaseTouch();			
	waitPressTouch();
	touchXY=superTouchReadXY();	
	
	res = GUI_checkMenu(StatesMenu, touchXY.px, touchXY.py);
	
	if (res >= 0)
	{
		stateName = strdup("Save #0");
		stateName[strlen(stateName)-1] = '0'+res;
		CPU_pack();	
		write_snapshot(stateFile, res, stateName);
		free(stateName);
	}

	free(stateFile);
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
	char *stateName;
	char *stateFile;
	
	stateFile = strdup(CFG.ROMFile);
	strcpy(strrchr(stateFile, '.'), ".sml");
	
	StatesMenu = GUI_createList(8);
	for (i = 0; i < 8; i++)
	{
		char *state_name;
	  	state_name = get_snapshot_name(stateFile, i);
	  	if (state_name == NULL)
	  		GUI_setItem(StatesMenu, i, "--Empty--", 0, 1);
	  	else
	  		GUI_setItem(StatesMenu, i, state_name, 0, 0);
	  	free(state_name);
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

	free(stateFile);
	free(StatesMenu);
	waitReleaseTouch();	
	GUI_displayMenu(mainMenu);	
	
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
	
	res = GUI_checkMenu(mainMenu, touchXY.px, touchXY.py);
	if (res == 0)
	{
		/* SELECT ROM */
		waitReleaseTouch();
		list->curs = 0;
		GUI_displayList(list);
		while (1)
		{
			waitHeldTouch();
			touchXY=superTouchReadXY();
			
			res = GUI_checkList(list, touchXY.px, touchXY.py);
			
			if (res >= 0)
			{
				if (!loadROM(list, res))
				{
					GUI_displayMenu(mainMenu);
					return;
				}
				GUI_displayList(list);
			}
		}
	}
	if (res == 1)
	{
		/* OPTIONS */
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
			CFG.BG_Layer ^= 0x01;
			if (CFG.BG_Layer & 0x01)
				strcpy(optionsMenu->items[1].str, "BG1 on");
			else			
				strcpy(optionsMenu->items[1].str, "BG1 off");
			break;				
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
			break;			
		case 6:		
			CFG.WaitVBlank ^= 1;
			if (CFG.WaitVBlank)
				strcpy(optionsMenu->items[6].str, "Wait vblank");
			else
				strcpy(optionsMenu->items[6].str, "No vblank");
			break;				
		case 7:		
			CFG.CPU_speedhack ^= 1;
			if (CFG.CPU_speedhack)
				strcpy(optionsMenu->items[7].str, "Speed hack");
			else
				strcpy(optionsMenu->items[7].str, "No Speed hack");
			break;				
		default:
			waitReleaseTouch();			
			GUI_displayMenu(mainMenu);			
			return;
		}
		
		GUI_displayMenuBack(optionsMenu);		
		}

	}

	if (res == 2)
	{
		handle_loadState();
		return;
	}

	if (res == 3)
	{
		handle_saveState();
		return;
	}

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
		GUI.log = 1;
		waitReleaseTouch();		
		consoleClear();
		LOG("Reset PPU...\n");
		PPU_reset();		
	}

		
//	iprintf("\x1b[22;0Hres = %d", res);

}
	
//---------------------------------------------------------------------------------
int main(void) 
{
//---------------------------------------------------------------------------------
	touchPosition touchXY;

	resetMemory2_ARM9();
	
	irqInit();
#ifndef TIMER_Y	
    TIMER3_CR &= ~TIMER_ENABLE; // not strictly necessary if the timer hasn't been enabled before
    TIMER3_DATA = 0;
    TIMER3_CR = TIMER_ENABLE | TIMER_DIV_1;
	irqSet(IRQ_VBLANK, Vblank);
    irqEnable(IRQ_VBLANK);
#else
	irqSet(IRQ_VBLANK, Vblank);
    irqEnable(IRQ_VBLANK);
#endif
    
//	screen_mode = DISPLAY_SCREEN_OFF;
	screen_mode = MODE_0_2D | DISPLAY_SPR_2D;
	videoSetMode(screen_mode);	//not using the main screen
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);	//sub bg 0 will be used to print text
	
	/* 256Ko for Tiles (SNES: 64Ko) */
	/* 256Ko for Sprites (SNES : 64Ko) */	
	vramSetBankA(VRAM_A_MAIN_SPRITE | VRAM_OFFSET(0)); // 0x6400000
	vramSetBankB(VRAM_B_MAIN_SPRITE | VRAM_OFFSET(1)); // 0x6420000
	vramSetBankC(VRAM_C_MAIN_BG_0x06000000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06020000);
	
	/* 32 Ko (+16ko) For Sub screen */ 
	vramSetBankH(VRAM_H_SUB_BG); 
	vramSetBankI(VRAM_I_SUB_BG);

	/* 32 first kilobytes for MAP */
	/* remaning memory for tiles */
/*
	BG0_CR = BG_16_COLOR | BG_PRIORITY_1 | BG_TILE_BASE(2) | BG_MAP_BASE(0) | BG_64x64;
	BG1_CR = BG_16_COLOR | BG_PRIORITY_2 | BG_TILE_BASE(4) | BG_MAP_BASE(4) | BG_64x64;
	BG2_CR = BG_16_COLOR | BG_PRIORITY_0 | BG_TILE_BASE(6) | BG_MAP_BASE(8) | BG_32x32;
	BG3_CR = BG_16_COLOR | BG_PRIORITY_3 | BG_TILE_BASE(8) | BG_MAP_BASE(12) | BG_32x32;
*/	
//	BG0_Y0 = 16;

	SUB_BG0_CR = BG_MAP_BASE(4);
	
	BG_PALETTE_SUB[255] = RGB15(31,31,31);	//by default font will be rendered with color 255

	//consoleInit() is a lot more flexible but this gets you up and running quick
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(4), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);

	iprintf(SNEMULDS_TITLE);

	swiWaitForVBlank();
	swiWaitForVBlank();

	initSNESEmpty();

	initGUI();
	
	FS_init();	
	FAT_chdir("/SNES");
    list = FS_getDirectoryList("SMC|SFC|SWC|FIG");
    FAT_chdir("/");    
    if (!list)	 
	{
		iprintf("No files found.\n"
		        "Please put ROMs in SNES directory of your CF/SD card\n");
		return 0;
	}
start:	
	GUI_displayList(list);
	 
	while (1) 
	{
		swiWaitForVBlank();
		scanKeys();
		keys = keysHeld();
		touchXY=touchReadXY();
		if (keys & KEY_TOUCH) 
		{
			int res = GUI_checkList(list, touchXY.px, touchXY.py);
			if (res >= 0)
			{
				if (loadROM(list, res) < 0)
					goto start;
				else
					break;
			}
		}
	}

   iprintf("Start!!!\n");
   GUI_displayMenu(mainMenu);

	while (1) 
	{
		go();
		handleMenu();	
	}
		
	return 0;
}
