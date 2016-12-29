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
#include "help.h"

static char *credits[] = {
  "Andrea Marchesini aka bakunin - <bakunin@somasuite.org>",
  "Lorenzo Cassulo - <lorenzo@somasuite.org>",
  "mkt0 - <mkt0@autistici.org>",
  "Martin Zelaia - <martintxo@sindominio.net>",
  NULL
};

void
help_show (void)
{
  GtkWidget *dialog;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *button;
  GdkPixbuf *pixbuf;
  GtkWidget *text;
  GtkWidget *scrolledwindow;
  GtkWidget *textarea;
  GtkTextIter iter;
  int i;

  dialog = gtk_dialog_new ();

  gtk_window_set_title (GTK_WINDOW (dialog), _("SomaX About..."));
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  box = gtk_vbox_new (FALSE, 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), box, TRUE, TRUE,
		      0);

  pixbuf = gdk_pixbuf_new_from_file (PATH_IMAGE, NULL);
  image = gtk_image_new_from_pixbuf (pixbuf);

  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (box), scrolledwindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request (scrolledwindow, 400, 100);

  textarea = gtk_text_view_new ();
  gtk_container_add (GTK_CONTAINER (scrolledwindow), textarea);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textarea), FALSE);
  gtk_text_view_set_justification (GTK_TEXT_VIEW (textarea),
				   GTK_JUSTIFY_CENTER);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textarea), FALSE);
  gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (textarea), 5);
  gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (textarea), 5);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (textarea), 5);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (textarea), 5);

  text = (GtkWidget *) gtk_text_view_get_buffer (GTK_TEXT_VIEW (textarea));
  gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (text), "text", "weight",
			      PANGO_WEIGHT_BOLD, "scale",
			      PANGO_SCALE_MEDIUM, NULL);


  i = 0;
  while (credits[i])
    {
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (text), &iter);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (text), &iter, credits[i], -1);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (text), &iter);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (text), &iter, "\n", -1);
      i++;
    }

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

/* EOF */
