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

#ifndef COMMONS_H
#define COMMONS_H

#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <gtk/gtk.h>

#include <libxml/parser.h>

#include <soma/code.h>
#include <soma/commons.h>
#include <soma/controller.h>
#include <soma/local.h>

#ifdef SOMAD_CMD
#  ifndef WIN32
#    include <vte/vte.h>
#  endif
#endif

#ifndef WIN32
#  include <signal.h>
#else
#  define signal( x , y )
#endif

#ifdef ENABLE_CONFUSE
#  include "../confuse/confuse.h"
#else
#  include <confuse.h>
#endif


#define AUTHORS "Andrea Marchesini <bakunin@autistici.org>"
#define COPYRIGHT "copyright GPL 2005"
#define SOMAX_TIMER 2

#define TIME_SIZE 8
#define getrandom(min, max) ((rand() % (int)(((max)+1) - (min))) + (min))

#ifdef ENABLE_NLS
#  include <locale.h>
#  include <libintl.h>
#  define _(string) gettext (string)
#  ifdef gettext_noop
#    define N_(string) gettext_noop (string)
#  else
#    define N_(string) (string)
#  endif
#else
#  define textdomain(string) (string)
#  define gettext(string) (string)
#  define dgettext(domain, message) (message)
#  define dcgettext(domain,message,type) (message)
#  define bindtextdomain(domain,directory) (domain)
#  define _(string) (string)
#  define N_(string) (string)
#endif

extern gint statusbar_id;
extern GtkWidget *statusbar;

/* commons.c */
char *somax_day(int);
char *somax_mini_day(int);
char *somax_month(int);
char *somax_mini_month(int);
void dialog_msg (char *);
int dialog_ask (char *);
int dialog_ask_with_cancel (char *);
int somax_check (soma_controller *);
char * somax_markup (char *text);
char *somax_from_utf8(char *str);
char *somax_to_utf8(char *str);

/* statusbar.c */
void statusbar_set (char *, ...);

/* other */
struct tm *get_time(void);
char **get_env(void);

#endif

/* EOF */
