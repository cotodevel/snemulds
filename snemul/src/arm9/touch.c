/*---------------------------------------------------------------------------------
	$Id: touch.c,v 1.4 2005/10/09 20:27:23 wntrmute Exp $

	touch screen input code

 	Copyright (C) 2005
		Dave Murphy (WinterMute)

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
	Revision 1.5  2006/09/04 11:00:00  PadrinatoR
	Modified touch reading code to avoid "stylus jumping". More info: http://forum.gbadev.org/viewtopic.php?t=10925
	Now, calibration and ADC to SCR conversion is done in ARM9 :)

	Revision 1.4  2005/10/09 20:27:23  wntrmute
	add check for arm7 updating IPC
	
	Revision 1.3  2005/08/30 17:53:11  wntrmute
	only include required headers
	
	Revision 1.2  2005/08/23 17:06:10  wntrmute
	converted all endings to unix

	Revision 1.1  2005/08/04 17:58:30  wntrmute
	added touch co-ordinate wrapper



---------------------------------------------------------------------------------*/

#include <nds.h>

#include <nds/ipc.h>
#include <nds/arm9/input.h>
#include <nds/jtypes.h>
#include <nds/system.h>

/************* TO PUT in include ********* */
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

#if 0
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
/************* TO PUT in include ********* */
#endif

static int32 a, b, c, d;

void PrecalculateCalibrationData(void){	
	c = ((PersonalData->calX1 - PersonalData->calX2) << 8)/(PersonalData->calX1px - PersonalData->calX2px);
	d = ((PersonalData->calY1 - PersonalData->calY2) << 8)/(PersonalData->calY1px - PersonalData->calY2px);
	
	a = ((((PersonalData->calX1 + PersonalData->calX2) << 8) - (c * (PersonalData->calX1px + PersonalData->calX2px))) << 9) >> 16;
	b = ((((PersonalData->calY1 + PersonalData->calY2) << 8) - (d * (PersonalData->calY1px + PersonalData->calY2px))) << 9) >> 16;

	c = 0x10000000 / c;
	d = 0x10000000 / d;
}

void ADC_to_SCR(uint16 adc_x, uint16 adc_y, int16 *scr_x, int16 *scr_y){
	s64 aux4;
	uint32 aux1, aux2, aux3, aux5, aux6, aux7;
	s32 aux8;

	aux1 = adc_x << 2;
	aux2 = aux1 >> 31;
	aux1 -= a;
	aux3 = aux2 - (a >> 31);
	aux4 = (c * aux1);
	aux5 = (c * aux3) + (aux4 >> 32);
	aux6 = c >> 31;
	aux7 = (aux6 * aux1) + aux5;
	aux8 = ((aux4 << 32) >> 32) >> 22;
	aux8 = aux8 | (aux7 << 10);
	if(aux8 < 0) aux8 = 0;
	if(aux8 > 255) aux8 = 255;
	*scr_x = aux8;

	aux1 = adc_y << 2;
	aux2 = aux1 >> 31;
	aux1 -= b;
	aux3 = aux2 - (b >> 31);
	aux4 = (d * aux1);
	aux5 = (d * aux3) + (aux4 >> 32);
	aux6 = d >> 31;
	aux7 = (aux6 * aux1) + aux5;
	aux8 = ((aux4 << 32) >> 32) >> 22;
	aux8 = aux8 | (aux7 << 10);
	if(aux8 < 0) aux8 = 0;
	if(aux8 > 191) aux8 = 191;
	*scr_y = aux8;
}

#if 1
//---------------------------------------------------------------------------------
mytouchPosition mytouchReadXY() {
//---------------------------------------------------------------------------------
	while (IPC->mailBusy);
	
	// LEYENDA
	//---------
	// last_measures = dword_211A6AC
	// newpress = dword_211A68C
	// released = dword_211A690
	// ¿?¿? = dword_211A6B0
	// ¿?¿? = dword_211A6A0

	static mytouchPosition touchPos, touchPos_Old;
	static u8 dword_211A6A0; //,dword_211A68C, dword_211A690;
	static u8 dword_211A6B0;
	u8 unk_bool;
	int16 dist_x, dist_y, med_x, med_y, aux1, aux2, aux3, aux4;

/*	if(touchPos.error == 0){
		if(touchPos.touched == 0){
			Stylus.Newpress = 1;
			Stylus.Released = 0;
		}else{
			Stylus.Newpress = 0;
			Stylus.Released = 1;
		}
	}else{
		Stylus.Newpress = 0;
		Stylus.Released = 0;
	}*/

	med_x = touchPos.px;
	med_y = touchPos.py;

	touchPos.error = IPC->tdiode2;
	touchPos.touched = IPC->tdiode1;

	touchPos.x = IPC->touchX;
	touchPos.y = IPC->touchY;

	touchPos.z1 = IPC->touchZ1;
	touchPos.z2 = IPC->touchZ2;

	ADC_to_SCR(touchPos.x, touchPos.y, &(touchPos.px), &(touchPos.py));

	if(touchPos.touched == 0){
		touchPos.touched = 0;
		touchPos_Old.px = -1;
		dword_211A6B0 = 0;
	}else{
		aux1 = touchPos.px;
		aux2 = touchPos.py;
		aux3 = touchPos_Old.px; //signed
		aux4 = touchPos_Old.py; //signed

		dist_x = aux1 - aux3;
		dist_y = aux2 - aux4;

		if(dword_211A6B0 != 0){
			touchPos.px = med_x;
			touchPos.py = med_y;
		}

		dist_x = dist_x << 16;
		dist_y = dist_y << 16;

		if(touchPos.error == 2 || touchPos.error == 0) touchPos_Old.px = aux1;
		if(touchPos.error == 1 || touchPos.error == 0) touchPos_Old.py = aux2;

		dist_x = dist_x >> 16;
		dist_y = dist_y >> 16;

		if( dword_211A6B0 == 0 ){
			touchPos.px = aux3;
			touchPos.py = aux4;
		}

		if(aux3 < 0){
			if( touchPos.error == 0){ //si no hubo "error"
				touchPos.touched = 0;
				dword_211A6B0 = 0;
				dword_211A6A0 = 1;
			}else{
				touchPos.touched = 0;
				touchPos_Old.px = -1;
				dword_211A6B0 = 0;
			}
		}else{
			if(dword_211A6B0 == 0){
				unk_bool = 1;

				if (touchPos.error == 0 && dist_x >= -12 && dist_x <= 12 && dist_y >= -12 && dist_y <= 12)
					unk_bool = 0;

				if(dword_211A6A0 == 0){ //Comprueba la direccion rara
					if(unk_bool == 0)
						dword_211A6B0 = 0;
					else
						dword_211A6B0 = 1;
				}else{
					touchPos.touched = 0;

					if(unk_bool == 0)
						dword_211A6A0 = 0;
					else
						dword_211A6A0 = 1;

					dword_211A6B0 = 0;
				}
			}else{
				if(touchPos.error == 0 && dist_x >= -12 && dist_x <= 12 && dist_y >= -12 && dist_y <= 12){
					touchPos.px = aux3;
					touchPos.py = aux4;
					dword_211A6B0 = 0;
				}else{
					dist_x = touchPos_Old.px - med_x;
					dist_y = touchPos_Old.py - med_y;

					if(dist_x >= -12 && dist_x <= 12 && dist_y >= -12 && dist_y <= 12){
						dword_211A6B0 = 0;
					}else{
						dword_211A6B0 = 1;
					}
				}
			}
		}
	}

	return touchPos;

}
#endif

touchPosition superTouchReadXY()
{
#ifdef USE_STDTOUCH 
	touchPosition touchXY;
	
	touchXY = touchReadXY();
	touchXY = touchReadXY();
	touchXY = touchReadXY();
	
	ADC_to_SCR(touchXY.x, touchXY.y, &(touchXY.px), &(touchXY.py));
	
#else
	touchPosition touchXY;
	mytouchPosition mtouchXY;
		
    mtouchXY = mytouchReadXY();	// It seems first measure is not already updayed
    mtouchXY = mytouchReadXY();	
    mtouchXY = mytouchReadXY();	
	touchXY.px = mtouchXY.px;
	touchXY.py = mtouchXY.py;
#endif	
	return touchXY;
}

