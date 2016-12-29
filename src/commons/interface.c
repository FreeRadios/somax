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
#include "save.h"
#include "x.h"

static GtkWidget *frame_unix, *frame_tcp;

#ifdef SOMAD_CMD
static gboolean daemon_start = FALSE;

gboolean
daemon_started (void)
{
  return daemon_start;
}
#endif

static void
frame_switch_unix (GtkWidget * w, gpointer dummy)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)) == FALSE)
    return;

  gtk_widget_set_sensitive (frame_tcp, FALSE);
  gtk_widget_set_sensitive (frame_unix, TRUE);
}

static void
frame_switch_tcp (GtkWidget * w, gpointer dummy)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)) == FALSE)
    return;

  gtk_widget_set_sensitive (frame_unix, FALSE);
  gtk_widget_set_sensitive (frame_tcp, TRUE);
}

void
login_next (GtkWidget * w, GtkWidget * d)
{
  gtk_widget_grab_focus (d);
}

void
login_ok (GtkWidget * w, GtkWidget * d)
{
  gtk_dialog_response (GTK_DIALOG (d), GTK_RESPONSE_OK);
}

/* Create the login interface */
soma_controller *
create_login (gpointer (*func) (gpointer), gboolean * local_connect,
	      gboolean (*daemon_run) (gchar *))
{
  GtkWidget *dialog;
  GtkWidget *box;
  GtkWidget *stock;
  GtkWidget *label;
  GtkWidget *server;
  GtkWidget *unixpath;
  GtkWidget *port;
  GtkWidget *password_tcp;
  GtkWidget *password_unix;
  GtkWidget *table;
  GtkWidget *cb_data;
#ifdef SOMA_USE_OPENSSL
  GtkWidget *cb_ssl;
#endif
  GtkWidget *expander;
  GtkAdjustment *adj;
#ifdef SOMAD_CMD
  GtkWidget *cb_somad = NULL;
  GtkWidget *cb_somad_cmd = NULL;
  GtkWidget *cbox;
#endif
  GSList *group = NULL;
  GtkWidget *frame;
  GtkWidget *radiobutton;

  char s[1024];
  int ret;
#ifdef SOMAD_CMD
  int status = 0;
#endif
  struct somax_config *config;

  soma_controller *controller;

  /* Get data from config file */
  config = get_data ();

  snprintf (s, sizeof (s), "%s %s", PACKAGE, VERSION);

  dialog =
    gtk_dialog_new_with_buttons (s, NULL,
				 GTK_DIALOG_MODAL |
				 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK,
				 GTK_RESPONSE_OK, GTK_STOCK_CANCEL,
				 GTK_RESPONSE_CANCEL, NULL);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  box = gtk_hbox_new (FALSE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (box), 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), box, FALSE, FALSE,
		      0);

  stock =
    gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
			      GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (box), stock, FALSE, FALSE, 0);

  frame = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);

  radiobutton = gtk_radio_button_new_with_mnemonic (group, _("Tcp Socket"));
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton));
  gtk_frame_set_label_widget (GTK_FRAME (frame), radiobutton);

  g_signal_connect ((gpointer) radiobutton, "toggled",
		    G_CALLBACK (frame_switch_tcp), NULL);

  table = gtk_table_new (0, 0, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_container_add (GTK_CONTAINER (frame), table);

  frame_tcp = table;

  label = gtk_label_new_with_mnemonic (_("Server: "));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);
  server = gtk_entry_new ();

  gtk_entry_set_text (GTK_ENTRY (server), config->server);

  gtk_table_attach_defaults (GTK_TABLE (table), server, 1, 2, 0, 1);

  label = gtk_label_new_with_mnemonic (_("Port: "));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

  adj = (GtkAdjustment *) gtk_adjustment_new (SOMA_PORT, 0, 65536, 1, 10, 10);
  port = gtk_spin_button_new (adj, 1, 0);
  gtk_table_attach_defaults (GTK_TABLE (table), port, 1, 2, 1, 2);

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (port), config->port);

  label = gtk_label_new_with_mnemonic (_("Password: "));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);
  password_tcp = gtk_entry_new ();

  gtk_entry_set_text (GTK_ENTRY (password_tcp), config->password);

  gtk_entry_set_visibility (GTK_ENTRY (password_tcp), FALSE);
  gtk_table_attach_defaults (GTK_TABLE (table), password_tcp, 1, 2, 2, 3);

  g_signal_connect ((gpointer) server, "activate", G_CALLBACK (login_next),
		    port);
  g_signal_connect ((gpointer) port, "activate", G_CALLBACK (login_next),
		    password_tcp);
  g_signal_connect ((gpointer) password_tcp, "activate",
		    G_CALLBACK (login_ok), dialog);

  frame = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);

  radiobutton = gtk_radio_button_new_with_mnemonic (group, _("Unix Socket"));
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton));
  gtk_frame_set_label_widget (GTK_FRAME (frame), radiobutton);

  g_signal_connect ((gpointer) radiobutton, "toggled",
		    G_CALLBACK (frame_switch_unix), NULL);

  table = gtk_table_new (0, 0, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_container_add (GTK_CONTAINER (frame), table);

  frame_unix = table;

  label = gtk_label_new_with_mnemonic (_("Unix: "));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

  unixpath = gtk_entry_new ();

  gtk_entry_set_text (GTK_ENTRY (unixpath), config->unixpath);

  gtk_table_attach_defaults (GTK_TABLE (table), unixpath, 1, 2, 0, 1);

  label = gtk_label_new_with_mnemonic (_("Password: "));
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);
  password_unix = gtk_entry_new ();

  gtk_entry_set_text (GTK_ENTRY (password_unix), config->password);

  gtk_entry_set_visibility (GTK_ENTRY (password_unix), FALSE);
  gtk_table_attach_defaults (GTK_TABLE (table), password_unix, 1, 2, 1, 2);

  g_signal_connect ((gpointer) unixpath, "activate", G_CALLBACK (login_next),
		    password_unix);
  g_signal_connect ((gpointer) password_unix, "activate",
		    G_CALLBACK (login_ok), dialog);

  expander = gtk_expander_new (_("Advanced options..."));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), expander, FALSE,
		      FALSE, 0);

  box = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (expander), box);

  cb_data = gtk_check_button_new_with_mnemonic (_("Remember data"));
  gtk_box_pack_start (GTK_BOX (box), cb_data, FALSE, FALSE, 0);

  if (config->saved)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb_data), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb_data), FALSE);

#ifdef SOMA_USE_OPENSSL
  cb_ssl = gtk_check_button_new_with_mnemonic (_("Use SSL connection"));
  gtk_box_pack_start (GTK_BOX (box), cb_ssl, FALSE, FALSE, 0);
  if (config->ssl)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb_ssl), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb_ssl), FALSE);
#endif

#ifdef SOMAD_CMD
  if (daemon_run)
    {
      cb_somad =
	gtk_check_button_new_with_mnemonic
	(_("Run somad as your user on this machine"));
      gtk_box_pack_start (GTK_BOX (box), cb_somad, FALSE, FALSE, 0);
      if (config->start)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb_somad), TRUE);
      else
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb_somad), FALSE);

      cbox = gtk_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (box), cbox, FALSE, FALSE, 0);

      label = gtk_label_new (_("Command:"));
      gtk_box_pack_start (GTK_BOX (cbox), label, FALSE, FALSE, 0);

      cb_somad_cmd = gtk_entry_new ();
      gtk_entry_set_text (GTK_ENTRY (cb_somad_cmd), config->command);

      gtk_box_pack_start (GTK_BOX (cbox), cb_somad_cmd, TRUE, TRUE, 0);
    }
#endif

  if (!config->type)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), FALSE);
      gtk_widget_set_sensitive (frame_unix, FALSE);
      gtk_widget_set_sensitive (frame_tcp, TRUE);
      gtk_widget_grab_focus (server);
    }
  else
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), TRUE);
      gtk_widget_set_sensitive (frame_unix, TRUE);
      gtk_widget_set_sensitive (frame_tcp, FALSE);
      gtk_widget_grab_focus (unixpath);
    }

  gtk_widget_show_all (dialog);

  while (1)
    {
#ifdef SOMAD_CMD
      if (daemon_run && config->command)
	{
	  g_free (config->command);
	  config->command = NULL;
	}
#endif

      if (config->server)
	{
	  g_free (config->server);
	  config->server = NULL;
	}

      if (config->unixpath)
	{
	  g_free (config->unixpath);
	  config->unixpath = NULL;
	}

      if (config->password)
	{
	  g_free (config->password);
	  config->password = NULL;
	}

      ret = gtk_dialog_run (GTK_DIALOG (dialog));

      if (ret == GTK_RESPONSE_OK)
	{
	  char *e_password_unix, *e_password_tcp;

#ifdef SOMAD_CMD
	  if (daemon_run)
	    {
	      config->start = daemon_start =
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cb_somad));

	      config->command =
		(char *) gtk_entry_get_text (GTK_ENTRY (cb_somad_cmd));
	      if (config->command && *config->command)
		config->command = g_strdup (config->command);
	      else
		config->command = NULL;
	    }
#endif

#ifdef SOMA_USE_OPENSSL
	  config->ssl =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cb_ssl));
#endif

	  config->type =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radiobutton));

	  config->server = (char *) gtk_entry_get_text (GTK_ENTRY (server));
	  if (config->server && *config->server)
	    config->server = g_strdup (config->server);
	  else
	    config->server = NULL;

	  e_password_unix =
	    (char *) gtk_entry_get_text (GTK_ENTRY (password_unix));
	  e_password_tcp =
	    (char *) gtk_entry_get_text (GTK_ENTRY (password_tcp));

	  config->unixpath =
	    (char *) gtk_entry_get_text (GTK_ENTRY (unixpath));
	  if (config->unixpath && *config->unixpath)
	    config->unixpath = g_strdup (config->unixpath);
	  else
	    config->unixpath = NULL;

	  config->port =
	    (int) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (port));

	  if (config->type == 0 && !config->server)
	    {
	      dialog_msg (_("No Server?"));
	      continue;
	    }
	  else if (config->type == 1 && !config->unixpath)
	    {
	      dialog_msg (_("No Unix Path?"));
	      continue;
	    }

	  config->password =
	    g_strdup (!config->type ? e_password_tcp : e_password_unix);
	  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cb_data)) ==
	      TRUE)
	    set_data (config);
	  else
	    set_data (NULL);

#ifdef SOMAD_CMD
	  if (daemon_run && !status && daemon_start == TRUE)
	    {

	      if (daemon_run (config->command))
		{
		  dialog_msg (_("Somad does not start!"));
		  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb_somad),
						FALSE);
		  daemon_start = FALSE;
		  continue;
		}

	      else
		gtk_widget_set_sensitive (cb_somad, FALSE);

	      status = 1;
	    }
#endif
	  if (config->type == 0)
	    controller =
	      somax_open_tcp (config->server, config->port, config->password,
#ifdef SOMA_USE_OPENSSL
			      config->ssl
#else
			      0
#endif
	      );
	  else
	    controller = somax_open_unix (config->unixpath, config->password,
#ifdef SOMA_USE_OPENSSL
					  config->ssl
#else
					  0
#endif
	      );

	  if (!controller)
	    {
	      free_data (config);
	      return NULL;
	    }

	  else if (somax_check (controller))
	    continue;

	  if (func && !g_thread_create ((gpointer) func, NULL, FALSE, NULL))
	    {
	      free_data (config);
	      return NULL;
	    }

	  if (local_connect)
	    {
	      if (config->type == 0 && (!strcmp (config->server, "localhost")
					|| !strcmp (config->server,
						    "127.0.0.1")))
		*local_connect = 1;
	      else if (config->type == 1)
		*local_connect = 1;
	      else
		*local_connect = 0;
	    }

	  gtk_widget_destroy (dialog);

	  free_data (config);
	  return controller;
	}

      else
	break;
    }

  gtk_widget_destroy (dialog);

  free_data (config);
  return NULL;
}

/* EOF */
