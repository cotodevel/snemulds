#include <nds.h>
#include <nds/memory.h>
#include <string.h>

#include "common.h"
#include "gfx.h"
#include "snes.h"
#include "cfg.h"
#include "apu.h"
#include "opcodes.h"
#include "gui_draw/gui.h"


uint32	joypad_conf_mode = 0;
uint32	mouse_cur_b;

int	setBacklight(int flags)
{
	SendArm7Command(8 | (flags << 16));
	return 0;
}

int myLCDSwap()
{
	lcdSwap();
	if (GUI.hide)
	{
		if (REG_POWERCNT & POWER_SWAP_LCDS)
			setBacklight(PM_BACKLIGHT_TOP);
		else
			setBacklight(PM_BACKLIGHT_BOTTOM);
	}
	return 0;
}

int get_joypad()
{
	int res = 0;

/*#define KEYS_CUR (( ((~REG_KEYINPUT)&0x3ff) | (((~IPC->buttons)&3)<<10) | \
	 			 (((~IPC->buttons)<<6) & (KEY_TOUCH|KEY_LID) ))^KEY_LID)	
	keys = KEYS_CUR;*/
	
	//scanKeys();	
	keys = keysCurrent();

#if 0
		if( (keys & KEY_L))
		{
			if ((keys & KEY_UP))
			{
				APU_MAX++;
			}
			if ((keys & KEY_DOWN))
			{
				APU_MAX--;
				if (APU_MAX < 100)
					APU_MAX = 100;
			}  
			if ((keys & KEY_LEFT))
			{
				APU_printLog();				
			}
			if ((keys & KEY_RIGHT))
			{
				LOG("%04x %04x %02x %02x %04x %04x\n", CPU.PC,
				(uint32)((sint32)PCptr+(sint32)SnesPCOffset),
				PORT_SNES_TO_SPC[1], PORT_SPC_TO_SNES[1],  
				
				 (*(uint32*)memUncached(0x2FE0000)) & 0xFFFF, *(uint16 *)(APU_RAM_ADDRESS+0x18));
				
				//PORT_SNES_TO_SPC[1] = 0x44; 		
			}
			
		}	
#endif	
	
	if ((keys & KEY_L) && ( keys & KEY_R ) && ( keys & KEY_START))
	{		
		if (keys & KEY_LEFT)
		{
			if (joypad_conf_mode)
				return 0;			
			CFG.mouse ^= 1;
/*			lcdSwap();
			if (GUI.hide)
				setBacklight(CFG.mouse ? PM_BACKLIGHT_BOTTOM : PM_BACKLIGHT_TOP);*/
			myLCDSwap();
			joypad_conf_mode = 1;
			return 0;			
		}
		if (keys & KEY_RIGHT)
		{
			if (joypad_conf_mode)
				return 0;			
			CFG.mouse = 0;
/*			lcdSwap();
			if (GUI.hide)
				setBacklight(CFG.mouse ? PM_BACKLIGHT_BOTTOM : PM_BACKLIGHT_TOP);*/
			myLCDSwap();			
			joypad_conf_mode = 1;
			return 0;				
		}		
		if (keys & KEY_UP)
		{
			if (joypad_conf_mode)
				return 0;
			PPU_ChangeLayerConf(CFG.LayersConf+1);
			joypad_conf_mode = 1;
			return 0;
		}			
		if (keys & KEY_DOWN)
		{
			if (joypad_conf_mode)
				return 0;	
			PPU_ChangeLayerConf(0);
			joypad_conf_mode = 1;
			return 0;
		}			
		joypad_conf_mode = 0;		
		return 0;
	} 
	 
#if 0	 
	if ((keys & KEY_L) && ( keys & KEY_R ) && ( keys & KEY_SELECT))
	{		
		if (keys & KEY_LEFT)
		{
			if (joypad_conf_mode)
				return 0;			
			CFG.BG_priority ^= 1;
			LOG("BG_pri = %d\n", CFG.BG_priority);
			joypad_conf_mode = 1;
			return 0;			
		}
		if (keys & KEY_RIGHT)
		{
			if (joypad_conf_mode)
				return 0;
			CFG.Debug ^= 1;
			if (CFG.Debug)
				showDebugMenu();
			joypad_conf_mode = 1;
			return 0;			
		}		
		if (keys & KEY_UP)
		{
			if (joypad_conf_mode)
				return 0;			
			CFG.Debug2 --;
			LOG("Debug = %d\n", CFG.Debug);
			return 0;			
		}				
		if (keys & KEY_DOWN)
		{
			if (joypad_conf_mode)
				return 0;			
			CFG.Debug2 ++;
			LOG("Debug = %d\n", CFG.Debug);
			return 0;			
		}		
		joypad_conf_mode = 0;		
		return 0;
	}
#endif	 	 
	 
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
	
	if (CFG.mouse)
	{
		if((keys & KEY_LEFT) || (keys & KEY_Y)) mouse_cur_b = 1;
		if((keys & KEY_RIGHT) || (keys & KEY_A)) mouse_cur_b = 2;
		if( ( !(keys & KEY_L) && (keys & KEY_DOWN) ) ||	
		    ( !(keys & KEY_R) && (keys & KEY_B) )) mouse_cur_b = 0;
		    
		if (keys & KEY_SELECT)
			PPU_reset();
		
		if( (keys & KEY_L) || (keys & KEY_R) )
		{
			if (((keys & KEY_UP) || (keys & KEY_X)) && GFX.YScroll > 0)
			{
				GFX.YScroll--;
				GFX.BG3YScroll = GFX.YScroll;
			}
			if (((keys & KEY_DOWN) || (keys & KEY_B)) && GFX.YScroll < 32)
			{
				GFX.YScroll++;
				GFX.BG3YScroll = GFX.YScroll;
			}  
		}

		touchPosition touchXY;
		touchRead(&touchXY);
		if (keys & KEY_TOUCH)
		{		
			int tx=0, ty=0;

			tx = touchXY.px;
			if (CFG.Scaled == 0) // No scaling
				ty = touchXY.py+GFX.YScroll;
			else if (CFG.Scaled == 1) // Half scaling
				ty = touchXY.py*208/192+12; // FIXME			
			else if (CFG.Scaled == 2) // Full screen
				ty = touchXY.py*224/192;
			
			if (CFG.MouseMode == 0)
			{
				SNES.mouse_x = tx; 
				SNES.mouse_x = ty;
			}
			else
			if (CFG.MouseMode == 1)
			{
				SNESC.RAM[CFG.MouseXAddr] = tx+CFG.MouseXOffset;
        		SNESC.RAM[CFG.MouseYAddr] = ty+CFG.MouseYOffset;				
			}
			if (CFG.MouseMode == 2)
			{		
				*(uint16 *)(&SNESC.RAM[CFG.MouseXAddr]) = tx*2;
        		*(uint16 *)(&SNESC.RAM[CFG.MouseYAddr]) = ty*2;
			}			
			if( (!(keys & KEY_L) &&  (keys & KEY_UP)) ||
				(!(keys & KEY_R) && (keys & KEY_X)) )
			{
				SNES.prev_mouse_x = SNES.mouse_x; 
				SNES.prev_mouse_y = SNES.mouse_y;
				SNES.mouse_b =  0;
			}
			else
				SNES.mouse_b =  mouse_cur_b;
		
		}
		else
		SNES.mouse_b =  0;
	}
		
	return res;
}
