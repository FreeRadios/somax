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

#include "plugin.h"
#include "plugin_internal.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

static gboolean dont_save = FALSE;
static char **env;

gboolean
key_event (GtkWidget * w, GdkEventKey * event, gpointer dummy)
{
  if (event->keyval == GDK_Escape)
    {
      gtk_main_quit ();
      return TRUE;
    }

  return FALSE;
}

void
save (char *what)
{
  gchar *s;
  FILE *fl;

  if (dont_save == TRUE)
    return;

  s = g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), "./somax", NULL);
  if (g_file_test (s, G_FILE_TEST_EXISTS) == FALSE)
    {
      g_mkdir (s, 0750);

      if (g_file_test (s, G_FILE_TEST_IS_DIR) == FALSE)
	{
	  g_message ("Error mkdir %s", s);
	  g_free (s);
	  return;
	}
    }

  g_free (s);
  s =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), "./somax",
		  "somax_edit_plugin", NULL);

  if (!(fl = fopen (s, "a")))
    {
      g_message ("Error opening %s", s);
      g_free (s);
      return;
    }

  fprintf (fl, "%s\n", what);
  fclose (fl);
  g_free (s);
}

void
populate (GtkWidget * combo)
{
  gchar *s;
  gchar *buffer;
  gchar **array;
  int i;

  s =
    g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), "./somax",
		  "somax_edit_plugin", NULL);

  if(g_file_get_contents(s, &buffer, NULL, NULL)==FALSE) {
	  g_free(s);
	  return;
  }

  g_free (s);

  for(i=0,array=g_strsplit(buffer, "\n", -1);array && array[i]; i++)
	      gtk_combo_box_prepend_text (GTK_COMBO_BOX (combo), array[i]);

      if (i > 10)
	dont_save = TRUE;

  g_free (buffer);
  g_strfreev(array);
}

void
edit (GtkWidget * w, GtkWidget * e)
{
  char **arg;
  gchar *what;
  gchar s[1024];
  int k, i;
  int len;
  char j[1024];
  GList *list = NULL;

  what = (gchar *) gtk_entry_get_text (GTK_ENTRY (GTK_BIN (e)->child));
  gtk_widget_hide (gtk_widget_get_toplevel (w));

  while (gtk_events_pending ())
    gtk_main_iteration ();

  snprintf (s, sizeof (s), "%s ", what);
  len = strlen (s);

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

  arg[k++] = somax_plgn_get_file ();
  arg[k] = NULL;

  if (fork ())
    {
      execve (arg[0], arg, env);
      exit (0);
    }

  if (what)
    save (what);

  gtk_main_quit ();
}


int
main (int argc, char **argv, char **e)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *combo;

  env = e;

  g_thread_init (NULL);
  gtk_init (NULL, NULL);

  somax_plgn_check_arg (argc, argv);
  somax_plgn_set_description ("Edit with...");
  somax_plgn_set_author ("Andrea Marchesini");
  somax_plgn_set_name ("somax_editor_plugin");
  somax_plgn_set_licence ("GPL");
  somax_plgn_set_version ("0.1");

  somax_plgn_print_data ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Edit with...");
  g_signal_connect ((gpointer) window, "delete_event",
		    G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect ((gpointer) window, "key-press-event",
		    GTK_SIGNAL_FUNC (key_event), NULL);

  frame = gtk_frame_new ("Edit with...");
  gtk_container_add (GTK_CONTAINER (window), frame);

  box = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), box);

  label = gtk_label_new ("Program:");
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  combo = gtk_combo_box_entry_new_text ();
  gtk_box_pack_start (GTK_BOX (box), combo, FALSE, FALSE, 0);

  populate (combo);

  button = gtk_button_new_with_mnemonic ("Edit");
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (edit), combo);

  gtk_widget_show_all (window);

  gtk_main ();

  somax_plgn_free ();

  return 0;
}

/* EOF */
