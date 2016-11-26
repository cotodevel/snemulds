#include <nds.h>
#include <nds/memory.h>
#include <nds/system.h>
#include <unistd.h>


#ifdef __cplusplus
extern "C" {
#endif

extern int memcpy_imm;
extern u8 iplRom[64];
extern u8 apuShowRom;
extern void ApuSetShowRom();

#ifdef __cplusplus
}
#endif