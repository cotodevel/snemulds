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

#ifndef __nds_clock_h__
#define __nds_clock_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <time.h>



#define SEC2MIN 60L
#define SEC2HOUR (60L*SEC2MIN)
#define SEC2DAY (24L*SEC2HOUR)


// RTC registers
#define	REG_CLOCK_CNT		(*((vuint8 *) 0x04000138))
#define WRITE_STATUS_REG1	0x60
#define READ_STATUS_REG1	0x61

// full 7 bytes for time and date
#define WRITE_DATA_REG1	0x64
#define READ_DATA_REG1	0x65

#define RTC_DELAY 48
#define CS_0    (1<<6)
#define CS_1    ((1<<6) | (1<<2))
#define SCK_0   (1<<5)
#define SCK_1   ((1<<5) | (1<<1))
#define SIO_0   (1<<4)
#define SIO_1   ((1<<4) | (1<<0))
#define SIO_out (1<<4)
#define SIO_in  (1)

#define RTC_PM (1<<6)
#define HOUR_MASK 0x3f



#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM7
extern void rtcTransaction(uchar *cmd, ulong cmdlen, uchar *res, ulong reslen);
extern uchar BCDToInt(uchar data);
extern ulong get_nds_seconds(uchar *time);
extern ulong nds_get_time7(void);
extern void nds_set_time7(ulong secs);
extern int * yrsize(int yr);
extern long tm2sec(struct tm *tmInst);
extern void sec2tm(ulong secs, struct tm *tmInst);

#endif

extern struct tm * getTime();
extern ulong getNDSRTCInSeconds();
extern uint8 TGDSgetDayOfWeek();
extern void TGDSsetDayOfWeek(uint8 DayOfWeek);

#ifdef __cplusplus
}
#endif