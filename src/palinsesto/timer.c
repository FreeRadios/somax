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
#include "../draw/draw.h"
#include "palinsesto.h"

int timer_time_parser_data (char *, int *, int *, int *);
int timer_time_parser_day (char *, int *);
int timer_time_parser_hourmin (char *, int *, int *);
struct somad_time *timer_time_parser (char *, char *, int);
static char *timer_mini_day (int d);
static gchar *make_valid_utf8 (const gchar * name);

void
timer_time_to_string (struct somad_time *timer, char **start, char **stop)
{
  char buffer[1024];
  char item[16];

  buffer[0] = 0;

  if (timer->start_year != -1)
    snprintf (item, sizeof (item), "%.4d", timer->start_year);
  else
    snprintf (item, sizeof (item), "xxxx");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, "-", sizeof (buffer));

  if (timer->start_month != -1)
    snprintf (item, sizeof (item), "%.2d", timer->start_month + 1);
  else
    snprintf (item, sizeof (item), "xx");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, "-", sizeof (buffer));

  if (timer->start_mday != -1)
    snprintf (item, sizeof (item), "%.2d", timer->start_mday);
  else
    snprintf (item, sizeof (item), "xx");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, " ", sizeof (buffer));

  if (timer->start_wday != -1)
    snprintf (item, sizeof (item), "%s", timer_mini_day (timer->start_wday));
  else
    snprintf (item, sizeof (item), "x");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, " ", sizeof (buffer));

  if (timer->start_hour != -1)
    snprintf (item, sizeof (item), "%.2d", timer->start_hour);
  else
    snprintf (item, sizeof (item), "xx");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, ":", sizeof (buffer));

  if (timer->start_min != -1)
    snprintf (item, sizeof (item), "%.2d", timer->start_min);
  else
    snprintf (item, sizeof (item), "xx");
  strncat (buffer, item, sizeof (buffer));

  *start = g_strdup (buffer);

  buffer[0] = 0;

  if (timer->stop_year != -1)
    snprintf (item, sizeof (item), "%.4d", timer->stop_year);
  else
    snprintf (item, sizeof (item), "xxxx");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, "-", sizeof (buffer));

  if (timer->stop_month != -1)
    snprintf (item, sizeof (item), "%.2d", timer->stop_month + 1);
  else
    snprintf (item, sizeof (item), "xx");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, "-", sizeof (buffer));

  if (timer->stop_mday != -1)
    snprintf (item, sizeof (item), "%.2d", timer->stop_mday);
  else
    snprintf (item, sizeof (item), "xx");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, " ", sizeof (buffer));

  if (timer->stop_wday != -1)
    snprintf (item, sizeof (item), "%s", timer_mini_day (timer->stop_wday));
  else
    snprintf (item, sizeof (item), "x");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, " ", sizeof (buffer));

  if (timer->stop_hour != -1)
    snprintf (item, sizeof (item), "%.2d", timer->stop_hour);
  else
    snprintf (item, sizeof (item), "xx");
  strncat (buffer, item, sizeof (buffer));
  strncat (buffer, ":", sizeof (buffer));

  if (timer->stop_min != -1)
    snprintf (item, sizeof (item), "%.2d", timer->stop_min);
  else
    snprintf (item, sizeof (item), "xx");
  strncat (buffer, item, sizeof (buffer));

  *stop = g_strdup (buffer);
}

struct somad_time *
timer_time_parser (char *start, char *stop, int timecontinued)
{
  int len;
  char *s1, *s2, *s3;
  struct somad_time *timer;

  if (!start || !stop)
    return NULL;

  timer = (struct somad_time *) g_malloc0 (sizeof (struct somad_time));
  timer->timecontinued = timecontinued;

  if (strlen (start) > strlen (stop))
    len = strlen (start);
  else
    len = strlen (stop);

  s1 = g_malloc (sizeof (char) * len);
  s2 = g_malloc (sizeof (char) * len);
  s3 = g_malloc (sizeof (char) * len);

  start = palinsesto_trim (start);
  len = sscanf (start, "%s %s %s", s1, s2, s3);

  switch (len)
    {
    case 0:
      g_free (s1);
      g_free (s2);
      g_free (s3);

      g_free (timer);
      return NULL;

    case 1:
      if (timer_time_parser_hourmin
	  (s1, &timer->start_hour, &timer->start_min))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      timer->start_wday = -1;
      timer->start_mday = -1;
      timer->start_month = -1;
      timer->start_year = -1;
      break;

    case 2:
      if (timer_time_parser_hourmin
	  (s2, &timer->start_hour, &timer->start_min))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      if (timer_time_parser_day (s1, &timer->start_wday))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      timer->start_mday = -1;
      timer->start_month = -1;
      timer->start_year = -1;
      break;

    case 3:
      if (timer_time_parser_hourmin
	  (s3, &timer->start_hour, &timer->start_min))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      if (timer_time_parser_day (s2, &timer->start_wday))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      if (timer_time_parser_data
	  (s1, &timer->start_year, &timer->start_month, &timer->start_mday))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      break;
    }

  stop = palinsesto_trim (stop);
  len = sscanf (stop, "%s %s %s", s1, s2, s3);

  switch (len)
    {
    case 0:
      g_free (s1);
      g_free (s2);
      g_free (s3);

      g_free (timer);
      return NULL;

    case 1:
      if (timer_time_parser_hourmin (s1, &timer->stop_hour, &timer->stop_min))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      timer->stop_wday = -1;
      timer->stop_mday = -1;
      timer->stop_month = -1;
      timer->stop_year = -1;
      break;

    case 2:
      if (timer_time_parser_hourmin (s2, &timer->stop_hour, &timer->stop_min))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      if (timer_time_parser_day (s1, &timer->stop_wday))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      timer->stop_mday = -1;
      timer->stop_month = -1;
      timer->stop_year = -1;
      break;

    case 3:
      if (timer_time_parser_hourmin (s3, &timer->stop_hour, &timer->stop_min))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      if (timer_time_parser_day (s2, &timer->stop_wday))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      if (timer_time_parser_data
	  (s1, &timer->stop_year, &timer->stop_month, &timer->stop_mday))
	{
	  g_free (s1);
	  g_free (s2);
	  g_free (s3);

	  g_free (timer);
	  return NULL;
	}

      break;
    }

  g_free (s1);
  g_free (s2);
  g_free (s3);

  timer_time_to_string (timer, &timer->start, &timer->stop);

  return timer;
}

/* Hour/Min */
int
timer_time_parser_hourmin (char *str, int *h, int *m)
{
  char *sh, *sm;
  int len = strlen (str);

  if (!(sh = strtok (str, ":")))
    return 1;
  sm = str + strlen (sh) + 1;
  if (strlen (sh) >= len - 1)
    return 1;

  if (*sh == 'x' || *sh == 'X')
    *h = -1;
  else
    {
      *h = atoi (sh);

      if (*h > 23 || *h < 0)
	*h = 0;
    }

  if (*sm == 'x' || *sm == 'X')
    *m = -1;
  else
    {
      *m = atoi (sm);
      if (*m > 59 || *m < 0)
	*m = 0;
    }

  return 0;
}

/* Day */
int
timer_time_parser_day (char *str, int *d)
{

  if (*str == 'x' || *str == 'X')
    {
      *d = -1;
      return 0;
    }
  if (!strcasecmp ("Sun", str) || !strcmp ("0", str))
    {
      *d = 0;
      return 0;
    }
  if (!strcasecmp ("Mon", str) || !strcmp ("1", str))
    {
      *d = 1;
      return 0;
    }
  if (!strcasecmp ("Tue", str) || !strcmp ("2", str))
    {
      *d = 2;
      return 0;
    }
  if (!strcasecmp ("Wed", str) || !strcmp ("3", str))
    {
      *d = 3;
      return 0;
    }
  if (!strcasecmp ("Thu", str) || !strcmp ("4", str))
    {
      *d = 4;
      return 0;
    }
  if (!strcasecmp ("Fri", str) || !strcmp ("5", str))
    {
      *d = 5;
      return 0;
    }
  if (!strcasecmp ("Sat", str) || !strcmp ("6", str))
    {
      *d = 6;
      return 0;
    }

  return 1;
}

/* Date */
int
timer_time_parser_data (char *str, int *y, int *m, int *d)
{
  char *sd;
  char *sm;
  char *sy;

  if (!(sy = strtok (str, "-")))
    {
      if (*sy == 'x' || *sy == 'X')
	*d = -1;
      else
	{
	  *d = atoi (sy);

	  if (*d > 30 || *d < 0)
	    *d = 0;
	}

      *m = -1;
      *y = -1;
      return 0;
    }

  if (!(sm = strtok (NULL, "-")))
    {
      if (*sy == 'x' || *sy == 'X')
	*m = -1;
      else
	{
	  *m = atoi (sy);
	  if (*m)
	    *m -= 1;

	  if (*m > 11 || *m < 0)
	    *m = 0;
	}

      if (*sm == 'x' || *sm == 'X')
	*d = -1;
      else
	{
	  *d = atoi (sm);

	  if (*d > 30 || *d < 0)
	    *d = 0;
	}

      *y = -1;
      return 1;
    }

  if (!(sd = strtok (NULL, "\0")))
    return 1;

  if (*sy == 'x' || *sy == 'X')
    *y = -1;
  else
    {
      *y = atoi (sy);

      if (*y > 3000 || *y < 0)
	*y = 0;
    }

  if (*sm == 'x' || *sm == 'X')
    *m = -1;
  else
    {
      *m = atoi (sm);
      if (*m)
	*m -= 1;

      if (*m > 11 || *m < 0)
	*m = 0;
    }

  if (*sd == 'x' || *sd == 'X')
    *d = -1;
  else
    {
      *d = atoi (sd);

      if (*d > 30 || *d < 0)
	*d = 0;
    }

  return 0;
}

int
timer_check_day (struct somad_time *timer, int y, int m, int d, int wd)
{
  int a, b, now, max;
  int ret;

  a = b = now = max = 0;

  /* YYYYDDMM... */
  if (timer->start_year != -1 && timer->stop_year != -1
      && timer->start_year <= timer->stop_year)
    {
      a = timer->start_year;
      b = timer->stop_year;
      now = y;
      max = 3000;
    }

  /* or DDMM... */
  if (timer->start_month != -1 && timer->stop_month != -1)
    {
      a = (a * 12) + timer->start_month;
      b = (b * 12) + timer->stop_month;
      now = (now * 12) + m;
      max = 36000;		/* 3000 years * 12 months */
    }

  /* or DDMM... */
  else if (max)
    {
      a = a * 12;
      b = b * 12;
      now = (now * 12) + m;
      max = 36000;		/* 3000 years * 12 months */
    }

  /* or MM... */
  if (timer->start_mday != -1 && timer->stop_mday != -1)
    {
      a = (a * 31) + timer->start_mday;
      b = (b * 31) + timer->stop_mday;
      now = (now * 31) + d;
      max = 1116000;		/* 3000 years * 12 months * 31 days */
    }
  else if (max)
    {
      a = a * 31;
      b = b * 31;
      now = (now * 31) + d;
      max = 1116000;		/* 3000 years * 12 months * 31 days */
    }

  if (max)
    {
      ret = timer_check_item (a, b, now, max);
      if (ret > 0)
	return 1;		/* 24 hours * 60 minutes */

      if (timer->timecontinued)
	return 0;
    }

  /* Checking the day of week */
  ret = timer_check_item (timer->start_wday, timer->stop_wday, wd, 7);
  if (ret > 0)
    return 1;			/* 24 hours * 60 minutes */

  if (timer->timecontinued && !ret && timer->start_wday != -1
      && timer->stop_wday != -1)
    return 0;

  return 0;
}

/* Check 2 element */
int
timer_check_item (int start, int stop, int now, int max)
{
  if (start == now && stop == now)
    return -3;

  if (start == now)
    return -1;

  if (stop == now)
    return -2;

  if (start < 0 || stop < 0)
    return 0;

  if (start < stop)
    {
      if (start < now && stop > now)
	return 0;

      if (start < now && stop < now)
	return max - now + start;

      return start - now;
    }

  else if (start > stop)
    {
      if (start > now && stop > now)
	return 0;

      if (start > now && stop < now)
	return start - now;

      return 0;
    }

  else
    {
      if (start < now)
	return max - now + start;

      return start - now;
    }
}

/* Check the pl */
int
timer_check (struct somad_time *timer, struct tm *check)
{
  struct tm *k;
  int date_ret, day_ret, min_ret;
  int a, b, now, max;

  if (!timer)
    return -1;

  k = check;

  date_ret = day_ret = min_ret = -4;
  a = b = now = max = 0;

  /* YYYYDDMM... */
  if (timer->start_year != -1 && timer->stop_year != -1
      && timer->start_year <= timer->stop_year)
    {
      a = timer->start_year;
      b = timer->stop_year;
      now = k->tm_year + 1900;
      max = 3000;
    }

  /* or DDMM... */
  if (timer->start_month != -1 && timer->stop_month != -1)
    {
      a = (a * 12) + timer->start_month;
      b = (b * 12) + timer->stop_month;
      now = (now * 12) + k->tm_mon;
      max = 36000;		/* 3000 years * 12 months */
    }
  else if (max)
    {
      a = a * 12;
      b = b * 12;
      now = (now * 12) + k->tm_mon;
      max = 36000;		/* 3000 years * 12 months */
    }

  /* or MM... */
  if (timer->start_mday != -1 && timer->stop_mday != -1)
    {
      a = (a * 31) + timer->start_mday;
      b = (b * 31) + timer->stop_mday;
      now = (now * 31) + k->tm_mday;
      max = 1116000;		/* 3000 years * 12 months * 31 days */
    }
  else if (max)
    {
      a = a * 31;
      b = b * 31;
      now = (now * 31) + k->tm_mday;
      max = 1116000;		/* 3000 years * 12 months * 31 days */
    }

  if (max)
    {
      date_ret = timer_check_item (a, b, now, max);
      if (date_ret > 0)
	return date_ret * 1440;	/* 24 hours * 60 minutes */

      if (timer->timecontinued)
	{
	  switch (date_ret)
	    {
	    case 0:
	      return 0;

	    case -1:
	      a = now = 0;
	      b = -1;

	      if (!timer->start_hour != -1)
		{
		  a = timer->start_hour;
		  now = k->tm_hour;
		  b = 0;
		}

	      if (!timer->start_min != -1)
		{
		  a = (a * 60) + timer->start_min;
		  now = (now * 60) + k->tm_min;
		  b = 0;
		}

	      if (b < 0)
		break;

	      if (now >= a)
		return 0;

	      break;

	    case -2:
	      a = now = 0;
	      b = -1;

	      if (!timer->stop_hour != -1)
		{
		  a = timer->stop_hour;
		  now = k->tm_hour;
		  b = 0;
		}

	      if (!timer->stop_min != -1)
		{
		  a = (a * 60) + timer->stop_min;
		  now = (now * 60) + k->tm_min;
		  b = 0;
		}

	      if (b < 0)
		break;

	      if (now < a)
		return 0;

	      break;

	    case -3:
	      break;
	    }
	}
    }

  /* Checking the day of week */
  day_ret =
    timer_check_item (timer->start_wday, timer->stop_wday, k->tm_wday, 7);
  if (day_ret > 0)
    return day_ret * 1440;	/* 24 hours * 60 minutes */

  if (timer->timecontinued && !day_ret && timer->start_wday != -1
      && timer->stop_wday != -1)
    return 0;

  a = b = now = max = 0;

  /* Checking the hours:minutes */
  if (timer->start_hour != -1 && timer->stop_hour != -1)
    {
      a = timer->start_hour;
      b = timer->stop_hour;
      now = k->tm_hour;
      max = 24;
    }

  if (timer->start_min != -1 && timer->stop_min != -1)
    {
      a = (a * 60) + timer->start_min;
      b = (b * 60) + timer->stop_min;
      now = (now * 60) + k->tm_min;
      max = 1440;
    }
  else if (max)
    {
      a = a * 60;
      b = b * 60;
      now = (now * 60) + k->tm_min;
      max = 1440;
    }

  if (max)
    {
      if (a == b)
	return 0;

      min_ret = timer_check_item (a, b, now, max);

      if (timer->timecontinued)
	{
	  if (a < b)
	    {
	      if ((now < a && (day_ret == -2 || date_ret == -2)) ||
		  (now > b && (day_ret == -1 || date_ret == -1)))
		return 0;
	    }
	  else if (a > b)
	    {
	      if (now < a && (day_ret == -1 || date_ret == -1))
		return min_ret + (a - b);
	      else if (now > b && (day_ret == -2 || date_ret == -2))
		{
		  return 1440 - b;	/* Correct: Tomorrow + (1440 - b) */
		}
	    }
	}

      if (min_ret > 0)
	return min_ret;

      if (min_ret == -1 || min_ret == 0)
	return 0;

      /* If NOW is the end of PALINSESTO i can't know the time of next
       * palinsesto, so... */
      if (min_ret == -2 || min_ret == -3)
	{
	  min_ret = timer_check_item (a, b, now + 1, max);
	  if (min_ret > 0)
	    return min_ret + 1;
	}

      if (min_ret < 0)
	return 0;

      return min_ret;
    }

  return 0;
}

static char *
timer_mini_day (int d)
{
  switch (d)
    {
    case 0:
      return "Sun";
    case 1:
      return "Mon";
    case 2:
      return "Tue";
    case 3:
      return "Wed";
    case 4:
      return "Thu";
    case 5:
      return "Fri";
    case 6:
      return "Sat";
    default:
      return "???";
    }
}

char *
p_old_markup (char *text)
{
  char *ret;
  int len;
  int i, j;
  char *tmp;

  if (!text)
    return g_strdup ("");

  if (!(tmp = g_locale_from_utf8 (text, -1, NULL, NULL, NULL)))
    tmp = g_strdup (text);

  len = strlen (tmp);
  ret = (char *) g_malloc (sizeof (char) * ((len * 2) + 1));

  for (i = j = 0; i < len; i++)
    {
      if (tmp[i] == '\"')
	ret[j++] = '\\';
      ret[j++] = tmp[i];
    }

  ret[j] = 0;
  g_free (tmp);

  return ret;
}

char *
p_xml_markup (char *text)
{
  char *ret;
  int len;
  int i, j;
  char *tmp;

  if (!text)
    return g_strdup ("");

  if (!(tmp = g_locale_from_utf8 (text, -1, NULL, NULL, NULL)))
    tmp = g_strdup (text);

  len = strlen (tmp);
  ret = (char *) g_malloc (sizeof (char) * ((len * 6) + 1));

  for (i = j = 0; i < len; i++)
    {
      if (tmp[i] == '<')
	{
	  ret[j++] = '&';
	  ret[j++] = 'l';
	  ret[j++] = 't';
	  ret[j++] = ';';
	}
      else if (tmp[i] == '>')
	{
	  ret[j++] = '&';
	  ret[j++] = 'g';
	  ret[j++] = 't';
	  ret[j++] = ';';
	}
      else if (tmp[i] == '&')
	{
	  ret[j++] = '&';
	  ret[j++] = 'a';
	  ret[j++] = 'm';
	  ret[j++] = 'p';
	  ret[j++] = ';';
	}
      else if (tmp[i] == '\'')
	{
	  ret[j++] = '&';
	  ret[j++] = 'a';
	  ret[j++] = 'p';
	  ret[j++] = 'o';
	  ret[j++] = 's';
	  ret[j++] = ';';
	}
      else if (tmp[i] == '\"')
	{
	  ret[j++] = '&';
	  ret[j++] = 'q';
	  ret[j++] = 'u';
	  ret[j++] = 'o';
	  ret[j++] = 't';
	  ret[j++] = ';';
	}
      else
	ret[j++] = tmp[i];
    }

  g_free (tmp);
  ret[j] = 0;

  return ret;
}

char *
p_utf8 (char *text)
{
  char *tmp;

  if (!text)
    return g_strdup ("");

  if (!(tmp = g_locale_to_utf8 (text, -1, NULL, NULL, NULL)))
    tmp = make_valid_utf8 (text);

  return tmp;
}

/* 
 * From GLIB 
 * Authors: Havoc Pennington <hp@redhat.com>, Owen Taylor <otaylor@redhat.com>
 */
static gchar *
make_valid_utf8 (const gchar * name)
{
  GString *string;
  const gchar *remainder, *invalid;
  gint remaining_bytes, valid_bytes;

  string = NULL;
  remainder = name;
  remaining_bytes = strlen (name);

  while (remaining_bytes != 0)
    {
      if (g_utf8_validate (remainder, remaining_bytes, &invalid))
	break;
      valid_bytes = invalid - remainder;

      if (string == NULL)
	string = g_string_sized_new (remaining_bytes);

      g_string_append_len (string, remainder, valid_bytes);
      g_string_append_c (string, '?');

      remaining_bytes -= valid_bytes + 1;
      remainder = invalid + 1;
    }

  if (string == NULL)
    return g_strdup (name);

  g_string_append (string, remainder);

  return g_string_free (string, FALSE);
}

/* EOF */
