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
#include "../draw/draw.h"

static GtkWidget *tooltip_win = NULL;
static int t_show = 0;

static GtkWidget *tooltip_l_time = NULL;
static GtkWidget *tooltip_s_time = NULL;

static gint tooltip_expose (gpointer dummy);
static void tooltip_draw_time (void);
static void tooltip_draw (struct somad_data *d);

/* callback for the tooltip showed */
static gint
tooltip_expose (gpointer dummy)
{
  GtkRequisition req;

  if (!tooltip_win)
    return TRUE;

  gtk_widget_size_request (tooltip_win, &req);
  gtk_paint_flat_box (tooltip_win->style, tooltip_win->window,
		      GTK_STATE_NORMAL, GTK_SHADOW_OUT,
		      NULL, GTK_WIDGET (tooltip_win), "tooltip",
		      0, 0, req.width, req.height);

  return FALSE;
}

/* This function update the timer */
void
tooltip_time_refresh (void)
{
  char *str;
  time_t tt;
  struct tm *k;
  int len, i;
  char s[1024];
  static char *previous = NULL;

  if (tooltip_s_time)
    {
      if ((k = get_time ()))
	{
	  str = asctime (k);
	  len = snprintf (s, sizeof (s), _("Somad Time: %s"), str);
	  for (i = 0; i < len; i++)
	    {
	      if (s[i] == '\n')
		{
		  s[i] = 0;
		  break;
		}
	    }

	  if (!previous || strcmp (previous, s))
	    {
	      gtk_label_set_text (GTK_LABEL (tooltip_s_time), s);
	      gtk_label_set_use_markup (GTK_LABEL (tooltip_s_time), TRUE);
	      gtk_misc_set_alignment (GTK_MISC (tooltip_s_time), 0.0, 0.5);

	      if (previous)
		g_free (previous);

	      previous = g_strdup (s);
	    }
	}
    }

  if (tooltip_l_time)
    {
      tt = time (NULL);
      if ((k = localtime (&tt)))
	{
	  str = asctime (k);
	  len = snprintf (s, sizeof (s), _("Local Time: %s"), str);
	  for (i = 0; i < len; i++)
	    {
	      if (s[i] == '\n')
		{
		  s[i] = 0;
		  break;
		}
	    }

	  if (!previous || strcmp (previous, s))
	    {
	      gtk_label_set_text (GTK_LABEL (tooltip_l_time), s);
	      gtk_label_set_use_markup (GTK_LABEL (tooltip_l_time), TRUE);
	      gtk_misc_set_alignment (GTK_MISC (tooltip_l_time), 0.0, 0.5);

	      if (previous)
		g_free (previous);

	      previous = g_strdup (s);
	    }
	}
    }
}

/* This function draws the time tooltip */
static void
tooltip_draw_time (void)
{
  GtkRequisition req;
  GtkWidget *box;
  GdkScreen *screen;
  GdkScreen *p_screen;
  int x, y, px, py, monitor_n;
  GdkRectangle monitor;

  t_show = 1;

  tooltip_win = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_widget_set_app_paintable (tooltip_win, TRUE);
  gtk_window_set_resizable (GTK_WINDOW (tooltip_win), FALSE);
  gtk_widget_set_name (tooltip_win, "somax-tooltips");
  gtk_container_set_border_width (GTK_CONTAINER (tooltip_win), 4);

  g_signal_connect (tooltip_win, "delete-event",
		    G_CALLBACK (tooltip_hide), NULL);

  g_signal_connect (tooltip_win, "expose_event",
		    G_CALLBACK (tooltip_expose), NULL);

  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (tooltip_win), box);

  tooltip_s_time = gtk_label_new ("");
  gtk_widget_show (tooltip_s_time);
  gtk_box_pack_start (GTK_BOX (box), tooltip_s_time, FALSE, FALSE, 0);

  tooltip_l_time = gtk_label_new ("");
  gtk_widget_show (tooltip_l_time);
  gtk_box_pack_start (GTK_BOX (box), tooltip_l_time, FALSE, FALSE, 0);

  tooltip_time_refresh ();

  screen = gtk_widget_get_screen (pl_draw);

  gdk_display_get_pointer (gdk_screen_get_display (screen),
			   &p_screen, &x, &y, NULL);

  gdk_window_get_origin (pl_draw->window, &px, &py);

  gtk_widget_size_request (tooltip_win, &req);

  screen = gtk_widget_get_screen (pl_draw);
  monitor_n = gdk_screen_get_monitor_at_point (screen, px, py);
  gdk_screen_get_monitor_geometry (screen, monitor_n, &monitor);

  if ((x + req.width) > monitor.x + monitor.width)
    x -= (x + req.width) - (monitor.x + monitor.width);
  else if (x < monitor.x)
    x = monitor.x;

  if ((y + req.height) > monitor.x + monitor.height)
    y -= (y + req.height) - (monitor.y + monitor.height);
  else if (y < monitor.y)
    y = monitor.y;

  gtk_window_move (GTK_WINDOW (tooltip_win), x, y);
  gtk_widget_show (tooltip_win);
}

/* This function draws the tooltip with the description of the transmission */
static void
tooltip_draw (struct somad_data *d)
{
  GtkRequisition req;
  GtkWidget *box;
  GtkWidget *label;
  GdkScreen *screen;
  GdkScreen *p_screen;
  int x, y, px, py, monitor_n;
  GdkRectangle monitor;
  char s[1024];
  gchar *tmp;

  t_show = 1;

  tooltip_win = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_widget_set_app_paintable (tooltip_win, TRUE);
  gtk_window_set_resizable (GTK_WINDOW (tooltip_win), FALSE);
  gtk_widget_set_name (tooltip_win, "somax-tooltips");
  gtk_container_set_border_width (GTK_CONTAINER (tooltip_win), 4);

  g_signal_connect (tooltip_win, "delete-event",
		    G_CALLBACK (tooltip_hide), NULL);

  g_signal_connect (tooltip_win, "expose_event",
		    G_CALLBACK (tooltip_expose), NULL);

  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (tooltip_win), box);

  tmp = somax_markup (d->description);
  snprintf (s, sizeof (s), _("Description: <b>%s</b>"), tmp);
  g_free (tmp);

  label = gtk_label_new (s);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  tmp = somax_markup (d->timer->start);
  snprintf (s, sizeof (s), _("Start: <b>%s</b>"), tmp);
  g_free (tmp);

  label = gtk_label_new (s);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  tmp = somax_markup (d->timer->stop);
  snprintf (s, sizeof (s), _("Stop: <b>%s</b>"), tmp);
  g_free (tmp);

  label = gtk_label_new (s);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  screen = gtk_widget_get_screen (pl_draw);

  gdk_display_get_pointer (gdk_screen_get_display (screen),
			   &p_screen, &x, &y, NULL);

  gdk_window_get_origin (pl_draw->window, &px, &py);

  gtk_widget_size_request (tooltip_win, &req);

  screen = gtk_widget_get_screen (pl_draw);
  monitor_n = gdk_screen_get_monitor_at_point (screen, px, py);
  gdk_screen_get_monitor_geometry (screen, monitor_n, &monitor);

  if ((x + req.width) > monitor.x + monitor.width)
    x -= (x + req.width) - (monitor.x + monitor.width);
  else if (x < monitor.x)
    x = monitor.x;

  if ((y + req.height) > monitor.x + monitor.height)
    y -= (y + req.height) - (monitor.y + monitor.height);
  else if (y < monitor.y)
    y = monitor.y;

  gtk_window_move (GTK_WINDOW (tooltip_win), x, y);
  gtk_widget_show (tooltip_win);
}

/* Hide the tooltip */
void
tooltip_hide (void)
{
  if (tooltip_win)
    {
      gtk_widget_hide (tooltip_win);
      gtk_widget_destroy (tooltip_win);
      tooltip_win = NULL;
    }

  tooltip_s_time = tooltip_l_time = NULL;

  t_show = 0;
}

/* Timer for the tooltip system */
int
tooltip_timer (gpointer dummy)
{
  int x, y;
  static int x_old = -1, y_old = -1;
  struct somad_data *d = NULL;
  GList *list = NULL;
  static struct somad_data *d_old = NULL;
  static int time_show = 0;

  int time_x = draw_get_time_x (pl_draw);
  int time_y = draw_get_time_y (pl_draw);

  gtk_widget_get_pointer (pl_draw, &x, &y);

  if (x < 0 || y < 0
      || gtk_window_is_active (GTK_WINDOW (gtk_widget_get_toplevel (pl_draw)))
      == FALSE)
    {
      d_old = NULL;
      x_old = -1;
      y_old = -1;
      tooltip_hide ();
      return TRUE;
    }

  if (x > time_x && x < time_x + TIME_SIZE && y > time_y
      && y < time_y + TIME_SIZE)
    {
      if (x == x_old && y == y_old)
	{
	  if (!time_show)
	    {
	      tooltip_hide ();
	      tooltip_draw_time ();
	      time_show++;
	    }

	}
      else
	{
	  time_show = 0;
	  tooltip_hide ();
	}

    }
  else if (y < time_y - 1)
    {

      list = draw_get_xy (pl_draw, x, y, somad_pl);

      if (x == x_old && y == y_old)
	{
	  if (list)
	    d = list->data;
	  else
	    d = NULL;

	  if (d && d != d_old)
	    {
	      tooltip_hide ();
	      tooltip_draw (d);
	    }
	}

      else if (d != d_old)
	{
	  tooltip_hide ();
	  d = NULL;
	}

      if (list)
	g_list_free (list);
    }

  x_old = x;
  y_old = y;
  d_old = d;

  return TRUE;
}

/* EOF */
