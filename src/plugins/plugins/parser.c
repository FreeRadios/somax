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

#include "plugin.h"
#include "plugin_internal.h"

static int __plgn_print_data = 0;

static char *__plgn_file = NULL;

static char *__plgn_description = NULL;
static char *__plgn_author = NULL;
static char *__plgn_licence = NULL;
static char *__plgn_version = NULL;
static char *__plgn_name = NULL;

void
somax_plgn_check_arg (int argc, char **argv)
{
  if (argc != 2)
    {
      fprintf (stdout,
	       "This software is part of somax-editor. Use it only as plugin!\n");
      exit (1);
    }

  if (strcmp (argv[1], "data")
      && g_file_test (argv[1], G_FILE_TEST_EXISTS) == FALSE)
    {
      fprintf (stdout,
	       "This software is part of somax-editor. Use it only as plugin!\n");
      exit (1);
    }

  if (!strcmp (argv[1], "data"))
    __plgn_print_data = 1;
  else
    __plgn_file = g_strdup (argv[1]);
}

void
somax_plgn_set_description (char *description)
{
  if (__plgn_description)
    free (__plgn_description);

  __plgn_description = strdup (description);
}

void
somax_plgn_set_author (char *author)
{
  if (__plgn_author)
    free (__plgn_author);

  __plgn_author = strdup (author);
}

void
somax_plgn_set_name (char *name)
{
  if (__plgn_name)
    free (__plgn_name);

  __plgn_name = strdup (name);
}

void
somax_plgn_set_licence (char *licence)
{
  if (__plgn_licence)
    free (__plgn_licence);

  __plgn_licence = strdup (licence);
}

void
somax_plgn_set_version (char *version)
{
  if (__plgn_version)
    free (__plgn_version);

  __plgn_version = strdup (version);
}

void
somax_plgn_print_data (void)
{
  if (__plgn_print_data == 1)
    {
      if (__plgn_description)
	fprintf (stdout, "description: %s\n", __plgn_description);

      if (__plgn_author)
	fprintf (stdout, "author: %s\n", __plgn_author);

      if (__plgn_name)
	fprintf (stdout, "name: %s\n", __plgn_name);

      if (__plgn_licence)
	fprintf (stdout, "licence: %s\n", __plgn_licence);

      if (__plgn_version)
	fprintf (stdout, "version: %s\n", __plgn_version);

      exit (0);
    }
}

struct somad_data *
somax_plgn_parser (void)
{
  struct somad_data *data = NULL;

  if (!__plgn_file)
    return NULL;

  if (palinsesto_parser_file (__plgn_file, &data))
    return NULL;

  return data;
}

void
somax_plgn_parser_free (struct somad_data *data)
{
  palinsesto_free (data);
}

void
somax_plgn_free (void)
{
  if (__plgn_description)
    {
      free (__plgn_description);
      __plgn_description = NULL;
    }

  if (__plgn_author)
    {
      free (__plgn_author);
      __plgn_author = NULL;
    }

  if (__plgn_licence)
    {
      free (__plgn_licence);
      __plgn_licence = NULL;
    }

  if (__plgn_version)
    {
      free (__plgn_version);
      __plgn_version = NULL;
    }

  if (__plgn_name)
    {
      free (__plgn_name);
      __plgn_name = NULL;
    }

  if (__plgn_file)
    {
      free (__plgn_file);
      __plgn_file = NULL;
    }
}

char *
somax_plgn_get_file (void)
{
  return __plgn_file;
}

struct somad_data *
somax_palinsesto_parser_file (char *file)
{
  struct somad_data *data = NULL;

  if (palinsesto_parser_file (file, &data))
    return NULL;

  return data;
}

struct somad_data *
somax_palinsesto_parser_buffer (char *file)
{
  struct somad_data *data = NULL;

  if (palinsesto_parser (file, &data))
    return NULL;

  return data;
}

void
somax_palinsesto_parser_free (struct somad_data *data)
{
  palinsesto_free (data);
}

struct somad_spot_data *
somax_spot_parser_file (char *file)
{
  struct somad_spot_data *data = NULL;

  if (spot_parser_file (file, &data))
    return NULL;

  return data;
}

struct somad_spot_data *
somax_spot_parser_buffer (char *file)
{
  struct somad_spot_data *data = NULL;

  if (spot_parser (file, &data))
    return NULL;

  return data;
}

void
somax_spot_parser_free (struct somad_spot_data *data)
{
  spot_free (data);
}

/* Privates... */

GdkColor *
draw_color (void)
{
  return NULL;
}

GdkColor *
draw_color_str (void)
{
  return NULL;
}

struct tm *
get_time (void)
{
  time_t tt = time (NULL);
  return localtime (&tt);
}

/* EOF */
