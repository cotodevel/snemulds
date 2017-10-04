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

#include "socket.h"
#include "in.h"
#include <netdb.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "gui.h"
#include "fs.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "ram.h"
#include "conf.h"
#include "snemul_str.h"
#include "frontend.h"
#include "main.h"
#include "dldi.h"
#include "ppu.h"
#include "InterruptsARMCores_h.h"
#include "specific_shared.h"
#include "ff.h"
#include "mem_handler_shared.h"
#include "reent.h"
#include "sys/types.h"
#include "engine.h"
#include "core.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefs.h"
#include "console.h"
#include "api_wrapper.h"
#include "apu_jukebox.h"
#include "toolchain_utils.h"
#include "about.h"

//wnifilib: multiplayer
#include "http_utils.h"
#include "client_http_handler.h"
#include "nifi.h"

#include "devoptab_devices.h"
#include "fsfat_layer.h"
#include "usrsettings.h"

#include "video.h"
#include "keypad.h"

int argc;
sint8 **argv;
int main(int _argc, sint8 **_argv) {
	
	IRQInit();
	
	//argc=_argc, argv=_argv;
	
	SpecificIPC->APU_ADDR_CNT = 0;
	SpecificIPC->APU_ADDR_CMD = 0;
	
	screen_mode = 0;
	SETDISPCNT_MAIN(0); //not using the main screen
	SETDISPCNT_SUB(MODE_0_2D | DISPLAY_BG0_ACTIVE); //sub bg 0 will be used to print text
	REG_BG0CNT = REG_BG1CNT = REG_BG2CNT = REG_BG3CNT = 0;
	
	/*
	// 256Ko for Tiles (SNES: 32-64Ko) 
	vramSetBankA(VRAM_A_MAIN_BG_0x06020000);
	vramSetBankB(VRAM_B_MAIN_BG_0x06040000);

	// 128Ko (+48kb) for sub screen / GUI 
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);

	// Some memory for ARM7 (128 Ko!)
	vramSetBankD(VRAM_D_ARM7_0x06000000);

	// 80Ko for Sprites (SNES : 32-64Ko) 
	vramSetBankE(VRAM_E_MAIN_SPRITE); // 0x6400000

	vramSetBankF(VRAM_F_MAIN_SPRITE);

	vramSetBankG(VRAM_G_BG_EXT_PALETTE);

	// 48ko For CPU 

	vramSetBankH(VRAM_H_LCD);
	vramSetBankI(VRAM_I_LCD);
	
	// 32 first kilobytes for MAP 
	// remaning memory for tiles 
	*/
	VRAM_SETUP(SNEMULDS_2DVRAM_SETUP());
	GUI_init();
	
	//Init DS Firmware Settings
	while(getFWSettingsstatus() == false){
		//TODO: add swidelay here
	}
	sint32 fwlanguage = (sint32)getLanguage();
	GUI_setLanguage(fwlanguage);
	
	//These are internal printf just use printf / iprintf if unsure
	GUI.printfy = 32;
	GUI_align_printf(GUI_TEXT_ALIGN_CENTER, RetTitleCompiledVersion());
	GUI_align_printf(GUI_TEXT_ALIGN_CENTER, (sint8*)SNEMULDS_SUBTITLE[0]);
    GUI.printfy += 32; // FIXME
	GUI_align_printf(GUI_TEXT_ALIGN_CENTER, _STR(4));
	
	initSNESEmpty();
	update_ram_snes();
    
	
	int i=0;
	// Clear "HDMA"
	for (i = 0; i < 192; i++){
		GFX.lineInfo[i].mode = -1;
	}
	
	// generate an exception
	//*(unsigned int*)0x02000004 = 0x64;
  
	//while(1==1){
	//}
	
	//cache test
	/*
	printf("vartest ori:%x",(sint32)(uint32)vartest);
	uint32 * ptruncached = (uint32*)EWRAMUncached((uint32)&vartest);
	
	uint32 * ptrcached = (uint32*)EWRAMCached((uint32)&vartest);
	*ptrcached = 0xabcdefed;
	printf("vartest:cachedwr:%x",*ptrcached);
	
	*ptruncached = 0x10111213;
	printf("vartest:uncachedwr:%x",*ptruncached);
	
	printf("vartest new:%x",(uint32)vartest);
	
	while(1==1){}
	*/
	
	//wifi test. nifi test will be later
	/*
	if(Wifi_InitDefault(true) == true)
	{
		printf("WIFI OK");
	}
	else{
		printf("WIFI FAIL");
	}
	while (1)
	{
		if ((keys & KEY_A))
		break;
	}
	*/
	
	
	
	//single player: (todo: player1 bugged dkc1)
	switch_dswnifi_mode((uint8)dswifi_idlemode);
	//nifi: 
	//switch_dswnifi_mode((uint8)dswifi_nifimode);
	//wifi: 
	//switch_dswnifi_mode((uint8)dswifi_wifimode);
	
	
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
	
	//ok
	//printf("devoptab_stdin.name:%s",devoptab_stdin.name);
	//printf("devoptab_stdout.name:%s",devoptab_stdout.name);
	//printf("devoptab_fatfs.name:%s",devoptab_fatfs.name);
	
	//instead of hardcoded fatfs, you will use getfatfsPath(sint8 * filename); for user file access
	//printf("%s",getfatfsPath("snes/smw.smc"));
	
	//test open/read:
	//rewrite ok
	/*
	int ret2 = FS_loadROMForPaging((sint8 *)&rom_buffer[0], getfatfsPath("snes/smw.smc"), ROM_STATIC_SIZE);
	printf("open %s ret: %d",getfatfsPath("snes/smw.smc"),ret2);	//ok
	printf("smw.smc: %x",(uint32)*(uint32*)&rom_buffer[0]);	//ok
	while(1);
	*/
	
	//test write/close
	//rewrite ok
	/*
	sint8 * testvar = "all right this save function (write close) appears to work fine";
	FS_printlog(testvar);
	while(1);
	*/
	
	/*
	//posix test case must succeed: status OK
	clrscr();
	int SIZE = 1;
	int NUMELEM = 5;

	FILE* fd = NULL;
    sint8 * buff = malloc(ROM_STATIC_SIZE);

    fd = fopen_fs(getfatfsPath("test.txt"),"r+");

	if(NULL == fd)
    {
        printf("fopen()(fd) Error!!!. Create file named:%s",getfatfsPath("test.txt"));
		while(1);
    }

    printf("File(fd) opened successfully through fopen()");
	
	FILE * fd2 = fopen_fs(getfatfsPath("tst.txt"),"w+");
	if(NULL == fd2)
    {
        printf("fopen() Error!!!. Create file named:%s",getfatfsPath("tst.txt"));
		while(1);
    }
	
	
	printf("File2(fd2) opened successfully through fopen()");
	
    if(SIZE*NUMELEM != fread_fs(buff,SIZE,NUMELEM,fd))
    {
        printf("fread() failed");
		while(1);
	}

    printf("Some bytes successfully read through fread()");
    printf("The bytes read are [%s] ",buff);
    if(0 != fseek_fs(fd,11,SEEK_CUR))
    {
        printf(" fseek()(fd) failed ");
		while(1);
	}
    printf(" fseek()(fd) successful");
	
	
	int ret3 = fwrite_fs(buff,SIZE,strlen(buff),fd);
    if(SIZE*NUMELEM != ret3)
    {
        printf(" fwrite()(fd) failed:%x",ret3);
		while(1);
	}
    
	
	
	int ret4 = fwrite_fs(buff,SIZE,strlen(buff),fd2);
    if(SIZE*NUMELEM != ret4)
    {
        printf(" fwrite()(fd2) failed:%x",ret4);
		while(1);
	}
    
	printf("filehandlesindex:%d:%d",fileno(fd),fileno(fd2));
	
	printf(" fwrite() successful (fd), data written to text file");
	fclose_fs(fd);
    
	printf(" fwrite() successful (fd2), data written to text file");
    fclose_fs(fd2);
    
	printf("File stream closed through fclose() (fd2)");
	printf("File stream closed through fclose() (fd)");
	
	while(1);
	*/
	
	
	//dldi new
	//printf("getdldifrommagic: %x",(uint32)getdldifrommagic());
	//minimaldldiDrvInst = getdldifrommagic();	//should work
	
	GUI_clear();
  
	
	//coto sbrk init
	//alloc/dealloc ok
	/*
	uint8 buf[256];
	sprintf((sint8*)buf,"linearalloc_end:%x",(int)(uint32*)this_heap_ptr);
	printf((sint8*)buf);
	
	uint8 buf2[256];
	sprintf((sint8*)buf2,"alloc:sbrk_ret:%x",(int)(uint32*)_sbrk(0x100));
	printf((sint8*)buf2);
	
	uint8 buf3[256];
	sprintf((sint8*)buf3,"free:sbrk_ret:%x",(int)(uint32*)_sbrk(-0x100));
	printf((sint8*)buf3);
	*/
	
	//alloc / realloc
	/*
	int freemem = get_available_mem();
	uint8 buf1[256];
	sprintf((sint8*)buf1,"freemem:%x",(int)freemem);
	printf((sint8*)buf1);
	
	uint32 * buf =(uint32*)_sbrk (freemem);
	
	uint8 buf3[256];
	sprintf((sint8*)buf3,"thisshouldbeok:%x", (int)buf);
	printf((sint8*)buf3);
	
	uint8 buf2[256];
	sprintf((sint8*)buf2,"thisshouldfail:%x", _sbrk (freemem));
	printf((sint8*)buf2);
	
	uint8 buf4[16];
	sprintf((sint8*)buf4,"allok");
	printf((sint8*)buf4);
	*/
	
	/*
	sint8 *p = sbrk(10);
	printf("last malloc = %p / %x", p, (int)p);
	
	sint8 *p2 = sbrk(-10);
	printf("last free = %p / %x", p2, (int)p2);
	*/
	
	/*
	//top / bottom memory tests
	int freesize = get_available_mem();
	//printf((sint8*)"freemem: %x / this should fail:%x",(int)freesize,sbrk(-1));	//ok
	printf((sint8*)"shouldbeok:%x",sbrk(freesize));
	printf((sint8*)"this should fail:%x / libc top:%x",sbrk(1),((uint32*)&__end__));
	printf((sint8*)"free:shouldbeok:%x",sbrk(-freesize));
	*/
	
	
	//coto sbrk end
	/* // DESCRIPTOR ok
	METHOD_DESCRIPTOR * method_signature = callback_append_signature((uint32*)&cback_build, (uint32*)&cback_build_end, (METHOD_DESCRIPTOR *)&Methods[0]);
	printf((sint8*)"address: %x , size: %d     ",method_signature->cback_address,method_signature->cback_size);
	
	volatile uint8 buf[1024*2];	//2K function buffer
	int retwrite = callback_export_file("armcallback.bin",method_signature);
	
	if(retwrite == -1){
		printf((sint8*)"export Error");
	}
	else{
		printf((sint8*)"export OK");
	}
	*/
	
	/*
	//VERSION //ok
	printf((sint8*)"EmuCoreVersion:%s",appver);
	*/
	
	// Load SNEMUL.CFG
	set_config_file(getfatfsPath("snemul.cfg"));	//correct: getfatfsPath("snemul.cfg")
	
	//printf((sint8*)"ReadSettingOldVal:%s",GUI_getConfigStr((sint8*)"GUI", (sint8*)"FileChooserOrder", NULL));
	//GUI_setConfigStr((sint8*)"GUI", (sint8*)"FileChooserOrder", "2");	//does not update
	//GUI_setConfigStrUpdateFile((sint8*)"GUI", (sint8*)"FileChooserOrder", "2");	//does update
	//printf((sint8*)"ReadSettingNewVal:%s",GUI_getConfigStr((sint8*)"GUI", (sint8*)"FileChooserOrder", NULL));	
	//printf((sint8*)"updateAssemblyParamsConfig:%d",updateAssemblyParamsConfig());	//ok
	
	//ok
	/*
	METHOD_DESCRIPTOR * method_signature = callback_append_signature((uint32*)&cback_build, (uint32*)&cback_build_end, (sint8*)"ARMHandlerWidget1",(METHOD_DESCRIPTOR *)&Methods[0]);
	printf((sint8*)"glueARMHandlerConfig:%d",glueARMHandlerConfig(&Version[0],method_signature));
	
	while (1)
	{
		
		if ((keys & KEY_A))
		break;
	}
	*/
	
	//standard sbrk 
	/*
	printf((sint8*)"ewrambase:%x",(int)&__ewram_base);
	printf((sint8*)"ewramend:%x",(int)&__eheap_end);
	
	sint8 *p = sbrk(10);
	printf("last malloc = %p / %x", p, (int)p);
	*/
	
	//Coto: newlib malloc test. PTR returned should NOT be 0 otherwise sbrk implementation is wrong.
	/*
	sint8 *p = malloc(10);
	printf("last malloc = %p / %x", p, (int)p);
	while(1==1){}	//so far ok
	*/
	
	
	
	
	for (i = 0; i < 100; i++)	GUI_clear();
	
	sint8 *ROMfile;
	if(readFrontend(&ROMfile,(sint8**)&CFG.ROMPath[0])){
		readOptionsFromConfig("Global");
		GUI_getConfig();
	}else{
		
		//old: CFG.ROMPath = get_config_string(NULL, "ROMPath", (sint8*)READ_GAME_DIR[0]);	//put 3rd arg into 2nd of CFG."Name"
		sprintf(CFG.ROMPath,"%s",get_config_string(NULL, "ROMPath", (sint8*)READ_GAME_DIR[0]));
		
		readOptionsFromConfig("Global");
		GUI_getConfig();
		
		//printf("readFrontend phail!");	//here
		//while(1);
		
		//parse init dir correctly (dir format)
		sprintf(CFG.ROMPath,"%s",getfatfsPath(CFG.ROMPath));
		//printf("Loading:%s",CFG.ROMPath);
		//while(1);
	
		
		//touchscreen new
		/*
		while (1)
		{
			if ((keysPressed() & KEY_A)){
				break;
			}
			
			if ((keysPressed() & KEY_B)){
				GUI_clear();
				//printf("test %d",0x00000001);
				//iprintf("also a test %x",0xc1c2c3c4);
				//iprintf("openfddevices %d, realsize: %d",(sint32)open_posix_filedescriptor_devices(),sizeof(struct devoptab_t));
				//printf("rompath:%s",CFG.ROMPath);
				
				//GUI_printf("test");
				//GUI_printf("TSCPENIRQ:%d",(sint32)penIRQread());	//ok
				//GUI_printf("X Tsc:%x",(uint16)SpecificIPC->touchX);	//OK
				//GUI_printf("Y Tsc:%x",(sint32)SpecificIPC->touchY);	//OK
				
				//replace with printf
				//printf("pxX Tsc:%d",(uint16)SpecificIPC->touchXpx);
				//printf("pxY Tsc:%d",(uint16)SpecificIPC->touchYpx);
				//printf("debugvar :%x",(uint32)SpecificIPC->debugvar);
				//printf("debugvar2 :%x",(uint32)SpecificIPC->debugvar2);
				
				//printf("nickname:%s",(char*)&SpecificIPC->nickname_schar8[0]);
				//printf("bdaymonth:%d",(sint32)SpecificIPC->DSFWSETTINGSInst.birthday_month);
				//printf("bdayday:%d",(sint32)SpecificIPC->DSFWSETTINGSInst.birthday_day);
				//printf("consoletype:%d",(sint32)SpecificIPC->consoletype);
				
				//printf("lang:%d",(unsigned int)fwlanguage);
				
				
				struct tm * tmInst = getTime();
				
				printf("0:%d",(unsigned int)tmInst->tm_sec);
				printf("1:%d",(unsigned int)tmInst->tm_min);
				printf("2:%d",(unsigned int)tmInst->tm_hour);
				printf("3:%d",(unsigned int)tmInst->tm_mday);
				printf("4:%d",(unsigned int)tmInst->tm_mon);
				printf("5:%d",(unsigned int)tmInst->tm_year);
				
				
				if(SpecificIPC->valid_dsfwsettings == true){
					printf("DS User settings read OK");
				}
				else{
					printf("DS User settings read ERROR");
				}
				
			}
			
			IRQWait(1,IRQ_VBLANK);
		}
		*/
		GUI_getROM(CFG.ROMPath);	//read rom from (path)touchscreen:output rom -> CFG.ROMFile
		//CFG.ROMPath corrupted here
		
		
		//printf("Loading:%s",CFG.Fullpath);
		//while(1);
		
	}
	
	//printf("Loading:%s",CFG.ROMPath);
	//while(1);
	//test implementation of SendMultipleWordACK: check if fifo was executed OK in other core
	//bool retval = SendMultipleWordACK(WIFI_SYNC, 0, 0, 0);
	//if(retval == true){
	//	printf("reply:ok");
	//}
	//else{
	//	printf("reply:false");
	//}
	//while(1);
	
	//printf("GUI_getROM():%s",ROMfile);	//rets filename	/rom.smc ALONE
	loadROM(CFG.ROMFile, 0);	//loadROM reads name only CFG.ROMFile
	GUI_deleteROMSelector(); // Should also free ROMFile
	GUI_createMainMenu();
    
    //ok so far
	
	while (1)
	{
        if (REG_POWERCNT & POWER_SWAP_LCDS){
			GUI_update();
		}
        
        if (keys & KEY_LID)
		{
			saveSRAM();
			APU_pause();
			//			APU_stop();
			// hinge is closed 
			// power off everything not needed 
			powerOFF(POWERMAN_ARM9 | POWER_LCD | POWER_2D_A | POWER_2D_B | POWER_SWAP_LCDS) ;
			// set system into sleep 
			while (keys & KEY_LID)
			{
				keys = keysPressed();
			}
			// wait a bit until returning power 
			// power on again 
			powerON(POWERMAN_ARM9 | POWER_LCD | POWER_2D_A | POWER_2D_B | POWER_SWAP_LCDS);
			// set up old irqs again 
			APU_pause();
		}
		
		
        if (!SNES.Stopped){
            go();   //boots here
        }
        
		
	}

}