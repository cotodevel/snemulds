#include "guiTGDS.h"

#ifndef __GUI_WIDGETS_H__
#define __GUI_WIDGETS_H__

/* ---------------------------- Widgets --------------------------------- */

#define GUI_SCROLLED		10000
#define GUI_LIST_CHANGED	10001

typedef sint8 t_GUIString[96];

typedef struct
{
	int		nb_items;
	int		first_item;
	int		cur_item;
	int		prev_item;
	//t_GUIString	*items;
	sint8	**items;
} t_GUIList;

int GUIList_handler(t_GUIZone *zone, int message, int param, void *arg);

typedef struct
{
	int	max;		// Maximum value (value + size < max)
	int	range;		// Selected range of value 
	int value;		// Current value
	int	pos;		// Graphical position of the cursor
	int drag_pos;	// Point where the cursor is dragged
	
// size of cursor = size * height / max	
// pos of cursor = value * (height / max)
// value = pos * max / height
//
} t_GUIScrollBar;

typedef struct
{
	uint16	str_index; // Index of first string
	uint8	nb_state; // Number of states
	uint8	cur_state; // Current state
} t_GUIChoiceButton;

#define GUI_CHOICE(s, n, c)	(void *)((s) | ((n) << 16) | ((c) << 24))

#define GUI_GET_CHOICE(scr, i) ((int)(scr)->zones[i].data >> 24)
#define GUI_SET_CHOICE(scr, i, v) \
	(scr)->zones[i].data = (void *)(((int)(scr)->zones[i].data & 0x00FFFFFF) | ((v) << 24))

#define GUI_STATIC_CENTER(s, p)	GUI_PARAM3(s, p, 0)
#define GUI_STATIC_LEFT(s, p)	GUI_PARAM3(s, p, 1)
#define GUI_STATIC_RIGHT(s, p)	GUI_PARAM3(s, p, 2)

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int GUIScrollBar_handler(t_GUIZone *zone, int message, int param, void *arg);
extern int GUIButton_handler(t_GUIZone *zone, int message, int param, void *arg);
extern int GUIImgButton_handler(t_GUIZone *zone, int message, int param, void *arg);
extern int GUIStrButton_handler(t_GUIZone *zone, int message, int param, void *arg);
extern void GUI_linkStrButton(t_GUIScreen *scr, int nb, int str, int keymask);
extern int	GUIImage_handler(t_GUIZone *zone, int message, int param, void *arg);
extern int GUIStatic_handler(t_GUIZone *zone, int message, int param, void *arg);
extern int GUIStaticEx_handler(t_GUIZone *zone, int message, int param, void *arg);
extern int GUIChoiceButton_handler(t_GUIZone *zone, int message, int param, void *arg);
extern t_GUIScreen	*GUI_newSelector(int nb_items, sint8 **items, int title, t_GUIFont *font);
extern sint8 *GUISelector_getSelected(t_GUIScreen *scr, int *index);
extern void GUI_deleteSelector(t_GUIScreen *scr);
extern t_GUIScreen	*GUI_newList(int nb_items, int max_size, int title, t_GUIFont *font);
extern void	GUI_setItem(t_GUIScreen *scr, int i, sint8 *s, int max_size);
extern void GUI_deleteList(t_GUIScreen *scr);
extern t_GUIMessage	PendingMessage;

#ifdef __cplusplus
}
#endif
