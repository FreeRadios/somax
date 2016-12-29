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

#include "plugin.h"
#include "plugin_internal.h"

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprintui/gnome-print-job-preview.h>
#include <libgnomeprint/gnome-print-pango.h>
#include <libgnomeprintui/gnome-print-dialog.h>

struct print_data
{
  gdouble page_width;
  gdouble page_height;

  gdouble margin_top;
  gdouble margin_bottom;
  gdouble margin_left;
  gdouble margin_right;

  gint page;
};

static struct print_data *
print_get_config (GnomePrintJob * job, GtkWidget * dialog)
{
  const GnomePrintUnit *unit;
  struct print_data *data;
  GnomePrintConfig *config;

  if (!(config = gnome_print_dialog_get_config ((GnomePrintDialog *) dialog))
      && !(config = gnome_print_job_get_config (job)))
    return NULL;

  data = g_malloc0 (sizeof (struct print_data));

  gnome_print_job_get_page_size_from_config (config, &data->page_width,
					     &data->page_height);

  if (gnome_print_config_get_length
      (config, (guchar *) GNOME_PRINT_KEY_PAGE_MARGIN_LEFT,
       &data->margin_left, &unit))
    gnome_print_convert_distance (&data->margin_left, unit,
				  GNOME_PRINT_PS_UNIT);

  if (gnome_print_config_get_length
      (config, (guchar *) GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT,
       &data->margin_right, &unit))
    gnome_print_convert_distance (&data->margin_right, unit,
				  GNOME_PRINT_PS_UNIT);

  if (gnome_print_config_get_length
      (config, (guchar *) GNOME_PRINT_KEY_PAGE_MARGIN_TOP, &data->margin_top,
       &unit))
    gnome_print_convert_distance (&data->margin_top, unit,
				  GNOME_PRINT_PS_UNIT);

  if (gnome_print_config_get_length
      (config, (guchar *) GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM,
       &data->margin_bottom, &unit))
    gnome_print_convert_distance (&data->margin_bottom, unit,
				  GNOME_PRINT_PS_UNIT);

  g_object_unref (G_OBJECT (config));
  return data;
}

static void
make_new_page (GnomePrintContext * gpc, struct print_data *config,
	       gdouble * y)
{
  char s[1024];
  PangoLayout *layout;
  gint width, height;

  if (config->page)
    {
      gnome_print_stroke (gpc);
      gnome_print_showpage (gpc);
    }

  config->page++;
  (*y) = config->page_height - config->margin_top;

  snprintf (s, sizeof (s), "Print Transmission %d", config->page);
  gnome_print_beginpage (gpc, (guchar *) s);

  layout = gnome_print_pango_create_layout (gpc);
  pango_layout_set_width (layout,
			  (config->page_width - config->margin_left -
			   config->margin_right) * PANGO_SCALE);

  snprintf (s, sizeof (s), "Page: %d", config->page);
  pango_layout_set_text (layout, s, strlen (s));
  pango_layout_get_pixel_size (layout, &width, &height);

  gnome_print_moveto (gpc,
		      config->margin_left +
		      ((config->page_width - config->margin_left -
			config->margin_right) / 2) - (width / 2),
		      config->margin_bottom + height);
  gnome_print_pango_layout (gpc, layout);

  g_object_unref (layout);
}

static void
write_title (GnomePrintContext * gpc, struct print_data *config, gdouble * y,
	     gchar * text)
{
  PangoLayout *layout;
  PangoAttribute *attr;
  PangoAttrList *list_attr;
  gint width, height;

  gnome_print_moveto (gpc, config->margin_left, *y);
  gnome_print_lineto (gpc, config->page_width - config->margin_right, *y);

  layout = gnome_print_pango_create_layout (gpc);
  pango_layout_set_width (layout,
			  (config->page_width - config->margin_left -
			   config->margin_right) * PANGO_SCALE);

  list_attr = pango_attr_list_new ();

  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  pango_attr_list_insert (list_attr, attr);
  attr->start_index = 0;
  attr->end_index = -1;

  pango_layout_set_attributes (layout, list_attr);

  attr = pango_attr_scale_new (PANGO_SCALE_X_LARGE);
  pango_attr_list_insert (list_attr, attr);
  attr->start_index = 0;
  attr->end_index = -1;

  pango_layout_set_attributes (layout, list_attr);

  pango_layout_set_text (layout, text, strlen (text));
  pango_layout_get_pixel_size (layout, &width, &height);

  if (*y <= config->margin_bottom + (height * 2))
    make_new_page (gpc, config, y);

  gnome_print_moveto (gpc, config->page_width - config->margin_right - width,
		      *y);
  gnome_print_pango_layout (gpc, layout);

  pango_attr_list_unref (list_attr);
  g_object_unref (layout);
  (*y) -= height;
}

static void
write_text (GnomePrintContext * gpc, struct print_data *config, gdouble * y,
	    gchar * text)
{
  PangoLayout *layout;
  gint width, height;

  layout = gnome_print_pango_create_layout (gpc);
  pango_layout_set_width (layout,
			  (config->page_width - config->margin_left -
			   config->margin_right) * PANGO_SCALE);

  pango_layout_set_markup (layout, text, strlen (text));
  pango_layout_get_pixel_size (layout, &width, &height);

  if (*y <= config->margin_bottom + (height * 2))
    make_new_page (gpc, config, y);

  gnome_print_moveto (gpc, config->margin_left, *y);
  gnome_print_pango_layout (gpc, layout);

  g_object_unref (layout);

  (*y) -= height;
}

static void
write_space (GnomePrintContext * gpc, struct print_data *config, gdouble * y)
{
  *y -= 10;

  if (*y <= config->margin_bottom + 20)
    make_new_page (gpc, config, y);
}

GtkWidget *
print_transmission (struct somad_data *data)
{
  GnomePrintJob *job;
  GnomePrintContext *gpc;
  GtkWidget *dialog;
  struct print_data *config;
  gdouble x, y;
  GtkWidget *w = NULL;
  struct somad_data *tmp;
  char s[1024];
  GList *list;
  gchar *str;

  gboolean preview;

  job = gnome_print_job_new (NULL);

  dialog =
    gnome_print_dialog_new (job, (guchar *) "Print your transmission", 0);

  switch (gtk_dialog_run (GTK_DIALOG (dialog)))
    {
    case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
      preview = FALSE;
      break;

    case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
      preview = TRUE;
      break;

    default:
      g_object_unref (G_OBJECT (job));
      gtk_widget_destroy (dialog);
      return NULL;
    }

  if (!(config = print_get_config (job, dialog)))
    {
      g_object_unref (G_OBJECT (job));
      return NULL;
    }

  gpc = gnome_print_job_get_context (job);

  make_new_page (gpc, config, &y);

  x = config->margin_left;

  tmp = data;

  while (tmp)
    {

      write_title (gpc, config, &y, tmp->description);

      str = somax_markup (tmp->description);
      snprintf (s, sizeof (s), "Description: <b>%s</b>", str);
      g_free (str);
      write_text (gpc, config, &y, s);

      snprintf (s, sizeof (s), "Priority: <b>%s</b>",
		tmp->priority ? "true" : "false");
      write_text (gpc, config, &y, s);

      str = somax_markup (tmp->timer->start);
      snprintf (s, sizeof (s), "Start: <b>%s</b>", str);
      g_free (str);
      write_text (gpc, config, &y, s);

      str = somax_markup (tmp->timer->stop);
      snprintf (s, sizeof (s), "Stop: <b>%s</b>", str);
      g_free (str);
      write_text (gpc, config, &y, s);

      snprintf (s, sizeof (s), "TimeContinued: <b>%s</b>",
		tmp->timer->timecontinued ? "true" : "false");
      write_text (gpc, config, &y, s);

      snprintf (s, sizeof (s), "SpotController: <b>%s</b>",
		tmp->spotcontroller ? "true" : "false");
      write_text (gpc, config, &y, s);

      str = somax_markup (tmp->jingle);
      snprintf (s, sizeof (s), "Jingle: <b>%s</b>", str);
      g_free (str);
      write_text (gpc, config, &y, s);

      str = somax_markup (tmp->prespot);
      snprintf (s, sizeof (s), "PreSpot: <b>%s</b>", str);
      g_free (str);
      write_text (gpc, config, &y, s);

      str = somax_markup (tmp->postspot);
      snprintf (s, sizeof (s), "PostSpot: <b>%s</b>", str);
      g_free (str);
      write_text (gpc, config, &y, s);

      switch (tmp->type)
	{
	case STREAM:
	  snprintf (s, sizeof (s), "Type: <b>Stream</b>");
	  write_text (gpc, config, &y, s);

	  str = somax_markup (tmp->stream);
	  snprintf (s, sizeof (s), "Stream: <b>%s</b>", str);
	  g_free (str);
	  write_text (gpc, config, &y, s);

	  snprintf (s, sizeof (s), "SoftStop: <b>%s</b>",
		    tmp->softstop ? "true" : "false");
	  write_text (gpc, config, &y, s);

	  break;

	case MODULE:
	  snprintf (s, sizeof (s), "Type: <b>Module</b>");
	  write_text (gpc, config, &y, s);

	  str = somax_markup (tmp->module);
	  snprintf (s, sizeof (s), "Module: <b>%s</b>", str);
	  g_free (str);
	  write_text (gpc, config, &y, s);

	  break;

	case SILENCE:
	  snprintf (s, sizeof (s), "Type: <b>Silence</b>");
	  write_text (gpc, config, &y, s);
	  break;

	default:
	  snprintf (s, sizeof (s), "Type: <b>Files</b>");
	  write_text (gpc, config, &y, s);

	  snprintf (s, sizeof (s), "N. Item: <b>%d</b>",
		    tmp->pathitem ? g_list_length (tmp->pathitem) : 0);
	  write_text (gpc, config, &y, s);

	  if (tmp->pathitem)
	    {
	      snprintf (s, sizeof (s), "Items:");
	      write_text (gpc, config, &y, s);

	      list = tmp->pathitem;
	      while (list)
		{
		  str = somax_markup ((char *) list->data);
		  snprintf (s, sizeof (s), "\t<b>%s</b>", str);
		  g_free (str);
		  write_text (gpc, config, &y, s);

		  list = list->next;
		}
	    }

	  snprintf (s, sizeof (s), "Ratio Item: <b>%d</b>", tmp->ratioitem);
	  write_text (gpc, config, &y, s);

	  snprintf (s, sizeof (s), "N. Spot: <b>%d</b>",
		    tmp->pathspot ? g_list_length (tmp->pathspot) : 0);
	  write_text (gpc, config, &y, s);

	  if (tmp->pathitem)
	    {
	      snprintf (s, sizeof (s), "Spots:");
	      write_text (gpc, config, &y, s);

	      list = tmp->pathspot;
	      while (list)
		{
		  str = somax_markup ((char *) list->data);
		  snprintf (s, sizeof (s), "\t<b>%s</b>", str);
		  g_free (str);
		  write_text (gpc, config, &y, s);

		  list = list->next;
		}
	    }

	  snprintf (s, sizeof (s), "Ratio Spot: <b>%d</b>", tmp->ratiospot);
	  write_text (gpc, config, &y, s);

	  snprintf (s, sizeof (s), "Random Item: <b>%s</b>",
		    tmp->randomitem ? "true" : "false");
	  write_text (gpc, config, &y, s);

	  snprintf (s, sizeof (s), "Random Spot: <b>%s</b>",
		    tmp->randomspot ? "true" : "false");
	  write_text (gpc, config, &y, s);

	  snprintf (s, sizeof (s), "SoftStop: <b>%s</b>",
		    tmp->softstop ? "true" : "false");
	  write_text (gpc, config, &y, s);

	  break;

	}

      write_space (gpc, config, &y);

      tmp = tmp->next;
    }

  /* BOTTOM */
  gnome_print_moveto (gpc, config->margin_left, y);
  gnome_print_lineto (gpc, config->page_width - config->margin_right, y);

  gnome_print_stroke (gpc);
  gnome_print_showpage (gpc);

  gnome_print_job_close (job);

  if (preview == TRUE)
    {
      w =
	gnome_print_job_preview_new (job,
				     (guchar *)
				     "Print preview your transmissions...");
      gtk_widget_show_all (w);
    }

  else
    gnome_print_job_print (job);

  gtk_widget_destroy (dialog);

  g_object_unref (G_OBJECT (gpc));
  g_object_unref (G_OBJECT (job));

  return w;
}

gboolean
print_timeout (struct somad_data * data)
{
  static int i = 0;
  static GtkWidget *w = NULL;

  if (!i)
    {
      w = print_transmission (data);
      i++;
    }

  else
    {
      if (!GTK_IS_WIDGET (w))
	{
	  gtk_main_quit ();
	  return 0;
	}
    }

  return 1;
}

int
main (int argc, char **argv)
{
  struct somad_data *data;

  g_thread_init (NULL);
  gtk_init (NULL, NULL);

  somax_plgn_check_arg (argc, argv);
  somax_plgn_set_description ("Print your palinsesto");
  somax_plgn_set_author ("Andrea Marchesini");
  somax_plgn_set_name ("somax_print_plugin");
  somax_plgn_set_licence ("GPL");
  somax_plgn_set_version ("0.1");

  somax_plgn_print_data ();

  if (!(data = somax_plgn_parser ()))
    {
      dialog_msg ("No transmission! Skip printing...");
      return 0;
    }

  print_timeout (data);
  g_timeout_add (500, (GSourceFunc) print_timeout, data);

  gtk_main ();

  somax_plgn_parser_free (data);
  somax_plgn_free ();

  return 0;
}

/* EOF */
