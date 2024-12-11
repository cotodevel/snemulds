#ifndef __main7_h__
#define __main7_h__

typedef void (*type_void)();
#include "spcdefs.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern int sampleRateDivider;
extern volatile int soundCursor;
extern bool paused;
extern bool SPC_disable;
extern void SetupSoundSPC();
extern void StopSoundSPC();
extern void LoadSpc(const uint8 *spc);
extern int PocketSPCVersion;

#ifdef __cplusplus
}
#endif

