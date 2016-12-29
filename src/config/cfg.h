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

#include <pwd.h>
#include <grp.h>
#include "../commons/commons.h"
#include "../commons/help.h"
#include "../filechooser/filechooser.h"

#ifdef ENABLE_CONFUSE
#  include "../confuse/confuse.h"
#else
#  include <confuse.h>
#endif

extern char **env;

struct config_data
{
  gboolean changed;
  gchar *file;

  GtkWidget *user;

  GtkWidget *group;

  GtkWidget *background;

  GtkWidget *unixsocket;

  GtkWidget *unixpath;

  GtkWidget *servername;

  GtkWidget *port;

  GtkWidget *listen;

  GtkWidget *password;
  GtkWidget *password2;

  GtkWidget *ssl;

  GtkWidget *certificate;
  GtkWidget *privatekey;

  GtkWidget *debug;

  GtkWidget *logfile;

  GtkWidget *pidfile;

  GtkWidget *pathitem;

  GtkWidget *pathspot;

  GtkWidget *programitem;

  GtkWidget *optionsitem;

  GtkWidget *programstream;

  GtkWidget *optionsstream;

  GtkWidget *palinsesto;

  GtkWidget *xmlsyntax;

  GtkWidget *symlinks;

  GtkWidget *spot;

  GtkWidget *pathmodules;

  GtkWidget *distribuitedfs;

  GtkWidget *distribuitedpath;

  GtkWidget *hostallow;

  GtkWidget *hostdeny;
};

#define LABEL_NEW( x ) label = gtk_label_new (x); \
  gtk_widget_show (label); \
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0); \
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

#define DATA_NEW( x ) gtk_widget_show (x); \
  gtk_table_attach (GTK_TABLE (table), x, 1, 2, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0);

#define TREEVIEW_NEW( x ) \
  scrolledwindow = gtk_scrolled_window_new (NULL, NULL); \
  gtk_widget_show (scrolledwindow); \
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), \
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); \
  gtk_table_attach (GTK_TABLE (table), scrolledwindow, 0, 2, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0); \
  i++; \
  \
  model = gtk_list_store_new (1, G_TYPE_STRING); \
  entry = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model)); \
  gtk_widget_set_size_request (entry, -1, 150); \
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (entry), FALSE); \
  gtk_container_add (GTK_CONTAINER (scrolledwindow), entry); \
  gtk_widget_show (entry); \
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (entry)); \
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE); \
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (entry), TRUE); \
  g_object_unref (model); \
  \
  renderer = gtk_cell_renderer_text_new (); \
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (entry), \
                                               -1, x , renderer, \
                                               "text", 0, NULL); \
  \
  hbox = gtk_hbox_new (TRUE, 0); \
  gtk_widget_show (hbox); \
  gtk_table_attach (GTK_TABLE (table), hbox, 0, 2, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0); \
  i++; \
  \
  button = gtk_button_new_with_label (_("Add Files")); \
  gtk_widget_show (button); \
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0); \
  g_signal_connect ((gpointer) button, "clicked", \
                    G_CALLBACK (config_treeview_add), entry); \
  \
  button = gtk_button_new_with_label (_("Add Directories")); \
  gtk_widget_show (button); \
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0); \
  g_signal_connect ((gpointer) button, "clicked", \
                  G_CALLBACK (config_treeview_add_dir), entry); \
  \
  button = gtk_button_new_with_label (_("Remove Item")); \
  gtk_widget_show (button); \
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0); \
  g_signal_connect ((gpointer) button, "clicked", \
                    G_CALLBACK (config_treeview_remove), entry);

#define TREEVIEW_DA_NEW( x ) \
  scrolledwindow = gtk_scrolled_window_new (NULL, NULL); \
  gtk_widget_show (scrolledwindow); \
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), \
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); \
  gtk_table_attach (GTK_TABLE (table), scrolledwindow, 0, 2, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0); \
  i++; \
  \
  model = gtk_list_store_new (1, G_TYPE_STRING); \
  entry = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model)); \
  gtk_widget_set_size_request (entry, -1, 150); \
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (entry), FALSE); \
  gtk_container_add (GTK_CONTAINER (scrolledwindow), entry); \
  gtk_widget_show (entry); \
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (entry)); \
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE); \
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (entry), TRUE); \
  g_object_unref (model); \
  \
  renderer = gtk_cell_renderer_text_new (); \
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (entry), \
                                               -1, x , renderer, \
                                               "text", 0, NULL); \
  \
  hbox = gtk_hbox_new (TRUE, 0); \
  gtk_widget_show (hbox); \
  gtk_table_attach (GTK_TABLE (table), hbox, 0, 2, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0); \
  i++; \
  \
  button = gtk_button_new_with_label (_("Add Host")); \
  gtk_widget_show (button); \
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0); \
  g_signal_connect ((gpointer) button, "clicked", \
                    G_CALLBACK (config_treeview_host_add), entry); \
  \
  button = gtk_button_new_with_label (_("Remove Host")); \
  gtk_widget_show (button); \
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0); \
  g_signal_connect ((gpointer) button, "clicked", \
                    G_CALLBACK (config_treeview_remove), entry);


void run_somax (GtkWidget *, gpointer);
void run_editor (GtkWidget *, gpointer);
void run_nextitem (GtkWidget *, gpointer);

void config_new (GtkWidget *, struct config_data *);

void config_open (GtkWidget *, struct config_data *);
gboolean config_open_real (gchar *, struct config_data *);

void config_save_as (GtkWidget *, struct config_data *);
void config_save (GtkWidget *, struct config_data *);

gint host_check(char *ip_str);

char *old_markup(char *str);

/* EOF */
