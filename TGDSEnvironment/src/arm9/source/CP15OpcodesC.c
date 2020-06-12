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

#include "utilsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "nds_cp15_misc.h"

void coherent_user_range_by_size(uint32 start,sint32 size){
	uint32 end = (uint32)((uint8*)(uint32*)start + (sint32)size);
	coherent_user_range(start,end);
}

void flush_dcache_all(){
	flush_dcache_area((uint32*)get_dtcm_start(), get_dtcm_size());
}