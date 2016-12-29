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

#include "plugins.h"

static GList *plugin_list = NULL;
static struct plugin_t *plugin_get_data (gchar *);
static void plugin_data_parse (gchar *, struct plugin_t *);

void
plugin_scanner (void)
{
  GDir *d;
  struct plugin_t *tmp;
  gchar *a;
  const gchar *file;

  if (!(d = g_dir_open (PLUGINS_PATH, 0, NULL)))
    return;

  while ((file = g_dir_read_name (d)))
    {
      if (*file == '.')
	continue;

      splash_set_text ("%s", file);

      a = g_build_path (G_DIR_SEPARATOR_S, PLUGINS_PATH, file, NULL);
      if (g_file_test (a, G_FILE_TEST_IS_REGULAR) == FALSE)
	{
	  g_free (a);
	  continue;
	}

      if (!(tmp = plugin_get_data (a)))
	{
	  g_free (a);
	  continue;
	}

      plugin_list = g_list_append (plugin_list, tmp);
    }

  g_dir_close (d);
}

static struct plugin_t *
plugin_get_data (gchar * a)
{
  struct plugin_t *tmp;
  GPid pid;
  gint fd;
  char *arg[3];
  int k = 0;
  char buffer[1024];
  char buf;

  arg[0] = a;
  arg[1] = "data";
  arg[2] = NULL;

  if (g_spawn_async_with_pipes
      (NULL, arg, get_env (), 0, NULL, NULL, &pid, NULL, &fd, NULL,
       NULL) == FALSE)
    return NULL;

  tmp = (struct plugin_t *) g_malloc0 (sizeof (struct plugin_t));

  while (read (fd, &buf, sizeof (char)) == 1)
    {
      if (buf == '\r' || buf == '\n' || k == sizeof (buffer) - 1)
	{
	  buffer[k] = 0;
	  plugin_data_parse (buffer, tmp);
	  k = 0;
	}

      else
	buffer[k++] = buf;
    }

  g_spawn_close_pid (pid);

  if (!tmp->name || !tmp->description)
    {
      if (tmp->name)
	g_free (tmp->name);

      if (tmp->description)
	g_free (tmp->description);

      if (tmp->author)
	g_free (tmp->author);

      if (tmp->version)
	g_free (tmp->version);

      g_free (tmp);

      return NULL;
    }

  tmp->file = a;
  return tmp;
}

void
plugin_starter (struct plugin_t *data, char *file)
{
  GPid pid;
  gchar *arg[3];

  arg[0] = data->file;
  arg[1] = file;
  arg[2] = NULL;

  if (g_spawn_async (NULL, arg, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

static void
plugin_data_parse (gchar * buf, struct plugin_t *data)
{
  if (!strncmp (buf, "author: ", strlen ("author: ")) && !data->author)
    {
      data->author = g_strdup (buf + strlen ("author: "));
      return;
    }

  if (!strncmp (buf, "description: ", strlen ("description: "))
      && !data->description)
    {
      data->description = g_strdup (buf + strlen ("description: "));
      return;
    }

  if (!strncmp (buf, "name: ", strlen ("name: ")) && !data->name)
    {
      data->name = g_strdup (buf + strlen ("name: "));
      return;
    }

  if (!strncmp (buf, "version: ", strlen ("version: ")) && !data->version)
    {
      data->version = g_strdup (buf + strlen ("version: "));
      return;
    }

  if (!strncmp (buf, "licence: ", strlen ("licence: ")) && !data->licence)
    {
      data->licence = g_strdup (buf + strlen ("licence: "));
      return;
    }
}

void
plugin_about (void)
{
  GtkWidget *dialog;
  GtkWidget *button;
  GtkWidget *sw;
  GtkWidget *tv;
  GtkListStore *model;
  GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkCellRenderer *renderer;
  GList *tmp;

  char s[1024];

  snprintf (s, sizeof (s), _("%s %s - plugins about"), PACKAGE, VERSION);

  dialog = gtk_dialog_new ();
  gtk_window_resize (GTK_WINDOW (dialog), 400, 300);
  gtk_window_set_title (GTK_WINDOW (dialog), s);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (sw);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), sw, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  model =
    gtk_list_store_new (5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING);
  tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tv), FALSE);
  gtk_container_add (GTK_CONTAINER (sw), tv);
  gtk_widget_show (tv);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tv), TRUE);
  g_object_unref (model);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tv),
					       -1, _("Plugin"), renderer,
					       "text", 0, NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tv),
					       -1, _("Author"), renderer,
					       "text", 1, NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tv),
					       -1, _("Version"), renderer,
					       "text", 2, NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tv),
					       -1, _("Licence"), renderer,
					       "text", 3, NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tv),
					       -1, _("Description"), renderer,
					       "text", 4, NULL);

  tmp = plugin_list;
  while (tmp)
    {
      gtk_list_store_append (model, &iter);
      gtk_list_store_set (model, &iter,
			  0, ((struct plugin_t *) tmp->data)->name,
			  1, ((struct plugin_t *) tmp->data)->author,
			  2, ((struct plugin_t *) tmp->data)->version,
			  3, ((struct plugin_t *) tmp->data)->licence,
			  4, ((struct plugin_t *) tmp->data)->description,
			  -1);
      tmp = tmp->next;
    }

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

GtkWidget *
plugin_new (void *func)
{
  GList *tmp;
  GtkWidget *mitem;
  GtkWidget *menu;
  GtkWidget *image;
  GtkWidget *item;

  if (!plugin_list)
    return NULL;

  tmp = plugin_list;

  mitem = gtk_menu_item_new_with_mnemonic (_("_Plugins"));

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), menu);

  while (tmp)
    {
      item = gtk_menu_item_new_with_mnemonic (((struct plugin_t *)
					       tmp->data)->description);
      gtk_widget_show (item);
      gtk_container_add (GTK_CONTAINER (menu), item);
      g_signal_connect ((gpointer) item, "activate",
			GTK_SIGNAL_FUNC (func), tmp->data);
      tmp = tmp->next;
    }

  item = gtk_menu_item_new ();
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  gtk_widget_set_sensitive (item, FALSE);

  item = gtk_image_menu_item_new_with_mnemonic (_("Plugins _About"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  g_signal_connect ((gpointer) item, "activate",
		    GTK_SIGNAL_FUNC (plugin_about), NULL);

  image = gtk_image_new_from_stock ("gtk-about", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);

  return mitem;
}

/* EOF */
