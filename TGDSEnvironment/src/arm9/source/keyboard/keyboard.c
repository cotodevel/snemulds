/*---------------------------------------------------------------------------------

	Extended Keyboard Example 5
	Author: Headkaze

---------------------------------------------------------------------------------*/

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "videoTGDS.h"
#include "consoleTGDS.h"
#include "dmaTGDS.h"
#include "keypadTGDS.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "keyboard_raw_lc.h"
#include "keyboard_raw_uc.h"
#include "keyboard_map_lc.h"
#include "keyboard_map_uc.h"
#include "keyboard_pal.h"
#include "keyboard_pal_hl.h"

#include "clickdown.h"
#include "clickup.h"
#include "keyboard.h"

const unsigned char keyboard_Hit[12][32] = {
	{ 0x0,ESC,ESC,0x0,F_1,F_1,F_2,F_2,F_3,F_3,F_4,F_4,F_5,F_5,F_6,F_6,F_7,F_7,F_8,F_8,F_9,F_9,F10,F10,F11,F11,F12,F12,0x0,EXT,EXT,0x0 },
	{ 0x0,ESC,ESC,0x0,F_1,F_1,F_2,F_2,F_3,F_3,F_4,F_4,F_5,F_5,F_6,F_6,F_7,F_7,F_8,F_8,F_9,F_9,F10,F10,F11,F11,F12,F12,0x0,EXT,EXT,0x0 },
	{ 0x0,'1','1','2','2','3','3','4','4','5','5','6','6','7','7','8','8','9','9','0','0','-','-','=','=',BSP,BSP,BSP,BSP,HOM,HOM,0x0 },
	{ 0x0,'1','1','2','2','3','3','4','4','5','5','6','6','7','7','8','8','9','9','0','0','-','-','=','=',BSP,BSP,BSP,BSP,HOM,HOM,0x0 },
	{ 0x0,TAB,'q','q','w','w','e','e','r','r','t','t','y','y','u','u','i','i','o','o','p','p','[','[',']',']','\\','\\',0x0,PGU,PGU,0x0 },
	{ 0x0,TAB,'q','q','w','w','e','e','r','r','t','t','y','y','u','u','i','i','o','o','p','p','[','[',']',']','\\','\\',0x0,PGU,PGU,0x0 },
	{ 0x0,CAP,CAP,'a','a','s','s','d','d','f','f','g','g','h','h','j','j','k','k','l','l',';',';','\'','\'',RET,RET,RET,RET,PGD,PGD,0x0 },
	{ 0x0,CAP,CAP,'a','a','s','s','d','d','f','f','g','g','h','h','j','j','k','k','l','l',';',';','\'','\'',RET,RET,RET,RET,PGD,PGD,0x0 },
	{ 0x0,SHF,SHF,SHF,'z','z','x','x','c','c','v','v','b','b','n','n','m','m',',',',','.','.','/','/',SHF,SHF,SHF,CRU,CRU,END,END,0x0 },
	{ 0x0,SHF,SHF,SHF,'z','z','x','x','c','c','v','v','b','b','n','n','m','m',',',',','.','.','/','/',SHF,SHF,SHF,CRU,CRU,END,END,0x0 },
	{ 0x0,CTL,CTL,NDSKEY,NDSKEY,ALT,ALT,'`','`',SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,INS,INS,DEL,DEL,SCN,SCN,CRL,CRL,CRD,CRD,CRR,CRR,0x0 },
	{ 0x0,CTL,CTL,NDSKEY,NDSKEY,ALT,ALT,'`','`',SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,INS,INS,DEL,DEL,SCN,SCN,CRL,CRL,CRD,CRD,CRR,CRR,0x0 }
};

const unsigned char keyboard_Hit_Shift[12][32] = {
	{ 0x0,ESC,ESC,0x0,F_1,F_1,F_2,F_2,F_3,F_3,F_4,F_4,F_5,F_5,F_6,F_6,F_7,F_7,F_8,F_8,F_9,F_9,F10,F10,F11,F11,F12,F12,0x0,EXT,EXT,0x0 },
	{ 0x0,ESC,ESC,0x0,F_1,F_1,F_2,F_2,F_3,F_3,F_4,F_4,F_5,F_5,F_6,F_6,F_7,F_7,F_8,F_8,F_9,F_9,F10,F10,F11,F11,F12,F12,0x0,EXT,EXT,0x0 },
	{ 0x0,'!','!','@','@','#','#','$','$','%','%','^','^','&','&','*','*','(','(',')',')','_','_','+','+',BSP,BSP,BSP,BSP,HOM,HOM,0x0 },
	{ 0x0,'!','!','@','@','#','#','$','$','%','%','^','^','&','&','*','*','(','(',')',')','_','_','+','+',BSP,BSP,BSP,BSP,HOM,HOM,0x0 },
	{ 0x0,TAB,'Q','Q','W','W','E','E','R','R','T','T','Y','Y','U','U','I','I','O','O','P','P','{','{','}','}','|','|',0x0,PGU,PGU,0x0 },
	{ 0x0,TAB,'Q','Q','W','W','E','E','R','R','T','T','Y','Y','U','U','I','I','O','O','P','P','{','{','}','}','|','|',0x0,PGU,PGU,0x0 },
	{ 0x0,CAP,CAP,'A','A','S','S','D','D','F','F','G','G','H','H','J','J','K','K','L','L',':',':','"','"',RET,RET,RET,RET,PGD,PGD,0x0 },
	{ 0x0,CAP,CAP,'A','A','S','S','D','D','F','F','G','G','H','H','J','J','K','K','L','L',':',':','"','"',RET,RET,RET,RET,PGD,PGD,0x0 },
	{ 0x0,SHF,SHF,SHF,'Z','Z','X','X','C','C','V','V','B','B','N','N','M','M','<','<','>','>','?','?',SHF,SHF,SHF,CRU,CRU,END,END,0x0 },
	{ 0x0,SHF,SHF,SHF,'Z','Z','X','X','C','C','V','V','B','B','N','N','M','M','<','<','>','>','?','?',SHF,SHF,SHF,CRU,CRU,END,END,0x0 },
	{ 0x0,CTL,CTL,NDSKEY,NDSKEY,ALT,ALT,'~','~',SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,INS,INS,DEL,DEL,SCN,SCN,CRL,CRL,CRD,CRD,CRR,CRR,0x0 },
	{ 0x0,CTL,CTL,NDSKEY,NDSKEY,ALT,ALT,'~','~',SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,SPC,INS,INS,DEL,DEL,SCN,SCN,CRL,CRL,CRD,CRD,CRR,CRR,0x0 }
};



static int g_dx=0;
static int g_dy=0;
static u16 g_MouseDown = false;
static int g_Mode = KB_NORMAL;
static int g_col = 0;

static unsigned int lasttilex=0, lasttiley=0;
static char lastkey = 0x0;

void setTile(uint16 *map, int x, int y, int pal)
{
	char c;
	int x2, y2;

	c = keyboard_Hit[y-TILE_OFFSET_Y][x];

	if(!c) return;

	map[(y*32)+x] &= ~(1 << 12);
	map[(y*32)+x] |= (pal << 12);

	x2 = x; y2 = y;
	while(keyboard_Hit[y2-TILE_OFFSET_Y][x2]==c)
	{
		map[(y2*32)+x2] &= ~(1 << 12);
		map[(y2*32)+x2] |= (pal << 12);

		x2 = x;
		while(keyboard_Hit[y2-TILE_OFFSET_Y][x2]==c) { map[(y2*32)+x2] &= ~(1 << 12); map[(y2*32)+x2] |= (pal << 12); x2++; }
		x2 = x;
		while(keyboard_Hit[y2-TILE_OFFSET_Y][x2]==c) { map[(y2*32)+x2] &= ~(1 << 12); map[(y2*32)+x2] |= (pal << 12); x2--; }

		x2 = x;
		y2++;
	}

	x2 = x; y2 = y;
	while(keyboard_Hit[y2-TILE_OFFSET_Y][x2]==c)
	{
		map[(y2*32)+x2] &= ~(1 << 12);
		map[(y2*32)+x2] |= (pal << 12);

		x2 = x;
		while(keyboard_Hit[y2-TILE_OFFSET_Y][x2]==c) { map[(y2*32)+x2] &= ~(1 << 12); map[(y2*32)+x2] |= (pal << 12); x2++; }
		x2 = x;
		while(keyboard_Hit[y2-TILE_OFFSET_Y][x2]==c) { map[(y2*32)+x2] &= ~(1 << 12); map[(y2*32)+x2] |= (pal << 12); x2--; }

		x2 = x;
		y2--;
	}
}

#define CHAR_SHIFT        2
#define SCREEN_SHIFT      8

void initKeyboard()
{
	SETDISPCNT_SUB(MODE_0_2D | DISPLAY_BG0_ACTIVE);

	BG0CNT2 = BG_COLOR_16 | BG_32x32 | (29 << SCREEN_SHIFT) | (1 << CHAR_SHIFT);
	
	dmaTransferWord(3, (uint32)&keyboard_pal, (uint32) BG_PALETTE_SUB, (uint32)keyboard_pal_size); //dmaCopy((uint16 *) &keyboard_pal, (uint16 *) BG_PALETTE_SUB, keyboard_pal_size);
	dmaTransferWord(3, (uint32)&keyboard_pal_hl, (uint32)&BG_PALETTE_SUB[16], (uint32)keyboard_pal_hl_size); //dmaCopy((uint16 *) &keyboard_pal_hl, (uint16 *) &BG_PALETTE_SUB[16], keyboard_pal_hl_size);
	dmaTransferWord(3, (uint32)&keyboard_map_lc, (uint32)SCREEN_BASE_BLOCK_SUB(29), (uint32)keyboard_map_lc_size); //dmaCopy((uint16 *) &keyboard_map_lc, (uint16 *) SCREEN_BASE_BLOCK_SUB(29), keyboard_map_lc_size);
	dmaTransferWord(3, (uint32)&keyboard_raw_lc, (uint32)CHAR_BASE_BLOCK_SUB(1), (uint32)keyboard_raw_lc_size); //dmaCopy((uint16 *) &keyboard_raw_lc, (uint16 *) CHAR_BASE_BLOCK_SUB(1), keyboard_raw_lc_size);
	
	g_dx=0;
	g_dy=0;
	g_MouseDown = false;
	g_Mode = KB_NORMAL;
	g_col = 0;

	lasttilex=0, lasttiley=0;
	lastkey = 0x0;
}

// returns - last key pressed
// str - pointer to string to store output
// max - maximum number of characters
// echo - echo input to console
// strYoffset -  only ysed when echo == ECHO_ON
char processKeyboard(char* str, unsigned int max, unsigned int echo, int strYoffset)
{
	scanKeys();
	
	int keysHld = keysPressed();
	//get the map
	uint16 *map = (uint16 *) SCREEN_BASE_BLOCK_SUB(29); 

    if(PEN_DOWN && !g_MouseDown)
	{
		g_dx = TGDSIPC->touchXpx;	//touchXY.px;
		g_dy = TGDSIPC->touchYpx;	//touchXY.py;
		g_MouseDown = true;
    } 
	else if(PEN_DOWN && g_MouseDown)
	{
		int i, j;
		
		i = TGDSIPC->touchXpx;	//touchXY.px;
		j = TGDSIPC->touchYpx;	//touchXY.py;

#ifdef USE_CUSTOM_PEN_JUMP_DETECTION_CODE
		int z1, z2;
		
		z1 = TGDSIPC->touchZ1;	//IPC->touchZ1;
		z2 = TGDSIPC->touchZ2;	//IPC->touchZ2;
	
		// This is checking z registers are not zero for pen jumping
		if(z1!=0 && z2!=0)
		{
#endif 
			g_dx = i;
			g_dy = j;

			unsigned int tilex, tiley;

			tilex = g_dx/8;
			tiley = g_dy/8;

			if(tilex>=1 && tilex<31 && tiley>=11 && tiley<23)
			{
				char c;

				if(g_Mode==KB_NORMAL)
					c = keyboard_Hit[tiley-TILE_OFFSET_Y][tilex];
				else
					c = keyboard_Hit_Shift[tiley-TILE_OFFSET_Y][tilex];


				//ori removed
				//if(lastkey != c)				
				//	playGenericSound(&clickdown, clickdown_size);

				setTile(map, lasttilex, lasttiley, 0);
				setTile(map, tilex, tiley, 1);
				lastkey = c; lasttilex = tilex; lasttiley = tiley;
			}
#ifdef USE_CUSTOM_PEN_JUMP_DETECTION_CODE
		} 
		else g_MouseDown = false;
#endif 
    }
	else if((!PEN_DOWN && g_MouseDown) || ((!PEN_DOWN && !g_MouseDown) && lastkey != 0)) {
		g_MouseDown = false;
		char c, buf[2];

		unsigned int tilex, tiley;

		tilex = g_dx/8;
		tiley = g_dy/8;

		if(tilex>=1 && tilex<31 && tiley>=11 && tiley<23)
		{
			if(g_Mode==KB_NORMAL)
				c = keyboard_Hit[tiley-TILE_OFFSET_Y][tilex];
			else
				c = keyboard_Hit_Shift[tiley-TILE_OFFSET_Y][tilex];

			//playGenericSound(&clickup, clickup_size);	//todo?? i dunno
			setTile(map, lasttilex, lasttiley, 0);

			lastkey = 0; lasttilex = 0; lasttiley = 0;

			buf[0] = c;
			buf[1] = (char)NULL;

			if(c==RET) // Return
			{
				if(echo==ECHO_ON)
				{
					//printf("\n");
					g_col = 0;
				}
			} 
			else if(c==BSP) // Backspace
			{
				if(strlen(str)>0)
				{
					if(echo==ECHO_ON)
					{
						if(g_col == 0)
						{
							g_col = 31;
							//printf("\x1b[1A\x1b[31C \x1b[1D");	//todo
						}
						else { 
							g_col--; 
							//printf("\x1b[1D \x1b[1D"); 			//todo
						}
					}
					str[strlen(str)-1] = (char) NULL;
				}
				
			} 
			else if(c==CAP) // Caps
			{
				lasttilex = 0; lasttiley = 0;
				if(g_Mode==KB_NORMAL) {
					dmaTransferWord(3, (uint32)&keyboard_map_uc, (uint32)SCREEN_BASE_BLOCK_SUB(29), (uint32)keyboard_map_uc_size); //dmaCopy((uint16 *) &keyboard_map_uc, (uint16 *) SCREEN_BASE_BLOCK_SUB(29), keyboard_map_uc_size);
					dmaTransferWord(3, (uint32)&keyboard_raw_uc, (uint32)CHAR_BASE_BLOCK_SUB(1), (uint32)keyboard_raw_uc_size); //dmaCopy((uint16 *) &keyboard_raw_uc, (uint16 *) CHAR_BASE_BLOCK_SUB(1), keyboard_raw_uc_size);

					map[(17*32)+1] |= (1 << 12);
					map[(17*32)+2] |= (1 << 12);
					map[(18*32)+1] |= (1 << 12);
					map[(18*32)+2] |= (1 << 12);

					g_Mode = KB_CAPS;
				}
				else {
					dmaTransferWord(3, (uint32)&keyboard_map_lc, (uint32)SCREEN_BASE_BLOCK_SUB(29), (uint32)keyboard_map_lc_size); //dmaCopy((uint16 *) &keyboard_map_lc, (uint16 *) SCREEN_BASE_BLOCK_SUB(29), keyboard_map_lc_size);
					dmaTransferWord(3, (uint32)&keyboard_raw_lc, (uint32)CHAR_BASE_BLOCK_SUB(1), (uint32)keyboard_raw_lc_size); //dmaCopy((uint16 *) &keyboard_raw_lc, (uint16 *) CHAR_BASE_BLOCK_SUB(1), keyboard_raw_lc_size);
					g_Mode = KB_NORMAL;
				}
			} 
			else if(c==SHF) // Shift
			{
				lasttilex = 0; lasttiley = 0;
				if(g_Mode==KB_NORMAL) {
					dmaTransferWord(3, (uint32)&keyboard_map_uc, (uint32)SCREEN_BASE_BLOCK_SUB(29), (uint32)keyboard_map_uc_size); //dmaCopy((uint16 *) &keyboard_map_uc, (uint16 *) SCREEN_BASE_BLOCK_SUB(29), keyboard_map_uc_size);
					dmaTransferWord(3, (uint32)&keyboard_raw_uc, (uint32)CHAR_BASE_BLOCK_SUB(1), (uint32)keyboard_raw_uc_size); //dmaCopy((uint16 *) &keyboard_raw_uc, (uint16 *) CHAR_BASE_BLOCK_SUB(1), keyboard_raw_uc_size);

					map[(19*32)+1] |= (1 << 12);
					map[(19*32)+2] |= (1 << 12);
					map[(19*32)+3] |= (1 << 12);
					map[(20*32)+1] |= (1 << 12);
					map[(20*32)+2] |= (1 << 12);
					map[(20*32)+3] |= (1 << 12);

					map[(19*32)+24] |= (1 << 12);
					map[(19*32)+25] |= (1 << 12);
					map[(19*32)+26] |= (1 << 12);
					map[(20*32)+24] |= (1 << 12);
					map[(20*32)+25] |= (1 << 12);
					map[(20*32)+26] |= (1 << 12);
					g_Mode = KB_SHIFT;
				}
				else {
					dmaTransferWord(3, (uint32)&keyboard_map_lc, (uint32)SCREEN_BASE_BLOCK_SUB(29), (uint32)keyboard_map_lc_size); //dmaCopy((uint16 *) &keyboard_map_lc, (uint16 *) SCREEN_BASE_BLOCK_SUB(29), keyboard_map_lc_size);
					dmaTransferWord(3, (uint32)&keyboard_raw_lc, (uint32)CHAR_BASE_BLOCK_SUB(1), (uint32)keyboard_raw_lc_size); //dmaCopy((uint16 *) &keyboard_raw_lc, (uint16 *) CHAR_BASE_BLOCK_SUB(1), keyboard_raw_lc_size);
					g_Mode = KB_NORMAL;
				}
			} 
			else {
				if(strlen(str)<max-1 && (c>=32 && c<=126)) {
					strcat(str, buf);
					if(echo==ECHO_ON)
					{
						if(strYoffset == 0){
							clrscr();
						}
						printfCoords(strYoffset, 5, "%c",c);
						
						g_col++;
						if(g_col == 33) g_col = 1;
					}
				}
				if(g_Mode == KB_SHIFT) // Undo Shift
				{
					dmaTransferWord(3, (uint32)&keyboard_map_lc, (uint32)SCREEN_BASE_BLOCK_SUB(29), (uint32)keyboard_map_lc_size); //dmaCopy((uint16 *) &keyboard_map_lc, (uint16 *) SCREEN_BASE_BLOCK_SUB(29), keyboard_map_lc_size);
					dmaTransferWord(3, (uint32)&keyboard_raw_lc, (uint32)CHAR_BASE_BLOCK_SUB(1), (uint32)keyboard_raw_lc_size); //dmaCopy((uint16 *) &keyboard_raw_lc, (uint16 *) CHAR_BASE_BLOCK_SUB(1), keyboard_raw_lc_size);
					g_Mode = KB_NORMAL;
				}
			}
			return c;
		} 
		else
		{
			setTile(map, lasttilex, lasttiley, 0);
			
			lastkey = 0; lasttilex = 0; lasttiley = 0;
		}
	}
	return 0;
}
