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

#ifndef MAKER_H
#define MAKER_H

#define MAKER_UNDOREDO_MAX_TIMER	2000
#define MAKER_UNDOREDO_MAX_UNDO		20

struct module_item {
  GtkWidget *widget;
  GtkWidget *widget_container;

  struct module_t *module;
};

struct maker_pl_data_t
{
  struct somad_data *pl;
  struct somad_data *maker_pl;

  GtkWidget *frame;
  GtkWidget *calendar;

  GtkWidget *box_files;
  GtkWidget *ratioitem;
  GtkWidget *ratiospot;
  GtkWidget *randomitem;
  GtkWidget *randomspot;
  GtkWidget *pathitem;
  GtkWidget *pathspot;

  GtkWidget *box_stream;
  GtkWidget *stream;

  GtkWidget *box_module;
  GtkWidget *module;
  GtkWidget *moduledata;

  GtkWidget *box_silence;

  GtkWidget *box_empty;

  GtkWidget *description;

  GtkWidget *start_year;
  GtkWidget *start_month;
  GtkWidget *start_mday;
  GtkWidget *start_wday;
  GtkWidget *start_hour;
  GtkWidget *start_min;

  GtkWidget *stop_year;
  GtkWidget *stop_month;
  GtkWidget *stop_mday;
  GtkWidget *stop_wday;
  GtkWidget *stop_hour;
  GtkWidget *stop_min;

  GtkWidget *timecontinued;
  GtkWidget *priority;
  GtkWidget *spotcontroller;
  GtkWidget *softstop;

  GtkWidget *jingle;
  GtkWidget *prespot;
  GtkWidget *postspot;

  GtkWidget *type;

  GtkWidget *draw;

  char *maker_pl_file;

  GtkWidget *frame_list;
  GtkWidget *frame_list_w;

  GtkWidget *duration_pathitem;
  GtkWidget *duration_pathspot;

  int is_saved;
  gboolean refresh_lock;

  soma_controller *controller;
  gboolean local;

  GList *modules_widget;
  GList *controller_widgets;

  GtkWidget *undoredo_current_widget;
  gchar *undoredo_current_value;
  GTimer *undoredo_timer;
  guint undoredo_timeout;
  GList *undoredo_undo_list;
  GList *undoredo_redo_list;
};

struct maker_spot_data_t
{
  struct somad_spot_data *spot;
  struct somad_spot_data *maker_spot;

  GtkWidget *frame;

  GtkWidget *box_files;
  GtkWidget *path;

  GtkWidget *description;

  GtkWidget *start_year;
  GtkWidget *start_month;
  GtkWidget *start_mday;
  GtkWidget *start_wday;
  GtkWidget *start_hour;
  GtkWidget *start_min;

  GtkWidget *stop_year;
  GtkWidget *stop_month;
  GtkWidget *stop_mday;
  GtkWidget *stop_wday;
  GtkWidget *stop_hour;
  GtkWidget *stop_min;

  GtkWidget *timecontinued;
  GtkWidget *repeat;

  char *maker_spot_file;

  GtkWidget *frame_list;
  GtkWidget *frame_list_w;

  GtkWidget *duration_path;

  int is_saved;
  gboolean refresh_lock;

  soma_controller *controller;
  gboolean local;

  GList *controller_widgets;

  GtkWidget *undoredo_current_widget;
  gchar *undoredo_current_value;
  GTimer *undoredo_timer;
  guint undoredo_timeout;
  GList *undoredo_undo_list;
  GList *undoredo_redo_list;
};

enum maker_undoredo_type_t {
  MAKER_UNDOREDO_NEW,
  MAKER_UNDOREDO_DESTROY,
  MAKER_UNDOREDO_CHANGE
};

struct maker_pl_undoredo_t {
  enum maker_undoredo_type_t type;
  struct somad_data *pl;
  struct somad_data *previous_pl;

  gchar *message;
};

struct maker_spot_undoredo_t {
  enum maker_undoredo_type_t type;
  struct somad_spot_data *spot;
  struct somad_spot_data *previous_spot;

  gchar *message;
};

/* palinsesto */
void			maker_pl_data_show	(struct somad_data *,
						 struct maker_pl_data_t *);

gboolean		maker_pl_draw_button_press (GtkWidget *,
						    GdkEventButton *,
						    struct maker_pl_data_t *);

GtkWidget *		create_maker_pl		(struct maker_pl_data_t *, 
						 void *controller,
						 gboolean local_add);

void			maker_pl_sync		(struct maker_pl_data_t *);
void			maker_pl_file_saved	(struct maker_pl_data_t *);
int			maker_pl_is_no_saved	(struct maker_pl_data_t *);

void			maker_pl_changed	(GtkWidget *,
						 struct maker_pl_data_t *);

struct somad_data *	maker_pl_dump_pl	(struct maker_pl_data_t *);
void			maker_pl_remove_pl	(struct maker_pl_data_t *);
void			maker_pl_insert_pl	(struct maker_pl_data_t *,
						 struct somad_data *);

void			maker_pl_change_color	(struct maker_pl_data_t *);
void			maker_pl_refresh	(struct maker_pl_data_t *);

void			maker_pl_set_controller	(struct maker_pl_data_t *,
						 void *controller);

/* spot */
GtkWidget *		create_maker_spot	(struct maker_spot_data_t *, 
						 void *controller,
						 gboolean local_add);
void			maker_spot_data_show	(struct somad_spot_data *,
						 struct maker_spot_data_t *);

gboolean		maker_spot_draw_button_press (GtkWidget *,
						      GdkEventButton *,
						      struct maker_pl_data_t *);

void			maker_spot_treeview_r_add (GtkWidget *,
						   GtkWidget *);

void			maker_spot_treeview_r_add_dir (GtkWidget *,
						        GtkWidget *);
void			maker_spot_sync		(struct maker_spot_data_t *);
void			maker_spot_file_saved	(struct maker_spot_data_t *);
int			maker_spot_is_no_saved	(struct maker_spot_data_t *);

void			maker_spot_changed	(GtkWidget *,
						 struct maker_spot_data_t *);

struct somad_spot_data * maker_spot_dump_spot	(struct maker_spot_data_t *);
void			maker_spot_remove_spot	(struct maker_spot_data_t *);
void			maker_spot_insert_spot	(struct maker_spot_data_t *,
						 struct somad_spot_data *);

void			maker_spot_change_color	(struct maker_spot_data_t *);
void			maker_spot_refresh	(struct maker_spot_data_t *);

void			maker_spot_set_controller
						(struct maker_spot_data_t *,
						 void *controller);

void			maker_pl_undoredo_work	(struct maker_pl_data_t *data,
						 gboolean what);

void			maker_spot_undoredo_work(struct maker_spot_data_t *data,
						 gboolean what);

void			maker_pl_undoredo_status
						(struct maker_pl_data_t *,
						 gboolean *undo,
						 gboolean *redo);

void			maker_spot_undoredo_status
						(struct maker_spot_data_t *,
						 gboolean *undo,
						 gboolean *redo);

void			maker_pl_undoredo_history
						(struct maker_pl_data_t *,
						 gboolean what,
						 GtkWidget *w);

void			maker_spot_undoredo_history
						(struct maker_spot_data_t *,
						 gboolean what,
						 GtkWidget *w);

/* Internal functions */
void			maker_treeview_r_add	(GtkWidget *,
						 GtkWidget *);

void			maker_treeview_r_add_dir (GtkWidget *,
						  GtkWidget *);

void			maker_file_r_add	(GtkWidget *,
						 GtkWidget *);

void			maker_controller_check	(void *controller,
						 GtkWidget * w,
						 GList ** list);

void			maker_popup_position	(GtkMenu * menu,
						 gint * xp,
						 gint * yp,
						 gboolean * p,
						 gpointer data);

gboolean		maker_changed_check	(GtkWidget * w);

void			maker_changed_destroy	(GtkWidget * w);

void			maker_changed_set	(GtkWidget * w);

gchar *			maker_message		(GtkWidget * w);

#endif

/* EOF */
