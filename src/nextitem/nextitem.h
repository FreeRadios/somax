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

#ifndef __NEXTITEM_H__
#define __NEXTITEM_H__

#include <gtk/gtk.h>
#include <soma/code.h>
#include <soma/commons.h>
#include <soma/controller.h>

GtkWidget *	nextitem_new			(soma_controller *controller,
						 gboolean local);
void		nextitem_add_stream		(void);
void		nextitem_add_l_directory	(void);
void		nextitem_add_l_file		(void);
void		nextitem_add_r_directory	(void);
void		nextitem_add_r_file		(void);
void		nextitem_show			(void);
void		nextitem_refresh		(void);
void		nextitem_timer_refresh		(char **, int);

#endif

/* EOF */
