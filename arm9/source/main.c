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
#include <stdio.h>
#include "posixHandleTGDS.h"
#include <string.h>
#include "fs.h"
#include "snes.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "conf.h"
#include "spifwTGDS.h"
#include "main.h"
#include "timerTGDS.h"
#include "guiTGDS.h"
#include "console_str.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "dmaTGDS.h"
#include "keypadTGDS.h"
#include "snemulds_memmap.h"
#include "biosTGDS.h"
#include "crc32.h"
#include "engine.h"
#include "powerTGDS.h"
#include "core.h"
#include "nds_cp15_misc.h"
#include "soundTGDS.h"
#include "special_mpu_settings.h"
#include "snemul_cfg.h"
#include "TGDSMemoryAllocator.h"
#include "TGDS_threads.h"

//TGDS Soundstreaming API
int internalCodecType = SRC_NONE; //Returns current sound stream format: WAV, ADPCM or NONE
struct fd * _FileHandleVideo = NULL; 
struct fd * _FileHandleAudio = NULL;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool stopSoundStreamUser(){
	if(SoundStreamStopSoundStreamARM9LibUtilsCallback != NULL){
		return SoundStreamStopSoundStreamARM9LibUtilsCallback(_FileHandleVideo, _FileHandleAudio, &internalCodecType);
	}
	return false;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void closeSoundUser() {
	//Stubbed. Gets called when closing an audiostream of a custom audio decoder
}

int _offsetY_tab[4] = { 16, 0, 32, 24 };

uint32 screen_mode;
int APU_MAX = 262;

bool handleROMSelect=false;
bool handleSPCSelect=false;

bool uninitializedEmu = false;
u8 savedUserSettings[1024*4];
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
	//Initial Dirs
	char romPath[MAX_TGDSFILENAME_LENGTH] = {0};
	strcpy(romPath, get_config_string("Global", "ROMPath", ""));
	strcpy(startFilePath, romPath); 
	
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
	CFG.WaitVBlank = get_config_int(section, "Vblank", 0); // CFG.WaitVBlank == 0 = vblank disabled / CFG.WaitVBlank == 1 = vblank fast / CFG.WaitVBlank == 2 = vblank full
	
	if((CFG.WaitVBlank < 0 ) || (CFG.WaitVBlank > 2)){
		CFG.WaitVBlank = 0; //vblank disabled
	}
	
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

	applyOptions();
}

bool resetSnemulDSConfig(){
	struct LZSSContext LZSSCtx = LZS_DecodeFromBuffer((u8 *)&snemul_cfg[0], (unsigned int)snemul_cfg_size);
	coherent_user_range_by_size((uint32)LZSSCtx.bufferSource, (int)LZSSCtx.bufferSize);
	int status = FS_saveFileFatFS("0:/snemul.cfg", (char*)LZSSCtx.bufferSource, LZSSCtx.bufferSize, true);	//force_file_creation == false here (we could destroy or corrupt saves..)
	TGDSARM9Free(LZSSCtx.bufferSource);
	if(status == 0){
		return true;
	}
	return false;
}

void parseCFGFile(){
	// Load SNEMUL.CFG
	GUI_printf("Load conf1");
	set_config_file(getfatfsPath("snemul.cfg"));
	GUI_printf("Load conf2");
	readOptionsFromConfig("Global");
	GUI_printf("Load conf3");
	GUI_getConfig();	
	GUI_printf("Load conf4");
}


#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool loadROM(char *name, int confirm){
	//wait until release A button
	scanKeys();
	u32 keys = keysPressed();
	while (keys&KEY_A){
		scanKeys();
		keys = keysPressed();
	}
	int size;
	char romname[100];
	int ROMheader;
	char *ROM;
	int crc;
	ROM_MAX_SIZE = 0;
	CFG.LargeROM = 0;
	strcpy(romname, name);
	strcpy(CFG.ROMFile, romname);
	clrscr();
	printf("Loading %s... ", romname);
	
	//Handle special cases for TWL extended mem games like Megaman X3 Zero Project
	coherent_user_range_by_size((uint32)TGDSIPCStartAddress, (int)sizeof(savedUserSettings));	
	memcpy((void*)&savedUserSettings[0], (const void*)TGDSIPCStartAddress, sizeof(savedUserSettings));	//memcpy( void* dest, const void* src, std::size_t count );
	
	size = FS_getFileSizeFatFS(romname);
	
	//Set up ROM paging initial state
	mem_init_paging((char*)CFG.ROMFile, size);

	ROM = (char *)SNES_ROM_ADDRESS;

	ROMheader = size & 8191;
	if (ROMheader != 0&& ROMheader != 512){
		ROMheader = 512;
	}

	char titleRead[32];
	strcpy(titleRead, &SNES.ROM_info.title[0]);
	SNEMULDS_IPC->APUSlowdown = (int)0; //No game titles slow down the APU, unless explicitely told.
	if(
		(__dsimode == true)
		&&
		(
		(strncmpi((char*)&titleRead[0], "MEGAMAN X", 9) == 0)
		||
		(strncmpi((char*)&titleRead[0], "DONKEY KONG COUNTRY 3", 21) == 0)
		||
		(strncmpi((char*)&titleRead[0], "EARTH BOUND", 11) == 0) 
		)
	){
		//Enable 16M EWRAM (TWL)
		u32 SFGEXT9 = *(u32*)0x04004008;
		//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
		SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x2 << 14);
		*(u32*)0x04004008 = SFGEXT9;
		ROM_MAX_SIZE = ROM_MAX_SIZE_TWLMODE;
		
		if (strncmpi((char*)&titleRead[0], "MEGAMAN X", 9) == 0){		//ROM masked as Read-Only, fixes Megaman X1,X2,X3 AP protection, thus making the game playable 100% (1/2)	
			ROM = (char *)ROM + (4*1024*1024); 
			setCpuClock(true);
		}
		else if (strncmpi((char*)&titleRead[0], "EARTH BOUND", 11) == 0){		//Enable Cached Samples: Earthbound	+ ROM masked as Read-Only
			ROM = (char *)ROM + (4*1024*1024); 
			setCpuClock(true);
		}
		else if (strncmpi((char*)&titleRead[0], "DONKEY KONG COUNTRY 3", 21) == 0){ //Fix DKC3 on TWL hardware
			ROM = ((char *)0x20F9F00) + (4*1024*1024); 
			setCpuClock(true);
		}
		printf("Extended TWL Mem.");
	}
	//NTR/TWL hardware fix: Solves BOF I & II freezing issues after battles 
	else if(
		(strncmpi((char*)&titleRead[0], "BREATH OF FIRE", 14) == 0)
		
	){
		if(__dsimode == true){
			//Enable 16M EWRAM (TWL)
			u32 SFGEXT9 = *(u32*)0x04004008;
			//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
			SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x2 << 14);
			*(u32*)0x04004008 = SFGEXT9;
			ROM_MAX_SIZE = ROM_MAX_SIZE_TWLMODE;
			ROM = (char *)ROM + (1024*256);
			setCpuClock(true);
			printf("Extended TWL Mem. (BOF fix)");
		}
		else{
			ROM_MAX_SIZE = ROM_MAX_SIZE_NTRMODE; //BOF games will always run in paging mode (NTR/TWL). Later, rom size is updated anyway.
			setCpuClock(false);
			printf("Normal NTR Mem. (BOF fix)");	
		}
		SNEMULDS_IPC->APUSlowdown = (int)1;
	}
	else{
		if(__dsimode == true){
			//Enable 4M EWRAM (TWL)
			u32 SFGEXT9 = *(u32*)0x04004008;
			//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
			SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x0 << 14);
			*(u32*)0x04004008 = SFGEXT9;	
		}
		ROM_MAX_SIZE = ROM_MAX_SIZE_NTRMODE;
		setCpuClock(false);
		printf("Normal NTR Mem.");
	}
	
	if(strncmpi((char*)&titleRead[0], "MEGAMAN 7", 9) == 0){
		CPU_speedhack = 2; //Full Speedhacks: //3 -> Cycles + Interrupts
	}
	else{
		CPU_speedhack = 0; //Speedhacks disabled.
	}
	
	//APU cached samples feature--
	//NTR mode:
	//Since we�ve ran out of memory for NTR mode, if SNES rom is higher than 2.8~ MB and game is LoROM, the BRR hashing feature will be disabled. 
	//Otherwise the feature will be enabled for either 4MB+ paging mode (HiROM always. LoROM is unimplemented for now), or SNES rom is 2.8~ MB or less. Both scenarios have 270K free of EWRAM.

	//TWL mode:
	//Plenty of free memory; Use the BRR hash buffer @ EWRAM offset : (SNES_ROM_ADDRESS_TWL (0x20F9F00) + ROM_MAX_SIZE_TWLMODE)
	apuCacheSamplesTWLMode = false; //false = normal sample rate, true = slower sample rate
	if(
		(
			(ROM_MAX_SIZE == ROM_MAX_SIZE_TWLMODE)	//TWL mode
			||
			(size != (3*1024*1024) )	//NTR mode 3M games can't use APU cache, no more ram left, due to rom direct mode.
		)
		||
		//NTR/TWL Mode: Breath Of Fire runs in paging mode now to get the correct audio speed
		(strncmpi((char*)&titleRead[0], "BREATH OF FIRE", 14) == 0)	
	){
		apuCacheSamples = 1;
		if (ROM_MAX_SIZE == ROM_MAX_SIZE_TWLMODE){	//TWL mode
			savedROMForAPUCache = (u32*)((int)ROM + ROM_MAX_SIZE);
		}
		else{
			savedROMForAPUCache = (u32*)APU_BRR_HASH_BUFFER_NTR;
		}
	}
	else{
		apuCacheSamples = 0;
	}
	
	//Only MegamanX2 & MegamanX3 has cached samples, or, exceptions below.
	if(
		(
			(!(strncmpi((char*)&titleRead[0], "MEGAMAN X2", 10) == 0))
			&&
			(!(strncmpi((char*)&titleRead[0], "MEGAMAN X3", 10) == 0))
		)
		&&
		//Prevent from picking up Breath Of Fire games. As they have cached samples enabled in NTR/TWL mode.
		(
			!(strncmpi((char*)&titleRead[0], "BREATH OF FIRE", 14) == 0)	
		)
	){
		apuCacheSamples = 0;
	}
	
	if(apuCacheSamples == 1){
		if(__dsimode == true){
			GUI_printf("APU Cached Samples: Enable [TWL mode]");
		}
		else{
			GUI_printf("APU Cached Samples: Enable [NTR mode]");
		}
	}
	else{
		GUI_printf("APU Cached Sample: Disable ");
	}
	
	
	if (
		(ROM_MAX_SIZE == ROM_MAX_SIZE_TWLMODE)	//TWL mode
		&&
		(strncmpi((char*)&titleRead[0], "EARTH BOUND", 11) == 0)	//Earthbound slower samplerate
	){		
		apuCacheSamplesTWLMode = true;
		GUI_printf("[Adjusted Samplerate]");
	}
	else{
		GUI_printf("[Normal Samplerate]");
	}
	int isHiROM = SNES.HiROM;
	strncpy((char*)&SNEMULDS_IPC->snesHeaderName[0], (char*)&SNES.ROM_info.title[0], sizeof(SNEMULDS_IPC->snesHeaderName));
	coherent_user_range_by_size((uint32)&SNEMULDS_IPC->snesHeaderName[0], (int)sizeof(SNEMULDS_IPC->snesHeaderName));
	
	initSNESEmpty(&uninitializedEmu, apuCacheSamples, apuCacheSamplesTWLMode, savedROMForAPUCache);
	clrscr();
	GUI_printf(" - - ");
	GUI_printf(" - - ");
	GUI_printf("File:%s - Size:%d", CFG.ROMFile, size);
	if (
		(
			(size-ROMheader > ROM_MAX_SIZE)
			||
			//NTR/TWL Mode: Breath Of Fire runs in paging mode now to get the correct audio speed
			(strncmpi((char*)&titleRead[0], "BREATH OF FIRE", 14) == 0)
		)
		&&
		(
			(isHiROM) //All LoROM games are ran from direct mode
			//||
			//(strncmpi((char*)&titleRead[0], "SUPER METROID", 13) == 0)
		)
	){
		if(isHiROM){
			FS_loadROMForPaging(ROM-ROMheader, CFG.ROMFile, PAGE_SIZE+ROMheader);
			CFG.LargeROM = true;
			crc = crc32(0, ROM, PAGE_SIZE);
			GUI_printf("(HiROM) Large ROM detected. CRC(1Mb) = %08x ", crc);
		}
		else{
			FS_loadROMForPaging(ROM-ROMheader, CFG.ROMFile, ROM_MAX_SIZE_NTRMODE_LOROM_PAGEMODE+ROMheader);
			CFG.LargeROM = true;
			crc = crc32(0, ROM, ROM_MAX_SIZE_NTRMODE_LOROM_PAGEMODE);
			GUI_printf("(LoROM) Large ROM detected. CRC(1Mb) = %08x ", crc);
		}
	}
	else{
		FS_loadROM(ROM-ROMheader, CFG.ROMFile);
		CFG.LargeROM = false;
		crc = crc32(0, ROM, size-ROMheader);
		GUI_printf("NOT Large ROM detected. CRC = %08x ", crc);
	}
	coherent_user_range_by_size((uint32)&savedUserSettings[0], (int)sizeof(savedUserSettings));	
	memcpy((void*)TGDSIPCStartAddress, (void*)&savedUserSettings[0], sizeof(savedUserSettings));	//restore them
	return changeROM(ROM-ROMheader, size);
}

int selectSong(char *name)
{
	strcpy(CFG.Playlist, name);
	CFG.Jukebox = 1;
	CFG.Sound_output = 0;
	APU_stop();
	
	u8 * spcFile = TGDSARM9Malloc(0x10200);
	if(spcFile == NULL){
		return -1;
	}
	if(FS_loadFileFatFS(CFG.Playlist, (char*)spcFile, 0x10200) < 0){
		//GUI_printf("selectSong(): Load error: %s", CFG.Playlist);
		TGDSARM9Free(spcFile);
		return -1;
	}
	//GUI_printf("WAITING");
	APU_playSpc(spcFile);	//blocking, wait APU init
	TGDSARM9Free(spcFile);
	return 0;
}

//Skip newlib-nds's dlmalloc abort handler and let dlmalloc memory manager handle gracefully invalid memory blocks, later to be re-assigned when fragmented memory gets re-arranged as valid memory blocks.
void ds_malloc_abortSkip(void){
	
}

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
__attribute__((section(".itcm")))
int main(int argc, char ** argv){
	
	/*			TGDS 1.6 Standard ARM9 Init code start	*/
	bool isTGDSCustomConsole = false;	//reloading cause issues. Thus this ensures Console to be inited even when reloading
	GUI_init(isTGDSCustomConsole);

	bool isCustomTGDSMalloc = true; //default newlib-nds's malloc throws segfaults. Use Xmalloc and overwrite memory, at the expense of allocation issues. This because SnemulDS has ran out of memory.
	setTGDSMemoryAllocator(getWoopsiSDKToolchainGenericDSMultibootMemoryAllocatorSetup(isCustomTGDSMalloc));
	
	isTGDSCustomConsole = true;
	GUI_init(isTGDSCustomConsole);
	GUI_clear();
	sint32 fwlanguage = (sint32)getLanguage(); //get language once User Settings have been loaded
	GUI_setLanguage(fwlanguage);

	bool minimumFSInitialization = true;
	int ret=FS_init(minimumFSInitialization);
	if (ret != 0)
	{
		GUI_printf(_STR(IDS_FS_FAILED));
	}
	GUI_printf(_STR(IDS_FS_SUCCESS));
	
	asm("mcr	p15, 0, r0, c7, c10, 4");
	flush_icache_all();
	flush_dcache_all();
	/*			TGDS 1.6 Standard ARM9 Init code end	*/
	
	//doesn't work on TGDS-MB v3 because VRAM D ARM7 is already used by SnemulDS
	/////////////////////////////////////////////////////////Reload TGDS Proj///////////////////////////////////////////////////////////
	/*
	char tmpName[256];
	char ext[256];
	if(__dsimode == true){
		char TGDSProj[256];
		char curChosenBrowseFile[256];
		strcpy(TGDSProj,"0:/");
		strcat(TGDSProj, "ToolchainGenericDS-multiboot");
		if(__dsimode == true){
			strcat(TGDSProj, ".srl");
		}
		else{
			strcat(TGDSProj, ".nds");
		}
		//Force ARM7 reload once 
		if( 
			(argc < 2) 
			&& 
			(strncmpi(argv[1], TGDSProj, strlen(TGDSProj)) != 0) 	
		){
			REG_IME = 0;
			MPUSet();
			REG_IME = 1;
			char startPath[MAX_TGDSFILENAME_LENGTH+1];
			strcpy(startPath,"/");
			strcpy(curChosenBrowseFile, TGDSProj);
			
			char thisTGDSProject[MAX_TGDSFILENAME_LENGTH+1];
			strcpy(thisTGDSProject, "0:/");
			strcat(thisTGDSProject, TGDSPROJECTNAME);
			if(__dsimode == true){
				strcat(thisTGDSProject, ".srl");
			}
			else{
				strcat(thisTGDSProject, ".nds");
			}
			
			//Boot .NDS file! (homebrew only)
			strcpy(tmpName, curChosenBrowseFile);
			separateExtension(tmpName, ext);
			strlwr(ext);
			
			//pass incoming launcher's ARGV0
			char arg0[256];
			int newArgc = 2;
			if (argc > 2) {
				//Arg0:	Chainload caller: TGDS-MB
				//Arg1:	This NDS Binary reloaded through ChainLoad
				//Arg2: This NDS Binary reloaded through ChainLoad's Argument0
				strcpy(arg0, (const char *)argv[2]);
				newArgc++;
			}
			
			char thisArgv[3][MAX_TGDSFILENAME_LENGTH];
			memset(thisArgv, 0, sizeof(thisArgv));
			strcpy(&thisArgv[0][0], curChosenBrowseFile);	//Arg0:	Chainload caller: TGDS-MB
			strcpy(&thisArgv[1][0], thisTGDSProject);	//Arg1:	NDS Binary reloaded through ChainLoad
			strcpy(&thisArgv[2][0], (char*)arg0);	//Arg2: NDS Binary reloaded through ChainLoad's ARG0
			addARGV(newArgc, (char*)&thisArgv);				
			if(TGDSMultibootRunNDSPayload(curChosenBrowseFile) == false){ //should never reach here, nor even return true. Should fail it returns false
				
			}
		}
	}
	*/
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	REG_IME = 0;
	
	//Discard old & enter new MPU context
	enterMPUConfig();
	
	setSnemulDSSpecial0xFFFF0000MPUSettings();
	
	//Leave & absorb new MPU context
	leaveMPUConfig();
	
	REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	//Set up PPU IRQ: HBLANK/VBLANK/VCOUNT
	REG_DISPSTAT = (DISP_HBLANK_IRQ | DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
	REG_IE |= (IRQ_HBLANK| IRQ_VBLANK | IRQ_VCOUNT);		
	
	//Set up PPU IRQ Vertical Line
	setVCountIRQLine(TGDS_VCOUNT_LINE_INTERRUPT);
	irqDisable(IRQ_VCOUNT|IRQ_TIMER1);	//SnemulDS abuses HBLANK IRQs, VCOUNT IRQs seem to cause a race condition
	powerOFF3DEngine(); //Power off ARM9 3D Engine to save power
	REG_IME = 1;
	
	//Let dlmalloc handle memory management
	extern void ds_malloc_abort(void);
	u32 * fnPtr = (u32 *)&ds_malloc_abort;
	mem_cpy((u8*)fnPtr, (u8*)&ds_malloc_abortSkip, 16);

	swiDelay(1000);
	setupDisabledExceptionHandler(); //on 0x00000000 (NULL) reference, skip the abort handler 
	
	if(__dsimode == true){
		TWLSetTouchscreenTWLMode();
	}
	
	SNEMULDS_IPC->APU_ADDR_CNT = SNEMULDS_IPC->APU_ADDR_ANS = SNEMULDS_IPC->APU_ADDR_CMD = 0;
	screen_mode = 0;
	update_spc_ports();
	uninitializedEmu = false;
	
	// Clear "HDMA"
	int i = 0;
	for (i = 0; i < 192; i++){
		GFX.lineInfo[i].mode = -1;
	}

	strcpy(startFilePath, "");
	//Parse snemul.cfg
	parseCFGFile();
	
	//Regenerate snemul.cfg if invalid
	if(strlen(startFilePath) < 3){
		bool ret = resetSnemulDSConfig();
		GUI_printf("--");
		if(ret == true){
			GUI_printf("Broken CFG File! Rebuild OK");
		}
		else{
			GUI_printf("Broken CFG File! Rebuild Error");
		}
		parseCFGFile();
	}
	memset(&guiSelItem, 0, sizeof(guiSelItem));
	guiSelItem.StructFDFromFS_getDirectoryListMethod = FT_FILE;
	
	//ARGV Support: Only supported through TGDS chainloading.
	switchToTGDSConsoleColors();
	bool isSnesFile = false;
	if (argc > 2) {
		//Arg0:	Chainload caller: TGDS-MB
		//Arg1:	This NDS Binary reloaded through ChainLoad
		//Arg2: This NDS Binary reloaded through ChainLoad's Argument0		
		//is sfc/smc? then valid
		strcpy(&CFG.ROMFile[0], (const char *)argv[2]);
		guiSelItem.filenameFromFS_getDirectoryListMethod = (char*)&CFG.ROMFile[0];
		isSnesFile = true;
	}
	//Handle file load
	do{
		if (isSnesFile == false) {
			guiSelItem.filenameFromFS_getDirectoryListMethod = GUI_getROMList(startFilePath);
		}
		isSnesFile = loadROM(guiSelItem.filenameFromFS_getDirectoryListMethod, 0);
	}
	while(isSnesFile == false);
	///////////////////////////////////////////
	if (!(argc > 2)) { 
		GUI_deleteROMSelector(); 	//Should also free ROMFile
	}
	
	switchToSnemulDSConsoleColors();
	
	//GBA Macro Mode
	{
		bool isGBAMacroModeKeyComb = false;
		GFX.DSFrame = 0;
		clrscr();
		printf("---");
		printf("---");
		printf("5 seconds timeout...");
		printf("Press X + Up to enable GBA Macro mode [Vblank full]");
		printf("Press X + Down to enable GBA Macro mode [Vblank fast]");
		printf("[GBA Macro Mode: Disabled] >%d", TGDSPrintfColor_Yellow);
		while(GFX.DSFrame < 360){
			scanKeys();
			u32 thisKeys = keysHeld();
			if( (thisKeys & KEY_UP) && (thisKeys & KEY_X) ){
				clrscr();
				printf("---");
				printf("---");
				printf("5 seconds timeout...");
				printf("Press X + Up to enable GBA Macro mode [Vblank full]");
				printf("Press X + Down to enable GBA Macro mode [Vblank fast]");
				printf("[GBA Macro Mode: Enabled] [Vblank full] >%d", TGDSPrintfColor_Green);

				isGBAMacroModeKeyComb = true;

				//Full vblank
				CFG.WaitVBlank = 2;
				
				//Saving enabled
				CFG.EnableSRAM = 1;

				break;	
			}

			else if( (thisKeys & KEY_DOWN) && (thisKeys & KEY_X) ){
				clrscr();
				printf("---");
				printf("---");
				printf("5 seconds timeout...");
				printf("Press X + Up to enable GBA Macro mode [Vblank full]");
				printf("Press X + Down to enable GBA Macro mode [Vblank fast]");
				printf("[GBA Macro Mode: Enabled] [Vblank fast] >%d", TGDSPrintfColor_Green);

				isGBAMacroModeKeyComb = true;
				
				//Fast vblank
				CFG.WaitVBlank = 1;
				
				//Saving enabled
				CFG.EnableSRAM = 1;

				break;	
			}

			swiDelay(1);
		}

		while(1==1){
			scanKeys();
			u32 thisKeys = keysHeld();
			if( 
				!(thisKeys & KEY_UP) 
				&&
				!(thisKeys & KEY_DOWN) 
				&&
				!(thisKeys & KEY_LEFT) 
				&&
				!(thisKeys & KEY_RIGHT) 
				&&
				!(thisKeys & KEY_A) 
				&&
				!(thisKeys & KEY_B) 
				&&
				!(thisKeys & KEY_X) 
				&&
				!(thisKeys & KEY_Y) 
			){
				break;
			}
		}

		if(isGBAMacroModeKeyComb == true){
			//Enable GBA Macro Mode here
			GUI.GBAMacroMode = true;
			TGDSLCDSwap();
			setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
		}
	}

	GUI_createMainMenu();	//Start GUI
	
	while (1){
		if(REG_DISPSTAT & DISP_VBLANK_IRQ){
			//Sync Events
			if(handleROMSelect==true){
				handleROMSelect=false;
				
				if (CFG.Sound_output || CFG.Jukebox){
					APU_pause();
				}

				//snes init
				SNEMULDS_IPC->APU_ADDR_CNT = SNEMULDS_IPC->APU_ADDR_ANS = SNEMULDS_IPC->APU_ADDR_CMD = 0;
				update_spc_ports();
				
				// Clear "HDMA"
				for (i = 0; i < 192; i++){
					GFX.lineInfo[i].mode = -1;
				}
				
				//Handle file load
				bool isSnesFile = false;
				do{
					memset(&guiSelItem, 0, sizeof(guiSelItem));
					guiSelItem.StructFDFromFS_getDirectoryListMethod = FT_FILE;
					guiSelItem.filenameFromFS_getDirectoryListMethod = GUI_getROMList(startFilePath);
					isSnesFile = loadROM(guiSelItem.filenameFromFS_getDirectoryListMethod, 0);
				}
				while(isSnesFile == false);
				///////////////////////////////////////////
				GUI_createMainMenu();	//	Start GUI
			}
			
			if(handleSPCSelect==true){
				handleSPCSelect=false;
				/*
				if (CFG.Sound_output || CFG.Jukebox)
					APU_pause();
				
				memset(&guiSelItem, 0, sizeof(guiSelItem));
				char * fileName = GUI_getSPCList(startSPCFilePath);
				guiSelItem.StructFDFromFS_getDirectoryListMethod = FT_FILE;
				guiSelItem.filenameFromFS_getDirectoryListMethod = (char*)fileName;
				selectSong(fileName);
				
				GUI_createMainMenu();	//Start GUI
				*/
			}
			GUI_update();
		}
		
		if (!SNES.Stopped){
			go();
		}
		HaltUntilIRQ(); //Save power until next irq
	}

	return 0;
}

//////////////////////////////////////////////////////// Threading User code start : TGDS Project specific ////////////////////////////////////////////////////////
//User callback when Task Overflows. Intended for debugging purposes only, as normal user code tasks won't overflow if a task is implemented properly.
//	u32 * args = This Task context
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void onThreadOverflowUserCode(u32 * args){
	struct task_def * thisTask = (struct task_def *)args;
	struct task_Context * parentTaskCtx = thisTask->parentTaskCtx;	//get parent Task Context node 
	char threadStatus[64];
	switch(thisTask->taskStatus){
		case(INVAL_THREAD):{
			strcpy(threadStatus, "INVAL_THREAD");
		}break;
		
		case(THREAD_OVERFLOW):{
			strcpy(threadStatus, "THREAD_OVERFLOW");
		}break;
		
		case(THREAD_EXECUTE_OK_WAIT_FOR_SLEEP):{
			strcpy(threadStatus, "THREAD_EXECUTE_OK_WAIT_FOR_SLEEP");
		}break;
		
		case(THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE):{
			strcpy(threadStatus, "THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE");
		}break;
	}
	
	char debOut2[256];
	char timerUnitsMeasurement[32];
	if( thisTask->taskStatus == THREAD_OVERFLOW){
		if(thisTask->timerFormat == tUnitsMilliseconds){
			strcpy(timerUnitsMeasurement, "ms");
		}
		else if(thisTask->timerFormat == tUnitsMicroseconds){
			strcpy(timerUnitsMeasurement, "us");
		} 
		else{
			strcpy(timerUnitsMeasurement, "-");
		}
		sprintf(debOut2, "[%s]. Thread requires at least (%d) %s. ", threadStatus, thisTask->remainingThreadTime, timerUnitsMeasurement);
	}
	else{
		sprintf(debOut2, "[%s]. ", threadStatus);
	}
	
	int TGDSDebuggerStage = 10;
	u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
	handleDSInitOutputMessage((char*)debOut2);
	handleDSInitError(TGDSDebuggerStage, (u32)fwNo);
	
	while(1==1){
		HaltUntilIRQ();
	}
}
//////////////////////////////////////////////////////////////////////// Threading User code end /////////////////////////////////////////////////////////////////////////////