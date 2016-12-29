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

static int win_nextitem_win = 0;

gboolean
win_nextitem_window_showed (void)
{
  return win_nextitem_win;
}

static gint
win_nextitem_window_show (GtkWidget * w, gpointer data)
{

  if (!win_nextitem_win)
    {
      win_nextitem_win = 1;
      nextitem_refresh ();
      gtk_widget_show (winnextitem);
    }
  else if (win_nextitem_win)
    {
      gtk_widget_hide (winnextitem);
      win_nextitem_win = 0;
    }

  return TRUE;
}

static gboolean
win_nextitem_key_event (GtkWidget * w, GdkEventKey * event, gpointer dummy)
{
  if (event->keyval == GDK_Escape)
    {
      win_nextitem_window_show (NULL, NULL);
      return TRUE;
    }

  return FALSE;
}

void
win_nextitem_new (void)
{
  GtkWidget *window;
  GtkWidget *w;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *image;
  GtkWidget *alignment;

  char s[1024];

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  snprintf (s, sizeof (s), "%s %s", PACKAGE, VERSION);
  gtk_window_set_title (GTK_WINDOW (window), s);
  gtk_window_resize (GTK_WINDOW (window), 300, 400);

  g_signal_connect ((gpointer) window, "delete_event",
		    G_CALLBACK (win_nextitem_window_show), NULL);

  g_signal_connect ((gpointer) window, "key-press-event",
		    GTK_SIGNAL_FUNC (win_nextitem_key_event), NULL);

  box = gtk_vbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (window), box);

  if (local_connect)
    w = nextitem_new (controller, TRUE);
  else
    w = nextitem_new (controller, FALSE);

  gtk_widget_show (w);
  gtk_box_pack_start (GTK_BOX (box), w, TRUE, TRUE, 0);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

  g_signal_connect ((gpointer) button, "clicked",
		    G_CALLBACK (win_nextitem_window_show), NULL);

  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  box = gtk_hbox_new (0, FALSE);
  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (alignment), box);

  image = gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (_("Close nextitem window"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  winnextitem = window;
}

void
win_nextitem_show (GtkWidget * w, gpointer dummy)
{
  if (!winnextitem)
    win_nextitem_new ();

  win_nextitem_window_show (NULL, NULL);
}

void
win_nextitem_add_stream (GtkWidget * w, gpointer dummy)
{
  if (!winnextitem)
    win_nextitem_new ();

  nextitem_add_stream ();
}

void
win_nextitem_add_r_dir (GtkWidget * w, gpointer dummy)
{
  if (!winnextitem)
    win_nextitem_new ();

  nextitem_add_r_directory ();
}

void
win_nextitem_add_r_file (GtkWidget * w, gpointer dummy)
{
  if (!winnextitem)
    win_nextitem_new ();

  nextitem_add_r_file ();
}

void
win_nextitem_add_l_dir (GtkWidget * w, gpointer dummy)
{
  if (!winnextitem)
    win_nextitem_new ();

  nextitem_add_l_directory ();
}

void
win_nextitem_add_l_file (GtkWidget * w, gpointer dummy)
{
  if (!winnextitem)
    win_nextitem_new ();

  nextitem_add_l_file ();
}

/* EOF */
