#ifndef __main7_h__
#define __main7_h__

typedef void (*type_void)();
#include "spcdefs.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

// Play buffer, left buffer is first MIXBUFSIZE * 2 uint16's, right buffer is next
extern uint16 *playBuffer;
extern volatile int soundCursor;
extern bool paused;
extern bool SPC_disable;
extern void SetupSoundSPC();
extern void StopSoundSPC();
extern void LoadSpc(const uint8 *spc);

#ifdef __cplusplus
}
#endif

