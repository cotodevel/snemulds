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
#include "global_settings.h"
#include "eventsTGDS.h"
#include "posixHandleTGDS.h"
#include "TGDSMemoryAllocator.h"
#include "ipcfifoTGDS.h"

__attribute__((section(".dtcm")))
bool handleROMSelect=false;

__attribute__((section(".dtcm")))
bool handleSPCSelect=false;

int _offsetY_tab[4] = { 16, 0, 32, 24 };
uint32 screen_mode;
int APU_MAX = 262;
u32 keys;

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
	char romPath[MAX_TGDSFILENAME_LENGTH+1] = {0};
	char spcPath[MAX_TGDSFILENAME_LENGTH+1] = {0};
	strcpy(romPath, get_config_string("Global", "ROMPath", ""));
	strcpy(spcPath, get_config_string("Global", "SPCPath", ""));
	
	strcpy(startFilePath, "/");	//Init var
	strcat(startFilePath, romPath); //Init var
	
	strcpy(startSPCFilePath, "/");
	strcat(startSPCFilePath, spcPath);
	
	strcpy(CFG.ROMPath, getfatfsPath(romPath));
	strcpy(CFG.SPCPath, getfatfsPath(spcPath));
	
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
	
	//CFG.MapExtMem = get_config_int(section, "MapExtMem", CFG.MapExtMem);	//SLOT-2 external memory is disabled
	CFG.EnableSRAM = get_config_int(section, "EnableSRAM", CFG.EnableSRAM);
	
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
	
	set_config_int(section, "EnableSRAM", CFG.EnableSRAM);
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

int loadROM(struct sGUISelectorItem * name)
{
	//wait until release A button
	scanKeys();
	u32 keys = keysPressed();
	while (keys&KEY_A){
		scanKeys();
		keys = keysPressed();
	}
	
	//file
	if(name->StructFDFromFS_getDirectoryListMethod == FT_FILE){
		int size;
		char romname[MAX_TGDSFILENAME_LENGTH+1] = {0};
		int ROMheader;
		char *ROM;
		int crc;
		
		// Save SRAM of previous game first
		saveSRAM();
		CFG.LargeROM = 0;
		
		//filename already has correct format
		if (
			(name->filenameFromFS_getDirectoryListMethod[0] == '0')
			&&
			(name->filenameFromFS_getDirectoryListMethod[1] == ':')
			&&
			(name->filenameFromFS_getDirectoryListMethod[2] == '/')
		){
				strcpy(romname, name->filenameFromFS_getDirectoryListMethod);
				strcpy(CFG.ROMFile, romname);
		}
		//otherwise build format
		else{
			strcpy(romname, getfatfsPath(startFilePath));
			if (startFilePath[strlen(startFilePath)-1] != '/'){
				strcat(romname, "/");
			}
			strcat(romname, name->filenameFromFS_getDirectoryListMethod);
			strcpy(CFG.ROMFile, romname);
		}
		clrscr();
		
		void *ptr = malloc(4);
		printf("ptr=%p... ", ptr);
		free(ptr);

		mem_clear_paging(); // FIXME: move me...
		ROM = (char *) SNES_ROM_ADDRESS;
		SnemulDSdmaFillHalfWord(3, 0, (uint32)ROM, (uint32)ROM_MAX_SIZE);	//Clear memory: ZIP Will use this as malloc
		
		bool zipFileLoaded = false;
		if(strstr (_FS_getFileExtension(name->filenameFromFS_getDirectoryListMethod),"ZIP")){	
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
		
		SnemulDSdmaFillHalfWord(3, 0, (uint32)ROM, (uint32)ROM_MAX_SIZE);	////Clear memory: ROM will use it
		
		size = FS_getFileSize(romname);
		ROMheader = size & 8191;
		if (ROMheader != 0&& ROMheader != 512)
			ROMheader = 512;

		if (size-ROMheader > ROM_MAX_SIZE)
		{
			FS_loadROMForPaging(ROM-ROMheader, romname, ROM_STATIC_SIZE+ROMheader);
			CFG.LargeROM = 1;
			crc = crc32(0, ROM, ROM_STATIC_SIZE);
			printf("Large ROM detected. CRC(1Mb) = %08x ", crc);
		}
		else
		{
			FS_loadROM(ROM-ROMheader, romname);
			CFG.LargeROM = 0;
			crc = crc32(0, ROM, size-ROMheader);
			printf("CRC = %08x ", crc);
		}
		
		changeROM(ROM-ROMheader, size);
		checkConfiguration(name->filenameFromFS_getDirectoryListMethod, crc);
		
		//Apply topScreen / bottomScreen setting
		if(CFG.TopScreenEmu == 0){
			SnemulDSLCDSwap();
		}
	}
	
	return 0;
}

int selectSong(char *name)
{
	char spcname[MAX_TGDSFILENAME_LENGTH+1];
	memset(spcname, 0, sizeof(spcname));
	strcat(spcname, name);
	strcpy(CFG.Playlist, spcname);
	CFG.Jukebox = 1;
	CFG.Sound_output = 0;
	APU_stop();
	
	u8 * spcFile = malloc(0x10200);
	if(spcFile == NULL){
		return -1;
	}
	if(FS_loadFile(spcname, spcFile, 0x10200) < 0){
		printf("selectSong(): Load error: %s", spcname);
		while(1==1){
			
		}
		free(spcFile);
		return -1;
	}
	APU_playSpc(spcFile);	//blocking, wait APU init
	free(spcFile);
	return 0;
}

//---------------------------------------------------------------------------------
__attribute__((section(".itcm")))
int main(int argc, char ** argv){
	/*			TGDS 1.5 Standard ARM9 Init code start	*/
	bool isTGDSCustomConsole = true;	//set default console or custom console: custom console
	GUI_init(isTGDSCustomConsole);
	clrscr();
	setTGDSMemoryAllocator(getProjectSpecificMemoryAllocatorSetup());
	sint32 fwlanguage = (sint32)getLanguage();
	GUI_setLanguage(fwlanguage);
	
	printf(_STR(IDS_INITIALIZATION));
	#ifdef ARM7_DLDI
	setDLDIARM7Address((u32 *)TGDSDLDI_ARM7_ADDRESS);	//Required by ARM7DLDI!
	#endif
	int ret=FS_init();
	if (ret == 0)
	{
		printf(_STR(IDS_FS_SUCCESS));
	}
	else if(ret == -1)
	{
		printf(_STR(IDS_FS_FAILED));
	}
	
	switch_dswnifi_mode(dswifi_idlemode);
	/*			TGDS 1.5 Standard ARM9 Init code end	*/
	memset(&startFilePath, 0, sizeof(startFilePath));
	memset(&startSPCFilePath, 0, sizeof(startSPCFilePath));
	
	DisableIrq(IRQ_VCOUNT|IRQ_TIMER1);	//SnemulDS abuses HBLANK IRQs, VCOUNT IRQs seem to cause a race condition
	disableSleepMode();	//Disable timeout-based sleep mode
	DisableSoundSampleContext();
	swiDelay(888);
	coherent_user_range_by_size((u32)IPC6, sizeof(struct sIPCSharedTGDSSpecific));
	IPC6->APU_ADDR_CNT = 0;
	IPC6->APU_ADDR_ANS = IPC6->APU_ADDR_CMD = 0;
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

	// Load SNEMUL.CFG
	printf("Load conf1");
	set_config_file(getfatfsPath("snemul.cfg"));
	
	//ext support removed for now
	/*
	{
		FILE *f=fopen("/moonshl2/extlink.dat","rb");
		if(!f){printf("Extlink cannot open.");while(1);}//__swiSleep();}
		fread(&extlink,1,sizeof(extlink),f);
		fclose(f);
		if(extlink.ID!=ExtLinkBody_ID){printf("Not valid extlink.");while(1);}//__swiSleep();}
	}
	*/
	
	printf("Load conf2");
	readOptionsFromConfig("Global");
	printf("Load conf3");
	GUI_getConfig();
	printf("Load conf4");
	
	GUI_getROMFirstTime(CFG.ROMPath);	//Output rom -> CFG.ROMFile
	memset(&guiSelItem, 0, sizeof(guiSelItem));
	guiSelItem.StructFDFromFS_getDirectoryListMethod = FT_FILE;
	guiSelItem.filenameFromFS_getDirectoryListMethod = (char*)&CFG.ROMFile[0];
	loadROM(&guiSelItem);
	GUI_deleteROMSelector(); 	//Should also free ROMFile
	GUI_createMainMenu();		//Start GUI
	
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
	
	while (1){
		
		if(REG_DISPSTAT & DISP_VBLANK_IRQ){
			//Sync Events
			if(handleROMSelect==true){
				handleROMSelect=false;
				GUI_getROMIterable(startFilePath);
				GUI_deleteROMSelector(); // Should also free lst
				GUI_createMainMenu();	//	Start GUI
			}
			
			if(handleSPCSelect==true){
				handleSPCSelect=false;
				GUI_getSPCIterable(startSPCFilePath);
				GUI_deleteROMSelector(); // Should also free ROMFile
				GUI_createMainMenu();	//Start GUI
			}
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
