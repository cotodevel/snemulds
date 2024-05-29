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

#ifndef __gfx_h__
#define __gfx_h__

#include "common.h"

typedef
       struct {
              unsigned char   *ptr;
              unsigned char   pos_x;
              unsigned char   pf:2;
              unsigned char   first_x:3;
              unsigned char   trans:1;
       } line_sprite_Info;

typedef
       struct {
		   unsigned  pos_x:8;
		   unsigned  pos_y:8;
	      unsigned   fst_tile:9;
	      unsigned   palette:3;
	      unsigned   pf_priority:2;
	      unsigned   flip:2;
       } sprite_Info;

typedef
       struct {
         unsigned	b:5;
         unsigned   g:5;
         unsigned	r:5;
         unsigned	t:1;
       } RealColor;
/*typedef
       struct {
         uint8		b;
         uint8		g;
         uint8		r;
         uint8		t;
       } RealColor;*/       

typedef struct {
	int		mode;
	uint16	lBG0_CR, lBG1_CR, lBG2_CR, lBG3_CR;
	uint32	lDISPLAY_CR;
	uint16	lBG0_X0, lBG0_Y0;
	uint16	lBG1_X0, lBG1_Y0;
	uint16	lBG2_X0, lBG2_Y0;
	uint16	lBG3_X0, lBG3_Y0;
	uint16	lBACK_color;
//	uint16	lBRIGHTNESS;
	uint32	lBLEND;
	sint16	A, B, C, D;
	int		CX, CY;	
} t_GFX_lineInfo;

struct s_gfx
{
  int		ScreenWidth, ScreenHeight;

  sprite_Info	spr_info[128];
  uchar		spr_info_ext[32]; /* additional information */
  int		Sprites_table_dirty;
  //uchar		spr_cnt[240];
  
  int		DSFrame;
  int		DSLastFrame;
  int		v_blank;

  int		spr_addr_select;
  int		spr_addr_base;
  
  int		spr_addr[2];
  int		spr_addr_vcount[2];
  uint32	spr_bank;

  int		tiles_dirty;

  int		BG_scroll_reg;
  int		Graph_enabled;
  int		Blank_Screen;
  int		was_not_blanked;

/*  short     tiles_x[8][33];
  uchar		tiles_ry[4];
  uchar		tiles_cnt[8];*/
  char		HighestSprite;
  int		old_scrollx[4], old_scrolly[4];
  int		tile_address[4];
  
  int		tile_slot[4];
  int		map_slot[4];
  
  int		map_slot_ds[4];
  int		map_size[4];
  

  int		SC_incr;
  int		FS_incr;
  int		FS_cnt, FS_cnt2;

  int		OAM_upper_byte;
  int		Old_SpriteAddress;
  int		old_brightness;
  int		Dummy_VRAMRead;

  uchar		new_colors[256], new_color;
  uchar		need_update;
  int		pal;
  int		FIXED_notblack;
  uint16	CG_RAM_mem_temp;
/*  RealColor SNESPal[256];
  RealColor	FIXED, BACK;*/
  uint16	SNESPal[256];
  uint16	FIXED, BACK;
  int		brightness;

  int		frame_d;
  int		nb_frames;
  int		real_nb_frames;
  
  int		speed;

  uint16	WIN_X1, WIN_X2, CWIN_X1, CWIN_X2;
  int           SubScreen, FIXED_color_addsub, SUBSCREEN_addsub;

  uint16	*DSMapAddress;
  uint32	VRAM_ptr; // Start of BG Tiles mem, used by convert function
  
  t_GFX_lineInfo	lineInfo[192];	

  int		YScroll; 
  int		BG3YScroll;

  uint8		tiles2b_def[4096]; // 4096 * 16o
  uint8		tiles4b_def[2048]; // 2048 * 32o
  uint8		tiles8b_def[1024]; // 1024 * 64o
  uint8		map_def[32+4]; // 32 * 2ko
  uint8		tiles_def[8+4]; // 8 * 8ko

  // Count the number of high and low priority for a line of block (for BG3)
  sint8		BG3TilePriority[64];
};

extern struct s_gfx	GFX;

void	init_GFX();
void	PPU_setScreen(int screen);
void	check_sprite_addr();
void	update_scrollx();
void	update_scrolly();
void	check_tile();
void    check_tile_addr();
void	PPU_setPalette(int c, uint16 rgb);
int		get_joypad();
void	update_joypads();
void	PPU_reset();
void	draw_screen();

void	PPU_add_tile_address(int bg);



#endif
