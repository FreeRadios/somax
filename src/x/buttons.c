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
#include "../palinsesto/palinsesto.h"
#include "../filechooser/filechooser.h"

static char *current_item_file = NULL;
static char *next_item_file = NULL;
static struct somad_data *palinsesto_name_pl = NULL;

static void palinsesto_copy (void);
static void spot_copy (void);
static void button_item (char *item, char **previous, char **item_file,
			 gboolean current, GtkWidget * button_item);

void
onlypause (GtkWidget * w, gpointer dummy)
{
  if (status_pause)
    somax_set_unpause (controller);
  else
    somax_set_pause (controller);

  timeout (NULL);
}

void
pauseskip (GtkWidget * w, gpointer dummy)
{
  somax_set_pause (controller);
  somax_skip (controller);
  timeout (NULL);
}

void
startstop (GtkWidget * w, gpointer dummy)
{
  if (!status_stop)
    somax_stop (controller, -1);
  else
    somax_start (controller);

  timeout (NULL);
}

void
skip (GtkWidget * w, gpointer dummy)
{
  somax_skip (controller);
  timeout (NULL);
}

void
skip_next (GtkWidget * w, gpointer dummy)
{
  somax_skip_next (controller);
  timeout (NULL);
}

void
read_directory (GtkWidget * w, gpointer dummy)
{
  somax_read_directory (controller);

  timeout (NULL);
}

void
read_palinsesto (GtkWidget * w, gpointer dummy)
{
  somax_read_palinsesto (controller);

  timeout (NULL);
}

void
read_spot (GtkWidget * w, gpointer dummy)
{
  somax_read_spot (controller);

  timeout (NULL);
}

void
old_palinsesto (GtkWidget * w, gpointer dummy)
{
  somax_old_palinsesto (controller);

  timeout (NULL);
}

void
old_spot (GtkWidget * w, gpointer dummy)
{
  somax_old_spot (controller);

  timeout (NULL);
}

void
stop_for (GtkWidget * w, gpointer dummy)
{
  int sec = 0;
  int d;

  d = gtk_spin_button_get_value (GTK_SPIN_BUTTON (stop_for_data.hours));
  if (d < 0)
    d = 0;
  else if (d > 99)
    d = 99;
  if (d)
    sec += d * 3600;

  d = gtk_spin_button_get_value (GTK_SPIN_BUTTON (stop_for_data.minutes));
  if (d < 0)
    d = 0;
  else if (d > 59)
    d = 59;
  if (d)
    sec += d * 60;

  d = gtk_spin_button_get_value (GTK_SPIN_BUTTON (stop_for_data.seconds));
  if (d < 0)
    d = 0;
  else if (d > 59)
    d = 59;
  sec += d;

  if (!sec)
    sec = -1;

  somax_stop (controller, sec);

  timeout (NULL);
}

void
s_shutdown (GtkWidget * w, GtkWidget * e)
{
  if (dialog_ask (_("Sure to quit somad?")) != GTK_RESPONSE_OK)
    return;

  set_quit = TRUE;

  somax_quit (controller);

  gtk_main_quit ();
}

void
nextitem_clicked (GtkWidget * w, gpointer dummy)
{
  gchar *a[2];
  GPid pid;

  a[0] = NEXTITEM;
  a[1] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

void
editor_clicked (GtkWidget * w, gpointer dummy)
{
  gchar *a[3];
  GPid pid;

  a[0] = EDITOR;
  a[1] = "-pl";
  a[2] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

void
config_clicked (GtkWidget * w, gpointer dummy)
{
  gchar *a[2];
  GPid pid;

  a[0] = CONFIG;
  a[1] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

void
defaultthis_clicked (GtkWidget * w, gpointer dummy)
{
  if (dialog_ask (_("Sure to set the current palinsesto as default?")) !=
      GTK_RESPONSE_OK)
    return;

  if (dialog_ask (_("Save old palinsesto in a local file?")) ==
      GTK_RESPONSE_OK)
    {
      gchar *file;
      char *buf;
      FILE *fl;

      if (!
	  (file =
	   file_chooser (_("Save your palinsesto"), GTK_SELECTION_SINGLE,
			 GTK_FILE_CHOOSER_ACTION_SAVE)))
	{
	  dialog_msg (_("Action aborted!"));
	  return;
	}

      buf = somax_get_old_palinsesto (controller);

      if (!(fl = fopen (file, "w")))
	{
	  dialog_msg (_("Error opening your file. Abort!"));
	  g_free (file);
	  return;
	}

      if (buf)
	{
	  fprintf (fl, "%s", buf);
	  g_free (buf);
	}

      fclose (fl);

      g_free (file);
    }


  if (somax_set_default_palinsesto (controller))
    dialog_msg (_("Error setting current palinsesto file as default"));

  else
    dialog_msg (_("Set current palinsesto file as default: done"));

}

void
editthis_clicked (GtkWidget * w, gpointer dummy)
{
  if (maker_pl_data.maker_pl)
    palinsesto_free (maker_pl_data.maker_pl);

  maker_pl_data.maker_pl = NULL;
  palinsesto_copy ();

  win_maker_pl_create ();
}

static void
palinsesto_copy (void)
{
  struct somad_data *pl, *old = NULL;
  struct somad_data *tmp;
  GList *list;

  tmp = somad_pl;

  while (tmp)
    {
      pl = (struct somad_data *) g_malloc (sizeof (struct somad_data));

      if (!old)
	maker_pl_data.maker_pl = pl;

      memcpy (pl, tmp, sizeof (struct somad_data));

      if (tmp->description)
	pl->description = g_strdup (tmp->description);

      pl->color = g_malloc (sizeof (GdkColor));
      memcpy (pl->color, tmp->color, sizeof (GdkColor));

      pl->timer = g_malloc (sizeof (struct somad_time));
      memcpy (pl->timer, tmp->timer, sizeof (struct somad_time));

      if (tmp->timer->start)
	pl->timer->start = g_strdup (tmp->timer->start);

      if (tmp->timer->stop)
	pl->timer->stop = g_strdup (tmp->timer->stop);

      if (tmp->module)
	pl->module = g_strdup (tmp->module);

      if (tmp->moduledata)
	pl->moduledata = g_strdup (tmp->moduledata);

      if (tmp->stream)
	pl->stream = g_strdup (tmp->stream);

      if (tmp->jingle)
	pl->jingle = g_strdup (tmp->jingle);

      if (tmp->prespot)
	pl->prespot = g_strdup (tmp->prespot);

      if (tmp->postspot)
	pl->postspot = g_strdup (tmp->postspot);

      list = tmp->pathitem;
      pl->pathitem = NULL;
      while (list)
	{
	  pl->pathitem =
	    g_list_append (pl->pathitem, g_strdup ((gchar *) list->data));
	  list = list->next;
	}

      list = tmp->pathspot;
      pl->pathspot = NULL;
      while (list)
	{
	  pl->pathspot =
	    g_list_append (pl->pathspot, g_strdup ((gchar *) list->data));
	  list = list->next;
	}

      pl->next = NULL;

      if (old)
	old->next = pl;

      old = pl;

      tmp = tmp->next;
    }
}

static void
spot_copy (void)
{
  struct somad_spot_data *spot, *old = NULL;
  struct somad_spot_data *tmp;
  GList *list;

  tmp = somad_spot;

  while (tmp)
    {
      spot =
	(struct somad_spot_data *) g_malloc (sizeof (struct somad_spot_data));

      if (!old)
	maker_spot_data.maker_spot = spot;

      memcpy (spot, tmp, sizeof (struct somad_spot_data));

      if (tmp->description)
	spot->description = g_strdup (tmp->description);

      spot->timer = g_malloc (sizeof (struct somad_time));
      memcpy (spot->timer, tmp->timer, sizeof (struct somad_time));

      if (tmp->timer->start)
	spot->timer->start = g_strdup (tmp->timer->start);

      if (tmp->timer->stop)
	spot->timer->stop = g_strdup (tmp->timer->stop);

      list = tmp->path;
      spot->path = NULL;
      while (list)
	{
	  spot->path =
	    g_list_append (spot->path, g_strdup ((gchar *) list->data));
	  list = list->next;
	}

      spot->next = NULL;

      if (old)
	old->next = spot;

      old = spot;

      tmp = tmp->next;
    }
}

void
spotdefaultthis_clicked (GtkWidget * w, gpointer dummy)
{
  if (dialog_ask (_("Sure to set the current spot file as default?")) !=
      GTK_RESPONSE_OK)
    return;

  if (dialog_ask (_("Save old spot file in a local file?")) ==
      GTK_RESPONSE_OK)
    {
      gchar *file;
      char *buf;
      FILE *fl;

      if (!
	  (file =
	   file_chooser (_("Save your spot file"), GTK_SELECTION_SINGLE,
			 GTK_FILE_CHOOSER_ACTION_SAVE)))
	{
	  dialog_msg (_("Action aborted!"));
	  return;
	}

      buf = somax_get_old_spot (controller);

      if (!(fl = fopen (file, "w")))
	{
	  dialog_msg (_("Error opening your file. Abort!"));
	  g_free (file);
	  return;
	}

      if (buf)
	{
	  fprintf (fl, "%s", buf);
	  g_free (buf);
	}

      fclose (fl);
      g_free (file);
    }

  if (somax_set_default_spot (controller))
    dialog_msg (_("Error setting current spot file as default"));

  else
    dialog_msg (_("Set current spot file as default: done"));

}

void
spoteditthis_clicked (GtkWidget * w, gpointer dummy)
{
  if (maker_spot_data.maker_spot)
    spot_free (maker_spot_data.maker_spot);

  maker_spot_data.maker_spot = NULL;
  spot_copy ();

  win_maker_spot_create ();
}

void
spoteditor_clicked (GtkWidget * w, gpointer dummy)
{
  gchar *a[3];
  GPid pid;

  a[0] = EDITOR;
  a[1] = "-sp";
  a[2] = NULL;

  if (g_spawn_async (NULL, a, NULL, 0, NULL, NULL, &pid, NULL) == TRUE)
    g_child_watch_add (pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
}

void
savepl_clicked (GtkWidget * w, gpointer dummy)
{
  gchar *file;
  char *buf;
  FILE *fl;

  if (!
      (file =
       file_chooser (_("Save current palinsesto"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SAVE)))
    {
      dialog_msg (_("Action aborted!"));
      return;
    }

  buf = somax_get_palinsesto (controller);

  if (!(fl = fopen (file, "w")))
    {
      dialog_msg (_("Error opening your file. Abort!"));
      g_free (file);
      return;
    }

  if (buf)
    {
      fprintf (fl, "%s", buf);
      g_free (buf);
    }

  fclose (fl);
  g_free (file);
}

void
savespot_clicked (GtkWidget * w, gpointer dummy)
{
  gchar *file;
  char *buf;
  FILE *fl;

  if (!
      (file =
       file_chooser (_("Save current spot file"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SAVE)))
    {
      dialog_msg (_("Action aborted!"));
      return;
    }

  buf = somax_get_spot (controller);

  if (!(fl = fopen (file, "w")))
    {
      dialog_msg (_("Error opening your file. Abort!"));
      g_free (file);
      return;
    }

  if (buf)
    {
      fprintf (fl, "%s", buf);
      g_free (buf);
    }

  fclose (fl);
  g_free (file);
}

void
savepl_old_clicked (GtkWidget * w, gpointer dummy)
{
  gchar *file;
  char *buf;
  FILE *fl;

  if (!
      (file =
       file_chooser (_("Save old palinsesto"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SAVE)))
    {
      dialog_msg (_("Action aborted!"));
      return;
    }

  buf = somax_get_old_palinsesto (controller);

  if (!(fl = fopen (file, "w")))
    {
      dialog_msg (_("Error opening your file. Abort!"));
      g_free (file);
      return;
    }

  if (buf)
    {
      fprintf (fl, "%s", buf);
      g_free (buf);
    }

  fclose (fl);
  g_free (file);
}

void
savespot_old_clicked (GtkWidget * w, gpointer dummy)
{
  gchar *file;
  char *buf;
  FILE *fl;

  if (!
      (file =
       file_chooser (_("Save old spot file"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SAVE)))
    {
      dialog_msg (_("Action aborted!"));
      return;
    }

  buf = somax_get_old_spot (controller);

  if (!(fl = fopen (file, "w")))
    {
      dialog_msg (_("Error opening your file. Abort!"));
      g_free (file);
      return;
    }

  if (buf)
    {
      fprintf (fl, "%s", buf);
      g_free (buf);
    }

  fclose (fl);
  g_free (file);
}

#define BUTTON_STR( x , y ) \
   if(stat && stat->stat && stat->stat->x[0]) {\
     tmp = somax_markup (stat->stat->x); \
     snprintf (s, sizeof (s), "<b>%s</b>", tmp); \
     gtk_label_set_markup(GTK_LABEL(y), s); \
     g_free(tmp); \
   } else \
     gtk_label_set_text(GTK_LABEL(y), "");

#define BUTTON_STR_V( x , y ) \
   if(x) { \
     tmp = somax_markup (x); \
     snprintf (s, sizeof (s), "<b>%s</b>", tmp); \
     gtk_label_set_markup(GTK_LABEL(y), s); \
     g_free(tmp); \
   } else \
     gtk_label_set_text(GTK_LABEL(y), "");

#define BUTTON_INT( x , y ) \
   if(stat && stat->stat && stat->stat->x) {\
     snprintf (s, sizeof (s), "<b>%d</b>", stat->stat->x); \
     gtk_label_set_markup(GTK_LABEL(y), s); \
   } else \
     gtk_label_set_text(GTK_LABEL(y), "");

#define BUTTON_TIME( x , y ) \
   if(stat && stat->stat && stat->stat->x) {\
     snprintf (s, sizeof (s), "<b>%s</b>", stat_duration(stat->stat->x)); \
     gtk_label_set_markup(GTK_LABEL(y), s); \
   } else \
     gtk_label_set_text(GTK_LABEL(y), "");

#define BUTTON_EMPTY( x ) \
   gtk_label_set_text(GTK_LABEL(x), "");

void
button_current_item (char *item)
{
  static char *previous = NULL;
  button_item (item, &previous, &current_item_file, TRUE, current_item);
}

void
button_next_item (char *item)
{
  static char *previous = NULL;
  button_item (item, &previous, &next_item_file, FALSE, next_item);
}

static void
button_item (char *item, char **previous, char **item_file, gboolean current,
	     GtkWidget * button_item)
{
  somax_stat_t *stat = NULL;
  char s[1024];

  if (!item)
    {
      snprintf (s, sizeof (s), "%s", _("No item"));

      if (*item_file)
	{
	  g_free (*item_file);
	  *item_file = NULL;
	}

      if (current)
	{
	  BUTTON_EMPTY (current_album);
	  BUTTON_EMPTY (current_year);
	  BUTTON_EMPTY (current_genre);
	}
      else
	{
	  BUTTON_EMPTY (next_album);
	  BUTTON_EMPTY (next_year);
	  BUTTON_EMPTY (next_genre);
	}
    }

  else
    {
      if (*item_file)
	g_free (*item_file);

      *item_file = g_strdup (item);

      stat = stat_get (item);

      if (!stat || !stat->stat || !stat->stat->title[0])
	snprintf (s, sizeof (s), "%s", stat_make_name (item));

      else if (stat && stat->stat && stat->stat->title[0]
	       && stat->stat->author[0])
	snprintf (s, sizeof (s), "%s - %s", stat->stat->title,
		  stat->stat->author);

      else
	snprintf (s, sizeof (s), "%s", stat->stat->title);
    }

  if (!*previous || strcmp (*previous, s))
    {
      gchar *tmp;

      gtk_button_set_label (GTK_BUTTON (button_item), s);

      if (*previous)
	g_free (*previous);
      *previous = g_strdup (s);

      if (current)
	{
	  BUTTON_STR (album, current_album);
	  BUTTON_INT (year, current_year);
	  BUTTON_STR (genre, current_genre);
	}
      else
	{
	  BUTTON_STR (album, next_album);
	  BUTTON_INT (year, next_year);
	  BUTTON_STR (genre, next_genre);
	}
    }

  if (stat)
    stat_unref (stat);
}

void
button_current_item_selected (GtkWidget * w, gpointer dummy)
{
  stat_popup (current_item_file);
}

void
button_next_item_selected (GtkWidget * w, gpointer dummy)
{
  stat_popup (next_item_file);
}

void
button_palinsesto_name (char *item)
{
  struct somad_data *data;

  if (!item)
    {
      gtk_button_set_label (GTK_BUTTON (palinsesto_name),
			    _("No transmission"));

      palinsesto_name_pl = NULL;
      return;
    }

  data = somad_pl;
  while (data)
    {
      if (data->description && !strcmp (item, data->description))
	break;

      data = data->next;
    }

  if (data)
    gtk_button_set_label (GTK_BUTTON (palinsesto_name), data->description);
  else
    gtk_button_set_label (GTK_BUTTON (palinsesto_name), _("No transmission"));

  palinsesto_name_pl = data;
}

void
button_palinsesto_name_selected (GtkWidget * w, gpointer dummy)
{
  if (palinsesto_name_pl)
    list_show (NULL, palinsesto_name_pl);
  else
    dialog_msg (_("No information about this transmission!"));
}

/* EOF */
