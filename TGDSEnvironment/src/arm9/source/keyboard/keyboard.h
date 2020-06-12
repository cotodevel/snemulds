#ifndef __keyboard9_h__
#define __keyboard9_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "ipcfifoTGDS.h"

#define KB_NORMAL 0
#define KB_CAPS   1
#define KB_SHIFT  2

#define ECHO_ON	 0
#define ECHO_OFF 1

#define BSP	0x8		// Backspace
#define CAP	0x2		// Caps
#define RET	'\n'	// Enter
#define SHF	0x4		// Shift

// comment out the line below to remove the custom pen jump detection code
//#define USE_CUSTOM_PEN_JUMP_DETECTION_CODE	1

#define F_1	0x3c
#define F_2	0x3d
#define F_3	0x3e
#define F_4	0x3f
#define F_5	0x40
#define F_6	0x41
#define F_7	0x42
#define F_8	0x43
#define F_9	0x44
#define F10	0x45
#define F11	0x46
#define F12	0x47

#define EXT	0x1		// Exit

#define HOM	0x47	// Home
#define PGU	0x49	// Page Up
#define PGD	0x51	// Page Down
#define END	0x4f	// End

#define TAB	'\t'	// Tab

#define ESC	0x1b	// Escape
#define BSP	0x8		// Backspace
#define CAP	0x2		// Caps
#define RET	'\n'	// Enter
#define SHF	0x4		// Shift
#define	CTL	0x1d	// Ctrl
#define SPC	0x20	// Space
#define ALT	0x38	// Alt
#define NDSKEY	0x4a	// DS
#define SCN	0x46	// Screen

#define CRU	0x48	// Cursor up
#define CRD	0x50	// Cursor down
#define CRL	0x4b	// Cursor Left
#define CRR	0x4d	// Cursor Right

#define INS	0x52	// Insert
#define DEL	0x53	// Delete

#define TILE_OFFSET_Y 11	// 11 tiles from the top

#define PEN_DOWN (~ (TGDSIPC->buttons7) & (1 << 6))

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const unsigned char keyboard_Hit[12][32];
extern const unsigned char keyboard_Hit_Shift[12][32];

extern void setTile(uint16 *map, int x, int y, int pal);
extern void initKeyboard();
extern char processKeyboard(char* str, unsigned int max, unsigned int echo, int strYoffset);

#ifdef __cplusplus
}
#endif
