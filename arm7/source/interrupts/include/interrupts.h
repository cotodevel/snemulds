//these just extend libnds interrupt libraries
#ifndef nds_interrupt7_headers
#define nds_interrupt7_headers

#include <nds.h>
#include <nds/interrupts.h>
#include <nds/system.h>
#include <nds/ipc.h>

#ifdef ARM7
#include <nds/arm7/i2c.h>
#endif


#endif

#ifdef __cplusplus
extern "C"{
#endif

//external (usermode)
extern void hblank();
extern void vblank();
extern void vcounter();
extern void timer1();

//internal code (kernel)
extern void IntrMainExt();

//libnds
extern void irqDummy(void);
extern struct IntTable irqTable[MAX_INTERRUPTS];

#ifdef ARM7
extern void i2cIRQHandler();
extern bool isDSiMode();
#endif

#ifdef INT_TABLE_SECTION
#else
extern struct IntTable irqTable[MAX_INTERRUPTS];
#endif
extern void irqInitExt(IntFn handler);


#ifdef __cplusplus
}
#endif
