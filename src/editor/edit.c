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

#include "editor.h"

static void editor_spot_delete (struct editor_data_t *data);
static void editor_pl_delete (struct editor_data_t *data);
static void editor_spot_cut (struct editor_data_t *data);
static void editor_pl_cut (struct editor_data_t *data);
static void editor_spot_copy (struct editor_data_t *data);
static void editor_pl_copy (struct editor_data_t *data);
static void editor_spot_paste (struct editor_data_t *data);
static void editor_pl_paste (struct editor_data_t *data);
static void editor_spot_undo (struct editor_data_t *data);
static void editor_pl_undo (struct editor_data_t *data);
static void editor_spot_redo (struct editor_data_t *data);
static void editor_pl_redo (struct editor_data_t *data);
static void editor_pl_undo_history (struct editor_data_t *data, GtkWidget *w);
static void editor_spot_undo_history (struct editor_data_t *data, GtkWidget *w);
static void editor_pl_redo_history (struct editor_data_t *data, GtkWidget *w);
static void editor_spot_redo_history (struct editor_data_t *data, GtkWidget *w);

void
editor_delete (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();

  if (!data)
    return;

  if (data->type == TYPE_PALINSESTO)
    editor_pl_delete (data);
  else
    editor_spot_delete (data);
}

static void
editor_pl_delete (struct editor_data_t *data)
{
  if (!data->pl->pl)
    return;

  if (dialog_ask (_("Sure to remove this transmission?")) != GTK_RESPONSE_OK)
    return;

  maker_pl_remove_pl (data->pl);

  statusbar_set (_("Transmission deleted"));
}

static void
editor_spot_delete (struct editor_data_t *data)
{
  if (!data->spot->spot)
    return;

  if (dialog_ask (_("Sure to remove this spot element?")) != GTK_RESPONSE_OK)
    return;

  maker_spot_remove_spot (data->spot);

  statusbar_set (_("Spot Element deleted"));
}

void
editor_cut (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();
  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_spot_cut (data);
  else
    editor_pl_cut (data);
}

static void
editor_pl_cut (struct editor_data_t *data)
{
  struct somad_data *pl;

  if (!data->pl->pl)
    return;

  if (!(pl = maker_pl_dump_pl (data->pl)))
    return;

  maker_pl_remove_pl (data->pl);

  if (dump_pl)
    palinsesto_free (dump_pl);

  dump_pl = pl;

  statusbar_set (_("Transmission cutted"));
}

static void
editor_spot_cut (struct editor_data_t *data)
{
  struct somad_spot_data *spot;

  if (!data->spot->spot)
    return;

  if (!(spot = maker_spot_dump_spot (data->spot)))
    return;

  maker_spot_remove_spot (data->spot);

  if (dump_spot)
    spot_free (dump_spot);

  dump_spot = spot;

  statusbar_set (_("Spot Element cutted"));
}

void
editor_copy (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();

  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_spot_copy (data);
  else
    editor_pl_copy (data);
}

static void
editor_pl_copy (struct editor_data_t *data)
{
  struct somad_data *pl;

  if (!data->pl->pl)
    return;

  if (!(pl = maker_pl_dump_pl (data->pl)))
    return;

  if (dump_pl)
    palinsesto_free (dump_pl);

  dump_pl = pl;

  statusbar_set (_("Transmission copied"));
}

static void
editor_spot_copy (struct editor_data_t *data)
{
  struct somad_spot_data *spot;

  if (!data->spot->spot)
    return;

  if (!(spot = maker_spot_dump_spot (data->spot)))
    return;

  if (dump_spot)
    spot_free (dump_spot);

  dump_spot = spot;

  statusbar_set (_("Spot Element copied"));
}

void
editor_paste (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();

  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_spot_paste (data);
  else
    editor_pl_paste (data);
}

static void
editor_pl_paste (struct editor_data_t *data)
{

  if (!data || !dump_pl)
    return;

  maker_pl_insert_pl (data->pl, palinsesto_dump (dump_pl));

  statusbar_set (_("Transmission pasted"));
}

static void
editor_spot_paste (struct editor_data_t *data)
{
  if (!data || !dump_spot)
    return;

  maker_spot_insert_spot (data->spot, spot_dump (dump_spot));

  statusbar_set (_("Spot Element pasted"));
}

void
editor_undo (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();
  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_spot_undo (data);
  else
    editor_pl_undo (data);
}

static void
editor_pl_undo (struct editor_data_t *data)
{
  if (!data->pl->pl)
    return;

  maker_pl_undoredo_work (data->pl, TRUE);

  statusbar_set (_("Transmission: undo done"));
}

static void
editor_spot_undo (struct editor_data_t *data)
{
  if (!data->spot->spot)
    return;

  maker_spot_undoredo_work (data->spot, TRUE);

  statusbar_set (_("Spot Element: undo done"));
}

void
editor_redo (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();
  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_spot_redo (data);
  else
    editor_pl_redo (data);
}

static void
editor_pl_redo (struct editor_data_t *data)
{
  if (!data->pl->pl)
    return;

  maker_pl_undoredo_work (data->pl, FALSE);

  statusbar_set (_("Transmission: redo done"));
}

static void
editor_spot_redo (struct editor_data_t *data)
{
  if (!data->spot->spot)
    return;

  maker_spot_undoredo_work (data->spot, FALSE);

  statusbar_set (_("Spot Element: redo done"));
}

void
editor_undo_history (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();
  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_spot_undo_history (data, w);
  else
    editor_pl_undo_history (data, w);
}

static void
editor_pl_undo_history (struct editor_data_t *data, GtkWidget *w)
{
  if (!data->pl->pl)
    return;

  maker_pl_undoredo_history (data->pl, TRUE, w);
}

static void
editor_spot_undo_history (struct editor_data_t *data, GtkWidget *w)
{
  if (!data->spot->spot)
    return;

  maker_spot_undoredo_history (data->spot, TRUE, w);
}

void
editor_redo_history (GtkWidget * w, gpointer dummy)
{
  struct editor_data_t *data = editor_get_data ();
  if (!data)
    return;

  if (data->type == TYPE_SPOT)
    editor_spot_redo_history (data, w);
  else
    editor_pl_redo_history (data, w);
}

static void
editor_pl_redo_history (struct editor_data_t *data, GtkWidget *w)
{
  if (!data->pl->pl)
    return;

  maker_pl_undoredo_history (data->pl, FALSE, w);
}

static void
editor_spot_redo_history (struct editor_data_t *data, GtkWidget *w)
{
  if (!data->spot->spot)
    return;

  maker_spot_undoredo_history (data->spot, FALSE, w);
}

/* EOF */
