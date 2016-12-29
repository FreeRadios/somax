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
#include "../commons/x.h"
#include "filechooser.h"

struct __remote_file_chooser_t
{
  void *controller;
  GtkWidget *dialog;
  int type;
  char *path;
};

static struct __remote_file_chooser_t __remote_file_chooser_data =
  { NULL, NULL, 0, NULL };

static void remote_file_chooser_refresh (GtkWidget *, char *);

static gboolean
remote_file_chooser_select (GtkWidget * widget, GdkEventButton * event,
			    gpointer dummy)
{
  if (event->type == GDK_2BUTTON_PRESS)
    {
      GtkTreeSelection *selection;
      GtkTreeModel *model;
      GtkTreeIter iter;
      char *data = NULL;

      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));

      if (__remote_file_chooser_data.type == GTK_SELECTION_SINGLE)
	{
	  if (gtk_tree_selection_get_selected (selection, &model, &iter) ==
	      FALSE)
	    return FALSE;

	  gtk_tree_model_get (model, &iter, 1, &data, -1);
	}

      else if (__remote_file_chooser_data.type == GTK_SELECTION_MULTIPLE)
	{
	  GList *list;

	  if (!
	      (list =
	       gtk_tree_selection_get_selected_rows (selection, &model)))
	    return TRUE;

	  gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, list->data);
	  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &data, -1);

	  g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
	  g_list_free (list);
	}

      if (!data)
	return FALSE;

      if (data[strlen (data) - 1] == '/')
	{
	  remote_file_chooser_refresh (widget, data);
	  if (__remote_file_chooser_data.path)
	    g_free (__remote_file_chooser_data.path);
	  __remote_file_chooser_data.path = g_strdup (data);
	}

      else
	gtk_dialog_response (GTK_DIALOG (__remote_file_chooser_data.dialog),
			     GTK_RESPONSE_OK);

      g_free (data);

    }

  return FALSE;
}

static void
remote_file_chooser_refresh (GtkWidget * textview, char *path)
{
  char **arg, **old;
  int i;
  char buf[1024];
  GtkListStore *store;
  GtkTreeIter iter;

  old = arg = somax_get_path (__remote_file_chooser_data.controller, path);

  store = (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (textview));

  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, ".", 1, path, -1);

  if (!strcmp (path, "/"))
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, "..", 1, path, -1);
    }
  else
    {
      for (i = strlen (path) - 2; i >= 0; i--)
	{
	  if (path[i] == '/')
	    {
	      strcpy (buf, path);
	      buf[i + 1] = 0;
	      break;
	    }
	}

      if (i < 0)
	strncpy (buf, path, sizeof (buf));

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, "..", 1, buf, -1);
    }

  if (!arg)
    return;

  while (*arg)
    {
      snprintf (buf, sizeof (buf), "%s%s", path, *arg);

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, *arg, 1, buf, -1);

      arg++;
    }

  somax_get_path_free (old);
}

void *
remote_file_chooser (char *title, GtkSelectionMode w, GtkFileChooserAction t,
		     soma_controller * controller)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *frame;
  GtkWidget *textview;
  GtkTreeSelection *selection;
  GtkListStore *model;
  GtkWidget *scrolledwindow;
  GtkCellRenderer *renderer;
  void *l = NULL;

  dialog = gtk_dialog_new ();

  __remote_file_chooser_data.dialog = dialog;
  __remote_file_chooser_data.controller = controller;
  __remote_file_chooser_data.type = w;

  gtk_window_resize (GTK_WINDOW (dialog), 400, 350);
  gtk_window_set_title (GTK_WINDOW (dialog), title);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE,
		      0);

  switch (t)
    {
    case GTK_FILE_CHOOSER_ACTION_OPEN:
      frame = gtk_frame_new (_("Files"));
      break;

    case GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER:
      frame = gtk_frame_new (_("Directories"));
      break;

    default:
      frame = gtk_frame_new (NULL);
      break;
    }

  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_container_add (GTK_CONTAINER (frame), scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
				       GTK_SHADOW_IN);

  model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  textview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (textview), TRUE);
  gtk_widget_show (textview);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (textview));
  gtk_tree_selection_set_mode (selection, w);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (textview), TRUE);
  g_object_unref (model);

  g_signal_connect ((gpointer) textview, "button-press-event",
		    GTK_SIGNAL_FUNC (remote_file_chooser_select), &w);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (textview),
					       -1, "Files", renderer,
					       "text", 0, NULL);

  button = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG
				(__remote_file_chooser_data.dialog), button,
				GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  if (t == GTK_FILE_CHOOSER_ACTION_SAVE)
    button = gtk_button_new_from_stock ("gtk-save");
  else
    button = gtk_button_new_from_stock ("gtk-open");
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG
				(__remote_file_chooser_data.dialog), button,
				GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  if (!__remote_file_chooser_data.path)
    __remote_file_chooser_data.path = g_strdup ("/");

  remote_file_chooser_refresh (textview, __remote_file_chooser_data.path);

  while (1)
    {
      if (gtk_dialog_run (GTK_DIALOG (__remote_file_chooser_data.dialog)) ==
	  GTK_RESPONSE_OK)
	{
	  GtkTreeIter iter;
	  GtkTreeModel *m;
	  void *data;

	  if (w == GTK_SELECTION_SINGLE)
	    {
	      if (t == GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER)
		{
		  if (gtk_tree_selection_get_selected (selection, &m, &iter)
		      == FALSE)
		    l = g_strdup (__remote_file_chooser_data.path);

		  else
		    {
		      char *d;
		      gtk_tree_model_get (m, &iter, 1, &d, -1);
		      if (d[strlen (d) - 1] == '/')
			l = d;

		      else
			{
			  l = g_strdup (__remote_file_chooser_data.path);
			  g_free (d);
			}
		    }
		}

	      else
		{
		  if (gtk_tree_selection_get_selected (selection, &m, &iter)
		      == FALSE)
		    break;

		  gtk_tree_model_get (m, &iter, 1, &data, -1);
		  l = data;
		}
	    }

	  else if (w == GTK_SELECTION_MULTIPLE)
	    {
	      GList *list, *old;

	      if (!
		  (list =
		   gtk_tree_selection_get_selected_rows (selection, &m)))
		break;

	      if (!list)
		break;

	      old = list;

	      while (list)
		{
		  gtk_tree_model_get_iter (GTK_TREE_MODEL (m), &iter,
					   list->data);
		  gtk_tree_model_get (GTK_TREE_MODEL (m), &iter, 1, &data,
				      -1);
		  gtk_tree_path_free (list->data);

		  l = g_slist_append (l, data);

		  list = list->next;
		}

	      g_list_free (old);
	    }

	  break;
	}

      else
	break;
    }

  gtk_widget_destroy (__remote_file_chooser_data.dialog);

  return l;
}

void
remote_file_chooser_cb (char *title, GtkSelectionMode w,
			GtkFileChooserAction t, soma_controller * controller,
			void (*func) (void *, void *), void *what)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *frame;
  GtkWidget *textview;
  GtkTreeSelection *selection;
  GtkListStore *model;
  GtkWidget *vbox;
  GtkWidget *scrolledwindow;
  GtkCellRenderer *renderer;
  GtkWidget *cb;
  void *l;
  int ret;
  static int cb_set = 0;

  dialog = gtk_dialog_new ();

  __remote_file_chooser_data.dialog = dialog;
  __remote_file_chooser_data.controller = controller;
  __remote_file_chooser_data.type = w;

  gtk_window_resize (GTK_WINDOW (dialog), 400, 350);
  gtk_window_set_title (GTK_WINDOW (dialog), title);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  vbox = GTK_DIALOG (dialog)->vbox;

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  switch (t)
    {
    case GTK_FILE_CHOOSER_ACTION_OPEN:
      frame = gtk_frame_new (_("Files"));
      break;

    case GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER:
      frame = gtk_frame_new (_("Directories"));
      break;

    default:
      frame = gtk_frame_new (NULL);
      break;
    }

  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_container_add (GTK_CONTAINER (frame), scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
				       GTK_SHADOW_IN);

  model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  textview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (textview), TRUE);
  gtk_widget_show (textview);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (textview));
  gtk_tree_selection_set_mode (selection, w);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (textview), TRUE);
  g_object_unref (model);

  g_signal_connect ((gpointer) textview, "button-press-event",
		    GTK_SIGNAL_FUNC (remote_file_chooser_select), &w);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (textview),
					       -1, "Files", renderer,
					       "text", 0, NULL);


  cb =
    gtk_check_button_new_with_label (_
				     ("Don't close this window after usage"));

  gtk_widget_show (cb);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), cb, FALSE, FALSE,
		      0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb), cb_set);

  button = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG
				(__remote_file_chooser_data.dialog), button,
				GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  if (t == GTK_FILE_CHOOSER_ACTION_SAVE)
    button = gtk_button_new_from_stock ("gtk-save");
  else
    button = gtk_button_new_from_stock ("gtk-open");
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG
				(__remote_file_chooser_data.dialog), button,
				GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  if (!__remote_file_chooser_data.path)
    __remote_file_chooser_data.path = g_strdup ("/");
  remote_file_chooser_refresh (textview, __remote_file_chooser_data.path);

  while (1)
    {
      l = NULL;

      ret = gtk_dialog_run (GTK_DIALOG (__remote_file_chooser_data.dialog));
      cb_set = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cb));

      if (ret == GTK_RESPONSE_OK)
	{
	  GtkTreeIter iter;
	  GtkTreeModel *m;
	  char *d;

	  if (w == GTK_SELECTION_SINGLE)
	    {
	      if (t == GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER)
		{
		  if (gtk_tree_selection_get_selected (selection, &m, &iter)
		      == FALSE)
		    l = g_strdup (__remote_file_chooser_data.path);
		  else
		    {
		      gtk_tree_model_get (m, &iter, 1, &d, -1);
		      if (d[strlen (d) - 1] == '/')
			l = d;
		      else
			{
			  l = g_strdup (__remote_file_chooser_data.path);
			  g_free (d);
			}
		    }
		}

	      else
		{
		  if (gtk_tree_selection_get_selected (selection, &m, &iter)
		      == FALSE)
		    {
		      if (cb_set)
			continue;
		      else
			break;
		    }

		  gtk_tree_model_get (m, &iter, 1, &d, -1);
		  l = d;
		}
	    }

	  else if (w == GTK_SELECTION_MULTIPLE)
	    {
	      GList *list, *old;

	      if (!
		  (list = old =
		   gtk_tree_selection_get_selected_rows (selection, &m)))
		{
		  if (cb_set)
		    continue;
		  else
		    break;
		}

	      while (list)
		{
		  gtk_tree_model_get_iter (GTK_TREE_MODEL (m), &iter,
					   list->data);
		  gtk_tree_model_get (GTK_TREE_MODEL (m), &iter, 1, &d, -1);
		  gtk_tree_path_free (list->data);

		  if (d[strlen (d) - 1] == '/')
		    {
		      if (t == GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER)
			l = g_slist_append (l, d);

		    }
		  else if (t == GTK_FILE_CHOOSER_ACTION_OPEN)
		    l = g_slist_append (l, d);

		  list = list->next;
		}

	      g_list_free (old);
	    }

	  if (!l)
	    {
	      if (cb_set)
		continue;
	      else
		break;
	    }

	  func (l, what);

	  if (cb_set)
	    continue;
	  else
	    break;
	}

      else
	break;
    }

  gtk_widget_destroy (__remote_file_chooser_data.dialog);

  return;
}

/* EOF */
