#include <nds.h>
#include "mixrate.h"

#define DTCM_IRQ_HANDLER	(*(VoidFunctionPointer *)0x02803FFC)
#define BIOS_INTR_ACK       (*(vu32*)0x02803FF8)
#define ALIGNED __attribute__ ((aligned(4)))
#define CODE_IN_ITCM __attribute__ ((section (".itcm")))
#define VAR_IN_DTCM __attribute__ ((section (".dtcm")))
#define ALIGNED_VAR_IN_DTCM __attribute__ ((aligned(4),section (".dtcm")))
