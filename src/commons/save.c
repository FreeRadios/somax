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
#include "save.h"

static FILE *
set_config_file (void *c, gchar * what)
{
  FILE *fl;
  gchar *s;

  if (!c)
    {
      s =
	g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax",
		      what, NULL);
      unlink (s);
      g_free (s);
      return NULL;
    }

  s = g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax", NULL);
  if (g_file_test (s, G_FILE_TEST_EXISTS) == FALSE)
    {
      g_mkdir (s, 0750);

      if (g_file_test (s, G_FILE_TEST_EXISTS) == FALSE)
	{
	  g_message ("Error mkdir %s", s);
	  g_free (s);
	  return NULL;
	}
    }

  g_free (s);
  s =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax", what, NULL);

  if (!(fl = fopen (s, "w")))
    {
      g_free (s);
      return NULL;
    }

  g_free (s);
  return fl;
}

void
set_data (struct somax_config *c)
{
  FILE *fl;

  if (!(fl = set_config_file (c, "config")))
    return;

  fprintf (fl, "password = \"%s\"\n", c->password ? c->password : "");
  fprintf (fl, "server = \"%s\"\n", c->server ? c->server : "");
  fprintf (fl, "port = %d\n", c->port ? c->port : SOMA_PORT);
  fprintf (fl, "unixpath = \"%s\"\n", c->unixpath ? c->unixpath : "");
  fprintf (fl, "type = %d\n", c->type);
#ifdef SOMA_USE_OPENSSL
  fprintf (fl, "ssl = %d\n", c->ssl);
#endif
#ifdef SOMAD_CMD
  fprintf (fl, "start = %d\n", c->start);
  fprintf (fl, "command = \"%s\"\n", c->command ? c->command : "");
#endif

  fclose (fl);
}

struct somax_config *
get_data (void)
{
  cfg_t *cfg;
  gchar *s;
  char *d;
  struct somax_config *c;

  c = (struct somax_config *) g_malloc0 (sizeof (struct somax_config));

  cfg_opt_t opts[] = {
    CFG_STR ("password", NULL, CFGF_NONE),
    CFG_STR ("server", "localhost", CFGF_NONE),
    CFG_INT ("port", SOMA_PORT, CFGF_NONE),
    CFG_STR ("unixpath", "/tmp/somad.sock", CFGF_NONE),
    CFG_INT ("type", 0, CFGF_NONE),
#ifdef SOMA_USE_OPENSSL
    CFG_INT ("ssl", 0, CFGF_NONE),
#endif
#ifdef SOMAD_CMD
    CFG_INT ("start", 0, CFGF_NONE),
    CFG_STR ("command", NULL, CFGF_NONE),
#endif
    CFG_END ()
  };

  cfg = cfg_init (opts, CFGF_NOCASE);

  s =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax", "config",
		  NULL);

  switch (cfg_parse (cfg, s))
    {
    case CFG_FILE_ERROR:
    case CFG_PARSE_ERROR:
      c->saved = 0;
      c->password = g_strdup ("");
      c->server = g_strdup ("localhost");
      c->port = SOMA_PORT;
      c->unixpath = g_strdup ("/tmp/somad.sock");
      c->type = 0;
#ifdef SOMA_USE_OPENSSL
      c->ssl = 0;
#endif
#ifdef SOMAD_CMD
      c->start = 0;
      c->command = g_strdup (SOMAD_CMD);
#endif
      g_free (s);
      return c;
    }

  g_free (s);
  c->saved = 1;

  if ((d = cfg_getstr (cfg, "password")))
    c->password = g_strdup (d);
  else
    c->password = g_strdup ("");

  if ((d = cfg_getstr (cfg, "server")))
    c->server = g_strdup (d);
  else
    c->server = g_strdup ("");

  c->port = cfg_getint (cfg, "port");

  if ((d = cfg_getstr (cfg, "unixpath")))
    c->unixpath = g_strdup (d);
  else
    c->unixpath = g_strdup ("");

  c->type = cfg_getint (cfg, "type");
#ifdef SOMA_USE_OPENSSL
  c->ssl = cfg_getint (cfg, "ssl");
#endif

#ifdef SOMAD_CMD
  c->start = cfg_getint (cfg, "start");

  if ((d = cfg_getstr (cfg, "command")))
    c->command = g_strdup (d);
  else
    c->command = g_strdup (SOMAD_CMD);
#endif

  cfg_free (cfg);
  return c;
}

void
free_data (struct somax_config *c)
{
  if (!c)
    return;

  if (c->password)
    g_free (c->password);

  if (c->server)
    g_free (c->server);

  if (c->unixpath)
    g_free (c->unixpath);

#ifdef SOMAD_CMD
  if (c->command)
    g_free (c->command);
#endif

  g_free (c);
}

void
set_vte (struct somax_config_vte *c)
{
  FILE *fl;

  if (!(fl = set_config_file (c, "vte")))
    return;

#ifdef SOMAD_CMD
  fprintf (fl, "font = \"%s\"\n", c->font ? c->font : "");
  fprintf (fl, "audible_bell = \"%s\"\n", c->audible_bell ? "true" : "false");
  fprintf (fl, "visible_bell = \"%s\"\n", c->visible_bell ? "true" : "false");
  fprintf (fl, "transparent = \"%s\"\n", c->transparent ? "true" : "false");
#endif

  fclose (fl);
}

struct somax_config_vte *
get_vte (void)
{
  cfg_t *cfg;
  gchar *s;
  char *d;
  struct somax_config_vte *c;

  c =
    (struct somax_config_vte *) g_malloc0 (sizeof (struct somax_config_vte));

  cfg_opt_t opts[] = {
#ifdef SOMAD_CMD
    CFG_STR ("font", NULL, CFGF_NONE),
    CFG_BOOL ("audible_bell", cfg_true, CFGF_NONE),
    CFG_BOOL ("visible_bell", cfg_false, CFGF_NONE),
    CFG_BOOL ("transparent", cfg_false, CFGF_NONE),
#endif
    CFG_END ()
  };

  cfg = cfg_init (opts, CFGF_NOCASE);

  s =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax", "vte",
		  NULL);

  switch (cfg_parse (cfg, s))
    {
    case CFG_FILE_ERROR:
    case CFG_PARSE_ERROR:
#ifdef SOMAD_CMD
      c->font = NULL;
      c->audible_bell = 0;
      c->visible_bell = 0;
      c->transparent = 0;
#endif
      g_free (s);
      return c;
    }

  g_free (s);

#ifdef SOMAD_CMD
  if ((d = cfg_getstr (cfg, "font")))
    c->font = g_strdup (d);
  else
    c->font = NULL;

  c->audible_bell = cfg_getbool (cfg, "audible_bell");
  c->visible_bell = cfg_getbool (cfg, "visible_bell");
  c->transparent = cfg_getbool (cfg, "transparent");
#endif

  cfg_free (cfg);
  return c;
}

void
free_vte (struct somax_config_vte *c)
{
#ifdef SOMAD_CMD
  if (!c)
    return;

  if (c->font)
    g_free (c->font);

  g_free (c);
#endif
}

void
set_timer (struct somax_config_timer *c)
{
  FILE *fl;

  if (!(fl = set_config_file (c, "timer")))
    return;

  fprintf (fl, "font = \"%s\"\n", c->font ? c->font : "");

  if (c->first_color)
    fprintf (fl, "first_color = \"#%.2X%.2X%.2X\"\n",
	     c->first_color->red >> 8, c->first_color->green >> 8,
	     c->first_color->blue >> 8);

  fprintf (fl, "first_seconds = %d\n", c->first_seconds);

  if (c->second_color)
    fprintf (fl, "second_color = \"#%.2X%.2X%.2X\"\n",
	     c->second_color->red >> 8, c->second_color->green >> 8,
	     c->second_color->blue >> 8);

  fprintf (fl, "second_seconds = %d\n", c->second_seconds);

  if (c->third_color)
    fprintf (fl, "third_color = \"#%.2X%.2X%.2X\"\n",
	     c->third_color->red >> 8, c->third_color->green >> 8,
	     c->third_color->blue >> 8);

  fprintf (fl, "third_seconds = %d\n", c->third_seconds);
  fprintf (fl, "decimal = %s\n", c->decimal == TRUE ? "true" : "false");

  fclose (fl);
}

struct somax_config_timer *
get_timer (void)
{
  cfg_t *cfg;
  gchar *s;
  char *d;
  struct somax_config_timer *c;

  c =
    (struct somax_config_timer *)
    g_malloc0 (sizeof (struct somax_config_timer));

  cfg_opt_t opts[] = {
    CFG_STR ("font", NULL, CFGF_NONE),

    CFG_STR ("first_color", NULL, CFGF_NONE),
    CFG_INT ("first_seconds", 0, CFGF_NONE),

    CFG_STR ("second_color", NULL, CFGF_NONE),
    CFG_INT ("second_seconds", 0, CFGF_NONE),

    CFG_STR ("third_color", NULL, CFGF_NONE),
    CFG_INT ("third_seconds", 0, CFGF_NONE),

    CFG_BOOL ("decimal", cfg_true, CFGF_NONE),

    CFG_END ()
  };

  cfg = cfg_init (opts, CFGF_NOCASE);

  s =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax", "timer",
		  NULL);

  switch (cfg_parse (cfg, s))
    {
    case CFG_FILE_ERROR:
    case CFG_PARSE_ERROR:
      g_free (s);
      return c;
    }

  g_free (s);

  if ((d = cfg_getstr (cfg, "font")) && *d)
    c->font = g_strdup (d);
  else
    c->font = NULL;

  if ((d = cfg_getstr (cfg, "first_color")) && *d)
    {
      c->first_color = g_malloc (sizeof (GdkColor));
      gdk_color_parse (d, c->first_color);
    }

  c->first_seconds = cfg_getint (cfg, "first_seconds");

  if ((d = cfg_getstr (cfg, "second_color")) && *d)
    {
      c->second_color = g_malloc (sizeof (GdkColor));
      gdk_color_parse (d, c->second_color);
    }

  c->second_seconds = cfg_getint (cfg, "second_seconds");

  if ((d = cfg_getstr (cfg, "third_color")) && *d)
    {
      c->third_color = g_malloc (sizeof (GdkColor));
      gdk_color_parse (d, c->third_color);
    }

  c->third_seconds = cfg_getint (cfg, "third_seconds");

  c->decimal = cfg_getbool (cfg, "decimal");

  cfg_free (cfg);
  return c;
}

void
free_timer (struct somax_config_timer *c)
{
#ifdef SOMAD_CMD
  if (!c)
    return;

  if (c->font)
    g_free (c->font);

  if (c->first_color)
    g_free (c->first_color);

  if (c->second_color)
    g_free (c->second_color);

  if (c->third_color)
    g_free (c->third_color);

  g_free (c);
#endif
}


void
set_preferences (struct somax_preferences_t *c)
{
  FILE *fl;

  if (!(fl = set_config_file (c, "preferences")))
    return;

  fprintf (fl, "request_infos = %d\n", c->request_infos);
  fprintf (fl, "info_download = %s\n",
	   c->info_download == TRUE ? "true" : "false");
  fprintf (fl, "window_size = %s\n",
	   c->window_size == TRUE ? "true" : "false");

  if (c->window_size == TRUE)
    {
      fprintf (fl, "window_width = %d\n", c->window_width);
      fprintf (fl, "window_height = %d\n", c->window_height);
    }

  fprintf (fl, "fullscreen = %s\n", c->fullscreen == TRUE ? "true" : "false");
  fprintf (fl, "statusbar = %s\n", c->statusbar == TRUE ? "true" : "false");
  fprintf (fl, "lists = %s\n", c->lists == TRUE ? "true" : "false");
  fprintf (fl, "item_infos = %s\n",
	   c->next_item_infos == TRUE ? "true" : "false");
  fprintf (fl, "next_item_infos = %s\n",
	   c->next_item_infos == TRUE ? "true" : "false");
  fprintf (fl, "timer_pb = %s\n", c->timer_pb == TRUE ? "true" : "false");

  fclose (fl);
}

struct somax_preferences_t *
get_preferences (void)
{
  cfg_t *cfg;
  gchar *s;
  struct somax_preferences_t *c;

  c =
    (struct somax_preferences_t *)
    g_malloc0 (sizeof (struct somax_preferences_t));

  cfg_opt_t opts[] = {
    CFG_INT ("request_infos", 2, CFGF_NONE),
    CFG_BOOL ("info_download", cfg_false, CFGF_NONE),
    CFG_BOOL ("window_size", cfg_true, CFGF_NONE),
    CFG_INT ("window_width", 0, CFGF_NONE),
    CFG_INT ("window_height", 0, CFGF_NONE),
    CFG_BOOL ("fullscreen", cfg_false, CFGF_NONE),
    CFG_BOOL ("statusbar", cfg_true, CFGF_NONE),
    CFG_BOOL ("lists", cfg_true, CFGF_NONE),
    CFG_BOOL ("item_infos", cfg_true, CFGF_NONE),
    CFG_BOOL ("next_item_infos", cfg_true, CFGF_NONE),
    CFG_BOOL ("timer_pb", cfg_true, CFGF_NONE),

    CFG_END ()
  };

  cfg = cfg_init (opts, CFGF_NOCASE);

  s =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax",
		  "preferences", NULL);

  switch (cfg_parse (cfg, s))
    {
    case CFG_FILE_ERROR:
    case CFG_PARSE_ERROR:
      g_free (s);

      c->request_infos = cfg_getint (cfg, "request_infos");
      c->info_download = cfg_getbool (cfg, "info_download");
      c->window_size = cfg_getbool (cfg, "window_size");
      c->window_width = cfg_getint (cfg, "window_width");
      c->window_height = cfg_getint (cfg, "window_height");
      c->fullscreen = cfg_getbool (cfg, "fullscreen");
      c->statusbar = cfg_getbool (cfg, "statusbar");
      c->lists = cfg_getbool (cfg, "lists");
      c->item_infos = cfg_getbool (cfg, "next_item_infos");
      c->next_item_infos = cfg_getbool (cfg, "next_item_infos");
      c->timer_pb = cfg_getbool (cfg, "timer_pb");
      return c;
    }

  g_free (s);

  c->request_infos = cfg_getint (cfg, "request_infos");
  c->info_download = cfg_getbool (cfg, "info_download");
  c->window_size = cfg_getbool (cfg, "window_size");
  c->window_width = cfg_getint (cfg, "window_width");
  c->window_height = cfg_getint (cfg, "window_height");
  c->fullscreen = cfg_getbool (cfg, "fullscreen");
  c->statusbar = cfg_getbool (cfg, "statusbar");
  c->lists = cfg_getbool (cfg, "lists");
  c->item_infos = cfg_getbool (cfg, "next_item_infos");
  c->next_item_infos = cfg_getbool (cfg, "next_item_infos");
  c->timer_pb = cfg_getbool (cfg, "timer_pb");

  cfg_free (cfg);
  return c;
}

void
free_preferences (struct somax_preferences_t *c)
{
  if (!c)
    return;

  g_free (c);
}

/* EOF */
