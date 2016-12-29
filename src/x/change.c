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
#include "../filechooser/filechooser.h"

void
change_palinsesto (GtkWidget * w, gpointer dummy)
{
  char *file;
  char buf[1024];

  if (!
      (file =
       file_chooser (PACKAGE " " VERSION, GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_OPEN)))
    return;

  snprintf (buf, sizeof (buf),
	    _("Are you sure to change the palinsesto of somad?"));
  if (dialog_ask (buf) != GTK_RESPONSE_OK)
    {
      g_free (file);
      return;
    }

  if (g_file_test (file, G_FILE_TEST_EXISTS) == FALSE)
    {
      snprintf (buf, sizeof (buf), _("Error: opening %s file"), file);
      dialog_msg (buf);
      g_free (file);
      return;
    }

  if (somax_new_palinsesto_file (controller, file))
    dialog_msg (_("Error changing the palinsesto file."));

  g_free (file);
}

void
change_spot (GtkWidget * w, gpointer dummy)
{
  char *file;
  char buf[1024];

  if (!
      (file =
       file_chooser (PACKAGE " " VERSION, GTK_SELECTION_SINGLE,
		     GTK_FILE_CHOOSER_ACTION_OPEN)))
    return;

  snprintf (buf, sizeof (buf),
	    _("Are you sure to change the spot file of somad?"));
  if (dialog_ask (buf) != GTK_RESPONSE_OK)
    {
      g_free (file);
      return;
    }

  if (g_file_test (file, G_FILE_TEST_EXISTS) == FALSE)
    {
      snprintf (buf, sizeof (buf), _("Error: opening %s file"), file);
      dialog_msg (buf);
      g_free (file);
      return;
    }

  if (somax_new_spot_file (controller, file))
    dialog_msg (_("Error changing the spot file."));

  g_free (file);
}

/* EOF */
