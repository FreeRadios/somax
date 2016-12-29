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

#include "cfg.h"

static int host_parse_single (char *ip);
static int host_is_numb (char *str);
static int host_parse_netmask (char *ip_str);
static int host_parse_netmask_data (char *str);
static int host_parse_netmask_number (char *str);

gint
host_check (char *ip_str)
{
  char a[100];
  int i, k, j;

  if (!host_parse_netmask (ip_str))
    return GTK_RESPONSE_OK;

  sprintf (a, "%s.", ip_str);

  k = i = j = 0;
  while (*(a + i))
    {
      if (*(a + i) == '.')
	{
	  *(a + i) = 0;

	  if (host_parse_single (a + k))
	    {
	      char s[1024];
	      snprintf (s, sizeof (s),
			"The ip '%s' has no a correct syntax. Do you want insert it?",
			ip_str);
	      return dialog_ask (s);
	    }

	  j++;
	  k = i + 1;
	}
      i++;
    }

  return GTK_RESPONSE_OK;
}

static int
host_parse_single (char *str)
{
  /* is empty ? */
  if (!strncasecmp (str, "x", 1))
    return 0;

  if (host_is_numb (str))
    return 0;

  /* is a condition ? */
  if (*str == '>' || *str == '<')
    {
      if (*(str + 1) == '=' && host_is_numb (str + 2))
	return 0;
      else if (host_is_numb (str + 1))
	return 0;
      else
	return 1;
    }

  if (*str == '!' && host_is_numb (str + 1))
    return 0;

  return 1;
}

static int
host_is_numb (char *str)
{
  int i = 0;

  while (*(str + i))
    {

      if (*(str + i) < '0' || *(str + i) > '9')
	return 0;
      i++;
    }

  return 1;
}

static int
host_parse_netmask (char *ip_str)
{
  int len = strlen (ip_str);
  char *str = g_strdup (ip_str);
  char *netmask;
  int i, k;

  for (i = k = 0; i < len; i++)
    {
      if (str[i] == '/')
	{
	  if (!k)
	    k = i;
	  else
	    {
	      g_free (str);
	      return 1;
	    }
	}
    }

  if (!k)
    {
      g_free (str);
      return 1;
    }

  netmask = g_strdup (str + k + 1);
  str[k] = 0;

  if (host_parse_netmask_number (netmask)
      && host_parse_netmask_data (netmask))
    {
      g_free (str);
      g_free (netmask);
      return 1;
    }

  if (host_parse_netmask_data (str))
    {
      g_free (str);
      g_free (netmask);
      return 1;
    }

  g_free (netmask);
  g_free (str);

  return 0;
}

static int
host_parse_netmask_data (char *ip_str)
{
  int i;
  int k;
  char *a;

  a = (char *) g_malloc (sizeof (char) + strlen (ip_str) + 2);

  sprintf (a, "%s.", ip_str);

  k = i = 0;
  while (*(a + i))
    {
      if (*(a + i) == '.')
	{
	  *(a + i) = 0;

	  if (!host_is_numb (a + k))
	    {
	      g_free (a);
	      return 1;
	    }

	  k = i + 1;
	}
      i++;
    }

  g_free (a);
  return 0;
}

static int
host_parse_netmask_number (char *str)
{
  int i, numb;

  i = 0;
  numb = 1;

  if (!host_is_numb (str))
    return 1;

  numb = atoi (str);

  if (numb < 0 || numb > 32)
    return 1;

  return 0;
}

/* EOF */
