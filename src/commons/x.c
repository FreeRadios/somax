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

#include "commons.h"
#include "x.h"

struct somax_cb_data
{
  GThread *th;
  gboolean flag;
  gboolean abort;

  soma_controller *c;
  int ret_int;

  char *ret_char;
  char **ret_dchar;
  soma_stat *ret_stat;
  soma_stat_dir *ret_stat_dir;

  int flag_int;
  int flag_int2;
  char *flag_char;
  char *flag_char2;
};

static GStaticMutex x_abort_mutex = G_STATIC_MUTEX_INIT;
static GThread *x_abort_th = NULL;
static GList *x_abort_list = NULL;

static gpointer x_abort_thread (gpointer dummy);
static void x_data_free (struct somax_cb_data *data);

static gboolean somax_wait (struct somax_cb_data *data);
static gboolean somax_wait_empty (struct somax_cb_data *data);
static GtkWidget *somax_wait_win (struct somax_cb_data *data);
static void somax_wait_win_abort (GtkWidget *, struct somax_cb_data *data);
GtkWidget *somax_win (void);
static void somax_win_abort (GtkWidget *, GtkWidget * window);
static gpointer somax_open_tcp_real (gpointer dummy);
static gpointer somax_open_unix_real (gpointer dummy);
static gpointer somax_stop_real (gpointer dummy);
static gpointer somax_start_real (gpointer dummy);
static gpointer somax_quit_real (gpointer dummy);
static gpointer somax_set_pause_real (gpointer dummy);
static gpointer somax_set_unpause_real (gpointer dummy);
static gpointer somax_skip_real (gpointer dummy);
static gpointer somax_skip_next_real (gpointer dummy);
static gpointer somax_read_directory_real (gpointer dummy);
static gpointer somax_read_palinsesto_real (gpointer dummy);
static gpointer somax_read_spot_real (gpointer dummy);
static gpointer somax_old_palinsesto_real (gpointer dummy);
static gpointer somax_old_spot_real (gpointer dummy);
static gpointer somax_set_default_palinsesto_real (gpointer dummy);
static gpointer somax_set_default_spot_real (gpointer dummy);
static gpointer somax_new_palinsesto_file_real (gpointer dummy);
static gpointer somax_new_spot_file_real (gpointer dummy);
static gpointer somax_get_palinsesto_real (gpointer dummy);
static gpointer somax_get_old_palinsesto_real (gpointer dummy);
static gpointer somax_get_spot_real (gpointer dummy);
static gpointer somax_get_old_spot_real (gpointer dummy);
static gpointer somax_get_path_real (gpointer dummy);
static gpointer somax_nextitem_set_spot_real (gpointer dummy);
static gpointer somax_nextitem_remove_real (gpointer dummy);
static gpointer somax_nextitem_set_real (gpointer dummy);
static gpointer somax_nextitem_path_set_real (gpointer dummy);
static gpointer somax_get_stat_real (gpointer dummy);
static gpointer somax_get_stat_dir_real (gpointer dummy);
static gpointer somax_get_stat_path_real (gpointer dummy);
static gpointer somax_get_stat_dir_path_real (gpointer dummy);
static gpointer somax_local_stat_real (gpointer dummy);
static gpointer somax_local_stat_dir_real (gpointer dummy);
static gpointer somax_remove_item_real (gpointer dummy);
static gpointer somax_remove_spot_real (gpointer dummy);

static gboolean
somax_wait (struct somax_cb_data *data)
{
  GtkWidget *w;
  GdkCursor *cursor;

  w = somax_wait_win (data);
  gtk_widget_show_all (w);

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (w->window, cursor);

  while (data->flag == FALSE && data->abort == FALSE)
    {
      while (gtk_events_pending () && data->flag == FALSE
	     && data->abort == FALSE)
	gtk_main_iteration ();

      g_usleep (500);
    }

  gdk_cursor_unref (cursor);
  gtk_widget_destroy (w);

  /* If abort, I insert in a list of abort sections: */
  if (data->abort == TRUE)
    {
      g_static_mutex_lock (&x_abort_mutex);
      x_abort_list = g_list_append (x_abort_list, data);

      /* Exec a thread to check them: */
      if (!x_abort_th)
	x_abort_th = g_thread_create (x_abort_thread, NULL, FALSE, NULL);

      g_static_mutex_unlock (&x_abort_mutex);
      return FALSE;
    }

  return TRUE;
}

static gboolean
somax_wait_empty (struct somax_cb_data *data)
{
  while (data->flag == FALSE && data->abort == FALSE)
    {
      while (gtk_events_pending () && data->flag == FALSE
	     && data->abort == FALSE)
	gtk_main_iteration ();

      g_usleep (500);
    }

  /* If abort, I insert in a list of abort sections: */
  if (data->abort == TRUE && data->flag == FALSE)
    {

      g_static_mutex_lock (&x_abort_mutex);
      x_abort_list = g_list_append (x_abort_list, data);

      /* Exec a thread to check them: */
      if (!x_abort_th)
	x_abort_th = g_thread_create (x_abort_thread, NULL, FALSE, NULL);

      g_static_mutex_unlock (&x_abort_mutex);
      return FALSE;
    }

  return TRUE;
}

static GtkWidget *
somax_wait_win (struct somax_cb_data *data)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *button;
  GtkWidget *sp;
  GtkWidget *label;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("Please Wait..."));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_type_hint (GTK_WINDOW (window),
			    GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);

  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  gtk_widget_set_size_request (window, 200, -1);

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (window), frame);

  box = gtk_vbox_new (FALSE, 8);
  gtk_container_add (GTK_CONTAINER (frame), box);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  image = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Please Wait..."));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  sp = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (box), sp, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  button = gtk_event_box_new ();
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_with_label (_("Abort"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (somax_wait_win_abort), data);

  return window;
}

static void
somax_wait_win_abort (GtkWidget * widget, struct somax_cb_data *data)
{
  data->abort = TRUE;
}

GtkWidget *
somax_win (void)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *sp;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("Please Wait..."));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_type_hint (GTK_WINDOW (window),
			    GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);

  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  gtk_window_resize (GTK_WINDOW (window), 200, 1);

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (window), frame);

  box = gtk_vbox_new (FALSE, 8);
  gtk_container_add (GTK_CONTAINER (frame), box);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  image = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  g_object_set_data (G_OBJECT (window), "label", label);

  sp = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (box), sp, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  button = gtk_event_box_new ();
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_with_label (_("Abort"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (somax_win_abort), window);

  return window;
}

static void
somax_win_abort (GtkWidget * widget, GtkWidget * window)
{
  gboolean *abort;

  if ((abort = g_object_get_data ((gpointer) window, "abort")))
    *abort = TRUE;
}

soma_controller *
somax_open_tcp (char *server, int port, char *password, int ssl)
{
  struct somax_cb_data *data;
  soma_controller *c;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->flag_char = g_strdup (server);
  data->flag_int = port;
  data->flag_char2 = g_strdup (password);
  data->flag_int2 = ssl;

  if (!(data->th = g_thread_create (somax_open_tcp_real, data, TRUE, NULL)))
    return NULL;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      c = data->c;
      x_data_free (data);
      return c;
    }

  return NULL;
}

static gpointer
somax_open_tcp_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->c =
    soma_open_tcp (data->flag_char, data->flag_int, data->flag_char2,
		   data->flag_int2);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

soma_controller *
somax_open_unix (char *unixpath, char *password, int ssl)
{
  struct somax_cb_data *data;
  soma_controller *c;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->flag_char = g_strdup (unixpath);
  data->flag_char2 = g_strdup (password);
  data->flag_int = ssl;

  if (!(data->th = g_thread_create (somax_open_unix_real, data, TRUE, NULL)))
    return NULL;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      c = data->c;
      x_data_free (data);
      return c;
    }

  return NULL;
}

static gpointer
somax_open_unix_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->c =
    soma_open_unix (data->flag_char, data->flag_char2, data->flag_int);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_stop (soma_controller * c, int time)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_int = time;

  if (!(data->th = g_thread_create (somax_stop_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_stop_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_stop (data->c, data->flag_int);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_start (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!(data->th = g_thread_create (somax_start_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_start_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_start (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_set_pause (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!(data->th = g_thread_create (somax_set_pause_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_set_pause_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_set_pause (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_set_unpause (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!(data->th = g_thread_create (somax_set_unpause_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_set_unpause_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_set_unpause (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_skip (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!(data->th = g_thread_create (somax_skip_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_skip_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_skip (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_skip_next (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!(data->th = g_thread_create (somax_skip_next_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_skip_next_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_skip_next (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_read_directory (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!
      (data->th =
       g_thread_create (somax_read_directory_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_read_directory_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_read_directory (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_read_palinsesto (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!
      (data->th =
       g_thread_create (somax_read_palinsesto_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_read_palinsesto_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_read_palinsesto (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_read_spot (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!(data->th = g_thread_create (somax_read_spot_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_read_spot_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_read_spot (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_old_palinsesto (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!
      (data->th =
       g_thread_create (somax_old_palinsesto_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_old_palinsesto_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_old_palinsesto (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_old_spot (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!(data->th = g_thread_create (somax_old_spot_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_old_spot_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_old_spot (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_quit (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!(data->th = g_thread_create (somax_quit_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_quit_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_quit (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

char *
somax_get_palinsesto (soma_controller * c)
{
  struct somax_cb_data *data;
  char *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!
      (data->th =
       g_thread_create (somax_get_palinsesto_real, data, TRUE, NULL)))
    return NULL;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_char;
      data->ret_char = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_get_palinsesto_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_char = soma_get_palinsesto (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

char *
somax_get_old_palinsesto (soma_controller * c)
{
  struct somax_cb_data *data;
  char *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!
      (data->th =
       g_thread_create (somax_get_old_palinsesto_real, data, TRUE, NULL)))
    return NULL;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_char;
      data->ret_char = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_get_old_palinsesto_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_char = soma_get_old_palinsesto (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_set_default_palinsesto (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!
      (data->th =
       g_thread_create (somax_set_default_palinsesto_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_set_default_palinsesto_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_set_default_palinsesto (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_new_palinsesto_file (soma_controller * c, char *file)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_char = g_strdup (file);

  if (!
      (data->th =
       g_thread_create (somax_new_palinsesto_file_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_new_palinsesto_file_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_new_palinsesto_file (data->c, data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

char *
somax_get_spot (soma_controller * c)
{
  struct somax_cb_data *data;
  char *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!(data->th = g_thread_create (somax_get_spot_real, data, TRUE, NULL)))
    return NULL;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_char;
      data->ret_char = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_get_spot_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_char = soma_get_spot (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

char *
somax_get_old_spot (soma_controller * c)
{
  struct somax_cb_data *data;
  char *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!
      (data->th =
       g_thread_create (somax_get_old_spot_real, data, TRUE, NULL)))
    return NULL;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_char;
      data->ret_char = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_get_old_spot_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_char = soma_get_old_spot (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_set_default_spot (soma_controller * c)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;

  if (!
      (data->th =
       g_thread_create (somax_set_default_spot_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_set_default_spot_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_set_default_spot (data->c);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_new_spot_file (soma_controller * c, char *file)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_char = g_strdup (file);

  if (!
      (data->th =
       g_thread_create (somax_new_spot_file_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_new_spot_file_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_new_spot_file (data->c, data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

char **
somax_get_path (soma_controller * c, char *file)
{
  struct somax_cb_data *data;
  char **ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_char = g_strdup (file);

  if (!(data->th = g_thread_create (somax_get_path_real, data, TRUE, NULL)))
    return NULL;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_dchar;
      data->ret_dchar = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_get_path_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_dchar = soma_get_path (data->c, data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

void
somax_get_path_free (char **a)
{
  soma_get_path_free (a);
}

void
somax_free (soma_controller * a)
{
  soma_free (a);
}

int
somax_nextitem_set_spot (soma_controller * c, int flag)
{
  struct somax_cb_data *data;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_int = flag;

  if (!
      (data->th =
       g_thread_create (somax_nextitem_set_spot_real, data, TRUE, NULL)))
    return 1;

  if (somax_wait (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_nextitem_set_spot_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_nextitem_set_spot (data->c, data->flag_int);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_nextitem_remove (GtkWidget * w, char *s, soma_controller * c, int flag)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_int = flag;

  if (!
      (data->th =
       g_thread_create (somax_nextitem_remove_real, data, TRUE, NULL)))
    return 1;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_nextitem_remove_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_nextitem_remove (data->c, data->flag_int);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_nextitem_set (GtkWidget * w, char *s, soma_controller * c, char *file)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_char = g_strdup (file);

  if (!
      (data->th =
       g_thread_create (somax_nextitem_set_real, data, TRUE, NULL)))
    return 1;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_nextitem_set_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_nextitem_set (data->c, data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_nextitem_path_set (GtkWidget * w, char *s, soma_controller * c,
			 char *file)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_char = g_strdup (file);

  if (!
      (data->th =
       g_thread_create (somax_nextitem_path_set_real, data, TRUE, NULL)))
    return 1;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_nextitem_path_set_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_nextitem_path_set (data->c, data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

soma_stat *
somax_get_stat (GtkWidget * w, char *s, soma_controller * c, char *flag)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  soma_stat *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_char = g_strdup (flag);

  if (!(data->th = g_thread_create (somax_get_stat_real, data, TRUE, NULL)))
    return NULL;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_stat;
      data->ret_stat = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_get_stat_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_stat = soma_get_stat (data->c, data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

soma_stat_dir *
somax_get_stat_dir (GtkWidget * w, char *s, soma_controller * c, char *flag)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  soma_stat_dir *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_char = g_strdup (flag);

  if (!
      (data->th =
       g_thread_create (somax_get_stat_dir_real, data, TRUE, NULL)))
    return NULL;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_stat_dir;
      data->ret_stat_dir = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_get_stat_dir_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_stat_dir = soma_get_stat_dir (data->c, data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

soma_stat *
somax_get_stat_path (GtkWidget * w, char *s, soma_controller * c, char *flag)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  soma_stat *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_char = g_strdup (flag);

  if (!
      (data->th =
       g_thread_create (somax_get_stat_path_real, data, TRUE, NULL)))
    return NULL;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_stat;
      data->ret_stat = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_get_stat_path_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_stat = soma_get_stat_path (data->c, data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

soma_stat_dir *
somax_get_stat_dir_path (GtkWidget * w, char *s, soma_controller * c,
			 char *flag)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  soma_stat_dir *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_char = g_strdup (flag);

  if (!
      (data->th =
       g_thread_create (somax_get_stat_dir_path_real, data, TRUE, NULL)))
    return NULL;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_stat_dir;
      data->ret_stat_dir = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_get_stat_dir_path_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_stat_dir =
    (void *) soma_get_stat_dir_path (data->c, data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

soma_stat *
somax_local_stat (GtkWidget * w, char *s, char *flag)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  soma_stat *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->flag_char = g_strdup (flag);

  if (!(data->th = g_thread_create (somax_local_stat_real, data, TRUE, NULL)))
    return NULL;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_stat;
      data->ret_stat = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_local_stat_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_stat = (void *) soma_local_stat (data->flag_char);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

soma_stat_dir *
somax_local_stat_dir (GtkWidget * w, char *s, char *flag)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  soma_stat_dir *ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->flag_char = g_strdup (flag);

  if (!
      (data->th =
       g_thread_create (somax_local_stat_dir_real, data, TRUE, NULL)))
    return NULL;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_stat_dir;
      data->ret_stat_dir = NULL;
      x_data_free (data);
      return ret;
    }

  return NULL;
}

static gpointer
somax_local_stat_dir_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_stat_dir = (void *) soma_local_stat_dir (data->flag_char, 0);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_remove_item (GtkWidget * w, char *s, soma_controller * c, int id)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_int = id;

  if (!
      (data->th = g_thread_create (somax_remove_item_real, data, TRUE, NULL)))
    return 1;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_remove_item_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_remove_item (data->c, data->flag_int);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

int
somax_remove_spot (GtkWidget * w, char *s, soma_controller * c, int id)
{
  struct somax_cb_data *data;
  GtkWidget *label;
  int ret;

  data = g_malloc0 (sizeof (struct somax_cb_data));

  data->c = c;
  data->flag_int = id;

  if (!
      (data->th = g_thread_create (somax_remove_spot_real, data, TRUE, NULL)))
    return 1;

  if ((label = g_object_get_data (G_OBJECT (w), "label")))
    gtk_label_set_text (GTK_LABEL (label), s);

  g_object_set_data (G_OBJECT (w), "abort", &data->abort);

  if (somax_wait_empty (data) == TRUE)
    {
      g_thread_join (data->th);

      ret = data->ret_int;
      x_data_free (data);
      return ret;
    }

  return 1;
}

static gpointer
somax_remove_spot_real (gpointer dummy)
{
  struct somax_cb_data *data = (struct somax_cb_data *) dummy;

  signal (SIGPIPE, SIG_IGN);
  data->ret_int = soma_remove_spot (data->c, data->flag_int);

  data->flag = TRUE;
  g_thread_exit (NULL);
  return NULL;
}

/* Garbage collector of threads: */
static gpointer
x_abort_thread (gpointer dummy)
{
  GList *list;
  struct somax_cb_data *data;

  while (1)
    {
      g_static_mutex_lock (&x_abort_mutex);
      list = x_abort_list;

      while (list)
	{
	  data = list->data;

	  if (data->flag == TRUE)
	    {
	      list = list->next;

	      x_abort_list = g_list_remove (x_abort_list, data);

	      g_thread_join (data->th);
	      x_data_free (data);
	      continue;
	    }

	  list = list->next;
	}

      if (!x_abort_list)
	break;

      g_static_mutex_unlock (&x_abort_mutex);
      g_usleep (500);
    }

  x_abort_th = NULL;
  g_static_mutex_unlock (&x_abort_mutex);

  g_thread_exit (NULL);
  return NULL;
}

static void
x_data_free (struct somax_cb_data *data)
{
  if (!data)
    return;

  if (data->ret_char)
    g_free (data->ret_char);

  if (data->ret_dchar)
    somax_get_path_free (data->ret_dchar);

  if (data->flag_char)
    g_free (data->flag_char);

  if (data->flag_char2)
    g_free (data->flag_char2);

  if (data->ret_stat)
    soma_stat_free (data->ret_stat);

  if (data->ret_stat_dir)
    soma_stat_dir_free (data->ret_stat_dir);

  g_free (data);
}

/* EOF */
