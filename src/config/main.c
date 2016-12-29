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

char **env;

static void start (struct config_data *data);

static GtkTreeModel *config_get_users (void);
static GtkTreeModel *config_get_groups (void);
static GtkTreeModel *config_get_servernames (void);

static int config_quit (GtkWidget *, struct config_data *);

static void config_changed (GtkWidget *, struct config_data *);

static void config_about (GtkWidget *, gpointer);

static void config_treeview_remove (GtkWidget *, GtkWidget *);
static void config_treeview_add (GtkWidget *, GtkWidget *);
static void config_treeview_add_dir (GtkWidget *, GtkWidget *);
static void config_treeview_host_add (GtkWidget *, GtkWidget *);
static void config_treeview_add_cb (void *l, void *t);
static void config_treeview_add_dir_cb (void *l, void *t);

static void config_entry_add (GtkWidget *, GtkWidget *);
static void config_entry_add_dir (GtkWidget *, GtkWidget *);

static void
start (struct config_data *data)
{
  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *image;
  GtkWidget *box;
  GtkWidget *hbox;
  GtkWidget *menubar;
  GtkWidget *menuitem;
  GtkWidget *menumenu;
  GtkWidget *notebook;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *scrolledwindow;
  GtkEntryCompletion *completion;
  GtkTreeModel *completion_model;
  GtkObject *adj;
  GtkListStore *model;
  GtkTreeSelection *selection;
  GtkCellRenderer *renderer;
  GtkAccelGroup *accel;

  gint i = 0;
  gchar s[1024];

  accel = gtk_accel_group_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  snprintf (s, sizeof (s), "%s-config %s", PACKAGE, VERSION);
  gtk_window_set_title (GTK_WINDOW (window), s);

  g_signal_connect ((gpointer) window, "delete_event",
		    G_CALLBACK (config_quit), data);

  box = gtk_vbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (window), box);

  menubar = gtk_menu_bar_new ();
  gtk_widget_show (menubar);
  gtk_box_pack_start (GTK_BOX (box), menubar, FALSE, FALSE, 0);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_File"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menubar), menuitem);

  menumenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menumenu);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-new", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (config_new), data);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-open", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (config_open), data);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-save", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (config_save), data);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-save-as", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (config_save_as), data);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-quit", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (config_quit), data);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_Tools"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menubar), menuitem);

  menumenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menumenu);

  menuitem = gtk_image_menu_item_new_with_mnemonic (_("Exec Soma_X"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (run_somax), NULL);

  image = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  menuitem =
    gtk_image_menu_item_new_with_mnemonic (_("Exec Somax-_NextItem"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (run_nextitem), NULL);

  image = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  menuitem = gtk_image_menu_item_new_with_mnemonic (_("Exec Somax-_Editor"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (run_editor), NULL);

  image = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_Help"));
  gtk_menu_item_set_right_justified (GTK_MENU_ITEM (menuitem), TRUE);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menubar), menuitem);

  menumenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menumenu);

  menuitem = gtk_image_menu_item_new_with_mnemonic (_("About..."));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (config_about), NULL);

  image = gtk_image_new_from_stock ("gtk-about", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_box_pack_start (GTK_BOX (box), notebook, TRUE, TRUE, 0);

  /** FIRST PAGE **/
  label = gtk_label_new (_("General"));
  gtk_widget_show (label);

  table = gtk_table_new (0, 0, FALSE);
  gtk_widget_show (table);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, label);

  /** USER ******************************************************************/
  LABEL_NEW (_("Run as user"));

  entry = gtk_entry_new ();
  completion = gtk_entry_completion_new ();
  gtk_entry_set_completion (GTK_ENTRY (entry), completion);
  if ((completion_model = config_get_users ()))
    {
      gtk_entry_completion_set_model (completion, completion_model);
      g_object_unref (completion_model);
    }
  gtk_entry_completion_set_text_column (completion, 0);
  g_object_unref (completion);

  DATA_NEW (entry);
  data->user = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** GROUP ******************************************************************/
  LABEL_NEW (_("Run as group"));

  entry = gtk_entry_new ();
  completion = gtk_entry_completion_new ();
  gtk_entry_set_completion (GTK_ENTRY (entry), completion);
  if ((completion_model = config_get_groups ()))
    {
      gtk_entry_completion_set_model (completion, completion_model);
      g_object_unref (completion_model);
    }
  gtk_entry_completion_set_text_column (completion, 0);
  g_object_unref (completion);

  DATA_NEW (entry);
  data->group = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** BACKGROUND ************************************************************/
  LABEL_NEW (_("Start in background"));

  entry = gtk_check_button_new_with_label ("activated");
  DATA_NEW (entry);
  data->background = entry;
  g_signal_connect ((gpointer) entry, "toggled", G_CALLBACK (config_changed),
		    data);
  i++;

  /** DEBUG *****************************************************************/
  LABEL_NEW (_("Debug Level"));

  entry = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("Disactive"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("Only Error"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry), _("Error and Warning"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry),
			     _("Info, Error and Warning"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (entry),
			     _("Debug (for developers)"));
  gtk_combo_box_set_active (GTK_COMBO_BOX (entry), 3);
  DATA_NEW (entry);
  data->debug = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** LOGFILE ***************************************************************/
  LABEL_NEW (_("File for logging"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->logfile = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add), entry);

  /** PIDFILE ***************************************************************/
  LABEL_NEW (_("File for pid"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->pidfile = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add), entry);

  /** PALINSESTO ***********************************************************/
  LABEL_NEW (_("File of Palinsesto"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->palinsesto = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add), entry);

  /** SPOT *****************************************************************/
  LABEL_NEW (_("File of Spot"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->spot = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add), entry);

  /** XML SYNTAX ***********************************************************/
  LABEL_NEW (_("Xml Syntax"));

  entry = gtk_check_button_new_with_label ("activated");
  DATA_NEW (entry);
  data->xmlsyntax = entry;
  g_signal_connect ((gpointer) entry, "toggled", G_CALLBACK (config_changed),
		    data);
  i++;

  /** SYMLINKS **************************************************************/
  LABEL_NEW (_("Symbolic Links"));

  entry = gtk_check_button_new_with_label ("activated");
  DATA_NEW (entry);
  data->symlinks = entry;
  g_signal_connect ((gpointer) entry, "toggled", G_CALLBACK (config_changed),
		    data);
  i++;

  /** PATHMODULES ***********************************************************/
  LABEL_NEW (_("Path Modules"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->pathmodules = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add_dir), entry);

  /** SECOND PAGE **/
  label = gtk_label_new (_("Admin interface"));
  gtk_widget_show (label);

  table = gtk_table_new (0, 0, FALSE);
  gtk_widget_show (table);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, label);

  /** UNIX SOCKET ***********************************************************/
  LABEL_NEW (_("Use Unix Socket"));

  entry = gtk_check_button_new_with_label ("activated");
  DATA_NEW (entry);
  data->unixsocket = entry;
  g_signal_connect ((gpointer) entry, "toggled", G_CALLBACK (config_changed),
		    data);
  i++;

  /** UNIX PATH *************************************************************/
  LABEL_NEW (_("Unix Socket Path"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->unixpath = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add), entry);

  /** SERVERNAME ************************************************************/
  LABEL_NEW (_("Server Binding"));

  entry = gtk_entry_new ();
  completion = gtk_entry_completion_new ();
  gtk_entry_set_completion (GTK_ENTRY (entry), completion);
  if ((completion_model = config_get_servernames ()))
    {
      gtk_entry_completion_set_model (completion, completion_model);
      g_object_unref (completion_model);
    }
  gtk_entry_completion_set_text_column (completion, 0);
  g_object_unref (completion);

  DATA_NEW (entry);
  data->servername = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** PORT ******************************************************************/
  LABEL_NEW (_("Port of admin"));

  adj = gtk_adjustment_new (SOMA_PORT, 0, 65536, 1, 10, 100);
  entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  DATA_NEW (entry);
  data->port = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** LISTEN ****************************************************************/
  LABEL_NEW (_("Number of Listens"));

  adj = gtk_adjustment_new (SOMA_LISTEN, 0, 1000, 1, 10, 100);
  entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  DATA_NEW (entry);
  data->listen = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** PASSWORD **************************************************************/
  LABEL_NEW (_("Password"));

  entry = gtk_entry_new ();
  gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);
  DATA_NEW (entry);
  data->password = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** PASSWORD2 *************************************************************/
  LABEL_NEW (_("(Re)Password"));

  entry = gtk_entry_new ();
  gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);
  DATA_NEW (entry);
  data->password2 = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** SSL ENABLE ************************************************************/
  LABEL_NEW (_("Use SSL"));

  entry = gtk_check_button_new_with_label ("activated");
  DATA_NEW (entry);
  data->ssl = entry;
  g_signal_connect ((gpointer) entry, "toggled", G_CALLBACK (config_changed),
		    data);
  i++;

  /** SSL CERTIFICATE *******************************************************/
  LABEL_NEW (_("SSL Certificate"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->certificate = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add), entry);

  /** SSL PRIVATE KEY *******************************************************/
  LABEL_NEW (_("SSL Private Key"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->privatekey = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add), entry);

  /** THIRD PAGE **/
  label = gtk_label_new (_("Items/Spots"));
  gtk_widget_show (label);

  table = gtk_table_new (0, 0, FALSE);
  gtk_widget_show (table);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, label);

  /** PATHITEM **************************************************************/
  TREEVIEW_NEW (_("PathItems"));
  data->pathitem = entry;

  /** PATHSPOT **************************************************************/
  TREEVIEW_NEW (_("PathSpot"));
  data->pathspot = entry;

  /** 4TH PAGE **/
  label = gtk_label_new (_("Player"));
  gtk_widget_show (label);

  table = gtk_table_new (0, 0, FALSE);
  gtk_widget_show (table);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, label);

  /** PROGRAMITEM ***********************************************************/
  LABEL_NEW (_("Default program"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->programitem = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add), entry);

  /** OPTIONSITEM ***********************************************************/
  LABEL_NEW (_("Options of default program"));

  entry = gtk_entry_new ();
  DATA_NEW (entry);
  data->optionsitem = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** PROGRAMSTREAM *********************************************************/
  LABEL_NEW (_("Stream program"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  DATA_NEW (hbox);
  i++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  data->programstream = entry;
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);

  button = gtk_button_new_with_label (_("Search"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_entry_add), entry);

  /** OPTIONSSTREAM ********************************************************/
  LABEL_NEW (_("Options of stream program"));

  entry = gtk_entry_new ();
  DATA_NEW (entry);
  data->optionsstream = entry;
  g_signal_connect ((gpointer) entry, "changed", G_CALLBACK (config_changed),
		    data);
  i++;

  /** 5TH PAGE **/
  label = gtk_label_new (_("Distribuited FS"));
  gtk_widget_show (label);

  table = gtk_table_new (0, 0, FALSE);
  gtk_widget_show (table);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, label);

  /** DISTRIBUITEDFS *******************************************************/
  LABEL_NEW (_("Distribuited FileSystem"));

  entry = gtk_check_button_new_with_label (_("activated"));
  DATA_NEW (entry);
  data->distribuitedfs = entry;
  g_signal_connect ((gpointer) entry, "toggled", G_CALLBACK (config_changed),
		    data);
  i++;

  /** DISTRIBUITEDPATH *****************************************************/
  TREEVIEW_NEW (_("Distribuited Paths"));
  data->distribuitedpath = entry;

  /** 6TH PAGE **/
  label = gtk_label_new (_("AllowDeny Hosts"));
  gtk_widget_show (label);

  table = gtk_table_new (0, 0, FALSE);
  gtk_widget_show (table);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, label);

  /** ALLOW HOSTS **********************************************************/
  TREEVIEW_DA_NEW (_("Allow Hosts"));
  data->hostallow = entry;

  /** DENY HOSTS **********************************************************/
  TREEVIEW_DA_NEW (_("Deny Hosts"));
  data->hostdeny = entry;

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock ("gtk-new");
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (config_new),
		    data);

  button = gtk_button_new_from_stock ("gtk-open");
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (config_open),
		    data);

  button = gtk_button_new_from_stock ("gtk-save");
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (config_save),
		    data);

  button = gtk_button_new_from_stock ("gtk-save-as");
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (config_save_as), data);

  button = gtk_button_new_from_stock ("gtk-quit");
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (config_quit),
		    data);

  gtk_window_add_accel_group (GTK_WINDOW (window), accel);
  gtk_widget_show (window);
}

int
main (int argc, char **argv, char **e)
{
  struct config_data *data;

#ifdef ENABLE_NLS
  setlocale (LC_ALL, "");
  textdomain (PACKAGE);
  bindtextdomain (PACKAGE, LOCALEDIR);
#endif

  g_thread_init (NULL);
  gtk_init (NULL, NULL);

  env = e;
  data = (struct config_data *) g_malloc0 (sizeof (struct config_data));

  start (data);

  if (argc > 1 && config_open_real (argv[1], data) == FALSE)
    {
      char s[1024];
      snprintf (s, sizeof (s), _("Error opening file: %s"), argv[1]);
      dialog_msg (s);
    }

  else
    {
      if (g_file_test ("/etc/somad/soma.cfg", G_FILE_TEST_EXISTS) == TRUE
	  && config_open_real ("/etc/somad/soma.cfg", data) == FALSE)
	dialog_msg (_("Error opening file: /etc/somad/soma.cfg"));
    }

  gtk_main ();

  return 0;
}

static GtkTreeModel *
config_get_users (void)
{
  GtkListStore *store;
  GtkTreeIter iter;
  struct passwd *pw;

  if (!(pw = getpwent ()))
    return NULL;

  store = gtk_list_store_new (1, G_TYPE_STRING);

  do
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, pw->pw_name, -1);

    }
  while ((pw = getpwent ()));

  endpwent ();

  return GTK_TREE_MODEL (store);
}

static GtkTreeModel *
config_get_groups (void)
{
  GtkListStore *store;
  GtkTreeIter iter;
  struct group *gr;

  if (!(gr = getgrent ()))
    return NULL;

  store = gtk_list_store_new (1, G_TYPE_STRING);

  do
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, gr->gr_name, -1);

    }
  while ((gr = getgrent ()));

  endgrent ();

  return GTK_TREE_MODEL (store);
}

static GtkTreeModel *
config_get_servernames (void)
{
  GtkListStore *store;
  GtkTreeIter iter;

  store = gtk_list_store_new (1, G_TYPE_STRING);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, "127.0.0.1", -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, "0.0.0.0", -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, "localhost", -1);

  return GTK_TREE_MODEL (store);
}

static int
config_quit (GtkWidget * w, struct config_data *data)
{
  gtk_main_quit ();

  if (data->changed)
    {
      switch (dialog_ask (_("Exit without save your data?")))
	{
	case GTK_RESPONSE_OK:
	  config_save (NULL, data);
	  break;
	case GTK_RESPONSE_CANCEL:
	  break;
	default:
	  return TRUE;
	}
    }

  return FALSE;
}

static void
config_changed (GtkWidget * w, struct config_data *data)
{
  data->changed = TRUE;
}

static void
config_treeview_remove (GtkWidget * w, GtkWidget * t)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *list, *b;
  int j, i = 0;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (t));

  list =
    gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection),
					  &model);

  b = list;

  while (list)
    {
      for (j = 0; j < i; j++)
	gtk_tree_path_prev (list->data);

      gtk_tree_model_get_iter (model, &iter, list->data);
      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

      gtk_tree_path_free (list->data);

      i++;
      list = list->next;
    }

  g_list_free (b);

}

static void
config_treeview_host_add (GtkWidget * w, GtkWidget * t)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *host;

  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *entry;
  GtkWidget *button;

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), _("Add Host"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  hbox = gtk_hbox_new (FALSE, 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE,
		      0);

  image = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock ("gtk-cancel");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button,
				GTK_RESPONSE_CANCEL);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  if (gtk_dialog_run (GTK_DIALOG (dialog)))
    {
      host = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));

      if (host && *host && host_check (host) == GTK_RESPONSE_OK)
	{
	  model = gtk_tree_view_get_model (GTK_TREE_VIEW (t));
	  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, host, -1);
	}
    }

  gtk_widget_destroy (dialog);
}

static void
config_treeview_add (GtkWidget * w, GtkWidget * t)
{
  file_chooser_cb (_("Add files"), GTK_SELECTION_MULTIPLE,
		   GTK_FILE_CHOOSER_ACTION_OPEN, config_treeview_add_cb, t);
}

static void
config_treeview_add_cb (void *l, void *t)
{
  GSList *file;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *tmp;

  if (!l || !t)
    return;

  file = (GSList *) l;

  while (file)
    {
      if (g_file_test (file->data, G_FILE_TEST_EXISTS) == TRUE)
	{

	  model = gtk_tree_view_get_model (GTK_TREE_VIEW (t));
	  gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	  tmp = somax_to_utf8 (file->data);
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, tmp, -1);
	  g_free (tmp);
	}

      g_free (file->data);
      file = file->next;
    }

  g_slist_free (l);
}

static void
config_treeview_add_dir (GtkWidget * w, GtkWidget * t)
{
  file_chooser_cb (_("Add directories"), GTK_SELECTION_MULTIPLE,
		   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		   config_treeview_add_dir_cb, t);
}

static void
config_treeview_add_dir_cb (void *l, void *t)
{
  GSList *file;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *tmp;

  if (!l || !t)
    return;

  file = (GSList *) l;

  while (file)
    {
      if (g_file_test (file->data, G_FILE_TEST_EXISTS) == TRUE)
	{
	  model = gtk_tree_view_get_model (GTK_TREE_VIEW (t));
	  gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	  tmp = somax_to_utf8 (file->data);
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, tmp, -1);
	  g_free (tmp);
	}

      g_free (file->data);
      file = file->next;
    }

  g_slist_free (l);
}

static void
config_entry_add (GtkWidget * w, GtkWidget * entry)
{
  gchar *file;
  gchar *tmp;

  if (!
      (file =
       file_chooser (_("Search your file"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_OPEN)))
    return;

  tmp = somax_to_utf8 (file);
  gtk_entry_set_text (GTK_ENTRY (entry), tmp);
  g_free (tmp);

  g_free (file);
}

static void
config_entry_add_dir (GtkWidget * w, GtkWidget * entry)
{
  gchar *file;
  gchar *tmp;

  if (!
      (file =
       file_chooser (_("Search your directory"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER)))
    return;

  tmp = somax_to_utf8 (file);
  gtk_entry_set_text (GTK_ENTRY (entry), tmp);
  g_free (tmp);

  g_free (file);
}

static void
config_about (GtkWidget * w, gpointer dummy)
{
  help_show ();
}

char *
old_markup (char *text)
{
  char *ret;
  int len;
  int i, j;
  char *tmp;

  if (!text)
    return g_strdup ("");

  if (!(tmp = g_locale_from_utf8 (text, -1, NULL, NULL, NULL)))
    tmp = g_strdup (text);

  len = strlen (tmp);
  ret = (char *) g_malloc (sizeof (char) * ((len * 2) + 1));

  for (i = j = 0; i < len; i++)
    {
      if (tmp[i] == '\"')
	ret[j++] = '\\';
      ret[j++] = tmp[i];
    }

  ret[j] = 0;
  g_free (tmp);

  return ret;
}

/* EOF */
