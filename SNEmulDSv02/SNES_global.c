/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include "snes.h"
#include "cpu.h"
#include "apu.h"
#include "gfx.h"
#include "cfg.h"

IN_DTCM
struct s_cpu	CPU;
struct s_apu	APU;
struct s_gfx	GFX;
struct s_cfg	CFG;
struct s_snes	SNES;
IN_DTCM
struct s_snescore	SNESC;
