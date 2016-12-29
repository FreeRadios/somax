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

static void editor_open_pl_file (char *file);
static void editor_open_spot_file (char *file);
static void editor_pl_save_as (struct editor_data_t *);
static void editor_spot_save_as (struct editor_data_t *);
static void editor_save_pl (struct editor_data_t *data);
static void editor_save_spot (struct editor_data_t *data);
static void editor_close_pl (struct editor_data_t *data);
static void editor_close_spot (struct editor_data_t *data);

gboolean
editor_open_file (char *file)
{
  char s[1024];

  if (!file)
    return FALSE;

  if (!palinsesto_check_file (file))
    editor_open_pl_file (file);

  else if (!spot_check_file (file))
    editor_open_spot_file (file);

  else
    {
      snprintf (s, sizeof (s), _("Error opening file %s!"), file);
      dialog_msg (s);

      return FALSE;
    }

  return TRUE;
}

static void
editor_open_pl_file (char *file)
{
  struct maker_pl_data_t *data;
  struct editor_data_t *tmp;

  tmp = editor_data;
  while (tmp)
    {
      if (tmp->type == TYPE_PALINSESTO && tmp->pl->maker_pl_file
	  && !strcmp (tmp->pl->maker_pl_file, file))
	{
	  dialog_msg (_("This file is already opened!"));
	  return;
	}
      tmp = tmp->next;
    }

  data = g_malloc0 (sizeof (struct maker_pl_data_t));

  if (palinsesto_parser_file (file, &data->maker_pl))
    {
      dialog_msg (_("Somad Palinsesto syntax error"));
      g_free (data);
    }

  else
    {
      data->maker_pl_file = g_strdup (file);

      editor_new_maker_pl (data);
      maker_pl_file_saved (data);
    }
}

static void
editor_open_spot_file (char *file)
{
  struct maker_spot_data_t *data;
  struct editor_data_t *tmp;

  tmp = editor_data;
  while (tmp)
    {
      if (tmp->type == TYPE_SPOT && tmp->spot->maker_spot_file
	  && !strcmp (tmp->spot->maker_spot_file, file))
	{
	  dialog_msg (_("This file is already opened!"));
	  return;
	}
      tmp = tmp->next;
    }

  data = g_malloc0 (sizeof (struct maker_spot_data_t));

  if (spot_parser_file (file, &data->maker_spot))
    {
      dialog_msg (_("Somad Spot file syntax error"));
      g_free (data);
    }
  else
    {

      data->maker_spot_file = g_strdup (file);

      editor_new_maker_spot (data);
      maker_spot_file_saved (data);
    }
}

void
editor_open (GtkWidget * w, gpointer dummy)
{
  char *file;
  char s[1024];

  editor_statusbar_lock = LOCK;

  statusbar_set (_("Open new file..."));

  if (!
      (file =
       file_chooser (_("Open a new file..."), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_OPEN)))
    {
      statusbar_set (_("Open file: aborted"));
      editor_statusbar_lock = WAIT;
      return;
    }

  if (editor_open_file (file) == FALSE)
    {
      gchar *tmp;

      tmp = somax_to_utf8 (file);
      snprintf (s, sizeof (s), _("File %s opened"), tmp);
      g_free (tmp);

      statusbar_set (s);
    }

  editor_statusbar_lock = WAIT;

  g_free (file);
}

void
editor_save_as (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();

  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_spot_save_as (data);
  else
    editor_pl_save_as (data);
}

static void
editor_pl_save_as (struct editor_data_t *data)
{
  char *file;
  char s[1024];
  char *tmp;

  editor_statusbar_lock = LOCK;

  statusbar_set (_("Save the new file..."));

  if (!
      (file =
       file_chooser (_("Save your palinsesto"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SAVE)))
    {
      statusbar_set (_("Save file: aborted"));
      editor_statusbar_lock = WAIT;

      return;
    }

  if (g_file_test (file, G_FILE_TEST_EXISTS) == TRUE
      && dialog_ask (_("The file exists. Overwrite?")) != GTK_RESPONSE_OK)
    {
      statusbar_set (_("Save file: aborted"));
      editor_statusbar_lock = WAIT;
      g_free (file);

      return;
    }

  maker_pl_sync (data->pl);

  if (palinsesto_save_file (file, data->pl->maker_pl))
    {
      dialog_msg (_("Error writing on file."));

      statusbar_set (_("Saved: error"));
      editor_statusbar_lock = WAIT;
      g_free (file);

      return;
    }

  maker_pl_file_saved (data->pl);

  if (data->pl->maker_pl_file)
    g_free (data->pl->maker_pl_file);

  data->pl->maker_pl_file = g_strdup (file);

  tmp = somax_to_utf8 (file);
  snprintf (s, sizeof (s), _("Pl: %s"), editor_parse_file (tmp));
  g_free (tmp);

  gtk_label_set_text (GTK_LABEL (data->label), s);

  statusbar_set (_("File saved!"));

  editor_statusbar_lock = WAIT;
  g_free (file);
}

static void
editor_spot_save_as (struct editor_data_t *data)
{
  char *file;
  char s[1024];
  char *tmp;

  editor_statusbar_lock = LOCK;

  statusbar_set (_("Save the new file..."));

  if (!
      (file =
       file_chooser (_("Save your spot file"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SAVE)))
    {
      statusbar_set (_("Save file: aborted"));
      editor_statusbar_lock = WAIT;

      return;
    }

  if (g_file_test (file, G_FILE_TEST_EXISTS) == TRUE
      && dialog_ask (_("The file exists. Overwrite?")) != GTK_RESPONSE_OK)
    {
      statusbar_set (_("Save file: aborted"));
      editor_statusbar_lock = WAIT;
      g_free (file);

      return;
    }

  maker_spot_sync (data->spot);

  if (spot_save_file (file, data->spot->maker_spot))
    {
      dialog_msg (_("Error writing on file."));

      statusbar_set (_("Saved: error"));
      editor_statusbar_lock = WAIT;
      g_free (file);

      return;
    }

  maker_spot_file_saved (data->spot);

  if (data->spot->maker_spot_file)
    g_free (data->spot->maker_spot_file);

  data->spot->maker_spot_file = g_strdup (file);

  tmp = somax_to_utf8 (file);
  snprintf (s, sizeof (s), _("Spot: %s"), editor_parse_file (tmp));
  g_free (tmp);

  gtk_label_set_text (GTK_LABEL (data->label), s);

  statusbar_set (_("File saved!"));

  editor_statusbar_lock = WAIT;
  g_free (file);
}

void
editor_save (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();

  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_save_spot (data);
  else
    editor_save_pl (data);
}

static void
editor_save_pl (struct editor_data_t *data)
{
  if (!data->pl->maker_pl_file)
    {
      editor_save_as (NULL, NULL);
      return;
    }

  editor_statusbar_lock = LOCK;

  statusbar_set (_("File saving..."));
  maker_pl_sync (data->pl);

  if (palinsesto_save_file (data->pl->maker_pl_file, data->pl->maker_pl))
    {
      dialog_msg (_("Error writing on file."));

      statusbar_set (_("Save: error"));
      editor_statusbar_lock = WAIT;

      return;
    }

  maker_pl_file_saved (data->pl);
  statusbar_set (_("File saved!"));

  editor_statusbar_lock = WAIT;
}

static void
editor_save_spot (struct editor_data_t *data)
{
  if (!data->spot->maker_spot_file)
    {
      editor_save_as (NULL, NULL);
      return;
    }

  editor_statusbar_lock = LOCK;

  statusbar_set (_("File saving..."));
  maker_spot_sync (data->spot);

  if (spot_save_file (data->spot->maker_spot_file, data->spot->maker_spot))
    {
      dialog_msg (_("Error writing on file."));

      statusbar_set (_("Save: error"));
      editor_statusbar_lock = WAIT;

      return;
    }

  maker_spot_file_saved (data->spot);
  statusbar_set (_("File saved!"));

  editor_statusbar_lock = WAIT;
}

void
editor_close (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();

  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_close_spot (data);
  else
    editor_close_pl (data);
}

static void
editor_close_pl (struct editor_data_t *data)
{
  struct editor_data_t *tmp;

  if ((maker_pl_is_no_saved (data->pl)
       && dialog_ask (_("Exit without save?")) == GTK_RESPONSE_OK)
      || !maker_pl_is_no_saved (data->pl))
    {

      editor_statusbar_lock = LOCK;
      statusbar_set (_("Closing..."));

      if (data == editor_data)
	editor_data = data->next;

      else
	{
	  tmp = editor_data;
	  while (tmp->next)
	    {
	      if (tmp->next == data)
		{
		  tmp->next = data->next;
		  break;
		}
	      tmp = tmp->next;
	    }
	}

      gtk_notebook_remove_page (GTK_NOTEBOOK (notebook),
				gtk_notebook_get_current_page (GTK_NOTEBOOK
							       (notebook)));
      palinsesto_free (data->pl->maker_pl);
      g_free (data->pl);
      g_free (data);

      statusbar_set (_("Closed"));
      editor_statusbar_lock = WAIT;
    }
}

static void
editor_close_spot (struct editor_data_t *data)
{
  struct editor_data_t *tmp;

  if ((maker_spot_is_no_saved (data->spot)
       && dialog_ask (_("Exit without save?")) == GTK_RESPONSE_OK)
      || !maker_spot_is_no_saved (data->spot))
    {

      editor_statusbar_lock = LOCK;
      statusbar_set (_("Closing..."));

      if (data == editor_data)
	editor_data = data->next;

      else
	{
	  tmp = editor_data;
	  while (tmp->next)
	    {
	      if (tmp->next == data)
		{
		  tmp->next = data->next;
		  break;
		}
	      tmp = tmp->next;
	    }
	}

      gtk_notebook_remove_page (GTK_NOTEBOOK (notebook),
				gtk_notebook_get_current_page (GTK_NOTEBOOK
							       (notebook)));
      spot_free (data->spot->maker_spot);
      g_free (data->spot);
      g_free (data);

      statusbar_set (_("Closed"));
      editor_statusbar_lock = WAIT;
    }
}

void
editor_new (GtkWidget * w, gpointer dummy)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *button;
  int ret;

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), _("New file..."));
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  hbox = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE,
		      0);

  image = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Palinsesto file or spot file?"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  button = gtk_button_new_with_mnemonic (_("Palinsesto File"));
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_NO);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  button = gtk_button_new_with_mnemonic (_("Spot File"));
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  ret = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  if (ret != GTK_RESPONSE_OK && ret != GTK_RESPONSE_NO)
    return;

  if (ret == GTK_RESPONSE_NO)
    editor_new_pl ();
  else
    editor_new_sp ();
}

void
editor_new_pl (void)
{
  struct maker_pl_data_t *data;

  data = g_malloc0 (sizeof (struct maker_pl_data_t));

  editor_new_maker_pl (data);
  statusbar_set (_("New editor created"));
}

void
editor_new_sp (void)
{
  struct maker_spot_data_t *data;

  data = g_malloc0 (sizeof (struct maker_spot_data_t));

  editor_new_maker_spot (data);
  statusbar_set (_("New editor created"));
}

/* EOF */
