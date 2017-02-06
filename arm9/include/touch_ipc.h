
#ifndef GBAEMU4DS_ARM9TOUCH
#define GBAEMU4DS_ARM9TOUCH

#include <nds.h>
#include <nds/touch.h>

#endif


#ifdef __cplusplus
extern "C"{
#endif

extern void touchRead_customIPC(touchPosition * touchpos_inst);
extern u32 keyscurr_ipc();

#ifdef __cplusplus
}
#endif
