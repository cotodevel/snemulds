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
#include "guiTGDS.h"
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
#include "guiTGDS.h"
#include "core.h"
#include "nds_cp15_misc.h"
#include "soundTGDS.h"
#include "spitscTGDS.h"
#include "snemul_cfg.h"

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

static u32 apuFix = 0;
uint32 screen_mode = 0;
int APU_MAX = 262;

__attribute__((section(".dtcm")))
bool handleROMSelect=false;

__attribute__((section(".dtcm")))
bool handleSPCSelect=false;

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
	//SNES button mapping
	SNES_A = get_config_hex("KEYS", "SNES_BUTTON_A", 0x00000080);
	SNES_B = get_config_hex("KEYS", "SNES_BUTTON_B", 0x00008000);
	SNES_X = get_config_hex("KEYS", "SNES_BUTTON_X", 0x00000040);
	SNES_Y = get_config_hex("KEYS", "SNES_BUTTON_Y", 0x00004000);
	SNES_L = get_config_hex("KEYS", "SNES_BUTTON_L", 0x00000020);
	SNES_R = get_config_hex("KEYS", "SNES_BUTTON_R", 0x00000010);
	SNES_SELECT = get_config_hex("KEYS", "SNES_BUTTON_SELECT", 0x00002000);
	SNES_START = get_config_hex("KEYS", "SNES_BUTTON_START", 0x00001000);
	SNES_UP = get_config_hex("KEYS", "SNES_BUTTON_UP", 0x00000800);
	SNES_DOWN = get_config_hex("KEYS", "SNES_BUTTON_DOWN", 0x00000400);
	SNES_LEFT = get_config_hex("KEYS", "SNES_BUTTON_LEFT", 0x00000200);
	SNES_RIGHT = get_config_hex("KEYS", "SNES_BUTTON_RIGHT", 0x00000100);
	
	//Initial Dirs
	char romPath[MAX_TGDSFILENAME_LENGTH+1] = {0};
	char spcPath[MAX_TGDSFILENAME_LENGTH+1] = {0};
	strcpy(romPath, get_config_string("Global", "ROMPath", ""));
	strcpy(spcPath, get_config_string("Global", "SPCPath", ""));
	strcpy(startFilePath, romPath); 
	strcpy(startSPCFilePath, spcPath);
	
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

bool uninitializedEmu = false;
static u8 savedUserSettings[1024*4];

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool loadROM(struct sGUISelectorItem * nameItem){
	//wait until release A button
	scanKeys();
	u32 keys = keysPressed();
	while (keys&KEY_A){
		scanKeys();
		keys = keysPressed();
	}
	
	//file
	if(nameItem->StructFDFromFS_getDirectoryListMethod == FT_FILE){
		int size;
		char romname[MAX_TGDSFILENAME_LENGTH+1] = {0};
		int ROMheader;
		char *ROM;
		int crc;
		
		//filename already has correct format
		if (
			(nameItem->filenameFromFS_getDirectoryListMethod[0] == '0')
			&&
			(nameItem->filenameFromFS_getDirectoryListMethod[1] == ':')
			&&
			(nameItem->filenameFromFS_getDirectoryListMethod[2] == '/')
		){
			strcpy(romname, nameItem->filenameFromFS_getDirectoryListMethod);
		}
		//otherwise build format
		else{
			strcpy(romname, getfatfsPath(startFilePath));
			if (romname[strlen(romname)-1] != '/'){
				strcat(romname, "/");
			}
			strcat(romname, nameItem->filenameFromFS_getDirectoryListMethod);
		}
		
		//There's a bug when rendering certain UI elements, some garbage may appear near the end of the filename, todo
		char ext[256];
		char tmp[256];
		strcpy(tmp,romname);
		separateExtension(tmp,ext);
		strlwr(ext);
		if(strlen(ext) > 4){
			romname[strlen(romname)-2] = '\0';
		}
		
		clrscr();
		printf("----");
		printf("----");
		printf("----");
		
		memset(CFG.ROMFile, 0, sizeof(CFG.ROMFile));
		strcpy(CFG.ROMFile, romname);
		
		//Handle special cases for TWL extended mem games like Megaman X3 Zero Project
		coherent_user_range_by_size((uint32)0x027FF000, (int)sizeof(savedUserSettings));	
		memcpy((void*)&savedUserSettings[0], (const void*)0x027FF000, sizeof(savedUserSettings));	//memcpy( void* dest, const void* src, std::size_t count );
		
		ROM = (char *)SNES_ROM_ADDRESS_NTR;
		size = FS_getFileSizeFatFS((char*)&CFG.ROMFile[0]);
		ROMheader = size & 8191;
		if (ROMheader != 0&& ROMheader != 512){
			ROMheader = 512;
		}

		FS_loadFileFatFS(CFG.ROMFile, ROM, PAGE_SIZE+ROMheader);
		load_ROM(ROM, size);
		int i = 20;
		while (i >= 0 && SNES.ROM_info.title[i] == ' '){
			SNES.ROM_info.title[i--] = '\0';
		}
		if(
			(__dsimode == true)
			&&
			(
			(strncmpi((char*)&SNES.ROM_info.title[0], "MEGAMAN X", 9) == 0)
			||
			(strncmpi((char*)&SNES.ROM_info.title[0], "DONKEY KONG COUNTRY 3", 21) == 0)
			||
			(strncmpi((char*)&SNES.ROM_info.title[0], "STREET FIGHTER ALPHA", 20) == 0)
			||
			(strncmpi((char*)&SNES.ROM_info.title[0], "STAR OCEAN", 10) == 0)
			)
		){
			//Enable 16M EWRAM (TWL)
			u32 SFGEXT9 = *(u32*)0x04004008;
			//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
			SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x2 << 14);
			*(u32*)0x04004008 = SFGEXT9;
			ROM_MAX_SIZE = ROM_MAX_SIZE_TWLMODE;
			//DKC3 needs this
			if(strncmpi((char*)&SNES.ROM_info.title[0], "DONKEY KONG COUNTRY 3", 21) == 0){
				ROM = (char *)SNES_ROM_ADDRESS_TWL;
			}
			//Otherwise the rest default NTR ROM base, or segfaults occur.
			else{
				ROM = (char *)SNES_ROM_ADDRESS_NTR;
			}
			if(strncmpi((char*)&SNES.ROM_info.title[0], "STREET FIGHTER ALPHA", 20) == 0){
				setCpuClock(true); //true: 133Mhz (TWL Mode only)
			}
			else{
				setCpuClock(false); //false: 66Mhz (NTR/TWL default CPU speed)
			}
			printf("Extended TWL Mem.");
		}
		else{
			if(__dsimode == true){
				//Enable 4M EWRAM (TWL)
				u32 SFGEXT9 = *(u32*)0x04004008;
				//14-15 Main Memory RAM Limit (0..1=4MB/DS, 2=16MB/DSi, 3=32MB/DSiDebugger)
				SFGEXT9 = (SFGEXT9 & ~(0x3 << 14)) | (0x0 << 14);
				*(u32*)0x04004008 = SFGEXT9;	
			}
			else{
				printf("This SnemulDS build is TWL mode only. Halting system.");
				while(1==1){}
			}
			ROM_MAX_SIZE = ROM_MAX_SIZE_NTRMODE;
			ROM = (char *)SNES_ROM_ADDRESS_NTR;
			printf("Normal NTR Mem.");
		}
		ROM_paging = (uchar *)((int)ROM+PAGE_SIZE); //SNES_ROM_PAGING_ADDRESS;
		ROM_PAGING_SIZE = (ROM_MAX_SIZE-PAGE_SIZE);
		
		//APU Fixes for proper sound speed
		if(
			(strncmpi((char*)&SNES.ROM_info.title[0], "MEGAMAN X3", 10) == 0)
			||
			(strncmpi((char*)&SNES.ROM_info.title[0], "MEGAMAN X2", 10) == 0)
			||
			(strncmpi((char*)&SNES.ROM_info.title[0], "DONKEY KONG COUNTRY 3", 21) == 0)
			){
			apuFix = 0;
			GUI_printf("APU Fix");
		}
		else{
			apuFix = 1;
		}
		initSNESEmpty(&uninitializedEmu, apuFix);
		memset((u8*)ROM, 0, (int)ROM_MAX_SIZE);	//Clear memory
		clrscr();
		GUI_printf(" - - ");
		GUI_printf(" - - ");
		GUI_printf("File:%s - Size:%d", CFG.ROMFile, size);
		if (size-ROMheader > ROM_MAX_SIZE){
			FS_loadROMForPaging(ROM-ROMheader, CFG.ROMFile, PAGE_SIZE+ROMheader);
			CFG.LargeROM = true;
			crc = crc32(0, ROM, PAGE_SIZE);
			GUI_printf("Large ROM detected. CRC(1Mb) = %08x ", crc);
		}
		else{
			FS_loadROM(ROM-ROMheader, CFG.ROMFile);
			CFG.LargeROM = false;
			crc = crc32(0, ROM, size-ROMheader);
			GUI_printf("CRC = %08x ", crc);
		}
		coherent_user_range_by_size((uint32)&savedUserSettings[0], (int)sizeof(savedUserSettings));	
		memcpy((void*)0x027FF000, (void*)&savedUserSettings[0], sizeof(savedUserSettings));	//restore them
		return reloadROM(ROM-ROMheader, size, crc, nameItem->filenameFromFS_getDirectoryListMethod);
	}
	return false;
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

char args[8][MAX_TGDSFILENAME_LENGTH];
char *argvs[8];

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int argc, char ** argv){
	/*			TGDS 1.6 Standard ARM9 Init code start	*/
	
	bool isTGDSCustomConsole = true;	//reloading cause issues. Thus this ensures Console to be inited even when reloading
	GUI_init(isTGDSCustomConsole);
	
	//xmalloc init removes args, so save them
	int i = 0;
	for(i = 0; i < argc; i++){
		argvs[i] = argv[i];
	}

	bool isCustomTGDSMalloc = true;
	setTGDSMemoryAllocator(getProjectSpecificMemoryAllocatorSetup(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, isCustomTGDSMalloc, TGDSDLDI_ARM7_ADDRESS));
	
	isTGDSCustomConsole = true;
	GUI_init(isTGDSCustomConsole);
	GUI_clear();
	sint32 fwlanguage = (sint32)getLanguage(); //get language once User Settings have been loaded
	GUI_setLanguage(fwlanguage);
	
	//argv destroyed here because of xmalloc init, thus restore them
	for(i = 0; i < argc; i++){
		argv[i] = argvs[i];
	}

	asm("mcr	p15, 0, r0, c7, c10, 4");
	flush_icache_all();
	flush_dcache_all();
	
	int ret=FS_init(); 
	if (ret == 0)
	{
		GUI_printf(_STR(IDS_FS_SUCCESS));
	}
	else{
		GUI_printf(_STR(IDS_FS_FAILED));
	}
	
	/*			TGDS 1.6 Standard ARM9 Init code end	*/
	
	REG_IME = 0;
	setSnemulDSSpecial0xFFFF0000MPUSettings();
	//TGDS-Projects -> legacy NTR TSC compatibility
	
	REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	//Set up PPU IRQ: HBLANK/VBLANK/VCOUNT
	REG_DISPSTAT = (DISP_HBLANK_IRQ | DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
	REG_IE |= (IRQ_HBLANK| IRQ_VBLANK);		
	
	//Set up PPU IRQ Vertical Line
	setVCountIRQLine(TGDS_VCOUNT_LINE_INTERRUPT);
	irqDisable(IRQ_VCOUNT|IRQ_TIMER1);	//SnemulDS abuses HBLANK IRQs, VCOUNT IRQs seem to cause a race condition
	REG_IME = 1;
	
	if(__dsimode == true){
		TWLSetTouchscreenTWLMode();
	}
	swiDelay(1000);
	
#ifndef DSEMUL_BUILD	
	GUI.printfy = 32;
    GUI.printfy += 32; // FIXME
#endif	
	
	memset(&startFilePath, 0, sizeof(startFilePath));
	memset(&startSPCFilePath, 0, sizeof(startSPCFilePath));
	SNEMULDS_IPC->APU_ADDR_CNT = SNEMULDS_IPC->APU_ADDR_ANS = SNEMULDS_IPC->APU_ADDR_CMD = 0;
	update_spc_ports();
	uninitializedEmu = true;
	
	// Clear "HDMA"
	for (i = 0; i < 192; i++){
		GFX.lineInfo[i].mode = -1;
	}
	
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
	
	strcpy(&CFG.ROMFile[0], "");
	memset(&guiSelItem, 0, sizeof(guiSelItem));
	guiSelItem.StructFDFromFS_getDirectoryListMethod = FT_FILE;
	
	switchToTGDSConsoleColors();
	
	//#define TSCDEBUG
	#ifdef TSCDEBUG
	clrscr();
	printf("--");
	printf("--");
	printf("--");
	
	while(1==1){
		scanKeys();
		u32 keys = keysDown();
		if(keys & KEY_TOUCH){
			struct touchPosition touch;
			// Deal with the Stylus.
			XYReadScrPosUser(&touch);
			
			printf("px: %d  py: %d ",touch.px, touch.py);
		}
	}
	#endif

	//ARGV Support: Only supported through TGDS chainloading.
	bool isSnesFile = false;
	if (argc > 2) {
		//arg 0: original NDS caller
		//arg 1: this NDS binary called
		//arg 2: this NDS binary's ARG0: filepath
		//arg 3: "dummy.arg"
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
		isSnesFile = loadROM(&guiSelItem);
	}
	while(isSnesFile == false);
	///////////////////////////////////////////
	if (!(argc > 2)) { 
		GUI_deleteROMSelector(); 	//Should also free ROMFile
	}
	
	switchToSnemulDSConsoleColors();
	GUI_createMainMenu();	//Start GUI
	
	//Some games require specific hacks to run
    if(strncmp((char*)&SNES.ROM_info.title[0], "BREATH OF FIRE 2", 16) == 0){
      APU_command(SNEMULDS_APUCMD_FORCESYNCON);
    }
	
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
				bool firstTime = false;
				initSNESEmpty(&uninitializedEmu, apuFix);

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
					isSnesFile = loadROM(&guiSelItem);
				}
				while(isSnesFile == false);
				///////////////////////////////////////////
				GUI_createMainMenu();	//	Start GUI
			}
			
			if(handleSPCSelect==true){
				handleSPCSelect=false;
				
				if (CFG.Sound_output || CFG.Jukebox)
					APU_pause();
				
				memset(&guiSelItem, 0, sizeof(guiSelItem));
				char * fileName = GUI_getSPCList(startSPCFilePath);
				guiSelItem.StructFDFromFS_getDirectoryListMethod = FT_FILE;
				guiSelItem.filenameFromFS_getDirectoryListMethod = (char*)fileName;
				selectSong(fileName);
				
				GUI_createMainMenu();	//Start GUI
			}
			GUI_update();
		}
		
		if (!SNES.Stopped){
			go();
		}
	}

	return 0;
}
