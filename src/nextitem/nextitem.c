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

#include "nextitem.h"
#include "../commons/commons.h"
#include "../commons/x.h"
#include "../filechooser/filechooser.h"

GList *nextitem = NULL;

static soma_controller *ns_controller;
static GtkWidget *nextitem_entry = NULL;
static GtkWidget *nextitem_tv = NULL;
static GtkWidget *nextitem_spot = NULL;

static int nextitem_cmp_fragment (char *, char *);
static int nextitem_cmp (char *, char *);
static void nextitem_add_stream_r (GtkWidget *, gpointer);

static void nextitem_add_r_directory_r (GtkWidget *, gpointer);
static void nextitem_add_r_directory_rec (GList **, char *);
static void nextitem_add_r_file_r (GtkWidget *, gpointer);

static void nextitem_add_l_directory_r (GtkWidget *, gpointer);
static void nextitem_add_l_directory_rec (GList **, char *);
static void nextitem_add_l_file_r (GtkWidget *, gpointer);

static void nextitem_remove (GtkWidget *, GtkTreeSelection *);
static void nextitem_spot_activated (GtkWidget *, gpointer);

static void nextitem_timeout (GList *, int);

static void nextitem_add_r_directory_cb (void *l, void *t);
static void nextitem_add_r_file_cb (void *l, void *t);
static void nextitem_add_l_directory_cb (void *return_value, void *t);
static void nextitem_add_l_file_cb (void *return_value, void *t);

static void nextitem_drag (GtkWidget * tv, GdkDragContext * context,
			   gpointer dummy);

static void nextitem_move_up (void *l, void *t);
static void nextitem_move_down (void *l, void *t);

static int
nextitem_cmp_fragment (char *a, char *b)
{
  int l1, l2;

  l1 = strlen (a);
  l2 = strlen (b);

  while (l2 <= l1)
    {

      l1 = strlen (a);

      if (!strncasecmp (a, b, l2))
	return 1;

      a++;
    }

  return 0;
}

static int
nextitem_cmp (char *a, char *b)
{
  char dump[1024];
  char part[1024];
  int k = 0;
  int i;
  int len = strlen (b);
  int ret = 1;

  strncpy (dump, b, sizeof (dump));
  dump[len] = 0;

  for (i = 0; i < len; i++)
    {
      if (dump[i] == ' ' || dump[i] == '\t' || dump[i] == '\v'
	  || dump[i] == '\n')
	{

	  if (i > k)
	    {
	      strncpy (part, dump + k, i - k);
	      part[i - k] = 0;
	      ret = nextitem_cmp_fragment (a, part);
	      if (!ret)
		break;
	    }
	  k = i + 1;
	}
    }

  if (i > k && ret > 0)
    {
      strncpy (part, dump + k, i - k);
      part[i - k] = 0;
      ret = nextitem_cmp_fragment (a, part);
    }

  return ret;
}

static void
nextitem_spot_activated (GtkWidget * w, gpointer dummy)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)) == TRUE)
    somax_nextitem_set_spot (ns_controller, 1);
  else
    somax_nextitem_set_spot (ns_controller, 0);
}

static void
nextitem_remove (GtkWidget * w, GtkTreeSelection * selection)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *list, *old;
  int k, j = 0;
  GtkWidget *tmp_win;
  GdkCursor *cursor;
  char s[1024];
  int len;

  if ((list = gtk_tree_selection_get_selected_rows (selection, &model)))
    {
      tmp_win = somax_win ();
      gtk_widget_show_all (tmp_win);

      cursor = gdk_cursor_new (GDK_WATCH);
      gdk_window_set_cursor (tmp_win->window, cursor);

      old = list;
      len = g_list_length (list);

      while (list)
	{
	  gtk_tree_model_get_iter (model, &iter, list->data);
	  gtk_tree_model_get (model, &iter, 1, &k, -1);
	  gtk_tree_path_free (list->data);

	  snprintf (s, sizeof (s), "Removing... %d of %d", j, len);

	  somax_nextitem_remove (tmp_win, s, ns_controller, k - j);

	  j++;

	  list = list->next;
	}

      gdk_cursor_unref (cursor);
      gtk_widget_destroy (tmp_win);

      g_list_free (old);
    }
}

void
nextitem_refresh (void)
{
  GtkListStore *store;
  GtkTreeIter iter;
  char *text;
  GList *tmp;
  int i;

  store =
    (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (nextitem_tv));
  text = (char *) gtk_entry_get_text (GTK_ENTRY (nextitem_entry));

  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  i = 0;
  tmp = nextitem;

  while (tmp)
    {
      if ((text && text[0] && nextitem_cmp (tmp->data, text))
	  || (!text || !text[0]))
	{
	  gtk_list_store_append (store, &iter);
	  gtk_list_store_set (store, &iter, 0, tmp->data, 1, i, -1);
	}

      i++;
      tmp = tmp->next;
    }
}

GtkWidget *
nextitem_new (soma_controller * c, gboolean local)
{
  GtkWidget *box;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *sw;
  GtkWidget *label;
  GtkWidget *image;
  GtkListStore *model;
  GtkTreeSelection *selection;
  GtkCellRenderer *renderer;

  ns_controller = c;

  vbox = gtk_vbox_new (FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (sw);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  nextitem_tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (nextitem_tv), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (nextitem_tv), TRUE);
  gtk_container_add (GTK_CONTAINER (sw), nextitem_tv);
  gtk_widget_show (nextitem_tv);

  g_signal_connect (G_OBJECT (nextitem_tv), "drag-end",
		    G_CALLBACK (nextitem_drag), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (nextitem_tv));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (nextitem_tv), TRUE);
  g_object_unref (model);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (nextitem_tv),
					       -1, _("NextItem List"),
					       renderer, "text", 0, NULL);

  nextitem_entry = gtk_entry_new ();
  gtk_widget_show (nextitem_entry);
  gtk_box_pack_start (GTK_BOX (hbox), nextitem_entry, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (nextitem_entry), "changed",
		    G_CALLBACK (nextitem_refresh), NULL);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (button), box);

  image = gtk_image_new_from_stock ("gtk-find", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("Search"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (nextitem_refresh), NULL);

  nextitem_spot =
    gtk_toggle_button_new_with_mnemonic (_("With Spot Interaction"));
  g_signal_connect (G_OBJECT (nextitem_spot), "toggled",
		    G_CALLBACK (nextitem_spot_activated), NULL);

  gtk_widget_show (nextitem_spot);
  gtk_box_pack_start (GTK_BOX (vbox), nextitem_spot, FALSE, TRUE, 0);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (nextitem_remove), selection);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (button), box);

  image = gtk_image_new_from_stock ("gtk-remove", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

  label = gtk_label_new (_("Remove Item"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (nextitem_add_stream_r), selection);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (button), box);

  image = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

  label = gtk_label_new (_("Add Stream"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (nextitem_add_r_file_r), NULL);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (button), box);

  image = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

  if (local == TRUE)
    label = gtk_label_new (_("Add Remote File"));
  else
    label = gtk_label_new (_("Add File"));

  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (nextitem_add_r_directory_r), NULL);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (button), box);

  image = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

  if (local == TRUE)
    label = gtk_label_new (_("Add Remote Directory"));
  else
    label = gtk_label_new (_("Add Directory"));

  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  if (local == TRUE)
    {
      hbox = gtk_hbox_new (TRUE, 0);
      gtk_widget_show (hbox);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

      button = gtk_button_new ();
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (nextitem_add_l_file_r), NULL);

      box = gtk_hbox_new (FALSE, 0);
      gtk_widget_show (box);
      gtk_container_add (GTK_CONTAINER (button), box);

      image = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_BUTTON);
      gtk_widget_show (image);
      gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

      label = gtk_label_new (_("Add Local File"));
      gtk_widget_show (label);
      gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

      button = gtk_button_new ();
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (nextitem_add_l_directory_r), NULL);

      box = gtk_hbox_new (FALSE, 0);
      gtk_widget_show (box);
      gtk_container_add (GTK_CONTAINER (button), box);

      image = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_BUTTON);
      gtk_widget_show (image);
      gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

      label = gtk_label_new (_("Add Local Directory"));
      gtk_widget_show (label);
      gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
    }

  /* UP and DOWN */
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (nextitem_move_up), NULL);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (button), box);

  image = gtk_image_new_from_stock ("gtk-go-up", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

  label = gtk_label_new (_("Move Up"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (nextitem_move_down), NULL);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (button), box);

  image = gtk_image_new_from_stock ("gtk-go-down", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

  label = gtk_label_new (_("Move Down"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  return vbox;
}

static void
nextitem_add_stream_r (GtkWidget * w, gpointer data)
{
  nextitem_add_stream ();
}

void
nextitem_add_stream (void)
{
  GtkWidget *dialog;
  GtkWidget *box;
  GtkWidget *table;
  GtkWidget *stock;
  GtkWidget *label;
  GtkWidget *stream;

  char s[1024];

  snprintf (s, sizeof (s), "%s %s", PACKAGE, VERSION);

  dialog =
    gtk_dialog_new_with_buttons (s, NULL, GTK_DIALOG_MODAL |
				 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK,
				 GTK_RESPONSE_OK, GTK_STOCK_CANCEL,
				 GTK_RESPONSE_CANCEL, NULL);

  box = gtk_hbox_new (FALSE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (box), 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), box, FALSE, FALSE,
		      0);

  stock =
    gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
			      GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (box), stock, FALSE, FALSE, 0);

  table = gtk_table_new (0, 0, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 0);

  label = gtk_label_new_with_mnemonic (_("Stream:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

  stream = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), stream, 1, 2, 0, 1);
  gtk_widget_show_all (box);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {

      char *s = (char *) gtk_entry_get_text (GTK_ENTRY (stream));

      if (s && *s)
	{
	  if (!strncmp (s, "http://", 7) || !strncmp (s, "https://", 8)
	      || !strncmp (s, "ftp://", 6))
	    {
	      GtkWidget *tmp_win;
	      GdkCursor *cursor;
	      char tmp_str[1024];

	      tmp_win = somax_win ();
	      gtk_widget_show_all (tmp_win);

	      cursor = gdk_cursor_new (GDK_WATCH);
	      gdk_window_set_cursor (tmp_win->window, cursor);

	      snprintf (tmp_str, sizeof (tmp_str), "Adding a stream...");
	      somax_nextitem_set (tmp_win, tmp_str, ns_controller, s);

	      gdk_cursor_unref (cursor);
	      gtk_widget_destroy (tmp_win);
	    }
	}
    }

  gtk_widget_destroy (dialog);
}

static void
nextitem_add_r_directory_rec (GList ** list, char *path)
{
  char **arg, **old;
  char s[1024];

  if ((arg = somax_get_path (ns_controller, path)))
    {
      old = arg;

      while (*arg)
	{
	  if (**arg != '.')
	    {
	      snprintf (s, sizeof (s), "%s/%s", path, *arg);

	      if (s[strlen (s) - 1] != '/')
		*list = g_list_append (*list, g_strdup (s));
	      else
		nextitem_add_r_directory_rec (list, s);
	    }

	  arg++;
	}

      somax_get_path_free (old);
    }
}

static void
nextitem_add_r_directory_r (GtkWidget * w, gpointer dummy)
{
  nextitem_add_r_directory ();
}

void
nextitem_add_r_directory (void)
{
  remote_file_chooser_cb (PACKAGE " " VERSION, GTK_SELECTION_MULTIPLE,
			  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			  ns_controller, nextitem_add_r_directory_cb, NULL);
}

static void
nextitem_add_r_directory_cb (void *return_value, void *t)
{
  char s[1024];
  GSList *ll_old, *ll;
  char **arg;
  GList *list = NULL, *old;
  char *l;

  if (!return_value)
    return;

  ll_old = ll = (GSList *) return_value;

  while (ll)
    {
      l = (char *) ll->data;

      if ((arg = somax_get_path (ns_controller, l)))
	{
	  nextitem_add_r_directory_rec (&list, l);

	  if (list)
	    {
	      GtkWidget *tmp_win;
	      GdkCursor *cursor;
	      int len;
	      int i = 1;

	      old = list;

	      len = g_list_length (list);

	      tmp_win = somax_win ();
	      gtk_widget_show_all (tmp_win);

	      cursor = gdk_cursor_new (GDK_WATCH);
	      gdk_window_set_cursor (tmp_win->window, cursor);

	      while (list)
		{
		  snprintf (s, sizeof (s), "Adding... %d of %d", i++, len);
		  somax_nextitem_path_set (tmp_win, s, ns_controller,
					   list->data);
		  g_free (list->data);

		  list = list->next;
		}

	      g_list_free (old);

	      gdk_cursor_unref (cursor);
	      gtk_widget_destroy (tmp_win);
	    }

	  somax_get_path_free (arg);
	}

      g_free (ll->data);
      ll = ll->next;
    }

  g_slist_free (ll_old);
}

static void
nextitem_add_r_file_r (GtkWidget * w, gpointer dummy)
{
  nextitem_add_r_file ();
}

void
nextitem_add_r_file (void)
{
  remote_file_chooser_cb (PACKAGE " " VERSION, GTK_SELECTION_MULTIPLE,
			  GTK_FILE_CHOOSER_ACTION_OPEN, ns_controller,
			  nextitem_add_r_file_cb, NULL);
}

static void
nextitem_add_r_file_cb (void *return_value, void *t)
{
  char s[1024];
  GSList *l, *old;
  GtkWidget *tmp_win;
  GdkCursor *cursor;
  int len;
  int i = 1;

  l = old = (GSList *) return_value;

  if (!l)
    return;

  len = g_slist_length (l);

  tmp_win = somax_win ();
  gtk_widget_show_all (tmp_win);

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (tmp_win->window, cursor);

  while (l)
    {
      snprintf (s, sizeof (s), "Adding... %d of %d", i++, len);

      if (((char *) l->data)[strlen (l->data) - 1] != '/')
	somax_nextitem_path_set (tmp_win, s, ns_controller, (char *) l->data);

      g_free (l->data);
      l = l->next;
    }

  gdk_cursor_unref (cursor);
  gtk_widget_destroy (tmp_win);

  g_slist_free (old);
}

void
nextitem_timer_refresh (char **l, int s)
{
  GList *nl = NULL;

  if (l)
    {
      while (*l)
	{
	  nl = g_list_append (nl, g_strdup (*l));
	  l++;
	}
    }

  nextitem_timeout (nl, s);
}

static void
nextitem_timeout (GList * nl, int s)
{
  int ret = 0;
  GList *a, *b;

  a = nextitem;
  b = nl;

  while (a && b)
    {
      if (strcmp (a->data, b->data))
	{
	  ret = 1;
	  break;
	}

      a = a->next;
      b = b->next;
    }

  if (ret || (a && !b) || (!a && b))
    {

      if (nextitem)
	{
	  nl = nextitem;

	  while (nl)
	    {
	      g_free (nl->data);
	      nl = nl->next;
	    }

	  g_list_free (nextitem);
	}

      nextitem = nl;

      nextitem_refresh ();
    }

  else if (nl)
    {
      GList *l;

      l = nl;
      while (l)
	{
	  g_free (l->data);
	  l = l->next;
	}

      g_list_free (nl);
    }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (nextitem_spot),
				s ? TRUE : FALSE);
}

static void
nextitem_add_l_directory_rec (GList ** list, char *path)
{
  GDir *dir;
  GList *files, *l;
  gchar *file;

  if (g_file_test (path, G_FILE_TEST_EXISTS) == FALSE)
    return;

  if (!(dir = g_dir_open (path, 0, NULL)))
    return;

  files = NULL;

  while ((file = (gchar *) g_dir_read_name (dir)))
    files = g_list_append (files, g_strdup (file));

  g_dir_close (dir);

  files = g_list_sort (files, (GCompareFunc) strcmp);

  for (l = files; l; l = l->next)
    {
      file = l->data;

      if (*file)
	continue;

      file = g_build_path (G_DIR_SEPARATOR_S, path, file, NULL);

      if (g_file_test (file, G_FILE_TEST_IS_DIR) == TRUE)
	nextitem_add_l_directory_rec (list, file);

      else if (g_file_test (file, G_FILE_TEST_EXISTS) == TRUE)
	*list = g_list_append (*list, g_strdup (file));

      g_free (file);
    }

  g_list_foreach (files, (GFunc) g_free, NULL);
  g_list_free (files);
}

static void
nextitem_add_l_directory_r (GtkWidget * w, gpointer dummy)
{
  nextitem_add_l_directory ();
}

void
nextitem_add_l_directory (void)
{
  file_chooser_cb (PACKAGE " " VERSION, GTK_SELECTION_SINGLE,
		   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		   nextitem_add_l_directory_cb, NULL);
}

static void
nextitem_add_l_directory_cb (void *return_value, void *t)
{
  char s[1024];
  char *l = (char *) return_value;
  GList *list = NULL, *old;

  if (!l)
    return;

  nextitem_add_l_directory_rec (&list, l);
  g_free (l);

  if (list)
    {
      GtkWidget *tmp_win;
      GdkCursor *cursor;
      int len;
      int i = 1;

      old = list;

      len = g_list_length (list);

      tmp_win = somax_win ();
      gtk_widget_show_all (tmp_win);

      cursor = gdk_cursor_new (GDK_WATCH);
      gdk_window_set_cursor (tmp_win->window, cursor);

      while (list)
	{
	  snprintf (s, sizeof (s), "Adding... %d of %d", i++, len);

	  if (g_file_test (list->data, G_FILE_TEST_IS_DIR) == TRUE)
	    somax_nextitem_set (tmp_win, s, ns_controller, list->data);

	  g_free (list->data);

	  list = list->next;
	}

      g_list_free (old);

      gdk_cursor_unref (cursor);
      gtk_widget_destroy (tmp_win);
    }
}

static void
nextitem_add_l_file_r (GtkWidget * w, gpointer dummy)
{
  nextitem_add_l_file ();
}

void
nextitem_add_l_file (void)
{
  file_chooser_cb (PACKAGE " " VERSION, GTK_SELECTION_MULTIPLE,
		   GTK_FILE_CHOOSER_ACTION_OPEN, nextitem_add_l_file_cb,
		   NULL);
}

static void
nextitem_add_l_file_cb (void *return_value, void *t)
{
  char s[1024];
  GSList *l, *old;
  GtkWidget *tmp_win;
  GdkCursor *cursor;
  int len;
  int i = 1;

  l = old = (GSList *) return_value;

  if (!l)
    return;

  len = g_slist_length (l);

  tmp_win = somax_win ();
  gtk_widget_show_all (tmp_win);

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (tmp_win->window, cursor);

  while (l)
    {
      snprintf (s, sizeof (s), "Adding... %d of %d", i++, len);

      if (g_file_test (l->data, G_FILE_TEST_EXISTS) == TRUE)
	somax_nextitem_set (tmp_win, s, ns_controller, l->data);

      g_free (l->data);
      l = l->next;
    }

  g_slist_free (old);

  gdk_cursor_unref (cursor);
  gtk_widget_destroy (tmp_win);
}

static void
nextitem_drag (GtkWidget * tv, GdkDragContext * context, gpointer dummy)
{

  GtkTreeModel *model;
  GtkTreeIter iter;
  GdkCursor *cursor;
  gint i;
  gchar s[1024];
  GList *l, *list = NULL;
  gint len = g_list_length (nextitem);
  GtkWidget *tmp_win = somax_win ();

  gtk_widget_show_all (tmp_win);
  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (tmp_win->window, cursor);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW ((tv)));
  if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
    {
      char *text;

      do
	{
	  gtk_tree_model_get (model, &iter, 0, &text, -1);
	  list = g_list_append (list, text);
	}
      while (gtk_tree_model_iter_next (model, &iter) == TRUE);
    }

  for (i = 0; i < len; i++)
    {
      snprintf (s, sizeof (s), "Removing... %d of %d", i, len);
      somax_nextitem_remove (tmp_win, s, ns_controller, 0);
    }

  for (i = 0, len = g_list_length (list), l = list; l; l = l->next)
    {
      snprintf (s, sizeof (s), "Adding... %d of %d", i++, len);
      somax_nextitem_set (tmp_win, s, ns_controller, l->data);
    }

  g_list_foreach (list, (GFunc) g_free, NULL);
  g_list_free (list);

  gdk_cursor_unref (cursor);
  gtk_widget_destroy (tmp_win);
}

static void
nextitem_move_down (void *l, void *t)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter, parent;
  GList *list, *old;
  gboolean exist;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (nextitem_tv));

  if (!
      (list = old = gtk_tree_selection_get_selected_rows (selection, &model)))
    {
      dialog_msg (_("No selected items!"));
      return;
    }

  list = g_list_last (list);
  while (list)
    {
      gtk_tree_model_get_iter (model, &iter, list->data);
      gtk_tree_model_get_iter (model, &parent, list->data);
      gtk_tree_path_free (list->data);

      exist = gtk_tree_model_iter_next (model, &parent);
      gtk_list_store_move_after (GTK_LIST_STORE (model), &iter,
				 exist == TRUE ? &parent : NULL);

      list = list->prev;
    }

  g_list_free (old);
  nextitem_drag (nextitem_tv, NULL, NULL);
}

static void
nextitem_move_up (void *l, void *t)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter, parent;
  GtkTreePath *path;
  GList *list, *old;
  gboolean exist;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (nextitem_tv));

  if (!
      (list = old = gtk_tree_selection_get_selected_rows (selection, &model)))
    {
      dialog_msg (_("No selected items!"));
      return;
    }

  while (list)
    {
      gtk_tree_model_get_iter (model, &iter, list->data);
      gtk_tree_path_free (list->data);

      if ((path = gtk_tree_model_get_path (model, &iter)))
	{
	  if (gtk_tree_path_prev (path) == FALSE
	      || gtk_tree_model_get_iter (model, &parent, path) == FALSE)
	    exist = FALSE;
	  else
	    exist = TRUE;

	  gtk_tree_path_free (path);
	}
      else
	exist = FALSE;

      gtk_list_store_move_before (GTK_LIST_STORE (model), &iter,
				  exist == TRUE ? &parent : NULL);

      list = list->next;
    }

  g_list_free (old);
  nextitem_drag (nextitem_tv, NULL, NULL);
}

/* EOF */
