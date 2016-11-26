//coto: created because we re-implement some opcodes.

#include <nds.h>
#include <nds/memory.h>
#include <nds/system.h>
#include <unistd.h>


#ifdef __cplusplus
extern "C" {
#endif

extern int memcpy_imm;
extern u8 iplRom[64];

//ARM asm opcodes 
u32 ldsbasm_arm(u32 x, u32 y);
u32 ldshasm_arm(u32 x, u32 y);
u32 ldru32extasm_arm(u32 x, u32 y);
u32 ldru16extasm_arm(u32 x, u32 y);
u32 ldru8extasm_arm(u32 x, u32 y);
u32 stru32extasm_arm(u32 x, u32 y);
u32 stru16extasm_arm(u32 x, u32 y);
u32 stru8extasm_arm(u32 x, u32 y);

u32 rorasm(u32 x, u16 y);

//THUMB asm opcodes
u32 ldsbasm_tmb(u32 x, u32 y);
u32 ldshasm_tmb(u32 x, u32 y);
u32 ldru32extasm_tmb(u32 x, u32 y);
u32 ldru16extasm_tmb(u32 x, u32 y);
u32 ldru8extasm_tmb(u32 x, u32 y);
u32 stru32extasm_tmb(u32 x, u32 y);
u32 stru16extasm_tmb(u32 x, u32 y);
u32 stru8extasm_tmb(u32 x, u32 y);

void ApuSetShowRom();


#ifdef __cplusplus
}
#endif