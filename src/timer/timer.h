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

#ifndef __TIMER_H__
#define __TIMER_H__

#include <gtk/gtk.h>

GtkWidget *	timer_new		(void);

void		timer_set		(GtkWidget *timer,
					 time_t current_timer,
					 int64_t end_timer);

void		timer_update		(GtkWidget *timer);

void		timer_clear		(GtkWidget *timer);

GtkWidget *	timer_preferences	(void);
gboolean	timer_preferences_check	(GtkWidget *);

#endif

/* EOF */
