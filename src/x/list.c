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

#define LABEL_NEW( x ) label = gtk_label_new (x); \
  gtk_widget_show (label); \
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0); \
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

#define DATA_STRING( x ) str=somax_markup(x); \
  snprintf (s, sizeof (s), "<b>%s</b>", str); \
  g_free(str);

#define DATA_NEW( x )  label = gtk_label_new (s); \
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE); \
  gtk_widget_show (label); \
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0); \
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);

void
list_today (GtkWidget * w, gpointer dummy)
{

  struct tm *k;
  int y, m, d;

  k = get_time ();

  y = k->tm_year + 1900;
  m = k->tm_mon;
  d = k->tm_mday;

  gtk_calendar_select_month (GTK_CALENDAR (calendar), (guint) m, (guint) y);
  gtk_calendar_select_day (GTK_CALENDAR (calendar), (guint) d);
}

void
list_refresh (GtkWidget * w, gpointer dummy)
{
  static int y = -1, m = -1, d = -1, day;
  struct somad_data *tmp;
  GtkWidget *box;
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *sep;
  GtkWidget *label;
  char s[1024];
  gchar *str;

  if (y < 0)
    {
      struct tm *k;
      k = get_time ();

      y = k->tm_year + 1900;
      m = k->tm_mon;
      d = k->tm_mday;
      day = k->tm_wday;

      gtk_calendar_select_month (GTK_CALENDAR (calendar), (guint) m,
				 (guint) y);
      gtk_calendar_select_day (GTK_CALENDAR (calendar), (guint) d);

    }
  else if (w)
    {
      GDate *date;

      gtk_calendar_get_date (GTK_CALENDAR (w), (guint *) & y, (guint *) & m,
			     (guint *) & d);

      if ((date = g_date_new_dmy (d, m + 1, y)))
	{
	  day = g_date_get_weekday (date);
	  g_date_free (date);
	}

      else
	return;

    }

  gtk_container_remove (GTK_CONTAINER (p_list), w_list);

  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (p_list), box);

  snprintf (s, sizeof (s), "<b>%s, %d %s %d</b>", somax_day (day), d,
	    somax_month (m), y);
  label = gtk_label_new (s);
  gtk_widget_show (label);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  sep = gtk_hseparator_new ();
  gtk_widget_show (sep);
  gtk_box_pack_start (GTK_BOX (box), sep, FALSE, FALSE, 0);

  tmp = somad_pl;
  while (tmp)
    {

      if (!timer_check_day (tmp->timer, y, m, d, day))
	{

	  button = gtk_button_new ();
	  gtk_widget_show (button);
	  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

	  g_signal_connect ((gpointer) button, "clicked",
			    G_CALLBACK (list_show), tmp);

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

	}

      tmp = tmp->next;
    }

  w_list = box;
}

void
list_show (GtkWidget * w, struct somad_data *data)
{
  GtkWidget *dialog;
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *button;
  char s[1024];
  gchar *str;
  int i = 0;

  if (!data)
    return;

  snprintf (s, sizeof (s), "%s %s - %s", PACKAGE, VERSION, data->description);

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), s);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  frame = gtk_frame_new (data->description);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE,
		      0);

  table = gtk_table_new (0, 0, FALSE);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (frame), table);

  LABEL_NEW (_("Description:"));
  DATA_STRING (data->description);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Priority:"));

  snprintf (s, sizeof (s), "<b>%s</b>", data->priority ? "true" : "false");
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Start:"));
  DATA_STRING (data->timer->start);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Stop:"));
  DATA_STRING (data->timer->stop);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("TimeContinued:"));

  snprintf (s, sizeof (s), "<b>%s</b>",
	    data->timer->timecontinued ? "true" : "false");
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("SpotController:"));

  snprintf (s, sizeof (s), "<b>%s</b>",
	    data->spotcontroller ? "true" : "false");
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Jingle:"));
  DATA_STRING (data->jingle);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("PreSpot:"));
  DATA_STRING (data->prespot);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("PostSpot:"));
  DATA_STRING (data->postspot);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Type:"));

  switch (data->type)
    {
    case STREAM:
      snprintf (s, sizeof (s), _("<b>Stream</b>"));
      break;
    case MODULE:
      snprintf (s, sizeof (s), _("<b>Module</b>"));
      break;
    case SILENCE:
      snprintf (s, sizeof (s), _("<b>Silence</b>"));
      break;
    default:
      snprintf (s, sizeof (s), _("<b>Files</b>"));
      break;
    }

  DATA_NEW (s);
  i++;

  switch (data->type)
    {
    case STREAM:
      LABEL_NEW (_("Stream:"));
      DATA_STRING (data->stream);
      DATA_NEW (s);
      i++;

      break;

    case MODULE:
      LABEL_NEW (_("Module:"));
      DATA_STRING (data->module);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Module Data:"));
      DATA_STRING (data->moduledata);
      DATA_NEW (s);
      i++;

      break;

    default:
      LABEL_NEW (_("N. Item:"));

      snprintf (s, sizeof (s), "<b>%d</b>",
		data->pathitem ? g_list_length (data->pathitem) : 0);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("N. Spot:"));

      snprintf (s, sizeof (s), "<b>%d</b>",
		data->pathspot ? g_list_length (data->pathspot) : 0);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Ratio Item:"));

      snprintf (s, sizeof (s), "<b>%d</b>", data->ratioitem);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Ratio Spot:"));

      snprintf (s, sizeof (s), "<b>%d</b>", data->ratiospot);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Random Item:"));

      snprintf (s, sizeof (s), "<b>%s</b>",
		data->randomitem ? "true" : "false");
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Random Spot:"));

      snprintf (s, sizeof (s), "<b>%s</b>",
		data->randomspot ? "true" : "false");
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("SoftStop:"));

      snprintf (s, sizeof (s), "<b>%s</b>",
		data->softstop ? "true" : "false");
      DATA_NEW (s);
      i++;

      break;

    }

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

void
list_show_list (GtkWidget * w, GList * list)
{
  struct somad_data *data;
  GtkWidget *dialog;
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *notebook;
  GtkWidget *label;
  GtkWidget *button;
  char s[1024];
  gchar *str;
  int i = 0;
  int page;

  if (!list)
    return;

  if (g_list_length (list) == 1)
    {
      list_show (NULL, list->data);
      return;
    }

  snprintf (s, sizeof (s), _("%s %s - Transmissions"), PACKAGE, VERSION);

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), s);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), notebook, TRUE,
		      TRUE, 0);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

  while (list)
    {

      data = list->data;

      frame = gtk_frame_new (data->description);
      gtk_widget_show (frame);

      table = gtk_table_new (0, 0, FALSE);
      gtk_widget_show (table);
      gtk_container_add (GTK_CONTAINER (frame), table);

      LABEL_NEW (_("Description:"));
      DATA_STRING (data->description);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Priority:"));

      snprintf (s, sizeof (s), "<b>%s</b>",
		data->priority ? "true" : "false");
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Start:"));
      DATA_STRING (data->timer->start);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Stop:"));
      DATA_STRING (data->timer->stop);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("TimeContinued:"));

      snprintf (s, sizeof (s), "<b>%s</b>",
		data->timer->timecontinued ? "true" : "false");
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("SpotController:"));

      snprintf (s, sizeof (s), "<b>%s</b>",
		data->spotcontroller ? "true" : "false");
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Jingle:"));
      DATA_STRING (data->jingle);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("PreSpot:"));
      DATA_STRING (data->prespot);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("PostSpot:"));
      DATA_STRING (data->postspot);
      DATA_NEW (s);
      i++;

      LABEL_NEW (_("Type:"));

      switch (data->type)
	{
	case STREAM:
	  snprintf (s, sizeof (s), _("<b>Stream</b>"));
	  break;
	case MODULE:
	  snprintf (s, sizeof (s), _("<b>Module</b>"));
	  break;
	case SILENCE:
	  snprintf (s, sizeof (s), _("<b>Silence</b>"));
	  break;
	default:
	  snprintf (s, sizeof (s), _("<b>Files</b>"));
	  break;
	}

      DATA_NEW (s);
      i++;

      switch (data->type)
	{
	case STREAM:
	  LABEL_NEW (_("Stream:"));
	  DATA_STRING (data->stream);
	  DATA_NEW (s);
	  i++;

	  break;

	case MODULE:
	  LABEL_NEW (_("Module:"));
	  DATA_STRING (data->module);
	  DATA_NEW (s);
	  i++;

	  LABEL_NEW (_("ModuleData:"));
	  DATA_STRING (data->moduledata);
	  DATA_NEW (s);
	  i++;

	  break;

	default:
	  LABEL_NEW (_("N. Item:"));

	  snprintf (s, sizeof (s), "<b>%d</b>",
		    data->pathitem ? g_list_length (data->pathitem) : 0);
	  DATA_NEW (s);
	  i++;

	  LABEL_NEW (_("N. Spot:"));

	  snprintf (s, sizeof (s), "<b>%d</b>",
		    data->pathspot ? g_list_length (data->pathspot) : 0);
	  DATA_NEW (s);
	  i++;

	  LABEL_NEW (_("Ratio Item:"));

	  snprintf (s, sizeof (s), "<b>%d</b>", data->ratioitem);
	  DATA_NEW (s);
	  i++;

	  LABEL_NEW (_("Ratio Spot:"));

	  snprintf (s, sizeof (s), "<b>%d</b>", data->ratiospot);
	  DATA_NEW (s);
	  i++;

	  LABEL_NEW (_("Random Item:"));

	  snprintf (s, sizeof (s), "<b>%s</b>",
		    data->randomitem ? "true" : "false");
	  DATA_NEW (s);
	  i++;

	  LABEL_NEW (_("Random Spot:"));

	  snprintf (s, sizeof (s), "<b>%s</b>",
		    data->randomspot ? "true" : "false");
	  DATA_NEW (s);
	  i++;

	  LABEL_NEW (_("SoftStop:"));

	  snprintf (s, sizeof (s), "<b>%s</b>",
		    data->softstop ? "true" : "false");
	  DATA_NEW (s);
	  i++;

	  break;

	}

      label = gtk_label_new (data->description);
      gtk_widget_show (label);

      page = gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

      if (data->priority)
	gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page);

      list = list->next;
    }

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

/* EOF */
