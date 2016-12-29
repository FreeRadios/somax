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
#include "../draw/draw.h"
#include "../timer/timer.h"

int status_running = -1;
int status_pause = -1;
int status_stop = -1;
time_t somad_timestep;

#define MAX_PB 14.0
#define MAX_LOOP 5

static void timeout_progress_bar (gdouble);
static void timeout_statusbar_set (char *str);
static void timeout_running (int running);
static void timeout_stop (int stop);
static void timeout_pause (int pause);
static int timeout_palinsesto (char *pl);
static void timeout_spot (char *spot);
static void timeout_next (void);
static void timeout_palinsesto_name (char *str);

static int try_reconnect (int);

struct thread_t
{
  GMutex *mutex;

  gint refresh;
  gint running;
  gint pause;
  gint stop;
  gboolean info;
  gboolean force_info;

  char **nextitem;

  int nextitem_spot;
  char *status;
  char *palinsesto;
  char *pl;
  char *spot;

  gdouble bar;
  time_t time;
  time_t time_play;

  char **item_list;
  char **spot_list;

  char *item;
  gboolean item_update;

  char *item_next;
  gboolean item_next_update;
};

static struct thread_t thread;

void
timeout_init (void)
{
  memset (&thread, 0, sizeof (thread));
  thread.mutex = g_mutex_new ();
}

int
timeout (gpointer dummy)
{
  int ret;
  static int draw_timer = 0;
  static int repeat = 0;
  static int old_info = -1;
  char *tmp;

  thread.refresh = preferences_data->request_infos;

  thread.info = preferences_data->info_download;

  if (preferences_data->window_size && main_window)
    {
      if (preferences_data->window_width != main_window->allocation.width ||
	  preferences_data->window_height != main_window->allocation.height)
	{
	  preferences_data->window_width = main_window->allocation.width;
	  preferences_data->window_height = main_window->allocation.height;
	  set_preferences (preferences_data);
	}
    }

  if (thread.info != old_info)
    {
      thread.force_info = thread.info;
      old_info = thread.info;
    }

  if (repeat == thread.refresh * 2)
    {
      repeat = 0;

      if ((ret = soma_error (controller)) != SOMA_ERR_OK)
	{
	  if (try_reconnect (ret))
	    {
	      quit (NULL, NULL);
	      return FALSE;
	    }

	  return TRUE;
	}
    }
  else
    repeat++;

  if (g_mutex_trylock (thread.mutex) == TRUE)
    {
      /* PROGRESS_BAR */
      timeout_progress_bar (thread.bar ? thread.bar / MAX_PB : 0);

      /* STATUS */
      if (thread.status)
	{
	  tmp = somax_to_utf8 (thread.status);
	  timeout_statusbar_set (tmp);
	  g_free (tmp);

	  free (thread.status);
	  thread.status = NULL;
	}

      /* RUNNING */
      timeout_running (thread.running);

      /* PAUSE */
      timeout_pause (thread.pause);

      /* STOP */
      timeout_stop (thread.stop);

      /* TIME */
      somad_timestep = thread.time;

      time_refresh ();
      tooltip_time_refresh ();

      /* NEXTITEM */
      if (win_nextitem_window_showed ())
	nextitem_timer_refresh (thread.nextitem, thread.nextitem_spot);

      /* GET_PALINSESTO */
      if (timeout_palinsesto (thread.pl))
	draw_timer = 1;

      /* GET_SPOT */
      timeout_spot (thread.spot);

      /* NEXT */
      timeout_next ();

      if (draw_timer == 30)
	draw_timer = 0;

      if (!draw_timer)
	draw_refresh (pl_draw, somad_pl, TRUE, 0, TRUE);

      draw_timer++;

      /* ITEM_LIST */
      if (thread.item_list)
	{
	  tlist_new (thread.item_list, TRUE);

	  if (thread.item_list)
	    soma_item_list_free (thread.item_list);
	  thread.item_list = NULL;
	}

      tlist_refresh (TRUE);

      /* SPOT_LIST */
      if (thread.spot_list)
	{
	  tlist_new (thread.spot_list, FALSE);

	  if (thread.spot_list)
	    soma_spot_list_free (thread.spot_list);
	  thread.spot_list = NULL;
	}

      tlist_refresh (FALSE);

      /* TIME_PLAY */
      if (!thread.item)
	tmp = NULL;
      else
	tmp = somax_to_utf8 (thread.item);

      if (thread.running)
	{
	  if (thread.time_play && tmp)
	    time_update_widget (thread.time_play, tmp);

	  else
	    timer_update (timer_widget);
	}
      else
	timer_clear (timer_widget);

      /* ITEM */
      if (thread.item_update == FALSE)
	{
	  button_current_item (tmp);
	  free (thread.item);
	  thread.item = NULL;
	  thread.item_update = TRUE;
	}

      if (thread.item_next_update == FALSE)
	{
	  button_next_item (thread.item_next);
	  free (thread.item_next);
	  thread.item_next = NULL;
	  thread.item_next_update = TRUE;
	}

      if (tmp)
	g_free (tmp);

      /* PALINSESTO_NAME */
      if (thread.palinsesto)
	{
	  tmp = somax_to_utf8 (thread.palinsesto);
	  timeout_palinsesto_name (tmp);
	  g_free (tmp);

	  free (thread.palinsesto);
	  thread.palinsesto = NULL;
	}

      g_mutex_unlock (thread.mutex);
    }

  return TRUE;
}

int
timeout_tooltip (gpointer dummy)
{
  int x, y;
  static int x_old = -1, y_old = -1;
  static int show = 0;

  gtk_widget_get_pointer (pl_draw, &x, &y);
  if (x < 0 || y < 0)
    return TRUE;

  if (x == x_old && y == y_old)
    {
      if (!show)
	show = 1;
    }

  else if (show)
    show = 0;

  x_old = x;
  y_old = y;

  return TRUE;
}

int
l_timeout_swap (int a, int b, int max)
{
  if (a == b)
    return 0;
  else if (a < b)
    return b - a;
  else
    return (max - a) + b;
}

static void
timeout_next (void)
{
  GList *list;

  if ((list = palinsesto_now (somad_pl)))
    {
      l_timeout_update ((struct somad_data *) list->data);
      g_list_free (list);
    }
}

void
l_timeout_update (struct somad_data *data)
{
  char buf[1024];
  struct tm *k;
  int i, a, b;
  int sec;

  if (!(k = get_time ()))
    return;

  if (!data)
    return;

  i = timer_check (data->timer, k);
  sec = (59 - k->tm_sec) + 1;

  if (i == 1)
    snprintf (buf, sizeof (buf), _("Next event: 0:00:%.2d"), sec);

  else if (i)
    {
      a = i / 60;
      b = i % 60;
      snprintf (buf, sizeof (buf), _("Next event: %d:%.2d:%.2d"), a, b, sec);
    }

  else
    {

      if (data->timer->stop_year != -1)
	k->tm_year =
	  l_timeout_swap (k->tm_year, data->timer->stop_year, 3333);
      else
	k->tm_year = 0;

      if (data->timer->stop_month != -1)
	k->tm_mon = l_timeout_swap (k->tm_mon, data->timer->stop_month, 12);
      else
	k->tm_mon = 0;

      if (data->timer->stop_mday != -1)
	k->tm_mday = l_timeout_swap (k->tm_mday, data->timer->stop_mday, 31);
      else
	k->tm_mday = 0;

      if (data->timer->stop_hour != -1)
	{
	  a = k->tm_hour * 60;
	  b = data->timer->stop_hour * 60;
	}

      else
	a = b = 0;

      if (data->timer->stop_min != -1)
	{
	  a += k->tm_min;
	  b += data->timer->stop_min;
	}

      a = l_timeout_swap (a, b, 1440);

      k->tm_hour = a / 60;
      k->tm_min = a % 60;

      i = k->tm_year * 518400;
      i += k->tm_mon * 43200;
      i += k->tm_mday * 1440;
      i += k->tm_hour * 60;
      i += k->tm_min;

      a = i / 60;
      b = i % 60;
      sec = (59 - k->tm_sec) + 1;

      if (i > 1)
	snprintf (buf, sizeof (buf), _("Next event: %d:%.2d:%.2d"), a, b,
		  sec);
      else
	snprintf (buf, sizeof (buf), _("Next event: 0:00:%.2d"), sec);
    }

  gtk_label_set_text (GTK_LABEL (l_timeout), buf);
}

static void
timeout_progress_bar (gdouble val)
{
  gchar s[10];
  static gdouble val_old = -1;

  if (set_quit == TRUE)
    return;

  if (val_old < 0 || val_old != val)
    {
      snprintf (s, sizeof (s), "%2.0f%%", val * 100);

      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (timer_pb), val);
      gtk_progress_bar_set_text (GTK_PROGRESS_BAR (timer_pb), s);

      while (gtk_events_pending ())
	gtk_main_iteration ();

      val_old = val;
    }
}

static void
timeout_statusbar_set (char *str)
{
  static char *str_old = NULL;

  if (!str_old || (str && strcmp (str_old, str)))
    {
      if (str_old)
	g_free (str_old);

      str_old = g_strdup (str);

      statusbar_set (str);
    }
}

static void
timeout_stop (int stop)
{
  if (stop != status_stop)
    {
      if (!stop)
	{
	  GtkWidget *box;
	  GtkWidget *image;
	  GtkWidget *label;

	  gtk_container_remove (GTK_CONTAINER (b_startstop), w_startstop);

	  box = gtk_hbox_new (FALSE, 0);
	  image = gtk_image_new_from_stock ("gtk-no", GTK_ICON_SIZE_BUTTON);
	  gtk_widget_show (image);
	  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

	  label = gtk_label_new_with_mnemonic (_("_Stop"));
	  gtk_widget_show (label);
	  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

	  gtk_container_add (GTK_CONTAINER (b_startstop), box);
	  w_startstop = box;
	  gtk_widget_show_all (b_startstop);
	}
      else
	{
	  GtkWidget *box;
	  GtkWidget *image;
	  GtkWidget *label;

	  gtk_container_remove (GTK_CONTAINER (b_startstop), w_startstop);

	  box = gtk_hbox_new (FALSE, 0);
	  image = gtk_image_new_from_stock ("gtk-yes", GTK_ICON_SIZE_BUTTON);
	  gtk_widget_show (image);
	  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

	  label = gtk_label_new_with_mnemonic (_("_Start"));
	  gtk_widget_show (label);
	  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

	  gtk_container_add (GTK_CONTAINER (b_startstop), box);
	  w_startstop = box;
	  gtk_widget_show_all (b_startstop);
	}

      status_stop = stop;
    }
}

static void
timeout_running (int running)
{
  if (running != status_running)
    {
      if (running)
	{
	  gtk_image_set_from_stock (GTK_IMAGE (image_running), "gtk-yes",
				    GTK_ICON_SIZE_SMALL_TOOLBAR);
	}
      else
	{
	  gtk_image_set_from_stock (GTK_IMAGE (image_running), "gtk-no",
				    GTK_ICON_SIZE_SMALL_TOOLBAR);

	  button_current_item (NULL);
	}

      status_running = running;
    }
}

static void
timeout_pause (int pause)
{
  if (pause != status_pause)
    {
      if (!pause)
	{
	  GtkWidget *box;
	  GtkWidget *image;
	  GtkWidget *label;

	  gtk_container_remove (GTK_CONTAINER (b_pause), w_pause);

	  box = gtk_hbox_new (FALSE, 0);
	  image = gtk_image_new_from_stock ("gtk-no", GTK_ICON_SIZE_BUTTON);
	  gtk_widget_show (image);
	  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

	  label = gtk_label_new_with_mnemonic (_("_Pause"));
	  gtk_widget_show (label);
	  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

	  gtk_container_add (GTK_CONTAINER (b_pause), box);
	  w_pause = box;
	  gtk_widget_show_all (b_pause);
	}
      else
	{
	  GtkWidget *box;
	  GtkWidget *image;
	  GtkWidget *label;

	  gtk_container_remove (GTK_CONTAINER (b_pause), w_pause);

	  box = gtk_hbox_new (FALSE, 0);
	  image = gtk_image_new_from_stock ("gtk-yes", GTK_ICON_SIZE_BUTTON);
	  gtk_widget_show (image);
	  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

	  label = gtk_label_new_with_mnemonic (_("_UnPause"));
	  gtk_widget_show (label);
	  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

	  gtk_container_add (GTK_CONTAINER (b_pause), box);
	  w_pause = box;
	  gtk_widget_show_all (b_pause);
	}

      status_pause = pause;
    }
}

static int
timeout_palinsesto (char *pl)
{
  static int parser_pl = 0;
  static char *pl_old = NULL;

  if ((!pl_old && pl) || (pl_old && pl && strcmp (pl_old, pl)))
    {

      if (pl_old)
	g_free (pl_old);

      if (pl)
	pl_old = g_strdup (pl);
      else
	pl_old = NULL;

      tooltip_hide ();

      if (somad_pl)
	{
	  palinsesto_free (somad_pl);
	  somad_pl = NULL;
	}

      if (palinsesto_parser (pl_old, &somad_pl))
	{
	  if (!parser_pl)
	    {
	      dialog_msg (_("Somad Palinsesto syntax error"));
	      parser_pl = 1;
	    }

	  g_free (pl_old);
	  pl_old = NULL;
	}
      else
	parser_pl = 0;

      list_refresh (NULL, NULL);
      draw_refresh (pl_draw, somad_pl, TRUE, 0, TRUE);
      return 1;
    }

  return 0;
}

static void
timeout_spot (char *spot)
{
  static int parser_spot = 0;
  static char *spot_old = NULL;

  if ((!spot_old && spot) || (spot_old && spot && strcmp (spot_old, spot)))
    {

      if (spot_old)
	g_free (spot_old);

      if (spot)
	spot_old = g_strdup (spot);
      else
	spot_old = NULL;

      if (somad_spot)
	{
	  spot_free (somad_spot);
	  somad_spot = NULL;
	}

      if (spot_parser (spot_old, &somad_spot))
	{
	  if (!parser_spot)
	    {
	      dialog_msg (_("Somad Spot file syntax error"));
	      parser_spot = 1;
	    }

	  g_free (spot_old);
	  spot_old = NULL;
	}

      else
	parser_spot = 0;
    }
}

static void
timeout_palinsesto_name (char *str)
{
  const gchar *str_old = NULL;

  if (palinsesto_name)
    str_old = gtk_button_get_label (GTK_BUTTON (palinsesto_name));

  if (!str_old || !str || strcmp (str_old, str))
    button_palinsesto_name (str);
}


gpointer
thread_start (gpointer dummy)
{
  char *str;
  char **list;
  int ret;
  time_t t;
  int changed = -1;
  static int loop = 0;

  signal (SIGPIPE, SIG_IGN);

  do
    {
      if (changed < 0)
	changed = 1;
      else
	changed = 0;

      /* STATUS */
      thread.bar = 0.0;

      str = soma_status (controller);

      g_mutex_lock (thread.mutex);

      if (thread.status)
	free (thread.status);

      thread.status = str;

      g_mutex_unlock (thread.mutex);

      /* PALINSESTO */
      thread.bar = 1.0;

      str = soma_palinsesto_name (controller);

      g_mutex_lock (thread.mutex);

      if (thread.palinsesto)
	free (thread.palinsesto);

      thread.palinsesto = str;

      g_mutex_unlock (thread.mutex);

      /* RUNNING */
      thread.bar = 2.0;

      ret = soma_running (controller);

      g_mutex_lock (thread.mutex);
      thread.running = ret;
      g_mutex_unlock (thread.mutex);

      /* TIME */
      thread.bar = 3.0;

      t = soma_time (controller);

      g_mutex_lock (thread.mutex);
      thread.time = t;
      g_mutex_unlock (thread.mutex);

      /* GET_PALINSESTO */
      thread.bar = 4.0;

      str = soma_get_palinsesto (controller);

      g_mutex_lock (thread.mutex);
      if (thread.pl)
	free (thread.pl);
      thread.pl = str;
      g_mutex_unlock (thread.mutex);

      /* GET_NEXTITEM */
      thread.bar = 5.0;

      list = soma_nextitem_list (controller);

      g_mutex_lock (thread.mutex);
      if (thread.nextitem)
	soma_nextitem_list_free (thread.nextitem);
      thread.nextitem = list;
      g_mutex_unlock (thread.mutex);

      /* GET_NEXTITEM_SPOT */
      thread.bar = 6.0;

      ret = soma_nextitem_get_spot (controller);

      g_mutex_lock (thread.mutex);
      thread.nextitem_spot = ret;
      g_mutex_unlock (thread.mutex);

      /* GET_SPOT */
      thread.bar = 7.0;

      str = soma_get_spot (controller);

      g_mutex_lock (thread.mutex);
      if (thread.spot)
	free (thread.spot);
      thread.spot = str;
      g_mutex_unlock (thread.mutex);

      /* GET_ITEM */
      thread.bar = 8.0;

      if ((str = soma_get_item (controller)))
	stat_new (controller, str);

      g_mutex_lock (thread.mutex);
      if (thread.item)
	free (thread.item);
      thread.item = str;
      thread.item_update = FALSE;
      g_mutex_unlock (thread.mutex);

      /* GET_FOLLOW_ITEM */
      thread.bar = 9.0;

      if ((str = soma_get_item_next (controller)))
	stat_new (controller, str);

      g_mutex_lock (thread.mutex);
      if (thread.item_next)
	free (thread.item_next);
      thread.item_next = str;
      thread.item_next_update = FALSE;
      g_mutex_unlock (thread.mutex);

      /* GET_TIME_PLAY */
      thread.bar = 10.0;

      t = soma_get_time_play (controller);

      g_mutex_lock (thread.mutex);
      thread.time_play = t;
      g_mutex_unlock (thread.mutex);

      /* STOP */
      thread.bar = 11.0;

      soma_get_stop (controller, &ret);

      g_mutex_lock (thread.mutex);
      thread.stop = ret;
      g_mutex_unlock (thread.mutex);

      /* PAUSE */
      thread.bar = 12.0;

      soma_get_pause (controller, &ret);

      g_mutex_lock (thread.mutex);
      thread.pause = ret;
      g_mutex_unlock (thread.mutex);

      if (!loop || changed)
	{
	  /* GET_ITEM_LIST */
	  thread.bar = 13.0;

	  list = soma_get_item_list (controller);

	  if (thread.info)
	    stat_check (controller, list);

	  g_mutex_lock (thread.mutex);
	  if (thread.item_list)
	    soma_item_list_free (thread.item_list);
	  thread.item_list = list;
	  g_mutex_unlock (thread.mutex);

	  /* GET_SPOT_LIST */
	  thread.bar = 14.0;

	  list = soma_get_spot_list (controller);

	  if (thread.info)
	    stat_check (controller, list);

	  g_mutex_lock (thread.mutex);
	  if (thread.spot_list)
	    soma_spot_list_free (thread.spot_list);
	  thread.spot_list = list;
	  g_mutex_unlock (thread.mutex);

	  loop = 0;
	}

      if (loop == MAX_LOOP)
	loop = 0;
      else
	loop++;

      /* SLEEP */
      sleep (thread.refresh);
    }
  while (!set_quit);

  signal (SIGPIPE, SIG_DFL);

  g_thread_exit (NULL);
  return NULL;
}

static void
try_reconnect_b (GtkWidget * b, int *c)
{
  *c = 1;
}

static int
try_reconnect (int error)
{
  GtkWidget *window;
  GtkWidget *pb;
  GtkWidget *sep;
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *label = NULL;

  int ret = 1;
  int cancel = 0;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("Reconnecting..."));
  gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

  box = gtk_vbox_new (FALSE, 8);
  gtk_container_add (GTK_CONTAINER (window), box);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  image = gtk_image_new_from_stock ("gtk-dialog-error", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  switch (error)
    {
    case SOMA_ERR_SSL:
      label = gtk_label_new (_("SSL error! Try to reconnecting..."));
      break;

    case SOMA_ERR_CONNECT:
      label = gtk_label_new (_("Connect error! Try to reconnecting..."));
      break;

    case SOMA_ERR_HOST:
      label = gtk_label_new (_("Unknown host! Try to reconnecting..."));
      break;

    case SOMA_ERR_PROTOCOL:
      label = gtk_label_new (_("Protocol error! Try to reconnecting..."));
      break;

    case SOMA_ERR_PASSWORD:
      label = gtk_label_new (_("Password error! Try to reconnecting..."));
      break;

    case SOMA_ERR_POSIX:
      label = gtk_label_new (_("System error! Try to reconnecting..."));
      break;

    case SOMA_ERR_USER:
      label = gtk_label_new (_("No correct data! Try to reconnecting..."));
      break;

    case SOMA_ERR_SSL_REQUEST:
      quit (NULL, NULL);
      break;

    case SOMA_ERR_NO_SSL_REQUEST:
      quit (NULL, NULL);
      break;

    default:
      label = gtk_label_new (_("Generic error! Try to reconnecting..."));
      break;
    }

  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  pb = gtk_progress_bar_new ();
  gtk_box_pack_start (GTK_BOX (box), pb, FALSE, FALSE, 0);

  sep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (box), sep, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock ("gtk-quit");
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (try_reconnect_b), &cancel);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (window);

  while (!cancel)
    {
      while (gtk_events_pending ())
	gtk_main_iteration ();

      gtk_progress_bar_pulse (GTK_PROGRESS_BAR (pb));
      g_usleep (50000);

      soma_check_password (controller);
      if (soma_error (controller) == SOMA_ERR_OK)
	{
	  ret = 0;
	  break;
	}
    }

  gtk_widget_destroy (window);

  return ret;
}

/* EOF */
