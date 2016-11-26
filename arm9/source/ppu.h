#ifndef ppu_snemulds
#define ppu_snemulds

//#define, BG_MAP_RAM(base) ((u16*)(((base)*0x800) + 0x06000000))   //already in libnds
#define BG_MAP_RAM_0x06020000(base) (u16*)((u16*)(((base)*0x800) + 0x06020000))
#define BG_MAP_RAM_0x06040000(base) (u16*)((u16*)(((base)*0x800) + 0x06040000))
#define BG_MAP_RAM_0x06060000(base) (u16*)((u16*)(((base)*0x800) + 0x06060000))

//#define BG_TILE_RAM(base)   ((u16*)(((base)*0x4000) + 0x06000000))    //already in libnds
#define BG_TILE_RAM_0x06020000(base)   (u16*)((u16*)(((base)*0x4000) + 0x06020000))    
#define BG_TILE_RAM_0x06040000(base)   (u16*)((u16*)(((base)*0x4000) + 0x06040000))    
#define BG_TILE_RAM_0x06060000(base)   (u16*)((u16*)(((base)*0x4000) + 0x06060000))


#endif

#include <nds.h>

#ifdef __cplusplus
extern "C" {
#endif

int	map_duplicate(int snes_block);


#ifdef __cplusplus
}
#endif