#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

/* PSP dirent.h, based on pspiofilemgr_dirent.h
   by Jim Paris <jim@jtan.com> */

#ifdef __cplusplus
extern "C" {
#endif
	
#include <pspiofilemgr_dirent.h> 
	
#define MAXNAMLEN 255
	
/* Same as SceIoDirent, just with a different name. */
struct dirent {
	/** File status. */
	SceIoStat 	d_stat;
	/** File name. */
	char 		d_name[256];
	/** Device-specific data. */
	void * 		d_private;
	int 		dummy;
};

typedef struct {
	SceUID		uid;
	struct dirent	de;
} DIR;

DIR *opendir (const char *);
struct dirent *readdir (DIR *);
int closedir (DIR *);

#ifdef __cplusplus
}
#endif

#endif
