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


//Taken from rtc.c InfernoDS.

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "clockTGDS.h"
#include "ipcfifoTGDS.h"
#include <string.h>
#include "biosTGDS.h"

#ifdef ARM9
#include "nds_cp15_misc.h"
#endif

//Clock: 

#ifdef ARM7

void rtcTransaction(uchar *cmd, ulong cmdlen, uchar *res, ulong reslen)
{
	uchar bit = 0;
	uchar i = 0;

	REG_CLOCK_CNT = CS_0 | SCK_1 | SIO_1;
	swiDelay(RTC_DELAY);
	REG_CLOCK_CNT = CS_1 | SCK_1 | SIO_1;
	swiDelay(RTC_DELAY);

	for (i = 0; i < cmdlen; i++) {
		for (bit = 0; bit < 8; bit++) {
			REG_CLOCK_CNT = CS_1 | SCK_0 | SIO_out | (cmd[i] >> 7);
			swiDelay(RTC_DELAY);

			REG_CLOCK_CNT = CS_1 | SCK_1 | SIO_out | (cmd[i] >> 7);
			swiDelay(RTC_DELAY);

			cmd[i] = cmd[i] << 1;
		}
	}

	for (i = 0; i < reslen; i++) {
		res[i] = 0;
		for (bit = 0; bit < 8; bit++) {
			REG_CLOCK_CNT = CS_1 | SCK_0;
			swiDelay(RTC_DELAY);
			
			REG_CLOCK_CNT = CS_1 | SCK_1;
			swiDelay(RTC_DELAY);

			if (REG_CLOCK_CNT & SIO_in)
				res[i] |= (1 << bit);
		}
	}

	REG_CLOCK_CNT = CS_0 | SCK_1;
	swiDelay(RTC_DELAY);
}

uchar BCDToInt(uchar data)
{
	return ((data & 0xF) + ((data & 0xF0) >> 4) * 10);
}

ulong get_nds_seconds(uchar *time)
{
	struct tm tmInst;
	uchar hours;
	uchar i;

	hours = BCDToInt(time[4] & HOUR_MASK);

	if ((time[4] & RTC_PM) && (hours < 12)) {
		hours += 12;
	}

	for (i = 0; i < 7; i++) {
		time[i] = BCDToInt(time[i]);
	}

	tmInst.tm_sec  = time[6];
	tmInst.tm_min  = time[5];
	tmInst.tm_hour = hours;
	
	TGDSsetDayOfWeek((uint8)time[3]);	//get day of week
	
	tmInst.tm_mday = time[2];
	tmInst.tm_mon  = time[1];
	tmInst.tm_year = time[0] + 2000;	
	//tmInst.tzoff = -1;
	
	
	memcpy((uint8*)&TGDSIPC->tmInst, (uint8*)&tmInst, sizeof(tmInst));
	return tm2sec(&tmInst);	
}


ulong nds_get_time7(void)
{
	uchar cmd;
	uchar time[8];

	cmd = READ_DATA_REG1;
	rtcTransaction(&cmd, 1, &(time[1]), 7);

	cmd = READ_STATUS_REG1;
	rtcTransaction(&cmd, 1, &(time[0]), 1);
	
	return get_nds_seconds(&(time[1]));

}

void nds_set_time7(ulong secs){
	/* TODO: revise */
	struct tm tmInst;
	uchar time[8];

	sec2tm(secs, &tmInst);

	time[6+1] = tmInst.tm_sec;
	time[5+1] = tmInst.tm_min;
	time[4+1] = tmInst.tm_hour;
	time[2+1] = tmInst.tm_mday;
	time[1+1] = tmInst.tm_mon;
	time[0+1] = tmInst.tm_year - 2000;
	//tmInst.tzoff = -1;

	time[0] = WRITE_DATA_REG1;
	rtcTransaction(time, 8, 0, 0);
};


/*
 *  days per month plus days/year
 */
static	int	dmsize[] =
{
	365, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
static	int	ldmsize[] =
{
	366, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/*
 *  return the days/month for the given year
 */
int * yrsize(int yr)
{
	if( (yr % 4 == 0) && (yr % 100 != 0 || yr % 400 == 0))
		return ldmsize;
	else
		return dmsize;
}

// taken from emu/Nt/ie-os.c
long tm2sec(struct tm *tmInst)
{
	long secs;
	int i, *d2m;

	secs = 0;

	/*
	 *  seconds per year
	 */
	for(i = 1970; i < tmInst->tm_year; i++){
		d2m = yrsize(i);
		secs += d2m[0] * SEC2DAY;
	}

	/*
	 *  seconds per month
	 */
	d2m = yrsize(tmInst->tm_year);
	for(i = 1; i < tmInst->tm_mon; i++){
		secs += d2m[i] * SEC2DAY;
	}

	/*
	 * secs in last month
	 */
	secs += (tmInst->tm_mday-1) * SEC2DAY;

	/*
	 * hours, minutes, seconds
	 */
	secs += tmInst->tm_hour * SEC2HOUR;
	secs += tmInst->tm_min * SEC2MIN;
	secs += tmInst->tm_sec;

	return secs;
}

void sec2tm(ulong secs, struct tm *tmInst)
{
	int d;
	long hms, day;
	int *d2m;

	/*
	 * break initial number into days
	 */
	hms = secs % SEC2DAY;
	day = secs / SEC2DAY;
	if(hms < 0) {
		hms += SEC2DAY;
		day -= 1;
	}

	/*
	 * generate hours:minutes:seconds
	 */
	tmInst->tm_sec = hms % 60;
	d = hms / 60;
	tmInst->tm_min = d % 60;
	d /= 60;
	tmInst->tm_hour = d;

	/*
	 * year number
	 */
	if(day >= 0)
		for(d = 1970; day >= *yrsize(d); d++)
			day -= *yrsize(d);
	else
		for (d = 1970; day < 0; d--)
			day += *yrsize(d-1);
	tmInst->tm_year = d;

	/*
	 * generate month
	 */
	d2m = yrsize(tmInst->tm_year);
	for(d = 1; day >= d2m[d]; d++)
		day -= d2m[d];
	tmInst->tm_mday = day + 1;
	tmInst->tm_mon = d;

	return;
}


#endif

//Shared:
struct tm * getTime(){
	
	return(struct tm *)(&TGDSIPC->tmInst);
}

ulong getNDSRTCInSeconds(){
	
	return(ulong)(TGDSIPC->ndsRTCSeconds);
}

uint8 TGDSgetDayOfWeek(){
	
	#ifdef ARM9
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)TGDSIPC, sizeof(struct sIPCSharedTGDS));
	#endif
	return (uint8)TGDSIPC->dayOfWeek;
}

void TGDSsetDayOfWeek(uint8 DayOfWeek){
	
	#ifdef ARM9
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)TGDSIPC, sizeof(struct sIPCSharedTGDS));
	#endif
	TGDSIPC->dayOfWeek = DayOfWeek;
}