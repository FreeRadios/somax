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

#ifndef DATA_H
#define DATA_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <soma/code.h>
#include <soma/commons.h>
#include <soma/controller.h>

typedef enum {
  FILES,
  STREAM,
  MODULE,
  SILENCE
} somad_type;

struct somad_time {
  char *start;
  char *stop;
  int timecontinued;

  int start_min;
  int start_hour;
  int start_year;
  int start_month;
  int start_mday;
  int start_wday;

  int stop_min;
  int stop_hour;
  int stop_year;
  int stop_month;
  int stop_mday;
  int stop_wday;

};

struct somad_spot_data {
  char *description;

  struct somad_time *timer;

  int repeat;

  GList *path;

  struct somad_spot_data *next;
};

struct somad_data {
  char *description;

  int priority;

  int spotcontroller;

  struct somad_time *timer;

  somad_type type;
  char *module;
  char *moduledata;
  int randomitem;
  int randomspot;
  int softstop;
  int ratioitem;
  int ratiospot;
  GList *pathitem;
  GList *pathspot;
  char *stream;
  char *jingle;
  char *prespot;
  char *postspot;

  GdkColor *color;
  int x1;
  int x2;
  int y;

  struct somad_data *next;
};

struct stop_for_d {
  GtkWidget *hours;
  GtkWidget *minutes;
  GtkWidget *seconds;
};

#endif

/* EOF */
