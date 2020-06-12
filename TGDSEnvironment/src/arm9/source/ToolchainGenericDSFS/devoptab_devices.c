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

#include "devoptab_devices.h"
#include "consoleTGDS.h"
#include "dsregs.h"
#include "typedefsTGDS.h"
#include "consoleTGDS.h"

#include "errno.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "fatfslayerTGDS.h"
#include "posixHandleTGDS.h"

//device name for stdin driver descriptor (max devoptab_fname_size size)
const sint8 * stdin_name_desc = (sint8*)("stdin");
//device name for stdout driver descriptor (max devoptab_fname_size size)
const sint8 * stdout_name_desc = (sint8*)("stdout");
//device name for stderr driver descriptor (max devoptab_fname_size size)
const sint8 * stderr_name_desc = (sint8*)("stderr");
//device name for stub driver descriptor (max devoptab_fname_size size)
const sint8 * stdstub_name_desc = (sint8*)("stub");

/* dtab for a stream device : stdin */
const struct devoptab_t devoptab_stdin = { "", &open_r_stdin, &close_r_stdin, &write_r_stdin, &read_r_stdin };

/* dtab for a stream device : stdout->console NDS */
const struct devoptab_t devoptab_stdout = { "", &open_r_stdout, &close_r_stdout, &write_r_stdout, &read_r_stdout };

/* dtab for a stream device : stderr->debug error newlib */
const struct devoptab_t devoptab_sterr = { "", &open_r_stderr, &close_r_stderr, &write_r_stderr, &read_r_stderr };

/* dtab for a stream device : FATFS */
const struct devoptab_t devoptab_sdFilesystem = { "", &open_r_fatfs, &close_r_fatfs, &write_r_fatfs, &read_r_fatfs };

/* dtab for a stream device : STUB */
const struct devoptab_t devoptab_stub = { "stub:/", 0, 0, 0, 0 };

const struct devoptab_t *devoptab_struct[OPEN_MAXTGDS] = {
   &devoptab_stdin, /* standard input */
   &devoptab_stdout, /* standard output */
   &devoptab_sterr, /* standard error */
   &devoptab_sdFilesystem, /* custom filesystem  devoptab */
   &devoptab_stub,	//stub
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub,
   &devoptab_stub
};

void initTGDSDevoptab(){	
	//newlib devoptabs
	memcpy ( (uint32*)&devoptab_stdin.name[0], (uint32*)stdin_name_desc, strlen(stdin_name_desc));
	memcpy ( (uint32*)&devoptab_stdout.name[0], (uint32*)stdout_name_desc, strlen(stdout_name_desc));
	memcpy ( (uint32*)&devoptab_sterr.name[0], (uint32*)stderr_name_desc, strlen(stderr_name_desc));
	memcpy ( (uint32*)&devoptab_stub.name[0], (uint32*)stdstub_name_desc, strlen(stdstub_name_desc));
	
	//devoptab_sdFilesystem is missing on purpose. Must be set manually through void initTGDS(bool defaultDriver, char * devoptabFSName)
}

sint32 open_posix_filedescriptor_devices(){
	sint32 i = 0;
	sint32 items = 0;	//POSIX devoptabs start at index 0
	for(i = 0; i < OPEN_MAXTGDS; i++){
		if( strcmp( (sint8*)(&devoptab_struct[i]->name), "stub:/" ) == 0 ){
			//ignore stubs
		}
		else{
			items++;
		}
	}
	return items;
}

//stdin: todo
int open_r_stdin ( struct _reent *r, const sint8 *path, int flags, int mode ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

int close_r_stdin ( struct _reent *r, int fd ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

_ssize_t write_r_stdin( struct _reent *r, int fd, const sint8 *ptr, int len ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

_ssize_t read_r_stdin ( struct _reent *r, int fd, sint8 *ptr, int len ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

int open_r_stdout ( struct _reent *r, const sint8 *path, int flags, int mode ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

int close_r_stdout ( struct _reent *r, int fd ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

//int _vfiprintf_r/_vfprintf_r is overriden in posix_hook_shared.c due to how newlib parses the printf buffer. So we retarget printf to GUI_Printf which already works.
_ssize_t write_r_stdout( struct _reent *r, int fd, const sint8 *ptr, int len ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

_ssize_t read_r_stdout ( struct _reent *r, int fd, sint8 *ptr, int len ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}


//stderr: todo
int open_r_stderr ( struct _reent *r, const sint8 *path, int flags, int mode ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

int close_r_stderr ( struct _reent *r, int fd ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

_ssize_t write_r_stderr( struct _reent *r, int fd, const sint8 *ptr, int len ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

_ssize_t read_r_stderr ( struct _reent *r, int fd, sint8 *ptr, int len ){
	return structfd_posixInvalidFileDirOrBufferHandle;
}

//fatfs: 
//returns / allocates a new struct fd index with either DIR or FIL structure allocated
int open_r_fatfs ( struct _reent *r, const sint8 *path, int flags, int mode ){
	  return fatfs_open(path, flags);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
}

//write (get struct FD index from FILE * handle)
_ssize_t read_r_fatfs( struct _reent *r, int fd, sint8 *ptr, int len ){	 
	int ret = structfd_posixInvalidFileDirOrBufferHandle;
	struct fd *f = getStructFD(fd);
    if (f == NULL)
    {
        errno = EBADF;
    }
    else if ((f->isused == structfd_isunused) || (!f->filPtr))
    {
		//Filehandle not allocated
        errno = EBADF;
    }
    else
    {
		ret = fatfs_read(fd, ptr, len);
    }
    return ret;
}

//close (get struct FD index from FILE * handle)
_ssize_t write_r_fatfs( struct _reent *r, int fd, const sint8 *ptr, int len ){
	
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	struct fd *f = getStructFD(fd);
    
	if (f == NULL){
		errno = EBADF;
    }
    else if ((f->isused == structfd_isunused) || (!f->filPtr) ){
		errno = EBADF;
    }
    else{
	    ret = fatfs_write(f->cur_entry.d_ino, (sint8*)ptr, len);
    }
    return ret;
	
}

//close (get struct FD index from FILE * handle)
int close_r_fatfs ( struct _reent *r, int fd ){
	int ret = structfd_posixInvalidFileDirOrBufferHandle;
	struct fd *f = getStructFD(fd);
    if (f == NULL){
        errno = EBADF;
    }
    else if ((f->isused == structfd_isunused) || (!f->filPtr)){
        errno = EBADF;
    }
    else{
		ret = fatfs_close(f->cur_entry.d_ino);
    }
	return ret;
}

#endif
