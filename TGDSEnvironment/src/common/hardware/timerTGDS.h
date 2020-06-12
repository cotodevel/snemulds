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

#ifndef __nds_timer_h__
#define __nds_timer_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#define TIMERXDATA(timerindx)     (*(vuint16*)(0x04000100|(timerindx<<2)))
#define TIMERXCNT(timerindx)     (*(vuint16*)(0x04000102|(timerindx<<2)))	//aka TIMER_CR(x)

#ifdef GBA
#define BUS_CLOCK (33513982/2)
#endif

#if defined (ARM7) || defined (ARM9) 
#define BUS_CLOCK (33513982)
#endif

#define TIMER_ENABLE    (1<<7)
#define TIMER_IRQ_REQ   (1<<6)
#define TIMER_CASCADE   (1<<2)

#define TIMER_DIV_1     (0)
#define TIMER_DIV_64    (1)
#define TIMER_DIV_256   (2)
#define TIMER_DIV_1024  (3)

#define TIMER_FREQ(n)    (-BUS_CLOCK/(n))
#define TIMER_FREQ_64(n)  (-(BUS_CLOCK>>6)/(n))
#define TIMER_FREQ_256(n) (-(BUS_CLOCK>>8)/(n))
#define TIMER_FREQ_1024(n) (-(BUS_CLOCK>>10)/(n))

#endif

#ifdef __cplusplus
extern "C"{
#endif


#ifdef __cplusplus
}
#endif