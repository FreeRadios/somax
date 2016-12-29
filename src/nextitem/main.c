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
#include "../commons/save.h"
#include "../commons/interface.h"
#include "../commons/x.h"
#include "../commons/help.h"
#include "../commons/save.h"
#include "nextitem.h"

static soma_controller *controller;
static char **env;
static GtkWidget *image_running;
static GtkWidget *timer_entry;
static int local_connect = 0;

static int quit (GtkWidget *, gpointer);
static int start (gpointer);
static void run_somax (GtkWidget *, gpointer);
static void run_editor (GtkWidget *, gpointer);
static void run_config (GtkWidget *, gpointer);
static void skip (GtkWidget *, gpointer);
static gint timeout (gpointer);
static void ns_help (GtkWidget *, gpointer);

static int
quit (GtkWidget * w, gpointer dummy)
{
  if (controller)
    somax_free (controller);

  gtk_main_quit ();

  return FALSE;
}

static int
start (gpointer dummy)
{
  GtkWidget *w;
  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *alignment;
  GtkWidget *box;
  GtkWidget *hbox;
  GtkWidget *menubar;
  GtkWidget *menuitem;
  GtkWidget *menumenu;
  GtkTooltips *tooltips;
  GtkObject *adj;
  controller = create_login (NULL, &local_connect, NULL);

  char s[1024];

  if (!controller)
    quit (NULL, NULL);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  snprintf (s, sizeof (s), "%s-nextitem %s", PACKAGE, VERSION);
  gtk_window_set_title (GTK_WINDOW (window), s);
  gtk_widget_set_size_request (window, -1, 400);

  g_signal_connect ((gpointer) window, "delete_event", G_CALLBACK (quit),
		    NULL);

  box = gtk_vbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (window), box);

  hbox = gtk_hbox_new (0, FALSE);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  menubar = gtk_menu_bar_new ();
  gtk_widget_show (menubar);
  gtk_box_pack_start (GTK_BOX (hbox), menubar, TRUE, TRUE, 0);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_File"));
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

  menuitem = gtk_image_menu_item_new_with_mnemonic (_("Exec Somax-_Editor"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);

  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (run_editor), NULL);

  image = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  menuitem = gtk_image_menu_item_new_with_mnemonic (_("Exec Somax-_Config"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);

  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (run_config), NULL);

  image = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-quit", NULL);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);

  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (quit), NULL);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_Help"));
  gtk_menu_item_set_right_justified (GTK_MENU_ITEM (menuitem), TRUE);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menubar), menuitem);

  menumenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menumenu);

  menuitem = gtk_image_menu_item_new_with_label (_("About..."));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (ns_help), NULL);

  image = gtk_image_new_from_stock ("gtk-about", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  tooltips = gtk_tooltips_new ();
  adj = gtk_adjustment_new (SOMAX_TIMER, 1, 99, 1, 10, 10);
  timer_entry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_widget_show (timer_entry);
  gtk_tooltips_set_tip (tooltips, timer_entry,
			_("Set timer refresh between somad and this somax"),
			NULL);
  gtk_box_pack_start (GTK_BOX (hbox), timer_entry, FALSE, FALSE, 0);

  image_running =
    gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_show (image_running);
  gtk_box_pack_start (GTK_BOX (hbox), image_running, FALSE, FALSE, 0);

  if (local_connect)
    w = nextitem_new (controller, TRUE);
  else
    w = nextitem_new (controller, FALSE);

  gtk_widget_show (w);
  gtk_box_pack_start (GTK_BOX (box), w, TRUE, TRUE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (skip), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  hbox = gtk_hbox_new (0, FALSE);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (alignment), hbox);

  image = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Skip current item"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (quit), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  hbox = gtk_hbox_new (0, FALSE);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (alignment), hbox);

  image = gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Close somax-nextitem"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  gtk_widget_show (window);

  g_timeout_add (2000, timeout, NULL);

  timeout (NULL);
  return 0;
}

int
main (int argc, char **argv, char **e)
{
#ifdef ENABLE_NLS
  setlocale (LC_ALL, "");
  textdomain (PACKAGE);
  bindtextdomain (PACKAGE, LOCALEDIR);
#endif

  signal (SIGPIPE, SIG_IGN);

  soma_local_init ();

  g_thread_init (NULL);
  gtk_init (&argc, &argv);

  env = e;

  g_timeout_add (1, start, NULL);
  gtk_main ();

  return 0;
}

static void
run_somax (GtkWidget * w, gpointer dummy)
{
  gchar *a[2];
  GPid pid;

  a[0] = X;
  a[1] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

static void
run_editor (GtkWidget * w, gpointer dummy)
{
  gchar *a[2];
  GPid pid;

  a[0] = EDITOR;
  a[1] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

static void
run_config (GtkWidget * w, gpointer dummy)
{
  gchar *a[2];
  GPid pid;

  a[0] = CONFIG;
  a[1] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

static gint
timeout (gpointer dummy)
{
  int ret;
  static int running = -1;
  static int t_refresh = SOMAX_TIMER;
  static char **list = NULL;

  ret = gtk_spin_button_get_value (GTK_SPIN_BUTTON (timer_entry));

  if (ret != t_refresh)
    {
      t_refresh = ret;
      g_timeout_add (ret * 1000, timeout, NULL);
      return FALSE;
    }

  if (!soma_time (controller) && somax_check (controller))
    {
      quit (NULL, NULL);
      return FALSE;
    }

  ret = soma_running (controller);
  if (ret != running)
    {
      if (ret)
	gtk_image_set_from_stock (GTK_IMAGE (image_running), "gtk-yes",
				  GTK_ICON_SIZE_SMALL_TOOLBAR);
      else
	gtk_image_set_from_stock (GTK_IMAGE (image_running), "gtk-no",
				  GTK_ICON_SIZE_SMALL_TOOLBAR);
      running = ret;
    }

  if (list)
    soma_nextitem_list_free (list);

  list = soma_nextitem_list (controller);

  nextitem_timer_refresh (list, soma_nextitem_get_spot (controller));

  return TRUE;
}

static void
skip (GtkWidget * w, gpointer dummy)
{
  somax_skip (controller);
}

static void
ns_help (GtkWidget * w, gpointer dummy)
{
  help_show ();
}

/* EOF */
