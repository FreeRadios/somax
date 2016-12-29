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

#include "somax.h"
#include "../draw/draw.h"
#include "../timer/timer.h"
#include "../palinsesto/palinsesto.h"

GtkWidget *b_pause;
GtkWidget *w_pause;
GtkWidget *b_startstop;
GtkWidget *w_startstop;
GtkWidget *p_list;
GtkWidget *w_list;
GtkWidget *image_running;
GtkWidget *item_infos;
GtkWidget *next_item_infos;
GtkWidget *timer_pb;
GtkWidget *calendar;
GtkWidget *pl_draw;
GtkWidget *b_time;
GtkWidget *l_timeout;
GtkWidget *tlist_item_tv;
GtkWidget *tlist_item_label;
GtkWidget *tlist_spot_tv;
GtkWidget *tlist_spot_label;
GtkWidget *current_item;
GtkWidget *next_item;
GtkWidget *palinsesto_name = NULL;
GtkWidget *timer_widget;
struct stop_for_d stop_for_data;

GtkWidget *statusbar_widget;
GtkWidget *lists_widget;

GtkWidget *current_album;
GtkWidget *current_year;
GtkWidget *current_genre;

GtkWidget *next_album;
GtkWidget *next_year;
GtkWidget *next_genre;

static void x_plugin_starter_p (GtkWidget *, struct plugin_t *);
static void x_help (GtkWidget *, gpointer);
static int daemon_show (void);
static void antitimer_resize (GtkWidget * window, GtkRequisition * req,
			      GtkWidget * paned);

gboolean
draw_button_press (GtkWidget * widget, GdkEventButton * event, gpointer dummy)
{
  int x, y;
  GList *list;

  if (!event->type == GDK_BUTTON_PRESS)
    return FALSE;

  gtk_widget_get_pointer (widget, &x, &y);
  if (x < 0 || y < 0
      || gtk_window_is_active (GTK_WINDOW (gtk_widget_get_toplevel (widget)))
      == FALSE)
    return FALSE;

  if (!(list = draw_get_xy (pl_draw, x, y, somad_pl)))
    return FALSE;

  list_show_list (NULL, list);
  g_list_free (list);

  return TRUE;
}

GtkWidget *
create_window (void)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *total_paned;
  GtkWidget *nvbox;
  GtkWidget *box;
  GtkWidget *fbox;
  GtkWidget *table;
  GtkWidget *fdbox;
  GtkWidget *menubar;
  GtkWidget *menu_menu;
  GtkWidget *menu_item;
  GtkWidget *scrolledwindow;
  GtkWidget *button;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *alignment;
  GtkWidget *sp;
  GtkObject *adj;
  GtkWidget *frame;
  GtkWidget *paned;
  GtkWidget *w;
  GtkTooltips *tooltips;
  GtkTreeStore *model;
  GtkTreeSelection *selection;
  GtkCellRenderer *renderer;
  GtkWidget *textview;
  gint col;

  GtkAccelGroup *accel;

  int x, y;

  char s[1024];

  snprintf (s, sizeof (s), "%s %s", PACKAGE, VERSION);

  tooltips = gtk_tooltips_new ();
  accel = gtk_accel_group_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), s);
  gtk_window_set_default_icon_from_file (PATH_ICON, NULL);

  if (preferences_data->fullscreen == TRUE)
    gtk_window_fullscreen (GTK_WINDOW (window));
  else if (preferences_data->window_size == TRUE
	   && preferences_data->window_width
	   && preferences_data->window_height)
    gtk_window_resize (GTK_WINDOW (window), preferences_data->window_width,
		       preferences_data->window_height);
  else
    {
      x = gdk_screen_width () * 2 / 3;
      if (x < 640)
	x = 640;

      y = gdk_screen_height () * 2 / 3;
      if (y < 480)
	y = 480;

      gtk_window_resize (GTK_WINDOW (window), x, y);
    }

  g_signal_connect ((gpointer) window, "delete_event",
		    G_CALLBACK (quit), NULL);

  /* VBOX - total ********************************************************** */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  /* BOX - menu, timebutton and nextevent ********************************** */
  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, FALSE, 0);

  menubar = gtk_menu_bar_new ();
  gtk_widget_show (menubar);
  gtk_box_pack_start (GTK_BOX (box), menubar, TRUE, TRUE, 0);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_SomaX"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menubar), menu_item);

  menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu_menu);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("Refresh _Directories"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (read_directory), NULL);

  image = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Change Palinsesto"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (change_palinsesto), NULL);

  image = gtk_image_new_from_stock ("gtk-find", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("Reload Palinsesto"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (read_palinsesto), NULL);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Old Palinsesto"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (old_palinsesto), NULL);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("C_hange Spot file"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  image = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (change_spot), NULL);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("Reload Spot File"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (read_spot), NULL);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("Old Spot File"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (read_spot), NULL);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Start/Stop"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (startstop),
		    NULL);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("S_kip item"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (skip),
		    NULL);

  image = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Pause/UnPause"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (onlypause),
		    NULL);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Pause and Skip"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (pauseskip),
		    NULL);

#ifdef SOMAD_CMD
  if (daemon_widget)
    {
      menu_item = gtk_menu_item_new ();
      gtk_widget_show (menu_item);
      gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
      gtk_widget_set_sensitive (menu_item, FALSE);

      menu_item = gtk_menu_item_new_with_mnemonic (_("Somad _Terminal"));
      gtk_widget_show (menu_item);
      gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
				 daemon_menu (daemon_widget, accel));

    }
#endif

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_image_menu_item_new_from_stock ("gtk-preferences", accel);
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (preferences), NULL);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("S_hutdown somad"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (s_shutdown),
		    NULL);

  image = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_menu_item_new_with_mnemonic (_("Quit"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (quit),
		    NULL);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_NextItem"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menubar), menu_item);

  menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu_menu);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Show List"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (win_nextitem_show), NULL);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Add Remote Item"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (win_nextitem_add_r_file), NULL);

  image = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("_Add Remote Directory"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (win_nextitem_add_r_dir), NULL);

  image = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  if (local_connect)
    {
      menu_item =
	gtk_image_menu_item_new_with_mnemonic (_("_Add Local Item"));
      gtk_widget_show (menu_item);
      gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

      g_signal_connect ((gpointer) menu_item, "activate",
			G_CALLBACK (win_nextitem_add_l_file), NULL);

      image = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
      gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

      menu_item =
	gtk_image_menu_item_new_with_mnemonic (_("_Add Local Directory"));
      gtk_widget_show (menu_item);
      gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

      g_signal_connect ((gpointer) menu_item, "activate",
			G_CALLBACK (win_nextitem_add_l_dir), NULL);

      image = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
      gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
    }

  menu_item = gtk_menu_item_new_with_mnemonic (_("Add _Stream"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (win_nextitem_add_stream), NULL);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Separated NextItem"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (nextitem_clicked), NULL);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Palinsesti"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menubar), menu_item);

  menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu_menu);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("_Edit this palinsesto"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (editthis_clicked), NULL);

  image = gtk_image_new_from_stock ("gtk-edit", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("_Create new palinsesto"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (editor_clicked), NULL);

  image = gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("Save current _palinsesto"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (savepl_clicked), NULL);

  image = gtk_image_new_from_stock ("gtk-save", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("Save old _palinsesto"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (savepl_old_clicked), NULL);

  image = gtk_image_new_from_stock ("gtk-save", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item =
    gtk_menu_item_new_with_mnemonic (_("_Set current palinsesto as default"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (defaultthis_clicked), NULL);

  menu_item = gtk_menu_item_new_with_mnemonic (_("Sp_ots"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menubar), menu_item);

  menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu_menu);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("_Edit the Spot File"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (spoteditthis_clicked), NULL);

  image = gtk_image_new_from_stock ("gtk-edit", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("_Create a new Spot File"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (spoteditor_clicked), NULL);

  image = gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("Save current _spot file"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (savespot_clicked), NULL);

  image = gtk_image_new_from_stock ("gtk-save", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item =
    gtk_image_menu_item_new_with_mnemonic (_("Save old _spot file"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (savespot_old_clicked), NULL);

  image = gtk_image_new_from_stock ("gtk-save", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item =
    gtk_menu_item_new_with_mnemonic (_("_Set current spot file as default"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (spotdefaultthis_clicked), NULL);

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Tools"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menubar), menu_item);

  menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu_menu);

  menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Config tool"));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);

  g_signal_connect ((gpointer) menu_item, "activate",
		    G_CALLBACK (config_clicked), NULL);

  image = gtk_image_new_from_stock ("gtk-preferences", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  if ((w = plugin_new (x_plugin_starter_p)))
    {
      gtk_widget_show_all (w);
      gtk_container_add (GTK_CONTAINER (menu_menu), w);
    }

  menu_item = gtk_menu_item_new_with_mnemonic (_("_Help"));
  gtk_menu_item_set_right_justified (GTK_MENU_ITEM (menu_item), TRUE);
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menubar), menu_item);

  menu_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu_menu);

  menu_item = gtk_image_menu_item_new_with_label (_("About..."));
  gtk_widget_show (menu_item);
  gtk_container_add (GTK_CONTAINER (menu_menu), menu_item);
  g_signal_connect ((gpointer) menu_item, "activate",
		    GTK_SIGNAL_FUNC (x_help), NULL);

  image = gtk_image_new_from_stock ("gtk-about", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);

  /* BUTTON TIME */
  b_time = gtk_button_new_with_label ("");
  gtk_widget_show (b_time);
  gtk_tooltips_set_tip (tooltips, b_time,
			_("Switch between local and remote time"), NULL);
  gtk_box_pack_start (GTK_BOX (box), b_time, FALSE, FALSE, 0);

  g_signal_connect ((gpointer) b_time, "clicked",
		    G_CALLBACK (time_clicked), NULL);

  l_timeout = gtk_label_new (_("Next Event: 00:00"));
  gtk_widget_show (l_timeout);
  gtk_box_pack_start (GTK_BOX (box), l_timeout, FALSE, FALSE, 0);

  /* NBOX - left and right body ******************************************** */
  total_paned = gtk_hpaned_new ();
  gtk_widget_show (total_paned);
  gtk_box_pack_start (GTK_BOX (vbox), total_paned, TRUE, TRUE, 0);
  g_signal_connect_after ((gpointer) window, "size-request",
			  G_CALLBACK (antitimer_resize), total_paned);

  /* LISTS of SX *********************************************************** */
  lists_widget = box = gtk_vbox_new (FALSE, 0);
  gtk_paned_pack1 (GTK_PANED (total_paned), box, TRUE, TRUE);

  if (preferences_data->lists == TRUE)
    gtk_widget_show (box);

  /* LISTS PANEL *********************************************************** */
  paned = gtk_vpaned_new ();
  gtk_widget_show (paned);
  gtk_box_pack_start (GTK_BOX (box), paned, TRUE, TRUE, 0);

  /* ITEM LIST ************************************************************* */
  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_paned_pack1 (GTK_PANED (paned), frame, TRUE, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  fbox = gtk_vbox_new (0, 0);
  gtk_widget_show (fbox);
  gtk_container_add (GTK_CONTAINER (frame), fbox);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_box_pack_start (GTK_BOX (fbox), scrolledwindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
				       GTK_SHADOW_IN);

  model = gtk_tree_store_new (LIST_NUMBER, G_TYPE_STRING, G_TYPE_BOOLEAN,
			      G_TYPE_STRING, G_TYPE_STRING);

  textview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (textview), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (textview), FALSE);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);
  gtk_widget_show (textview);

  g_signal_connect (GTK_OBJECT (textview), "button_press_event",
		    GTK_SIGNAL_FUNC (tlist_popup_menu), (gpointer) 0);

  g_signal_connect (textview, "row_activated", G_CALLBACK (tlist_selected),
		    NULL);

  tlist_item_tv = textview;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (textview));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (textview), TRUE);
  g_object_unref (model);

  renderer = gtk_cell_renderer_text_new ();

  g_object_set (renderer, "style", PANGO_STYLE_ITALIC,
		"weight", PANGO_WEIGHT_BOLD, NULL);

  col = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (textview),
						     -1, _("Item List"),
						     renderer, "text",
						     LIST_FILE, "style_set",
						     LIST_STYLE, "weight_set",
						     LIST_STYLE, NULL);

  gtk_tree_view_column_set_resizable (gtk_tree_view_get_column
				      (GTK_TREE_VIEW (textview), col - 1),
				      TRUE);

  renderer = gtk_cell_renderer_text_new ();

  col = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (textview),
						     -1, _("Duration"),
						     renderer, "text",
						     LIST_DURATION, NULL);

  gtk_tree_view_column_set_resizable (gtk_tree_view_get_column
				      (GTK_TREE_VIEW (textview), col - 1),
				      TRUE);

  fdbox = gtk_hbox_new (0, 0);
  gtk_widget_show (fdbox);
  gtk_box_pack_start (GTK_BOX (fbox), fdbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("Total:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (fdbox), label, TRUE, TRUE, 0);

  label = gtk_label_new ("00:00:00.0");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (fdbox), label, TRUE, TRUE, 0);

  tlist_item_label = label;

  /* SPOT LIST ************************************************************* */
  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_paned_pack2 (GTK_PANED (paned), frame, TRUE, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  fbox = gtk_vbox_new (0, 0);
  gtk_widget_show (fbox);
  gtk_container_add (GTK_CONTAINER (frame), fbox);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_box_pack_start (GTK_BOX (fbox), scrolledwindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
				       GTK_SHADOW_IN);

  model = gtk_tree_store_new (LIST_NUMBER, G_TYPE_STRING, G_TYPE_BOOLEAN,
			      G_TYPE_STRING, G_TYPE_STRING);

  textview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (textview), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (textview), FALSE);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);
  gtk_widget_show (textview);

  g_signal_connect (GTK_OBJECT (textview), "button_press_event",
		    GTK_SIGNAL_FUNC (tlist_popup_menu), (gpointer) 1);

  g_signal_connect (textview, "row_activated", G_CALLBACK (tlist_selected),
		    NULL);

  tlist_spot_tv = textview;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (textview));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (textview), TRUE);
  g_object_unref (model);

  renderer = gtk_cell_renderer_text_new ();

  g_object_set (renderer, "style", PANGO_STYLE_ITALIC, NULL);

  col = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (textview),
						     -1, _("Spot List"),
						     renderer, "text",
						     LIST_FILE, "style_set",
						     LIST_STYLE, "weight_set",
						     LIST_STYLE, NULL);

  gtk_tree_view_column_set_resizable (gtk_tree_view_get_column
				      (GTK_TREE_VIEW (textview), col - 1),
				      TRUE);

  renderer = gtk_cell_renderer_text_new ();

  col = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (textview),
						     -1, _("Duration"),
						     renderer, "text",
						     LIST_DURATION, NULL);

  gtk_tree_view_column_set_resizable (gtk_tree_view_get_column
				      (GTK_TREE_VIEW (textview), col - 1),
				      TRUE);

  fdbox = gtk_hbox_new (0, 0);
  gtk_widget_show (fdbox);
  gtk_box_pack_start (GTK_BOX (fbox), fdbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("Total:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (fdbox), label, TRUE, TRUE, 0);

  label = gtk_label_new ("00:00:00.0");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (fdbox), label, TRUE, TRUE, 0);

  tlist_spot_label = label;

  /* BOX of DX ************************************************************* */
  nvbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (nvbox);
  gtk_paned_pack2 (GTK_PANED (total_paned), nvbox, TRUE, TRUE);

  /* BOX of SX TOP ********************************************************* */
  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (nvbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  box = gtk_hbox_new (0, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (frame), box);

  label = gtk_label_new (_("Current Transmission:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  palinsesto_name = gtk_button_new_with_label ("");
  gtk_widget_show (palinsesto_name);
  gtk_box_pack_start (GTK_BOX (box), palinsesto_name, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) palinsesto_name, "clicked",
		    G_CALLBACK (button_palinsesto_name_selected), NULL);
  gtk_widget_set_size_request (palinsesto_name, 200, -1);

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (nvbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtk_vbox_new (0, 0);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (frame), table);

  box = gtk_hbox_new (0, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (table), box, FALSE, FALSE, 0);

  label = gtk_label_new (_("Current Item:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  current_item = gtk_button_new_with_label ("");
  gtk_widget_show (current_item);
  gtk_box_pack_start (GTK_BOX (box), current_item, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) current_item, "clicked",
		    G_CALLBACK (button_current_item_selected), NULL);
  gtk_widget_set_size_request (current_item, 200, -1);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button, _("Skip the current item"), NULL);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (skip), NULL);

  image = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  item_infos = frame = gtk_frame_new (NULL);

  if (preferences_data->item_infos == TRUE)
    gtk_widget_show (item_infos);

  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtk_box_pack_start (GTK_BOX (table), frame, FALSE, FALSE, 0);

  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (frame), box);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("Album:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  current_album = label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("Year:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  current_year = label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("Genre:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  current_genre = label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  timer_widget = timer_new ();
  gtk_widget_show (timer_widget);
  gtk_container_set_border_width (GTK_CONTAINER (timer_widget), 3);
  gtk_box_pack_start (GTK_BOX (table), timer_widget, TRUE, TRUE, 0);

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (nvbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtk_vbox_new (0, 0);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (frame), table);

  box = gtk_hbox_new (0, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (table), box, FALSE, FALSE, 0);

  label = gtk_label_new (_("Next Item:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  next_item = gtk_button_new_with_label ("");
  gtk_widget_show (next_item);
  gtk_box_pack_start (GTK_BOX (box), next_item, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) next_item, "clicked",
		    G_CALLBACK (button_next_item_selected), NULL);
  gtk_widget_set_size_request (next_item, 200, -1);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button, _("Skip the follow item"), NULL);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (skip_next),
		    NULL);

  image = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (button), image);

  next_item_infos = frame = gtk_frame_new (NULL);

  if (preferences_data->next_item_infos == TRUE)
    gtk_widget_show (next_item_infos);

  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtk_box_pack_start (GTK_BOX (table), frame, FALSE, FALSE, 0);

  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (frame), box);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("Album:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  next_album = label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("Year:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  next_year = label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("Genre:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  next_genre = label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  /* BODY of SX - BOX up *************************************************** */
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (nvbox), hbox, FALSE, FALSE, 0);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_box_pack_start (GTK_BOX (hbox), scrolledwindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
				       GTK_SHADOW_IN);

  p_list = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (p_list);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), p_list);

  w_list = gtk_label_new ("");
  gtk_widget_show (w_list);
  gtk_container_add (GTK_CONTAINER (p_list), w_list);

  /* BOX of CALENDAR ******************************************************* */
  box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (hbox), box, FALSE, FALSE, 0);

  calendar = gtk_calendar_new ();
  gtk_widget_show (calendar);
  gtk_box_pack_start (GTK_BOX (box), calendar, FALSE, FALSE, 0);
  gtk_calendar_set_display_options (GTK_CALENDAR (calendar),
				    GTK_CALENDAR_SHOW_HEADING
				    | GTK_CALENDAR_SHOW_DAY_NAMES);

  g_signal_connect ((gpointer) calendar, "day_selected",
		    G_CALLBACK (list_refresh), NULL);

  button = gtk_button_new_with_mnemonic (_("To_day"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (list_today), NULL);

  /* HBOX first LINE of BUTTONS ******************************************** */
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (nvbox), hbox, FALSE, FALSE, 0);

  /* BUTTON CHANGE PALINSESTO */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_("Click to change palinsesto file"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (change_palinsesto), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-find", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Change Palinsesto"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* BUTTON RELOAD PALINSESTO */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_("Click to reload the palinsesto file"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (read_palinsesto), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Reload Palinsesto"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* BUTTON OLD PALINSESTO */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_("Click to set the old palinsesto file"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (old_palinsesto), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Old Palinsesto"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* HBOX seconds LINE of BUTTONS ****************************************** */
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (nvbox), hbox, FALSE, FALSE, 0);

  /* BUTTON CHANGE SPOT */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button, _("Click to change spot file"),
			NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (change_spot), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-find", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Change Spot File"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* BUTTON RELOAD SPOT */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_("Click to reload the spot file"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (read_spot), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Reload Spot File"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* BUTTON OLD SPOT */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_("Click to set the old spot file"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (old_spot), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Old Spot File"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* HBOX third LINE of BUTTONS ******************************************** */
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (nvbox), hbox, FALSE, FALSE, 0);

  /* BUTTON STOP/START */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button, _("Click to stop or start somad"),
			NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (startstop),
		    NULL);

  b_startstop = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (b_startstop);
  gtk_container_add (GTK_CONTAINER (button), b_startstop);

  w_startstop = gtk_label_new (_("Wait..."));
  gtk_widget_show (w_startstop);
  gtk_container_add (GTK_CONTAINER (b_startstop), w_startstop);

  /* BUTTON STOP FOR... */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_("Click to stop somad for some seconds..."), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (hbox), box, TRUE, TRUE, 0);

  adj = gtk_adjustment_new (0, 0, 99, 1, 10, 10);
  sp = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_widget_show (sp);
  gtk_box_pack_start (GTK_BOX (box), sp, TRUE, TRUE, 0);
  stop_for_data.hours = sp;

  label = gtk_label_new (_("h"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  adj = gtk_adjustment_new (0, 0, 60, 1, 10, 10);
  sp = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_widget_show (sp);
  gtk_box_pack_start (GTK_BOX (box), sp, TRUE, TRUE, 0);
  stop_for_data.minutes = sp;

  label = gtk_label_new (_("m"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  adj = gtk_adjustment_new (0, 0, 60, 1, 10, 10);
  sp = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
  gtk_widget_show (sp);
  gtk_box_pack_start (GTK_BOX (box), sp, TRUE, TRUE, 0);
  stop_for_data.seconds = sp;

  label = gtk_label_new (_("s"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (stop_for),
		    NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-jump-to", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("Stop for:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* HBOX forth LINE of BUTTONS ******************************************** */
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (nvbox), hbox, FALSE, FALSE, 0);

  /* BUTTON SKIP */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button, _("Skip the current item"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (skip), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("S_kip Item"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* BUTTON READ DIR */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_
			("Click to force a new reading for directories and files"),
			NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (read_directory),
		    NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("Refresh _Directories"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* BUTTON SHUTDOWN */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_("Click to shutdown the somad server"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (s_shutdown),
		    NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Shutdown somad"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* HBOX 5th LINE of BUTTONS ********************************************** */
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (nvbox), hbox, FALSE, FALSE, 0);

  /* BUTTON PAUSE/UNPAUSE */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_("Click to set somad in pause/unpause mode"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (onlypause),
		    NULL);

  b_pause = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (b_pause);
  gtk_container_add (GTK_CONTAINER (button), b_pause);

  w_pause = gtk_label_new (_("Wait..."));
  gtk_widget_show (w_pause);
  gtk_container_add (GTK_CONTAINER (b_pause), w_pause);

  /* BUTTON PAUSE/UNPAUSE */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_tooltips_set_tip (tooltips, button,
			_("Pause and skip the current item"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (pauseskip),
		    NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-jump-to", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("Pause and Skip"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  /* SERVER **************************************************************** */
#ifdef SOMAD_CMD
  if (daemon_widget)
    {
      button = gtk_button_new_with_mnemonic (_("Server Status"));
      gtk_widget_show (button);
      gtk_box_pack_start (GTK_BOX (nvbox), button, FALSE, FALSE, 0);
      g_signal_connect ((gpointer) button, "clicked",
			G_CALLBACK (daemon_show), NULL);
    }
#endif

  /* DRAW TIMELINE ********************************************************* */
  pl_draw = draw_new (&somad_pl, draw_button_press, NULL, FALSE, NULL, NULL);
  gtk_widget_show (pl_draw);
  gtk_box_pack_start (GTK_BOX (vbox), pl_draw, FALSE, FALSE, 0);

  /* BOX FOR THE STATUSBAR ************************************************* */
  statusbar_widget = box = gtk_hbox_new (FALSE, 0);

  if (preferences_data->statusbar == TRUE)
    gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, FALSE, 0);

  image_running =
    gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_show (image_running);
  gtk_box_pack_start (GTK_BOX (box), image_running, FALSE, FALSE, 0);

  timer_pb = gtk_progress_bar_new ();
  gtk_widget_set_size_request (timer_pb, 100, -1);
  gtk_box_pack_start (GTK_BOX (box), timer_pb, FALSE, FALSE, 0);

  if (preferences_data->timer_pb == TRUE)
    gtk_widget_show (timer_pb);

  statusbar = gtk_statusbar_new ();
  statusbar_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "");
  gtk_widget_show (statusbar);
  gtk_box_pack_start (GTK_BOX (box), statusbar, TRUE, TRUE, 0);

  gtk_window_add_accel_group (GTK_WINDOW (window), accel);
  return window;
}

static void
antitimer_resize (GtkWidget * window, GtkRequisition * req, GtkWidget * paned)
{
  static int h = 0;
  static int w = 0;

  if (h != window->allocation.height || w != window->allocation.width)
    {
      h = window->allocation.height;
      w = window->allocation.width;

      gtk_paned_set_position (GTK_PANED (paned), w / 3);
    }
}

static void
x_plugin_starter_p (GtkWidget * w, struct plugin_t *data)
{
  gchar s[1024], *file;
  int d = 0;

  while (d < 1000)
    {
      snprintf (s, sizeof (s), "somax_pls_%.3d.cfg", d);
      file = g_build_path (G_DIR_SEPARATOR_S, g_get_tmp_dir (), s, NULL);

      if (g_file_test (file, G_FILE_TEST_EXISTS) == FALSE)
	break;

      g_free (file);
      d++;
    }

  if (d == 1000)
    {
      snprintf (s, sizeof (s), "somax_pls_%.3d.cfg", getrandom (0, 1000));
      file = g_build_path (G_DIR_SEPARATOR_S, g_get_tmp_dir (), s, NULL);
    }

  if (palinsesto_save_file (file, somad_pl))
    {
      dialog_msg (_("Error writing on file."));
      g_free (file);
      return;
    }

  plugin_starter (data, file);
  g_free (file);
}

static void
x_help (GtkWidget * w, gpointer dummy)
{
  help_show ();
}

#ifdef SOMAD_CMD
static int
daemon_show (void)
{
  static GtkWidget *window = NULL;

  if (!window)
    {
      GtkWidget *scroll;
      GtkWidget *box;
      GtkWidget *frame;

      window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title (GTK_WINDOW (window), _("Server Status"));
      g_signal_connect ((gpointer) window, "delete_event",
			G_CALLBACK (daemon_show), NULL);

      frame = gtk_frame_new (_("Server Status"));
      gtk_container_add (GTK_CONTAINER (window), frame);

      scroll =
	gtk_vscrollbar_new (GTK_ADJUSTMENT
			    (VTE_TERMINAL (daemon_widget)->adjustment));
      GTK_WIDGET_UNSET_FLAGS (scroll, GTK_CAN_FOCUS);

      box = gtk_hbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (frame), box);

      gtk_box_pack_start (GTK_BOX (box), daemon_widget, TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (box), scroll, FALSE, TRUE, 0);

      gtk_widget_show_all (window);
    }

  else
    gtk_widget_ref (daemon_widget);

  return 1;
}

#endif

/* EOF */
