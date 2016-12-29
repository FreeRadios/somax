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
#include "../timer/timer.h"

struct somax_preferences_t *preferences_data = NULL;

static void request_infos_cb (GtkWidget * widget, GtkWidget * label);

void
preferences (void)
{
  GtkWidget *dialog;
  GtkWidget *frame;
  GtkWidget *gbox;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *notebook;
  gchar s[1024];

  GtkWidget *request_infos;
  GtkWidget *request_infos_label;
  GtkWidget *info_download;
  GtkWidget *window_size;
  GtkWidget *fullscreen;
  GtkWidget *statusbar_show;
  GtkWidget *timer_pb_show;
  GtkWidget *lists;
  GtkWidget *item_infos_show;
  GtkWidget *next_item_infos_show;
  GtkWidget *timer_configure;

  dialog = gtk_dialog_new ();
  g_snprintf (s, sizeof (s), "%s %s - %s", PACKAGE, VERSION,
	      _("Preferences..."));
  gtk_window_set_title (GTK_WINDOW (dialog), s);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  notebook = gtk_notebook_new ();
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), notebook, TRUE,
		      TRUE, 0);

  /* INTERFACE */
  gbox = gtk_vbox_new (FALSE, 8);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), gbox,
			    gtk_label_new (_("Interface")));

  frame = gtk_frame_new (_("Window"));
  gtk_box_pack_start (GTK_BOX (gbox), frame, FALSE, FALSE, 0);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  label = gtk_label_new (_("Save the window size:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  window_size = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (window_size),
				preferences_data->window_size);
  gtk_box_pack_start (GTK_BOX (hbox), window_size, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  label = gtk_label_new (_("Fullscreen:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  fullscreen = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fullscreen),
				preferences_data->fullscreen);
  gtk_box_pack_start (GTK_BOX (hbox), fullscreen, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  label = gtk_label_new (_("Show infos about the current item:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  item_infos_show = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (item_infos_show),
				preferences_data->item_infos);
  gtk_box_pack_start (GTK_BOX (hbox), item_infos_show, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  label = gtk_label_new (_("Show infos about the follow item:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  next_item_infos_show = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (next_item_infos_show),
				preferences_data->next_item_infos);
  gtk_box_pack_start (GTK_BOX (hbox), next_item_infos_show, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  label = gtk_label_new (_("Show the statusbar:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  statusbar_show = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (statusbar_show),
				preferences_data->statusbar);
  gtk_box_pack_start (GTK_BOX (hbox), statusbar_show, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  label = gtk_label_new (_("Show Somad Progress Bar:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  timer_pb_show = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (timer_pb_show),
				preferences_data->timer_pb);
  gtk_box_pack_start (GTK_BOX (hbox), timer_pb_show, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  label = gtk_label_new (_("Show the lists (the left part of the UI):"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  lists = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (lists),
				preferences_data->lists);
  gtk_box_pack_start (GTK_BOX (hbox), lists, FALSE, FALSE, 0);

  /* INTERFACE */
  gbox = gtk_vbox_new (FALSE, 8);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), gbox,
			    gtk_label_new (_("Timer")));

  frame = gtk_frame_new (_("Timer"));
  gtk_box_pack_start (GTK_BOX (gbox), frame, FALSE, FALSE, 0);

  timer_configure = timer_preferences ();
  gtk_container_add (GTK_CONTAINER (frame), timer_configure);

  /* SOMAD */
  gbox = gtk_vbox_new (FALSE, 8);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), gbox,
			    gtk_label_new (_("Somad")));

  frame = gtk_frame_new (_("Request informations"));
  gtk_box_pack_start (GTK_BOX (gbox), frame, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  label = gtk_label_new (_("Request informations:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  request_infos = gtk_hscale_new_with_range (1, 99, 1);
  gtk_range_set_value (GTK_RANGE (request_infos),
		       (gdouble) preferences_data->request_infos);
  gtk_scale_set_draw_value (GTK_SCALE (request_infos), FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), request_infos, FALSE, FALSE, 0);

  request_infos_label = gtk_label_new ("20 seconds");
  gtk_box_pack_start (GTK_BOX (vbox), request_infos_label, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) request_infos, "value_changed",
		    G_CALLBACK (request_infos_cb), request_infos_label);
  request_infos_cb (request_infos, request_infos_label);

  frame = gtk_frame_new (_("Infos about files"));
  gtk_box_pack_start (GTK_BOX (gbox), frame, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  label =
    gtk_label_new (_
		   ("Download Infos about files:\n(Only if you have a fast internet connection)"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  info_download = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (info_download),
				preferences_data->info_download);
  gtk_box_pack_start (GTK_BOX (hbox), info_download, FALSE, FALSE, 0);

  /* OTHER */
  button = gtk_button_new_from_stock ("gtk-cancel");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button,
				GTK_RESPONSE_CANCEL);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);

  while (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      if (timer_preferences_check (timer_configure) == FALSE)
	continue;

      preferences_data->request_infos =
	(gint) gtk_range_get_value (GTK_RANGE (request_infos));
      preferences_data->info_download =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (info_download));
      preferences_data->window_size =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window_size));

      if (preferences_data->fullscreen !=
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fullscreen)))
	{
	  if (preferences_data->fullscreen == TRUE)
	    gtk_window_unfullscreen (GTK_WINDOW (main_window));
	  else
	    gtk_window_fullscreen (GTK_WINDOW (main_window));

	  preferences_data->fullscreen = !preferences_data->fullscreen;
	}

      if (preferences_data->statusbar !=
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (statusbar_show)))
	{
	  if (preferences_data->statusbar == TRUE)
	    gtk_widget_hide (statusbar_widget);
	  else
	    gtk_widget_show (statusbar_widget);

	  preferences_data->statusbar = !preferences_data->statusbar;
	}

      if (preferences_data->timer_pb !=
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (timer_pb_show)))
	{
	  if (preferences_data->timer_pb == TRUE)
	    gtk_widget_hide (timer_pb);
	  else
	    gtk_widget_show (timer_pb);

	  preferences_data->timer_pb = !preferences_data->timer_pb;
	}

      if (preferences_data->lists !=
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (lists)))
	{
	  if (preferences_data->lists == TRUE)
	    gtk_widget_hide (lists_widget);
	  else
	    gtk_widget_show (lists_widget);

	  preferences_data->lists = !preferences_data->lists;
	}

      if (preferences_data->item_infos !=
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (item_infos_show)))
	{
	  if (preferences_data->item_infos == TRUE)
	    gtk_widget_hide (item_infos);
	  else
	    gtk_widget_show (item_infos);

	  preferences_data->item_infos = !preferences_data->item_infos;
	}

      if (preferences_data->next_item_infos !=
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					(next_item_infos_show)))
	{
	  if (preferences_data->next_item_infos == TRUE)
	    gtk_widget_hide (next_item_infos);
	  else
	    gtk_widget_show (next_item_infos);

	  preferences_data->next_item_infos =
	    !preferences_data->next_item_infos;
	}

      set_preferences (preferences_data);
      break;
    }

  gtk_widget_destroy (dialog);

}

static void
request_infos_cb (GtkWidget * widget, GtkWidget * label)
{
  gint value = (gint) gtk_range_get_value (GTK_RANGE (widget));
  gchar s[1024];

  snprintf (s, sizeof (s), "%d %s", value,
	    value != 1 ? _("seconds") : _("second"));
  gtk_label_set_text (GTK_LABEL (label), s);
}

/* EOF */
