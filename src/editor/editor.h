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
#include "../maker/maker.h"
#include "../commons/data.h"
#include "../commons/help.h"
#include "../commons/save.h"
#include "../commons/interface.h"
#include "../filechooser/filechooser.h"
#include "../palinsesto/palinsesto.h"
#include "../plugins/plugins.h"
#include "../modules/modules.h"

#ifdef ENABLE_GTKSOURCEVIEW
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagesmanager.h>
#include <gtksourceview/gtksourceiter.h>
#include <gtksourceview/gtksourceprintjob.h>
#endif

#define TYPE_SPOT	0
#define TYPE_PALINSESTO	1

struct editor_data_t
{
  struct editor_data_t *next;

  GtkWidget *label;
  GtkWidget *maker;

  int type;

  struct maker_pl_data_t *pl;
  struct maker_spot_data_t *spot;
};

#define LOCK	1
#define WAIT	2
#define UNLOCK	3

extern char **env;
extern struct editor_element_t editor_element;
extern struct somax_preferences_t *preferences_data;

extern int editor_statusbar_lock;

extern struct editor_data_t *editor_data;
extern GtkWidget *notebook;
extern GtkWidget *undo_widget, *undo_menu, *undo_history;
extern GtkWidget *redo_widget, *redo_menu, *redo_history;
extern struct somad_data *dump_pl;
extern struct somad_spot_data *dump_spot;

extern GtkWidget *connect_label;
extern soma_controller *controller;

/* color.c */
void editor_select_color (GtkWidget *, gpointer);

/* edit.c */
void editor_undo (GtkWidget *, gpointer);
void editor_redo (GtkWidget *, gpointer);
void editor_undo_history (GtkWidget *, gpointer);
void editor_redo_history (GtkWidget *, gpointer);
void editor_delete (GtkWidget *, gpointer);
void editor_cut (GtkWidget *, gpointer);
void editor_copy (GtkWidget *, gpointer);
void editor_paste (GtkWidget *, gpointer);

/* file.c */
gboolean editor_open_file(char *);
void editor_open (GtkWidget *, gpointer);
void editor_export (GtkWidget *, gpointer);
void editor_save_as (GtkWidget *, gpointer);
void editor_save (GtkWidget *, gpointer);
void editor_close (GtkWidget *, gpointer);
void editor_new (GtkWidget *, gpointer);
void editor_new_pl(void);
void editor_new_sp(void);

/* main.c */
int editor_quit (GtkWidget *, GdkEvent *, gpointer);

/* maker.c */
struct editor_data_t *editor_get_data (void);
gint editor_timeout (gpointer);
char *editor_parse_file (char *);
void editor_new_maker_pl (struct maker_pl_data_t *);
void editor_new_maker_spot (struct maker_spot_data_t *);

/* edit.c */
void editor_view(GtkWidget *, gpointer);
void editor_your_view(GtkWidget *, gpointer);
void editor_preferences(GtkWidget *, gpointer);

/* daemon.c */
void daemon_connect(void);

/* EOF */
