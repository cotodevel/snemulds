/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include <nds.h>
#include <nds/registers_alt.h>
#include <malloc.h>
#include <string.h>

#include "common.h"
#include "gfx.h"
#include "snes.h"
#include "cfg.h"

extern int		screen_mode; // NDS MAIN screen mode

uint8   sprite_tiles_def[1024];
uint8   tiles_def[4][1024];

/* should be 64 bytes long */
typedef struct s_OAM_entry
{
	uint8 Y;
	uint8 rot_data:2;
	uint8 mode:2;
	uint8 mosaic:1;
	uint8 color_depth:1;
	uint8 shape:2;
	
	uint16 X:9;
	uint8 rot_data2:3;
	uint8 flip:2;
	uint8 size:2;
	
	uint16 tile_index:10;
	uint8 pr:2;
	uint8 palette:4;
	
	uint16 rot_data3;
} t_OAM_entry;


extern u32 keys;

int get_joypad()
{
	int res = 0;

#define KEYS_CUR (( ((~REG_KEYINPUT)&0x3ff) | (((~IPC->buttons)&3)<<10) | \
	 			 (((~IPC->buttons)<<6) & (KEY_TOUCH|KEY_LID) ))^KEY_LID)	
	keys = KEYS_CUR; 
/*	scanKeys();	
	keys = keysHeld();*/
	if( keys & KEY_B ) res |= 0x8000;
	if( keys & KEY_Y ) res |= 0x4000;
	if( keys & KEY_SELECT ) res |= 0x2000;
	if( keys & KEY_START ) res |= 0x1000;
	if( keys & KEY_UP ) res |= 0x0800;
	if( keys & KEY_DOWN ) res |= 0x0400;
	if( keys & KEY_LEFT ) res |= 0x0200;
	if( keys & KEY_RIGHT ) res |= 0x0100;
	if( keys & KEY_A ) res |= 0x0080;
	if( keys & KEY_X ) res |= 0x0040;
	if( keys & KEY_L ) res |= 0x0020;
	if( keys & KEY_R ) res |= 0x0010;
	return res;
}

uint16 bittab[16];

void    init_render()
{
  int  PixelOdd = 1;
  int   h;
  int   i;

/* 2/4 bits in planar mode to 4 bits converter */
  for (i = 0;i < 16; i++)
    {
      h = 0;
      if (i & 8) h |= PixelOdd;
      if (i & 4) h |= PixelOdd << 4;
      if (i & 2) h |= PixelOdd << 8;
      if (i & 1) h |= PixelOdd << 12;
      bittab[i] = h;
    }
}

void check_sprite_addr()
{ 
	GFX.tiles_dirty = 1;	
}

void check_tile_addr()
{
	GFX.tiles_dirty = 1;	
}

void check_tile()
{
    int tmp;
    int		addr = (SNES.PPU_Port[0x16]<<1)&0xFFFF;

//GFX.tiles_dirty = 1;
   // FIXME: adapt? selon le mode graphique

   tmp = (addr-GFX.tile_address[0])/32;
   if (tmp >= 0 && tmp < 1024)
    	tiles_def[0][tmp] = 0;
   tmp = (addr-GFX.tile_address[1])/32;
   if (tmp >= 0 && tmp < 1024)
    	tiles_def[1][tmp] = 0;   
   tmp = (addr-GFX.tile_address[2])/16;
   if (tmp >= 0 && tmp < 1024)
    	tiles_def[2][tmp] = 0;   
    	
  tmp = (addr-GFX.spr_addr_base)/32;    	
   if (tmp >= 0 && tmp < 1024)
    	sprite_tiles_def[tmp] = 0;   
  tmp = (addr-GFX.spr_addr_base-GFX.spr_addr_select)/32;    	
   if (tmp >= 0 && tmp < 1024)
    	sprite_tiles_def[tmp] = 0;   
/*
  GFX.tiles_def[0][addr/16] = 0;
  GFX.tiles_def[1][addr/32] = 0;
  GFX.tiles_def[2][addr/64] = 0;
  	*/
}

#define CONVERT_SPR_TILE(tn) (((tn)&0xF)|(((tn)>>4)<<5))
//#define CONVERT_SPR_TILE(tn) (tn)

#define SNES_VRAM_OFFSET ((SNES_Port[0x01]&0x03) << 14)

void     add_sprite_tile_4(uint16 tilenb, int pos)
{
  uint8		a;
  int		k;
  uint16	c1,c2;
  uint8		*tile_ptr;
  uint32	tile_addr;
  uint16	*VRAM_ptr;

  if (tilenb&0x100)
    tile_addr = (tilenb+pos)*32+GFX.spr_addr_base+GFX.spr_addr_select;
  else
    tile_addr = (tilenb+pos)*32+GFX.spr_addr_base;
 /* if (GFX.tiles_def[1][tile_addr/32] == 1)
  	return;*/    
  if (sprite_tiles_def[tilenb+pos])
    return;  	
  VRAM_ptr = SPRITE_GFX + CONVERT_SPR_TILE(tilenb+pos)*16;    
  tile_ptr = SNESC.VRAM+tile_addr;

  for (k=0;k<8;k++,tile_ptr+=2)
    {
      c1 = c2 = 0;
      if ((a = tile_ptr[0x00]))
        {
          c1 |= bittab[(a>>4)];
          c2 |= bittab[(a&0xf)];
        }
      if ((a = tile_ptr[0x01]))
        {
          c1 |= bittab[(a>>4)]<<1;
          c2 |= bittab[(a&0xf)]<<1;
        }
      if ((a = tile_ptr[0x10]))
        {
          c1 |= bittab[(a>>4)]<<2;
          c2 |= bittab[(a&0xf)]<<2;
        }
      if ((a = tile_ptr[0x11]))
        {
          c1 |= bittab[(a>>4)]<<3;
          c2 |= bittab[(a&0xf)]<<3;
        }
      
		*VRAM_ptr++ = c1;      
		*VRAM_ptr++ = c2;
    }
// GFX.tiles_def[1][tile_addr/32] = 1;    
  sprite_tiles_def[tilenb+pos] = 1;    
}

void     add_tile_2(uint16 tilenb, uint8 bg)
{
  uint8		a;
  int		k;
  uint16	c1,c2;
  uint8		*tile_ptr;
  uint32	tile_addr;
  uint16	*VRAM_ptr;

  tile_addr = GFX.tile_address[bg]+tilenb*16;
/*  if (GFX.tiles_def[0][tile_addr/16] == 1)
  	return;*/
  VRAM_ptr = (uint16 *)(BG_TILE_RAM(2+bg*2)+tilenb*32);  
  tile_ptr = SNESC.VRAM+tile_addr;    

  for (k=0;k<8;k++,tile_ptr+=2)
    {
      c1 = c2 = 0;
      if ((a = tile_ptr[0x00]))
        {
          c1 |= bittab[(a>>4)];
          c2 |= bittab[(a&0xf)];
        }
      if ((a = tile_ptr[0x01]))
        {
          c1 |= bittab[(a>>4)]<<1;
          c2 |= bittab[(a&0xf)]<<1;
        }
      
		*VRAM_ptr++ = c1;      
		*VRAM_ptr++ = c2;
    }
    
//  GFX.tiles_def[0][tile_addr/16] = 1;    
  tiles_def[bg][tilenb] = 1;    
}

void     add_tile_4(uint16 tilenb, uint8 bg)
{
  uint8		a;
  int		k;
  uint16	c1,c2;
  uint8		*tile_ptr;
  uint32	tile_addr;  
  uint16	*VRAM_ptr;

  tile_addr = GFX.tile_address[bg]+tilenb*32;
/*  if (GFX.tiles_def[1][tile_addr/32] == 1)
  	return;*/       
//  iprintf("%d %d\n", tilenb, bg);
  tile_ptr = SNESC.VRAM+tile_addr;    
  VRAM_ptr = (uint16 *)(BG_TILE_RAM(2+bg*2)+tilenb*32);    
//  VRAM_ptr = (uint16 *)(BG_TILE_RAM(2+bg*2)+tilenb*32);

  for (k=0;k<8;k++,tile_ptr+=2)
    {
      c1 = c2 = 0;
      if ((a = tile_ptr[0x00]))
        {
          c1 |= bittab[(a>>4)];
          c2 |= bittab[(a&0xf)];
        }
      if ((a = tile_ptr[0x01]))
        {
          c1 |= bittab[(a>>4)]<<1;
          c2 |= bittab[(a&0xf)]<<1;
        }
      if ((a = tile_ptr[0x10]))
        {
          c1 |= bittab[(a>>4)]<<2;
          c2 |= bittab[(a&0xf)]<<2;
        }
      if ((a = tile_ptr[0x11]))
        {
          c1 |= bittab[(a>>4)]<<3;
          c2 |= bittab[(a&0xf)]<<3;
        }
      
		*VRAM_ptr++ = c1;      
		*VRAM_ptr++ = c2;
    }
//  GFX.tiles_def[1][tile_addr/32] = 1;    
  tiles_def[bg][tilenb] = 1;    
}


void add_tile_8(short tilenb, unsigned char bg)
{
	// TO DO
}


inline void	PPU_setMap(int i, int j, int tilenb, int bg, int p, int f)
{
  uint16 *map_addr = (uint16 *)BG_MAP_RAM(bg*4);

  if (bg == 2)
  {
/*  if ((SNES.PPU_Port[0x05]&7) == 0)
  	p += bg*2;*/

  	// FIXME : 2 bits palettes here
    p += 8; 
    *(map_addr + i + j*32) = (tilenb) | (f << 10) | (p << 12);
  }
  else
  {
  	if (i == 32)
  	  *(map_addr + 1024 + j*32) = (tilenb) | (f << 10) | (p << 12);
  	else
  	*(map_addr + i + j*32) = (tilenb) | (f << 10) | (p << 12);
  }		
}


#define DRAW_TILE(I, J, TILENB, BG, P, F) \
	PPU_setMap(I, J, TILENB, BG, P, F) 

/*\
 if (tiles[TILENB][BG][P][F] != NULL) { \
   draw_rle_sprite(buf_screen, tiles[TILENB][BG][P][F], X, Y); \
   if (X > 0xf8 && Y > 0xf8) \
     draw_rle_sprite(buf_screen, tiles[TILENB][BG][P][F], X-0x100, Y-0x100); \
   if (X > 0xf8) draw_rle_sprite(buf_screen, tiles[TILENB][BG][P][F], X-0x100, Y); \
   if (Y > 0xf8) draw_rle_sprite(buf_screen, tiles[TILENB][BG][P][F], X, Y-0x100); \
 }
*/

static int _offsetY_tab[3] = { 16, 0, 32 };

void update_scroll()
{
  	int offsetY = _offsetY_tab[CFG.YScroll];
	
   BG0_X0 = (SNES.PPU_Port[(0x0D)+(0<<1)]&0x7);
   BG0_Y0 = (SNES.PPU_Port[(0x0E)+(0<<1)]&0x7)+offsetY;
   BG1_X0 = (SNES.PPU_Port[(0x0D)+(1<<1)]&0x7);
   BG1_Y0 = (SNES.PPU_Port[(0x0E)+(1<<1)]&0x7)+offsetY;
   BG2_X0 = (SNES.PPU_Port[(0x0D)+(2<<1)]&0x7);
   BG2_Y0 = (SNES.PPU_Port[(0x0E)+(2<<1)]&0x7)+16;
   BG3_X0 = (SNES.PPU_Port[(0x0D)+(3<<1)]&0x7);
   BG3_Y0 = (SNES.PPU_Port[(0x0E)+(3<<1)]&0x7)+16;
}


#define ADD_TILE(BG, TILENB, P, F, BG_MODE) \
  if (!tiles_def[BG][TILENB]) \
  switch (BG_MODE) { \
    case 2 : add_tile_2(TILENB, BG); break; \
    case 4 : add_tile_4(TILENB, BG); break; \
    case 8 : add_tile_8(TILENB, BG); break; \
  }

void draw_plane_32_30_squiz(unsigned char bg, unsigned char bg_mode)
{
  int i, j, j2, map_address;
  unsigned short tilenb, scrollx, scrolly;
  unsigned char nb_tilex, nb_tiley, tile_size, f, p;
  int delta = 0;

  if (SNES.PPU_Port[0x05]&(0x10 << bg)) {
    nb_tilex = 16; nb_tiley = SNES.PPU_Port[0x33]&4 ? 15 : 14; tile_size = 4;
  } else {
    nb_tilex = 32; nb_tiley = SNES.PPU_Port[0x33]&4 ? 30 : 28; tile_size = 3;
  }
  scrollx = SNES.PPU_Port[(0x0D)+(bg<<1)]; scrolly = SNES.PPU_Port[(0x0E)+(bg<<1)];
  map_address  = (SNES.PPU_Port[0x07+bg]&0xfc)<<0x9;

  if (CFG.BG3Squish == 0)
  	delta = 2;
  if (CFG.BG3Squish == 1)
  	delta = 1;
  j2 = delta;
  for (j=0; j <nb_tiley; j++) {
	if (nb_tilex == 32) {
	  if (j >= (16-delta) & j < (16+delta))
	    continue;
	} else
	{
	  if (j >= 8 &&  j < 9)
	    continue;  		
	}
  	
    for (i=0; i<nb_tilex; i++) {
    	
//      scrollx = scroll_x[bg][j]; scrolly = scroll_y[bg][j];
      tilenb = SNESC.VRAM[map_address+i*2+j*64]+(SNESC.VRAM[map_address+i*2+j*64+1]<<8);
      p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14; tilenb &= 0x3ff;
      if (nb_tilex == 32) {
        ADD_TILE(bg, tilenb, p, f, bg_mode);
        PPU_setMap(i, j2, tilenb, bg, p, f);
      } else {
      	// FIXME
/*        ADD_TILE(bg, tilenb,    p, f, bg_mode);
        ADD_TILE(bg, tilenb+1,  p, f, bg_mode);
        ADD_TILE(bg, tilenb+16, p, f, bg_mode);
        ADD_TILE(bg, tilenb+17, p, f, bg_mode);
        DRAW_TILE_2(i,   j2,   tilenb,    bg, p, f);
        DRAW_TILE_2(i+1, j2,   tilenb+1,  bg, p, f);
        DRAW_TILE_2(i,   j2+1, tilenb+16, bg, p, f);
        DRAW_TILE_2(i+1, j2+1, tilenb+17, bg, p, f);
        j2+=2;*/        
      }
      
    }
    j2++;    
  }
}


void draw_plane_32_30(unsigned char bg, unsigned char bg_mode)
{
  int i, j, map_address;
  unsigned short tilenb, scrollx, scrolly;
  unsigned char x, nb_tilex, nb_tiley, tile_size, f, p;

  if (SNES.PPU_Port[0x05]&(0x10 << bg)) {
    nb_tilex = 16; nb_tiley = SNES.PPU_Port[0x33]&4 ? 15 : 14; tile_size = 4;
  } else {
    nb_tilex = 32; nb_tiley = SNES.PPU_Port[0x33]&4 ? 30 : 28; tile_size = 3;
  }
  scrollx = SNES.PPU_Port[(0x0D)+(bg<<1)] >> tile_size; 
  scrolly = SNES.PPU_Port[(0x0E)+(bg<<1)] >> tile_size;
  map_address  = (SNES.PPU_Port[0x07+bg]&0xfc)<<0x9;

  for (j=0; j <nb_tiley; j++) {
  	uint16 *tile_ptr = (uint16 *)(SNESC.VRAM+map_address+(j+scrolly&0x1f)*64);
    for (i=0; i<nb_tilex; i++) {
      tilenb = tile_ptr[i+scrollx&0x1f];
      p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14; tilenb &= 0x3ff;
      if (nb_tilex == 32) {
        ADD_TILE(bg, tilenb, p, f, bg_mode);
        DRAW_TILE(i, j, tilenb, bg, p, f);
      } else {
        ADD_TILE(bg, tilenb,    p, f, bg_mode);
        ADD_TILE(bg, tilenb+1,  p, f, bg_mode);
        ADD_TILE(bg, tilenb+16, p, f, bg_mode);
        ADD_TILE(bg, tilenb+17, p, f, bg_mode);
        DRAW_TILE(i*2,   j*2,   tilenb,    bg, p, f);
        DRAW_TILE(i*2+1, j*2,   tilenb+1,  bg, p, f);
        DRAW_TILE(i*2,   j*2+1, tilenb+16, bg, p, f);
        DRAW_TILE(i*2+1, j*2+1, tilenb+17, bg, p, f);
      }
    }
  }
}

void draw_plane_64_30(unsigned char bg, unsigned char bg_mode)
{
  int i, j, map_address;
  unsigned short tilenb, scrollx, scrolly;
  unsigned char nb_tilex, nb_tiley, tile_size, f, p;
//  short x, y;

  if (SNES.PPU_Port[0x05]&(0x10 << bg)) {
    nb_tilex = 16; nb_tiley = SNES.PPU_Port[0x33]&4 ? 15 : 14; tile_size = 4;
  } else {
    nb_tilex = 32; nb_tiley = SNES.PPU_Port[0x33]&4 ? 30 : 28; tile_size = 3;
  }
  scrollx = SNES.PPU_Port[(0x0D)+(bg<<1)] >> tile_size;
  scrolly = SNES.PPU_Port[(0x0E)+(bg<<1)] >> tile_size;
  map_address  = (SNES.PPU_Port[0x07+bg]&0xfc)<<0x9;
  for (j=0; j<nb_tiley; j++) { 	
  	uint16 *tile_ptr = (uint16 *)(SNESC.VRAM+map_address+(j+scrolly&0x1f)*64);
    for (i=0; i<nb_tilex+1; i++) {
      if ((i+scrollx&0x20) == 0) {
      	tilenb = tile_ptr[i+scrollx&0x1f];
//        tilenb = *(uint16 *)(SNESC.VRAM+map_address+(i+scrollx&0x1f)*2+(j+scrolly&0x1f)*64);
      } else {
      	tilenb = tile_ptr[1024+(i+scrollx&0x1f)];
//        tilenb = *(uint16 *)(SNESC.VRAM+map_address+2048+(i+scrollx&0x1f)*2+(j+scrolly&0x1f)*64);
      }
      p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14; tilenb &= 0x3ff;
      if (nb_tilex == 32) {
        ADD_TILE(bg, tilenb, p, f, bg_mode);
        DRAW_TILE(i, j, tilenb, bg, p, f);
      } else {
        ADD_TILE(bg, tilenb,    p, f, bg_mode);
        ADD_TILE(bg, tilenb+1,  p, f, bg_mode);
        ADD_TILE(bg, tilenb+16, p, f, bg_mode);
        ADD_TILE(bg, tilenb+17, p, f, bg_mode);
        DRAW_TILE(i*2,   j*2,   tilenb,    bg, p, f);
        DRAW_TILE(i*2+1, j*2,   tilenb+1,  bg, p, f);
        DRAW_TILE(i*2,   j*2+1, tilenb+16, bg, p, f);
        DRAW_TILE(i*2+1, j*2+1, tilenb+17, bg, p, f);
      }
    }
  }
}

void draw_plane_32_60(unsigned char bg, unsigned char bg_mode)
{
  int i, j, map_address;
  unsigned short tilenb;
  unsigned char nb_tilex, nb_tiley, tile_size, f, p;
  short x, y, scrollx, scrolly;

  if (SNES.PPU_Port[0x05]&(0x10 << bg)) {
    nb_tilex = 16; nb_tiley = SNES.PPU_Port[0x33]&4 ? 15 : 14; tile_size = 4;
  } else {
    nb_tilex = 32; nb_tiley = SNES.PPU_Port[0x33]&4 ? 30 : 28; tile_size = 3;
  }
  scrollx = SNES.PPU_Port[(0x0D)+(bg<<1)] >> tile_size;  
  scrolly = SNES.PPU_Port[(0x0E)+(bg<<1)] >> tile_size;
  map_address  = (SNES.PPU_Port[0x07+bg]&0xfc)<<0x9;
  for (j=0; j<nb_tiley+1; j++) {
  	uint16 *tile_ptr;
  	if ((j+scrolly&0x20) == 0)
  	  tile_ptr = (uint16 *)(SNESC.VRAM+map_address+(j+scrolly&0x1f)*64);
    else
      tile_ptr = (uint16 *)(SNESC.VRAM+map_address+2048+(j+scrolly&0x1f)*64);
    for (i=0; i<nb_tilex; i++) {
      tilenb = tile_ptr[i+scrollx&0x1f];
/*      	tilenb = *(uint16 *)(SNESC.VRAM+map_address+(i+scrollx&0x1f)*2+(j+scrolly&0x1f)*64);
      else
      	tilenb = *(uint16 *)(SNESC.VRAM+map_address+2048+(i+scrollx&0x1f)*2+(j+scrolly&0x1f)*64);*/      	
      p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14; tilenb &= 0x3ff;
      if (nb_tilex == 32) {
        ADD_TILE(bg, tilenb, p, f, bg_mode);
        DRAW_TILE(i, j, tilenb, bg, p, f);
      } else {
        ADD_TILE(bg, tilenb,    p, f, bg_mode);
        ADD_TILE(bg, tilenb+1,  p, f, bg_mode);
        ADD_TILE(bg, tilenb+16, p, f, bg_mode);
        ADD_TILE(bg, tilenb+17, p, f, bg_mode);
        DRAW_TILE(i*2,   j*2,   tilenb,    bg, p, f);
        DRAW_TILE(i*2+1, j*2,   tilenb+1,  bg, p, f);
        DRAW_TILE(i*2,   j*2+1, tilenb+16, bg, p, f);
        DRAW_TILE(i*2+1, j*2+1, tilenb+17, bg, p, f);
      }
    }
  }
}
/*
void draw_block(int bg, int bg_mode,
                int i, int j, int map_address, int scrollx, int scrolly, int tile_size)
{
  unsigned char f, p;
  unsigned short tilenb;
  short x,y;

  if ((i+(scrollx >> 3)&0x20) == 0 && (j+(scrolly >> 3)&0x20) == 0) {
    tilenb = (SNESC.VRAM[map_address+(i+(scrollx >> 3)&0x1f)*2+(j+(scrolly >> 3)&0x1f)*64]+
             (SNESC.VRAM[map_address+(i+(scrollx >> 3)&0x1f)*2+(j+(scrolly >> 3)&0x1f)*64+1]<<8));
  } else {
    if ((i+(scrollx >> 3)&0x20) == 0x20 && (j+(scrolly >> 3)&0x20) == 0) {
      tilenb = (SNESC.VRAM[map_address+2048+(i+(scrollx >> 3)&0x1f)*2+(j+(scrolly >> 3)&0x1f)*64]+
               (SNESC.VRAM[map_address+2048+(i+(scrollx >> 3)&0x1f)*2+(j+(scrolly >> 3)&0x1f)*64+1]<<8));
    } else {
      if ((i+(scrollx >> 3)&0x20) == 0 && (j+(scrolly >> 3)&0x20) == 0x20) {
        tilenb = (SNESC.VRAM[map_address+4096+(i+(scrollx >> 3)&0x1f)*2+(j+(scrolly >> 3)&0x1f)*64]+
                 (SNESC.VRAM[map_address+4096+(i+(scrollx >> 3)&0x1f)*2+(j+(scrolly >> 3)&0x1f)*64+1]<<8));
      } else {
        if ((i+(scrollx >> 3)&0x20) == 0x20 && (j+(scrolly >> 3)&0x20) == 0x20)
          tilenb = (SNESC.VRAM[map_address+6144+(i+(scrollx >> 3)&0x1f)*2+(j+(scrolly >> 3)&0x1f)*64]+
                   (SNESC.VRAM[map_address+6144+(i+(scrollx >> 3)&0x1f)*2+(j+(scrolly >> 3)&0x1f)*64+1]<<8));
      }
    }
  }

  p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14; tilenb &= 0x3ff;
  y = ((j<<tile_size)-(scrolly&7)); x = ((i<<tile_size)-(scrollx&7));
  if (tile_size == 3) {
    if (!tiles_def[bg][tilenb][p][f])
      ADD_TILE(bg, tilenb, p, f, bg_mode);
    DRAW_TILE(x, y, tilenb, bg, p, f);
  } else {
    if (!tiles_def[bg][tilenb][p][f])
      ADD_TILE(bg, tilenb,    p, f, bg_mode);
    if (!tiles_def[bg][tilenb+1][p][f])
      ADD_TILE(bg, tilenb+1,  p, f, bg_mode);
    if (!tiles_def[bg][tilenb+16][p][f])
      ADD_TILE(bg, tilenb+16, p, f, bg_mode);
    if (!tiles_def[bg][tilenb+17][p][f])
      ADD_TILE(bg, tilenb+17, p, f, bg_mode);
    DRAW_TILE(x,   y,   tilenb,    bg, p, f);
    DRAW_TILE(x+8, y,   tilenb+1,  bg, p, f);
    DRAW_TILE(x,   y+8, tilenb+16, bg, p, f);
    DRAW_TILE(x+8, y+8, tilenb+17, bg, p, f);
  }
}
*/

int o;

void draw_plane_64_60(unsigned char bg, unsigned char bg_mode)
{
  int i, j, map_address;
  unsigned short tilenb, scrollx, scrolly;
  unsigned char nb_tilex, nb_tiley, tile_size, f, p;
  short x, y;
  uint16	*map_ptr;
  

  if (SNES.PPU_Port[0x05]&(0x10 << bg)) {
    nb_tilex = 16; nb_tiley = SNES.PPU_Port[0x33]&4 ? 15 : 14; tile_size = 4;
  } else {
    nb_tilex = 32; nb_tiley = SNES.PPU_Port[0x33]&4 ? 30 : 28; tile_size = 3;
  }
  scrollx = SNES.PPU_Port[(0x0D)+(bg<<1)]>>tile_size;
  scrolly = SNES.PPU_Port[(0x0E)+(bg<<1)]>>tile_size;
  map_address  = (SNES.PPU_Port[0x07+bg]&0xfc)<<0x9;
  for (j=0; j<nb_tiley+1; j++) {
  	map_ptr = (uint16 *)(SNESC.VRAM+map_address+(j+scrolly&0x1f)*64);
    for (i=0; i<nb_tilex+1; i++) {
      if ((i+scrollx&0x20) == 0 && (j+scrolly&0x20) == 0)
        tilenb = map_ptr[(i+scrollx&0x1f)];
      else 
      if ((i+scrollx&0x20) == 0x20 && (j+scrolly&0x20) == 0)
        tilenb = map_ptr[1024+(i+scrollx&0x1f)];
      else
      if ((i+scrollx&0x20) == 0 && (j+scrolly&0x20) == 0x20)
        tilenb = map_ptr[2048+(i+scrollx&0x1f)];
      else
      if ((i+scrollx&0x20) == 0x20 && (j+scrolly&0x20) == 0x20)
        tilenb = map_ptr[3072+(i+scrollx&0x1f)];
        
      p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14; tilenb &= 0x3ff;
      if (nb_tilex == 32) {
        ADD_TILE(bg, tilenb, p, f, bg_mode);               
        DRAW_TILE(i, j, tilenb, bg, p, f);      
      } else {
        ADD_TILE(bg, tilenb,    p, f, bg_mode);
        ADD_TILE(bg, tilenb+1,  p, f, bg_mode);
        ADD_TILE(bg, tilenb+16, p, f, bg_mode);
        ADD_TILE(bg, tilenb+17, p, f, bg_mode);
        DRAW_TILE(i*2,   y*2,   tilenb,    bg, p, f);
        DRAW_TILE(i*2+1, y*2,   tilenb+1,  bg, p, f);
        DRAW_TILE(i*2,   y*2+1, tilenb+16, bg, p, f);
        DRAW_TILE(i*2+1, y*2+1, tilenb+17, bg, p, f);
      }
      
      
    }
  }
}



#define SPRITE_ADD_X(INDEX) \
  -(((GFX.spr_info_ext[INDEX>>2]&(1<<((INDEX&0x3)<<1))) != 0)<<8)

#define SPRITE_POS_Y(INDEX) \
  (GFX.spr_info[INDEX].pos_y > 239 ? (char)GFX.spr_info[INDEX].pos_y : GFX.spr_info[INDEX].pos_y)


void draw_tile_sprite_normal(int TILENB, int X, int Y, int SIZEX)
{
    if (!sprite_tiles_def[GFX.spr_info[TILENB].fst_tile+(Y*16+X)])
      add_sprite_tile_4(GFX.spr_info[TILENB].fst_tile, (Y*16+X));
}

void draw_sprites(/*unsigned char pf*/)
{
  int	i, x, y;
  void	(*draw_tile_sprite)(int, int, int, int);
  int	spr_size;
  t_OAM_entry sprite;

//  memset(GFX.spr_cnt, 0, 240); 

  if (GFX.HighestSprite == 0)
    i = 127;
  else
    i = GFX.HighestSprite-1;
    
  ((uint32 *)&sprite)[0] = 0;
  ((uint32 *)&sprite)[1] = 0;
  while (1) 
  {
/*    if (GFX.spr_info[i].pf_priority == pf)
    {*/
    if ((GFX.spr_info[i].pos_y < 224 || GFX.spr_info[i].pos_y > 240) &&
        (GFX.spr_info[i].pos_x || GFX.spr_info[i].pos_y || GFX.spr_info[i].fst_tile
     || GFX.spr_info[i].palette || GFX.spr_info[i].flip)) {

	    sprite.tile_index = CONVERT_SPR_TILE(GFX.spr_info[i].fst_tile);
	    sprite.flip = GFX.spr_info[i].flip;
	    sprite.palette = GFX.spr_info[i].palette;
	    sprite.X = GFX.spr_info[i].pos_x+SPRITE_ADD_X(i);
	    sprite.Y = GFX.spr_info[i].pos_y-_offsetY_tab[CFG.YScroll];
	    sprite.pr = 3-GFX.spr_info[i].pf_priority;
	   	    
      spr_size = GFX.spr_info_ext[i>>2]&(1<<((i&0x3)<<1)+1);
	  draw_tile_sprite = draw_tile_sprite_normal;

      switch (SNES.PPU_Port[0x01]>>5)
      {
        case 0x00 : if (spr_size) {
                      sprite.size = 1;        	
                      for (y = 0; y < 2; y++)
                        for (x = 0; x < 2; x++)
                          draw_tile_sprite(i, x, y, 8);
                    } else {
                      sprite.size = 0;                    	
                      draw_tile_sprite(i, 0, 0, 0);
                    } break;
        case 0x01 : if (spr_size) {
                      sprite.size = 2;        	
                      for (y = 0; y < 4; y++)
                        for (x = 0; x < 4; x++)
                          draw_tile_sprite(i, x, y, 24);
                    } else {
                      sprite.size = 0;                    	
                      draw_tile_sprite(i, 0, 0, 0);
                    } break;
        case 0x02 : if (spr_size) {
                      sprite.size = 3;        	
                      for (y = 0; y < 8; y++)
                        for (x = 0; x < 8; x++)
                          draw_tile_sprite(i, x, y, 56);
                    } else {
                      sprite.size = 0;                    	
                      draw_tile_sprite(i, 0, 0, 0);
                    } break;
        case 0x03 : if (spr_size) {
                      sprite.size = 2;        	
                      for (y = 0; y < 4; y++)
                        for (x = 0; x < 4; x++)
                          draw_tile_sprite(i, x, y, 24);
                    } else {
                     sprite.size = 1;
                      for (y = 0; y < 2; y++)
                        for (x = 0; x < 2; x++)
                          draw_tile_sprite(i, x, y, 8);
                    } break;
        case 0x04 : if (spr_size) {
        	          sprite.size = 3;
                      for (y = 0; y < 8; y++)
                        for (x = 0; x < 8; x++)
                          draw_tile_sprite(i, x, y, 56);
                    } else {
                      sprite.size = 1;	
                      for (y = 0; y < 2; y++)
                        for (x = 0; x < 2; x++)
                          draw_tile_sprite(i, x, y, 8);
                    } break;
        case 0x05 : if (spr_size) {
        	          sprite.size = 3;
                      for (y = 0; y < 8; y++)
                        for (x = 0; x < 8; x++)
                          draw_tile_sprite(i, x, y, 56);
                    } else {
                      sprite.size = 2;
                      for (y = 0; y < 4; y++)
                        for (x = 0; x < 4; x++)
                          draw_tile_sprite(i, x, y, 24);
                    } break;
      }
	((uint32 *)OAM)[i*2] = ((uint32 *)&sprite)[0];
	((uint32 *)OAM)[i*2+1] = ((uint32 *)&sprite)[1];
      
    } else {
	((uint32 *)OAM)[i*2] = 0x0200;
	((uint32 *)OAM)[i*2+1] = 0;
     }
//    }    	
    i--;
    if (i == GFX.HighestSprite-1) return;
    if (i == -1) i = 127;
  }
}

#define DRAW_PLANE(BG, BG_MODE) \
  if (CFG.BG3Squish != 2 && BG == 2) \
        draw_plane_32_30_squiz(BG, BG_MODE); \
  else \
  switch(SNES.PPU_Port[0x07+BG]&3) { \
    case 0: { draw_plane_32_30(BG, BG_MODE); } break; \
    case 1: { draw_plane_64_30(BG, BG_MODE); } break; \
    case 2: { draw_plane_32_60(BG, BG_MODE); } break; \
    case 3: { draw_plane_64_60(BG, BG_MODE); } break; \
  }

void draw_screen2(NB_BG, MODE_1, MODE_2, MODE_3, MODE_4)

{
   sint8 	order[4] = { 4, 4, 4, 4 };
   uint32	SB;
   
#define push(x) \
{ if (x==0) order[0] = 0; else order[0]++; \
  if (x==1) order[1] = 0; else order[1]++; \
  if (x==2) order[2] = 0; else order[2]++; \
  if (x==3) order[3] = 0; else order[3]++; \
}   
  
  DISPLAY_CR &= 0xffff00ff;
  
  SB = SNES.PPU_Port[0x2D]&CFG.BG_Layer&((1<<NB_BG)-1);
  
  if (SB&0x08) push(3);
  if (SB&0x04) push(2);

  if (SB&0x02) push(1);
  if (SB&0x01) push(0);

  if ((SB&0x04) && (SNES.PPU_Port[0x05]&8)) push(2);

  SB = SNES.PPU_Port[0x2C]&CFG.BG_Layer&((1<<NB_BG)-1);

  if (SB&0x08) push(3);
  if (SB&0x04) push(2);

  if ((SB&0x04) && !(SNES.PPU_Port[0x05]&8)) push(2)

  if (SB&0x02) push(1);
  if (SB&0x01) push(0);

  if ((SB&0x04) && (SNES.PPU_Port[0x05]&8)) push(2);


//  iprintf("%x %x %x %x\n", order[0], order[1], order[2], order[3]);

   if (order[0] < 4)    
     BG0_CR = BG_16_COLOR | order[0] | BG_TILE_BASE(2) | BG_MAP_BASE(0) | ((SNES.PPU_Port[0x07+0]&3) << 14);
   if (order[1] < 4)    
     BG1_CR = BG_16_COLOR | order[1] | BG_TILE_BASE(4) | BG_MAP_BASE(4) | ((SNES.PPU_Port[0x07+1]&3) << 14);
   if (order[2] < 4)    
     BG2_CR = BG_16_COLOR | order[2] | BG_TILE_BASE(6) | BG_MAP_BASE(8) | ((SNES.PPU_Port[0x07+2]&3) << 14);
   if (order[3] < 4)    
     BG3_CR = BG_16_COLOR | order[3] | BG_TILE_BASE(8) | BG_MAP_BASE(12) | ((SNES.PPU_Port[0x07+3]&3) << 14);
  
  SB = (SNES.PPU_Port[0x2D]|SNES.PPU_Port[0x2C])&CFG.BG_Layer&((1<<NB_BG)-1);

  if (CFG.BG_Layer & 0x10) {
  	DISPLAY_CR |= DISPLAY_SPR_ACTIVE;
  }

  if ((SB&0x08)) {
  	DISPLAY_CR |= DISPLAY_BG3_ACTIVE;
    DRAW_PLANE(3, MODE_4);
  }
  if ((SB&0x04)) {
  	DISPLAY_CR |= DISPLAY_BG2_ACTIVE;
    DRAW_PLANE(2, MODE_3);
  }
  if ((SB&0x02)) {
  	DISPLAY_CR |= DISPLAY_BG1_ACTIVE;
    DRAW_PLANE(1, MODE_2);
  }
  if ((SB&0x01)) {
  	DISPLAY_CR |= DISPLAY_BG0_ACTIVE;
    DRAW_PLANE(0, MODE_1);
  }

  GFX.SUBSCREEN_addsub = 
      (SNES.PPU_Port[0x31]&0x1F) && (SNES.PPU_Port[0x30]&2) &&
      !(SNES.PPU_Port[0x30]&1) &&
      (!(SNES.PPU_Port[0x30]&0xF0 == 0x40) || SNES.PPU_Port[0x26] < SNES.PPU_Port[0x27]);
  GFX.FIXED_color_addsub =
      (SNES.PPU_Port[0x31]&0x1F) && !(SNES.PPU_Port[0x30]&2) &&
      !(SNES.PPU_Port[0x30]&1) && GFX.FIXED_notblack &&
      (!(SNES.PPU_Port[0x30]&0xF0 == 0x40) || SNES.PPU_Port[0x26] < SNES.PPU_Port[0x27]);
   if (GFX.SUBSCREEN_addsub && !SNES.PPU_Port[0x2D]) {
      GFX.FIXED_color_addsub = 1; GFX.SUBSCREEN_addsub = 0;
    }

  if (GFX.SUBSCREEN_addsub)
  {
  	LOG("SUBSCREEN_addsub\n");
  BLEND_CR = BLEND_ALPHA | BLEND_DST_BG0 | BLEND_SRC_BG1;
  BLEND_AB = 0x0F | (0x0F << 8);
  BLEND_Y  = 8 | 8<<8; // 50/50 Blend
  }
  else
  if (GFX.FIXED_color_addsub)
  {
  	LOG("FIXED_addsub\n");
  BLEND_CR = BLEND_ALPHA | BLEND_SRC_BG0 | BLEND_DST_BG1;
  BLEND_AB = 0x0F | (0x0F << 8);
  BLEND_Y  = 8 | 8<<8; // 50/50 Blend
  }  
  else
  {
  	BLEND_CR = BLEND_NONE;
  }

}

uint8 brightness;
uint8 old_brightness;

extern int frame;

static inline void dmaFillWords(const void* src, void* dest, uint32 size) {
	DMA_SRC(3)  = (uint32)src;
	DMA_DEST(3) = (uint32)dest;
	DMA_CR(3)   = DMA_COPY_WORDS | DMA_SRC_FIX | (size>>2);
	while(DMA_CR(3) & DMA_BUSY);
}

void PPU_reset()
{
  int i;
	
  DISPLAY_CR &= 0xffff00ff;

  for (i = 0; i < 128; i++)
  {  
	((uint32 *)OAM)[i*2] = 0x0200;
	((uint32 *)OAM)[i*2+1] = 0;
  }
  old_brightness = 0;
  
  // Clear DS VRAM
  i = 0;
#define DS_VRAM  ((uint16*)0x6800000)
  dmaFillWords(&i, DS_VRAM,  656*1024);
  
  frame = 0;
  memset(tiles_def, 0, 4*1024);
  memset(sprite_tiles_def, 0, 1024);
  	
}



inline void	PPU_setBackColor(int r, int g, int b)
{
    BG_PALETTE[0] = RGB15((r*brightness)/0xF, (g*brightness)/0xF, (b*brightness)/0xF) ;
}

void	update_scrolly(int bg)
{
  int delta;

  if (GFX.tiles_ry[bg] != 8 && SNES.PPU_Port[(0x0E)+bg*2] != GFX.old_scrolly[bg]) {
    delta = GFX.tiles_ry[bg] + SNES.PPU_Port[(0x0E)+bg*2]-GFX.old_scrolly[bg];
    if (delta >= 0 && delta < 8)
      GFX.tiles_ry[bg] = delta;
    else
      GFX.tiles_ry[bg] = 8;
  }
}

void	update_scrollx(int bg)
{
  int i, delta;

  if (GFX.tiles_ry[bg] != 8 && SNES.PPU_Port[(0x0D)+bg*2] != GFX.old_scrollx[bg]) {
    delta = SNES.PPU_Port[(0x0D)+bg*2]-GFX.old_scrollx[bg];

    if (delta < -7 || delta > 7)
      GFX.tiles_ry[bg] = 8;
    else {
      for (i = 0; i < GFX.tiles_cnt[bg*2]; i++)
        GFX.tiles_x[bg*2][i] -= delta;
      for (i = 0; i < GFX.tiles_cnt[bg*2+1]; i++)
        GFX.tiles_x[bg*2+1][i] -= delta;
    }
  }
}

//extern char *logbuf;
extern int v_blank;

void draw_screen()
{
  if (CFG.WaitVBlank || !GFX.need_update)
  {
	  if (SNES.PPU_Port[0x00]&0x80) {
	    if (!GFX.Blank_Screen)
	    {
	//      clear_to_color(screen, 0);
	    }
	    GFX.Blank_Screen = 1;
	    return;
	  } 
	  
	  if ((SNES.PPU_Port[0x00]&0x0f) == 0) {
	//      clear_to_color(screen, 0);
	      return;
	  }
  }

    GFX.Blank_Screen = 0;   
    if (CFG.WaitVBlank) 
    	swiWaitForVBlank();
#if 1    	
    else
    {
    	if (!v_blank)
    	{
    		GFX.need_update = 1;
    		return;
    	}
    }
#endif    

#ifdef TIMER_Y      
    SNES.stat_before = DISP_Y;
#else
	SNES.stat_before = TIMER3_DATA;
#endif
    
   // FIXME
   PPU_setBackColor(GFX.BACK.r/2, GFX.BACK.g/2, GFX.BACK.b/2);

    update_scroll();
    if (GFX.tiles_dirty)
	{
		LOG("Clear all tiles\n");
/*		memset(GFX.tiles_def[0], 0, 4096); 
		memset(GFX.tiles_def[1], 0, 2048);
		memset(GFX.tiles_def[2], 0, 1024);*/
		memset(tiles_def, 0, 4*1024);
		memset(sprite_tiles_def, 0, 1024);
		
		GFX.tiles_dirty = 0;
	}

    
    if (GFX.Sprites_table_dirty)
    {
  	  draw_sprites();
      GFX.Sprites_table_dirty = 0;  	
    }	
     	  
    switch (SNES.PPU_Port[0x05]&7) {
      case 0 : draw_screen2(4, 2, 2, 2, 2); break;
      case 1 : draw_screen2(3, 4, 4, 2, 0); break;
      case 2 : draw_screen2(2, 4, 4, 0, 0); break;
      case 3 : draw_screen2(2, 8, 4, 0, 0); break;
      case 4 : draw_screen2(2, 8, 2, 0, 0); break;
      case 5 : draw_screen2(3, 4, 4, 2, 0); break;
      case 6 : draw_screen2(3, 4, 4, 2, 0); break;
      case 7 : draw_screen2(3, 4, 4, 2, 0); break;
    }

#ifdef TIMER_Y	
	if (SNES.stat_before > DISP_Y)
	   SNES.stat_GFX += 262+DISP_Y-SNES.stat_before;
	else
	   SNES.stat_GFX += DISP_Y-SNES.stat_before;
#else
	if (SNES.stat_before > TIMER3_DATA)
		SNES.stat_GFX += 65536+TIMER3_DATA-SNES.stat_before;
	else
		SNES.stat_GFX += TIMER3_DATA-SNES.stat_before;
#endif
  
#ifdef TIMER_Y  
    LOG("CPU=%d GFX=%d\n", SNES.stat_CPU, SNES.stat_GFX, frame);
#else
	LOG("CPU=%d GFX=%d IOR=%d DMA=%d\n", SNES.stat_CPU, SNES.stat_GFX, SNES.stat_IOREGS, SNES.stat_DMA);
#endif

#if 1
{
/*	int max = 0;
	    
	    logbuf[0] = 0;
      for (i = 0; i < 256; i++)
      {
      	sprintf(logbuf+strlen(logbuf), "%d (%02x)\n", SNES.stat_OPC[i]/SNES.stat_OPC_cnt[i], i);
      	if (SNES.stat_OPC[i] > SNES.stat_OPC[max])
      		max = i;
      }*/      	
//      LOG("CPU=%d IOR=%d OPC=%d(%x)\n", SNES.stat_CPU/33, SNES.stat_IOREGS/33, SNES.stat_OPC[max]/33, max);
/*      for (i = 0; i < 256; i++)
      {
      	SNES.stat_OPC[i] = 0;
      }*/
	      
}
#endif
    SNES.stat_CPU = 0;  
    SNES.stat_GFX = 0;
    SNES.stat_IOREGS = 0;
    SNES.stat_DMA = 0;

#if 0
  }
#endif 
}



inline void	PPU_setPalette(int c, int r, int g, int b)
{
//	iprintf("%d %d %d %d\n", c, r, g, b);
	if (c < 128) // TILE color
	{
		if (c > 0) // FIXME
		BG_PALETTE[c] = RGB15((r*brightness)/0xF, (g*brightness)/0xF, (b*brightness)/0xF) ;
		// Recopie pour les palettes 2 bits
		if (c < 32)
		  BG_PALETTE[128+((c>>2)<<4)+(c&3)] = RGB15((r*brightness)/0xF, (g*brightness)/0xF, (b*brightness)/0xF) ;
	}
	else // SPRITE color
		SPRITE_PALETTE[c-128] = RGB15((r*brightness)/0xF, (g*brightness)/0xF, (b*brightness)/0xF) ;
}

void	PPU_setScreen(int value)
{
	int i;
 /*    if ((value&0x80) == 0)
     	screen_mode &= ~DISPLAY_SCREEN_OFF;
     else
     	screen_mode |= DISPLAY_SCREEN_OFF;  	
     videoSetMode(screen_mode);*/
     //BRIGHTNESS = -31+((value & 0xF)*2);
    
     
     brightness = (value & 0xF);
     if (brightness == old_brightness)
       return;
     old_brightness = brightness;
     LOG("SCREEN : %x\n", value);
          
     for (i = 0; i < 256; i++)
     {
       PPU_setPalette(i, GFX.SNESPal[i].r/2, GFX.SNESPal[i].g/2, GFX.SNESPal[i].b/2);
	 }
}

void PPU_update()
{
	int i;
	
	PPU_reset();
	for (i = 0; i < 256; i++)
	{
	  PPU_setPalette(i, GFX.SNESPal[i].r/2, GFX.SNESPal[i].g/2, GFX.SNESPal[i].b/2);
	}
}