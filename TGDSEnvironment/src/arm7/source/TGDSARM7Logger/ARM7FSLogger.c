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

#include <stdarg.h>
#include <string.h>
#include "ARM7FS.h"
#include "limitsTGDS.h"

int curARM7LoggerWriteOffset = 0;
int ARM7Logger(char *str)
{
	char buffOut[MAX_TGDSFILENAME_LENGTH+1];
	memset(buffOut, 0, sizeof(buffOut));
	strcpy(buffOut, str);
	buffOut[strlen(str) + 1] = '\0';
	ARM7FS_BufferSaveByIRQ((u8*)&buffOut[0], curARM7LoggerWriteOffset, strlen(str) + 1); 
	curARM7LoggerWriteOffset+=(strlen(str) + 1);
	return (strlen(str) + 1);
}