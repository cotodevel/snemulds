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

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "fs.h"
#include "snes.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "interrupts.h"
#include "conf.h"
#include "dswnifi_lib.h"

//#define USE_EMUL

int _offsetY_tab[4] = { 16, 0, 32, 24 };

uint32 screen_mode;
int APU_MAX = 262;
//volatile int h_blank;


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
	
	//CFG.MapExtMem = get_config_int(section, "MapExtMem", CFG.MapExtMem);
	
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
		printf("Section : %s ", section);
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

	/*	if (ROM && FS_shouldFreeROM())
	 free(ROM);*/
	CFG.LargeROM = 0;
	strcpy(romname, CFG.ROMPath);
	if (CFG.ROMPath[strlen(CFG.ROMPath)-1] != '/')
		strcat(romname, "/");
	strcat(romname, name);
	strcpy(CFG.ROMFile, romname);
	clrscr();
	
	printf("Loading %s... ", romname);

	void *ptr = malloc(4);
	printf("ptr=%p... ", ptr);
	free(ptr);

	mem_clear_paging(); // FIXME: move me...
	ROM = (char *) SNES_ROM_ADDRESS;
	dmaFillHalfWord(3, 0, (uint32)ROM, (uint32)ROM_MAX_SIZE);	//Clear memory: ZIP Will use this as malloc
	
	bool zipFileLoaded = false;
	if(strstr (_FS_getFileExtension(name),"ZIP")){	
		zipFileLoaded = true;
	}
	
	if(zipFileLoaded == true){
		//build into tmpFile2, filename.smc out of passed filename.ext compressed
		char outFile[MAX_TGDSFILENAME_LENGTH+1] = {0};
		char inFile[MAX_TGDSFILENAME_LENGTH+1] = {0};
		char temp1[MAX_TGDSFILENAME_LENGTH+1] = {0};
		char temp2[MAX_TGDSFILENAME_LENGTH+1] = {0};
		int sizeExt=strlen(_FS_getFileExtension(CFG.ROMFile))+1;
		strncpy(temp1, CFG.ROMFile, strlen(CFG.ROMFile) - sizeExt);	//"filename" (no extension)
		sprintf(outFile,"%s%s",temp1,".smc");
		printf("decompressing:%s",CFG.ROMFile);
		printf("->%s",outFile);
		sprintf(romname,"%s",outFile);
		//Decompress File for reload later
		int stat = load_gz((char*)CFG.ROMFile, (char*)outFile);
	}
	
	dmaFillHalfWord(3, 0, (uint32)ROM, (uint32)ROM_MAX_SIZE);	////Clear memory: ROM will use it
	
	size = FS_getFileSize(romname);
	ROMheader = size & 8191;
	if (ROMheader != 0&& ROMheader != 512)
		ROMheader = 512;

#ifndef USE_GBFS	
	if (size-ROMheader > ROM_MAX_SIZE)
	{
		FS_loadROMForPaging(ROM-ROMheader, romname, ROM_STATIC_SIZE+ROMheader);
		CFG.LargeROM = 1;
		crc = crc32(0, ROM, ROM_STATIC_SIZE);
		printf("Large ROM detected. CRC(1Mb) = %08x ", crc);
	}
	else
#endif	
	{
		FS_loadROM(ROM-ROMheader, romname);
		CFG.LargeROM = 0;
		crc = crc32(0, ROM, size-ROMheader);
		printf("CRC = %08x ", crc);
	}

	//ROM += 0x400000; // Protected ROM

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
	if (FS_loadFile(spcname, APU_RAM_ADDRESS, 0x10200) < 0)
		return -1;
	APU_playSpc();
	// Wait APU init
	IRQVBlankWait();
	IRQVBlankWait();
	IRQVBlankWait();
	IRQVBlankWait();
	//	printf("\nDBG: %s", DEBUG_BUF);	
	return 0;
}




//---------------------------------------------------------------------------------
__attribute__((section(".itcm")))
int main(int argc, char ** argv){
	/*			TGDS 1.5 Standard ARM9 Init code start	*/
	bool project_specific_console = true;	//set default console or custom console: custom console
	GUI_init(project_specific_console);
	
	clrscr();
	sint32 fwlanguage = (sint32)getLanguage();
	GUI_setLanguage(fwlanguage);
	
	printf(_STR(IDS_INITIALIZATION));
	int ret=FS_init();
	if (ret == 0)
	{
		printf(_STR(IDS_FS_SUCCESS));
		//FRESULT res = FS_chdir("0:/");
	}
	else if(ret == -1)
	{
		printf(_STR(IDS_FS_FAILED));
	}
	
	switch_dswnifi_mode(dswifi_idlemode);
	/*			TGDS 1.5 Standard ARM9 Init code end	*/
	
	DisableIrq(IRQ_VCOUNT);	//SnemulDS abuses HBLANK IRQs, VCOUNT IRQs seem to cause a race condition
	
	/*
	//debug
	printf(" - ");
	printf(" - ");
	printf(" - ");
	printf(" - ");
	printf("wait");
	
	while(1==1){
		IRQVBlankWait();
	}
	*/
	
	struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;
	TGDSUSERIPC->APU_ADDR_CNT = 0;
	TGDSUSERIPC->APU_ADDR_ANS = TGDSUSERIPC->APU_ADDR_CMD = 0;
	screen_mode = 0;
	
#ifndef DSEMUL_BUILD	
	GUI.printfy = 32;
	//GUI_align_printf(GUI_TEXT_ALIGN_CENTER, SNEMULDS_TITLE);
	//GUI_align_printf(GUI_TEXT_ALIGN_CENTER, SNEMULDS_SUBTITLE);
    GUI.printfy += 32; // FIXME
	//GUI_align_printf(GUI_TEXT_ALIGN_CENTER, _STR(4));
#endif	
	
	update_spc_ports();
	initSNESEmpty();

	// Clear "HDMA"
	int i = 0;
	for (i = 0; i < 192; i++){
		GFX.lineInfo[i].mode = -1;
		t_GFX_lineInfo *l = &GFX.lineInfo[i];
		memset((u8*)l, 0, sizeof(GFX.lineInfo));
	}
#if 0
	{	char *p = malloc(10);
		printf("RAM = %p last malloc = %p", SNESC.RAM, p);
	}
	/* TOUCH SCREEN TEST */
	while (1)
	{
		int i;

		mytouchPosition mytouchXY;

		keys = keysPressed();
		//		mytouchXY=mytouchReadXY();

		printf("keys = %x %d ", keys, SNES.h_blank);
		if (keys & KEY_TOUCH)
		{
			touchXY=superTouchReadXY();
			printf("x = %d y = %d       ", touchXY.px, touchXY.py);
			//		waitReleaseTouch();	
		}

		if ((keys & KEY_START))
		break;
	}
#endif	


#ifndef	DSEMUL_BUILD	
	//for (i = 0; i < 100; i++)
	//	IRQVBlankWait();
#endif	
	printf("Load conf1");
	// Load SNEMUL.CFG
	set_config_file(getfatfsPath("snemul.cfg"));	//set_config_file("snemul.cfg");
	
	//removed
	/*
	{
		FILE *f=fopen("/moonshl2/extlink.dat","rb");
		if(!f){printf("Extlink cannot open.");while(1);}//__swiSleep();}
		fread(&extlink,1,sizeof(extlink),f);
		fclose(f);
		if(extlink.ID!=ExtLinkBody_ID){printf("Not valid extlink.");while(1);}//__swiSleep();}
	}
	*/
	
	//CFG.ROMPath = get_config_string(NULL, "ROMPath", GAMES_DIR);
	
	printf("Load conf2");
	readOptionsFromConfig("Global");
	printf("Load conf3");
	GUI_getConfig();	
	printf("Load conf4");

	//char *ROMfile = GUI_getROM(CFG.ROMPath);
	//clrscr();
	
	//single player:
	switch_dswnifi_mode(dswifi_idlemode);
	
	char bufstr[0x100];
	sprintf(bufstr,"%s",getfatfsPath("snes"));	//0:/snes
	sprintf(CFG.ROMPath,"%s",bufstr);
	
	GUI_getROM(bufstr);	//read rom from (path)touchscreen:output rom -> CFG.ROMFile
	
	loadROM(CFG.ROMFile, 0);
	GUI_deleteROMSelector(); // Should also free ROMFile
	GUI_createMainMenu();	//Start GUI
	
//trace code
#if 0
	printf("CPU_init=%p ", CPU_init);
	printf("CPU_goto=%p ", CPU_goto2);
	printf("logbuf=%p ", logbuf);

	while (1)
	{
		keys = keysPressed();
		if ((keys & KEY_START))
		break;
	}
#endif	
	
	while (1)
	{
		if(REG_DISPSTAT & DISP_VBLANK_IRQ){
			GUI_update();
		}
		
		go();
		
		#ifdef GDB_ENABLE
		setBacklight(POWMAN_BACKLIGHT_TOP_BIT|POWMAN_BACKLIGHT_BOTTOM_BIT);
		//GDB Stub Process must run here
		int retGDBVal = remoteStubMain();
		if(retGDBVal == remoteStubMainWIFINotConnected){
			if (switch_dswnifi_mode(dswifi_gdbstubmode) == true){
				clrscr();
				//Show IP and port here
				printf("[Connect to GDB]:");
				char IP[16];
				printf("Port:%d GDB IP:%s", remotePort, print_ip((uint32)Wifi_GetIP(), IP));
				remoteInit();
			}
			else{
				//GDB Client Reconnect:ERROR
			}
		}
		else if(retGDBVal == remoteStubMainWIFIConnectedGDBDisconnected){
			setWIFISetup(false);
			if (switch_dswnifi_mode(dswifi_gdbstubmode) == true){ // gdbNdsStart() called
				reconnectCount++;
				clrscr();
				//Show IP and port here
				printf("[Re-Connect to GDB]:So far:%d time(s)",reconnectCount);
				char IP[16];
				printf("Port:%d GDB IP:%s", remotePort, print_ip((uint32)Wifi_GetIP(), IP));
				remoteInit();
			}
		}
		
		//else should be connected and GDB running at desired IP/port
		#endif
	}

	return 0;
}
