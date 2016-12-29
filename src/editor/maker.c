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

#include "editor.h"

struct editor_data_t *
editor_get_data (void)
{
  struct editor_data_t *data;
  gint page;

  page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  data = editor_data;

  while (data)
    {
      if (!page)
	return data;

      page--;

      data = data->next;
    }

  return NULL;
}

gint
editor_timeout (gpointer dummy)
{
  struct editor_data_t *data;
  gboolean undo, redo;
  static int loop = 0;

  undo = redo = FALSE;

  data = editor_get_data ();

  if (data && data->type == TYPE_PALINSESTO)
    maker_pl_undoredo_status (data->pl, &undo, &redo);
  else if (data && data->type == TYPE_SPOT)
    maker_spot_undoredo_status (data->spot, &undo, &redo);

  if(undo) {
  gtk_widget_set_sensitive (undo_widget, TRUE);
  gtk_widget_set_sensitive (undo_menu, TRUE);
  gtk_widget_set_sensitive (undo_history, TRUE);
  } else {
  gtk_widget_set_sensitive (undo_widget, FALSE);
  gtk_widget_set_sensitive (undo_menu, FALSE);
  gtk_widget_set_sensitive (undo_history, FALSE);
  }

  if(redo) {
  gtk_widget_set_sensitive (redo_widget, TRUE);
  gtk_widget_set_sensitive (redo_menu,TRUE);
  gtk_widget_set_sensitive (redo_history, TRUE);
  } else {
  gtk_widget_set_sensitive (redo_widget, FALSE);
  gtk_widget_set_sensitive (redo_menu,FALSE);
  gtk_widget_set_sensitive (redo_history, FALSE);
  }

  if (loop == 4)
    loop = 0;
  else
    {
      loop++;
      return TRUE;
    }

  switch (editor_statusbar_lock)
    {
    case LOCK:
      return TRUE;
    case WAIT:
      editor_statusbar_lock = UNLOCK;
      return TRUE;
    }

  if (!data
      || (data->type == TYPE_SPOT && !maker_spot_is_no_saved (data->spot))
      || (data->type == TYPE_PALINSESTO && !maker_pl_is_no_saved (data->pl)))
    {
      statusbar_set (_("Editing..."));
    }

  return TRUE;
}

char *
editor_parse_file (char *file)
{
  int i;

  for (i = strlen (file); i; i--)
    {
      if (file[i] == '/')
	return file + i + 1;
    }

  return file;
}

void
editor_new_maker_pl (struct maker_pl_data_t *data)
{
  struct editor_data_t *tmp;
  gint page;

  if (editor_data)
    {
      tmp = editor_data;
      while (tmp->next)
	tmp = tmp->next;

      tmp->next = g_malloc (sizeof (struct editor_data_t));
      tmp = tmp->next;
    }

  else
    editor_data = tmp = g_malloc (sizeof (struct editor_data_t));

  tmp->next = NULL;
  tmp->pl = data;
  tmp->type = TYPE_PALINSESTO;

  tmp->maker = create_maker_pl (data, controller, TRUE);
  gtk_widget_show (tmp->maker);

  if (data->maker_pl_file)
    tmp->label = gtk_label_new (editor_parse_file (data->maker_pl_file));
  else
    tmp->label = gtk_label_new (_("New Palinsesto"));

  gtk_widget_show (tmp->label);

  page =
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), tmp->maker,
			      tmp->label);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page);
  statusbar_set (_("New palinsesto layer"));
}

void
editor_new_maker_spot (struct maker_spot_data_t *data)
{
  struct editor_data_t *tmp;
  gint page;

  if (editor_data)
    {
      tmp = editor_data;
      while (tmp->next)
	tmp = tmp->next;

      tmp->next = g_malloc (sizeof (struct editor_data_t));
      tmp = tmp->next;
    }

  else
    editor_data = tmp = g_malloc (sizeof (struct editor_data_t));

  tmp->next = NULL;
  tmp->spot = data;
  tmp->type = TYPE_SPOT;

  tmp->maker = create_maker_spot (data, controller, TRUE);
  gtk_widget_show (tmp->maker);

  if (data->maker_spot_file)
    tmp->label = gtk_label_new (editor_parse_file (data->maker_spot_file));
  else
    tmp->label = gtk_label_new (_("New Spot File"));

  gtk_widget_show (tmp->label);

  page =
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), tmp->maker,
			      tmp->label);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page);
  statusbar_set (_("New spot layer"));
}

/* EOF */
