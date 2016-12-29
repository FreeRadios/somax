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

#include "somax.h"
#include "../palinsesto/palinsesto.h"
#include "../filechooser/filechooser.h"

static char *win_maker_pl_file = NULL;
static GtkWidget *win_maker_pl_save_button = NULL;
static GtkWidget *win_maker_pl_save_toolbar = NULL;

static char *win_maker_spot_file = NULL;
static GtkWidget *win_maker_spot_save_button = NULL;
static GtkWidget *win_maker_spot_save_toolbar = NULL;

static struct somad_data *dump_pl = NULL;
static struct somad_spot_data *dump_spot = NULL;

static void win_maker_pl_update (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_save (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_save_as (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_change_color (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_show_d (GtkWidget *, struct maker_pl_data_t *);
static int win_maker_pl_show (GtkWidget *, GdkEvent *,
			      struct maker_pl_data_t *);
static void win_maker_pl_undo (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_redo (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_undo_history (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_redo_history (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_cut (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_copy (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_paste (GtkWidget *, struct maker_pl_data_t *);
static void win_maker_pl_delete (GtkWidget *, struct maker_pl_data_t *);
static gboolean win_maker_pl_refresh (GtkWidget * window);
static void win_maker_pl_destroy (GtkWidget * window);

static void win_maker_spot_update (GtkWidget *, struct maker_spot_data_t *);
static void win_maker_spot_save (GtkWidget *, struct maker_spot_data_t *);
static void win_maker_spot_save_as (GtkWidget *, struct maker_spot_data_t *);
static void win_maker_spot_show_d (GtkWidget *, struct maker_spot_data_t *);
static int win_maker_spot_show (GtkWidget *, GdkEvent *,
				struct maker_spot_data_t *);
static void win_maker_spot_undo (GtkWidget *, struct maker_spot_data_t *);
static void win_maker_spot_redo (GtkWidget *, struct maker_spot_data_t *);
static void win_maker_spot_undo_history (GtkWidget *,
					 struct maker_spot_data_t *);
static void win_maker_spot_redo_history (GtkWidget *,
					 struct maker_spot_data_t *);
static void win_maker_spot_cut (GtkWidget *, struct maker_spot_data_t *);
static void win_maker_spot_copy (GtkWidget *, struct maker_spot_data_t *);
static void win_maker_spot_paste (GtkWidget *, struct maker_spot_data_t *);
static void win_maker_spot_delete (GtkWidget *, struct maker_spot_data_t *);
static gboolean win_maker_spot_refresh (GtkWidget * window);
static void win_maker_spot_destroy (GtkWidget * window);

static void
win_maker_pl_show_d (GtkWidget * w, struct maker_pl_data_t *data)
{
  if ((maker_pl_is_no_saved (data)
       && dialog_ask (_("Exit without save?")) == GTK_RESPONSE_OK)
      || !maker_pl_is_no_saved (data))
    gtk_widget_destroy (gtk_widget_get_toplevel (w));
}

static int
win_maker_pl_show (GtkWidget * w, GdkEvent * event,
		   struct maker_pl_data_t *data)
{
  if ((maker_pl_is_no_saved (data)
       && dialog_ask (_("Exit without save?")) == GTK_RESPONSE_OK)
      || !maker_pl_is_no_saved (data))
    {
      return FALSE;
    }

  return TRUE;
}

void
win_maker_pl_create (void)
{
  GtkWidget *window;
  GtkWidget *w;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *ubox;
  GtkWidget *vubox;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *toolbar;
  GtkWidget *sep;
  GtkWidget *alignment;

  GtkWidget *undo_widget, *redo_widget;
  GtkWidget *undo_history, *redo_history;

  gchar s[1024];
  gint y, x;
  guint id;

  snprintf (s, sizeof (s), "%s %s", PACKAGE, VERSION);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), s);
  gtk_window_set_default_icon_from_file (PATH_ICON, NULL);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);

  y = gdk_screen_height () * 2 / 3;
  if (y < 480)
    y = 480;

  x = gdk_screen_width () * 2 / 3;
  if (x < 600)
    x = 600;

  gtk_widget_set_size_request (window, x, y);
  g_signal_connect ((gpointer) window, "delete_event",
		    G_CALLBACK (win_maker_pl_show), &maker_pl_data);

  vbox = gtk_vbox_new (0, FALSE);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  toolbar = gtk_toolbar_new ();
  gtk_widget_show (toolbar);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-save");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_save), &maker_pl_data);

  gtk_widget_set_sensitive (button, FALSE);
  win_maker_pl_save_toolbar = button;

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-save-as");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_save_as), &maker_pl_data);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_item_new ();
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);

  ubox = gtk_hbox_new (0, 0);
  gtk_widget_show (ubox);
  gtk_container_add (GTK_CONTAINER (button), ubox);

  undo_widget = button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_undo), &maker_pl_data);

  vubox = gtk_vbox_new (0, 0);
  gtk_widget_show (vubox);
  gtk_container_add (GTK_CONTAINER (button), vubox);

  image = gtk_image_new_from_stock ("gtk-undo", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  image = gtk_label_new (_("undo"));
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  undo_history = button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_undo_history),
		    &maker_pl_data);

  image = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  button = (GtkWidget *) gtk_tool_item_new ();
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);

  ubox = gtk_hbox_new (0, 0);
  gtk_widget_show (ubox);
  gtk_container_add (GTK_CONTAINER (button), ubox);

  redo_widget = button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_redo), &maker_pl_data);

  vubox = gtk_vbox_new (0, 0);
  gtk_widget_show (vubox);
  gtk_container_add (GTK_CONTAINER (button), vubox);

  image = gtk_image_new_from_stock ("gtk-redo", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  image = gtk_label_new (_("redo"));
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  redo_history = button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_redo_history),
		    &maker_pl_data);

  image = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-cut");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_cut), &maker_pl_data);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-copy");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_copy), &maker_pl_data);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-paste");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_paste), &maker_pl_data);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-delete");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_pl_delete), &maker_pl_data);

  if (local_connect)
    w = create_maker_pl (&maker_pl_data, controller, TRUE);
  else
    w = create_maker_pl (&maker_pl_data, controller, FALSE);

  gtk_widget_show (w);
  gtk_box_pack_start (GTK_BOX (vbox), w, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (win_maker_pl_change_color), &maker_pl_data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-select-color", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Change color"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  win_maker_pl_save_button = gtk_button_new ();
  gtk_widget_show (win_maker_pl_save_button);
  gtk_box_pack_start (GTK_BOX (hbox), win_maker_pl_save_button, TRUE, TRUE,
		      0);

  g_signal_connect ((gpointer) win_maker_pl_save_button, "clicked",
		    G_CALLBACK (win_maker_pl_save), &maker_pl_data);

  gtk_widget_set_sensitive (win_maker_pl_save_button, FALSE);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (win_maker_pl_save_button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-save", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Save on file"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (win_maker_pl_save_as), &maker_pl_data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-save-as", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Save on a new file"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (win_maker_pl_update), &maker_pl_data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Update the palinsesto of somad"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (win_maker_pl_show_d), &maker_pl_data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Close this window"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  id = g_timeout_add (200, (GSourceFunc) win_maker_pl_refresh, window);
  g_object_set_data (G_OBJECT (window), "refresh", (gpointer) id);
  g_object_set_data (G_OBJECT (window), "undo_widget", undo_widget);
  g_object_set_data (G_OBJECT (window), "redo_widget", redo_widget);
  g_object_set_data (G_OBJECT (window), "undo_history", undo_history);
  g_object_set_data (G_OBJECT (window), "redo_history", redo_history);
  g_signal_connect ((gpointer) window, "destroy",
		    G_CALLBACK (win_maker_pl_destroy), NULL);

  gtk_widget_show (window);
}

static gboolean
win_maker_pl_refresh (GtkWidget * window)
{
  gboolean u, r;

  GtkWidget *undo_widget =
    g_object_get_data (G_OBJECT (window), "undo_widget");
  GtkWidget *redo_widget =
    g_object_get_data (G_OBJECT (window), "redo_widget");
  GtkWidget *undo_history =
    g_object_get_data (G_OBJECT (window), "undo_history");
  GtkWidget *redo_history =
    g_object_get_data (G_OBJECT (window), "redo_history");

  maker_pl_undoredo_status (&maker_pl_data, &u, &r);

  gtk_widget_set_sensitive (undo_widget, u);
  gtk_widget_set_sensitive (redo_widget, r);
  gtk_widget_set_sensitive (undo_history, u);
  gtk_widget_set_sensitive (redo_history, r);
  return TRUE;
}

static void
win_maker_pl_destroy (GtkWidget * window)
{
  guint id = (guint) g_object_get_data (G_OBJECT (window), "refresh");

  if (id)
    {
      g_source_remove (id);
      g_object_steal_data (G_OBJECT (window), "refresh");
    }
}

static void
win_maker_pl_update (GtkWidget * w, struct maker_pl_data_t *data)
{
  gchar s[1024], *file;
  int d = 0;

  maker_pl_sync (data);

  while (d < 1000)
    {
      snprintf (s, sizeof (s), "somax_pls_%.3d.cfg", d);
      file = g_build_path (G_DIR_SEPARATOR_S, g_get_tmp_dir (), s, NULL);

      if (g_file_test (file, G_FILE_TEST_EXISTS) == FALSE)
	break;

      g_free (file);
      d++;
    }

  if (d == 1000)
    {
      snprintf (s, sizeof (s), "somax_pls_%.3d.cfg", getrandom (0, 1000));
      file = g_build_path (G_DIR_SEPARATOR_S, g_get_tmp_dir (), s, NULL);
    }

  if (palinsesto_save_file (file, data->maker_pl))
    {
      dialog_msg (_("Error writing on file."));
      g_free (file);
      return;
    }

  maker_pl_file_saved (data);

  if (somax_new_palinsesto_file (data->controller, file))
    dialog_msg (_("Error changing palinsesto file."));
  else
    dialog_msg (_("Palinsesto updated!"));

  unlink (file);
  g_free (file);
}

static void
win_maker_pl_save (GtkWidget * w, struct maker_pl_data_t *data)
{
  if (!win_maker_pl_file)
    {
      win_maker_pl_save_as (NULL, data);
      return;
    }

  maker_pl_sync (data);

  if (palinsesto_save_file (win_maker_pl_file, data->maker_pl))
    {
      dialog_msg (_("Error writing on file."));
      return;
    }

  dialog_msg (_("Palinsesto saved!"));

  maker_pl_file_saved (data);
}

static void
win_maker_pl_save_as (GtkWidget * w, struct maker_pl_data_t *data)
{
  gchar *file;

  if (!
      (file =
       file_chooser (_("Save your palinsesto"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SAVE)))
    return;

  if (g_file_test (file, G_FILE_TEST_EXISTS) == TRUE
      && dialog_ask (_("The file exists. Overwrite?")) != GTK_RESPONSE_OK)
    {
      g_free (file);
      return;
    }

  maker_pl_sync (data);

  if (palinsesto_save_file (file, data->maker_pl))
    {
      dialog_msg (_("Error writing on file."));
      g_free (file);
      return;
    }

  dialog_msg (_("Palinsesto saved!"));

  gtk_widget_set_sensitive (win_maker_pl_save_button, TRUE);
  gtk_widget_set_sensitive (win_maker_pl_save_toolbar, TRUE);

  maker_pl_file_saved (data);

  if (win_maker_pl_file)
    g_free (win_maker_pl_file);

  win_maker_pl_file = file;
}

static void
win_maker_pl_change_color (GtkWidget * w, struct maker_pl_data_t *data)
{
  maker_pl_change_color (data);
}

static void
win_maker_pl_undo (GtkWidget * w, struct maker_pl_data_t *data)
{
  maker_pl_undoredo_work (data, TRUE);
}

static void
win_maker_pl_redo (GtkWidget * w, struct maker_pl_data_t *data)
{
  maker_pl_undoredo_work (data, FALSE);
}

static void
win_maker_pl_undo_history (GtkWidget * w, struct maker_pl_data_t *data)
{
  maker_pl_undoredo_history (data, TRUE, w);
}

static void
win_maker_pl_redo_history (GtkWidget * w, struct maker_pl_data_t *data)
{
  maker_pl_undoredo_history (data, FALSE, w);
}

static void
win_maker_pl_cut (GtkWidget * w, struct maker_pl_data_t *data)
{
  struct somad_data *pl;

  if (!data->pl)
    return;

  if (!(pl = maker_pl_dump_pl (data)))
    return;

  maker_pl_remove_pl (data);

  palinsesto_free (dump_pl);
  dump_pl = pl;
}

static void
win_maker_pl_copy (GtkWidget * w, struct maker_pl_data_t *data)
{
  struct somad_data *pl;

  if (!(pl = maker_pl_dump_pl (data)))
    return;

  palinsesto_free (dump_pl);
  dump_pl = pl;
}

static void
win_maker_pl_paste (GtkWidget * w, struct maker_pl_data_t *data)
{
  if (!dump_pl)
    return;

  maker_pl_insert_pl (data, palinsesto_dump (dump_pl));
}

static void
win_maker_pl_delete (GtkWidget * w, struct maker_pl_data_t *data)
{
  if (!data->pl)
    return;

  if (dialog_ask (_("Sure to remove this transmission?")) != GTK_RESPONSE_OK)
    return;

  maker_pl_remove_pl (data);
}

static void
win_maker_spot_show_d (GtkWidget * w, struct maker_spot_data_t *data)
{
  if ((maker_spot_is_no_saved (data)
       && dialog_ask (_("Exit without save?")) == GTK_RESPONSE_OK)
      || !maker_spot_is_no_saved (data))
    gtk_widget_destroy (gtk_widget_get_toplevel (w));
}

static int
win_maker_spot_show (GtkWidget * w, GdkEvent * event,
		     struct maker_spot_data_t *data)
{
  if ((maker_spot_is_no_saved (data)
       && dialog_ask (_("Exit without save?")) == GTK_RESPONSE_OK)
      || !maker_spot_is_no_saved (data))
    {
      return FALSE;
    }

  return TRUE;
}

void
win_maker_spot_create (void)
{
  GtkWidget *window;
  GtkWidget *w;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *ubox;
  GtkWidget *vubox;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *toolbar;
  GtkWidget *sep;
  GtkWidget *alignment;

  GtkWidget *undo_widget, *redo_widget;
  GtkWidget *undo_history, *redo_history;

  gchar s[1024];
  gint y, x;
  guint id;

  snprintf (s, sizeof (s), "%s %s", PACKAGE, VERSION);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), s);
  gtk_window_set_default_icon_from_file (PATH_ICON, NULL);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);

  y = gdk_screen_height () * 2 / 3;
  if (y < 480)
    y = 480;

  x = gdk_screen_width () * 2 / 3;
  if (x < 600)
    x = 600;

  gtk_widget_set_size_request (window, x, y);

  g_signal_connect ((gpointer) window, "delete_event",
		    G_CALLBACK (win_maker_spot_show), &maker_spot_data);

  vbox = gtk_vbox_new (0, FALSE);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  toolbar = gtk_toolbar_new ();
  gtk_widget_show (toolbar);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-save");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_save), &maker_spot_data);

  gtk_widget_set_sensitive (button, FALSE);
  win_maker_spot_save_toolbar = button;

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-save-as");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_save_as),
		    &maker_spot_data);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_item_new ();
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);

  ubox = gtk_hbox_new (0, 0);
  gtk_widget_show (ubox);
  gtk_container_add (GTK_CONTAINER (button), ubox);

  undo_widget = button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_undo), &maker_spot_data);

  vubox = gtk_vbox_new (0, 0);
  gtk_widget_show (vubox);
  gtk_container_add (GTK_CONTAINER (button), vubox);

  image = gtk_image_new_from_stock ("gtk-undo", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  image = gtk_label_new (_("undo"));
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  undo_history = button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_undo_history),
		    &maker_spot_data);

  image = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  button = (GtkWidget *) gtk_tool_item_new ();
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);

  ubox = gtk_hbox_new (0, 0);
  gtk_widget_show (ubox);
  gtk_container_add (GTK_CONTAINER (button), ubox);

  redo_widget = button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_redo), &maker_spot_data);

  vubox = gtk_vbox_new (0, 0);
  gtk_widget_show (vubox);
  gtk_container_add (GTK_CONTAINER (button), vubox);

  image = gtk_image_new_from_stock ("gtk-redo", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  image = gtk_label_new (_("redo"));
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  redo_history = button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_redo_history),
		    &maker_spot_data);

  image = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-cut");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_cut), &maker_spot_data);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-copy");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_copy), &maker_spot_data);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-paste");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_paste), &maker_spot_data);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-delete");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (win_maker_spot_delete),
		    &maker_spot_data);

  if (local_connect)
    w = create_maker_spot (&maker_spot_data, controller, TRUE);
  else
    w = create_maker_spot (&maker_spot_data, controller, FALSE);

  gtk_widget_show (w);
  gtk_box_pack_start (GTK_BOX (vbox), w, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  win_maker_spot_save_button = gtk_button_new ();
  gtk_widget_show (win_maker_spot_save_button);
  gtk_box_pack_start (GTK_BOX (hbox), win_maker_spot_save_button, TRUE, TRUE,
		      0);

  g_signal_connect ((gpointer) win_maker_spot_save_button, "clicked",
		    G_CALLBACK (win_maker_spot_save), &maker_spot_data);

  gtk_widget_set_sensitive (win_maker_spot_save_button, FALSE);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (win_maker_spot_save_button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-save", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Save on file"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (win_maker_spot_save_as), &maker_spot_data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-save-as", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Save on a new file"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (win_maker_spot_update), &maker_spot_data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Update the spot file of somad"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (win_maker_spot_show_d), &maker_spot_data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Close this window"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  id = g_timeout_add (200, (GSourceFunc) win_maker_spot_refresh, window);
  g_object_set_data (G_OBJECT (window), "refresh", (gpointer) id);
  g_object_set_data (G_OBJECT (window), "undo_widget", undo_widget);
  g_object_set_data (G_OBJECT (window), "undo_widget", undo_widget);
  g_object_set_data (G_OBJECT (window), "redo_history", redo_history);
  g_object_set_data (G_OBJECT (window), "redo_history", redo_history);
  g_signal_connect ((gpointer) window, "destroy",
		    G_CALLBACK (win_maker_spot_destroy), NULL);

  gtk_widget_show (window);
}

static gboolean
win_maker_spot_refresh (GtkWidget * window)
{
  gboolean u, r;

  GtkWidget *undo_widget =
    g_object_get_data (G_OBJECT (window), "undo_widget");
  GtkWidget *redo_widget =
    g_object_get_data (G_OBJECT (window), "redo_widget");
  GtkWidget *undo_history =
    g_object_get_data (G_OBJECT (window), "undo_history");
  GtkWidget *redo_history =
    g_object_get_data (G_OBJECT (window), "redo_history");

  maker_spot_undoredo_status (&maker_spot_data, &u, &r);

  gtk_widget_set_sensitive (undo_widget, u);
  gtk_widget_set_sensitive (redo_widget, r);
  gtk_widget_set_sensitive (undo_history, u);
  gtk_widget_set_sensitive (redo_history, r);
  return TRUE;
}

static void
win_maker_spot_destroy (GtkWidget * window)
{
  guint id = (guint) g_object_get_data (G_OBJECT (window), "refresh");

  if (id)
    {
      g_source_remove (id);
      g_object_steal_data (G_OBJECT (window), "refresh");
    }
}

static void
win_maker_spot_save (GtkWidget * w, struct maker_spot_data_t *data)
{
  if (!win_maker_spot_file)
    {
      win_maker_spot_save_as (NULL, data);
      return;
    }

  maker_spot_sync (data);

  if (spot_save_file (win_maker_spot_file, data->maker_spot))
    {
      dialog_msg (_("Error writing on file."));
      return;
    }

  dialog_msg (_("Spot file saved!"));

  maker_spot_file_saved (data);
}

static void
win_maker_spot_save_as (GtkWidget * w, struct maker_spot_data_t *data)
{
  gchar *file;

  if (!
      (file =
       file_chooser (_("Save your spot file"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SAVE)))
    return;

  if (g_file_test (file, G_FILE_TEST_EXISTS) == TRUE
      && dialog_ask (_("The file exists. Overwrite?")) != GTK_RESPONSE_OK)
    {
      g_free (file);
      return;
    }

  maker_spot_sync (data);

  if (spot_save_file (file, data->maker_spot))
    {
      dialog_msg (_("Error writing on file."));
      g_free (file);
      return;
    }

  dialog_msg (_("Spot file saved!"));

  gtk_widget_set_sensitive (win_maker_spot_save_button, TRUE);
  gtk_widget_set_sensitive (win_maker_spot_save_toolbar, TRUE);

  maker_spot_file_saved (data);

  if (win_maker_spot_file)
    g_free (win_maker_spot_file);

  win_maker_spot_file = file;
}

static void
win_maker_spot_update (GtkWidget * w, struct maker_spot_data_t *data)
{
  gchar s[1024], *file;
  int d = 0;

  maker_spot_sync (data);

  while (d < 1000)
    {
      snprintf (s, sizeof (s), "somax_spots_%.3d.cfg", d);
      file = g_build_path (G_DIR_SEPARATOR_S, g_get_tmp_dir (), s, NULL);

      if (g_file_test (file, G_FILE_TEST_EXISTS) == FALSE)
	break;

      g_free (file);
      d++;
    }

  if (d == 1000)
    {
      snprintf (s, sizeof (s), "somax_spots_%.3d.cfg", getrandom (0, 1000));
      file = g_build_path (G_DIR_SEPARATOR_S, g_get_tmp_dir (), s, NULL);
    }

  if (spot_save_file (file, data->maker_spot))
    {
      dialog_msg (_("Error writing on file."));
      g_free (file);
      return;
    }

  maker_spot_file_saved (data);

  if (somax_new_spot_file (data->controller, file))
    dialog_msg (_("Error changing spot file."));
  else
    dialog_msg (_("Spot updated!"));

  unlink (file);
  g_free (file);
}

static void
win_maker_spot_undo (GtkWidget * w, struct maker_spot_data_t *data)
{
  maker_spot_undoredo_work (data, TRUE);
}

static void
win_maker_spot_redo (GtkWidget * w, struct maker_spot_data_t *data)
{
  maker_spot_undoredo_work (data, FALSE);
}

static void
win_maker_spot_undo_history (GtkWidget * w, struct maker_spot_data_t *data)
{
  maker_spot_undoredo_history (data, TRUE, w);
}

static void
win_maker_spot_redo_history (GtkWidget * w, struct maker_spot_data_t *data)
{
  maker_spot_undoredo_history (data, FALSE, w);
}

static void
win_maker_spot_cut (GtkWidget * w, struct maker_spot_data_t *data)
{
  struct somad_spot_data *spot;

  if (!data->spot)
    return;

  if (!(spot = maker_spot_dump_spot (data)))
    return;

  maker_spot_remove_spot (data);
  spot_free (dump_spot);
  dump_spot = spot;
}

static void
win_maker_spot_copy (GtkWidget * w, struct maker_spot_data_t *data)
{
  struct somad_spot_data *spot;

  if (!(spot = maker_spot_dump_spot (data)))
    return;

  spot_free (dump_spot);
  dump_spot = spot;
}

static void
win_maker_spot_paste (GtkWidget * w, struct maker_spot_data_t *data)
{
  if (!dump_spot)
    return;

  maker_spot_insert_spot (data, spot_dump (dump_spot));
}

static void
win_maker_spot_delete (GtkWidget * w, struct maker_spot_data_t *data)
{
  if (!data->spot)
    return;

  if (dialog_ask (_("Sure to remove this spot element?")) != GTK_RESPONSE_OK)
    return;

  maker_spot_remove_spot (data);
}

/* EOF */
