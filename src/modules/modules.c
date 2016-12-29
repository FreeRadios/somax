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

#include "../commons/commons.h"
#include "../commons/data.h"
#include "../commons/splash.h"
#include "modules.h"

static GList *module_list = NULL;

static struct module_t *module_get_data (gchar * file);

void
module_scanner (void)
{
  GDir *d;
  struct module_t *tmp;
  gchar *a;
  const gchar *file;

  if (!(d = g_dir_open (MODULES_PATH, 0, NULL)))
    return;

  while ((file = g_dir_read_name (d)))
    {
      if (*file == '.')
	continue;

      if (g_str_has_suffix (file, G_MODULE_SUFFIX) == FALSE)
	continue;

      splash_set_text ("%s", file);

      a = g_build_path (G_DIR_SEPARATOR_S, MODULES_PATH, file, NULL);

      if (g_file_test (a, G_FILE_TEST_IS_REGULAR) == FALSE)
	{
	  g_free (a);
	  continue;
	}

      if (!(tmp = module_get_data (a)))
	{
	  g_free (a);
	  continue;
	}

      module_list = g_list_append (module_list, tmp);
    }

  g_dir_close (d);
}

GList *
module_get_list (void)
{
  return module_list;
}

static struct module_t *
module_get_data (gchar * a)
{
  GModule *module;
  struct module_t *tmp;

  char *(*name) (void);

  void *somax_module_new;
  void *somax_module_set_value;
  void *somax_module_get_value;

  tmp = (struct module_t *) g_malloc0 (sizeof (struct module_t));

  if (!(module = g_module_open (a, G_MODULE_BIND_MASK)))
    {
      g_free (tmp);
      g_message ("error: opening %s", a);
      return NULL;
    }

  if (g_module_symbol(module, "somax_module_new", &somax_module_new)==FALSE)
    {
      g_free (tmp);
      g_module_close (module);
      g_message ("error: %s - somax_module_new", a);
      return NULL;
    }

  if (g_module_symbol(module, "somax_module_name", (gpointer)&name)==FALSE)
    {
      g_free (tmp);
      g_module_close (module);
      g_message ("error: %s - somax_module_name", a);
      return NULL;
    }

  if (g_module_symbol(module, "somax_module_set_value", &somax_module_set_value)==FALSE)
    {
      g_free (tmp);
      g_module_close (module);
      g_message ("error: %s - somax_module_set_value", a);
      return NULL;
    }

  if (g_module_symbol(module, "somax_module_get_value", &somax_module_get_value)==FALSE)
    {
      g_free (tmp);
      g_module_close (module);
      g_message ("error: %s - somax_module_get_value", a);
      return NULL;
    }

  tmp->file = a;

  tmp->new = somax_module_new;
  tmp->set_value = somax_module_set_value;
  tmp->get_value = somax_module_get_value;
  tmp->name = name ();

  return tmp;
}

/* EOF */
