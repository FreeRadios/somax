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

#ifndef __PLUGINS_H__
#define __PLUGINS_H__

#include "../commons/commons.h"
#include "../commons/splash.h"

struct plugin_t
{
  gchar *name;
  gchar *author;
  gchar *description;
  gchar *version;
  gchar *licence;

  gchar *file;
};

GtkWidget *	plugin_new		(void *);

void		plugin_scanner		(void);

void		plugin_starter		(struct plugin_t *,
					 char *);

void		plugin_about 		(void);

#endif

/* EOF */
