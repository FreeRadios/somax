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

GtkWidget *connect_label;
soma_controller *controller = NULL;

static void daemon_reset (void);

void
daemon_connect (void)
{
  if (controller)
    {
      gtk_label_set_text (GTK_LABEL (connect_label), _("Connect"));
      soma_free (controller);
      controller = NULL;

      daemon_reset ();
      return;
    }

  if (!(controller = create_login (NULL, NULL, NULL)))
    return;

  gtk_label_set_text (GTK_LABEL (connect_label), _("Disconnect"));
  daemon_reset ();
}

static void
daemon_reset (void)
{
  struct editor_data_t *data;

  for (data = editor_data; data; data = data->next)
    {
      if (data->type == TYPE_PALINSESTO)
	maker_pl_set_controller (data->pl, controller);
      else
	maker_spot_set_controller (data->spot, controller);
    }
}

/* EOF */
