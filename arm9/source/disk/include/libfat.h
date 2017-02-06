#ifndef libfat_snemulds
#define libfat_snemulds

#include "fatfile.h"
#include "fatdir.h"




#endif

#ifdef __cplusplus
extern "C" {
#endif

extern bool fatMount (const char* name, const DISC_INTERFACE* interface, sec_t startSector, uint32_t cacheSize, uint32_t SectorsPerPage);
extern bool fatMountSimple (const char* name, const DISC_INTERFACE* interface);
extern void fatUnmount (const char* name);
extern bool fatInit (uint32_t cacheSize, bool setAsDefaultDevice);
extern bool fatInitDefault (void);
extern void fatGetVolumeLabel (const char* name, char *label);

extern const devoptab_t dotab_fat;
#ifdef __cplusplus
}
#endif
