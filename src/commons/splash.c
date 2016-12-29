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

#include "splash.h"

static GtkWidget *splash_win = NULL;

static GtkWidget *splash_init (void);

void
splash_showhide (void)
{
  time_t now, old;

  if (!splash_win)
    {
      old = time (NULL);

      splash_win = splash_init ();

      /* hahah... the logo is realy important! */
      while (1)
	{
	  now = time (NULL);

	  if ((now - old) >= 2)
	    break;

	  while (gtk_events_pending ())
	    gtk_main_iteration ();

	  g_usleep (500);
	}
    }

  else
    {
      gtk_widget_destroy (splash_win);
      splash_win = NULL;
    }

  while (gtk_events_pending ())
    gtk_main_iteration ();
}

static GtkWidget *
splash_init (void)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *image;
  GdkPixbuf *i;
  GtkWidget *label;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), PACKAGE);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_type_hint (GTK_WINDOW (window),
			    GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  gtk_widget_realize (window);

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_container_add (GTK_CONTAINER (window), frame);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  i = gdk_pixbuf_new_from_file (PATH_IMAGE, NULL);
  image = gtk_image_new_from_pixbuf (i);
  gtk_widget_show (image);

  gtk_box_pack_start (GTK_BOX (vbox), image, TRUE, TRUE, 0);

  label = gtk_label_new (_("Test module:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);

  label = gtk_label_new (_("initialize data"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);

  g_object_set_data (G_OBJECT (window), "label", label);

  gtk_widget_show (window);

  return window;
}

/* The message on boot splash */
void
splash_set_text (char *str, ...)
{

  char buf[100];
  va_list va;
  GtkWidget *label;

  label = g_object_get_data (G_OBJECT (splash_win), "label");

  va_start (va, str);

  vsnprintf (buf, 100, str, va);
  gtk_label_set_text (GTK_LABEL (label), buf);
  va_end (va);

  while (gtk_events_pending ())
    gtk_main_iteration ();
}

/* EOF */
