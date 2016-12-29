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
#include "filechooser.h"

static void
file_chooser_response (GtkWidget * w, gpointer dummy)
{
  gtk_dialog_response (GTK_DIALOG (w), GTK_RESPONSE_ACCEPT);
}

void *
file_chooser (char *title, GtkSelectionMode w, GtkFileChooserAction t)
{
  GtkWidget *dialog;
  static char *path = NULL;
  void *l = NULL;

  dialog = gtk_file_chooser_dialog_new (title, NULL, t,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					t ==
					GTK_FILE_CHOOSER_ACTION_SAVE ?
					GTK_STOCK_SAVE : GTK_STOCK_OPEN,
					GTK_RESPONSE_ACCEPT, NULL);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  g_signal_connect ((gpointer) dialog, "file_activated",
		    G_CALLBACK (file_chooser_response), NULL);

  if (w == GTK_SELECTION_MULTIPLE)
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);
  else
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);

  if (!path)
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
					 g_get_home_dir ());
  else
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), path);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      if (w == GTK_SELECTION_MULTIPLE)
	{
	  l = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (dialog));

	  if (l)
	    {

	      if (path)
		g_free (path);

	      path = g_path_get_dirname ((gchar *) ((GSList *) l)->data);
	    }
	}
      else
	{
	  l = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

	  if (l)
	    {
	      if (path)
		g_free (path);

	      path = g_path_get_dirname ((gchar *) l);
	    }
	}
    }

  gtk_widget_destroy (dialog);
  return l;
}

void
file_chooser_cb (char *title, GtkSelectionMode w, GtkFileChooserAction t,
		 void (*func) (void *, void *), void *what)
{
  GtkWidget *dialog;
  GtkWidget *cb;
  static char *path = NULL;
  void *l = NULL;
  static int cb_set = 0;

  if (!func)
    return;

  dialog = gtk_file_chooser_dialog_new (title, NULL, t,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					t ==
					GTK_FILE_CHOOSER_ACTION_SAVE ?
					GTK_STOCK_SAVE : GTK_STOCK_OPEN,
					GTK_RESPONSE_ACCEPT, NULL);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  g_signal_connect ((gpointer) dialog, "file_activated",
		    G_CALLBACK (file_chooser_response), NULL);

  cb =
    gtk_check_button_new_with_label (_
				     ("Don't close this window after usage"));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), cb, FALSE, FALSE,
		      0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb), cb_set);
  gtk_widget_show (cb);

  if (w == GTK_SELECTION_MULTIPLE)
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);
  else
    gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);

  if (!path)
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
					 g_get_home_dir ());
  else
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), path);

  gtk_widget_show (dialog);

  while (1)
    {
      if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_ACCEPT)
	break;

      cb_set = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cb));

      if (w == GTK_SELECTION_MULTIPLE)
	{
	  l = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (dialog));

	  if (l)
	    {
	      if (path)
		g_free (path);

	      path = g_path_get_dirname ((gchar *) ((GSList *) l)->data);
	    }
	}
      else
	{
	  l = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

	  if (l)
	    {
	      if (path)
		g_free (path);

	      path = g_path_get_dirname ((gchar *) l);
	    }
	}

      func (l, what);

      if (!cb_set)
	break;
    }

  gtk_widget_destroy (dialog);
}

/* EOF */
