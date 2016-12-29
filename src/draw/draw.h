/* SomaX - Copyright (C) 2005-2007 bakunin - Andrea Marchesini 
 *                                 <bakunin@autistici.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 * 02110-1301, USA.
 */

#ifndef __DRAW_H__
#define __DRAW_H__

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

GtkWidget *		draw_new	(void *expose_data,
					 gboolean (*button_press_func) (GtkWidget *, GdkEventButton *, gpointer), 
					 void *button_press_data,
					 gboolean resizable,
					 gboolean (*resize_func) (GtkWidget *, gpointer), 
					 void *resize_data);
GdkColor *		draw_color	(void);
GdkColor *		draw_color_str	(char *);
void			draw_refresh	(GtkWidget *draw,
					 struct somad_data *father,
					 gboolean queue_draw,
					 time_t show_time,
					 gboolean show_now);

GList *			draw_get_xy	(GtkWidget *draw,
					 int x,
					 int y,
					 struct somad_data *head);

gint			draw_get_time_x	(GtkWidget *draw);
gint			draw_get_time_y	(GtkWidget *draw);

#endif

/* EOF */
