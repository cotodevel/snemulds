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

#ifdef ARM9

#ifndef __devoptab_devices_h__
#define __devoptab_devices_h__

#include "posixHandleTGDS.h"
#include "devoptab_devices.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "ff.h"
#include "limitsTGDS.h"


#ifdef __cplusplus
extern "C"{
#endif

//stdin / stdout name descriptors 
extern const sint8 * stdin_name_desc;
extern const sint8 * stdout_name_desc;
extern const sint8 * stderr_name_desc;
extern const sint8 * stdstub_name_desc;

extern const struct devoptab_t devoptab_stdin;
extern const struct devoptab_t devoptab_stdout;
extern const struct devoptab_t devoptab_sterr;
extern const struct devoptab_t devoptab_sdFilesystem;
extern const struct devoptab_t devoptab_stub;	//initialize up to OPEN_MAXTGDS POSIX
extern const struct devoptab_t *devoptab_struct[OPEN_MAXTGDS];

extern int open_r_stdin ( struct _reent *r, const sint8 *path, int flags, int mode );
extern int close_r_stdin ( struct _reent *r, int fd );
extern _ssize_t write_r_stdin( struct _reent *r, int fd, const sint8 *ptr, int len );
extern _ssize_t read_r_stdin ( struct _reent *r, int fd, sint8 *ptr, int len );

extern int open_r_stdout ( struct _reent *r, const sint8 *path, int flags, int mode );
extern int close_r_stdout ( struct _reent *r, int fd );
extern _ssize_t write_r_stdout( struct _reent *r, int fd, const sint8 *ptr, int len );
extern _ssize_t read_r_stdout ( struct _reent *r, int fd, sint8 *ptr, int len );

extern int open_r_stderr ( struct _reent *r, const sint8 *path, int flags, int mode );
extern int close_r_stderr ( struct _reent *r, int fd );
extern _ssize_t write_r_stderr( struct _reent *r, int fd, const sint8 *ptr, int len );
extern _ssize_t read_r_stderr ( struct _reent *r, int fd, sint8 *ptr, int len );

extern int open_r_fatfs ( struct _reent *r, const sint8 *path, int flags, int mode );
extern int close_r_fatfs ( struct _reent *r, int fd );
extern _ssize_t write_r_fatfs( struct _reent *r, int fd, const sint8 *ptr, int len );
extern _ssize_t read_r_fatfs ( struct _reent *r, int fd, sint8 *ptr, int len );

#ifdef ARM9

extern sint32 open_posix_filedescriptor_devices();

extern void initTGDSDevoptab();
#endif

#ifdef __cplusplus
}
#endif


#endif

#endif
