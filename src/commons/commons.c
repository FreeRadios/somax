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

static gchar *make_valid_utf8 (const gchar * name);

char *
somax_day (int d)
{
  switch (d)
    {
    case 0:
    case 7:
      return _("Sunday");
    case 1:
      return _("Monday");
    case 2:
      return _("Tuesday");
    case 3:
      return _("Wednesday");
    case 4:
      return _("Thursday");
    case 5:
      return _("Friday");
    case 6:
      return _("Saturday");
    default:
      return _("Unknown");
    }
}

char *
somax_mini_day (int d)
{
  switch (d)
    {
    case 0:
      return _("Sun");
    case 1:
      return _("Mon");
    case 2:
      return _("Tue");
    case 3:
      return _("Wed");
    case 4:
      return _("Thu");
    case 5:
      return _("Fri");
    case 6:
      return _("Sat");
    default:
      return _("???");
    }
}

char *
somax_month (int d)
{
  switch (d)
    {
    case 0:
      return _("January");
    case 1:
      return _("February");
    case 2:
      return _("March");
    case 3:
      return _("April");
    case 4:
      return _("May");
    case 5:
      return _("June");
    case 6:
      return _("July");
    case 7:
      return _("August");
    case 8:
      return _("September");
    case 9:
      return _("October");
    case 10:
      return _("November");
    case 11:
      return _("December");
    default:
      return _("Unknown");
    }
}

char *
somax_mini_month (int d)
{
  switch (d)
    {
    case 0:
      return _("Jan");
    case 1:
      return _("Feb");
    case 2:
      return _("Mar");
    case 3:
      return _("Apr");
    case 4:
      return _("May");
    case 5:
      return _("Jun");
    case 6:
      return _("Jul");
    case 7:
      return _("Aug");
    case 8:
      return _("Sep");
    case 9:
      return _("Oct");
    case 10:
      return _("Nov");
    case 11:
      return _("Dec");
    default:
      return _("???");
    }
}

char *
somax_markup (char *text)
{
  char *ret;
  int len;
  int i, j;

  if (!text)
    return g_strdup ("");

  len = strlen (text);
  ret = (char *) g_malloc (sizeof (char) * ((len * 6) + 1));

  for (i = j = 0; i < len; i++)
    {
      if (text[i] == '<')
	{
	  ret[j++] = '&';
	  ret[j++] = 'l';
	  ret[j++] = 't';
	  ret[j++] = ';';
	}
      else if (text[i] == '>')
	{
	  ret[j++] = '&';
	  ret[j++] = 'g';
	  ret[j++] = 't';
	  ret[j++] = ';';
	}
      else if (text[i] == '&')
	{
	  ret[j++] = '&';
	  ret[j++] = 'a';
	  ret[j++] = 'm';
	  ret[j++] = 'p';
	  ret[j++] = ';';
	}
      else if (text[i] == '\'')
	{
	  ret[j++] = '&';
	  ret[j++] = 'a';
	  ret[j++] = 'p';
	  ret[j++] = 'o';
	  ret[j++] = 's';
	  ret[j++] = ';';
	}
      else if (text[i] == '\"')
	{
	  ret[j++] = '&';
	  ret[j++] = 'q';
	  ret[j++] = 'u';
	  ret[j++] = 'o';
	  ret[j++] = 't';
	  ret[j++] = ';';
	}
      else
	ret[j++] = text[i];
    }

  ret[j] = 0;

  return ret;
}

int
somax_check (soma_controller * c)
{
  if (!c)
    return 1;

  switch (soma_error (c))
    {

    case SOMA_ERR_OK:
      return 0;

    case SOMA_ERR_SSL:
      dialog_msg (_("SSL error!"));
      return 1;

    case SOMA_ERR_CONNECT:
      dialog_msg (_("Connect error!"));
      return 1;

    case SOMA_ERR_HOST:
      dialog_msg (_("Host unknown!"));
      return 1;

    case SOMA_ERR_PROTOCOL:
      dialog_msg (_("Protocol error!"));
      return 1;

    case SOMA_ERR_PASSWORD:
      dialog_msg (_("Password error!"));
      return 1;

    case SOMA_ERR_POSIX:
      dialog_msg (_("System error!"));
      return 1;

    case SOMA_ERR_USER:
      dialog_msg (_("No correct data!"));
      return 1;

    case SOMA_ERR_SSL_REQUEST:
      dialog_msg
	(_
	 ("Somad is running with the SSL admin interface.\nActive the ssl encryption!"));
      return 1;

    case SOMA_ERR_NO_SSL_REQUEST:
      dialog_msg
	(_
	 ("Somad is not running with the SSL admin interface.\nDeactive the ssl encryption!"));
      return 1;

    default:
      dialog_msg (_("Internal error!"));
      return 1;
    }
}

char *
somax_to_utf8 (char *str)
{
  char *tmp;

  if (!str)
    return g_strdup ("");

  if (!(tmp = g_locale_to_utf8 (str, -1, NULL, NULL, NULL)))
    tmp = make_valid_utf8 (str);

  return tmp;
}

char *
somax_from_utf8 (char *str)
{
  char *tmp;

  if (!str)
    return g_strdup ("");

  if (!(tmp = g_locale_from_utf8 (str, -1, NULL, NULL, NULL)))
    tmp = g_strdup (str);

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
