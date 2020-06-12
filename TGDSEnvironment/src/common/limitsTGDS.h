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
#ifndef TGDSLIMITS_H
#define TGDSLIMITS_H

#include <dirent.h>

#define OPEN_MAXTGDS (int)(OPEN_MAXFILEDES)					//Available POSIX File Descriptors (from POSIX -> TGDS)
#define MAX_TGDSFILENAME_LENGTH (int)(NAME_MAX)				//NAME_MAX: Max filename (POSIX) that inherits into TGDSFILENAME_LENGTH

#endif

