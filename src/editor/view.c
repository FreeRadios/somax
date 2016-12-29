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

static void view_preference_ok (GtkWidget *, gpointer);
static void view_preference_close (GtkWidget *, gpointer);

#ifdef ENABLE_GTKSOURCEVIEW

static GtkWidget *view_undo_button;
static GtkWidget *view_redo_button;
static GtkWidget *view_source;
static GtkSourceBuffer *view_buffer;
static gboolean view_change = FALSE;
static gchar *view_output = NULL;
static GtkWidget *view_window = NULL;

static void view_new (char *);
static void view_paste (GtkWidget *, gpointer);
static void view_copy (GtkWidget *, gpointer);
static void view_cut (GtkWidget *, gpointer);
static void view_find (GtkWidget *, gpointer);
static void view_undo (GtkWidget *, gpointer);
static void view_redo (GtkWidget *, gpointer);
static void view_changed (GtkWidget *, gpointer);
static void view_close (GtkWidget *, gpointer);
static void view_quit (GtkWidget *, gpointer);
static void view_new_file (GtkWidget *, gpointer);
static void view_open (GtkWidget *, gpointer);
static void view_save (GtkWidget *, gpointer);
static void view_save_as (GtkWidget *, gpointer);
static void view_save_real (void);
static int view_search (char *, GtkSourceSearchFlags, int, GtkTextIter *);
static int view_search_dialog (void);
static int view_check (gpointer);

void
editor_view (GtkWidget * w, gpointer dummy)
{
  gchar s[1024], *file;
  int d = 0;
  struct editor_data_t *edit = editor_get_data ();

  if (!edit)
    return;

  editor_statusbar_lock = LOCK;
  statusbar_set (_("Running manual editor..."));

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

  if (edit->type == TYPE_PALINSESTO)
    {
      if (palinsesto_save_file (s, edit->pl->maker_pl))
	{
	  dialog_msg (_("Error writing on file."));

	  editor_statusbar_lock = WAIT;
	  return;
	}
    }
  else if (edit->type == TYPE_SPOT)
    {
      if (spot_save_file (s, edit->spot->maker_spot))
	{
	  dialog_msg (_("Error writing on file."));

	  editor_statusbar_lock = WAIT;
	  return;
	}
    }

  view_new (s);

  editor_statusbar_lock = WAIT;
}

static void
view_new (gchar * file)
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *hbox;
  GtkWidget *handlebox;
  GtkWidget *toolbar;
  GtkTooltips *tooltips;
  GtkWidget *sep;
  GtkWidget *button;
  GtkWidget *sw;
  GtkAccelGroup *accel;
  GtkWidget *menubar;
  GtkWidget *menuitem;
  GtkWidget *menumenu;
  GtkWidget *image;

  GtkSourceLanguagesManager *lm;
  GtkSourceLanguage *lang;
  GSList *l, *old;

  char s[1024];
  gint x, y;

  gchar *a;

  if (g_file_get_contents (file, &a, NULL, NULL) == FALSE)
    {
      dialog_msg (_("Internal error."));
      return;
    }

  snprintf (s, sizeof (s), _("%s -editor %s - manual edit"), PACKAGE,
	    VERSION);

  tooltips = gtk_tooltips_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), s);
  gtk_window_set_default_icon_from_file (PATH_ICON, NULL);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);

  accel = gtk_accel_group_new ();

  y = gdk_screen_height () * 2 / 3;
  if (y < 480)
    y = 480;

  x = gdk_screen_width () * 2 / 3;
  if (x < 480)
    x = 480;

  gtk_widget_set_size_request (window, x, y);
  g_signal_connect ((gpointer) window, "delete_event",
		    G_CALLBACK (gtk_widget_destroy), NULL);

  box = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), box);

  menubar = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (box), menubar, FALSE, FALSE, 0);

  menuitem = gtk_menu_item_new_with_mnemonic (_("_File"));
  gtk_container_add (GTK_CONTAINER (menubar), menuitem);

  menumenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menumenu);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-new", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (view_new_file), NULL);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-open", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (view_open), NULL);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-save", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (view_save), NULL);

  menuitem = gtk_image_menu_item_new_from_stock ("gtk-save-as", accel);
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (view_save_as), NULL);

  menuitem = gtk_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  gtk_widget_set_sensitive (menuitem, FALSE);

  menuitem = gtk_image_menu_item_new_with_mnemonic (_("Close and update"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (view_close), file);

  image = gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  menuitem =
    gtk_image_menu_item_new_with_mnemonic (_("Close without update"));
  gtk_widget_show (menuitem);
  gtk_container_add (GTK_CONTAINER (menumenu), menuitem);
  g_signal_connect ((gpointer) menuitem, "activate",
		    GTK_SIGNAL_FUNC (view_quit), file);

  image = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

  handlebox = gtk_handle_box_new ();
  gtk_box_pack_start (GTK_BOX (box), handlebox, FALSE, TRUE, 0);

  toolbar = gtk_toolbar_new ();
  gtk_container_add (GTK_CONTAINER (handlebox), toolbar);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);

  view_undo_button =
    (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-undo");
  gtk_container_add (GTK_CONTAINER (toolbar), view_undo_button);
  gtk_tooltips_set_tip (tooltips, view_undo_button, _("Undo the last action"),
			NULL);
  gtk_widget_set_sensitive (view_undo_button, FALSE);
  g_signal_connect ((gpointer) view_undo_button, "clicked",
		    G_CALLBACK (view_undo), NULL);

  view_redo_button =
    (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-redo");
  gtk_container_add (GTK_CONTAINER (toolbar), view_redo_button);
  gtk_tooltips_set_tip (tooltips, view_redo_button,
			_("Redo the undone action"), NULL);

  gtk_widget_set_sensitive (view_redo_button, FALSE);
  g_signal_connect ((gpointer) view_redo_button, "clicked",
		    G_CALLBACK (view_redo), NULL);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-find");
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  gtk_tooltips_set_tip (tooltips, button, _("Search for text"), NULL);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (view_find),
		    NULL);

  sep = (GtkWidget *) gtk_separator_tool_item_new ();
  gtk_container_add (GTK_CONTAINER (toolbar), sep);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-cut");
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  gtk_tooltips_set_tip (tooltips, button, _("Cut the selection"), NULL);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (view_cut),
		    NULL);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-copy");
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  gtk_tooltips_set_tip (tooltips, button, _("Copy the selection"), NULL);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (view_copy),
		    NULL);

  button = (GtkWidget *) gtk_tool_button_new_from_stock ("gtk-paste");
  gtk_container_add (GTK_CONTAINER (toolbar), button);
  gtk_tooltips_set_tip (tooltips, button, _("Past the clipboard"), NULL);

  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (view_paste),
		    NULL);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (box), sw, TRUE, TRUE, 0);
  lm = gtk_source_languages_manager_new ();

  old = l =
    (GSList *) gtk_source_languages_manager_get_available_languages (lm);

  while (l)
    {
      char *name;

      lang = GTK_SOURCE_LANGUAGE (l->data);

      name = gtk_source_language_get_id (lang);

      if (!strcmp (name, "XML"))
	{
	  free (name);
	  break;
	}

      free (name);
      l = l->next;
    }

  if (!l)
    return;

  g_slist_free (old);

  view_buffer =
    gtk_source_buffer_new_with_language (GTK_SOURCE_LANGUAGE (lang));

  view_source = gtk_source_view_new_with_buffer (view_buffer);
  gtk_source_view_set_show_line_numbers (GTK_SOURCE_VIEW (view_source), TRUE);
  gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW (view_source), TRUE);
  gtk_source_buffer_set_highlight (view_buffer, FALSE);
  gtk_widget_grab_focus (view_source);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (view_source), TRUE);

  gtk_container_add (GTK_CONTAINER (sw), view_source);

  g_signal_connect ((gpointer) view_buffer, "changed",
		    G_CALLBACK (view_changed), NULL);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label (_("Close without update somax-editor"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (view_quit),
		    file);

  button = gtk_button_new_with_label (_("Save and update somax-editor"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (view_close),
		    file);

  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (view_buffer), a, -1);

  g_free (a);

  gtk_window_add_accel_group (GTK_WINDOW (window), accel);
  gtk_widget_show_all (window);
  g_timeout_add (200, view_check, NULL);

  view_window = window;
}

static void
view_paste (GtkWidget * w, gpointer dummy)
{
  gtk_text_buffer_paste_clipboard (GTK_TEXT_BUFFER (view_buffer),
				   gtk_clipboard_get (GDK_NONE), NULL, TRUE);
}

static void
view_copy (GtkWidget * w, gpointer dummy)
{
  gtk_text_buffer_copy_clipboard (GTK_TEXT_BUFFER (view_buffer),
				  gtk_clipboard_get (GDK_NONE));
}

static void
view_cut (GtkWidget * w, gpointer dummy)
{
  gtk_text_buffer_cut_clipboard (GTK_TEXT_BUFFER (view_buffer),
				 gtk_clipboard_get (GDK_NONE), TRUE);
}

static void
view_find (GtkWidget * w, gpointer dummy)
{
  GtkWidget *dialog;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *entry;

  GtkWidget *casesensitive;
  GtkWidget *start_doc;
  GtkWidget *backward;

  GtkSourceSearchFlags flags;
  GtkTextIter iter;
  int forward;

  dialog =
    gtk_dialog_new_with_buttons (_("Find For Text..."), NULL,
				 GTK_DIALOG_DESTROY_WITH_PARENT,
				 GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
				 GTK_STOCK_FIND, GTK_RESPONSE_OK, NULL);

  table = gtk_table_new (0, 0, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table, FALSE,
		      FALSE, 0);

  label = gtk_label_new (_("Search For:"));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

  entry = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), entry, 1, 2, 0, 1);

  casesensitive = gtk_check_button_new_with_mnemonic (_("Match case"));
  gtk_table_attach_defaults (GTK_TABLE (table), casesensitive, 0, 2, 1, 2);

  start_doc = gtk_check_button_new_with_mnemonic (_("Search form the _head"));
  gtk_table_attach_defaults (GTK_TABLE (table), start_doc, 0, 2, 2, 3);

  backward = gtk_check_button_new_with_mnemonic (_("Search _backwards"));
  gtk_table_attach_defaults (GTK_TABLE (table), backward, 0, 2, 3, 4);

  gtk_widget_show_all (table);

  while (1)
    {

      switch (gtk_dialog_run (GTK_DIALOG (dialog)))
	{

	case GTK_RESPONSE_OK:
	  {
	    char *what;

	    flags =
	      GTK_SOURCE_SEARCH_VISIBLE_ONLY | GTK_SOURCE_SEARCH_TEXT_ONLY;
	    if (!gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (casesensitive)))
	      flags |= GTK_SOURCE_SEARCH_CASE_INSENSITIVE;

	    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (backward)))
	      forward = 0;
	    else
	      forward = 1;

	    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (start_doc)))
	      gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER
						  (view_buffer), &iter, 0);
	    else
	      {
		GtkTextIter sel_bound;

		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER
						  (view_buffer), &iter,
						  gtk_text_buffer_get_mark
						  (GTK_TEXT_BUFFER
						   (view_buffer), "insert"));

		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER
						  (view_buffer), &sel_bound,
						  gtk_text_buffer_get_mark
						  (GTK_TEXT_BUFFER
						   (view_buffer),
						   "selection_bound"));

		if (!forward)
		  gtk_text_iter_order (&sel_bound, &iter);
		else
		  gtk_text_iter_order (&iter, &sel_bound);
	      }

	    what = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));
	    if (!view_search (what, flags, forward, &iter))
	      {
		gtk_widget_destroy (dialog);
		return;
	      }

	    break;
	  }

	default:
	  gtk_widget_destroy (dialog);
	  return;
	}
    }
}

static int
view_search (char *what, GtkSourceSearchFlags flags, int forward,
	     GtkTextIter * iter)
{

  GtkTextIter mstart, mend;
  int found;

  if (!forward)
    found =
      gtk_source_iter_forward_search (iter, what, flags, &mstart, &mend,
				      NULL);
  else
    found =
      gtk_source_iter_backward_search (iter, what, flags, &mstart, &mend,
				       NULL);

  if (found)
    {
      gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (view_buffer), &mstart);
      gtk_text_buffer_move_mark_by_name (GTK_TEXT_BUFFER (view_buffer),
					 "selection_bound", &mend);
    }
  if (!found && view_search_dialog () == GTK_RESPONSE_YES)
    {
      gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (view_buffer),
					  iter, 0);
      found = view_search (what, flags, 0, iter);
    }

  return found;
}

static int
view_search_dialog (void)
{
  GtkWidget *dialog;
  GtkWidget *label;
  int ret;

  dialog = gtk_dialog_new_with_buttons (_("Find For Text..."), NULL,
					GTK_DIALOG_MODAL |
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_YES, GTK_RESPONSE_YES,
					GTK_STOCK_NO, GTK_RESPONSE_NO, NULL);
  label =
    gtk_label_new (_("No result founded.\nRestart the palinsesto file?"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, FALSE,
		      FALSE, 0);

  ret = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  return ret;
}

static void
view_undo (GtkWidget * w, gpointer dummy)
{
  if (!gtk_source_buffer_can_undo (GTK_SOURCE_BUFFER (view_buffer)))
    return;

  gtk_source_buffer_undo (GTK_SOURCE_BUFFER (view_buffer));
}

static void
view_redo (GtkWidget * w, gpointer dummy)
{
  if (!gtk_source_buffer_can_redo (GTK_SOURCE_BUFFER (view_buffer)))
    return;

  gtk_source_buffer_redo (GTK_SOURCE_BUFFER (view_buffer));
}

static void
view_changed (GtkWidget * w, gpointer dummy)
{
  view_change = TRUE;
}

static int
view_check (gpointer dummy)
{
  if (!GTK_IS_WIDGET (view_buffer))
    return FALSE;

  if (!gtk_source_buffer_can_redo (GTK_SOURCE_BUFFER (view_buffer)))
    gtk_widget_set_sensitive (view_redo_button, FALSE);
  else
    gtk_widget_set_sensitive (view_redo_button, TRUE);

  if (!gtk_source_buffer_can_undo (GTK_SOURCE_BUFFER (view_buffer)))
    gtk_widget_set_sensitive (view_undo_button, FALSE);
  else
    gtk_widget_set_sensitive (view_undo_button, TRUE);

  return TRUE;
}

static void
view_close (GtkWidget * w, gpointer dummy)
{
  gchar *buffer;
  GtkTextIter start;
  GtkTextIter end;
  struct editor_data_t *data;

  if (dialog_ask (_("Sure to update somax-editor?")) != GTK_RESPONSE_OK)
    return;

  data = editor_get_data ();

  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (view_buffer), &start,
				      0);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (view_buffer), &end);

  buffer =
    gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (view_buffer), &start, &end,
			       TRUE);

  if (data->type == TYPE_PALINSESTO)
    {
      palinsesto_free (data->pl->maker_pl);
      data->pl->maker_pl = NULL;

      if (palinsesto_parser (buffer, &data->pl->maker_pl))
	dialog_msg (_("Somad Palinsesto syntax error"));

      else
	{
	  maker_pl_refresh (data->pl);
	  maker_pl_data_show (data->pl->maker_pl, data->pl);
	  maker_pl_file_saved (data->pl);
	}
    }
  else
    {
      spot_free (data->spot->maker_spot);
      data->spot->maker_spot = NULL;

      if (spot_parser (buffer, &data->spot->maker_spot))
	dialog_msg (_("Somad Spot file syntax error"));

      else
	{
	  maker_spot_refresh (data->spot);
	  maker_spot_data_show (data->spot->maker_spot, data->spot);
	  maker_spot_file_saved (data->spot);
	}
    }

  g_free (buffer);

  /* I can't use get_widget_get_toplevel because the toplevel of a menuitem
   * is "misticaly" the GTK_MENU and not the GTK_WINDOW. */
  gtk_widget_destroy (view_window);

  if (view_output)
    {
      g_free (view_output);
      view_output = NULL;
    }

  unlink ((gchar *) dummy);
}

static void
view_quit (GtkWidget * w, gpointer dummy)
{
  if (dialog_ask (_("Sure to quit with save?")) == GTK_RESPONSE_OK)
    gtk_widget_destroy (view_window);

  if (view_output)
    {
      g_free (view_output);
      view_output = NULL;
    }

  unlink ((gchar *) dummy);
}

static void
view_new_file (GtkWidget * w, gpointer dummy)
{
  GtkTextIter start, end;

  gtk_source_buffer_begin_not_undoable_action (view_buffer);

  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (view_buffer), &start,
				      0);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (view_buffer), &end);

  gtk_text_buffer_delete (GTK_TEXT_BUFFER (view_buffer), &start, &end);

  view_change = FALSE;

  gtk_source_buffer_end_not_undoable_action (view_buffer);

}

static void
view_open (GtkWidget * w, gpointer dummy)
{
  char *f;
  char *a;

  if (!
      (f =
       file_chooser (_("Open..."), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_OPEN)))
    return;

  if (g_file_get_contents (f, &a, NULL, NULL) == FALSE)
    {
      dialog_msg (_("Error opening file."));
      g_free (f);
      return;
    }

  if (view_output)
    g_free (view_output);

  view_output = g_strdup (f);

  g_free (f);

  f = somax_to_utf8 (a);
  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (view_buffer), f, -1);
  g_free (f);

  g_free (a);
}

static void
view_save (GtkWidget * w, gpointer dummy)
{
  if (view_output)
    view_save_real ();
  else
    view_save_as (NULL, NULL);
}

static void
view_save_as (GtkWidget * w, gpointer dummy)
{
  gchar *a =
    file_chooser (_("Open a palinsesto file..."), GTK_SELECTION_SINGLE,
		  GTK_FILE_CHOOSER_ACTION_OPEN);

  if (!a)
    return;

  view_output = a;

  view_save_real ();

  g_free (a);
}

static void
view_save_real (void)
{
  FILE *fl;
  GtkTextIter start;
  GtkTextIter end;
  char *t;
  char *tmp;

  if (!(fl = fopen (view_output, "w")))
    {
      char s[1024];

      tmp = somax_to_utf8 (view_output);
      snprintf (s, sizeof (s), _("I can't write on %s file."), tmp);
      g_free (tmp);

      dialog_msg (s);
      return;
    }

  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (view_buffer), &start,
				      0);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (view_buffer), &end);

  t =
    gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (view_buffer), &start, &end,
			       TRUE);

  tmp = somax_from_utf8 (t);
  fprintf (fl, "%s", tmp);
  g_free (tmp);

  fclose (fl);
  g_free (t);

  view_change = FALSE;
}
#endif

void
editor_preferences (GtkWidget * w, gpointer dummy)
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *entry;

  gchar s[1024];
  gchar *file;
  gchar *buffer;

  snprintf (s, 1024, _("%s-editor %s - preferences"), PACKAGE, VERSION);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), s);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_default_icon_from_file (PATH_ICON, NULL);
  g_signal_connect ((gpointer) window, "delete_event",
		    G_CALLBACK (gtk_widget_destroy), NULL);

  box = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), box);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  label = gtk_label_new (_("Editor: "));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

  file =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax", "editor",
		  NULL);

  if (g_file_get_contents (file, &buffer, NULL, NULL) == TRUE)
    {
      gchar **array = g_strsplit (buffer, "\n", -1);

      if (array && array[0])
	{
	  gtk_entry_set_text (GTK_ENTRY (entry), array[0]);
	  g_strfreev (array);
	}

      g_free (buffer);
    }

  g_free (file);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock ("gtk-cancel");
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (view_preference_close), NULL);

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (view_preference_ok), entry);

  gtk_widget_show_all (window);
}

static void
view_preference_close (GtkWidget * w, gpointer dummy)
{
  gtk_widget_destroy (gtk_widget_get_toplevel (w));
}

static void
view_preference_ok (GtkWidget * w, gpointer dummy)
{
  FILE *fl;
  gchar *s;
  GtkWidget *e = (GtkWidget *) dummy;

  s = g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax", NULL);
  if (g_file_test (s, G_FILE_TEST_EXISTS) == FALSE)
    {
      g_mkdir (s, 0750);

      if (g_file_test (s, G_FILE_TEST_EXISTS) == FALSE)
	{
	  g_message (_("Error mkdir %s"), s);
	  g_free (s);
	  return;
	}
    }

  g_free (s);
  s =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax", "editor",
		  NULL);

  if (!(fl = fopen (s, "w")))
    {
      g_message (_("Error open %s"), s);
      g_free (s);
      return;
    }

  fprintf (fl, "%s\n", gtk_entry_get_text (GTK_ENTRY (e)));
  fclose (fl);
  g_free (s);

  gtk_widget_destroy (gtk_widget_get_toplevel (w));
}

static void
editor_your_view_cb (GPid pid, gint status, gboolean * flag)
{
  g_spawn_close_pid (pid);
  *flag = TRUE;
}

void
editor_your_view (GtkWidget * w, gpointer dummy)
{
  gchar *buffer;
  gchar s[1024];
  int k, len, i;
  GList *list = NULL;
  char *file;
  gboolean flag = FALSE;
  GPid pid;
  gchar **arg;
  char j[1024];
  struct editor_data_t *edit = editor_get_data ();

  i = 0;
  while (i < 1000)
    {
      snprintf (s, sizeof (s), "somax_pls_%.3d.cfg", i);
      file = g_build_path (G_DIR_SEPARATOR_S, g_get_tmp_dir (), s, NULL);

      if (g_file_test (file, G_FILE_TEST_EXISTS) == FALSE)
	break;

      g_free (file);
      i++;
    }

  if (i == 1000)
    {
      snprintf (s, sizeof (s), "somax_pls_%.3d.cfg", getrandom (0, 1000));
      file = g_build_path (G_DIR_SEPARATOR_S, g_get_tmp_dir (), s, NULL);
    }

  if (edit->type == TYPE_PALINSESTO)
    {
      if (palinsesto_save_file (file, edit->pl->maker_pl))
	{
	  dialog_msg (_("Error writing on file."));

	  editor_statusbar_lock = WAIT;
	  return;
	}
    }
  else if (edit->type == TYPE_SPOT)
    if (spot_save_file (file, edit->spot->maker_spot))
      {
	dialog_msg (_("Error writing on file."));

	editor_statusbar_lock = WAIT;
	return;
      }

  g_free (file);

  file =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), ".somax", "editor",
		  NULL);

  if (g_file_get_contents (file, &buffer, NULL, NULL) == FALSE)
    {
      dialog_msg (_("Set your editor before!"));
      g_free (file);
      return;
    }

  snprintf (j, sizeof (j), buffer);
  g_free (buffer);

  g_free (file);

  k = strlen (j);
  for (i = 0; i < k; i++)
    {
      if (j[i] == '\n')
	{
	  j[i] = 0;
	  break;
	}
    }

  len = snprintf (s, sizeof (s), "%s ", j);

  for (k = i = 0; i < len; i++)
    {
      if (s[i] != ' ' && k < sizeof (j))
	{
	  j[k++] = s[i];
	}

      else if (s[i] == ' ' && k)
	{
	  j[k] = 0;
	  list = g_list_append (list, g_strdup (j));
	  k = 0;
	}
    }

  arg = g_malloc (sizeof (char *) * (g_list_length (list) + 2));
  k = 0;

  while (list)
    {
      arg[k++] = list->data;
      list = list->next;
    }

  arg[k++] = file;
  arg[k] = NULL;

  gtk_widget_set_sensitive ((GtkWidget *) dummy, FALSE);
  while (gtk_events_pending ())
    gtk_main_iteration ();

  if (g_spawn_async (NULL, arg, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) editor_your_view_cb, &flag);

  while (flag == FALSE)
    {
      while (gtk_events_pending ())
	gtk_main_iteration ();
      g_usleep (1000);
    }

  if (edit->type == TYPE_PALINSESTO)
    {
      palinsesto_free (edit->pl->maker_pl);
      edit->pl->maker_pl = NULL;

      if (palinsesto_parser_file (file, &edit->pl->maker_pl))
	dialog_msg (_("Somad Palinsesto syntax error"));

      else
	{
	  maker_pl_refresh (edit->pl);

	  maker_pl_data_show (edit->pl->maker_pl, edit->pl);
	  maker_pl_file_saved (edit->pl);
	}
    }

  else if (edit->type == TYPE_SPOT)
    {
      spot_free (edit->spot->maker_spot);
      edit->spot->maker_spot = NULL;

      if (spot_parser_file (file, &edit->spot->maker_spot))
	dialog_msg (_("Somad Spot file syntax error"));

      else
	{
	  maker_spot_refresh (edit->spot);
	  maker_spot_data_show (edit->spot->maker_spot, edit->spot);
	  maker_spot_file_saved (edit->spot);
	}
    }

  gtk_widget_set_sensitive ((GtkWidget *) dummy, TRUE);
  while (gtk_events_pending ())
    gtk_main_iteration ();

  unlink (file);
}

/* EOF */
