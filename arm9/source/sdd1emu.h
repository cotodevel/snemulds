#ifndef SDD1EMU_H
#define SDD1EMU_H

#include "typedefsTGDS.h"

#ifdef __cplusplus
extern "C"{
#endif

extern void SDD1_decompress(uint8 *out, uint8 *in, int output_length);
extern uint8 ProbGetBit(uint8 context);
extern uint8 GetCodeword(int bits);
extern uint8 GetBit(uint8 cur_bitplane);
extern uint8 run_table[128];

#ifdef __cplusplus
}
#endif

#endif
