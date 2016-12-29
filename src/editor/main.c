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

#include "editor.h"

int editor_statusbar_lock;
char **env = NULL;
struct somax_preferences_t *preferences_data;

struct editor_data_t *editor_data = NULL;
GtkWidget *notebook;
GtkWidget *undo_widget, *undo_menu, *undo_history;
GtkWidget *redo_widget, *redo_menu, *redo_history;
struct somad_data *dump_pl = NULL;
struct somad_spot_data *dump_spot = NULL;

static void editor_plugin_starter_p (GtkWidget *, struct plugin_t *);
static void usage (void);
static void editor_help (GtkWidget *, gpointer);

void
fatal (char *text)
{
  dialog_msg (text);
  exit (1);
}

char **
get_env (void)
{
  return env;
}

struct tm *
get_time (void)
{
  time_t tt = time (NULL);
  return localtime (&tt);
}

int
editor_quit (GtkWidget * w, GdkEvent * event, gpointer dummy)
{
  struct editor_data_t *tmp;
  GList *list = NULL;
  GList *l;
  char s[1024];
  int len;

  tmp = editor_data;

  while (tmp)
    {
      switch (tmp->type)
	{
	case TYPE_SPOT:
	  if (maker_spot_is_no_saved (tmp->spot))
	    {
	      if (tmp->spot->maker_spot_file)
		list =
		  g_list_append (list,
				 g_strdup (editor_parse_file
					   (tmp->spot->maker_spot_file)));
	      else
		list = g_list_append (list, g_strdup (_("New Spot File")));
	    }
	  break;

	case TYPE_PALINSESTO:
	  if (maker_pl_is_no_saved (tmp->pl))
	    {
	      if (tmp->pl->maker_pl_file)
		list =
		  g_list_append (list,
				 g_strdup (editor_parse_file
					   (tmp->pl->maker_pl_file)));
	      else
		list = g_list_append (list, g_strdup (_("New Palinsesto")));
	    }
	  break;
	}

      tmp = tmp->next;
    }

  if (!list)
    {
      gtk_main_quit ();
      return FALSE;
    }

  len = g_list_length (list);

  snprintf (s, sizeof (s), _("Exit without save?\nCheck: "));

  l = list;
  while (l)
    {
      len--;
      strncat (s, (char *) l->data, strlen (s));

      if (len)
	strncat (s, ", ", strlen (s));

      free (l->data);
      l = l->next;
    }

  g_list_free (list);

  if (dialog_ask (s) == GTK_RESPONSE_OK)
    {
      gtk_main_quit ();
      return FALSE;
    }

  return TRUE;
}

int
main (int argc, char **argv, char **e)
{
  GtkWidget *window;
  GtkWidget *vbox, *box, *ubox, *vubox;
  GtkWidget *menubar;
  GtkWidget *menuitem;
  GtkWidget *menumenu;
  GtkWidget *toolbar;
  GtkWidget *sep;
  GtkWidget *button;
  GtkWidget *image;
  GtkAccelGroup *accel;
  GtkWidget *w;

  char s[1024];
  int y, x;

#ifdef ENABLE_NLS
  setlocale (LC_ALL, "");
  textdomain (PACKAGE);
  bindtextdomain (PACKAGE, LOCALEDIR);
#endif

  for (y = 1; y < argc; y++)
    {
      if (!strcmp (argv[y], "-h") || !strcmp (argv[y], "--help"))
	{
	  usage ();
	  return 0;
	}
    }

  env = e;

  g_thread_init (NULL);
  gtk_init (NULL, NULL);

  splash_showhide ();

  preferences_data = get_preferences ();

  plugin_scanner ();
  module_scanner ();
  soma_local_init ();

  splash_showhide ();

  accel = gtk_accel_group_new ();

  snprintf (s, sizeof (s), "%s-editor %s", PACKAGE, VERSION);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), s);
  gtk_window_set_default_icon_from_file (PATH_ICON, NULL);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);

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
		    G_CALLBACK (editor_quit), NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  box = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (box);
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, FALSE, 0);

  menubar = gtk_menu_bar_new ();
  gtk_widget_show (menubar);
  gtk_box_pack_start (GTK_BOX (box), menubar, TRUE, TRUE, 0);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_File"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menubar), menuitem);

  menumenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menumenu);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-new", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_new), NULL);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-open", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_open), NULL);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-save", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_save), NULL);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-save-as", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_save_as), NULL);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-close", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_close), NULL);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-quit", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_quit), NULL);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_Edit"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menubar), menuitem);

  menumenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menumenu);

  undo_menu = menuitem =
    gtk_image_menu_item_new_from_stock ("gtk-undo", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_undo), NULL);

  redo_menu = menuitem =
    gtk_image_menu_item_new_from_stock ("gtk-redo", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_redo), NULL);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-cut", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_cut), NULL);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-copy", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_copy), NULL);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-paste", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_paste), NULL);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-delete", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_delete), NULL);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_View"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menubar), menuitem);

  menumenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menumenu);

  menuitem = gtk_image_menu_item_new_with_mnemonic (_("_Change Color"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_select_color), NULL);

  image = gtk_image_new_from_stock ("gtk-select-color", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

#ifdef ENABLE_GTKSOURCEVIEW
  menuitem = gtk_image_menu_item_new_with_mnemonic (_("Manual _Edit"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_view), NULL);

  image = gtk_image_new_from_stock ("gtk-edit", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
#endif

  menuitem = gtk_image_menu_item_new_with_mnemonic (_("Use _Your Editor"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_your_view), window);

  image = gtk_image_new_from_stock ("gtk-edit", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_with_mnemonic (_("Set your editor..."));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (editor_preferences), NULL);

  image = gtk_image_new_from_stock ("gtk-preferences", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  if ((w = plugin_new (editor_plugin_starter_p)))
    {
      gtk_widget_show (w);
      gtk_container_add (GTK_CONTAINER (menubar), w);
    }

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
		    GTK_SIGNAL_FUNC (editor_help), NULL);

  image = gtk_image_new_from_stock ("gtk-about", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (daemon_connect), NULL);

  connect_label = gtk_label_new (_("Connect"));
  gtk_widget_show (connect_label);
  gtk_container_add (GTK_CONTAINER (button), connect_label);

  toolbar = gtk_toolbar_new ();
  gtk_widget_show (toolbar);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-new");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_new), NULL);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-open");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_open), NULL);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-save");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_save), NULL);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-save-as");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_save_as), NULL);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-close");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_close), NULL);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *)gtk_tool_item_new();
  gtk_widget_show(button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);

  ubox = gtk_hbox_new(0, 0);
  gtk_widget_show (ubox);
  gtk_container_add(GTK_CONTAINER(button), ubox);

  undo_widget = button = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_undo), NULL);

  vubox = gtk_vbox_new(0, 0);
  gtk_widget_show (vubox);
  gtk_container_add(GTK_CONTAINER(button), vubox);

  image = gtk_image_new_from_stock("gtk-undo",GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show(image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  image = gtk_label_new(_("undo"));
  gtk_widget_show(image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);
  
  undo_history = button = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_undo_history), NULL);
  
  image = gtk_arrow_new(GTK_ARROW_DOWN,GTK_SHADOW_NONE);
  gtk_widget_show (image);
  gtk_container_add(GTK_CONTAINER(button), image);

  button = (GtkWidget *)gtk_tool_item_new();
  gtk_widget_show(button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);

  ubox = gtk_hbox_new(0, 0);
  gtk_widget_show (ubox);
  gtk_container_add(GTK_CONTAINER(button), ubox);

  redo_widget = button = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_redo), NULL);

  vubox = gtk_vbox_new(0, 0);
  gtk_widget_show (vubox);
  gtk_container_add(GTK_CONTAINER(button), vubox);

  image = gtk_image_new_from_stock("gtk-redo",GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show(image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);

  image = gtk_label_new(_("redo"));
  gtk_widget_show(image);
  gtk_box_pack_start (GTK_BOX (vubox), image, TRUE, TRUE, 0);
  
  redo_history = button = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (ubox), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_redo_history), NULL);
  
  image = gtk_arrow_new(GTK_ARROW_DOWN,GTK_SHADOW_NONE);
  gtk_widget_show (image);
  gtk_container_add(GTK_CONTAINER(button), image);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-cut");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_cut), NULL);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-copy");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_copy), NULL);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-paste");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_paste), NULL);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_widget_show (sep);
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-delete");
  gtk_widget_show (button);
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  g_signal_connect ((gpointer) button, "clicked",
		    GTK_SIGNAL_FUNC (editor_delete), NULL);

  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

  statusbar = gtk_statusbar_new ();
  gtk_widget_show (statusbar);
  gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);

  if (argc == 1)
    editor_new (NULL, NULL);

  else
    {
      for (y = 1; y < argc; y++)
	{
	  if (!strcmp (argv[y], "-pl"))
	    editor_new_pl ();

	  else if (!strcmp (argv[y], "-sp"))
	    editor_new_sp ();

	  else
	    editor_open_file (argv[y]);
	}
    }

  if (!editor_data)
    editor_new_pl ();

  statusbar_set (_("Welcome to somax editor!"));
  editor_statusbar_lock = WAIT;

  g_timeout_add (500, editor_timeout, NULL);

  gtk_window_add_accel_group (GTK_WINDOW (window), accel);
  gtk_widget_show (window);

  gtk_main ();

  free_preferences (preferences_data);

  return 0;
}

static void
editor_plugin_starter_p (GtkWidget * w, struct plugin_t *data)
{
  gchar s[1024], *file;
  int d = 0;
  struct editor_data_t *edit = editor_get_data ();

  if (!edit)
    return;

  if (edit->type == TYPE_SPOT)
    {
      dialog_msg (_("This file is a spot file. No plugin can run!"));
      return;
    }

  editor_statusbar_lock = LOCK;
  statusbar_set (_("Running plugin '%s'..."), data->name);

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

  if (palinsesto_save_file (file, edit->pl->maker_pl))
    {
      dialog_msg (_("Error writing on file."));

      editor_statusbar_lock = WAIT;
      g_free (file);
      return;
    }

  plugin_starter (data, file);
  g_free (file);

  editor_statusbar_lock = WAIT;
}

static void
usage (void)
{
  fprintf (stdout, "%s %s %s %s\n\n", PACKAGE, VERSION, AUTHORS, COPYRIGHT);

  fprintf (stdout, _("\t%s [options] <file1> <file2> ...\n\n"), PACKAGE);
  fprintf (stdout, _("\t-h  or --help               this help\n"));
  fprintf (stdout, _("\t-pl or --palinsesto         new palinsesto file\n"));
  fprintf (stdout, _("\t-sp or --spot               new spot file\n"));
  fprintf (stdout, "\n");
}

static void
editor_help (GtkWidget * w, gpointer dummy)
{
  help_show ();
}

/* EOF */
