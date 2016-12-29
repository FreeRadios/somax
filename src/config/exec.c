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

#include "cfg.h"

void
run_somax (GtkWidget * w, gpointer dummy)
{
  gchar *a[2];
  GPid pid;

  a[0] = X;
  a[1] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

void
run_editor (GtkWidget * w, gpointer dummy)
{
  gchar *a[2];
  GPid pid;

  a[0] = EDITOR;
  a[1] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

void
run_nextitem (GtkWidget * w, gpointer dummy)
{
  gchar *a[2];
  GPid pid;

  a[0] = NEXTITEM;
  a[1] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

/* EOF */
