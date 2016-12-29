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

#ifndef SOMAX_H
#define SOMAX_H

#include "../commons/commons.h"
#include "../commons/x.h"
#include "../commons/help.h"
#include "../commons/stat.h"
#include "../commons/data.h"
#include "../commons/save.h"
#include "../commons/interface.h"
#include "../maker/maker.h"
#include "../nextitem/nextitem.h"
#include "../plugins/plugins.h"
#include "../modules/modules.h"

extern soma_controller *controller;
extern int local_connect;

extern GtkWidget *main_window;
extern GtkWidget *statusbar_widget;
extern GtkWidget *lists_widget;

extern GtkWidget *winnextitem;
extern GtkWidget *b_startstop;
extern GtkWidget *w_startstop;
extern GtkWidget *b_pause;
extern GtkWidget *w_pause;
extern GtkWidget *p_list;
extern GtkWidget *w_list;
extern GtkWidget *image_running;
extern GtkWidget *timer_pb;
extern GtkWidget *item_infos;
extern GtkWidget *next_item_infos;
extern int status_pause;
extern int status_stop;
extern time_t somad_timestep;
extern struct somad_data *somad_pl;
extern struct somad_spot_data *somad_spot;
extern struct maker_pl_data_t maker_pl_data;
extern struct maker_spot_data_t maker_spot_data;
extern GtkWidget *calendar;
extern GtkWidget *pl_draw;
extern GtkWidget *b_time;
extern GtkWidget *l_timeout;
extern GtkWidget *tlist_item_tv;
extern GtkWidget *tlist_item_label;
extern GtkWidget *tlist_spot_tv;
extern GtkWidget *tlist_spot_label;
extern GtkWidget *current_item;
extern GtkWidget *next_item;
extern GtkWidget *palinsesto_name;
extern GtkWidget *timer_widget;
extern struct stop_for_d stop_for_data;
extern gboolean set_quit;
extern char **env;

extern GtkWidget *current_album;
extern GtkWidget *current_year;
extern GtkWidget *current_genre;

extern GtkWidget *next_album;
extern GtkWidget *next_year;
extern GtkWidget *next_genre;

#ifdef SOMAD_CMD
extern GtkWidget *daemon_widget;
#endif

enum {
  LIST_FILE,
  LIST_STYLE,
  LIST_FILENAME,
  LIST_DURATION,
  LIST_NUMBER
};

extern struct somax_preferences_t *preferences_data;

/* interface.c */
GtkWidget *create_window (void);
gboolean draw_button_press(GtkWidget *, GdkEventButton *, gpointer);

/* main.c */
void fatal (char *);
int login (gpointer);
void start (void);
int quit (GtkWidget *, gpointer);

/* timeout.c */
void timeout_init(void);
int timeout (gpointer);
void l_timeout_update(struct somad_data *);
gpointer thread_start(gpointer);

/* buttons.c */
void onlypause (GtkWidget *, gpointer);
void pauseskip (GtkWidget *, gpointer);
void startstop (GtkWidget *, gpointer);
void read_directory (GtkWidget *, gpointer);
void read_palinsesto (GtkWidget *, gpointer);
void old_palinsesto (GtkWidget *, gpointer);
void read_spot (GtkWidget *, gpointer);
void old_spot (GtkWidget *, gpointer);
void stop_for (GtkWidget *, gpointer);
void skip (GtkWidget *, gpointer);
void skip_next (GtkWidget *, gpointer);
void s_shutdown (GtkWidget *, GtkWidget *);
void config_clicked(GtkWidget *, gpointer);
void editthis_clicked(GtkWidget *, gpointer);
void defaultthis_clicked(GtkWidget *, gpointer);
void editor_clicked(GtkWidget *, gpointer);
void spoteditthis_clicked(GtkWidget *, gpointer);
void spotdefaultthis_clicked(GtkWidget *, gpointer);
void spoteditor_clicked(GtkWidget *, gpointer);
void nextitem_clicked(GtkWidget *, gpointer);
void savepl_clicked(GtkWidget *, gpointer);
void savepl_old_clicked(GtkWidget *, gpointer);
void savespot_clicked(GtkWidget *, gpointer);
void savespot_old_clicked(GtkWidget *, gpointer);
void button_current_item (char *str);
void button_current_item_selected (GtkWidget *, gpointer);
void button_next_item (char *str);
void button_next_item_selected (GtkWidget *, gpointer);
void button_palinsesto_name (char *str);
void button_palinsesto_name_selected (GtkWidget *, gpointer);

/* time.c */
int check_time (char *);
void time_refresh(void);
void time_clicked(GtkWidget *, gpointer);
void time_update_widget (time_t, char *);

/* list.c */
void list_refresh(GtkWidget *, gpointer);
void list_today(GtkWidget *, gpointer);
void list_show(GtkWidget *, struct somad_data *);
void list_show_list(GtkWidget *, GList *);

/* tlist.c */
void tlist_new(char **list, gboolean);
void tlist_refresh(gboolean);
void tlist_selected (GtkTreeView *tree, GtkTreePath *path, 
		    GtkTreeViewColumn *column);
gint tlist_popup_menu (GtkWidget * tv, GdkEventButton * event, gpointer dummy);

/* description.c */
void description_refresh(GList *);

/* tooltips.c */
int tooltip_timer (gpointer);
void tooltip_hide(void);
void tooltip_time_refresh(void);

/* change.c */
void change_palinsesto(GtkWidget *w, gpointer);
void change_spot(GtkWidget *w, gpointer);

/* nextitem.c */
void win_nextitem_show(GtkWidget *, gpointer);
gboolean win_nextitem_window_showed(void);
void win_nextitem_add_stream(GtkWidget *, gpointer);
void win_nextitem_add_l_dir(GtkWidget *, gpointer);
void win_nextitem_add_l_file(GtkWidget *, gpointer);
void win_nextitem_add_r_dir(GtkWidget *, gpointer);
void win_nextitem_add_r_file(GtkWidget *, gpointer);

/* maker.c */
void win_maker_pl_create(void);
void win_maker_spot_create(void);

/* daemon.c */
#ifdef SOMAD_CMD
gboolean daemon_run(gchar *);
GtkWidget *daemon_menu (GtkWidget * vte, GtkAccelGroup *);
#endif

/* preferences.c */
void preferences(void);

#endif

/* EOF */
