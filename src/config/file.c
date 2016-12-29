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

#include "cfg.h"
#include "comments.h"

static gboolean config_check_save (struct config_data *);
static gboolean config_save_real (gchar *, struct config_data *);
static void config_check_error (struct config_data *);

void
config_new (GtkWidget * w, struct config_data *data)
{
  int pre;
  GtkListStore *store;
  GtkTreeIter iter;

  if (data->changed == TRUE
      && dialog_ask (_("Exit without save your data?")) == GTK_RESPONSE_OK)
    config_save (NULL, data);

  if (dialog_ask ("Do you want a precompiled form?") == GTK_RESPONSE_OK)
    pre = 1;
  else
    pre = 0;

  /** USER **/
  gtk_entry_set_text (GTK_ENTRY (data->user), "");

  /** GROUP **/
  gtk_entry_set_text (GTK_ENTRY (data->group), "");

  /** BACKGROUND **/
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->background), FALSE);

  /** UNIX SOCKET **/
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->unixsocket), FALSE);

  /** UNIXPATH **/
  if (pre)
    gtk_entry_set_text (GTK_ENTRY (data->unixpath), "/tmp/somad.sock");
  else
    gtk_entry_set_text (GTK_ENTRY (data->unixpath), "");

  /** SERVERNAME **/
  gtk_entry_set_text (GTK_ENTRY (data->servername), "");

  /** PORT **/
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->port), SOMA_PORT);

  /** LISTEN **/
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->listen), SOMA_LISTEN);

  /** PASSWORD **/
  gtk_entry_set_text (GTK_ENTRY (data->password), "");
  gtk_entry_set_text (GTK_ENTRY (data->password2), "");

  /** SSL **/
  if (pre)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->ssl), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->ssl), FALSE);

  /** SSL - CERTIFICATE **/
  if (pre)
    gtk_entry_set_text (GTK_ENTRY (data->certificate),
			"/etc/somad/certificate.pem");
  else
    gtk_entry_set_text (GTK_ENTRY (data->certificate), "");

  /** SSL - PRIVATEKEY **/
  if (pre)
    gtk_entry_set_text (GTK_ENTRY (data->privatekey),
			"/etc/somad/private.key");
  else
    gtk_entry_set_text (GTK_ENTRY (data->privatekey), "");

  /** DEBUG **/
  gtk_combo_box_set_active (GTK_COMBO_BOX (data->debug), 3);

  /** LOGFILE **/
  if (pre)
    gtk_entry_set_text (GTK_ENTRY (data->logfile), "/var/log/somad.log");
  else
    gtk_entry_set_text (GTK_ENTRY (data->logfile), "");

  /** PIDFILE **/
  if (pre)
    gtk_entry_set_text (GTK_ENTRY (data->pidfile), "/var/run/somad.pid");
  else
    gtk_entry_set_text (GTK_ENTRY (data->pidfile), "");

  /** PATHITEM **/
  store =
    (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (data->pathitem));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  /** PATHSPOT **/
  store =
    (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (data->pathspot));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  /** PROGRAMITEM **/
  if (pre)
    gtk_entry_set_text (GTK_ENTRY (data->programitem), "somaplayer");
  else
    gtk_entry_set_text (GTK_ENTRY (data->programitem), "");

  /** OPTIONSITEM **/
  gtk_entry_set_text (GTK_ENTRY (data->optionsitem), "");

  /** PROGRAMSTREAM **/
  gtk_entry_set_text (GTK_ENTRY (data->programstream), "");

  /** OPTIONSSTREAM **/
  gtk_entry_set_text (GTK_ENTRY (data->optionsstream), "");

  /** PALINSESTO **/
  if (pre)
    gtk_entry_set_text (GTK_ENTRY (data->palinsesto),
			"/etc/somad/palinsesto.cfg");
  else
    gtk_entry_set_text (GTK_ENTRY (data->palinsesto), "");

  /** SPOT **/
  if (pre)
    gtk_entry_set_text (GTK_ENTRY (data->spot), "/etc/somad/spot.cfg");
  else
    gtk_entry_set_text (GTK_ENTRY (data->spot), "");

  /** XML SYNTAX **/
  if (pre)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->xmlsyntax), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->xmlsyntax), FALSE);

  /** SYM LINKS **/
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->symlinks), FALSE);

  /** PATHMODULES **/
  if (pre)
    gtk_entry_set_text (GTK_ENTRY (data->pathmodules), "/usr/share/soma");
  else
    gtk_entry_set_text (GTK_ENTRY (data->pathmodules), "");

  /** DISTRIBUITEDPATH **/
  store =
    (GtkListStore *)
    gtk_tree_view_get_model (GTK_TREE_VIEW (data->distribuitedpath));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  if (pre)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->distribuitedfs),
				  TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->distribuitedfs),
				  FALSE);

  /** HOSTALLOW **/
  store =
    (GtkListStore *)
    gtk_tree_view_get_model (GTK_TREE_VIEW (data->hostallow));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  /** HOSTDENY **/
  store =
    (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (data->hostdeny));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  data->changed = FALSE;

  if (data->file)
    g_free (data->file);

  data->file = NULL;
}

void
config_open (GtkWidget * w, struct config_data *data)
{
  gchar *file;
  char s[1024];

  if (data->changed == TRUE
      && dialog_ask (_("Exit without save your data?")) == GTK_RESPONSE_OK)
    config_save (NULL, data);

  if (!
      (file =
       file_chooser (_("Open a config file"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_OPEN)))
    return;

  if (config_open_real (file, data) == FALSE)
    {
      gchar *tmp;

      tmp = somax_to_utf8 (file);
      snprintf (s, sizeof (s), _("Error: opening file: %s"), file);
      g_free (tmp);

      dialog_msg (s);
    }

  g_free (file);
}

gboolean
config_open_real (gchar * file, struct config_data *data)
{
  cfg_t *cfg;
  gchar *d, *tmp;
  int b, n;

  GtkListStore *store;
  GtkTreeIter iter;

  cfg_opt_t opts[] = {
    SOMA_CONFIG_STRUCT
  };

  cfg = cfg_init (opts, CFGF_NOCASE);

  switch (cfg_parse (cfg, file))
    {
    case CFG_FILE_ERROR:
      return FALSE;

    case CFG_PARSE_ERROR:
      return FALSE;
    }

  /** USER **/
  d = cfg_getstr (cfg, "User");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->user), tmp);
  g_free (tmp);

  /** GROUP **/
  d = cfg_getstr (cfg, "Group");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->group), tmp);
  g_free (tmp);

  /** BACKGROUND **/
  if (cfg_getbool (cfg, "Background"))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->background), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->background),
				  FALSE);

  /** UNIX SOCKET **/
  if (cfg_getbool (cfg, "UnixSocket"))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->unixsocket), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->unixsocket),
				  FALSE);

  /** UNIXPATH **/
  d = cfg_getstr (cfg, "UnixPath");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->unixpath), tmp);
  g_free (tmp);

  /** SERVERNAME **/
  d = cfg_getstr (cfg, "ServerName");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->servername), tmp);
  g_free (tmp);

  /** PORT **/
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->port),
			     cfg_getint (cfg, "Port"));

  /** LISTEN **/
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (data->listen),
			     cfg_getint (cfg, "Listen"));

  /** PASSWORD **/
  d = cfg_getstr (cfg, "Password");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->password), tmp);
  gtk_entry_set_text (GTK_ENTRY (data->password2), tmp);
  g_free (tmp);

  /** SSL **/
  if (cfg_getbool (cfg, "Ssl"))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->ssl), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->ssl), FALSE);

  /** SSL - CERTIFICATE **/
  d = cfg_getstr (cfg, "Certificate");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->certificate), tmp);
  g_free (tmp);

  /** SSL - PRIVATEKEY **/
  d = cfg_getstr (cfg, "PrivateKey");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->privatekey), tmp);
  g_free (tmp);

  /** DEBUG **/
  gtk_combo_box_set_active (GTK_COMBO_BOX (data->debug),
			    cfg_getint (cfg, "Debug"));

  /** LOGFILE **/
  d = cfg_getstr (cfg, "LogFile");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->logfile), tmp);
  g_free (tmp);

  /** PIDFILE **/
  d = cfg_getstr (cfg, "PidFile");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->pidfile), tmp);
  g_free (tmp);

  /** PATHITEM **/
  store =
    (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (data->pathitem));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  n = cfg_size (cfg, "PathItem");
  for (b = 0; b < n; b++)
    {
      d = cfg_getnstr (cfg, "PathItem", b);
      tmp = somax_to_utf8 (d);
      gtk_list_store_append (GTK_LIST_STORE (store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (store), &iter, 0, tmp, -1);
      g_free (tmp);
    }

  /** PATHSPOT **/
  store =
    (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (data->pathspot));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  n = cfg_size (cfg, "PathSpot");
  for (b = 0; b < n; b++)
    {
      d = cfg_getnstr (cfg, "PathSpot", b);
      tmp = somax_to_utf8 (d);
      gtk_list_store_append (GTK_LIST_STORE (store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (store), &iter, 0, tmp, -1);
      g_free (tmp);
    }

  /** PROGRAMITEM **/
  d = cfg_getstr (cfg, "ProgramItem");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->programitem), tmp);
  g_free (tmp);

  /** OPTIONSITEM **/
  d = cfg_getstr (cfg, "OptionsItem");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->optionsitem), tmp);
  g_free (tmp);

  /** PROGRAMSTREAM **/
  d = cfg_getstr (cfg, "ProgramStream");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->programstream), tmp);
  g_free (tmp);

  /** OPTIONSSTREAM **/
  d = cfg_getstr (cfg, "OptionsStream");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->optionsstream), tmp);
  g_free (tmp);

  /** PALINSESTO **/
  d = cfg_getstr (cfg, "Palinsesto");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->palinsesto), tmp);
  g_free (tmp);

  /** SPOT **/
  d = cfg_getstr (cfg, "Spot");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->spot), tmp);
  g_free (tmp);

  /** XML SYNTAX **/
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->xmlsyntax),
				cfg_getbool (cfg, "XmlSyntax"));

  /** SYM LINKS **/
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->symlinks),
				cfg_getbool (cfg, "SymLinks"));

  /** PATHMODULES **/
  d = cfg_getstr (cfg, "PathModules");
  tmp = somax_to_utf8 (d);
  gtk_entry_set_text (GTK_ENTRY (data->pathmodules), tmp);

  /** DISTRIBUITEDPATH **/
  store =
    (GtkListStore *)
    gtk_tree_view_get_model (GTK_TREE_VIEW (data->distribuitedpath));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  n = cfg_size (cfg, "DistribuitedPath");
  for (b = 0; b < n; b++)
    {
      d = cfg_getnstr (cfg, "DistribuitedPath", b);
      tmp = somax_to_utf8 (d);
      gtk_list_store_append (GTK_LIST_STORE (store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (store), &iter, 0, tmp, -1);
      g_free (tmp);
    }

  if (n)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->distribuitedfs),
				  TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->distribuitedfs),
				  TRUE);

  /** HOSTALLOW **/
  store =
    (GtkListStore *)
    gtk_tree_view_get_model (GTK_TREE_VIEW (data->hostallow));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  n = cfg_size (cfg, "HostAllow");
  for (b = 0; b < n; b++)
    {
      d = cfg_getnstr (cfg, "HostAllow", b);
      tmp = somax_to_utf8 (d);
      gtk_list_store_append (GTK_LIST_STORE (store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (store), &iter, 0, tmp, -1);
      g_free (tmp);
    }

  /** HOSTDENY **/
  store =
    (GtkListStore *) gtk_tree_view_get_model (GTK_TREE_VIEW (data->hostdeny));
  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (store), &iter, NULL, 0))
    gtk_list_store_remove (store, &iter);

  n = cfg_size (cfg, "HostDeny");
  for (b = 0; b < n; b++)
    {
      d = cfg_getnstr (cfg, "HostDeny", b);
      tmp = somax_to_utf8 (d);
      gtk_list_store_append (GTK_LIST_STORE (store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (store), &iter, 0, tmp, -1);
      g_free (tmp);
    }

  data->changed = FALSE;

  if (data->file)
    g_free (data->file);

  data->file = g_strdup (file);

  cfg_free (cfg);

  return TRUE;
}

static gboolean
config_check_save (struct config_data *data)
{
  if (!strcmp
      (gtk_entry_get_text (GTK_ENTRY (data->password)),
       gtk_entry_get_text (GTK_ENTRY (data->password2))))
    return TRUE;

  return FALSE;
}

void
config_save_as (GtkWidget * w, struct config_data *data)
{
  gchar *file;
  char s[1024];

  if (config_check_save (data) == FALSE)
    {
      dialog_msg (_("The passwords are different!"));
      return;
    }

  if (!
      (file =
       file_chooser (_("Exit without save your data"), GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_SAVE)))
    return;

  if (config_save_real (file, data) == FALSE)
    {
      gchar *tmp;

      tmp = somax_to_utf8 (file);
      snprintf (s, sizeof (s), _("Error writing file: %s"), tmp);
      g_free (tmp);

      dialog_msg (s);
    }

  else
    {
      config_check_error (data);

      if (data->file)
	g_free (data->file);

      data->file = file;
    }
}

void
config_save (GtkWidget * w, struct config_data *data)
{
  char s[1024];

  if (!data->file)
    {
      config_save_as (w, data);
      return;
    }

  if (config_check_save (data) == FALSE)
    {
      dialog_msg (_("The passwords are different!"));
      return;
    }

  if (config_save_real (data->file, data) == FALSE)
    {
      snprintf (s, sizeof (s), _("Error writing file: %s"), data->file);
      dialog_msg (s);
    }

  else
    {
      config_check_error (data);
    }
}

static gboolean
config_save_real (gchar * file, struct config_data *data)
{
  gchar *d, *tmp;
  FILE *fl;
  int b;

  GtkTreeModel *model;
  GtkTreeIter iter;


  if (!(fl = fopen (file, "w")))
    return FALSE;

  fprintf (fl, "# This config file is generated by %s %s\n\n", PACKAGE,
	   VERSION);

  /** USER **/
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->user));
  fprintf (fl, COMMENT_USER);
  tmp = old_markup (d);
  fprintf (fl, "%sUser = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** GROUP **/
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->group));
  fprintf (fl, COMMENT_GROUP);
  tmp = old_markup (d);
  fprintf (fl, "%sGroup = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** BACKGROUND **/
  fprintf (fl, COMMENT_BACKGROUND);
  fprintf (fl, "Background = %s\n\n",
	   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->background))
	   == TRUE ? "true" : "false");

  /** UNIXSOCKET **/
  fprintf (fl, COMMENT_UNIXSOCKET);
  fprintf (fl, "UnixSocket = %s\n\n",
	   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->unixsocket))
	   == TRUE ? "true" : "false");

  /** UNIXPATH **/
  fprintf (fl, COMMENT_UNIXPATH);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->unixpath));
  tmp = old_markup (d);
  fprintf (fl, "%sUnixPath = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** SERVERNAME **/
  fprintf (fl, COMMENT_SERVERNAME);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->servername));
  tmp = old_markup (d);
  fprintf (fl, "%sServerName = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** PORT **/
  fprintf (fl, COMMENT_PORT);
  fprintf (fl, "Port = \"%d\"\n\n",
	   (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->port)));

  /** LISTEN **/
  fprintf (fl, COMMENT_LISTEN);
  fprintf (fl, "Listen = \"%d\"\n\n",
	   (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (data->listen)));

  /** PASSWORD **/
  fprintf (fl, COMMENT_PASSWORD);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->password));
  tmp = old_markup (d);
  fprintf (fl, "%sPassword = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** SSL **/
  fprintf (fl, COMMENT_SSL);
  fprintf (fl, "Ssl = %s\n\n",
	   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->ssl)) ==
	   TRUE ? "true" : "false");

  /** SSL - CERTIFICATE **/
  fprintf (fl, COMMENT_CERTIFICATE);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->certificate));
  tmp = old_markup (d);
  fprintf (fl, "%sCertificate = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** SSL - PRIVATEKEY **/
  fprintf (fl, COMMENT_PRIVATEKEY);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->privatekey));
  tmp = old_markup (d);
  fprintf (fl, "%sPrivateKey = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** DEBUG **/
  fprintf (fl, COMMENT_DEBUG);
  fprintf (fl, "Debug = \"%d\"\n\n",
	   gtk_combo_box_get_active (GTK_COMBO_BOX (data->debug)));

  /** LOGFILE **/
  fprintf (fl, COMMENT_LOGFILE);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->logfile));
  tmp = old_markup (d);
  fprintf (fl, "%sLogFile = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** PIDFILE **/
  fprintf (fl, COMMENT_PIDFILE);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->pidfile));
  tmp = old_markup (d);
  fprintf (fl, "%sPidFile = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** PATHITEM **/
  fprintf (fl, COMMENT_PATHITEM);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW ((data->pathitem)));
  if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
    {
      gchar *text;
      gboolean done;

      fprintf (fl, "PathItem = {\n");

      do
	{
	  gtk_tree_model_get (model, &iter, 0, &text, -1);
	  done = gtk_tree_model_iter_next (model, &iter);
	  tmp = old_markup (text);
	  fprintf (fl, "\t\"%s\"%s\n", tmp, done == TRUE ? "," : "");
	  g_free (tmp);
	  g_free (text);
	}
      while (done);

      fprintf (fl, "}\n\n");
    }

  else
    {
      fprintf (fl, "#PathItem = {}\n\n");
    }

  /** PATHSPOT **/
  fprintf (fl, COMMENT_PATHSPOT);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW ((data->pathspot)));
  if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
    {
      gchar *text;
      gboolean done;

      fprintf (fl, "PathSpot = {\n");

      do
	{
	  gtk_tree_model_get (model, &iter, 0, &text, -1);
	  done = gtk_tree_model_iter_next (model, &iter);
	  tmp = old_markup (text);
	  fprintf (fl, "\t\"%s\"%s\n", tmp, done == TRUE ? "," : "");
	  g_free (tmp);
	  g_free (text);
	}
      while (done);

      fprintf (fl, "}\n\n");
    }
  else
    {
      fprintf (fl, "#PathSpot = {}\n\n");
    }

  /** PROGRAMITEM **/
  fprintf (fl, COMMENT_PROGRAMITEM);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->programitem));
  tmp = old_markup (d);
  fprintf (fl, "%sProgramItem = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** OPTIONSITEM **/
  fprintf (fl, COMMENT_OPTIONSITEM);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->optionsitem));
  tmp = old_markup (d);
  fprintf (fl, "%sOptionsItem = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** PROGRAMSTREAM **/
  fprintf (fl, COMMENT_PROGRAMSTREAM);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->programstream));
  tmp = old_markup (d);
  fprintf (fl, "%sProgramStream = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** OPTIONSSTREAM **/
  fprintf (fl, COMMENT_OPTIONSSTREAM);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->optionsstream));
  tmp = old_markup (d);
  fprintf (fl, "%sOptionsStream = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** PALINSESTO **/
  fprintf (fl, COMMENT_PALINSESTO);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->palinsesto));
  tmp = old_markup (d);
  fprintf (fl, "%sPalinsesto = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** SPOT **/
  fprintf (fl, COMMENT_SPOT);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->spot));
  tmp = old_markup (d);
  fprintf (fl, "%sSpot = \"%s\"\n\n", *tmp ? "" : "#", tmp);
  g_free (tmp);

  /** XML SYNTAX **/
  fprintf (fl, COMMENT_XMLSYNTAX);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->xmlsyntax)) ==
      TRUE)
    fprintf (fl, "XmlSyntax = true\n\n");
  else
    fprintf (fl, "XmlSyntax = false\n\n");

  /** SYM LINKS **/
  fprintf (fl, COMMENT_SYMLINKS);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->symlinks)) ==
      TRUE)
    fprintf (fl, "SymLinks = true\n\n");
  else
    fprintf (fl, "SymLinks = false\n\n");

  /** PATHMODULES **/
  fprintf (fl, COMMENT_PATHMODULES);
  d = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->pathmodules));
  tmp = old_markup (d);
  fprintf (fl, "%sPathModules = \"%s\"\n\n", d ? "" : "#", tmp);
  g_free (tmp);

  /** DISTRIBUITEDFS **/
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->distribuitedfs))
      == TRUE)
    b = 1;
  else
    b = 0;

  /** DISTRIBUITEDPATH **/
  fprintf (fl, COMMENT_DISTRIBUITEDPATH);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW ((data->distribuitedpath)));
  if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
    {
      gchar *text;
      gboolean done;

      fprintf (fl, "%sDistribuitedPath = {\n", b ? "" : "#");

      do
	{
	  gtk_tree_model_get (model, &iter, 0, &text, -1);
	  done = gtk_tree_model_iter_next (model, &iter);
	  tmp = old_markup (text);
	  fprintf (fl, "%s\t\"%s\"%s\n", b ? "" : "#", tmp,
		   done == TRUE ? "," : "");
	  g_free (tmp);
	  g_free (text);
	}
      while (done);

      fprintf (fl, "%s}\n\n", b ? "" : "#");
    }

  else
    fprintf (fl, "#DistribuitedPath = {}\n\n");

  /** HOSTALLOW **/
  fprintf (fl, COMMENT_HOSTALLOW);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW ((data->hostallow)));
  if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
    {
      gchar *text;
      gboolean done;

      fprintf (fl, "HostAllow = {\n");

      do
	{
	  gtk_tree_model_get (model, &iter, 0, &text, -1);
	  done = gtk_tree_model_iter_next (model, &iter);
	  tmp = old_markup (text);
	  fprintf (fl, "\t\"%s\"%s\n", tmp, done == TRUE ? "," : "");
	  g_free (tmp);
	  g_free (text);
	}
      while (done);

      fprintf (fl, "}\n\n");
    }

  else
    fprintf (fl, "#HostAllow = {}\n\n");

  /** HOSTDENY **/
  fprintf (fl, COMMENT_HOSTDENY);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW ((data->hostdeny)));
  if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
    {
      gchar *text;
      gboolean done;

      fprintf (fl, "HostDeny = {\n");

      do
	{
	  gtk_tree_model_get (model, &iter, 0, &text, -1);
	  done = gtk_tree_model_iter_next (model, &iter);
	  tmp = old_markup (text);
	  fprintf (fl, "\t\"%s\"%s\n", tmp, done == TRUE ? "," : "");
	  g_free (tmp);
	  g_free (text);
	}
      while (done);

      fprintf (fl, "}\n\n");
    }

  else
    fprintf (fl, "#HostDeny = {}\n\n");

  fclose (fl);

  chmod (file, 0600);

  data->changed = FALSE;

  return TRUE;
}

static void
config_check_error (struct config_data *data)
{
  gchar *tmp;
  gchar s[1024];
  int i = 1;
  GString *str;

  str = g_string_new ("Your data saved!");

  tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->logfile));

  if (!tmp || !*tmp)
    {
      snprintf (s, sizeof (s), "\n%d. Set the LogFile!", i++);
      str = g_string_append (str, s);
    }

  tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->pidfile));

  if (!tmp || !*tmp)
    {
      snprintf (s, sizeof (s), "\n%d. Set the PidFile!", i++);
      str = g_string_append (str, s);
    }

  tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->programitem));

  if (!tmp || !*tmp)
    {
      snprintf (s, sizeof (s), "\n%d. Set the ProgramItem!", i++);
      str = g_string_append (str, s);
    }

  tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->palinsesto));

  if (!tmp || !*tmp)
    {
      snprintf (s, sizeof (s), "\n%d. Set the Palinsesto!", i++);
      str = g_string_append (str, s);
    }

  tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->password));

  if (!tmp || !*tmp)
    {
      snprintf (s, sizeof (s), "\n%d. Set the Password!", i++);
      str = g_string_append (str, s);
    }

  tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->spot));

  if (!tmp || !*tmp)
    {
      snprintf (s, sizeof (s), "\n%d. Set the Spot!", i++);
      str = g_string_append (str, s);
    }

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->xmlsyntax)) ==
      FALSE)
    {
      snprintf (s, sizeof (s), "\n%d. XmlSyntax is necessary for somax!",
		i++);
      str = g_string_append (str, s);
    }

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->ssl)) == TRUE)
    {
      tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->certificate));

      if (!tmp || !*tmp)
	{
	  snprintf (s, sizeof (s), "\n%d. Set the Certificate!", i++);
	  str = g_string_append (str, s);
	}

      tmp = (gchar *) gtk_entry_get_text (GTK_ENTRY (data->privatekey));

      if (!tmp || !*tmp)
	{
	  snprintf (s, sizeof (s), "\n%d. Set the PrivateKey!", i++);
	  str = g_string_append (str, s);
	}

    }

  dialog_msg (str->str);
  g_string_free (str, TRUE);
}

/* EOF */
