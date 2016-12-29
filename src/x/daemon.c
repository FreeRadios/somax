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

#ifdef SOMAD_CMD

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
# error Use configure; make; make install
#endif

#include "somax.h"

#define DAEMON_VTE_SIZE_DEFAULT 10
#define DAEMON_VTE_FONT_DEFAULT "Sans 10"

GtkWidget *daemon_widget = NULL;

int daemon_start = 0;

struct somax_config_vte *config_vte = NULL;

static void daemon_restart (GtkWidget * v, char **);
static gint daemon_popup (GtkWidget *, GdkEventButton *, gpointer);
static void daemon_increase (GtkWidget * w, gpointer vte);
static void daemon_decrease (GtkWidget * w, gpointer vte);
static void daemon_audible_bell (GtkWidget * w, gpointer vte);
static void daemon_visible_bell (GtkWidget * w, gpointer vte);
static void daemon_transparent (GtkWidget * w, gpointer vte);
static void daemon_font (GtkWidget * w, gpointer vte);
static char **daemon_split (char *ar);
static void daemon_free (char **arg);
static gboolean daemon_destroy (GtkWidget * widget, GdkEvent * event,
				gpointer dummy);

/* Popup waiting for... */
static GtkWidget *
daemon_wait_win (void)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *label = NULL;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("SomaD starting..."));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_type_hint (GTK_WINDOW (window),
			    GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);

  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  gtk_widget_set_size_request (window, 200, -1);

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (window), frame);

  box = gtk_vbox_new (FALSE, 8);
  gtk_container_add (GTK_CONTAINER (frame), box);

  hbox = gtk_hbox_new (FALSE, 0);
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  image = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new ("SomaD starting...");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  return window;
}

/* This function return the terminal if somad started or NULL if somad does not
 * started */
gboolean
daemon_run (char *cmd)
{
  GtkWidget *v;
  PangoFontDescription *pango;
  time_t old, now;
  GtkWidget *w;
  GdkCursor *cursor;
  char **argv;
  int pid;

  if (!config_vte)
    config_vte = get_vte ();

  v = vte_terminal_new ();
  vte_terminal_set_emulation (VTE_TERMINAL (v), "xterm");
  vte_terminal_set_mouse_autohide (VTE_TERMINAL (v), TRUE);

  vte_terminal_set_audible_bell (VTE_TERMINAL (v), config_vte->audible_bell);
  vte_terminal_set_visible_bell (VTE_TERMINAL (v), config_vte->visible_bell);

/*
 * If i decomment this line, somax does not connect to somad...
 *  daemon_transparent_flag = config->transparent;
 * vte_terminal_set_background_transparent (VTE_TERMINAL (v), daemon_transparent_flag);
 */

  vte_terminal_reset (VTE_TERMINAL (v), TRUE, TRUE);
  vte_terminal_set_default_colors (VTE_TERMINAL (v));
  vte_terminal_set_cursor_blinks (VTE_TERMINAL (v), TRUE);
  vte_terminal_set_scroll_on_output (VTE_TERMINAL (v), TRUE);
  vte_terminal_set_scroll_on_keystroke (VTE_TERMINAL (v), TRUE);

  if (!config_vte->font)
    config_vte->font = g_strdup (DAEMON_VTE_FONT_DEFAULT);

  pango = pango_font_description_from_string (config_vte->font);

  vte_terminal_set_font (VTE_TERMINAL (v), pango);

  argv = daemon_split (cmd);
  pid = vte_terminal_fork_command (VTE_TERMINAL (v), argv[0], argv, env, ".",
				   FALSE, FALSE, FALSE);

  g_signal_connect (G_OBJECT (v), "child-exited", G_CALLBACK (daemon_restart),
		    argv);

  g_signal_connect (GTK_OBJECT (v), "button_press_event",
		    GTK_SIGNAL_FUNC (daemon_popup), NULL);

  g_object_set_data ((gpointer) v, "failed", (gpointer) 0);
  g_object_set_data ((gpointer) v, "init", (gpointer) 1);

  old = time (NULL);

  w = daemon_wait_win ();
  gtk_widget_show_all (w);

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (w->window, cursor);

  while (1)
    {
      now = time (NULL);

      if ((now - old) >= 2)
	break;

      while (gtk_events_pending ())
	gtk_main_iteration ();

      g_usleep (500);
    }

  if (g_object_get_data ((gpointer) v, "failed") == (gpointer) 1)
    {
      gtk_widget_destroy (v);
      daemon_free (argv);

      gdk_cursor_unref (cursor);
      gtk_widget_destroy (w);

      return 1;
    }

  g_object_set_data ((gpointer) v, "init", 0);

  gdk_cursor_unref (cursor);
  gtk_widget_destroy (w);

  g_signal_connect (v, "destroy-event", G_CALLBACK (daemon_destroy), NULL);
  daemon_widget = v;

  return 0;
}

/* callback for the vte terminal */
static void
daemon_restart (GtkWidget * v, char **argv)
{
  if (g_object_get_data ((gpointer) v, "init") == (gpointer) 1)
    {
      g_object_set_data ((gpointer) v, "failed", (gpointer) 1);
      return;
    }
  vte_terminal_fork_command (VTE_TERMINAL (v), argv[0], argv, env, ".", FALSE,
			     FALSE, FALSE);
}

/* Callback for the right button of the mouse */
static gint
daemon_popup (GtkWidget * vte, GdkEventButton * event, gpointer dummy)
{
  static GtkWidget *menu = NULL;
  if (event->button == 3)
    {
      if (!menu)
	menu = daemon_menu (vte, NULL);

      gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, event->button,
		      event->time);

      return TRUE;
    }

  return FALSE;
}

/* It returns a menu for the administration of the terminal. If accel != null
 * it sets some events to the accelgroup. This function does not static because
 * it is runed by interface.c */
GtkWidget *
daemon_menu (GtkWidget * vte, GtkAccelGroup * accel)
{
  GtkWidget *menu;
  GtkWidget *item;
  GtkWidget *image;

  menu = gtk_menu_new ();

  /* INCREASE */
  item = gtk_image_menu_item_new_with_mnemonic (_("_Increase font size"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  image = gtk_image_new_from_stock ("gtk-zoom-in", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);

  g_signal_connect ((gpointer) item, "activate", G_CALLBACK (daemon_increase),
		    vte);

  if (accel)
    gtk_widget_add_accelerator (item, "activate", accel, GDK_Up,
				GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  /* DECREASE */
  item = gtk_image_menu_item_new_with_mnemonic (_("_Decrease font size"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  image = gtk_image_new_from_stock ("gtk-zoom-out", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);

  if (accel)
    gtk_widget_add_accelerator (item, "activate", accel, GDK_Down,
				GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  g_signal_connect ((gpointer) item, "activate", G_CALLBACK (daemon_decrease),
		    vte);

  /* SEPARATOR */
  item = gtk_image_menu_item_new ();
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  /* FONTS */
  item = gtk_image_menu_item_new_with_mnemonic (_("_Change font"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  image = gtk_image_new_from_stock ("gtk-select-font", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);

  g_signal_connect ((gpointer) item, "activate", G_CALLBACK (daemon_font),
		    vte);

  /* SEPARATOR */
  item = gtk_image_menu_item_new ();
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  /* AUDIBLE BELL */
  item = gtk_check_menu_item_new_with_mnemonic (_("_Audible bell"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  if (config_vte->audible_bell)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
  else
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), FALSE);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (daemon_audible_bell), vte);

  /* VISIBLE BELL */
  item = gtk_check_menu_item_new_with_mnemonic (_("_Visible bell"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  if (config_vte->visible_bell)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
  else
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), FALSE);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (daemon_visible_bell), vte);

  /* TRANSPARENT BACKGROUND */
  item = gtk_check_menu_item_new_with_mnemonic (_("_Transparent background"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  if (config_vte->transparent)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
  else
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), FALSE);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (daemon_transparent), vte);

  return menu;
}

/* Increase the font size */
static void
daemon_increase (GtkWidget * w, gpointer vte)
{
  gchar *name;
  PangoFontDescription *pango;

  pango = (PangoFontDescription *) vte_terminal_get_font (VTE_TERMINAL (vte));
  pango_font_description_set_size (pango,
				   (pango_font_description_get_size (pango) +
				    1) * PANGO_SCALE);

  name = (gchar *) pango_font_description_to_string (pango);

  if (config_vte->font)
    g_free (config_vte->font);

  config_vte->font = name;
  set_vte (config_vte);

  vte_terminal_set_font (VTE_TERMINAL (vte), pango);
}

/* Decrease the font size */
static void
daemon_decrease (GtkWidget * w, gpointer vte)
{
  gchar *name;
  PangoFontDescription *pango;

  pango = (PangoFontDescription *) vte_terminal_get_font (VTE_TERMINAL (vte));
  pango_font_description_set_size (pango,
				   (pango_font_description_get_size (pango) -
				    1) * PANGO_SCALE);

  name = (gchar *) pango_font_description_to_string (pango);

  if (config_vte->font)
    g_free (config_vte->font);

  config_vte->font = name;
  set_vte (config_vte);

  vte_terminal_set_font (VTE_TERMINAL (vte), pango);
}

/* (De)Active the audible bell */
static void
daemon_audible_bell (GtkWidget * w, gpointer vte)
{
  config_vte->audible_bell = !config_vte->audible_bell;
  set_vte (config_vte);

  vte_terminal_set_audible_bell (VTE_TERMINAL (vte),
				 config_vte->audible_bell);
}

/* (De)Active the visible bell */
static void
daemon_visible_bell (GtkWidget * w, gpointer vte)
{
  config_vte->visible_bell = !config_vte->visible_bell;
  set_vte (config_vte);

  vte_terminal_set_visible_bell (VTE_TERMINAL (vte),
				 config_vte->visible_bell);
}

/* (De)Active the transparent background */
static void
daemon_transparent (GtkWidget * w, gpointer vte)
{
  config_vte->transparent = !config_vte->transparent;
  set_vte (config_vte);

  vte_terminal_set_background_transparent (VTE_TERMINAL (vte),
					   config_vte->transparent);
}

/* This function shows the font selection widget and updates the font of
 * terminal */
static void
daemon_font (GtkWidget * w, gpointer vte)
{
  GtkWidget *font;
  PangoFontDescription *pango;
  gchar *name;
  gint ret;

  font = gtk_font_selection_dialog_new (_("Select a font"));
  gtk_container_set_border_width (GTK_CONTAINER (font), 2);
  gtk_window_set_type_hint (GTK_WINDOW (font), GDK_WINDOW_TYPE_HINT_DIALOG);

  pango = (PangoFontDescription *) vte_terminal_get_font (VTE_TERMINAL (vte));
  name = (gchar *) pango_font_description_to_string (pango);

  gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG (font),
					   name);

  g_free (name);

  gtk_widget_show (font);

  gtk_widget_show_all (font);
  ret = gtk_dialog_run (GTK_DIALOG (font));

  while (1)
    {
      if (ret == GTK_RESPONSE_OK || ret == GTK_RESPONSE_APPLY)
	{
	  if ((name =
	       gtk_font_selection_dialog_get_font_name
	       (GTK_FONT_SELECTION_DIALOG (font)))
	      && (pango = pango_font_description_from_string (name)))
	    {
	      if (config_vte->font)
		g_free (config_vte->font);

	      config_vte->font = g_strdup (name);
	      set_vte (config_vte);

	      vte_terminal_set_font (VTE_TERMINAL (vte), pango);
	    }
	}

      if (ret != GTK_RESPONSE_APPLY)
	break;

      ret = gtk_dialog_run (GTK_DIALOG (font));
    }

  gtk_widget_destroy (font);
}

static char **
daemon_split (char *ar)
{
  int a = 0;
  char opt[1024];
  GList *list = NULL, *tmp;
  char **arg;

  while (*ar)
    {
      if (*ar == ' ' || *ar == '\t')
	{
	  ar++;
	  if (a)
	    {
	      opt[a] = 0;
	      list = g_list_append (list, g_strdup (opt));
	    }
	  a = 0;
	}
      else
	while (*ar && *ar != ' ' && *ar != '\t')
	  if (a < 1023)
	    opt[a++] = *ar++;
    }

  if (a)
    {
      opt[a] = 0;
      list = g_list_append (list, g_strdup (opt));
    }

  if (!list)
    {
      arg = (char **) g_malloc (sizeof (char *) * 2);
      arg[0] = g_strdup (SOMAD_CMD);
      arg[1] = NULL;

      return arg;
    }

  arg = (char **) g_malloc (sizeof (char *) * (g_list_length (list) + 1));

  tmp = list;
  a = 0;
  while (tmp)
    {
      arg[a++] = tmp->data;
      tmp = tmp->next;
    }

  arg[a] = NULL;

  g_list_free (list);
  return arg;
}

static void
daemon_free (char **arg)
{
  char **a = arg;

  while (*arg)
    {
      free (*arg);
      arg++;
    }

  free (a);
}

static gboolean
daemon_destroy (GtkWidget * widget, GdkEvent * event, gpointer dummy)
{
  free_vte (config_vte);
  config_vte = NULL;

  return FALSE;
}

#endif

/* EOF */
