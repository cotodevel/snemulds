@ The APU state saved into a small block of contiguous memory for quick loading
.EXTERN     APU_STATE
@ Pointer to APU memory, 0x10000 bytes, then read mem table, 0x800 bytes, then write mem table 0x800, then extra mem 0x40
.EXTERN     APU_MEM

@ r0, r1, r2 - temp

#define APU_REG_A r3
#define APU_REG_X r4
#define APU_REG_Y r5
#define APU_RAMBASE r6
#define APU_DP r7
@ PC is adjusted to be pointing into ram already
#define APU_PC r8
@ bit 0 - C, bit 1 - V, bit 2- H, bits 3+ cycles left
#define APU_CYCLES r9
#define APU_OPTABLE r10
#define APU_NZ r11

@ r12 - temp
@ r13 - sp
@ r14 - lr
@ r15 - pc

#define CYCLES_C_FLAG 1
#define CYCLES_V_FLAG 2
#define CYCLES_H_FLAG 4

#define CYCLE_SHIFT 3           @ Amount to shift cycle amounts before adding/subtracting them

#define ONE_APU_CYCLE 21

#define CheckIntrLoop 0
