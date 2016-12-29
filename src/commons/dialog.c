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

int
dialog_ask (char *text)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *button;
  int ret;

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), _("Question"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  hbox = gtk_hbox_new (FALSE, 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE,
		      0);

  image = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new (text);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock ("gtk-no");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_NO);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  ret = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  return ret;
}

int
dialog_ask_with_cancel (char *text)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *button;
  int ret;

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), _("Question"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  hbox = gtk_hbox_new (FALSE, 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE,
		      0);

  image = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new (text);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock ("gtk-cancel");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button,
				GTK_RESPONSE_CANCEL);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  button = gtk_button_new_from_stock ("gtk-no");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_NO);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  ret = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  return ret;
}

void
dialog_msg (char *text)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *button;

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), _("Information"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  hbox = gtk_hbox_new (FALSE, 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE,
		      0);

  image = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new (text);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

/* EOF */
