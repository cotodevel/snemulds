/*---------------------------------------------------------------------------------
	$Id: touch.c,v 1.18 2006/04/04 23:05:19 wntrmute Exp $

	Touch screen control for the ARM7

	Copyright (C) 2005
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)
		
		modified by PADRIANTOR
		
		put in pocketSPC by Nicolas Lannier (archeide)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
			must not claim that you wrote the original software. If you use
			this software in a product, an acknowledgment in the product
			documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
			must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
			distribution.

	$Log: touch.c,v $
	Revision 1.19  2006/09/04 11:00:00  PadrinatoR
	Modified touch reading code to avoid "stylus jumping". More info: http://forum.gbadev.org/viewtopic.php?t=10925

	Revision 1.18  2006/04/04 23:05:19  wntrmute
	Added pressure reading to touchscreen function
	
	Revision 1.17  2006/02/21 23:50:10  wntrmute
	corrected build errors
	
	Revision 1.16  2006/02/21 20:25:49  dovoto
	Fixed some compilation errors (missing paranthesis and missing include)
	
	Revision 1.15  2006/02/21 00:28:32  wntrmute
	disable interrupts around touch screen reading
	
	Revision 1.14  2006/01/30 18:59:45  wntrmute
	improved touch code
	
	Revision 1.13  2006/01/12 11:13:55  wntrmute
	modified touch reading code from suggesrions found here -> http://forum.gbadev.org/viewtopic.php?t=7980
	
	Revision 1.12  2005/12/17 01:03:05  wntrmute
	corrected typos
	changed to median values
	
	Revision 1.11  2005/12/11 22:49:53  wntrmute
	use con for console device name
	
	Revision 1.10  2005/10/17 15:35:56  wntrmute
	use weighted averaging
	
	Revision 1.9  2005/10/03 21:19:34  wntrmute
	use ratiometric mode
	lock touchscreen on and average several readings
	
	Revision 1.8  2005/09/12 06:51:58  wntrmute
	tidied touch code
	
	Revision 1.7  2005/09/07 18:05:37  wntrmute
	use macros for device settings

	Revision 1.6  2005/08/23 17:06:10  wntrmute
	converted all endings to unix

	Revision 1.5  2005/08/01 23:12:17  wntrmute
	extended touchReadXY to return touchscreen co-ordinates as well as screen co-ordinates

	Revision 1.4  2005/07/29 00:57:40  wntrmute
	updated file headers
	added touchReadXY function
	made header C++ compatible

	Revision 1.3  2005/07/12 17:32:20  wntrmute
	updated file header

	Revision 1.2  2005/07/11 23:12:15  wntrmute
	*** empty log message ***

---------------------------------------------------------------------------------*/

#include <nds/jtypes.h>
#include <nds/system.h>
#include <nds/arm7/touch.h>
#include <nds/interrupts.h>

#include <stdlib.h>

#define TSC_MEASURE_TEMP1    0x84
#define TSC_MEASURE_Y        0x91
#define TSC_MEASURE_BATTERY  0xA4
#define TSC_MEASURE_Z1       0xB1
#define TSC_MEASURE_Z2       0xC1
#define TSC_MEASURE_X        0xD1
#define TSC_MEASURE_AUX      0xE4
#define TSC_MEASURE_TEMP2    0xF4

//---------------------------------------------------------------------------------
typedef struct sMyIPC {
//---------------------------------------------------------------------------------
#if 0
  uint32 heartbeat;          // counts frames
#endif  

	int16 touchX,   touchY;   // TSC X, Y
#if 0	
	u8 touchXpx, touchYpx; // TSC X, Y pixel values
#else	
	int16 touchXpx, touchYpx; // TSC X, Y pixel values
#endif	
	int16 touchZ1,  touchZ2;  // TSC x-panel measurements
	uint16 tdiode1,  tdiode2;  // TSC temperature diodes
	uint32 temperature;        // TSC computed temperature

	uint16 buttons;            // X, Y, /PENIRQ buttons

	union {
		uint8 curtime[8];        // current time response from RTC

    struct {
			u8 rtc_command;
			u8 rtc_year;           //add 2000 to get 4 digit year
			u8 rtc_month;          //1 to 12
			u8 rtc_day;            //1 to (days in month)

			u8 rtc_incr;
			u8 rtc_hours;          //0 to 11 for AM, 52 to 63 for PM
			u8 rtc_minutes;        //0 to 59
			u8 rtc_seconds;        //0 to 59
    };
  };
	u8 touched;				  //TSC touched?  
	u8 touchError;	//TSC error reading data?

	uint16 battery;            // battery life ??  hopefully.  :)
	uint16 aux;                // i have no idea...

  vuint8 mailBusy;
} tMyIPC;


#define MyIPC ((tMyIPC volatile *)(0x027FF000))


#if 0
u16 readtouch(u32 command) {
    u16 result;

    REG_SPICNT = SPI_ENABLE | 0xA01;
    REG_SPIDATA = command;
    while (REG_SPICNT & SPI_BUSY);

    REG_SPIDATA = 0;
    while (REG_SPICNT & SPI_BUSY);
    result = REG_SPIDATA;

    REG_SPICNT  = SPI_ENABLE | 0x201;
    REG_SPIDATA = 0;
    while (REG_SPICNT & SPI_BUSY);

    return ((result & 0x7F) << 5) | (REG_SPIDATA >> 3);
}

void updateMyIPC()
{
	MyIPC->buttons = REG_KEYXY;
    MyIPC->touched = !(REG_KEYXY & 0x40) ? 1 : 0;
	MyIPC->touchX = readtouch(TSC_MEASURE_X);
	MyIPC->touchY = readtouch(TSC_MEASURE_Y);	
}

#else

static u8 last_time_touched = 0;

static u8 range_counter_1 = 0;
static u8 range_counter_2 = 0;
static u8 range = 20;
static u8 min_range = 20;

typedef struct mytouchPosition {
	u8 error;
	u8 touched;
	int16	x;
	int16	y;
	int16	px;
	int16	py;
	int16	z1;
	int16	z2;
} mytouchPosition;



//---------------------------------------------------------------------------------
uint16 touchRead(uint32 command) {
//---------------------------------------------------------------------------------
	uint16 result, result2;

	uint32 oldIME = REG_IME;

	REG_IME = 0;
	
	SerialWaitBusy();

	// Write the command and wait for it to complete
	REG_SPICNT = SPI_ENABLE | SPI_BAUD_2MHz | SPI_DEVICE_TOUCH | SPI_CONTINUOUS; //0x0A01;
	REG_SPIDATA = command;
	SerialWaitBusy();

	// Write the second command and clock in part of the data
	REG_SPIDATA = 0;
	SerialWaitBusy();
	result = REG_SPIDATA;

	// Clock in the rest of the data (last transfer)
	REG_SPICNT = SPI_ENABLE | 0x201;
	REG_SPIDATA = 0;
	SerialWaitBusy();

	result2 = REG_SPIDATA >>3;

	REG_IME = oldIME;

	// Return the result
	return ((result & 0x7F) << 5) | result2;
}


//---------------------------------------------------------------------------------
uint32 touchReadTemperature(int * t1, int * t2) {
//---------------------------------------------------------------------------------
	*t1 = touchRead(TSC_MEASURE_TEMP1);
	*t2 = touchRead(TSC_MEASURE_TEMP2);
	return 8490 * (*t2 - *t1) - 273*4096;
}

int16 readTouchValue(uint32 command, int16 *dist_max, u8 *err){
	int16 values[5];
	int32 aux1, aux2, aux3, dist, dist2, result;
	u8 i, j, k;

	*err = 1;

	SerialWaitBusy();

	REG_SPICNT = 0x8A01;
	REG_SPIDATA = command;

	SerialWaitBusy();

	for(i=0; i<5; i++){
		REG_SPIDATA = 0;
		SerialWaitBusy();

		aux1 = REG_SPIDATA;
		aux1 = aux1 & 0xFF;
		aux1 = aux1 << 16;
		aux1 = aux1 >> 8;

		values[4-i] = aux1;

		REG_SPIDATA = command;
		SerialWaitBusy();

		aux1 = REG_SPIDATA;
		aux1 = aux1 & 0xFF;
		aux1 = aux1 << 16;

		aux1 = values[4-i] | (aux1 >> 16);
		values[4-i] = ((aux1 & 0x7FF8) >> 3);
	}

	REG_SPICNT = 0x8201;
	REG_SPIDATA = 0;
	SerialWaitBusy();

	dist = 0;
	for(i=0; i<4; i++){
		aux1 = values[i];

		for(j=i+1; j<5; j++){
			aux2 = values[j];
			aux2 = abs(aux1 - aux2);
			if(aux2>dist) dist = aux2;
		}
	}

	*dist_max = dist;

	for(i=0; i<3; i++){
		aux1 = values[i];

		for(j=i+1; j<4; j++){
			aux2 = values[j];
			dist = abs(aux1 - aux2);

			if( dist <= range ){
				for(k=j+1; k<5; k++){
					aux3 = values[k];
					dist2 = abs(aux1 - aux3);

					if( dist2 <= range ){
						result = aux2 + (aux1 << 1);
						result = result + aux3;
						result = result >> 2;
						result = result & (~7);

						*err = 0;

						break;
					}
				}
			}
		}
	}

	if((*err) == 1){
		result = values[0] + values[4];
		result = result >> 1;
		result = result & (~7);
	}

	return (result & 0xFFF);
}

void UpdateRange(uint8 *this_range, int16 dist_max_in_last_reading, u8 error_reading_data, u8 tsc_touched){
	//range_counter_1 = counter_0x380A98C
	//range_counter_2 = counter_0x380A990
	//Initial values:
	// range = 20
	// min_range = 20

	if(tsc_touched != 0){
		if(error_reading_data == 0){
			range_counter_2 = 0;

			if(dist_max_in_last_reading >= ((*this_range) >> 1)){
				range_counter_1 = 0;
			}else{
				range_counter_1++;

				if(range_counter_1 >= 4){
					range_counter_1 = 0;

					if((*this_range) > min_range){
						(*this_range)--;
						range_counter_2 = 3;
					}
				}
			}
		}else{
			range_counter_1 = 0;
			range_counter_2++;

			if(range_counter_2 >= 4){

				range_counter_2 = 0;

				if((*this_range) < 35){  //0x23 = 35
					*this_range = (*this_range) + 1;
				}
			}
		}
	}else{
		range_counter_2 = 0;
		range_counter_1 = 0;
	}
}


u8 CheckStylus(void){

	SerialWaitBusy();

	REG_SPICNT = 0x8A01;
	REG_SPIDATA = 0x84;

	SerialWaitBusy();

	REG_SPIDATA = 0;

	SerialWaitBusy();

	REG_SPICNT = 0x8201;
	REG_SPIDATA = 0;

	SerialWaitBusy();

	if(last_time_touched == 1){
		if( !(REG_KEYXY & 0x40) )
			return 1;
		else{
			REG_SPICNT = 0x8A01;
			REG_SPIDATA = 0x84;

			SerialWaitBusy();

			REG_SPIDATA = 0;

			SerialWaitBusy();

			REG_SPICNT = 0x8201;
			REG_SPIDATA = 0;

			SerialWaitBusy();

			return !(REG_KEYXY & 0x40) ? 2 : 0;
		}
	}else{
		return !(REG_KEYXY & 0x40) ? 1 : 0;
	}
}

mytouchPosition ReadTouchScreen(void){
	int16 dist_max_y, dist_max_x, dist_max;
	u8 error, error_where, first_check, i;

	mytouchPosition touchPos;

	uint32 oldIME = REG_IME;

	REG_IME = 0;

	first_check = CheckStylus();
	if(first_check != 0){
		error_where = 0;

		touchPos.z1 =  readTouchValue(TSC_MEASURE_Z1, &dist_max, &error);
		touchPos.z2 =  readTouchValue(TSC_MEASURE_Z2, &dist_max, &error);

		touchPos.x = readTouchValue(TSC_MEASURE_X, &dist_max_x, &error);
		if(error==1) error_where += 1;

		touchPos.y = readTouchValue(TSC_MEASURE_Y, &dist_max_y, &error);
		if(error==1) error_where += 2;

		REG_SPICNT = 0x8A01;
		for(i=0; i<12; i++){
			REG_SPIDATA = 0;

			SerialWaitBusy();
		}

		REG_SPICNT = 0x8201;
		REG_SPIDATA = 0;

		SerialWaitBusy();

		if(first_check == 2) error_where = 3;

		switch( CheckStylus() ){
		case 0:
			touchPos.touched = 0;
			last_time_touched = 0;
			break;
		case 1:
			touchPos.touched = 1;
			last_time_touched = 1;

			if(dist_max_x > dist_max_y)
				dist_max = dist_max_x;
			else
				dist_max = dist_max_y;

			break;
		case 2:
			touchPos.touched = 1;
			last_time_touched = 0;
			error_where = 3;

			break;
		}
	}else{
		error_where = 3;
		//touchPos.x = 0;
		//touchPos.y = 0;
		touchPos.touched = 0;
		last_time_touched = 0;
	}

	touchPos.error = error_where;

	UpdateRange(&range, dist_max, touchPos.error, touchPos.touched);

	REG_IME = oldIME;

	return touchPos;
}


void updateMyIPC()
{
	static int heartbeat = 0;
 
	uint16 but=0, batt=0;
	int t1=0, t2=0;
	uint32 temp=0;
	uint8 ct[sizeof(MyIPC->curtime)];
	u32 i;
	mytouchPosition tempPos;
 
	// Update the heartbeat
	heartbeat++;
 
	// Read the touch screen
 
	but = REG_KEYXY;


	//MODIFIED BY PADRINATOR (padrinator at gmail dot com) 
	tempPos = ReadTouchScreen();	
	//MODIFIED BY PADRINATOR (padrinator at gmail dot com)


	batt = touchRead(TSC_MEASURE_BATTERY);
 
	// Read the time
	rtcGetTime((uint8 *)ct);
	BCDToInteger((uint8 *)&(ct[1]), 7);
 
	// Read the temperature
	temp = touchReadTemperature(&t1, &t2);
 
	MyIPC->mailBusy = 1;
	// Update the MyIPC struct
#if 0	
	MyIPC->heartbeat	= heartbeat;
#endif	
	MyIPC->buttons		= but;
	//MODIFIED BY PADRINATOR (padrinator at gmail dot com) 
	MyIPC->touchError = tempPos.error;

	MyIPC->touched   = tempPos.touched;
	MyIPC->touchX    = tempPos.x;
    MyIPC->touchY    = tempPos.y;

	MyIPC->touchZ1 = tempPos.z1;
	MyIPC->touchZ2 = tempPos.z2;
	//MODIFIED BY PADRINATOR (padrinator at gmail dot com) 
	MyIPC->battery		= batt;
	MyIPC->mailBusy = 0;
 
	for(i=0; i<sizeof(ct); i++) {
		MyIPC->curtime[i] = ct[i];
	}
 
	MyIPC->temperature = temp;
	MyIPC->tdiode1 = t1;
	MyIPC->tdiode2 = t2;	
}
#endif