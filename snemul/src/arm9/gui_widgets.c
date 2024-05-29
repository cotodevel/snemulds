#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "common.h"
#include "gui.h"
#include "gui_widgets.h"

#include "snemul_str.h"

extern t_GUIMessage	PendingMessage;

int	GUIList_handler(t_GUIZone *zone, int message, int param, void *arg)
{
	t_GUIList *this = (t_GUIList *)zone->data;
	
	switch (message)
	{
	case GUI_DRAW:
	{
		int i;
		int gh = GUI_getFontHeight(zone)+1;	
		int height = GUI_getZoneTextHeight(zone);
		for (i = 0; i < height && this->first_item+i < this->nb_items; i++)
		{
			int c = param == 0 ? GUI_BLACK : -1;
			if (i == this->prev_item)
				c = GUI_BLACK;
			if (i == this->cur_item)
				c = GUI_LIGHTRED;
			if (c >= 0)
				GUI_drawBar(zone, c, 0, i*gh, 
							zone->x2-zone->x1, (i+1)*gh);
			GUI_drawText(zone, 0, i*gh, GUI_WHITE, this->items[this->first_item+i]);
		}
		return 1;
	}
	case GUI_EVENT:
	{
		t_GUIEvent	*ev = (t_GUIEvent *)arg;
		if (param == GUI_EVENT_STYLUS)
		{
			int item = (ev->stl.y - zone->y1) / (GUI_getFontHeight(zone)+1);
			
			if (this->first_item+item >= this->nb_items)
				return 1;
			
			this->prev_item = this->cur_item;
			this->cur_item = item;
			zone->handler(zone, GUI_DRAW, 1, 0);
			return 1;
		}
		
		if (param == GUI_EVENT_BUTTON)
		{
			if (ev->joy.repeated & KEY_UP)
			{
				if (this->first_item+this->cur_item == 0)
					return 1;				
				this->prev_item = this->cur_item;
				if (this->cur_item == 0)
				{
					this->first_item--;
					GUI_dispatchMessage(GUI.screen, GUI_LIST_CHANGED, this->first_item, 0);
					zone->handler(zone, GUI_DRAW, 0, 0);
				}
				else
				{
					this->cur_item--;
					zone->handler(zone, GUI_DRAW, 1, 0);
				}
				return 1;
			}
			if (ev->joy.repeated & KEY_DOWN)
			{
				if (this->first_item+this->cur_item == this->nb_items-1)
					return 1;
				this->prev_item = this->cur_item;				
				if (this->cur_item == GUI_getZoneTextHeight(zone) - 1)
				{
					this->first_item++;
					GUI_dispatchMessage(GUI.screen, GUI_LIST_CHANGED, this->first_item, 0);
					zone->handler(zone, GUI_DRAW, 0, 0);
				}
				else
				{
					this->cur_item++;
					zone->handler(zone, GUI_DRAW, 1, 0);
				}
				return 1;
			}			
		}
		
	}
	break;
	case GUI_SCROLLED:
	{
		this->first_item = param;
		zone->handler(zone, GUI_DRAW, 0, 0);
		return 1;
	}
	}
	return 0;
}

int	GUIScrollBar_handler(t_GUIZone *zone, int message, int param, void *arg)
{
	t_GUIScrollBar *this = (t_GUIScrollBar *)zone->data;
	
	switch (message)
	{
	case GUI_DRAW:
	{
		GUI_drawBar(zone, GUI_DARKGREY, 0, 0, 32, zone->y2-zone->y1);
		GUI_drawBar(zone, GUI_GREY, 0, this->value * (zone->y2 - zone->y1) / this->max, 
						32, (this->value+this->range) * (zone->y2-zone->y1) / this->max);
		return 1;
	}
	break;
	case GUI_EVENT:
	{
		if (param == GUI_EVENT_STYLUS)
		{
			t_GUIEvent	*ev = (t_GUIEvent *)arg;
			switch (ev->event)
			{
			case EVENT_STYLUS_PRESSED:
			{
				int value, pos;
				
				pos = ev->stl.y-zone->y1;
				value = pos * this->max / (zone->y2-zone->y1);
				
				this->pos = -1;
				if (pos >= this->value * (zone->y2 - zone->y1) / this->max &&
					pos <= (this->value+this->range) * (zone->y2 - zone->y1) / this->max)
				{
					this->pos = pos;
					this->drag_pos = pos - this->value * (zone->y2 - zone->y1) / this->max;
					return 1;
				}
				
				if (value < 0)
					value = 0;
				if (value > this->max-this->range)
					value = this->max-this->range;
				if (this->value != value)
				{
					this->value = value;					
					//GUI_printf(0, 20, "%d %d | %d", value, this->pos); 
					zone->handler(zone, GUI_DRAW, 0, 0);				
					GUI_dispatchMessage(GUI.screen, GUI_SCROLLED, this->value, this);
				}
				return 1;
			}
			case EVENT_STYLUS_DRAGGED:
			{
				if (this->pos == -1)
					return 1;	
				
				int pos, value;
				pos = this->pos + ev->stl.dy;	
				if (pos < 0)
					return 0;
				value = (pos - this->drag_pos) * this->max / (zone->y2 - zone->y1);
				if (value < 0 || value > this->max - this->range)
					return 0;
//				GUI_printf(0, 21, "%d %d %d | %d %d", value, this->pos, pos, this->max, this->range);				
				this->pos = pos;
				if (this->value != value)
				{
					this->value = value;
					zone->handler(zone, GUI_DRAW, 0, 0);
				
					GUI_dispatchMessage(GUI.screen, GUI_SCROLLED, this->value, this);
				}
				return 1;
			}
			}
		}
	}
	break;
	case GUI_LIST_CHANGED:
		this->value = param;
		zone->handler(zone, GUI_DRAW, 0, 0);
		return 1;
	}
	return 0;
}

int	GUIButton_handler(t_GUIZone *zone, int message, int param, void *arg)
{
	int ret = 0;
	
	switch (message)
	{
	case GUI_DRAW:
	{
		int color_bar = (zone->state & GUI_ST_PRESSED) ? GUI_DARKGREY2 : GUI_DARKGREY;
		int color_rect = (zone->state & GUI_ST_FOCUSED) ? GUI_WHITE : GUI_GREY;
		GUI_drawBar(zone, color_bar, 0, 0, zone->x2-zone->x1, zone->y2-zone->y1);
		GUI_drawRect(zone, color_rect, 0, 0, zone->x2-zone->x1, zone->y2-zone->y1);
		return 1;
	}

	case GUI_EVENT:
	{
		if (zone->state & GUI_ST_DISABLED)
			return 0;
		
		t_GUIEvent	*ev = (t_GUIEvent *)arg;
		int			old_state = zone->state;
	
		switch (param)
		{
		case GUI_EVENT_BUTTON:
			if (ev->joy.pressed & zone->keymask)
			{
				zone->state |= GUI_ST_PRESSED;
				ret = 1;
			}
			if (ev->joy.released & zone->keymask)
			{
				zone->state &= ~GUI_ST_PRESSED;
				GUI_dispatchMessage(GUI.screen, GUI_COMMAND, zone->id, zone->data);
				ret = 1;
			}
			break;
			
		case GUI_EVENT_STYLUS:
			if (ev->event == EVENT_STYLUS_PRESSED || ev->event == EVENT_STYLUS_DRAGGED)
			{
				GUI_setFocus(GUI.screen, zone->id);
				zone->state |= GUI_ST_PRESSED;
			}
			if (ev->event == EVENT_STYLUS_RELEASED)
			{
				GUI_dispatchMessage(GUI.screen, GUI_COMMAND, zone->id, zone->data);
				zone->state &= ~GUI_ST_PRESSED;
			}
			ret = 1;
			break;
		case GUI_EVENT_FOCUS:
			zone->state |= GUI_ST_FOCUSED;			
			break;
		case GUI_EVENT_UNFOCUS:
			zone->state &= ~GUI_ST_PRESSED;
			zone->state &= ~GUI_ST_FOCUSED;			
			break;		
		case GUI_EVENT_ENTERZONE:
			GUI_setFocus(GUI.screen, zone->id);
			zone->state |= GUI_ST_PRESSED;
			break;				
		case GUI_EVENT_LEAVEZONE:
			zone->state &= ~GUI_ST_PRESSED;
			break;
		}

		if (old_state != zone->state)
			zone->handler(zone, GUI_DRAW, 0, 0);
	}	break;
	}
	return ret;
}

int	GUIStatic_handler(t_GUIZone *zone, int message, int param, void *arg)
{
	char *str;
	
	switch (message)
	{
	case GUI_DRAW:
		if ((int)zone->data < 1024)
			str = GUI.string[(int)zone->data];
		else
			str = zone->data; 			
		GUI_drawAlignText(zone, 0, (zone->y2-zone->y1)/2, GUI_WHITE, str);
		return 1;
	}
	return 0;
}

int	GUIStaticEx_handler(t_GUIZone *zone, int message, int param, void *arg)
{
	switch (message)
	{
	case GUI_DRAW:
	{
		int str = (int)zone->data & 0xFFFF;
		int str_arg = ((int)zone->data >> 16) & 0xFF;
		int flags = ((int)zone->data >> 24) & 0xFF;
		
		char buf[64];
		snprintf(buf, 64, GUI.string[str], str_arg); // FIXME
		GUI_drawAlignText(zone, flags, (zone->y2-zone->y1)/2, GUI_WHITE, buf);
		return 1;
	}
	}
	return 0;
}

int	GUIStrButton_handler(t_GUIZone *zone, int message, int param, void *arg)
{
	char *str;
	
	switch (message)
	{
	case GUI_DRAW:
		GUIButton_handler(zone, GUI_DRAW, param, arg);
		if ((int)zone->data < 1024)
			str = GUI.string[(int)zone->data];
		else
			str = zone->data; 
		GUI_drawAlignText(zone, 0, (zone->y2-zone->y1)/2, GUI_WHITE, str);
		return 1;
	case GUI_EVENT:
		return GUIButton_handler(zone, GUI_EVENT, param, arg);
	}
	return 0;
}

void GUI_linkStrButton(t_GUIScreen *scr, int nb, int str, int keymask)
{
	GUI_linkObject(scr, nb, str, GUIStrButton_handler);
	scr->zones[nb].keymask = keymask;
}

int	GUIChoiceButton_handler(t_GUIZone *zone, int message, int param, void *arg)
{
	t_GUIChoiceButton *cb = (t_GUIChoiceButton *)&zone->data;
	char	*str;

	switch (message)
	{
	case GUI_DRAW:
		GUIButton_handler(zone, GUI_DRAW, param, arg);
		
		//GUI_printf(0, 5, "%d %d %d", cb->str_index, cb->nb_state, cb->cur_state);
		
		str = GUI.string[cb->str_index+cb->cur_state];

		GUI_drawAlignText(zone, 0, (zone->y2-zone->y1)/2, GUI_WHITE, str);
		return 1;
	case GUI_EVENT:
	{
		int ret;
		ret = GUIButton_handler(zone, GUI_EVENT, param, arg);
		if (ret && PendingMessage.msg == GUI_COMMAND)
		{
			// Got a command
			cb->cur_state++;
			if (cb->cur_state >= cb->nb_state)
				cb->cur_state = 0;
			PendingMessage.arg = *(void **)cb;
			zone->handler(zone, GUI_DRAW, 0, 0);
		}
		return ret;
	}
	}
	return 0;
}

int	GUIImgButton_handler(t_GUIZone *zone, int message, int param, void *arg)
{
	switch (message)
	{
	case GUI_DRAW:
	{
		uint32 normal = ((uint32)zone->data) >> 16;
		uint32 pressed = ((uint32)zone->data) & 0xFFFF;
		
//		if (zone->state & (GUI_ST_PRESSED|GUI_ST_FOCUSED))
		if (zone->state & GUI_ST_PRESSED)		
			GUI_drawImage(zone, GUI.screen->img_list->img[pressed], 0, 0);
		else
			GUI_drawImage(zone, GUI.screen->img_list->img[normal], 0, 0);
		return 1;
	}
	case GUI_EVENT:
		return GUIButton_handler(zone, GUI_EVENT, param, arg);
	}
	return 0;
}

int	GUIImage_handler(t_GUIZone *zone, int message, int param, void *arg)
{
	switch (message)
	{
	case GUI_DRAW:
	{
		if ((uint32)zone->data < 1024)
		{
			GUI_drawImage(zone, GUI.screen->img_list->img[(uint32)zone->data], 0, 0);
		} else
		{
			GUI_drawImage(zone, zone->data, 0, 0);
		}
		return 1;
	}
	}
	return 0;
}

t_GUIScreen	*GUI_newSelector(int nb_items, char **items, int title, t_GUIFont *font)
{
	t_GUIScreen	*scr_select;
	
	scr_select = GUI_newScreen(5);
	GUI_setZone(scr_select, 0, 0, 32, 256-40, 32+128);
	GUI_setZone(scr_select, 1, 0, 0, 256, 32);
	GUI_setZone(scr_select, 2, 256-32, 32, 256, 32+128);
	GUI_setZone(scr_select, 3, 0, 192-20, 128, 192);
	GUI_setZone(scr_select, 4, 128, 192-20, 256, 192);

	int	i;
	for (i = 0; i < 5; i++)
		scr_select->zones[i].font = font;
	  
    t_GUIList *list = malloc(sizeof(t_GUIList));
    memset(list, 0, sizeof(t_GUIList));
	list->nb_items = nb_items;
	list->items = items;
	GUI_linkObject(scr_select, 0, list, GUIList_handler);

	if (nb_items > GUI_getZoneTextHeight(&scr_select->zones[0]))
	{
	t_GUIScrollBar *sb = malloc(sizeof(t_GUIScrollBar));
	memset(sb, 0, sizeof(t_GUIScrollBar));
	sb->range = GUI_getZoneTextHeight(&scr_select->zones[0]);
	sb->max = nb_items;
	GUI_linkObject(scr_select, 2, sb, GUIScrollBar_handler);
	}
	
	scr_select->curs = 0;
	
	GUI_linkObject(scr_select, 1, (void *)title, GUIStatic_handler);
	
	GUI_linkStrButton(scr_select, 3, IDS_OK, KEY_A);
	GUI_linkStrButton(scr_select, 4, IDS_CANCEL, KEY_B);
	
	return scr_select;
}

char *GUISelector_getSelected(t_GUIScreen *scr, int *index) 
{
	t_GUIList *list = scr->zones[0].data;

	if (index)
		*index = list->first_item+list->cur_item;
		
	if (list->cur_item == -1)
		return NULL;
	return list->items[list->first_item+list->cur_item];
}

void GUI_deleteSelector(t_GUIScreen *scr)
{
	t_GUIList *list = scr->zones[0].data;
	free(list->items);
	if (scr->zones[2].data)
		free(scr->zones[2].data);
	free(scr->zones[0].data);
	free(scr);
}

t_GUIScreen	*GUI_newList(int nb_items, int max_size, int title, t_GUIFont *font)
{
	t_GUIScreen	*scr_select;
	
	scr_select = GUI_newScreen(5);
	GUI_setZone(scr_select, 0, 0, 32, 256, 32+128); // List
	GUI_setZone(scr_select, 1, 0, 0, 256, 32); // Static
	GUI_setZone(scr_select, 3, 0, 192-20, 128, 192); // Ok button
	GUI_setZone(scr_select, 4, 128, 192-20, 256, 192); // Cancel button
	  
	int i;
	for (i = 0; i < 5; i++)
		scr_select->zones[i].font = font;
	  
	char **items = malloc(nb_items*sizeof(char *) + nb_items*max_size);
	
	for (i = 0; i < nb_items; i++)
		items[i] = (char *)items + nb_items*sizeof(char *) + i*max_size;
	
	// List
    t_GUIList *list = malloc(sizeof(t_GUIList));
    memset(list, 0, sizeof(t_GUIList));
	list->nb_items = nb_items;
	list->items = items;
	GUI_linkObject(scr_select, 0, list, GUIList_handler);
	
	scr_select->curs = 0;

	GUI_linkObject(scr_select, 1, (void *)title, GUIStatic_handler);
	
	GUI_linkStrButton(scr_select, 3, IDS_OK, KEY_A);
	GUI_linkStrButton(scr_select, 4, IDS_CANCEL, KEY_B);
	
	return scr_select;
}

void	GUI_setItem(t_GUIScreen *scr, int i, char *s, int max_size)
{
	t_GUIList *list = scr->zones[0].data;
	strncpy(list->items[i], s, max_size);
}

void GUI_deleteList(t_GUIScreen *scr)
{
	t_GUIList *list = scr->zones[0].data;
	free(list->items);
	free(scr->zones[0].data);
	free(scr);
}

