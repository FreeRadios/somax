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

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
# error Use configure; make; make install
#endif

#include "commons.h"

gint statusbar_id;
GtkWidget *statusbar;

void
statusbar_set (char *what, ...)
{
  va_list va;
  char s[1024];
  int ret;

  va_start (va, what);

  ret = vsnprintf (s, sizeof (s), what, va);

  while (s[ret] == '\n' || s[ret] == '\r')
    ret--;

  s[ret] = 0;

  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), statusbar_id);
  statusbar_id =
    gtk_statusbar_push (GTK_STATUSBAR (statusbar), statusbar_id, s);

  va_end (va);
}

/* EOF */
