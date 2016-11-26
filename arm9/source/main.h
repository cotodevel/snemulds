#ifndef MAIN_SNEMULDS
#define MAIN_SNEMULDS
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
#endif


#ifdef __cplusplus
extern "C" {
#endif

void	PPU_updateGFX(int line);
void	APU_clear();
void 	PPU_ChangeLayerConf(int i);
int 	saveSRAM();
void	GUI_clear();
void 	mem_clear_paging();
int		FS_loadROMInExtRAM(char *ROM, char *filename, int size, int total_size);
u32	    crc32 (unsigned int crc, const void *buffer, unsigned int size);
int		changeROM(char *ROM, int size);
void	APU_stop();
int		FS_loadFile(char *filename, char *buf, int size);
void	APU_playSpc();
void 	GUI_setLanguage(int lang);
void	GUI_align_printf(int flags, char *fmt, ...);
int 	initSNESEmpty();
void 	GUI_getConfig();
char 	*GUI_getROM(char *rompath);
void 	GUI_deleteROMSelector();
void 	GUI_createMainMenu();
void	APU_pause();
int 	go();

//main.c
//extern int CPU_goto();

#ifdef ASM_OPCODES
//extern int CPU_init();
//extern int CPU_goto2();
#endif

extern char logbuf[];
extern uint32 AsmDebug[16];


//NDS MEMORY
//DTCM TOP full memory
extern u32 dtcm_top_ld __attribute__((section(".dtcm")));
//DTCM TOP reserved by compiler/user memory
extern u32 dtcm_end_alloced __attribute__((section(".dtcm")));

extern unsigned int __ewram_start;
extern unsigned int __ewram_end;


typedef int (*intfuncptr)();
typedef u32 (*u32funcptr)();
typedef void (*voidfuncptr)();


//snapshot.c
void	FS_lock();
void	FS_unlock();
void	APU_stop();
void	APU_loadSpc();
void	GUI_console_printf(int x, int y, char *fmt, ...);
void	APU_saveSpc();



#ifdef __cplusplus
}
#endif