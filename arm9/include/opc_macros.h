/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006-2007 archeide, All rights reserved. */
/***********************************************************/
/*
This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License as 
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.
*/

#ifndef OPC_MACROS_H_
#define OPC_MACROS_H_

#if 0
inline void	pushb(uint8 b)
{
	SNES.RAM[S] = b;
	S--;
}

inline void	pushw(uint16 w)
{
	S--;
	SET_WORD16(SNES.RAM+S, w);
	S--;
}

inline uint8	pullb()
{
	S++;
	return SNES.RAM[S];
}

inline uint16	pullw()
{
	uint16 w;
	
	S++;	
	w = GET_WORD16(SNES.RAM+S);
	S++;
	return w;
}

inline uchar   stack_getbyte(uint8 offset)
{
  return SNES.RAM[S+offset];
}

inline void	stack_setbyte(uint8 offset, uchar byte)
{
  SNES.RAM[S+offset] = byte;
}

inline ushort  stack_getword(uint8 offset)
{
  return GET_WORD16(SNES.RAM+S+offset);	
}

inline void  stack_setword(uint8 offset, uint16 word)
{
  SET_WORD16(SNES.RAM+S+offset, word);
}

inline uchar   direct_getbyte(uint32 offset)
{
  uint16 _offset = (uint16)(D+offset);	 
  if (_offset < 0x2000)
  {	
  	return SNES.RAM[_offset];
  }
  else
  	return mem_getbyte(_offset, 0);	
}

inline uchar   direct_getbyte2(uint32 offset)
{
  uint16 _offset = (uint16)(D+offset);	
  if (_offset+2 < 0x2000)
  {
  	return SNES.RAM[_offset+2];
  }
  else
  	return mem_getbyte(_offset+2, 0);	
}

inline void	direct_setbyte(uint32 offset, uchar byte)
{
  uint16 _offset = (uint16)(D+offset);	
  if (_offset < 0x2000)
  	SNES.RAM[_offset] = byte;
  else
  	mem_setbyte(_offset, 0, byte);	
}

inline ushort  direct_getword(uint32 offset)
{
  uint16 _offset = (uint16)(D+offset);	
  if (_offset < 0x2000)
  {
  	return GET_WORD16(SNES.RAM+_offset);
  }
  else
  	return mem_getword(_offset, 0);	
}

inline void  direct_setword(uint32 offset, uint16 word)
{
  uint16 _offset = (uint16)(D+offset);
  if (_offset < 0x2000)
  	SET_WORD16(SNES.RAM+_offset, word)
  else
  	mem_setword(_offset, 0, word);	
}

inline uint8 rol_b(uint8 a)
{
	uint16 t = a;
	t <<= 1;
	t |= F_C;
	F_C = (t >= 0x100);
	return (uint8)t;
}

inline uint16 rol_w(uint16 a)
{
	uint32 t = a;
	t <<= 1;
	t |= F_C;
	F_C = (t >= 0x10000);
	return (uint16)t;
}

inline uint8 ror_b(uint8 a)
{
	uint16 t = a;
	t |= F_C << 8;
	F_C = (t & 1);
	t >>= 1;		
	return (uint8)t;
}

inline uint16 ror_w(uint16 a)
{
	uint32 t = a;
	t |= F_C << 16;
	F_C = (t & 1);
	t >>= 1;
	return (uint16)t;
}
#endif

#define _TRB_(addr_mode) \
  if (P&P_M) { \
    uint8 opsrc = GETBYTE_##addr_mode; \
    SETBYTE_##addr_mode(~rA & opsrc); TESTB_1(rA & opsrc); \
  } else { \
    uint16 opsrc = GETWORD_##addr_mode; \
    SETWORD_##addr_mode(~rA & opsrc); TESTW_1(rA & opsrc); \
  } \
  UPDATEPC_##addr_mode

#define _TSB_(addr_mode) \
  if (P&P_M) { \
    uint8 opsrc = GETBYTE_##addr_mode;  \
    SETBYTE_##addr_mode(rA | opsrc); TESTB_1(rA & opsrc); \
  } else { \
    uint16 opsrc = GETWORD_##addr_mode;  \
    SETWORD_##addr_mode(rA | opsrc); TESTW_1(rA & opsrc); \
  } \
  UPDATEPC_##addr_mode
  
  // CHECK ME : ORA

#define _ORA_(addr_mode) \
  if (P&P_M) { \
    rA |= GETBYTE_##addr_mode; TESTB_2(rA); \
  } else { \
    rA |= GETWORD_##addr_mode; TESTW_2(rA); \
  } \
  UPDATEPC_##addr_mode;


#define _ASL_(addr_mode) \
  if (P&P_M) { \
    uint8 tmp; \
    tmp = GETBYTE_##addr_mode; F_C = (tmp&0x80) >> 7; \
    tmp <<= 1; SETBYTE_##addr_mode(tmp); TESTB_2(tmp); \
  } else { \
    uint16 tmp;   \
    tmp = GETWORD_##addr_mode; F_C = (tmp&0x8000) >> 15; \
    tmp <<= 1; SETWORD_##addr_mode(tmp); TESTW_2(tmp); \
  } \
  UPDATEPC_##addr_mode;

#define _INC_(addr_mode) \
  if (P&P_M) { \
    uint8 tmp = GETBYTE_##addr_mode+1; \
    SETBYTE_##addr_mode(tmp); TESTB_2(tmp); \
  } else { \
    uint16 tmp = GETWORD_##addr_mode+1; \
    SETWORD_##addr_mode(tmp); TESTW_2(tmp); \
  } \
  UPDATEPC_##addr_mode;

#define _LSR_(addr_mode) \
  if (P&P_M) { \
    uint8 tmp; \
    tmp = GETBYTE_##addr_mode; F_C = (tmp&0x01); \
    tmp >>= 1; SETBYTE_##addr_mode(tmp); TESTB_2(tmp); \
  } else { \
    uint16 tmp;   \
    tmp = GETWORD_##addr_mode; F_C = (tmp&0x0001); \
    tmp >>= 1; SETWORD_##addr_mode(tmp); TESTW_2(tmp); \
  } \
  UPDATEPC_##addr_mode;

#define _AND_(addr_mode) \
  if (P&P_M) { \
    rA = (rA&0xff00)|((rA & GETBYTE_##addr_mode)); TESTB_2(rA); \
  } else { \
    rA = rA & GETWORD_##addr_mode; TESTW_2(rA); \
  } \
  UPDATEPC_##addr_mode;

#define _BIT_(addr_mode) \
  if (P&P_M) {\
    uint8 opsrc = GETBYTE_##addr_mode; \
    F_Z = (opsrc&rA); \
    F_V = (opsrc&0x40) != 0; \
    F_N = (opsrc&0x80); \
  } else { \
    uint16 opsrc = GETWORD_##addr_mode; \
    F_Z = (opsrc&rA); \
    F_V = (opsrc&0x4000) != 0; \
    F_N = (opsrc&0x8000) >> 8; \
  } \
  UPDATEPC_##addr_mode;

#define _ROL_(addr_mode) \
  if (P&P_M) { \
    uint8 tmp = rol_b(GETBYTE_##addr_mode); \
    SETBYTE_##addr_mode(tmp); TESTB_2(tmp);\
  } else { \
    uint16 tmp = rol_w(GETWORD_##addr_mode); \
    SETWORD_##addr_mode(tmp); TESTW_2(tmp);\
  } \
  UPDATEPC_##addr_mode;

#define _DEC_(addr_mode) \
  if (P&P_M) { \
    uint8 tmp = GETBYTE_##addr_mode-1; \
    SETBYTE_##addr_mode(tmp); TESTB_2(tmp); \
  } else { \
    uint16 tmp = GETWORD_##addr_mode-1; \
    SETWORD_##addr_mode(tmp); TESTW_2(tmp);\
  } \
  UPDATEPC_##addr_mode;
  
#define _STZ_(addr_mode) \
  if (P&P_M) \
    SETBYTE_##addr_mode(0); \
  else \
    SETWORD_##addr_mode(0); \
   UPDATEPC_##addr_mode;    
  

#define _ADC_(addr_mode) \
  if (P&P_M) { \
    ADC8(GETBYTE_##addr_mode); \
  } else { \
	ADC16(GETWORD_##addr_mode); \
  } \
  UPDATEPC_##addr_mode
  
#define _SBC_(addr_mode) \
  if (P&P_M) { \
    SBC8(GETBYTE_##addr_mode); \
  } else { \
	SBC16(GETWORD_##addr_mode); \
  } \
  UPDATEPC_##addr_mode; 
  
  
#define _ROR_(addr_mode) \
  if (P&P_M) { \
    uint8 tmp = ror_b(GETBYTE_##addr_mode); \
    SETBYTE_##addr_mode(tmp); TESTB_2(tmp);\
  } else { \
    uint16 tmp = ror_w(GETWORD_##addr_mode); \
    SETWORD_##addr_mode(tmp); TESTW_2(tmp);\
  } \
  UPDATEPC_##addr_mode;

#define _STX_(addr_mode) \
  if (P&P_X) { \
    SETBYTE_##addr_mode(X); \
  } else { \
    SETWORD_##addr_mode(X); \
  } \
  UPDATEPC_##addr_mode;

#define _STY_(addr_mode) \
  if (P&P_X) { \
    SETBYTE_##addr_mode(Y); \
  } else { \
    SETWORD_##addr_mode(Y); \
  } \
  UPDATEPC_##addr_mode;

#define _STA_(addr_mode) \
  if (P&P_M) { \
    SETBYTE_##addr_mode(rA); \
  } else { \
    SETWORD_##addr_mode(rA); \
  } \
  UPDATEPC_##addr_mode;
  
#define _LDX_(addr_mode) \
  if (P&P_X) { \
    X = (X&0xff00) | GETBYTE_##addr_mode; TESTB_2(X);\
  } else { \
    X = GETWORD_##addr_mode; TESTW_2(X);\
  } \
  UPDATEPC_##addr_mode;

#define _LDY_(addr_mode) \
  if (P&P_X) { \
    Y = (Y&0xff00) | GETBYTE_##addr_mode; TESTB_2(Y);\
  } else { \
    Y = GETWORD_##addr_mode; TESTW_2(Y);\
  } \
  UPDATEPC_##addr_mode;

#define _LDA_(addr_mode) \
  if (P&P_M) { \
    rA = (rA&0xff00) | GETBYTE_##addr_mode; TESTB_2(rA);\
  } else { \
    rA = GETWORD_##addr_mode; TESTW_2(rA);\
  } \
  UPDATEPC_##addr_mode;
  
#define _CMP_(addr_mode) \
  if (P&P_M) { \
    TESTB_3(rA, GETBYTE_##addr_mode); \
  } else { \
    TESTW_3(rA, GETWORD_##addr_mode); \
  } \
  UPDATEPC_##addr_mode;

#define _CMP_M_(addr_mode) \
  TESTB_3(rA, GETBYTE_##addr_mode); \
  UPDATEPC_##addr_mode;

#define _CMP_m_(addr_mode) \
  TESTW_3(rA, GETWORD_##addr_mode); \
  UPDATEPC_##addr_mode;
  


#define _CPX_(addr_mode) \
  if (P&P_X) { \
    TESTB_3(X, GETBYTE_##addr_mode); \
  } else { \
    TESTW_3(X, GETWORD_##addr_mode); \
  } \
  UPDATEPC_##addr_mode;

#define _CPY_(addr_mode) \
  if (P&P_X) { \
    TESTB_3(Y, GETBYTE_##addr_mode); \
  } else { \
    TESTW_3(Y, GETWORD_##addr_mode); \
  } \
  UPDATEPC_##addr_mode;
  
#define _EOR_(addr_mode) \
  if (P&P_M) { \
    rA = rA ^ GETBYTE_##addr_mode; TESTB_2(rA); \
  } else { \
    rA = rA ^ GETWORD_##addr_mode; TESTW_2(rA); \
  } \
  UPDATEPC_##addr_mode;
      

#endif /*OPC_MACROS_H_*/
