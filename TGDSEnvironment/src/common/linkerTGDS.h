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

#ifndef __linker_ext_shared_h__
#define __linker_ext_shared_h__

#include "exceptionTGDS.h"

//Interrupt Routines
#ifdef ARM7

	#ifdef EXCEPTION_VECTORS_0x00000000
	//todo: replace projects that their own exception vectors @ 0x00000000 methods for raising exceptions
	#endif
	
	#ifdef EXCEPTION_VECTORS_0xffff0000
		#define INTERRUPT_VECTOR *((uint32*)&_arm7_irqhandler)
		#define SWI_CHECKBITS *((uint32*)&_arm7_irqcheckbits)
	#endif
	
#endif

#ifdef ARM9

	#ifdef EXCEPTION_VECTORS_0x00000000
	//todo: replace projects that their own exception vectors @ 0x00000000 methods for raising exceptions
	#endif
	
	#ifdef EXCEPTION_VECTORS_0xffff0000
		#define INTERRUPT_VECTOR *((uint32*)&_arm9_irqhandler)
		#define SWI_CHECKBITS *((uint32*)&_arm9_irqcheckbits)
	#endif
	
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif

//access to vectors from LD map
#ifdef ARM9
extern 	vuint32	_arm9_irqhandler;
extern	vuint32	_arm9_irqcheckbits;
#endif

#ifdef ARM7
extern 	vuint32	_arm7_irqhandler;
extern	vuint32	_arm7_irqcheckbits;
#endif

#ifdef __cplusplus
}
#endif