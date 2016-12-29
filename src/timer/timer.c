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
#include "../commons/save.h"
#include "timer.h"

static time_t timer_current = 0;
static time_t timer_old = 0;
static int64_t timer_end = 0;
static time_t timer_diff = 0;
static gint timer_id = 0;

static struct somax_config_timer *config_timer = NULL;

static gboolean timer_warning (GtkWidget * timer);

static void timer_update_real (GtkWidget *, time_t current, int64_t end);
static void timer_resize (GtkWidget *);
static gboolean timer_destroy (GtkWidget *, GdkEvent *, gpointer);
static gboolean timer_configure (GtkWidget *, GdkEventButton * event,
				 gpointer);

GtkWidget *
timer_new (void)
{
  GtkWidget *pb;
  GtkWidget *box;
  GtkWidget *timer;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *event;
  PangoFontDescription *pfd;

  if (!config_timer)
    config_timer = get_timer ();

  pfd = pango_font_description_new ();
  pango_font_description_set_family (pfd,
				     config_timer->font ? config_timer->
				     font : "Sans");

  timer = gtk_vbox_new (0, 0);

  box = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (timer), box, TRUE, TRUE, 0);

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  event = gtk_event_box_new ();
  gtk_widget_show (event);
  gtk_container_add (GTK_CONTAINER (frame), event);

  gtk_widget_set_events (event, GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);

  g_signal_connect (G_OBJECT (event), "button-press-event",
		    G_CALLBACK (timer_configure), NULL);

  if (config_timer->decimal == TRUE)
    label = gtk_label_new ("--:--:--.- / --:--:--.-");
  else
    label = gtk_label_new ("--:--:-- / --:--:--");

  gtk_widget_show (label);
  gtk_container_add (GTK_CONTAINER (event), label);
  gtk_widget_modify_font (label, pfd);

  g_object_set_data ((gpointer) timer, "label", label);

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  event = gtk_event_box_new ();
  gtk_widget_show (event);
  gtk_container_add (GTK_CONTAINER (frame), event);
  g_object_set_data ((gpointer) timer, "cd_event", event);

  gtk_widget_set_events (event, GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);

  g_signal_connect (G_OBJECT (event), "button-press-event",
		    G_CALLBACK (timer_configure), NULL);

  if (config_timer->decimal == TRUE)
    label = gtk_label_new ("--:--:--.-");
  else
    label = gtk_label_new ("--:--:--");

  gtk_widget_show (label);
  gtk_container_add (GTK_CONTAINER (event), label);
  gtk_widget_modify_font (label, pfd);

  g_object_set_data ((gpointer) timer, "cd_label", label);

  pb = gtk_progress_bar_new ();
  gtk_widget_show (pb);
  gtk_box_pack_start (GTK_BOX (timer), pb, FALSE, FALSE, 0);
  g_object_set_data ((gpointer) timer, "pb", pb);

  g_object_set_data ((gpointer) timer, "pfd", pfd);
  g_signal_connect (timer, "destroy-event", G_CALLBACK (timer_destroy), NULL);

  return timer;
}

void
timer_set (GtkWidget * timer, time_t current, int64_t end)
{
  timer_update_real (timer, current, end);

  timer_old = time (NULL);
  timer_current = current;
  timer_end = end;
}

void
timer_update (GtkWidget * timer)
{
  time_t current;

  if (!timer_old)
    return;

  current = timer_current + (time (NULL) - timer_old);
  timer_update_real (timer, current, timer_end);
}

static void
timer_update_real (GtkWidget * timer, time_t current, int64_t end)
{
  GtkWidget *label;
  GtkWidget *pb;
  char s[1024];
  int c_secs, c_mins, c_hours;
  int hours, mins, secs, us;
  time_t diff;
  static time_t current_old = 0;
  static int64_t end_old = 0;

  if (current == current_old && end == end_old)
    return;

  if (current < 0)
    current = 0;

  current_old = current;
  end_old = end;

  if (!(label = g_object_get_data ((gpointer) timer, "label")))
    return;

  c_secs = current;
  c_mins = c_secs / 60;
  c_secs %= 60;
  c_hours = c_mins / 60;
  c_mins %= 60;

  if (end)
    {
      secs = end / SOMA_TIME_BASE;
      us = end % SOMA_TIME_BASE;
      mins = secs / 60;
      secs %= 60;
      hours = mins / 60;
      mins %= 60;

      if (config_timer->decimal == TRUE)
	snprintf (s, sizeof (s), "%02d:%02d:%02d.0 / %02d:%02d:%02d.%01d",
		  c_hours, c_mins, c_secs, hours, mins, secs,
		  (10 * us) / SOMA_TIME_BASE);
      else
	snprintf (s, sizeof (s), "%02d:%02d:%02d / %02d:%02d:%02d",
		  c_hours, c_mins, c_secs, hours, mins, secs);
    }

  else
    {
      if (config_timer->decimal == TRUE)
	snprintf (s, sizeof (s), "%02d:%02d:%02d.0 / --:--:--.-", c_hours,
		  c_mins, c_secs);
      else
	snprintf (s, sizeof (s), "%02d:%02d:%02d / --:--:--", c_hours, c_mins,
		  c_secs);
    }

  gtk_label_set_text (GTK_LABEL (label), s);

  if (!(label = g_object_get_data ((gpointer) timer, "cd_label")))
    return;

  if (end > 0)
    {
      diff = (end / SOMA_TIME_BASE) - current;

      c_secs = diff;
      c_mins = c_secs / 60;
      c_secs %= 60;
      c_hours = c_mins / 60;
      c_mins %= 60;

      if (config_timer->decimal == TRUE)
	snprintf (s, sizeof (s), "%02d:%02d:%02d.0", c_hours, c_mins, c_secs);
      else
	snprintf (s, sizeof (s), "%02d:%02d:%02d", c_hours, c_mins, c_secs);

      gtk_label_set_text (GTK_LABEL (label), s);

      timer_diff = diff;

      if (diff <=
	  (config_timer->first_seconds ? config_timer->first_seconds : 20)
	  && !timer_id)
	timer_id = g_timeout_add (500, (GSourceFunc) timer_warning, timer);
    }

  else
    {
      if (config_timer->decimal == TRUE)
	gtk_label_set_text (GTK_LABEL (label), "--:--:--.-");
      else
	gtk_label_set_text (GTK_LABEL (label), "--:--:--");
    }

  if ((pb = g_object_get_data ((gpointer) timer, "pb")))
    {

      if (end > 0 && current > 0)
	{
	  gchar buf[1024];
	  gdouble fraction =
	    ((gdouble) current) / ((gdouble) (end / SOMA_TIME_BASE));
	  gint percent = current * 100 / (end / SOMA_TIME_BASE);

	  if (fraction >= 0 && fraction <= 1)
	    {
	      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (pb), fraction);

	      snprintf (buf, sizeof (buf), "%3d%%", percent);
	      gtk_progress_bar_set_text (GTK_PROGRESS_BAR (pb), buf);
	    }
	  else
	    {
	      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (pb), 0);
	      gtk_progress_bar_set_text (GTK_PROGRESS_BAR (pb), "");
	    }
	}
    }

  timer_resize (timer);
}

void
timer_clear (GtkWidget * timer)
{
  GtkWidget *label;
  GtkWidget *pb;

  if (!(label = g_object_get_data ((gpointer) timer, "label")))
    return;

  if (config_timer->decimal == TRUE)
    gtk_label_set_text (GTK_LABEL (label), "--:--:--.- / --:--:--.-");
  else
    gtk_label_set_text (GTK_LABEL (label), "--:--:-- / --:--:--");

  if (!(label = g_object_get_data ((gpointer) timer, "cd_label")))
    return;

  if (config_timer->decimal == TRUE)
    gtk_label_set_text (GTK_LABEL (label), "--:--:--.-");
  else
    gtk_label_set_text (GTK_LABEL (label), "--:--:--");

  if (!(pb = g_object_get_data ((gpointer) timer, "pb")))
    return;

  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (pb), 0);
}

static gboolean
timer_warning (GtkWidget * timer)
{
  GtkWidget *event = g_object_get_data ((gpointer) timer, "cd_event");
  GtkStyle *style;
  static gboolean i = FALSE;

  if (!event)
    {
      timer_id = 0;
      return FALSE;
    }

  if (timer_diff <= 0
      || timer_diff >=
      (config_timer->first_seconds ? config_timer->first_seconds : 20))
    {
      style = gtk_style_copy (gtk_widget_get_default_style ());
      gtk_widget_set_style (event, style);
      g_object_unref (style);
      timer_id = 0;
      return FALSE;
    }

  style = gtk_style_copy (gtk_widget_get_default_style ());

  if (i == FALSE)
    {
      GdkColor color;

      if (timer_diff <=
	  (config_timer->third_seconds ? config_timer->third_seconds : 5))
	{
	  if (config_timer->third_color)
	    memcpy (&color, config_timer->third_color, sizeof (GdkColor));
	  else
	    gdk_color_parse ("red", &color);
	}

      else if (timer_diff <=
	       (config_timer->second_seconds ? config_timer->
		second_seconds : 10))
	{
	  if (config_timer->second_color)
	    memcpy (&color, config_timer->second_color, sizeof (GdkColor));
	  else
	    gdk_color_parse ("yellow", &color);
	}

      else
	{
	  if (config_timer->first_color)
	    memcpy (&color, config_timer->first_color, sizeof (GdkColor));
	  else
	    gdk_color_parse ("green", &color);
	}

      style->bg[GTK_STATE_NORMAL] = color;
    }

  gtk_widget_set_style (event, style);
  g_object_unref (style);
  i = !i;

  return TRUE;
}

static gint
timer_resize_label (GtkWidget *timer, GtkWidget * widget, PangoFontDescription * pfd)
{
  PangoRectangle logical;
  PangoLayout *layout;
  gint h, w;
  gint size;

  if(GTK_WIDGET_VISIBLE(timer)==FALSE)
    return pango_font_description_get_size (pfd);

  layout = gtk_widget_create_pango_layout (widget, NULL);
  pango_layout_set_text (layout, gtk_label_get_text (GTK_LABEL (widget)), -1);

  h = widget->allocation.height * PANGO_SCALE;
  w = widget->allocation.width * PANGO_SCALE;

  pfd = pango_font_description_copy (pfd);
  size = pango_font_description_get_size (pfd);
  pango_layout_set_font_description (layout, pfd);

  while (1)
    {
      pango_layout_get_extents (layout, NULL, &logical);
      if (logical.height > h || logical.width > w)
	break;

      size += PANGO_SCALE;
      pango_font_description_set_size (pfd, size);
      pango_layout_set_font_description (layout, pfd);
    }

  while (1)
    {
      pango_layout_get_extents (layout, NULL, &logical);
      if (logical.height < h && logical.width < w)
	break;

      size -= PANGO_SCALE;
      pango_font_description_set_size (pfd, size);
      pango_layout_set_font_description (layout, pfd);
    }

  pango_layout_get_extents (layout, NULL, &logical);
  pango_font_description_free (pfd);
  g_object_unref (layout);
  return size;
}

static void
timer_resize (GtkWidget * timer)
{
  gint p_height, p_width;
  PangoFontDescription *pfd;
  gint s1, s2;
  gboolean go = FALSE;
  GtkWidget *label, *cd_label;

  pfd = g_object_get_data ((gpointer) timer, "pfd");
  p_height = (gint) g_object_get_data ((gpointer) timer, "height");
  p_width = (gint) g_object_get_data ((gpointer) timer, "width");

  /* Le dimensioni sono cambiate? */
  if (p_height != timer->allocation.height
      || p_width != timer->allocation.width)
    go = TRUE;

  /* E' cambiato il font? */
  if (config_timer->font
      && strcmp (pango_font_description_get_family (pfd), config_timer->font))
    {
      pango_font_description_set_family (pfd, config_timer->font);
      go = TRUE;
    }

  /* Devo procedere? */
  if (go == FALSE)
    return;

  /* Memorizzo le nuove: */
  g_object_set_data ((gpointer) timer, "height",
		     (gpointer) timer->allocation.height);
  g_object_set_data ((gpointer) timer, "width",
		     (gpointer) timer->allocation.width);

  label = g_object_get_data ((gpointer) timer, "label");
  cd_label = g_object_get_data ((gpointer) timer, "cd_label");

  /* Quando e' grande il font giusto: */
  s1 = timer_resize_label (timer, label, pfd);
  s2 = timer_resize_label (timer, cd_label, pfd);

  if (s1 > s2)
    s1 = s2;

  if (pango_font_description_get_size (pfd) != s1)
    {
      pango_font_description_set_size (pfd, s1);

      gtk_widget_modify_font (label, pfd);
      gtk_widget_modify_font (cd_label, pfd);
    }
}

static gboolean
timer_destroy (GtkWidget * widget, GdkEvent * event, gpointer dummy)
{
  PangoFontDescription *pfd;

  if ((pfd = g_object_get_data ((gpointer) widget, "pfd")))
    {
      pango_font_description_free (pfd);
      g_object_steal_data ((gpointer) widget, "pfd");
    }

  free_timer (config_timer);
  config_timer = NULL;
  return FALSE;
}

static gboolean
timer_configure (GtkWidget * widget, GdkEventButton * event, gpointer dummy)
{
  GtkWidget *window;
  GtkWidget *preferences;
  GtkWidget *button;
  gchar s[1024];

  g_snprintf (s, sizeof (s), "%s %s - %s", PACKAGE, VERSION,
	      _("Timer Preferences"));

  window = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (window), s);
  gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_widget_set_size_request (window, 300, -1);

  preferences = timer_preferences ();
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), preferences, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (window), button, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  button = gtk_button_new_from_stock ("gtk-cancel");
  gtk_dialog_add_action_widget (GTK_DIALOG (window), button,
				GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (window);

  while (gtk_dialog_run (GTK_DIALOG (window)) == GTK_RESPONSE_OK)
    if (timer_preferences_check (preferences) == TRUE)
      break;

  gtk_widget_destroy (window);
  return TRUE;
}

GtkWidget *
timer_preferences (void)
{
  GtkWidget *box, *frame;
  GtkWidget *label, *table, *font;
  GtkWidget *first_color, *first_seconds;
  GtkWidget *second_color, *second_seconds;
  GtkWidget *third_color, *third_seconds;
  GtkWidget *decimal;
  gchar s[1024];

  box = gtk_vbox_new (FALSE, 0);

  frame = gtk_frame_new (_("Fonts"));
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);

  font = gtk_font_button_new ();
  gtk_font_button_set_show_size (GTK_FONT_BUTTON (font), FALSE);
  gtk_font_button_set_use_size (GTK_FONT_BUTTON (font), FALSE);
  gtk_container_add (GTK_CONTAINER (frame), font);
  snprintf (s, sizeof (s), "%s 20",
	    config_timer->font ? config_timer->font : "Sans");
  gtk_font_button_set_font_name (GTK_FONT_BUTTON (font), s);

  frame = gtk_frame_new (_("First Timer"));
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);

  table = gtk_table_new (2, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new (_("Color:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

  first_color = gtk_color_button_new ();
  if (config_timer->first_color)
    gtk_color_button_set_color (GTK_COLOR_BUTTON (first_color),
				config_timer->first_color);
  else
    {
      GdkColor color;
      gdk_color_parse ("green", &color);
      gtk_color_button_set_color (GTK_COLOR_BUTTON (first_color), &color);
    }

  gtk_table_attach_defaults (GTK_TABLE (table), first_color, 1, 2, 0, 1);

  label = gtk_label_new (_("Seconds:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

  first_seconds = gtk_spin_button_new_with_range (0, 999, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (first_seconds),
			     config_timer->first_seconds ? config_timer->
			     first_seconds : 20);
  gtk_table_attach_defaults (GTK_TABLE (table), first_seconds, 1, 2, 1, 2);

  frame = gtk_frame_new (_("Second Timer"));
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);

  table = gtk_table_new (2, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new (_("Color:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

  second_color = gtk_color_button_new ();
  if (config_timer->second_color)
    gtk_color_button_set_color (GTK_COLOR_BUTTON (second_color),
				config_timer->second_color);
  else
    {
      GdkColor color;
      gdk_color_parse ("yellow", &color);
      gtk_color_button_set_color (GTK_COLOR_BUTTON (second_color), &color);
    }

  gtk_table_attach_defaults (GTK_TABLE (table), second_color, 1, 2, 0, 1);

  label = gtk_label_new (_("Seconds:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

  second_seconds = gtk_spin_button_new_with_range (0, 999, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (second_seconds),
			     config_timer->second_seconds ? config_timer->
			     second_seconds : 10);
  gtk_table_attach_defaults (GTK_TABLE (table), second_seconds, 1, 2, 1, 2);

  frame = gtk_frame_new (_("Third Timer"));
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);

  table = gtk_table_new (2, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new (_("Color:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

  third_color = gtk_color_button_new ();
  if (config_timer->third_color)
    gtk_color_button_set_color (GTK_COLOR_BUTTON (third_color),
				config_timer->third_color);
  else
    {
      GdkColor color;
      gdk_color_parse ("red", &color);
      gtk_color_button_set_color (GTK_COLOR_BUTTON (third_color), &color);
    }

  gtk_table_attach_defaults (GTK_TABLE (table), third_color, 1, 2, 0, 1);

  label = gtk_label_new (_("Seconds:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

  third_seconds = gtk_spin_button_new_with_range (0, 999, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (third_seconds),
			     config_timer->third_seconds ? config_timer->
			     third_seconds : 5);
  gtk_table_attach_defaults (GTK_TABLE (table), third_seconds, 1, 2, 1, 2);

  frame = gtk_frame_new (_("Show Decimals Value"));
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);

  decimal = gtk_check_button_new_with_label (_("Show Decimals Value"));
  gtk_container_add (GTK_CONTAINER (frame), decimal);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (decimal),
				config_timer->decimal);

  g_object_set_data(G_OBJECT(box), "first_seconds", first_seconds);
  g_object_set_data(G_OBJECT(box), "second_seconds", second_seconds);
  g_object_set_data(G_OBJECT(box), "third_seconds", third_seconds);
  g_object_set_data(G_OBJECT(box), "first_color", first_color);
  g_object_set_data(G_OBJECT(box), "second_color", second_color);
  g_object_set_data(G_OBJECT(box), "third_color", third_color);
  g_object_set_data(G_OBJECT(box), "font", font);
  g_object_set_data(G_OBJECT(box), "decimal", decimal);

  return box;
}

gboolean
timer_preferences_check (GtkWidget * box)
{
  gint f_sec, s_sec, t_sec;
  GtkWidget *first_seconds, *second_seconds, *third_seconds;
  GtkWidget *first_color, *second_color, *third_color;
  GtkWidget *font;
  GtkWidget *decimal;
  gchar *f;

  first_seconds = g_object_get_data(G_OBJECT(box), "first_seconds");
  second_seconds = g_object_get_data(G_OBJECT(box), "second_seconds");
  third_seconds = g_object_get_data(G_OBJECT(box), "third_seconds");
  first_color = g_object_get_data(G_OBJECT(box), "first_color");
  second_color = g_object_get_data(G_OBJECT(box), "second_color");
  third_color = g_object_get_data(G_OBJECT(box), "third_color");
  font = g_object_get_data(G_OBJECT(box), "font");
  decimal = g_object_get_data(G_OBJECT(box), "decimal");

  f_sec = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (first_seconds));
  s_sec = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (second_seconds));
  t_sec = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (third_seconds));

  if (s_sec >= f_sec)
    {
      dialog_msg (_("The first timer can't be minor then second one."));
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (first_seconds), s_sec + 1);
      return FALSE;
    }

  if (t_sec >= s_sec)
    {
      dialog_msg (_("The second timer can't be minor then third one."));
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (second_seconds), t_sec + 1);
      return FALSE;
    }

  config_timer->first_seconds = f_sec;
  config_timer->second_seconds = s_sec;
  config_timer->third_seconds = t_sec;

  if (!config_timer->first_color)
    config_timer->first_color = g_malloc (sizeof (GdkColor));
  if (!config_timer->second_color)
    config_timer->second_color = g_malloc (sizeof (GdkColor));
  if (!config_timer->third_color)
    config_timer->third_color = g_malloc (sizeof (GdkColor));

  gtk_color_button_get_color (GTK_COLOR_BUTTON (first_color),
			      config_timer->first_color);
  gtk_color_button_get_color (GTK_COLOR_BUTTON (second_color),
			      config_timer->second_color);
  gtk_color_button_get_color (GTK_COLOR_BUTTON (third_color),
			      config_timer->third_color);

  if ((f = (gchar *) gtk_font_button_get_font_name (GTK_FONT_BUTTON (font))))
    {
      gchar *font = g_strdup (f);
      int len = strlen (font);

      while (--len)
	if (font[len] == ' ')
	  break;

      if (len)
	font[len] = 0;

      if (config_timer->font)
	g_free (config_timer->font);

      config_timer->font = g_strdup (font);
      g_free (font);
    }

  config_timer->decimal =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (decimal));

  set_timer (config_timer);

  return TRUE;
}

/* EOF */
