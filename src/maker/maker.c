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

#include "../commons/commons.h"
#include "../commons/data.h"
#include "../commons/x.h"
#include "../commons/stat.h"
#include "maker.h"
#include "../draw/draw.h"
#include "../palinsesto/palinsesto.h"
#include "../filechooser/filechooser.h"
#include "../modules/modules.h"

static void maker_treeview_r_add_cb (void *l, void *t);
static void maker_treeview_r_add_dir_cb (void *l, void *t);

void
maker_controller_check (void *controller, GtkWidget * w, GList ** list)
{
  gtk_widget_set_sensitive (w, controller ? TRUE : FALSE);
  *list = g_list_prepend (*list, w);
}

void
maker_file_r_add (GtkWidget * w, GtkWidget * entry)
{
  char *file = NULL;
  char s[1024];
  struct maker_pl_data_t *pl;
  struct maker_spot_data_t *spot;

  if ((pl = g_object_get_data (G_OBJECT (entry), "data_pl"))
      && pl->controller)
    file =
      remote_file_chooser (_("Search a remote file"), GTK_SELECTION_SINGLE,
			   GTK_FILE_CHOOSER_ACTION_OPEN, pl->controller);

  else
    if ((spot = g_object_get_data (G_OBJECT (entry), "data_spot"))
	&& spot->controller)
    file =
      remote_file_chooser (_("Search a remote file"), GTK_SELECTION_SINGLE,
			   GTK_FILE_CHOOSER_ACTION_OPEN, spot->controller);

  if (!file)
    return;

  snprintf (s, sizeof (s), "%c%s", DISTRIBUITED_CHAR, file);
  gtk_entry_set_text (GTK_ENTRY (entry), s);
  g_free (file);
}

void
maker_treeview_r_add (GtkWidget * w, GtkWidget * t)
{
  struct maker_pl_data_t *pl;
  struct maker_spot_data_t *spot;

  if ((pl = g_object_get_data (G_OBJECT (t), "data_pl")) && pl->controller)
    remote_file_chooser_cb (_("Add files"), GTK_SELECTION_MULTIPLE,
			    GTK_FILE_CHOOSER_ACTION_OPEN, pl->controller,
			    maker_treeview_r_add_cb, t);

  else
    if ((spot = g_object_get_data (G_OBJECT (t), "data_spot"))
	&& spot->controller)
    remote_file_chooser_cb (_("Add files"), GTK_SELECTION_MULTIPLE,
			    GTK_FILE_CHOOSER_ACTION_OPEN, spot->controller,
			    maker_treeview_r_add_cb, t);
}

static void
maker_treeview_r_add_cb (void *l, void *t)
{
  GSList *file, *old;
  GtkTreeModel *model;
  GtkTreeIter iter;
  char s[1024];
  char *fl;
  struct maker_pl_data_t *pl;
  struct maker_spot_data_t *spot;


  if (!l || !t)
    return;

  old = file = (GSList *) l;

  while (file)
    {
      fl = (char *) file->data;

      snprintf (s, sizeof (s), "%c%s", DISTRIBUITED_CHAR, (char *) fl);
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (t));
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, s, -1);

      g_free (file->data);

      file = file->next;
    }

  g_slist_free (old);

  if ((pl = g_object_get_data (G_OBJECT (t), "data_pl")))
    maker_pl_changed (NULL, pl);

  else if ((spot = g_object_get_data (G_OBJECT (t), "data_spot")))
    maker_spot_changed (NULL, spot);
}

void
maker_treeview_r_add_dir (GtkWidget * w, GtkWidget * t)
{
  struct maker_pl_data_t *pl;
  struct maker_spot_data_t *spot;

  if ((pl = g_object_get_data (G_OBJECT (t), "data_pl")) && pl->controller)
    remote_file_chooser_cb (_("Add directories"), GTK_SELECTION_MULTIPLE,
			    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			    pl->controller, maker_treeview_r_add_dir_cb, t);

  else if ((spot = g_object_get_data (G_OBJECT (t), "data_spot"))
	   && spot->controller)
    remote_file_chooser_cb (_("Add directories"), GTK_SELECTION_MULTIPLE,
			    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			    spot->controller, maker_treeview_r_add_dir_cb, t);
}

static void
maker_treeview_r_add_dir_cb (void *l, void *t)
{
  GSList *file, *old;
  GtkTreeModel *model;
  GtkTreeIter iter;
  char s[1024];
  char *fl;
  struct maker_pl_data_t *pl;
  struct maker_spot_data_t *spot;

  if (!l || !t)
    return;

  old = file = (GSList *) l;

  while (file)
    {
      fl = (char *) file->data;

      snprintf (s, sizeof (s), "%c%s", DISTRIBUITED_CHAR, (char *) fl);
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (t));
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, s, -1);

      g_free (file->data);

      file = file->next;
    }

  g_slist_free (old);

  if ((pl = g_object_get_data (G_OBJECT (t), "data_pl")))
    maker_pl_changed (NULL, pl);

  else if ((spot = g_object_get_data (G_OBJECT (t), "data_spot")))
    maker_spot_changed (NULL, spot);
}

void
maker_popup_position (GtkMenu * menu, gint * xp, gint * yp, gboolean * p,
		      gpointer data)
{
  GtkRequisition req;
  GdkScreen *screen;
  int x, y, px, py, monitor_n;
  GdkRectangle monitor;
  int menu_size;

  gtk_widget_realize (GTK_WIDGET (menu));
  menu_size = GTK_WIDGET (menu)->allocation.width;

  gdk_window_get_origin (GTK_WIDGET (data)->window, &px, &py);
  gtk_widget_size_request (gtk_widget_get_toplevel (GTK_WIDGET (data)), &req);

  y =
    py + GTK_WIDGET (data)->allocation.y +
    GTK_WIDGET (data)->allocation.height + 1;
  x =
    px + GTK_WIDGET (data)->allocation.x +
    GTK_WIDGET (data)->allocation.width - menu_size;

  screen =
    gtk_widget_get_screen (gtk_widget_get_toplevel (GTK_WIDGET (data)));
  monitor_n = gdk_screen_get_monitor_at_point (screen, px, py);
  gdk_screen_get_monitor_geometry (screen, monitor_n, &monitor);

  gtk_widget_size_request (GTK_WIDGET (menu), &req);

  if ((x + req.width) > monitor.x + monitor.width)
    x -= (x + req.width) - (monitor.x + monitor.width);
  else if (x < monitor.x)
    x = monitor.x;

  if ((y + req.height) > monitor.y + monitor.height)
    y -= GTK_WIDGET (data)->allocation.height + req.height + 1;
  else if (y < monitor.y)
    y = monitor.y;

  *xp = x;
  *yp = y;
}

gboolean
maker_changed_check (GtkWidget * w)
{
  gpointer *previous = g_object_get_data (G_OBJECT (w), "previous");

  if (GTK_IS_ENTRY (w))
    {
      if (!previous)
	return TRUE;

      if (!strcmp ((gchar *) previous, gtk_entry_get_text (GTK_ENTRY (w))))
	return FALSE;
    }

  else if (GTK_IS_COMBO_BOX (w))
    {
      if ((gint) previous == gtk_combo_box_get_active (GTK_COMBO_BOX (w)))
	return FALSE;
    }

  else if (GTK_IS_CHECK_BUTTON (w) || GTK_IS_TOGGLE_BUTTON (w))
    {
      if ((gboolean) previous ==
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)))
	return FALSE;
    }

  else if (GTK_IS_TEXT_BUFFER (w))
    {
      GtkTextIter start, end;
      gboolean ret = TRUE;
      gchar *c;

      if (!previous)
	return TRUE;

      gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (w), &start, 0);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (w), &end);

      if ((c =
	   gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (w), &start, &end,
				      TRUE)))
	{
	  ret = !strcmp ((gchar *) previous, c) ? FALSE : TRUE;
	  g_free (c);
	}

      return ret;
    }

  /* FIXME: other widgets? */

  return TRUE;
}

void
maker_changed_destroy (GtkWidget * w)
{
  gpointer previous = g_object_get_data (G_OBJECT (w), "previous");

  if (!previous)
    return;

  if (GTK_IS_ENTRY (w) || GTK_IS_TEXT_BUFFER (w))
    g_free (previous);

  /* FIXME: other widgets? */

  g_object_steal_data (G_OBJECT (w), "previous");
}

void
maker_changed_set (GtkWidget * w)
{
  if (GTK_IS_ENTRY (w))
    {
      gchar *previous = g_object_get_data (G_OBJECT (w), "previous");
      if (previous)
	g_free (previous);

      if (GTK_IS_SPIN_BUTTON (w))
	{
	  gchar *tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (w));
	  if (*tmp)
	    g_object_set_data (G_OBJECT (w), "previous", g_strdup (tmp));
	  else
	    {
	      tmp =
		g_strdup_printf ("%d",
				 gtk_spin_button_get_value_as_int
				 (GTK_SPIN_BUTTON (w)));
	      g_object_set_data (G_OBJECT (w), "previous", tmp);
	    }
	}
      else
	g_object_set_data (G_OBJECT (w), "previous",
			   g_strdup (gtk_entry_get_text (GTK_ENTRY (w))));
    }

  else if (GTK_IS_COMBO_BOX (w))
    g_object_set_data (G_OBJECT (w), "previous",
		       (gpointer)
		       gtk_combo_box_get_active (GTK_COMBO_BOX (w)));

  else if (GTK_IS_CHECK_BUTTON (w) || GTK_IS_TOGGLE_BUTTON (w))
    g_object_set_data (G_OBJECT (w), "previous",
		       (gpointer)
		       gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)));

  else if (GTK_IS_TEXT_BUFFER (w))
    {
      GtkTextIter start, end;
      gchar *c;

      gchar *previous = g_object_get_data (G_OBJECT (w), "previous");
      if (previous)
	g_free (previous);

      gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (w), &start, 0);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (w), &end);

      if ((c =
	   gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (w), &start, &end,
				      TRUE)))
	g_object_set_data (G_OBJECT (w), "previous", c);
      else
	g_object_steal_data (G_OBJECT (w), "previous");
    }

  /* FIXME: other widgets? */
}

gchar *
maker_message (GtkWidget * w)
{
  gchar *message = g_object_get_data (G_OBJECT (w), "message");
  gint add = (gint) g_object_get_data (G_OBJECT (w), "add");
  gchar *ret;

  if (!message)
    return g_strdup (_("No description for this change"));

  if (GTK_IS_ENTRY (w))
    {
      if (GTK_IS_SPIN_BUTTON (w))
	{
	  gchar *tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (w));
	  if (*tmp)
	    ret =
	      g_strdup_printf (message, gtk_entry_get_text (GTK_ENTRY (w)));
	  else
	    {
	      tmp =
		g_strdup_printf ("%d",
				 gtk_spin_button_get_value_as_int
				 (GTK_SPIN_BUTTON (w)));
	      ret = g_strdup_printf (message, tmp);
	      g_free (tmp);

	    }
	}
      else
	ret = g_strdup_printf (message, gtk_entry_get_text (GTK_ENTRY (w)));
    }

  else if (GTK_IS_COMBO_BOX (w))
    {
      gchar *tmp;
      gint id = gtk_combo_box_get_active (GTK_COMBO_BOX (w));

      if (id > 0)
	tmp =
	  g_strdup_printf ("%d",
			   gtk_combo_box_get_active (GTK_COMBO_BOX (w)) +
			   add);
      else
	tmp = g_strdup_printf ("ANY");
      ret = g_strdup_printf (message, tmp);
      g_free (tmp);
    }

  else if (GTK_IS_CHECK_BUTTON (w) || GTK_IS_TOGGLE_BUTTON (w))
    ret =
      g_strdup_printf (message,
		       gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)) ?
		       _("true") : _("false"));

  else
    ret = g_strdup (message);

  /* FIXME: other widgets? */
  return ret;
}

/* EOF */
