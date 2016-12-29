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

char **env;
gboolean set_quit = FALSE;

struct somad_data *somad_pl = NULL;
struct somad_spot_data *somad_spot = NULL;
struct maker_pl_data_t maker_pl_data;
struct maker_spot_data_t maker_spot_data;
int local_connect = 0;

soma_controller *controller = NULL;
GtkWidget *winnextitem = NULL;
GtkWidget *main_window = NULL;

void
fatal (char *text)
{
  dialog_msg (text);
  quit (NULL, NULL);
}

int
login (gpointer dummy)
{
  set_quit = 0;

  timeout_init ();
#ifdef SOMAD_CMD
  controller = create_login (thread_start, &local_connect, daemon_run);
#else
  controller = create_login (thread_start, &local_connect, NULL);
#endif

  if (!controller)
    quit (NULL, NULL);

  else
    {
      main_window = create_window ();
      gtk_widget_show (main_window);
      timeout (NULL);

      g_timeout_add (500, timeout, NULL);
      g_timeout_add (200, tooltip_timer, NULL);
    }

  return 0;
}

void
start (void)
{
  g_timeout_add (1, login, NULL);
  gtk_main ();
}

struct tm *
get_time (void)
{
  return localtime (&somad_timestep);
}

char **
get_env (void)
{
  return env;
}

int
main (int argc, char **argv, char **e)
{
#ifdef ENABLE_NLS
  setlocale (LC_ALL, "");
  textdomain (PACKAGE);
  bindtextdomain (PACKAGE, LOCALEDIR);
#endif

  env = e;

  signal (SIGPIPE, SIG_IGN);

  g_thread_init (NULL);
  gtk_init (&argc, &argv);

  splash_showhide ();

  plugin_scanner ();
  module_scanner ();
  soma_local_init ();

  preferences_data = get_preferences ();
  splash_showhide ();

  stat_init ();

  start ();

  free_preferences (preferences_data);
  signal (SIGPIPE, SIG_DFL);

  return 0;
}

int
quit (GtkWidget * w, gpointer dummy)
{
  static int no_rec = 0;

  if (no_rec)
    return TRUE;

  no_rec = 1;

#ifdef SOMAD_CMD
  if (daemon_started ())
    {
      switch (dialog_ask_with_cancel
	      (_("Somad running. You want shutdown it?")))
	{
	case GTK_RESPONSE_OK:
	  somax_quit (controller);
	  break;

	case GTK_RESPONSE_NO:
	  break;

	default:
	  no_rec = 0;
	  return TRUE;
	}
    }
#endif

  set_quit = TRUE;

  gtk_main_quit ();

  no_rec = 0;

  return FALSE;
}

/* EOF */
