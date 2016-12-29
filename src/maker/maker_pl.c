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

#define MAKER_ADD_LABEL( x ) label = gtk_label_new( x ); \
  gtk_widget_show(label); \
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); \
  gtk_table_attach(GTK_TABLE(table), label, 0,1, i, i+1,   GTK_FILL, 0, 0, 0);

#define MAKER_ADD_WIDGET( x ) gtk_widget_show( x ); \
  gtk_table_attach(GTK_TABLE(table), x , 1,2, i, i+1,   GTK_FILL | GTK_EXPAND, 0, 0, 0);

static void maker_pl_new_t (GtkWidget *, struct maker_pl_data_t *);
static void maker_pl_save_t (GtkWidget *, struct maker_pl_data_t *);
static void maker_pl_save_real_t (GtkWidget * w, struct maker_pl_data_t *data,
				  struct somad_data *pl);
static void maker_pl_remove_t (GtkWidget *, struct maker_pl_data_t *);
static void maker_pl_list_refresh (struct maker_pl_data_t *);
static void maker_pl_undoredo (GtkWidget * w, struct maker_pl_data_t *data);
static gboolean maker_pl_undoredo_timeout (struct maker_pl_data_t *data);
static void maker_pl_changed_type (GtkWidget *, struct maker_pl_data_t *);
static void maker_pl_file_l_add (GtkWidget *, GtkWidget *);
static void maker_pl_box_hide_all (struct maker_pl_data_t *);
static void maker_pl_box_files (struct maker_pl_data_t *);
static void maker_pl_box_stream (struct maker_pl_data_t *);
static void maker_pl_box_module (struct maker_pl_data_t *);
static void maker_pl_box_module_so (struct maker_pl_data_t *,
				    struct module_item *);
static void maker_pl_box_silence (struct maker_pl_data_t *);
static void maker_pl_treeview_l_add (GtkWidget *, GtkWidget *);
static void maker_pl_treeview_l_add_dir (GtkWidget *, GtkWidget *);
static void maker_pl_treeview_remove (GtkWidget *, GtkWidget *);
static time_t maker_pl_calendar_time (GtkWidget *);
static void maker_pl_calendar_changed (GtkWidget *, struct maker_pl_data_t *);
static void maker_pl_calendar_today (GtkWidget *, struct maker_pl_data_t *);
static void maker_pl_treeview_l_add_cb (void *l, void *t);
static void maker_pl_treeview_l_add_dir_cb (void *l, void *t);
static void maker_pl_duration_pathitem (GtkWidget *,
					struct maker_pl_data_t *);
static void maker_pl_duration_pathspot (GtkWidget *,
					struct maker_pl_data_t *);
static void maker_pl_duration (struct maker_pl_data_t *, GtkWidget *,
			       GtkWidget *);
static void maker_pl_selected (GtkTreeView *, GtkTreePath *,
			       GtkTreeViewColumn *, struct maker_pl_data_t *);
static void maker_pl_remove_pl_real (struct maker_pl_data_t *data,
				     struct somad_data *pl, gboolean undo,
				     gboolean insert);
static void maker_pl_undoredo_new (struct maker_pl_data_t *,
				   enum maker_undoredo_type_t, gboolean undo,
				   gchar * message);
static void maker_pl_undoredo_free (struct maker_pl_undoredo_t *);
static void maker_pl_destroy (GtkWidget * w, struct maker_pl_data_t *data);

/* Active the element in the principal window */
gboolean
maker_pl_draw_resize (GtkWidget * widget, struct maker_pl_data_t *data)
{
  gint start_hour = data->pl->timer->start_hour;
  gint start_min = data->pl->timer->start_min;
  gint stop_hour = data->pl->timer->stop_hour;
  gint stop_min = data->pl->timer->stop_min;

  data->pl->timer->start_hour =
    gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->start_hour));
  data->pl->timer->start_min =
    gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->start_min));
  data->pl->timer->stop_hour =
    gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->stop_hour));
  data->pl->timer->stop_min =
    gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->stop_min));

  maker_pl_undoredo_new (data, MAKER_UNDOREDO_CHANGE, TRUE,
			 _("Timeline manualy resized"));

  data->pl->timer->start_hour = start_hour;
  data->pl->timer->start_min = start_min;
  data->pl->timer->stop_hour = stop_hour;
  data->pl->timer->stop_min = stop_min;

  if (data->pl)
    maker_pl_data_show (data->pl, data);

  maker_pl_save_t (NULL, data);
  return TRUE;
}

/* Active the element in the principal window */
gboolean
maker_pl_draw_button_press (GtkWidget * widget, GdkEventButton * event,
			    struct maker_pl_data_t * data)
{
  int x, y;
  GList *list;
  GList *cur;

  if (!event->type == GDK_BUTTON_PRESS)
    return FALSE;

  gtk_widget_get_pointer (widget, &x, &y);

  if (x < 0 || y < 0
      || gtk_window_is_active (GTK_WINDOW (gtk_widget_get_toplevel (widget)))
      == FALSE)
    return FALSE;

  if (!(list = draw_get_xy (data->draw, x, y, data->maker_pl)))
    return FALSE;

  if ((cur = g_list_last (list))->data != data->pl)
    maker_pl_data_show (cur->data, data);
  g_list_free (list);

  return TRUE;
}

/* Create the window */
GtkWidget *
create_maker_pl (struct maker_pl_data_t * data, void *c, gboolean local)
{
  GtkWidget *scrolledwindow;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *nbox;
  GtkWidget *t;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *alignment;
  GtkWidget *table;
  GtkWidget *entry;
  GtkObject *adj;
  GtkWidget *vport;
  GtkWidget *notebook;
  GtkWidget *textview;
  GtkListStore *model;
  GtkTreeSelection *selection;
  GtkCellRenderer *renderer;
  GtkTooltips *tooltips;
  struct tm *k;
  GList *module;

  int j, i = 0;

  char s[1024];

  if (!(k = get_time ()))
    return NULL;

  tooltips = gtk_tooltips_new ();

  /* For the editor, the controller must be saved in a struct data */
  /* Editor has this NULL, somax no. */
  data->controller = c;
  data->local = local;
  data->pl = NULL;

  if (data->modules_widget)
    {
      module = data->modules_widget;
      while (module)
	{
	  g_free (module->data);
	  module = module->next;
	}
      g_list_free (data->modules_widget);
      data->modules_widget = NULL;
    }

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (hbox), box, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (box), 8);

  frame = gtk_frame_new (_("Transmission"));
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


  data->calendar = gtk_calendar_new ();
  gtk_widget_show (data->calendar);
  gtk_box_pack_start (GTK_BOX (box), data->calendar, FALSE, FALSE, 0);
  gtk_calendar_set_display_options (GTK_CALENDAR (data->calendar),
				    GTK_CALENDAR_SHOW_HEADING
				    | GTK_CALENDAR_SHOW_DAY_NAMES);

  g_signal_connect ((gpointer) data->calendar, "day_selected",
		    G_CALLBACK (maker_pl_calendar_changed), data);

  button = gtk_button_new_with_mnemonic (_("_Today"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_pl_calendar_today), data);

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);

  label = gtk_label_new (_("<b>Edit your transmission</b>"));
  gtk_widget_show (label);

  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);

  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_set_sensitive (frame, FALSE);
  data->frame = frame;

  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_container_add (GTK_CONTAINER (frame), notebook);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

  /* PAGE GENERAL */
  label = gtk_label_new (_("General"));
  gtk_widget_show (label);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, label);

  vport = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (vport);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), vport);

  table = gtk_table_new (0, 0, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (vport), table);

  MAKER_ADD_LABEL (_("Description:"));

  entry = gtk_entry_new ();
  MAKER_ADD_WIDGET (entry);
  data->description = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_pl_changed), data);
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
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  gtk_table_attach (GTK_TABLE (t), entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);
  g_object_set_data (G_OBJECT (entry), "add", (gpointer) k->tm_year + 1899);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Start Year changed to: %s"));

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
		    G_CALLBACK (maker_pl_changed), data);
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
		    G_CALLBACK (maker_pl_changed), data);
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
		    G_CALLBACK (maker_pl_changed), data);
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
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  gtk_table_attach (GTK_TABLE (t), entry, 3, 4, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Start Hour changed to: %s"));

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
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  gtk_table_attach (GTK_TABLE (t), entry, 5, 6, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Start Minute changed to: %s"));

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
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  gtk_table_attach (GTK_TABLE (t), entry, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);
  g_object_set_data (G_OBJECT (entry), "add", (gpointer) k->tm_year + 1899);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Year changed to: %s"));

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
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  gtk_table_attach (GTK_TABLE (t), entry, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Month changed to: %s"));

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
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  gtk_table_attach (GTK_TABLE (t), entry, 5, 6, 0, 1, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Day changed to: %s"));

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
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  gtk_table_attach (GTK_TABLE (t), entry, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Weekly Day changed to: %s"));

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
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  gtk_table_attach (GTK_TABLE (t), entry, 3, 4, 1, 2, GTK_FILL | GTK_EXPAND,
		    0, 0, 0);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Hour changed to: %s"));

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
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  gtk_table_attach (GTK_TABLE (t), entry, 5, 6, 1, 2,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (0), 0, 0);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Stop Minute changed to: %s"));

  i++;

  MAKER_ADD_LABEL (_("TimeContinued:"));

  entry =
    gtk_check_button_new_with_mnemonic (_("active timecontinued attribute"));
  MAKER_ADD_WIDGET (entry);
  data->timecontinued = entry;
  g_signal_connect ((gpointer) entry, "toggled",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("TimeContinued changed to: %s"));

  i++;

  MAKER_ADD_LABEL (_("Priority"));

  entry = gtk_check_button_new_with_mnemonic (_("active priority attribute"));
  MAKER_ADD_WIDGET (entry);
  data->priority = entry;
  g_signal_connect ((gpointer) entry, "toggled",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Priority changed to: %s"));

  i++;

  MAKER_ADD_LABEL (_("SpotController:"));

  entry =
    gtk_check_button_new_with_mnemonic (_("active spotcontroller system"));
  MAKER_ADD_WIDGET (entry);
  data->spotcontroller = entry;
  g_signal_connect ((gpointer) entry, "toggled",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("SpotController changed to: %s"));

  i++;

  MAKER_ADD_LABEL (_("SoftStop:"));

  entry = gtk_check_button_new_with_mnemonic (_("active softstop attribute"));
  MAKER_ADD_WIDGET (entry);
  data->softstop = entry;
  g_signal_connect ((gpointer) entry, "toggled",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("SoftStop changed to: %s"));

  /* PAGE TYPE */
  label = gtk_label_new (_("Type"));
  gtk_widget_show (label);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, label);

  vport = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (vport);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), vport);

  table = gtk_table_new (0, 0, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (vport), table);

  i = 0;

  MAKER_ADD_LABEL (_("Type:"));

  entry = gtk_combo_box_new_text ();
  MAKER_ADD_WIDGET (entry);
  data->type = entry;
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("files"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("stream"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("silence"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("module generic"));

  /* Get module name: */
  module = module_get_list ();

  while (module)
    {
      struct module_item *module_widget;

      module_widget =
	(struct module_item *) g_malloc0 (sizeof (struct module_item));

      module_widget->module = (struct module_t *) module->data;

      if ((module_widget->widget = ((struct module_t *) module->data)->new ())
	  && GTK_IS_WIDGET (module_widget->widget))
	{
	  data->modules_widget =
	    g_list_append (data->modules_widget, module_widget);
	  gtk_combo_box_append_text (GTK_COMBO_BOX (entry),
				     ((struct module_t *) module->data)->
				     name);
	}

      else
	{
	  g_message ("Excluding module '%s'", module_widget->module->name);
	  g_free (module_widget);
	}

      module = module->next;
    }

  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_pl_changed_type), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message", _("Type changed to: %s"));

  i++;

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_table_attach (GTK_TABLE (table), frame, 0, 2, i, i + 1,
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), 0, 0);

  hbox = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  /* EMPTY BOX */
  label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  data->box_empty = label;

  /* SILENCE BOX */
  label = gtk_label_new (_("Silence: no options"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  data->box_silence = label;

  /* GENERIC MODULE BOX */
  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (hbox), scrolledwindow, TRUE, TRUE, 0);
  data->box_module = scrolledwindow;

  vport = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (vport);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), vport);

  nbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (vport), nbox);

  box = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (nbox), box, FALSE, FALSE, 0);

  label = gtk_label_new (_("Module:"));
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (box), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message", _("Module changed to: %s"));
  data->module = entry;

  box = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (nbox), box, TRUE, TRUE, 0);

  label = gtk_label_new (_("Module Data:"));
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (box), scrolledwindow, TRUE, TRUE, 0);

  textview = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), TRUE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD);
  gtk_widget_show (textview);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);
  data->moduledata =
    (GtkWidget *) gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));

  g_signal_connect ((gpointer) data->moduledata, "changed",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message", _("Module data changed"));

  /* MODULES BOX */
  module = data->modules_widget;

  while (module)
    {
      scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
      gtk_widget_show (scrolledwindow);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX (hbox), scrolledwindow, TRUE, TRUE, 0);
      ((struct module_item *) module->data)->widget_container =
	scrolledwindow;

      vport = gtk_viewport_new (NULL, NULL);
      gtk_widget_show (vport);
      gtk_container_add (GTK_CONTAINER (scrolledwindow), vport);

      gtk_container_add (GTK_CONTAINER (vport),
			 ((struct module_item *) module->data)->widget);

      module = module->next;
    }

  /* STREAM BOX */
  box = gtk_hbox_new (0, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), box, TRUE, TRUE, 0);
  data->box_stream = box;

  label = gtk_label_new (_("Stream:"));
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (box), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message", _("Stream changed to: %s"));
  data->stream = entry;

  /* FILES BOX */
  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (hbox), scrolledwindow, TRUE, TRUE, 0);
  data->box_files = scrolledwindow;

  vport = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (vport);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), vport);

  table = gtk_table_new (0, 0, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_container_add (GTK_CONTAINER (vport), table);

  i = 0;

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_table_attach_defaults (GTK_TABLE (table), scrolledwindow, 0, 2, i,
			     i + 1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  model = gtk_list_store_new (1, G_TYPE_STRING);

  entry = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (entry), TRUE);
  g_object_set_data (G_OBJECT (entry), "data_pl", data);

  gtk_widget_set_size_request (entry, -1, 150);

  g_signal_connect (entry, "row_activated", G_CALLBACK (maker_pl_selected),
		    data);

  data->pathitem = entry;

  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (entry), FALSE);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), entry);
  gtk_widget_show (entry);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (entry));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (entry), TRUE);

  g_object_unref (model);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (entry),
					       -1, _("PathItem"), renderer,
					       "text", 0, NULL);

  i++;

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_table_attach (GTK_TABLE (table), box, 0, 2, i, i + 1,
		    (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0,
		    0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_treeview_r_add), data->pathitem);
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
		    G_CALLBACK (maker_treeview_r_add_dir), data->pathitem);
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
			G_CALLBACK (maker_pl_treeview_l_add), data->pathitem);
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
			G_CALLBACK (maker_pl_treeview_l_add_dir),
			data->pathitem);
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
		    G_CALLBACK (maker_pl_treeview_remove), data->pathitem);
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
		    G_CALLBACK (maker_pl_duration_pathitem), data);

  label = gtk_label_new (_("Duration: n/a"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  data->duration_pathitem = label;

  /* PAGE ITEMS */
  label = gtk_label_new (_("Items"));
  gtk_widget_show (label);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, label);

  vport = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (vport);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), vport);

  table = gtk_table_new (0, 0, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (vport), table);

  i = 0;

  MAKER_ADD_LABEL (_("Jingle:"));

  box = gtk_hbox_new (FALSE, 0);
  MAKER_ADD_WIDGET (box);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (box), entry, TRUE, TRUE, 0);
  data->jingle = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message", _("Jingle changed to: %s"));

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_file_r_add), entry);
  maker_controller_check (data->controller, button,
			  &data->controller_widgets);
  gtk_tooltips_set_tip (tooltips, button, _("Remote Add File"), NULL);

  image =
    gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
			     "maker_file_remote.png");
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  if (data->local == TRUE)
    {
      button = gtk_button_new ();
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (maker_pl_file_l_add), entry);
      gtk_tooltips_set_tip (tooltips, button, _("Local Add File"), NULL);

      image =
	gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
				 "maker_file_locale.png");
      gtk_widget_show (image);
      gtk_container_add (GTK_CONTAINER (button), image);
    }

  i++;

  MAKER_ADD_LABEL (_("PreSpot:"));

  box = gtk_hbox_new (FALSE, 0);
  MAKER_ADD_WIDGET (box);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (box), entry, TRUE, TRUE, 0);
  data->prespot = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("PreSpot changed to: %s"));

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_file_r_add), entry);
  maker_controller_check (data->controller, button,
			  &data->controller_widgets);
  gtk_tooltips_set_tip (tooltips, button, _("Remote Add File"), NULL);

  image =
    gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
			     "maker_file_remote.png");
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  if (data->local == TRUE)
    {
      button = gtk_button_new ();
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (maker_pl_file_l_add), entry);
      gtk_tooltips_set_tip (tooltips, button, _("Local Add File"), NULL);

      image =
	gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
				 "maker_file_locale.png");
      gtk_widget_show (image);
      gtk_container_add (GTK_CONTAINER (button), image);
    }

  i++;

  MAKER_ADD_LABEL (_("PostSpot:"));

  box = gtk_hbox_new (FALSE, 0);
  MAKER_ADD_WIDGET (box);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (box), entry, TRUE, TRUE, 0);
  data->postspot = entry;
  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("PostSpot changed to: %s"));

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_file_r_add), entry);
  maker_controller_check (data->controller, button,
			  &data->controller_widgets);
  gtk_tooltips_set_tip (tooltips, button, _("Remote Add File"), NULL);

  image =
    gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
			     "maker_file_remote.png");
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  if (data->local == TRUE)
    {
      button = gtk_button_new ();
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (maker_pl_file_l_add), entry);
      gtk_tooltips_set_tip (tooltips, button, _("Local Add File"), NULL);

      image =
	gtk_image_new_from_file (PATH_IMAGES G_DIR_SEPARATOR_S
				 "maker_file_locale.png");
      gtk_widget_show (image);
      gtk_container_add (GTK_CONTAINER (button), image);
    }

  i++;

  MAKER_ADD_LABEL (_("RatioItem:"));

  adj = gtk_adjustment_new (0, 0, 99, 1, 10, 10);
  entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);

  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("RatioItem changed to: %s"));
  data->ratioitem = entry;

  MAKER_ADD_WIDGET (entry);

  i++;

  MAKER_ADD_LABEL (_("RatioSpot:"));

  adj = gtk_adjustment_new (0, 0, 99, 1, 10, 10);
  entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);

  g_signal_connect ((gpointer) entry, "changed",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("RatioSpot changed to: %s"));
  data->ratiospot = entry;

  MAKER_ADD_WIDGET (entry);

  i++;

  MAKER_ADD_LABEL (_("Random Item:"));

  entry =
    gtk_check_button_new_with_mnemonic (_
					("Active random selection for the items"));
  MAKER_ADD_WIDGET (entry);

  g_signal_connect ((gpointer) entry, "toggled",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Random Item changed to: %s"));
  data->randomitem = entry;

  i++;

  MAKER_ADD_LABEL (_("Random Spot:"));

  entry =
    gtk_check_button_new_with_mnemonic (_
					("Active random selection for the spot"));
  MAKER_ADD_WIDGET (entry);

  g_signal_connect ((gpointer) entry, "toggled",
		    G_CALLBACK (maker_pl_changed), data);
  g_signal_connect ((gpointer) entry, "delete_event",
		    G_CALLBACK (maker_changed_destroy), data);
  g_object_set_data (G_OBJECT (entry), "message",
		     _("Random Spot changed to: %s"));
  data->randomspot = entry;

  i++;

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_table_attach_defaults (GTK_TABLE (table), scrolledwindow, 0, 2, i,
			     i + 1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  model = gtk_list_store_new (1, G_TYPE_STRING);

  entry = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (entry), TRUE);
  g_object_set_data (G_OBJECT (entry), "data_pl", data);

  gtk_widget_set_size_request (entry, -1, 150);

  g_signal_connect (entry, "row_activated", G_CALLBACK (maker_pl_selected),
		    data);

  data->pathspot = entry;

  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (entry), FALSE);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), entry);
  gtk_widget_show (entry);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (entry));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (entry), TRUE);

  g_object_unref (model);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (entry),
					       -1, _("PathSpot"), renderer,
					       "text", 0, NULL);

  i++;

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_table_attach (GTK_TABLE (table), box, 0, 2, i, i + 1,
		    (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0,
		    0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_treeview_r_add), data->pathspot);
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
		    G_CALLBACK (maker_treeview_r_add_dir), data->pathspot);
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
			G_CALLBACK (maker_pl_treeview_l_add), data->pathspot);
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
			G_CALLBACK (maker_pl_treeview_l_add_dir),
			data->pathspot);
      gtk_tooltips_set_tip (tooltips, button, _("Local Add Directory"), NULL);

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
		    G_CALLBACK (maker_pl_treeview_remove), data->pathspot);
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
		    G_CALLBACK (maker_pl_duration_pathspot), data);

  label = gtk_label_new (_("Duration: n/a"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  data->duration_pathspot = label;

  /* DOWN MAKER  */
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (maker_pl_new_t),
		    data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("New Transmission"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (maker_pl_remove_t), data);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-remove", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Remove Transmission"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  entry =
    draw_new (&data->maker_pl, (void *) maker_pl_draw_button_press, data,
	      TRUE, (void *) maker_pl_draw_resize, data);
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
  data->draw = entry;

  data->frame_list_w = NULL;

  if (data->maker_pl)
    maker_pl_data_show (data->maker_pl, data);
  else
    maker_pl_box_hide_all (data);

  maker_pl_list_refresh (data);

  g_signal_connect ((gpointer) vbox, "delete_event",
		    G_CALLBACK (maker_pl_destroy), data);

  return vbox;
}

void
maker_pl_data_show (struct somad_data *pl, struct maker_pl_data_t *data)
{
  int j;
  GtkStyle *style;
  GtkListStore *store;
  GtkTreeIter iter;
  GList *list;
  struct tm *k;
  GList *module;
  struct module_item *mt;
  char *moduledata;

  if (!(k = get_time ()))
    return;

  maker_pl_box_hide_all (data);
  data->pl = NULL;

  if (!pl)
    return;

  data->refresh_lock = TRUE;

  data->pl = pl;
  gtk_widget_set_sensitive (data->frame, TRUE);

  style = gtk_style_copy (gtk_widget_get_default_style ());

  style->fg[GTK_STATE_NORMAL] = *pl->color;
  gtk_widget_set_style (gtk_frame_get_label_widget (GTK_FRAME (data->frame)),
			style);

  gtk_entry_set_text (GTK_ENTRY (data->description), data->pl->description);

  if (pl->timer->start_year == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_year), 0);

  else
    {
      j = pl->timer->start_year - (k->tm_year + 1900);
      gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_year),
				j > -1 ? j + 1 : 0);
    }

  if (pl->timer->start_month == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_month), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_month),
			      pl->timer->start_month + 1);

  if (pl->timer->start_mday == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_mday), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_mday),
			      pl->timer->start_mday);

  if (pl->timer->start_wday == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_wday), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->start_wday),
			      pl->timer->start_wday + 1);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->start_hour),
			     pl->timer->start_hour);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->start_min),
			     pl->timer->start_min);

  if (pl->timer->stop_year == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_year), 0);

  else
    {
      j = pl->timer->stop_year - (k->tm_year + 1900);
      gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_year),
				j > -1 ? j + 1 : 0);
    }

  if (pl->timer->stop_month == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_month), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_month),
			      pl->timer->stop_month + 1);

  if (pl->timer->stop_mday == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_mday), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_mday),
			      pl->timer->stop_mday);

  if (pl->timer->stop_wday == -1)
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_wday), 0);

  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (data->stop_wday),
			      pl->timer->stop_wday + 1);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->stop_hour),
			     pl->timer->stop_hour);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->stop_min),
			     pl->timer->stop_min);

  if (pl->timer->timecontinued)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->timecontinued),
				  TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->timecontinued),
				  FALSE);

  if (pl->priority)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->priority), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->priority), FALSE);

  if (pl->spotcontroller)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->spotcontroller),
				  TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->spotcontroller),
				  FALSE);

  if (pl->softstop)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->softstop), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->softstop), FALSE);

  gtk_entry_set_text (GTK_ENTRY (data->jingle), pl->jingle ? pl->jingle : "");

  gtk_entry_set_text (GTK_ENTRY (data->prespot),
		      pl->prespot ? pl->prespot : "");

  gtk_entry_set_text (GTK_ENTRY (data->postspot),
		      pl->postspot ? pl->postspot : "");

  gtk_entry_set_text (GTK_ENTRY (data->stream), pl->stream ? pl->stream : "");

  gtk_entry_set_text (GTK_ENTRY (data->module), pl->module ? pl->module : "");

  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (data->moduledata),
			    pl->moduledata ? pl->moduledata : "", -1);

  module = data->modules_widget;

  while (module)
    {
      mt = module->data;

      moduledata = pl->moduledata ? g_strdup (pl->moduledata) : NULL;
      mt->module->set_value (mt->widget, moduledata);
      if (moduledata)
	g_free (moduledata);

      module = module->next;
    }

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->ratioitem),
			     pl->ratioitem);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->ratiospot),
			     pl->ratiospot);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->randomitem),
				pl->randomitem);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->randomspot),
				pl->randomspot);

  gtk_label_set_text (GTK_LABEL (data->duration_pathitem),
		      _("Duration: n/a"));
  gtk_label_set_text (GTK_LABEL (data->duration_pathspot),
		      _("Duration: n/a"));

  store =
    (GtkListStore *)
    gtk_tree_view_get_model (GTK_TREE_VIEW ((data->pathitem)));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  list = data->pl->pathitem;
  while (list)
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, list->data, -1);
      list = list->next;
    }

  store =
    (GtkListStore *)
    gtk_tree_view_get_model (GTK_TREE_VIEW ((data->pathspot)));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  list = data->pl->pathspot;
  while (list)
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, list->data, -1);
      list = list->next;
    }

  switch (pl->type)
    {
    case STREAM:
      gtk_combo_box_set_active (GTK_COMBO_BOX (data->type), 1);
      maker_pl_box_stream (data);
      break;

    case SILENCE:
      gtk_combo_box_set_active (GTK_COMBO_BOX (data->type), 2);
      maker_pl_box_silence (data);
      break;

    case MODULE:
      {
	int i = 0, t = 0;
	struct module_item *mt, *module_true = NULL;
	int len = strlen (MODULES_PATH);

	module = data->modules_widget;

	while (module)
	  {
	    mt = module->data;

	    if (pl->module
		&& (!strcmp (pl->module, mt->module->file)
		    || (strlen (mt->module->file) > len
			&& !strcmp (pl->module, mt->module->file + len + 1))))
	      {
		t = i;
		module_true = mt;
	      }

	    module = module->next;
	    i++;
	  }

	if (!module_true)
	  {
	    gtk_combo_box_set_active (GTK_COMBO_BOX (data->type), 3);
	    maker_pl_box_module (data);
	  }

	else
	  {
	    gtk_combo_box_set_active (GTK_COMBO_BOX (data->type), t + 4);
	    maker_pl_box_module_so (data, module_true);
	  }
      }

      break;

    default:
      gtk_combo_box_set_active (GTK_COMBO_BOX (data->type), 0);
      maker_pl_box_files (data);
      break;
    }

  data->refresh_lock = FALSE;
  maker_pl_refresh (data);
}

static void
maker_pl_save_t (GtkWidget * w, struct maker_pl_data_t *data)
{
  maker_pl_save_real_t (w, data, data->pl);
  maker_pl_refresh (data);
}

static void
maker_pl_save_real_t (GtkWidget * w, struct maker_pl_data_t *data,
		      struct somad_data *pl)
{
  int i;
  struct tm *k;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTextIter start, end;
  gchar *c;

  if (!(k = get_time ()))
    return;

  if (pl)
    {

      if (pl->description)
	g_free (pl->description);

      if ((c = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->description)))
	  && *c)
	pl->description = g_strdup (c);
      else
	pl->description = NULL;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->start_year));
      if (i <= 0)
	i = -1;
      else
	i += k->tm_year + 1900 - 1;

      pl->timer->start_year = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->start_month));
      if (i <= 0)
	i = -1;
      else
	i -= 1;

      pl->timer->start_month = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->start_mday));
      if (i <= 0)
	i = -1;

      pl->timer->start_mday = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->start_wday));
      if (i <= 0)
	i = -1;
      else
	i -= 1;
      pl->timer->start_wday = i;

      pl->timer->start_hour =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->start_hour));
      pl->timer->start_min =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->start_min));

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->stop_year));
      if (i <= 0)
	i = -1;
      else
	i += k->tm_year + 1900 - 1;

      pl->timer->stop_year = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->stop_month));
      if (i <= 0)
	i = -1;
      else
	i -= 1;

      pl->timer->stop_month = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->stop_mday));
      if (i <= 0)
	i = -1;

      pl->timer->stop_mday = i;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->stop_wday));
      if (i <= 0)
	i = -1;
      else
	i -= 1;
      pl->timer->stop_wday = i;

      pl->timer->stop_hour =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->stop_hour));
      pl->timer->stop_min =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->stop_min));

      pl->timer->timecontinued =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
				      (data->timecontinued));

      pl->priority =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->priority));

      pl->spotcontroller =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
				      (data->spotcontroller));

      pl->softstop =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->softstop));

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->type));

      switch (i)
	{
	case 0:
	  pl->type = FILES;
	  break;
	case 1:
	  pl->type = STREAM;
	  break;
	case 2:
	  pl->type = SILENCE;
	  break;
	case 3:
	  pl->type = MODULE;
	  break;
	default:
	  pl->type = MODULE;
	  break;
	}

      if (pl->jingle)
	g_free (pl->jingle);

      if ((c = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->jingle))) && *c)
	pl->jingle = g_strdup (c);
      else
	pl->jingle = NULL;

      if (pl->prespot)
	g_free (pl->prespot);

      if ((c = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->prespot)))
	  && *c)
	pl->prespot = g_strdup (c);
      else
	pl->prespot = NULL;

      if (pl->postspot)
	g_free (pl->postspot);

      if ((c = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->postspot)))
	  && *c)
	pl->postspot = g_strdup (c);
      else
	pl->postspot = NULL;

      if (pl->stream)
	g_free (pl->stream);

      if ((c = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->stream))) && *c)
	pl->stream = g_strdup (c);
      else
	pl->stream = NULL;

      if (pl->module)
	g_free (pl->module);

      pl->module = NULL;

      if (pl->moduledata)
	g_free (pl->moduledata);

      pl->moduledata = NULL;

      i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->type));

      if (i == 3)
	{
	  if ((c = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->module)))
	      && *c)
	    pl->module = g_strdup (c);

	  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER
					      (data->moduledata), &start, 0);
	  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (data->moduledata),
					&end);

	  if ((c =
	       gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (data->moduledata),
					  &start, &end, TRUE)) && *c)
	    pl->moduledata = g_strdup (c);
	}

      else if (i > 3)
	{
	  GList *module;
	  struct module_item *mt;

	  i -= 4;

	  module = data->modules_widget;
	  while (module)
	    {
	      if (!i)
		break;
	      module = module->next;
	      i--;
	    }

	  if (module)
	    {
	      mt = (struct module_item *) module->data;
	      pl->module = g_strdup (mt->module->file);
	      pl->moduledata = mt->module->get_value (mt->widget);
	    }
	}

      pl->ratioitem =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->ratioitem));
      pl->ratiospot =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->ratiospot));

      pl->randomitem =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->randomitem));

      pl->randomspot =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->randomspot));

      if (pl->pathitem)
	{
	  GList *list;

	  list = pl->pathitem;
	  while (list)
	    {
	      g_free (list->data);
	      list = list->next;
	    }

	  g_list_free (pl->pathitem);
	  pl->pathitem = NULL;
	}

      model = gtk_tree_view_get_model (GTK_TREE_VIEW ((data->pathitem)));
      if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
	{
	  char *text;

	  do
	    {
	      gtk_tree_model_get (model, &iter, 0, &text, -1);
	      pl->pathitem = g_list_append (pl->pathitem, text);
	    }
	  while (gtk_tree_model_iter_next (model, &iter) == TRUE);
	}

      if (pl->pathspot)
	{
	  GList *list;

	  list = pl->pathspot;
	  while (list)
	    {
	      g_free (list->data);
	      list = list->next;
	    }

	  g_list_free (pl->pathspot);
	  pl->pathspot = NULL;
	}

      model = gtk_tree_view_get_model (GTK_TREE_VIEW ((data->pathspot)));
      if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
	{
	  char *text;

	  do
	    {
	      gtk_tree_model_get (model, &iter, 0, &text, -1);
	      pl->pathspot = g_list_append (pl->pathspot, text);
	    }
	  while (gtk_tree_model_iter_next (model, &iter) == TRUE);
	}

      if (pl->timer->start)
	g_free (pl->timer->start);

      pl->timer->start = NULL;

      if (pl->timer->stop)
	g_free (pl->timer->stop);

      pl->timer->stop = NULL;

      timer_time_to_string (pl->timer, &pl->timer->start, &pl->timer->stop);
    }
}

void
maker_pl_sync (struct maker_pl_data_t *data)
{
  maker_pl_save_t (NULL, data);
}

void
maker_pl_file_saved (struct maker_pl_data_t *data)
{
  data->is_saved = 0;
}

int
maker_pl_is_no_saved (struct maker_pl_data_t *data)
{
  return data->is_saved;
}

static void
maker_pl_remove_t (GtkWidget * w, struct maker_pl_data_t *data)
{
  if (!data->pl)
    return;

  if (dialog_ask (_("Sure to remove this transmission?")) != GTK_RESPONSE_OK)
    return;

  maker_pl_remove_pl (data);
}

struct somad_data *
maker_pl_dump_pl (struct maker_pl_data_t *data)
{
  struct somad_data *tmp;
  GList *list;

  if (!data->pl)
    return NULL;

  tmp = g_malloc (sizeof (struct somad_data));
  memcpy (tmp, data->pl, sizeof (struct somad_data));

  list = data->pl->pathitem;
  tmp->pathitem = NULL;
  while (list)
    {
      tmp->pathitem =
	g_list_append (tmp->pathitem, g_strdup ((gchar *) list->data));
      list = list->next;
    }

  list = data->pl->pathspot;
  tmp->pathspot = NULL;
  while (list)
    {
      tmp->pathspot =
	g_list_append (tmp->pathspot, g_strdup ((gchar *) list->data));
      list = list->next;
    }

  if (data->pl->stream)
    tmp->stream = g_strdup (data->pl->stream);

  if (data->pl->jingle)
    tmp->jingle = g_strdup (data->pl->jingle);

  if (data->pl->prespot)
    tmp->prespot = g_strdup (data->pl->prespot);

  if (data->pl->postspot)
    tmp->postspot = g_strdup (data->pl->postspot);

  if (data->pl->module)
    tmp->module = g_strdup (data->pl->module);

  if (data->pl->moduledata)
    tmp->moduledata = g_strdup (data->pl->moduledata);

  tmp->timer = g_malloc (sizeof (struct somad_time));
  memcpy (tmp->timer, data->pl->timer, sizeof (struct somad_time));

  if (data->pl->timer->start)
    tmp->timer->start = g_strdup (data->pl->timer->start);

  if (data->pl->timer->stop)
    tmp->timer->stop = g_strdup (data->pl->timer->stop);

  if (data->pl->description)
    tmp->description = g_strdup (data->pl->description);

  tmp->color = g_malloc (sizeof (GdkColor));
  memcpy (tmp->color, data->pl->color, sizeof (GdkColor));

  tmp->next = NULL;

  return tmp;
}

void
maker_pl_remove_pl (struct maker_pl_data_t *data)
{
  maker_pl_remove_pl_real (data, data->pl, TRUE, TRUE);
  data->pl = NULL;
}

static void
maker_pl_new_t (GtkWidget * w, struct maker_pl_data_t *data)
{
  struct somad_data *tmp;

  tmp = g_malloc0 (sizeof (struct somad_data));
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

  tmp->type = FILES;
  tmp->softstop = 1;
  tmp->ratioitem = 10;
  tmp->ratiospot = 2;
  tmp->color = draw_color ();

  timer_time_to_string (tmp->timer, &tmp->timer->start, &tmp->timer->stop);

  maker_pl_insert_pl (data, tmp);
  maker_pl_undoredo_new (data, MAKER_UNDOREDO_NEW, TRUE,
			 _("New trasmission created"));
}

static void
maker_pl_dump_switch_pl (struct maker_pl_data_t *data)
{
  struct somad_data *pl = maker_pl_dump_pl (data);

  if (data->maker_pl == data->pl)
    {
      pl->next = data->maker_pl->next;
      data->maker_pl = pl;
    }
  else
    {
      struct somad_data *tmp = data->maker_pl;
      while (tmp->next)
	{
	  if (tmp->next == data->pl)
	    {
	      pl->next = tmp->next->next;
	      tmp->next = pl;
	      break;
	    }

	  tmp = tmp->next;
	}
    }

  data->pl = pl;
}

void
maker_pl_insert_pl (struct maker_pl_data_t *data, struct somad_data *pl)
{
  palinsesto_insert (pl, &data->maker_pl);
  maker_pl_data_show (pl, data);
}

static void
maker_pl_list_clicked (GtkWidget * w, struct maker_pl_data_t *data)
{
  maker_pl_data_show (g_object_get_data (G_OBJECT (w), "transmission"), data);
}

static void
maker_pl_list_refresh (struct maker_pl_data_t *data)
{
  GtkWidget *box;
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *label;
  GtkStyle *style;
  struct somad_data *tmp;
  gchar *str;
  char s[1024];

  if (data->frame_list_w)
    gtk_container_remove (GTK_CONTAINER (data->frame_list),
			  data->frame_list_w);

  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (data->frame_list), box);

  data->frame_list_w = box;

  tmp = data->maker_pl;
  while (tmp)
    {
      button = gtk_button_new ();
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
      g_object_set_data (G_OBJECT (button), "transmission", tmp);

      style = gtk_style_copy (gtk_widget_get_default_style ());
      style->bg[GTK_STATE_NORMAL] = *tmp->color;
      gtk_widget_set_style (button, style);

      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (maker_pl_list_clicked), data);

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

static void
maker_pl_calendar_today (GtkWidget * w, struct maker_pl_data_t *data)
{
  struct tm *k;
  int y, m, d;
  k = get_time ();

  y = k->tm_year + 1900;
  m = k->tm_mon;
  d = k->tm_mday;

  gtk_calendar_select_month (GTK_CALENDAR (data->calendar), (guint) m,
			     (guint) y);
  gtk_calendar_select_day (GTK_CALENDAR (data->calendar), (guint) d);

}

static void
maker_pl_calendar_changed (GtkWidget * w, struct maker_pl_data_t *data)
{
  maker_pl_refresh (data);
}

static time_t
maker_pl_calendar_time (GtkWidget * w)
{
  GDate *date;
  guint y, m, d;
  struct tm k;

  gtk_calendar_get_date (GTK_CALENDAR (w), &y, &m, &d);

  if ((date = g_date_new_dmy (d, m + 1, y)))
    {
      g_date_to_struct_tm (date, &k);
      g_date_free (date);
      return mktime (&k);
    }

  else
    return 0;
}

void
maker_pl_refresh (struct maker_pl_data_t *data)
{
  draw_refresh (data->draw, data->maker_pl, TRUE,
		maker_pl_calendar_time (data->calendar), FALSE);
  maker_pl_list_refresh (data);
}

void
maker_pl_changed (GtkWidget * w, struct maker_pl_data_t *data)
{
  if (maker_changed_check (w) == FALSE)
    return;

  if (data->refresh_lock == FALSE)
    {
      maker_pl_undoredo (w, data);
      maker_pl_save_t (NULL, data);
      data->is_saved = 1;
    }
}

static void
maker_pl_box_hide_all (struct maker_pl_data_t *data)
{
  GList *list;

  gtk_widget_hide (data->box_silence);
  gtk_widget_hide (data->box_module);
  gtk_widget_hide (data->box_files);
  gtk_widget_hide (data->box_stream);
  gtk_widget_show_all (data->box_empty);

  list = data->modules_widget;
  while (list)
    {
      gtk_widget_hide (((struct module_item *) list->data)->widget_container);
      list = list->next;
    }
}

static void
maker_pl_box_files (struct maker_pl_data_t *data)
{
  GList *list;

  gtk_widget_hide (data->box_empty);
  gtk_widget_hide (data->box_silence);
  gtk_widget_hide (data->box_module);
  gtk_widget_show_all (data->box_files);
  gtk_widget_hide (data->box_stream);

  list = data->modules_widget;
  while (list)
    {
      gtk_widget_hide (((struct module_item *) list->data)->widget_container);
      list = list->next;
    }
}

static void
maker_pl_box_stream (struct maker_pl_data_t *data)
{
  GList *list;

  gtk_widget_hide (data->box_empty);
  gtk_widget_hide (data->box_silence);
  gtk_widget_hide (data->box_module);
  gtk_widget_hide (data->box_files);
  gtk_widget_show_all (data->box_stream);

  list = data->modules_widget;
  while (list)
    {
      gtk_widget_hide (((struct module_item *) list->data)->widget_container);
      list = list->next;
    }
}

static void
maker_pl_box_silence (struct maker_pl_data_t *data)
{
  GList *list;

  gtk_widget_hide (data->box_empty);
  gtk_widget_show_all (data->box_silence);
  gtk_widget_hide (data->box_module);
  gtk_widget_hide (data->box_files);
  gtk_widget_hide (data->box_stream);

  list = data->modules_widget;
  while (list)
    {
      gtk_widget_hide (((struct module_item *) list->data)->widget_container);
      list = list->next;
    }
}

static void
maker_pl_box_module (struct maker_pl_data_t *data)
{
  GList *list;

  gtk_widget_hide (data->box_empty);
  gtk_widget_hide (data->box_silence);
  gtk_widget_show_all (data->box_module);
  gtk_widget_hide (data->box_files);
  gtk_widget_hide (data->box_stream);

  list = data->modules_widget;
  while (list)
    {
      gtk_widget_hide (((struct module_item *) list->data)->widget_container);
      list = list->next;
    }
}

static void
maker_pl_box_module_so (struct maker_pl_data_t *data,
			struct module_item *item)
{
  GList *list;

  gtk_widget_hide (data->box_empty);
  gtk_widget_hide (data->box_silence);
  gtk_widget_hide (data->box_module);
  gtk_widget_hide (data->box_files);
  gtk_widget_hide (data->box_stream);

  list = data->modules_widget;
  while (list)
    {
      if (list->data != item)
	gtk_widget_hide (((struct module_item *) list->data)->
			 widget_container);
      list = list->next;
    }

  gtk_widget_show_all (item->widget_container);
}

static void
maker_pl_changed_type (GtkWidget * w, struct maker_pl_data_t *data)
{
  int i;

  i = gtk_combo_box_get_active (GTK_COMBO_BOX (data->type));

  switch (i)
    {
    case 1:
      maker_pl_box_stream (data);
      break;

    case 2:
      maker_pl_box_silence (data);
      break;

    case 3:
      maker_pl_box_module (data);
      break;

    default:

      if (i > 3)
	{
	  GList *list;
	  i -= 4;

	  list = data->modules_widget;
	  while (list)
	    {
	      if (!i)
		{
		  maker_pl_box_module_so (data,
					  (struct module_item *) list->data);
		  i = -1;
		  break;
		}

	      list = list->next;
	      i--;
	    }

	  if (i != -1)
	    maker_pl_box_files (data);

	}
      else
	maker_pl_box_files (data);

      break;
    }

  maker_pl_changed (w, data);
}

static void
maker_pl_file_l_add (GtkWidget * w, GtkWidget * entry)
{
  char *file;
  char *tmp;

  if (!
      (file =
       file_chooser (_("Search a local file"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_OPEN)))
    return;

  tmp = somax_to_utf8 (file);
  gtk_entry_set_text (GTK_ENTRY (entry), tmp);
  g_free (tmp);
  g_free (file);
}

static void
maker_pl_treeview_l_add (GtkWidget * w, GtkWidget * t)
{
  file_chooser_cb (_("Add files"), GTK_SELECTION_MULTIPLE,
		   GTK_FILE_CHOOSER_ACTION_OPEN, maker_pl_treeview_l_add_cb,
		   t);
}

static void
maker_pl_treeview_l_add_cb (void *l, void *t)
{
  GSList *file;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *tmp;
  struct maker_pl_data_t *pl;

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

  if ((pl =
       (struct maker_pl_data_t *) g_object_get_data (G_OBJECT (t),
						     "data_pl")))
    maker_pl_changed (NULL, pl);
}

static void
maker_pl_treeview_l_add_dir (GtkWidget * w, GtkWidget * t)
{
  file_chooser_cb (_("Add directories"), GTK_SELECTION_MULTIPLE,
		   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		   maker_pl_treeview_l_add_dir_cb, t);
}

static void
maker_pl_treeview_l_add_dir_cb (void *l, void *t)
{
  GSList *file;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *tmp;
  struct maker_pl_data_t *pl;

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

  if ((pl = g_object_get_data (G_OBJECT (t), "data_pl")))
    maker_pl_changed (NULL, pl);
}

static void
maker_pl_treeview_remove (GtkWidget * w, GtkWidget * t)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *list, *b;
  int j, i = 0;
  struct maker_pl_data_t *pl;

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

  if ((pl =
       (struct maker_pl_data_t *) g_object_get_data (G_OBJECT (t),
						     "data_pl")))
    maker_pl_changed (NULL, pl);

}

void
maker_pl_open (GtkWidget * w, struct maker_pl_data_t *data)
{
  char *file;

  if ((maker_pl_is_no_saved (data)
       && dialog_ask (_("Open new file without save?")) == GTK_RESPONSE_OK)
      || !maker_pl_is_no_saved (data))
    {

      if (!
	  (file =
	   file_chooser (_("Open a palinsesto file"), GTK_SELECTION_SINGLE,
			 GTK_FILE_CHOOSER_ACTION_OPEN)))
	return;

      if (data->maker_pl)
	palinsesto_free (data->maker_pl);
      data->maker_pl = NULL;

      if (palinsesto_parser_file (file, &data->maker_pl))
	dialog_msg (_("Somad Palinsesto syntax error"));

      else
	{
	  if (data->maker_pl_file)
	    g_free (data->maker_pl_file);

	  data->maker_pl_file = g_strdup (file);

	  maker_pl_refresh (data);
	}

      g_free (file);
    }
}

void
maker_pl_change_color (struct maker_pl_data_t *data)
{
  GtkWidget *dialog;
  GtkWidget *button;

  if (!data->pl)
    {
      dialog_msg (_("No transmission selected!"));
      return;
    }

  dialog = gtk_color_selection_dialog_new (_("Select the color..."));
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  button = GTK_COLOR_SELECTION_DIALOG (dialog)->ok_button;
  gtk_widget_show (button);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  button = GTK_COLOR_SELECTION_DIALOG (dialog)->cancel_button;
  gtk_widget_show (button);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_color_selection_set_current_color (GTK_COLOR_SELECTION
					 (GTK_COLOR_SELECTION_DIALOG
					  (dialog)->colorsel),
					 data->pl->color);

  gtk_widget_show (dialog);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      GdkColor *color;

      color = (GdkColor *) g_malloc (sizeof (GdkColor));

      gtk_color_selection_get_current_color (GTK_COLOR_SELECTION
					     (GTK_COLOR_SELECTION_DIALOG
					      (dialog)->colorsel), color);

      maker_pl_undoredo_new (data, MAKER_UNDOREDO_CHANGE, TRUE,
			     _("Color changed"));

      g_free (data->pl->color);
      data->pl->color = color;

      maker_pl_data_show (data->pl, data);
    }

  gtk_widget_destroy (dialog);
}

static void
maker_pl_duration_pathspot (GtkWidget * widget, struct maker_pl_data_t *data)
{
  maker_pl_duration (data, data->pathspot, data->duration_pathspot);
}


static void
maker_pl_duration_pathitem (GtkWidget * widget, struct maker_pl_data_t *data)
{
  maker_pl_duration (data, data->pathitem, data->duration_pathitem);
}

static void
maker_pl_duration (struct maker_pl_data_t *data, GtkWidget * tv,
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
maker_pl_selected (GtkTreeView * tree, GtkTreePath * path,
		   GtkTreeViewColumn * column, struct maker_pl_data_t *data)
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
maker_pl_set_controller (struct maker_pl_data_t *data, void *controller)
{
  GList *list;

  data->controller = controller;

  for (list = data->controller_widgets; list; list = list->next)
    gtk_widget_set_sensitive (GTK_WIDGET (list->data),
			      controller ? TRUE : FALSE);
}

static void
maker_pl_remove_pl_real (struct maker_pl_data_t *data, struct somad_data *pl,
			 gboolean undo, gboolean insert)
{
  if (!pl)
    return;

  if (data->maker_pl == pl)
    {
      data->maker_pl = pl->next;
    }
  else
    {
      struct somad_data *tmp;
      tmp = data->maker_pl;

      while (tmp->next)
	{
	  if (tmp->next == pl)
	    {
	      tmp->next = tmp->next->next;
	      break;
	    }

	  tmp = tmp->next;
	}
    }

  if (insert == TRUE)
    maker_pl_undoredo_new (data, MAKER_UNDOREDO_DESTROY, undo,
			   _("Transmission removed"));

  gtk_widget_set_sensitive (data->frame, FALSE);

  maker_pl_refresh (data);
}

/* UNDOREDO */
static void
maker_pl_undoredo (GtkWidget * w, struct maker_pl_data_t *data)
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
  if (w && GTK_IS_ENTRY (w) && !GTK_IS_SPIN_BUTTON (w)
      && w == data->undoredo_current_widget && (!data->undoredo_timer
						|| (data->undoredo_timer
						    && g_timer_elapsed (data->
									undoredo_timer,
									NULL)
						    <=
						    MAKER_UNDOREDO_MAX_TIMER)))
    {
      if (!data->undoredo_timer)
	data->undoredo_timer = g_timer_new ();

      if (!data->undoredo_timeout)
	{
	  data->undoredo_current_value =
	    g_strdup (gtk_entry_get_text (GTK_ENTRY (w)));

	  data->undoredo_timeout =
	    g_timeout_add (MAKER_UNDOREDO_MAX_TIMER,
			   (GSourceFunc) maker_pl_undoredo_timeout, data);
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

  maker_changed_set (w);

  /* Dump: */
  message = maker_message (w);
  maker_pl_undoredo_new (data, MAKER_UNDOREDO_CHANGE, TRUE, message);
  g_free (message);

  /* Remove the last item: */
  if (g_list_length (data->undoredo_undo_list) >= MAKER_UNDOREDO_MAX_UNDO)
    {
      GList *last = g_list_last (data->undoredo_undo_list);
      maker_pl_undoredo_free (last->data);
      data->undoredo_undo_list =
	g_list_remove (data->undoredo_undo_list, last->data);
    }
}

static gboolean
maker_pl_undoredo_timeout (struct maker_pl_data_t *data)
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
    maker_pl_undoredo (data->undoredo_current_widget, data);

  return FALSE;
}

static void
maker_pl_undoredo_new (struct maker_pl_data_t *data,
		       enum maker_undoredo_type_t type, gboolean undo,
		       gchar * message)
{
  struct maker_pl_undoredo_t *new;

  new = g_malloc0 (sizeof (struct maker_pl_undoredo_t));
  new->message = g_strdup (message);

  switch (type)
    {
    case MAKER_UNDOREDO_NEW:
    case MAKER_UNDOREDO_DESTROY:
      new->pl = data->pl;
      break;

    case MAKER_UNDOREDO_CHANGE:
      new->previous_pl = data->pl;
      maker_pl_dump_switch_pl (data);
      new->pl = data->pl;
      break;
    }

  new->type = type;

  if (undo == TRUE)
    data->undoredo_undo_list = g_list_prepend (data->undoredo_undo_list, new);
  else
    data->undoredo_redo_list = g_list_prepend (data->undoredo_redo_list, new);
}

static void
maker_pl_undoredo_free (struct maker_pl_undoredo_t *data)
{
  if (data->previous_pl)
    {
      data->previous_pl = NULL;
      palinsesto_free (data->previous_pl);
    }

  if (data->pl)
    {
      data->pl = NULL;
      palinsesto_free (data->pl);
    }

  if (data->message)
    g_free (data->message);

  g_free (data);
}

void
maker_pl_undoredo_status (struct maker_pl_data_t *data, gboolean * undo,
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
maker_pl_undoredo_work_n (GtkWidget * widget, struct maker_pl_data_t *data)
{
  gint id = (gint) g_object_get_data (G_OBJECT (widget), "id");
  gboolean what = (gboolean) g_object_get_data (G_OBJECT (widget), "what");

  for (; id >= 0; id--)
    maker_pl_undoredo_work (data, what);
}

void
maker_pl_undoredo_work (struct maker_pl_data_t *data, gboolean what)
{
  GList **list, **undo_list;
  struct maker_pl_undoredo_t *undo;
  struct somad_data *tmp;

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
      maker_pl_remove_pl_real (data, undo->pl, !what, FALSE);
      undo->type = MAKER_UNDOREDO_DESTROY;
      *undo_list = g_list_prepend (*undo_list, undo);
      break;

    case MAKER_UNDOREDO_DESTROY:
      maker_pl_insert_pl (data, undo->pl);
      undo->type = MAKER_UNDOREDO_NEW;
      *undo_list = g_list_prepend (*undo_list, undo);
      break;

    case MAKER_UNDOREDO_CHANGE:
      maker_pl_remove_pl_real (data, undo->pl, !what, FALSE);
      maker_pl_insert_pl (data, undo->previous_pl);
      tmp = undo->pl;
      undo->pl = undo->previous_pl;
      undo->previous_pl = tmp;

      *undo_list = g_list_prepend (*undo_list, undo);
      break;
    }

  maker_pl_refresh (data);
}

static void
maker_pl_destroy (GtkWidget * w, struct maker_pl_data_t *data)
{
  g_list_foreach (data->undoredo_undo_list, (GFunc) maker_pl_undoredo_free,
		  NULL);
  data->undoredo_undo_list = NULL;
  g_list_foreach (data->undoredo_redo_list, (GFunc) maker_pl_undoredo_free,
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
maker_pl_undoredo_history (struct maker_pl_data_t *data, gboolean what,
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
	gtk_menu_item_new_with_label (((struct maker_pl_undoredo_t *) list->
				       data)->message);
      g_object_set_data (G_OBJECT (item), "id", (gpointer) i);
      g_object_set_data (G_OBJECT (item), "what", (gpointer) what);
      gtk_widget_show (item);
      gtk_container_add (GTK_CONTAINER (menu), item);

      g_signal_connect ((gpointer) item, "activate",
			G_CALLBACK (maker_pl_undoredo_work_n), data);
    }

  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, maker_popup_position, w, 0, 0);
}

/* EOF */
