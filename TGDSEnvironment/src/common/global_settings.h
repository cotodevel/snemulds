
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


#ifndef __nds_global_settings_h__
#define __nds_global_settings_h__

//These settings will enable certain features to either ARM7 or ARM9. Make sure to select just one.
//Changing them requires the whole TGDS to be recompiled, and linked against the TGDS project.

//#define EXCEPTION_VECTORS_0x00000000
#define EXCEPTION_VECTORS_0xffff0000

#if defined(EXCEPTION_VECTORS_0x00000000) && defined(EXCEPTION_VECTORS_0xffff0000)
#error "Please provide either EXCEPTION_VECTORS_0xffff0000 or EXCEPTION_VECTORS_0x00000000"
#endif

//ARM7: DLDI Code runs from ARM7 (experimental)
//ARM9: DLDI Code runs from ARM9 (breaks DSi compatibility! But stable)
//#define ARM7_DLDI
#define ARM9_DLDI

#if defined(ARM7_DLDI) && defined(ARM9_DLDI)
#error "DLDI is either ARM7 or ARM9 only."
#endif


#endif