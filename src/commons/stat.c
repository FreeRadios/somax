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

#include "commons.h"
#include "stat.h"
#include "../ghthash/ght_hash_table.h"

#define HASH_ELEMENT 256
#define HASH_MAX_SIZE 8192

#define LABEL_NEW( x ) label = gtk_label_new (x); \
  gtk_widget_show (label); \
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0); \
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

#define DATA_STRING( x ) tmp = somax_markup (x); \
  snprintf (s, sizeof (s), "<b>%s</b>", tmp); \
  g_free(tmp);

#define DATA_NEW( x )  label = gtk_label_new (s); \
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE); \
  gtk_widget_show (label); \
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, i, i+1, \
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), \
                    (GtkAttachOptions) (0), 0, 0); \
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);

static GMutex *mutex;

static ght_hash_table_t *hash;

static void stat_empty (void);
static void stat_request (soma_controller * controller, char *file);

/* Inizialize the hash table */
void
stat_init (void)
{
  mutex = g_mutex_new ();

  g_mutex_lock (mutex);
  hash = ght_create (HASH_ELEMENT);
  g_mutex_unlock (mutex);
}

/* Check a list of items or spots and ask the stats about these elements */
void
stat_check (soma_controller * controller, char **stat)
{
  somax_stat_t *tmp;

  if (!stat || !*stat)
    return;

  g_mutex_lock (mutex);

  if (ght_size (hash) >= HASH_MAX_SIZE)
    stat_empty ();

  g_mutex_unlock (mutex);

  while (*stat)
    {
      if (!(tmp = stat_get (*stat)))
	stat_request (controller, *stat);
      else
	stat_unref (tmp);

      stat++;
    }
}

/* Return the stat if it exists in the hash table */
somax_stat_t *
stat_get (char *file)
{
  somax_stat_t *stat;

  if (!file)
    return NULL;

  g_mutex_lock (mutex);

  stat = ght_get (hash, strlen (file), file);

  /* Reference ++ */
  if (stat)
    stat->ref++;

  g_mutex_unlock (mutex);

  return stat;
}

/* Get stats from the soma daemon */
static void
stat_request (soma_controller * controller, char *file)
{
  soma_stat *stat;
  somax_stat_t *data;

  if (!file)
    return;

  if (*file == DISTRIBUITED_CHAR)
    {
      if (!(stat = soma_get_stat_path (controller, file + 1)))
	return;
    }

  else if (!(stat = soma_get_stat (controller, file)))
    return;

  g_mutex_lock (mutex);

  data = (somax_stat_t *) g_malloc0 (sizeof (somax_stat_t));
  data->stat = stat;

  ght_insert (hash, data, strlen (file), file);

  g_mutex_unlock (mutex);
}

/* Remove all elements in the hash table */
static void
stat_empty (void)
{
  ght_iterator_t iterator;
  void *key;
  somax_stat_t *stat;

  for (stat = (somax_stat_t *) ght_first (hash, &iterator, &key); stat;
       stat = (somax_stat_t *) ght_next (hash, &iterator, &key))
    {
      if (!stat->ref)
	{
	  ght_remove (hash, strlen (key), key);
	  soma_stat_free (stat->stat);
	  free (stat);
	}
    }
}

void
stat_ref (somax_stat_t * stat)
{
  g_mutex_lock (mutex);

  if (!stat)
    {
      g_mutex_unlock (mutex);
      return;
    }

  stat->ref++;

  g_mutex_unlock (mutex);
}

void
stat_unref (somax_stat_t * stat)
{
  g_mutex_lock (mutex);

  if (!stat || stat->ref <= 0)
    {
      g_mutex_unlock (mutex);
      return;
    }

  stat->ref--;
  g_mutex_unlock (mutex);
}

/* Return the duration string from a int64_t */
char *
stat_duration (int64_t duration)
{
  static char buf[1024];
  int hours, mins, secs, us;

  secs = duration / SOMA_TIME_BASE;
  us = duration % SOMA_TIME_BASE;
  mins = secs / 60;
  secs %= 60;
  hours = mins / 60;
  mins %= 60;

  snprintf (buf, sizeof (buf), "%02d:%02d:%02d.%01d", hours, mins, secs,
	    (10 * us) / SOMA_TIME_BASE);

  return buf;
}

/* Draw a popup with the stat of a file */
void
stat_popup (char *file)
{
  somax_stat_t *stat;

  if (!(stat = stat_get (file)))
    {
      dialog_msg (_("No information about this file!"));
      return;
    }

  stat_struct_popup (stat->stat);
  stat_unref (stat);
}

/* Draw a popup with the stat of a data struct soma_stat */
void
stat_struct_popup (soma_stat * stat)
{
  GtkWidget *dialog;
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *button;
  char s[1024];
  gchar *tmp;
  int i = 0;
  int j = 0;
  soma_stat_stream *stream;

  if (!stat)
    {
      dialog_msg (_("No information about this file!"));
      return;
    }

  snprintf (s, sizeof (s), "%s %s - %s", PACKAGE, VERSION,
	    stat_make_name (stat->filename));

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), s);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  frame = gtk_frame_new (stat_make_name (stat->filename));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE,
		      0);

  table = gtk_table_new (0, 0, FALSE);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (frame), table);

  LABEL_NEW (_("Filename:"));
  DATA_STRING (stat->filename);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Title:"));
  DATA_STRING (stat->title);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Author:"));
  DATA_STRING (stat->author);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Copyright:"));
  DATA_STRING (stat->copyright);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Comment:"));
  DATA_STRING (stat->comment);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Album:"));
  DATA_STRING (stat->album);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Year:"));
  snprintf (s, sizeof (s), "<b>%d</b>", stat->year);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Track:"));
  snprintf (s, sizeof (s), "<b>%d</b>", stat->track);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Genre:"));
  DATA_STRING (stat->genre);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Start Time:"));
  snprintf (s, sizeof (s), "<b>%s</b>", stat_duration (stat->start_time));
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Duration:"));
  snprintf (s, sizeof (s), "<b>%s</b>", stat_duration (stat->duration));
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Bitrate:"));
  snprintf (s, sizeof (s), "<b>%d</b>", stat->bitrate);
  DATA_NEW (s);
  i++;

  stream = stat->streams;

  while (stream)
    {
      snprintf (s, sizeof (s), _("Stream %d:"), ++j);
      LABEL_NEW (s);
      DATA_STRING (stream->stream);
      DATA_NEW (s);
      i++;

      stream = stream->next;
    }

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

/* Draw a popup with the stat of a data struct soma_stat_dir */
void
stat_struct_dir_popup (soma_stat_dir * stat)
{
  GtkWidget *dialog;
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *button;
  char s[1024];
  int i = 0;

  if (!stat)
    {
      dialog_msg (_("No information about this directory!"));
      return;
    }

  snprintf (s, sizeof (s), "%s %s - %s", PACKAGE, VERSION,
	    stat_make_name (stat->dirname));

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), s);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  frame = gtk_frame_new (stat_make_name (stat->dirname));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE,
		      0);

  table = gtk_table_new (0, 0, FALSE);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (frame), table);

  LABEL_NEW (_("Dirname:"));

  snprintf (s, sizeof (s), "<b>%s</b>", stat->dirname);
  DATA_NEW (s);
  i++;

  LABEL_NEW (_("Duration:"));

  snprintf (s, sizeof (s), "<b>%s</b>", stat_duration (stat->duration));
  DATA_NEW (s);
  i++;

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (button);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

char *
stat_make_name (char *file)
{
  int i;

  if (!strncmp (file, "http://", 7) ||
      !strncmp (file, "https://", 8) ||
      !strncmp (file, "ftp://", 6) || !strncmp (file, "ftps://", 7))
    return file;

  // -2 because -1 is '\0' and -2 is a possible '/' of a directory
  for (i = strlen (file) - 2; i >= 0; i--)
    if (file[i] == '/')
      return file + i + 1;

  return file;
}

void
stat_new (soma_controller * controller, char *file)
{
  somax_stat_t *tmp;

  if (!(tmp = stat_get (file)))
    stat_request (controller, file);
  else
    stat_unref (tmp);
}

/* EOF */
