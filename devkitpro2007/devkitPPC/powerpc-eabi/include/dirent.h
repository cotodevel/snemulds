#ifndef _DIRENT_H_
#define _DIRENT_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/dirent.h>

#if !defined(MAXNAMLEN) && !defined(_POSIX_SOURCE)
#define MAXNAMLEN 4096
#endif

#ifdef __cplusplus
}
#endif
#endif /*_DIRENT_H_*/
