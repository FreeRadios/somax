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
#include "../commons/stat.h"
#include "../commons/x.h"
#include "maker.h"
#include "../draw/draw.h"
#include "../palinsesto/palinsesto.h"
#include "../filechooser/filechooser.h"

#define MAKER_ADD_LABEL( x ) label = gtk_label_new( x ); \
  gtk_widget_show(label); \
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); \
  gtk_table_attach(GTK_TABLE(table), label, 0,1, i, i+1, \
		   GTK_FILL, 0, 0, 0);

#define MAKER_ADD_WIDGET( x ) gtk_widget_show( x ); \
  gtk_table_attach(GTK_TABLE(table), x , 1,2, i, i+1, GTK_FILL | GTK_EXPAND, \
		   0, 0, 0);

static void maker_spot_new_t (GtkWidget *, struct maker_spot_data_t *);
static void maker_spot_save_t (GtkWidget *, struct maker_spot_data_t *);
static void maker_spot_save_real_t (GtkWidget * w,
				    struct maker_spot_data_t *data,
				    struct somad_spot_data *spot);
static void maker_spot_remove_t (GtkWidget *, struct maker_spot_data_t *);
static void maker_spot_list_refresh (struct maker_spot_data_t *);
static void maker_spot_undoredo (GtkWidget * w,
				 struct maker_spot_data_t *data);
static gboolean maker_spot_undoredo_timeout (struct maker_spot_data_t *data);
static void maker_spot_treeview_l_add (GtkWidget *, GtkWidget *);
static void maker_spot_treeview_l_add_dir (GtkWidget *, GtkWidget *);
static void maker_spot_treeview_remove (GtkWidget *, GtkWidget *);
static void maker_spot_treeview_l_add_cb (void *l, void *t);
static void maker_spot_treeview_l_add_dir_cb (void *l, void *t);
static void maker_spot_duration_path (GtkWidget *,
				      struct maker_spot_data_t *);
static void maker_spot_duration (struct maker_spot_data_t *, GtkWidget *,
				 GtkWidget *);
static void maker_spot_selected (GtkTreeView *, GtkTreePath *,
				 GtkTreeViewColumn *,
				 struct maker_spot_data_t *);
static void maker_spot_remove_spot_real (struct maker_spot_data_t *data,
					 struct somad_spot_data *spot,
					 gboolean undo, gboolean insert);
static void maker_spot_undoredo_new (struct maker_spot_data_t *,
				     enum maker_undoredo_type_t,
				     gboolean undo, gchar * message);
static void maker_spot_undoredo_free (struct maker_spot_undoredo_t *);
static void maker_spot_destroy (GtkWidget * w,
				struct maker_spot_data_t *data);

/* Create the window */
GtkWidget *
create_maker_spot (struct maker_spot_data_t *data, void *c, gboolean local)
{
  GtkWidget *scrolledwindow;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *t;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *alignment;
  GtkWidget *table;
  GtkWidget *entry;
  GtkWidget *vport;
  GtkObject *adj;
  GtkListStore *model;
  GtkTreeSelection *selection;
  GtkCellRenderer *renderer;
  GtkTooltips *tooltips;
  struct tm *k;

  int j, i = 0;

  char s[1024];

  if (!(k = get_time ()))
    return NULL;

  tooltips = gtk_tooltips_new ();

  /* For the editor, the controller must be saved in a struct data */
  /* Editor has this NULL, somax no. */
  data->controller = c;
  data->local = local;
  data->spot = NULL;

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (hbox), box, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (box), 8);

  frame = gtk_frame_new (_("Spot element"));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
  gtk_widget_set_size_request (frame, 300, -1);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_container_add (GTK_CONTAINER (frame), scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  data->frame_list = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (data->frame_list);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), data->frame_list);

  frame = gtk_frame_new (_("Edit your spot element"));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_set_sensitive (frame, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
  data->frame = frame;

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scrolledwindow);

  vport = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (vport);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), vport);

  table = gtk_table_new (0, 0, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (vport), table);

  MAKER_ADD_LABEL (_("Description"));

  entry = gtk_entry_new ();
  MAKER_ADD_WIDGET (entry);
  data->description = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Description changed to: %s"));

  i++;

  MAKER_ADD_LABEL (_("Start:"));

  t = gtk_table_new (2, 6, FALSE);
  gtk_widget_show (t);
  gtk_table_attach (GTK_TABLE (table), t, 1, 2, i, i + 1,
		    GTK_FILL | GTK_EXPAND, 0, 0, 0);

  label = gtk_label_new (_(" Y "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  entry = gtk_combo_box_new_text ();
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->start_year = entry;
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("any"));
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "add", (gpointer) k->tm_year + 1899);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Start Year changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  /* 2100=1900+200 */
  for (j = k->tm_year + 1900; j < k->tm_year + 2100; j++)
    {
      snprintf (s, sizeof (s), "%d", j);
      gtk_combo_box_append_text (GTK_COMBO_BOX (entry), s);
    }

  label = gtk_label_new (_(" M "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 2, 3, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  entry = gtk_combo_box_new_text ();
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->start_month = entry;
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("any"));
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Start Month changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  for (j = 1; j < 13; j++)
    {
      snprintf (s, sizeof (s), "%s", somax_month (j - 1));
      gtk_combo_box_append_text (GTK_COMBO_BOX (entry), s);
    }

  label = gtk_label_new (_(" D "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 4, 5, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  entry = gtk_combo_box_new_text ();
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->start_mday = entry;
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("any"));
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Start Day changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 5, 6, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  for (j = 1; j < 32; j++)
    {
      snprintf (s, sizeof (s), "%d", j);
      gtk_combo_box_append_text (GTK_COMBO_BOX (entry), s);
    }

  label = gtk_label_new (_(" day "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  entry = gtk_combo_box_new_text ();
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->start_wday = entry;
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("any"));
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Start Weekly Day changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  for (j = 0; j < 7; j++)
    gtk_combo_box_append_text (GTK_COMBO_BOX (entry), somax_day (j));

  label = gtk_label_new (_(" h "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 2, 3, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  adj = gtk_adjustment_new (0, 0, 24, 1, 10, 10);
  entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_widget_show (entry);
  data->start_hour = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Start Hour changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 3, 4, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  label = gtk_label_new (_(" m "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 4, 5, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  adj = gtk_adjustment_new (0, 0, 60, 1, 10, 10);
  entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->start_min = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Start Minute changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 5, 6, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  i++;

  MAKER_ADD_LABEL (_("Stop:"));

  t = gtk_table_new (2, 6, FALSE);
  gtk_widget_show (t);
  gtk_table_attach (GTK_TABLE (table), t, 1, 2, i, i + 1,
		    GTK_FILL | GTK_EXPAND, 0, 0, 0);

  label = gtk_label_new (_(" Y "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  entry = gtk_combo_box_new_text ();
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->stop_year = entry;
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("any"));
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "add", (gpointer) k->tm_year + 1899);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Year changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  /* 2100=1900+200 */
  for (j = k->tm_year + 1900; j < k->tm_year + 2100; j++)
    {
      snprintf (s, sizeof (s), "%d", j);
      gtk_combo_box_append_text (GTK_COMBO_BOX (entry), s);
    }

  label = gtk_label_new (_(" M "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 2, 3, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  entry = gtk_combo_box_new_text ();
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->stop_month = entry;
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("any"));
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Month changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  for (j = 1; j < 13; j++)
    {
      snprintf (s, sizeof (s), "%s", somax_month (j - 1));
      gtk_combo_box_append_text (GTK_COMBO_BOX (entry), s);
    }

  label = gtk_label_new (_(" D "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 4, 5, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  entry = gtk_combo_box_new_text ();
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->stop_mday = entry;
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("any"));
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Day changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 5, 6, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  for (j = 1; j < 32; j++)
    {
      snprintf (s, sizeof (s), "%d", j);
      gtk_combo_box_append_text (GTK_COMBO_BOX (entry), s);
    }

  label = gtk_label_new (_(" day "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  entry = gtk_combo_box_new_text ();
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->stop_wday = entry;
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("any"));
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Weekly Day changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  for (j = 0; j < 7; j++)
    gtk_combo_box_append_text (GTK_COMBO_BOX (entry), somax_day (j));

  label = gtk_label_new (_(" h "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 2, 3, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  adj = gtk_adjustment_new (0, 0, 24, 1, 10, 10);
  entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_widget_show (entry);
  data->stop_hour = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Hour changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 3, 4, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  label = gtk_label_new (_(" m "));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (t), label, 4, 5, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  adj = gtk_adjustment_new (0, 0, 60, 1, 10, 10);
  entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_widget_set_size_request (entry, 80, -1);
  gtk_widget_show (entry);
  data->stop_min = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Minute changed to: %s"));
  gtk_table_attach (GTK_TABLE (t), entry, 5, 6, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);

  i++;

  MAKER_ADD_LABEL (_("TimeContinued:"));

  entry =
    gtk_check_button_new_with_mnemonic (_("active timecontinued attribute"));
  MAKER_ADD_WIDGET (entry);
  data->timecontinued = entry;
  g_signal_connect ((gpointer) entry, "toggled",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("TimeContinued changed to: %s"));

  i++;

  MAKER_ADD_LABEL (_("Repeat:"));

  adj = gtk_adjustment_new (0, 0, 99, 1, 10, 10);
  entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  MAKER_ADD_WIDGET (entry);
  data->repeat = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_spot_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message", _("Repeat changed to: %s"));

  i++;

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_table_attach (GTK_TABLE (table), scrolledwindow, 0, 2, i, i + 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  model = gtk_list_store_new (1, G_TYPE_STRING);

  entry = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (entry), TRUE);
  g_object_set_data (G_OBJECT (entry), "data_spot", data);

  gtk_widget_set_size_request (entry, -1, 150);

  g_signal_connect (entry, "row_activated", G_CALLBACK (maker_spot_selected),
		    data);

  data->path = entry;

  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (entry), FALSE);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), entry);
  gtk_widget_show (entry);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (entry));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (entry), TRUE);

  g_object_unref (model);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (entry),
					       -1, _("Path"), renderer,
					       "text", 0, NULL);

  i++;

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_table_attach (GTK_TABLE (table), box, 0, 2, i, i + 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (0), 0, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_treeview_r_add), data->path);
  maker_controller_check (data->controller, button,
			  &data->controller_widgets);
  gtk_tooltips_set_tip (tooltips, button, _("Remote Add Files"), NULL);

  image =
    gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
			     "maker_file_remote.png");
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);


  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_treeview_r_add_dir), data->path);
  maker_controller_check (data->controller, button,
			  &data->controller_widgets);
  gtk_tooltips_set_tip (tooltips, button, _("Remote Add Directories"), NULL);

  image =
    gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
			     "maker_dir_remote.png");
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  if (data->local == TRUE)
    {
      button = gtk_button_new ();
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (maker_spot_treeview_l_add), data->path);
      gtk_tooltips_set_tip (tooltips, button, _("Local Add Files"), NULL);

      image =
	gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
				 "maker_file_locale.png");
      gtk_widget_show (image);
      gtk_container_add (GTK_CONTAINER (button), image);

      button = gtk_button_new ();
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (maker_spot_treeview_l_add_dir),
			data->path);
      gtk_tooltips_set_tip (tooltips, button, _("Local Add Directories"),
			    NULL);

      image =
	gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
				 "maker_dir_locale.png");
      gtk_widget_show (image);
      gtk_container_add (GTK_CONTAINER (button), image);
    }

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_spot_treeview_remove), data->path);
  gtk_tooltips_set_tip (tooltips, button, _("Remove selected items"), NULL);

  image =
    gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
			     "maker_remove.png");
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  i++;

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_table_attach (GTK_TABLE (table), box, 0, 2, i, i + 1,
		    (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0,
		    0);

  button = gtk_button_new_with_label (_("Check Duration"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_spot_duration_path), data);

  label = gtk_label_new (_("Duration: n/a"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  data->duration_path = label;

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_spot_new_t), data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("New Spot"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_spot_remove_t), data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-remove", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Remove Spot"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  data->frame_list_w = NULL;

  if (data->maker_spot)
    maker_spot_data_show (data->maker_spot, data);

  maker_spot_list_refresh (data);

  g_signal_connect ((gpointer) vbox, "delete_event",
		    G_CALLBACK (maker_spot_destroy), data);

  return vbox;
}

void
maker_spot_data_show (struct somad_spot_data *spot,
		      struct maker_spot_data_t *data)
{
  int j;
  GtkListStore *store;
  GtkTreeIter iter;
  GList *list;
  struct tm *k;

  if (!(k = get_time ()))
    return;

  data->spot = NULL;

  if (!spot)
    return;

  data->refresh_lock = TRUE;

  data->spot = spot;
  gtk_widget_set_sensitive (data->frame, TRUE);

  gtk_entry_set_text (GTK_ENTRY (data->description), data->spot->description);

  if (spot->timer->start_year == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_year), 0);

  else
    {
      j = spot->timer->start_year - (k->tm_year + 1900);
      gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_year),
				j > -1 ? j + 1 : 0);
    }

  if (spot->timer->start_month == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_month), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_month),
			      spot->timer->start_month + 1);

  if (spot->timer->start_mday == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_mday), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_mday),
			      spot->timer->start_mday);

  if (spot->timer->start_wday == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_wday), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_wday),
			      spot->timer->start_wday + 1);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->start_hour),
			     spot->timer->start_hour);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->start_min),
			     spot->timer->start_min);

  if (spot->timer->stop_year == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_year), 0);

  else
    {
      j = spot->timer->stop_year - (k->tm_year + 1900);
      gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_year),
				j > -1 ? j + 1 : 0);
    }

  if (spot->timer->stop_month == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_month), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_month),
			      spot->timer->stop_month + 1);

  if (spot->timer->stop_mday == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_mday), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_mday),
			      spot->timer->stop_mday);

  if (spot->timer->stop_wday == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_wday), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_wday),
			      spot->timer->stop_wday + 1);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->stop_hour),
			     spot->timer->stop_hour);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->stop_min),
			     spot->timer->stop_min);

  if (spot->timer->timecontinued)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->timecontinued),
				  TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->timecontinued),
				  FALSE);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->repeat), spot->repeat);

  gtk_label_set_text (GTK_LABEL (data->duration_path), _("Duration: n/a"));

  store =
    (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW ((data->path)));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  list = data->spot->path;
  while (list)
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, list->data, -1);
      list = list->next;
    }

  data->refresh_lock = FALSE;
  maker_spot_refresh (data);
}

static void
maker_spot_save_t (GtkWidget * w, struct maker_spot_data_t *data)
{
  maker_spot_save_real_t (w, data, data->spot);
  maker_spot_refresh (data);
}

static void
maker_spot_save_real_t (GtkWidget * w, struct maker_spot_data_t *data,
			struct somad_spot_data *spot)
{
  int i;
  struct tm *k;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *c;

  if (!(k = get_time ()))
    return;

  if (spot)
    {

      if (spot->description)
	g_free (spot->description);

      if ((c = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->description)))
	  && *c)
	spot->description = g_strdup (c);
      else
	spot->description = NULL;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->start_year));
      if (i == 0)
	i = -1;
      else
	i += k->tm_year + 1900 - 1;

      spot->timer->start_year = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->start_month));
      if (i == 0)
	i = -1;
      else
	i -= 1;

      spot->timer->start_month = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->start_mday));
      if (i == 0)
	i = -1;

      spot->timer->start_mday = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->start_wday));
      if (i == 0)
	i = -1;
      else
	i -= 1;
      spot->timer->start_wday = i;

      spot->timer->start_hour =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->start_hour));
      spot->timer->start_min =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->start_min));

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->stop_year));
      if (i == 0)
	i = -1;
      else
	i += k->tm_year + 1900 - 1;

      spot->timer->stop_year = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->stop_month));
      if (i == 0)
	i = -1;
      else
	i -= 1;

      spot->timer->stop_month = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->stop_mday));
      if (i == 0)
	i = -1;

      spot->timer->stop_mday = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->stop_wday));
      if (i == 0)
	i = -1;
      else
	i -= 1;
      spot->timer->stop_wday = i;

      spot->timer->stop_hour =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->stop_hour));
      spot->timer->stop_min =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->stop_min));

      spot->timer->timecontinued =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
				      (data->timecontinued));

      spot->repeat =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->repeat));

      if (spot->path)
	{
	  GList *list;

	  list = spot->path;
	  while (list)
	    {
	      g_free (list->data);
	      list = list->next;
	    }

	  g_list_free (spot->path);
	  spot->path = NULL;
	}

      model = gtk_tree_view_get_model (GTK_TREE_VIEW ((data->path)));
      if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
	{
	  char *text;

	  do
	    {
	      gtk_tree_model_get (model, &iter, 0, &text, -1);
	      spot->path = g_list_append (spot->path, text);
	    }
	  while (gtk_tree_model_iter_next (model, &iter) == TRUE);
	}

      if (spot->timer->start)
	g_free (spot->timer->start);

      spot->timer->start = NULL;

      if (spot->timer->stop)
	g_free (spot->timer->stop);

      spot->timer->stop = NULL;

      timer_time_to_string (spot->timer, &spot->timer->start,
			    &spot->timer->stop);
    }
}

void
maker_spot_sync (struct maker_spot_data_t *data)
{
  maker_spot_save_t (NULL, data);
}

void
maker_spot_file_saved (struct maker_spot_data_t *data)
{
  data->is_saved = 0;
}

int
maker_spot_is_no_saved (struct maker_spot_data_t *data)
{
  return data->is_saved;
}

static void
maker_spot_remove_t (GtkWidget * w, struct maker_spot_data_t *data)
{
  if (!data->spot)
    return;

  if (dialog_ask (_("Sure to remove this spot element?")) != GTK_RESPONSE_OK)
    return;

  maker_spot_remove_spot (data);
}

struct somad_spot_data *
maker_spot_dump_spot (struct maker_spot_data_t *data)
{
  struct somad_spot_data *tmp;
  GList *list;

  if (!data->spot)
    return NULL;

  tmp = g_malloc (sizeof (struct somad_spot_data));
  memcpy (tmp, data->spot, sizeof (struct somad_spot_data));

  list = data->spot->path;
  tmp->path = NULL;
  while (list)
    {
      tmp->path = g_list_append (tmp->path, g_strdup ((gchar *) list->data));
      list = list->next;
    }

  tmp->timer = g_malloc (sizeof (struct somad_time));
  memcpy (tmp->timer, data->spot->timer, sizeof (struct somad_time));

  if (data->spot->timer->start)
    tmp->timer->start = g_strdup (data->spot->timer->start);

  if (data->spot->timer->stop)
    tmp->timer->stop = g_strdup (data->spot->timer->stop);

  if (data->spot->description)
    tmp->description = g_strdup (data->spot->description);

  tmp->next = NULL;

  return tmp;
}

void
maker_spot_remove_spot (struct maker_spot_data_t *data)
{
  maker_spot_remove_spot_real (data, data->spot, TRUE, TRUE);
}


static void
maker_spot_remove_spot_real (struct maker_spot_data_t *data,
			     struct somad_spot_data *spot, gboolean undo,
			     gboolean insert)
{
  if (!spot)
    return;

  if (data->maker_spot == spot)
    {
      data->maker_spot = spot->next;
    }
  else
    {
      struct somad_spot_data *tmp;
      tmp = data->maker_spot;

      while (tmp->next)
	{
	  if (tmp->next == spot)
	    {
	      tmp->next = tmp->next->next;
	      break;
	    }

	  tmp = tmp->next;
	}
    }

  if (insert == TRUE)
    maker_spot_undoredo_new (data, MAKER_UNDOREDO_DESTROY, undo,
			     _("Spot removed"));

  gtk_widget_set_sensitive (data->frame, FALSE);

  maker_spot_refresh (data);
}

static void
maker_spot_new_t (GtkWidget * w, struct maker_spot_data_t *data)
{
  struct somad_spot_data *tmp;

  tmp = g_malloc0 (sizeof (struct somad_spot_data));
  tmp->description = g_strdup (_("No description"));

  tmp->timer = g_malloc0 (sizeof (struct somad_time));

  tmp->timer->start_year = -1;
  tmp->timer->start_month = -1;
  tmp->timer->start_mday = -1;
  tmp->timer->start_wday = -1;
  tmp->timer->stop_year = -1;
  tmp->timer->stop_month = -1;
  tmp->timer->stop_mday = -1;
  tmp->timer->stop_wday = -1;

  timer_time_to_string (tmp->timer, &tmp->timer->start, &tmp->timer->stop);

  maker_spot_insert_spot (data, tmp);
  maker_spot_undoredo_new (data, MAKER_UNDOREDO_NEW, TRUE,
			   _("New spot created"));
}

static void
maker_spot_dump_switch_spot (struct maker_spot_data_t *data)
{
  struct somad_spot_data *spot = maker_spot_dump_spot (data);

  if (data->maker_spot == data->spot)
    {
      spot->next = data->maker_spot->next;
      data->maker_spot = spot;
    }
  else
    {
      struct somad_spot_data *tmp = data->maker_spot;
      while (tmp->next)
	{
	  if (tmp->next == data->spot)
	    {
	      spot->next = tmp->next->next;
	      tmp->next = spot;
	      break;
	    }

	  tmp = tmp->next;
	}
    }

  data->spot = spot;
}

void
maker_spot_insert_spot (struct maker_spot_data_t *data,
			struct somad_spot_data *spot)
{
  spot_insert (spot, &data->maker_spot);
  maker_spot_data_show (spot, data);
}

static void
maker_spot_list_clicked (GtkWidget * w, struct maker_spot_data_t *data)
{
  maker_spot_data_show (g_object_get_data (G_OBJECT (w), "spot"), data);
}

static void
maker_spot_list_refresh (struct maker_spot_data_t *data)
{
  GtkWidget *box;
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *label;
  struct somad_spot_data *tmp;
  gchar *str;
  char s[1024];

  if (data->frame_list_w)
    gtk_container_remove (GTK_CONTAINER (data->frame_list),
			  data->frame_list_w);

  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (data->frame_list), box);

  data->frame_list_w = box;

  tmp = data->maker_spot;
  while (tmp)
    {
      button = gtk_button_new ();
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
      g_object_set_data (G_OBJECT (button), "spot", tmp);

      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (maker_spot_list_clicked), data);

      vbox = gtk_vbox_new (FALSE, 0);
      gtk_widget_show (vbox);
      gtk_container_add (GTK_CONTAINER (button), vbox);

      str = somax_markup (tmp->description);
      snprintf (s, sizeof (s), _("Description: <b>%s</b>"), str);
      g_free (str);

      label = gtk_label_new (s);

      gtk_widget_show (label);
      gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

      str = somax_markup (tmp->timer->start);
      snprintf (s, sizeof (s), _("Start: <b>%s</b>"), str);
      g_free (str);

      label = gtk_label_new (s);
      gtk_widget_show (label);
      gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

      str = somax_markup (tmp->timer->stop);
      snprintf (s, sizeof (s), _("Stop: <b>%s</b>"), str);
      g_free (str);

      label = gtk_label_new (s);
      gtk_widget_show (label);
      gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

      tmp = tmp->next;
    }
}

void
maker_spot_refresh (struct maker_spot_data_t *data)
{
  maker_spot_list_refresh (data);
}

void
maker_spot_changed (GtkWidget * w, struct maker_spot_data_t *data)
{
  if (maker_changed_check (w) == FALSE)
    return;

  if (data->refresh_lock == FALSE)
    {
      maker_spot_undoredo (w, data);
      maker_spot_save_t (NULL, data);
      data->is_saved = 1;
    }
}

static void
maker_spot_treeview_l_add (GtkWidget * w, GtkWidget * t)
{
  file_chooser_cb (_("Add files"), GTK_SELECTION_MULTIPLE,
		   GTK_FILE_CHOOSER_ACTION_OPEN, maker_spot_treeview_l_add_cb,
		   t);
}

static void
maker_spot_treeview_l_add_cb (void *l, void *t)
{
  GSList *file;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *tmp;
  struct maker_spot_data_t *spot;

  if (!l || !t)
    return;

  file = (GSList *) l;

  while (file)
    {
      if (g_file_test (file->data, G_FILE_TEST_EXISTS) == TRUE)
	{

	  model = gtk_tree_view_get_model (GTK_TREE_VIEW (t));
	  gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	  tmp = somax_to_utf8 (file->data);
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, tmp, -1);
	  g_free (tmp);
	}

      g_free (file->data);
      file = file->next;
    }

  g_slist_free (l);

  if ((spot = g_object_get_data (G_OBJECT (t), "data_spot")))
    maker_spot_changed (NULL, spot);
}

static void
maker_spot_treeview_l_add_dir (GtkWidget * w, GtkWidget * t)
{
  file_chooser_cb (_("Add directories"), GTK_SELECTION_MULTIPLE,
		   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		   maker_spot_treeview_l_add_dir_cb, t);
}

static void
maker_spot_treeview_l_add_dir_cb (void *l, void *t)
{
  GSList *file;
  GtkTreeModel *model;
  GtkTreeIter iter;
  char *tmp;
  struct maker_spot_data_t *spot;

  if (!l || !t)
    return;

  file = (GSList *) l;

  while (file)
    {
      if (g_file_test (file->data, G_FILE_TEST_EXISTS) == TRUE)
	{

	  model = gtk_tree_view_get_model (GTK_TREE_VIEW (t));
	  gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	  tmp = somax_to_utf8 (file->data);
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, tmp, -1);
	  g_free (tmp);
	}

      g_free (file->data);
      file = file->next;
    }

  g_slist_free (l);

  if ((spot = g_object_get_data (G_OBJECT (t), "data_spot")))
    maker_spot_changed (NULL, spot);
}

static void
maker_spot_treeview_remove (GtkWidget * w, GtkWidget * t)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *list, *b;
  int j, i = 0;
  struct maker_spot_data_t *spot;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (t));

  if (!
      (list =
       gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection),
					     &model)))
    return;

  b = list;

  while (list)
    {
      for (j = 0; j < i; j++)
	gtk_tree_path_prev (list->data);

      gtk_tree_model_get_iter (model, &iter, list->data);
      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
      gtk_tree_path_free (list->data);

      i++;
      list = list->next;
    }

  g_list_free (b);

  if ((spot = g_object_get_data (G_OBJECT (t), "data_spot")))
    maker_spot_changed (NULL, spot);
}

void
maker_spot_open (GtkWidget * w, struct maker_spot_data_t *data)
{
  char *file;

  if ((maker_spot_is_no_saved (data)
       && dialog_ask (_("Open new file without save?")) == GTK_RESPONSE_OK)
      || !maker_spot_is_no_saved (data))
    {

      if (!
	  (file =
	   file_chooser (_("Open a palinsesto file"), GTK_SELECTION_SINGLE,
			 GTK_FILE_CHOOSER_ACTION_OPEN)))
	return;

      if (data->maker_spot)
	spot_free (data->maker_spot);
      data->maker_spot = NULL;

      if (spot_parser_file (file, &data->maker_spot))
	dialog_msg (_("Somad Spot file syntax error"));

      else
	{
	  if (data->maker_spot_file)
	    g_free (data->maker_spot_file);

	  data->maker_spot_file = g_strdup (file);

	  maker_spot_refresh (data);
	}

      g_free (file);
    }
}

static void
maker_spot_duration_path (GtkWidget * widget, struct maker_spot_data_t *data)
{
  maker_spot_duration (data, data->path, data->duration_path);
}

static void
maker_spot_duration (struct maker_spot_data_t *data, GtkWidget * tv,
		     GtkWidget * label)
{
  GtkWidget *popup;
  GdkCursor *cursor;
  GtkTreeIter iter;
  GtkListStore *store;
  int64_t duration;
  soma_stat *stat;
  soma_stat_dir *dir;
  gchar *name;
  char s[1024];

  popup = somax_win ();
  gtk_widget_show_all (popup);

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (popup->window, cursor);

  store = (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (tv));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter) != TRUE)
    {
      gdk_cursor_unref (cursor);
      gtk_widget_destroy (popup);
      return;
    }

  duration = 0;

  do
    {
      gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &name, -1);

      if (name)
	{
	  /* Local file ? */
	  if (g_file_test (name, G_FILE_TEST_EXISTS) == TRUE)
	    {
	      snprintf (s, sizeof (s), _("Check %s"), name);

	      if (g_file_test (name, G_FILE_TEST_IS_DIR) == TRUE
		  && (dir = somax_local_stat_dir (popup, s, name)))
		{
		  duration += dir->duration;
		  somax_local_stat_dir_free (dir);
		}

	      else if ((stat = somax_local_stat (popup, s, name)))
		{
		  duration += stat->duration;
		  somax_local_stat_free (stat);
		}
	    }

	  /* Remote file with distribuited filesystem ? */
	  else if (data->controller && *name == DISTRIBUITED_CHAR)
	    {
	      snprintf (s, sizeof (s), _("Check %s"), name + 1);

	      if (name[strlen (name) - 1] == '/')
		{
		  if ((dir =
		       somax_get_stat_dir_path (popup, s, data->controller,
						name + 1)))
		    {
		      duration += dir->duration;
		      somax_stat_dir_free (dir);
		    }
		}

	      else
		if ((stat =
		     somax_get_stat_path (popup, s, data->controller,
					  name + 1)))
		{
		  duration += stat->duration;
		  somax_stat_free (stat);
		}
	    }

	  /* Remote file ? */
	  else if (data->controller)
	    {
	      snprintf (s, sizeof (s), _("Check %s"), name + 1);

	      if (name[strlen (name) - 1] == '/')
		{
		  if ((dir =
		       somax_get_stat_dir (popup, s, data->controller,
					   name + 1)))
		    {
		      duration += dir->duration;
		      somax_stat_dir_free (dir);
		    }
		}

	      else
		if ((stat =
		     somax_get_stat (popup, s, data->controller, name + 1)))
		{
		  duration += stat->duration;
		  somax_stat_free (stat);
		}
	    }

	  g_free (name);
	}
    }
  while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter));

  gtk_label_set_text (GTK_LABEL (label), stat_duration (duration));

  gdk_cursor_unref (cursor);
  gtk_widget_destroy (popup);
}

static void
maker_spot_selected (GtkTreeView * tree, GtkTreePath * path,
		     GtkTreeViewColumn * column,
		     struct maker_spot_data_t *data)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *name;
  soma_stat_dir *dir;
  soma_stat *stat;
  GdkCursor *cursor;
  GtkWidget *popup;
  char s[1024];

  model = gtk_tree_view_get_model (tree);
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 0, &name, -1);

  if (!name)
    return;

  popup = somax_win ();
  gtk_widget_show_all (popup);

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (popup->window, cursor);

  /* Local file ? */
  if (g_file_test (name, G_FILE_TEST_EXISTS) == TRUE)
    {
      snprintf (s, sizeof (s), _("Check %s"), name);

      if (g_file_test (name, G_FILE_TEST_IS_DIR) == TRUE
	  && (dir = somax_local_stat_dir (popup, s, name)))
	{
	  gdk_cursor_unref (cursor);
	  gtk_widget_destroy (popup);
	  stat_struct_dir_popup (dir);
	  somax_local_stat_dir_free (dir);
	  g_free (name);
	  return;
	}

      else if ((stat = somax_local_stat (popup, s, name)))
	{
	  gdk_cursor_unref (cursor);
	  gtk_widget_destroy (popup);
	  stat_struct_popup (stat);
	  somax_local_stat_free (stat);
	  g_free (name);
	  return;
	}
    }

  /* Remote file with distribuited filesystem ? */
  else if (data->controller && *name == DISTRIBUITED_CHAR)
    {
      snprintf (s, sizeof (s), _("Check %s"), name + 1);

      if (name[strlen (name) - 1] == '/')
	{
	  if ((dir =
	       somax_get_stat_dir_path (popup, s, data->controller,
					name + 1)))
	    {
	      gdk_cursor_unref (cursor);
	      gtk_widget_destroy (popup);
	      stat_struct_dir_popup (dir);
	      somax_stat_dir_free (dir);
	      g_free (name);
	      return;
	    }
	}

      else
	if ((stat =
	     somax_get_stat_path (popup, s, data->controller, name + 1)))
	{
	  gdk_cursor_unref (cursor);
	  gtk_widget_destroy (popup);
	  stat_struct_popup (stat);
	  somax_stat_free (stat);
	  g_free (name);
	  return;
	}
    }

  /* Remote file ? */
  else if (data->controller)
    {
      snprintf (s, sizeof (s), _("Check %s"), name + 1);

      if (name[strlen (name) - 1] == '/')
	{
	  if ((dir =
	       somax_get_stat_dir (popup, s, data->controller, name + 1)))
	    {
	      gdk_cursor_unref (cursor);
	      gtk_widget_destroy (popup);
	      stat_struct_dir_popup (dir);
	      somax_stat_dir_free (dir);
	      g_free (name);
	      return;
	    }
	}

      else if ((stat = somax_get_stat (popup, s, data->controller, name + 1)))
	{
	  gdk_cursor_unref (cursor);
	  gtk_widget_destroy (popup);
	  stat_struct_popup (stat);
	  somax_stat_free (stat);
	  g_free (name);
	  return;
	}
    }

  g_free (name);

  gdk_cursor_unref (cursor);
  gtk_widget_destroy (popup);
  dialog_msg (_("No information about this file!"));
}

void
maker_spot_set_controller (struct maker_spot_data_t *data, void *controller)
{
  GList *list;

  data->controller = controller;

  for (list = data->controller_widgets; list; list = list->next)
    gtk_widget_set_sensitive (GTK_WIDGET (list->data),
			      controller ? TRUE : FALSE);
}

/* UNDOREDO */
static void
maker_spot_undoredo (GtkWidget * w, struct maker_spot_data_t *data)
{
  gchar *message;

  if (w && GTK_IS_ENTRY (w) && GTK_IS_SPIN_BUTTON (w))
    {
      const gchar *tmp = gtk_entry_get_text (GTK_ENTRY (w));
      if (tmp && *tmp)
	{
	  data->refresh_lock = TRUE;
	  gtk_spin_button_set_value (GTK_SPIN_BUTTON (w),
				     (gdouble) atoi (tmp));
	  data->refresh_lock = FALSE;
	}
    }

  /* The Entry widget returns a change event for any keyboard pression, so
   * I use a timer any 2 seconds: */
  if (w && GTK_IS_ENTRY (w) && w == data->undoredo_current_widget
      && data->undoredo_timer
      && g_timer_elapsed (data->undoredo_timer,
			  NULL) <= MAKER_UNDOREDO_MAX_TIMER)
    {
      if (!data->undoredo_timeout)
	{
	  data->undoredo_current_value =
	    g_strdup (gtk_entry_get_text (GTK_ENTRY (w)));

	  data->undoredo_timeout =
	    g_timeout_add (MAKER_UNDOREDO_MAX_TIMER,
			   (GSourceFunc) maker_spot_undoredo_timeout, data);
	}
      return;
    }

  if (data->undoredo_timer)
    {
      g_timer_destroy (data->undoredo_timer);
      data->undoredo_timer = NULL;
    }

  if (w && GTK_IS_ENTRY (w))
    {
      data->undoredo_timer = g_timer_new ();
      data->undoredo_current_widget = w;
    }
  else
    data->undoredo_current_widget = NULL;

  /* Dump: */
  message = maker_message (w);
  maker_spot_undoredo_new (data, MAKER_UNDOREDO_CHANGE, TRUE, message);
  g_free (message);

  /* Remove the last item: */
  if (g_list_length (data->undoredo_undo_list) >= MAKER_UNDOREDO_MAX_UNDO)
    {
      GList *last = g_list_last (data->undoredo_undo_list);
      maker_spot_undoredo_free (last->data);
      data->undoredo_undo_list =
	g_list_remove (data->undoredo_undo_list, last->data);
    }
}

static gboolean
maker_spot_undoredo_timeout (struct maker_spot_data_t *data)
{
  gboolean todo = TRUE;

  if (data->undoredo_current_value)
    {
      if (data->undoredo_current_widget
	  &&
	  !strcmp (gtk_entry_get_text
		   (GTK_ENTRY (data->undoredo_current_widget)),
		   data->undoredo_current_value))
	todo = FALSE;

      g_free (data->undoredo_current_value);
      data->undoredo_current_value = NULL;
    }

  data->undoredo_timeout = 0;

  if (todo == TRUE)
    maker_spot_undoredo (data->undoredo_current_widget, data);

  return FALSE;
}

static void
maker_spot_undoredo_new (struct maker_spot_data_t *data,
			 enum maker_undoredo_type_t type, gboolean undo,
			 gchar * message)
{
  struct maker_spot_undoredo_t *new;

  new = g_malloc0 (sizeof (struct maker_spot_undoredo_t));
  new->message = g_strdup (message);

  switch (type)
    {
    case MAKER_UNDOREDO_NEW:
    case MAKER_UNDOREDO_DESTROY:
      new->spot = data->spot;
      break;

    case MAKER_UNDOREDO_CHANGE:
      new->previous_spot = data->spot;
      maker_spot_dump_switch_spot (data);
      new->spot = data->spot;
      break;
    }

  new->type = type;

  if (undo == TRUE)
    data->undoredo_undo_list = g_list_prepend (data->undoredo_undo_list, new);
  else
    data->undoredo_redo_list = g_list_prepend (data->undoredo_redo_list, new);
}

static void
maker_spot_undoredo_free (struct maker_spot_undoredo_t *data)
{
  if (data->previous_spot)
    {
      data->previous_spot->next = NULL;
      spot_free (data->previous_spot);
    }

  if (data->spot)
    {
      data->spot->next = NULL;
      spot_free (data->spot);
    }

  if (data->message)
    g_free (data->message);

  g_free (data);
}

void
maker_spot_undoredo_status (struct maker_spot_data_t *data, gboolean * undo,
			    gboolean * redo)
{
  if (data->undoredo_undo_list)
    *undo = TRUE;
  else
    *undo = FALSE;

  if (data->undoredo_redo_list)
    *redo = TRUE;
  else
    *redo = FALSE;
}

static void
maker_spot_undoredo_work_n (GtkWidget * widget,
			    struct maker_spot_data_t *data)
{
  gint id = (gint) g_object_get_data (G_OBJECT (widget), "id");
  gboolean what = (gboolean) g_object_get_data (G_OBJECT (widget), "what");

  for (; id >= 0; id--)
    maker_spot_undoredo_work (data, what);
}

void
maker_spot_undoredo_work (struct maker_spot_data_t *data, gboolean what)
{
  GList **list, **undo_list;
  struct maker_spot_undoredo_t *undo;
  struct somad_spot_data *tmp;

  if (what == TRUE)
    {
      list = &data->undoredo_undo_list;
      undo_list = &data->undoredo_redo_list;
    }
  else
    {
      list = &data->undoredo_redo_list;
      undo_list = &data->undoredo_undo_list;
    }

  if (!list || !*list)
    return;

  undo = (*list)->data;
  *list = g_list_remove (*list, undo);

  switch (undo->type)
    {
    case MAKER_UNDOREDO_NEW:
      maker_spot_remove_spot_real (data, undo->spot, !what, FALSE);
      undo->type = MAKER_UNDOREDO_DESTROY;
      *undo_list = g_list_prepend (*undo_list, undo);
      break;

    case MAKER_UNDOREDO_DESTROY:
      maker_spot_insert_spot (data, undo->spot);
      undo->type = MAKER_UNDOREDO_NEW;
      *undo_list = g_list_prepend (*undo_list, undo);
      break;

    case MAKER_UNDOREDO_CHANGE:
      maker_spot_remove_spot_real (data, undo->spot, !what, FALSE);
      maker_spot_insert_spot (data, undo->previous_spot);
      tmp = undo->spot;
      undo->spot = undo->previous_spot;
      undo->previous_spot = tmp;

      *undo_list = g_list_prepend (*undo_list, undo);
      break;
    }

  maker_spot_refresh (data);
}

static void
maker_spot_destroy (GtkWidget * w, struct maker_spot_data_t *data)
{
  g_list_foreach (data->undoredo_undo_list, (GFunc) maker_spot_undoredo_free,
		  NULL);
  data->undoredo_undo_list = NULL;
  g_list_foreach (data->undoredo_redo_list, (GFunc) maker_spot_undoredo_free,
		  NULL);
  data->undoredo_redo_list = NULL;

  if (data->undoredo_timer)
    {
      g_timer_destroy (data->undoredo_timer);
      data->undoredo_timer = NULL;
    }

  if (data->undoredo_timeout)
    {
      g_source_remove (data->undoredo_timeout);
      data->undoredo_timeout = 0;
    }

  if (data->undoredo_current_value)
    {
      g_free (data->undoredo_current_value);
      data->undoredo_current_value = NULL;
    }
}

void
maker_spot_undoredo_history (struct maker_spot_data_t *data, gboolean what,
			     GtkWidget * w)
{
  static GtkWidget *menu = NULL;
  GtkWidget *item;
  GList *list;
  gint i;

  if (!menu)
    menu = gtk_menu_new ();
  else
    gtk_container_foreach (GTK_CONTAINER (menu),
			   (GtkCallback) gtk_widget_destroy, NULL);

  if (what == TRUE)
    list = data->undoredo_undo_list;
  else
    list = data->undoredo_redo_list;

  for (i = 0; list; list = list->next, i++)
    {
      item =
	gtk_menu_item_new_with_label (((struct maker_spot_undoredo_t *) list->
				       data)->message);
      g_object_set_data (G_OBJECT (item), "id", (gpointer) i);
      g_object_set_data (G_OBJECT (item), "what", (gpointer) what);
      gtk_widget_show (item);
      gtk_container_add (GTK_CONTAINER (menu), item);

      g_signal_connect ((gpointer) item, "activate",
			G_CALLBACK (maker_spot_undoredo_work_n), data);
    }

  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, maker_popup_position, w, 0, 0);
}

/* EOF */
