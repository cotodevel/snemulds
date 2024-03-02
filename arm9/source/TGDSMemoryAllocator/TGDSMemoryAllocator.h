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

#ifndef __TGDSMemoryAllocator_h__
#define __TGDSMemoryAllocator_h__

#include "posixHandleTGDS.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

	////////[Custom Memory implementation is selected]////////

//Definition that overrides the weaksymbol expected from toolchain to init ARM9's TGDS memory allocation
extern struct AllocatorInstance * getProjectSpecificMemoryAllocatorSetup(bool isCustomTGDSMalloc);

#ifdef __cplusplus
}
#endif