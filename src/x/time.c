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

static int f_time = 0;

int
check_time (char *str)
{
  int i;
  int h = 0;
  int m = 0;
  int s = 0;
  int part = 0;

  for (i = 0; i < strlen (str); i++)
    {

      if (str[i] >= '0' && str[i] <= '9')
	part = part * 10 + str[i] - '0';

      else if (str[i] == 'h' || str[i] == 'H')
	{
	  h += part;
	  part = 0;
	}

      else if (str[i] == 'm' || str[i] == 'M')
	{
	  m += part;
	  part = 0;
	}

      else if (str[i] == 's' || str[i] == 'S')
	{
	  s += part;
	  part = 0;
	}
    }

  s += part;
  s += m * 60;
  s += h * 3600;

  return s;
}

void
time_refresh (void)
{
  char *buf, t[1024];
  static char *previous = NULL;
  struct tm *k;
  int i, len;
  time_t tt;

  switch (f_time)
    {
    case 0:
      if (!(k = get_time ()))
	return;

      if (!(buf = asctime (k)))
	return;

      len = snprintf (t, sizeof (t), _("Somad Time: %s"), buf);

      for (i = 0; i < len; i++)
	{
	  if (t[i] == '\n')
	    {
	      t[i] = 0;
	      break;
	    }
	}

      break;

    case 1:
      tt = time (NULL);
      if (!(k = localtime (&tt)))
	return;

      if (!(buf = asctime (k)))
	return;

      len = snprintf (t, sizeof (t), _("Local Time: %s"), buf);

      for (i = 0; i < len; i++)
	{
	  if (t[i] == '\n')
	    {
	      t[i] = 0;
	      break;
	    }
	}


    }

  if (!previous || strcmp (previous, t))
    {
      if (previous)
	g_free (previous);

      previous = g_strdup (t);
      gtk_button_set_label (GTK_BUTTON (b_time), t);
    }
}

void
time_clicked (GtkWidget * w, gpointer dummy)
{
  switch (f_time)
    {
    case 0:
      f_time++;
      break;
    case 1:
      f_time = 0;
      break;
    }

  time_refresh ();
}

void
time_update_widget (time_t time, char *item)
{
  somax_stat_t *stat;
  int64_t duration = 0;

  if (!time && !item)
    {
      timer_clear (timer_widget);
      return;
    }

  if (item && (stat = stat_get (item)))
    {

      if (stat->stat)
	duration = stat->stat->duration;

      stat_unref (stat);
    }

  timer_set (timer_widget, time, duration);
}

/* EOF */
