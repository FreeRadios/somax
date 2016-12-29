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

#ifndef SAVE_H
#define SAVE_H

struct somax_config {
  char *password;

  int saved;

  char *server;
  int port;
  char *unixpath;

  int type;
#ifdef SOMA_USE_OPENSSL
  int ssl;
#endif

#ifdef SOMAD_CMD
  int start;
  char *command;
#endif
};

void set_data(struct somax_config *);
struct somax_config *get_data(void);
void free_data(struct somax_config *);

struct somax_config_vte {
#ifdef SOMAD_CMD
  char *font;

  int audible_bell;
  int visible_bell;
  int transparent;
#endif
};

void set_vte(struct somax_config_vte *);
struct somax_config_vte *get_vte(void);
void free_vte(struct somax_config_vte *);

struct somax_config_timer {
  char *font;

  GdkColor *first_color;
  int first_seconds;
  
  GdkColor *second_color;
  int second_seconds;

  GdkColor *third_color;
  int third_seconds;

  gboolean decimal;
};

void set_timer(struct somax_config_timer *);
struct somax_config_timer *get_timer(void);
void free_timer(struct somax_config_timer *);

struct somax_preferences_t {
  gint request_infos;
  gboolean info_download;
  gboolean window_size;
  gboolean window_width;
  gboolean window_height;
  gboolean fullscreen;
  gboolean statusbar;
  gboolean lists;
  gboolean item_infos;
  gboolean next_item_infos;
  gboolean timer_pb;
};

void set_preferences(struct somax_preferences_t *);
struct somax_preferences_t *get_preferences(void);
void free_preferences(struct somax_preferences_t *);


#endif

/* EOF */
