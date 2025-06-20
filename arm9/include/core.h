/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

#ifndef __core_h__
#define __core_h__

#include "dsregs.h"
#include "typedefsTGDS.h"
#include "opcodes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define bzero(p, s)	memset(p, 0, s)

//snes irqs
#define IRQ_PENDING_FLAG    (1 << 11)

#define PPU_H_BEAM_IRQ_SOURCE	(1 << 0)
#define PPU_V_BEAM_IRQ_SOURCE	(1 << 1)
#define GSU_IRQ_SOURCE		(1 << 2)
#define SA1_IRQ_SOURCE		(1 << 7)
#define SA1_DMA_IRQ_SOURCE	(1 << 5)

#define SNES_IRQ_SOURCE	    (1 << 7)
#define TIMER_IRQ_SOURCE    (1 << 6)
#define DMA_IRQ_SOURCE	    (1 << 5)

//ppu irq io
//5-4   H/V IRQ (0=Disable, 1=At H=H + V=Any, 2=At V=V + H=0, 3=At H=H + V=V)
#define HV_IRQ_H_V_DISABLED (0x00)
#define HV_IRQ_HH_V_ANY     (0x10)
#define HV_IRQ_VV_H_0       (0x20)
#define HV_IRQ_HH_HV        (0x30)
//7     VBlank NMI Enable  (0=Disable, 1=Enable) (Initially disabled on reset)
#define VBLANK_NMI_IRQENABLE          (0x80)

typedef int (*intfuncptr)();
typedef uint32 (*u32funcptr)();
typedef void (*voidfuncptr)();

//ReadWrite mapped table defs
typedef void (*IOWriteFunc)(uint32 addr, uint32 byte);
typedef uint32 (*IOReadFunc)(uint32 addr);

//masked bits from a joypad port

//Old Style Joypad Registers	$4016	JOYSER0	single (write)	read/write	any time that is not auto-joypad
//Old Style Joypad Registers	$4017	JOYSER1	many (read)	read	any time that is not auto-joypad

//int get_joypad() is the source for these write joypad callbacks. (any DS source)
//the whole emulator will read always from read_joypad1 and read_joypad2 same for write_joypad1 and write_joypad2

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void fillMemory( void * addr, uint32 count, uint32 value );
extern void zeroMemory( void * addr, uint32 count );

//Snes Hardware
extern struct s_cpu	CPU;
//extern struct s_snes	SNES;
//extern struct s_snescore	SNESC;
//extern struct s_gfx	GFX;
//extern struct s_cfg	CFG;

extern int _offsetY_tab[4];
extern uint32 screen_mode;
extern int APU_MAX;
extern uint32 keys;
extern int	SPC700_emu;
extern int	PPU_fastDMA_2118_1(int offs, int bank, int len);
extern void DMA_transfert(uchar port);
extern void		HDMA_transfert(uint8 port);
extern uint32	IONOP_DMA_READ(uint32 addr);
extern uint32	IONOP_PPU_READ(uint32 addr);
extern void	IONOP_PPU_WRITE(uint32 addr, uint32 byte);
extern void	IONOP_DMA_WRITE(uint32 addr, uint32 byte);
extern void	W4016(uint32 addr, uint32 value);
extern void	W4017(uint32 addr, uint32 value);
extern void	W4200(uint32 addr, uint32 value);
extern void	W4203(uint32 addr, uint32 value);
extern void	W4206(uint32 addr, uint32 value);
extern void	W4207(uint32 addr, uint32 value);
extern void	W4208(uint32 addr, uint32 value);
extern void	W420B(uint32 addr, uint32 value);
extern void	W420C(uint32 addr, uint32 value);
extern uint32	R4016(uint32 addr);
extern uint32	R4017(uint32 addr);
extern uint32	R4210(uint32 addr);
extern uint32	R4211(uint32 addr);
extern uint32	R4212(uint32 addr);
extern uint32	R2121(uint32 addr);
extern uint32	R213X(uint32 addr);
extern uint32	R2137(uint32 addr);
extern uint32	R2138(uint32 addr);
extern uint32	R2139(uint32 addr);
extern uint32	R213A(uint32 addr);
extern uint32	R213B(uint32 addr);
extern uint32	R213C(uint32 addr);
extern uint32	R213D(uint32 addr);
extern uint32	R213F(uint32 addr);
extern uint32	R2140(uint32 addr);
extern uint32	R2141(uint32 addr);
extern uint32	R2142(uint32 addr);
extern uint32	R2143(uint32 addr);
extern uint32	R2180(uint32 addr);
extern void	W2100(uint32 addr, uint32 value);
extern void	W2101(uint32 addr, uint32 value);
extern void	W2102(uint32 addr, uint32 value);
extern void	W2103(uint32 addr, uint32 value);
extern void	W2104(uint32 addr, uint32 value);
extern void	W2105(uint32 addr, uint32 value);
extern void	W2106(uint32 addr, u32 value);
extern void	W2107(uint32 addr, uint32 value);
extern void	W2108(uint32 addr, uint32 value);
extern void	W2109(uint32 addr, uint32 value);
extern void	W210A(uint32 addr, uint32 value);
extern void	W210B(uint32 addr, uint32 value);
extern void	W210C(uint32 addr, uint32 value);
extern void	W210D(uint32 addr, uint32 value);
extern void	W210E(uint32 addr, uint32 value);
extern void	W210F(uint32 addr, uint32 value);
extern void	W2110(uint32 addr, uint32 value);
extern void	W2111(uint32 addr, uint32 value);
extern void	W2112(uint32 addr, uint32 value);
extern void	W2113(uint32 addr, uint32 value);
extern void	W2114(uint32 addr, uint32 value);
extern void	W2115(uint32 addr, uint32 value);
extern void	W2116(uint32 addr, uint32 value);
extern void	W2117(uint32 addr, uint32 value);
extern void	W2118(uint32 addr, uint32 value);
extern void	W2119(uint32 addr, uint32 value);
extern void	W211A(uint32 addr, uint32 value);
extern void	W211B(uint32 addr, uint32 value);
extern void	W211C(uint32 addr, uint32 value);
extern void	W211D(uint32 addr, uint32 value);
extern void	W211E(uint32 addr, uint32 value);
extern void	W211F(uint32 addr, uint32 value);
extern void	W2120(uint32 addr, uint32 value);
extern void	W2121(uint32 addr, uint32 value);
extern void	W2122(uint32 addr, uint32 value);
extern void	W2132(uint32 addr, uint32 value);
extern void	W2133(uint32 addr, uint32 value);
extern void	pseudoSleep(int d);
extern volatile uint32 dummy;
extern void	W2140(uint32 addr, uint32 value);
extern void	W2141(uint32 addr, uint32 value);
extern void	W2142(uint32 addr, uint32 value);
extern void	W2143(uint32 addr, uint32 value);
extern void	W2180(uint32 addr, uint32 value);
extern void	WW210D(uint32 addr, uint32 value);
extern void	WW210E(uint32 addr, uint32 value);
extern void	WW210F(uint32 addr, uint32 value);
extern void	WW2110(uint32 addr, uint32 value);
extern void	WW2111(uint32 addr, uint32 value);
extern void	WW2112(uint32 addr, uint32 value);
extern void	WW2113(uint32 addr, uint32 value);
extern void	WW2114(uint32 addr, uint32 value);
extern void	WW2122(uint32 addr, uint32 value);

//Window Registers
extern void	W2126(uint32 addr, u32 val);
extern void	W2127(uint32 addr, u32 val);
extern void	W2128(uint32 addr, u32 val);
extern void	W2129(uint32 addr, u32 val);
extern void	W2123(uint32 addr, u32 val);
extern void	W2124(uint32 addr, u32 val);
extern void	W2125(uint32 addr, u32 val);
extern void	W212A(uint32 addr, u32 val);
extern void	W212B(uint32 addr, u32 val);
extern void	W212E(uint32 addr, u32 val);
extern void	W212F(uint32 addr, u32 val);

extern IOWriteFunc	IOWrite_PPU[0x90];
extern IOWriteFunc	IOWriteWord_PPU[0x90];
extern IOReadFunc	IORead_PPU[0x90];
extern IOWriteFunc	IOWrite_DMA[0x20];
extern IOReadFunc	IORead_DMA[0x20];
extern void	PPU_port_write(uint32 address, uint8 byte);
extern uint8	PPU_port_read(uint32 address);
extern void	DMA_port_write(uint32 address, uint8 byte);
extern uint8	DMA_port_read(uint32 address);
extern void HDMA_write_port(uchar port, uint8 *data);
extern void	HDMA_write();
extern void	read_mouse();
extern void read_scope();
extern void	update_joypads();
extern void SNES_update();
extern void GoNMI();
extern void GoIRQ();

//snes.c
extern void	init_GFX();
extern void	reset_GFX();
extern void	reset_CPU();
extern void	reset_SNES();
extern void	UnInterleaveROM();
extern void	load_ROM(sint8 *ROM, int ROM_size);

//input.c
extern int get_joypad();

extern uint16 read_joypad1();
extern uint16 read_joypad2();
extern void write_joypad1(uint16 bits);
extern void write_joypad2(uint16 bits);


#ifdef __cplusplus
}
#endif
