#include <nds.h>
#include <nds/registers_alt.h>

#define SERIAL_CR REG_SPICNT
#define SERIAL_DATA REG_SPIDATA

/*
#define DISP_SR   (*(vuint16*)0x04000004)
#define DISP_IN_VBLANK   BIT(0)
#define DISP_IN_HBLANK   BIT(1)
#define DISP_YTRIGGERED   BIT(2)
#define DISP_VBLANK_IRQ   BIT(3)
#define DISP_HBLANK_IRQ   BIT(4)
#define DISP_YTRIGGER_IRQ BIT(5)
*/

#include "mixrate.h"

#define DTCM_IRQ_HANDLER	(*(VoidFunctionPointer *)0x02803FFC)
#define BIOS_INTR_ACK       (*(vu32*)0x02803FF8)

#define ALIGNED __attribute__ ((aligned(4)))
#define CODE_IN_ITCM __attribute__ ((section (".itcm")))
//#define VAR_IN_ITCM __attribute__ ((section (".itcm")))
#define VAR_IN_DTCM __attribute__ ((section (".dtcm")))
#define ALIGNED_VAR_IN_DTCM __attribute__ ((aligned(4),section (".dtcm")))
