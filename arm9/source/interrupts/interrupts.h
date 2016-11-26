#include <nds.h>

#ifdef __cplusplus
extern "C"{
#endif

//r0        0=Return immediately if an old flag was already set (NDS9: bugged!)
//          1=Discard old flags, wait until a NEW flag becomes set
//r1        Interrupt flag(s) to wait for (same format as IE/IF registers)
extern void IntrHandlerAsm();
extern void InterruptHandler(void);

extern void vcounter();
extern void Vblank();
extern void Hblank();

#ifdef __cplusplus
}
#endif
