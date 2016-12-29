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
#include "../draw/draw.h"
#include "palinsesto.h"

static int palinsesto_parser_real (char *, struct somad_data **,
			           gboolean);

void
palinsesto_free (struct somad_data *pl)
{
  struct somad_data *tmp, *old;
  GList *list;

  if (!pl)
    return;

  tmp = pl;

  while (tmp)
    {
      if (tmp->pathitem)
	{
	  list = tmp->pathitem;
	  while (list)
	    {
	      g_free (list->data);
	      list = list->next;
	    }

	  g_list_free (tmp->pathitem);
	}

      if (tmp->pathspot)
	{
	  list = tmp->pathspot;
	  while (list)
	    {
	      g_free (list->data);
	      list = list->next;
	    }

	  g_list_free (tmp->pathspot);
	}

      if (tmp->stream)
	g_free (tmp->stream);

      if (tmp->jingle)
	g_free (tmp->jingle);

      if (tmp->module)
	g_free (tmp->module);

      if (tmp->moduledata)
	g_free (tmp->moduledata);

      if (tmp->timer)
	{
	  if (tmp->timer->start)
	    g_free (tmp->timer->start);

	  if (tmp->timer->stop)
	    g_free (tmp->timer->stop);

	  g_free (tmp->timer);
	}

      if (tmp->description)
	g_free (tmp->description);

      old = tmp;
      tmp = tmp->next;

      g_free (old);
    }
}

static void
palinsesto_null (void *fl, const char *str, ...)
{
  /* void... */
}

int
palinsesto_check_file (char *file)
{
  xmlDocPtr doc;
  xmlNodePtr cur;

  xmlSetGenericErrorFunc (xmlGenericErrorContext, palinsesto_null);

  if (!(doc = xmlParseFile (file)))
    return 1;

  if (!(cur = xmlDocGetRootElement (doc)))
    {
      xmlFreeDoc (doc);
      return 1;
    }

  if (xmlStrcmp (cur->name, (xmlChar *) "PalinsestoXML"))
    {
      xmlFreeDoc (doc);
      return 1;
    }

  xmlFreeDoc (doc);
  return 0;
}

int
palinsesto_parser_file (char *file, struct somad_data **father)
{
  return palinsesto_parser_real (file, father, TRUE);
}

int
palinsesto_parser (char *pl, struct somad_data **father)
{
  return palinsesto_parser_real (pl, father, FALSE);
}

static int
palinsesto_parser_real (char *pl, struct somad_data **father,
			    gboolean file_buf)
{
  struct somad_data *tmp;
  char *c;
  char *tmp_start = NULL;
  char *tmp_stop = NULL;
  int tmp_timecontinued = 0;
  xmlDocPtr doc;
  xmlNodePtr cur, cur_pl, cur_item;

  if (!pl)
    return 1;

  *father = NULL;

  if (!*pl)
    return 0;

  if (file_buf == TRUE)
    doc = xmlParseFile (pl);
  else
    doc = xmlParseMemory (pl, strlen (pl));

  if (!doc)
    return 1;

  if (!(cur = xmlDocGetRootElement (doc)))
    {
      xmlFreeDoc (doc);
      return 1;
    }

  if (xmlStrcmp (cur->name, (xmlChar *) "PalinsestoXML"))
    {
      xmlFreeDoc (doc);
      return 1;
    }

  for (cur = cur->children; cur != NULL; cur = cur->next)
    {

      if (!xmlStrcmp (cur->name, (xmlChar *) "Palinsesto"))
	{
	  tmp = g_malloc0 (sizeof (struct somad_data));

	  cur_pl = cur->children;

	  for (; cur_pl != NULL; cur_pl = cur_pl->next)
	    {
	      if (!xmlStrcmp (cur_pl->name, (xmlChar *) "SpotController"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)) || !atoi (c))
		    tmp->spotcontroller = 0;
		  else
		    tmp->spotcontroller = 1;

		  if (c)
		    free (c);
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Description"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)))
		    tmp->description = p_utf8 (_("No description"));
		  else
		    {
		      tmp->description = p_utf8 (c);
		      free (c);
		    }

		  if ((c = (char *) xmlGetProp (cur_pl, (xmlChar *) "color")))
		    {
		      tmp->color = draw_color_str (c);
		      free (c);
		    }

		  if (!tmp->color)
		    tmp->color = draw_color ();
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Type"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1))
		      || !strcasecmp (c, "files"))
		    tmp->type = FILES;
		  else if (!strcasecmp (c, "stream"))
		    tmp->type = STREAM;
		  else if (!strcasecmp (c, "module"))
		    tmp->type = MODULE;
		  else if (!strcasecmp (c, "silence"))
		    tmp->type = SILENCE;
		  else
		    tmp->type = FILES;

		  if (c)
		    free (c);
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Module"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)))
		    tmp->module = g_strdup ("");
		  else
		    {
		      tmp->module = p_utf8 (c);
		      free (c);
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "ModuleData"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)))
		    tmp->moduledata = g_strdup ("");
		  else
		    {
		      tmp->moduledata = p_utf8 (c);
		      free (c);
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Stream"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)))
		    tmp->stream = g_strdup ("");
		  else
		    {
		      tmp->stream = p_utf8 (c);
		      free (c);
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Jingle"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)))
		    tmp->jingle = g_strdup ("");
		  else
		    {
		      tmp->jingle = p_utf8 (c);
		      free (c);
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "PreSpot"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)))
		    tmp->prespot = g_strdup ("");
		  else
		    {
		      tmp->prespot = p_utf8 (c);
		      free (c);
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "PostSpot"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)))
		    tmp->postspot = g_strdup ("");
		  else
		    {
		      tmp->postspot = p_utf8 (c);
		      free (c);
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Priority"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)) || !atoi (c))
		    tmp->priority = 0;
		  else
		    tmp->priority = 1;

		  if (c)
		    free (c);
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "RandomItem"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)) || !atoi (c))
		    tmp->randomitem = 0;
		  else
		    tmp->randomitem = 1;

		  if (c)
		    free (c);
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "RandomSpot"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)) || !atoi (c))
		    tmp->randomspot = 0;
		  else
		    tmp->randomspot = 1;

		  if (c)
		    free (c);
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "SoftStop"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)) || !atoi (c))
		    tmp->softstop = 0;
		  else
		    tmp->softstop = 1;

		  if (c)
		    free (c);
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "RatioItem"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)) || !atoi (c))
		    tmp->ratioitem = 0;
		  else
		    tmp->ratioitem = atoi (c);

		  if (c)
		    free (c);
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "RatioSpot"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)) || !atoi (c))
		    tmp->ratiospot = 0;
		  else
		    tmp->ratiospot = atoi (c);

		  if (c)
		    free (c);
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "PathItem"))
		{
		  cur_item = cur_pl->children;

		  for (; cur_item != NULL; cur_item = cur_item->next)
		    {
		      if (!xmlStrcmp (cur_item->name, (xmlChar *) "item")
			  && (c =
			      (char *) xmlNodeListGetString (doc,
							     cur_item->
							     xmlChildrenNode,
							     1)))
			{
			  tmp->pathitem =
			    g_list_append (tmp->pathitem, p_utf8 (c));
			  free (c);
			}
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "PathSpot"))
		{
		  cur_item = cur_pl->children;

		  for (; cur_item != NULL; cur_item = cur_item->next)
		    {
		      if (!xmlStrcmp (cur_item->name, (xmlChar *) "item")
			  && (c =
			      (char *) xmlNodeListGetString (doc,
							     cur_item->
							     xmlChildrenNode,
							     1)))
			{
			  tmp->pathspot =
			    g_list_append (tmp->pathspot, p_utf8 (c));
			  free (c);
			}
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Start"))
		tmp_start =
		  (char *) xmlNodeListGetString (doc, cur_pl->xmlChildrenNode,
						 1);

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Stop"))
		tmp_stop =
		  (char *) xmlNodeListGetString (doc, cur_pl->xmlChildrenNode,
						 1);

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "TimeContinued"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)) || !atoi (c))
		    tmp_timecontinued = 0;
		  else
		    tmp_timecontinued = 1;

		  if (c)
		    free (c);
		}
	    }

	  tmp->timer =
	    timer_time_parser (tmp_start, tmp_stop, tmp_timecontinued);

	  if (tmp_start)
	    free (tmp_start);
	  if (tmp_stop)
	    free (tmp_stop);

	  if (!tmp->timer)
	    {
	      xmlFreeDoc (doc);

	      palinsesto_free (*father);
	      *father = NULL;
	      return 1;
	    }

	  palinsesto_insert (tmp, father);
	}
    }

  xmlFreeDoc (doc);

  return 0;
}

/* Check of the correct palinsesto */
GList *
palinsesto_now (struct somad_data * data)
{
  GList *list = NULL;
  struct somad_data *pl;
  int i = 0;
  GList *min_list = NULL;
  int min_t = -1;

  pl = data;

  while (pl)
    {
      i = timer_check (pl->timer, get_time ());

      if (!i)
	list = g_list_append (list, pl);

      else if (min_t == -1)
	{
	  min_list = g_list_append (min_list, pl);
	  min_t = i;
	}

      else if (min_t > i)
	{
	  g_list_free (min_list);
	  min_list = g_list_append (NULL, pl);
	  min_t = i;
	}

      else if (min_t == i)
	{
	  min_list = g_list_append (min_list, pl);
	}

      pl = pl->next;
    }

  if (!list && min_t != -1)
    return min_list;

  return list;
}

void
palinsesto_insert (struct somad_data *data, struct somad_data **father)
{
  struct somad_data *tmp;

  if (!*father)
    {
      data->next = NULL;
      *father = data;
      return;
    }

  if (strcmp ((*father)->timer->start, data->timer->start) > 0)
    {
      data->next = *father;
      *father = data;
      return;
    }

  tmp = *father;
  while (tmp->next)
    {

      if (strcmp (tmp->next->timer->start, data->timer->start) > 0)
	{
	  data->next = tmp->next;
	  tmp->next = data;
	  return;
	}

      tmp = tmp->next;
    }

  tmp->next = data;
  data->next = NULL;
}

int
palinsesto_save_file (char *file, struct somad_data *tmp)
{
  FILE *fl;
  GList *list;
  char str_color[8];
  unsigned char red, blue, green;
  char *str;

  if (!(fl = fopen (file, "w")))
    return 1;

  fprintf (fl,
	   "<?xml version=\"1.0\" standalone=\"no\"?>\n"
	   "<!DOCTYPE PalinsestoXML SYSTEM \"http://www.somasuite.org/xml/soma-xml-palinsesto.dtd\">\n\n");

  fprintf (fl, "<!-- This palinsesto file is generated by %s %s. -->\n\n",
	   PACKAGE, VERSION);

  fprintf (fl, "<PalinsestoXML>\n\n");

  while (tmp)
    {
      if (tmp->color)
	{
	  red = (tmp->color->red >> 8);
	  blue = (tmp->color->blue >> 8);
	  green = (tmp->color->green >> 8);
	  snprintf
	    (str_color,
	     sizeof (str_color), "#%.2X%.2X%.2X", red, green, blue);
	}
      else
	str_color[0] = 0;

      fprintf (fl, "\t<Palinsesto>\n");

      str = p_xml_markup (tmp->description);
      fprintf (fl, "\t\t<Description color=\"%s\">%s</Description>\n\n",
	       str_color, str);
      g_free (str);

      fprintf (fl, "\t\t<Priority>%d</Priority>\n", tmp->priority);

      str = p_xml_markup (tmp->timer->start);
      fprintf (fl, "\t\t<Start>%s</Start>\n", str);
      g_free (str);

      str = p_xml_markup (tmp->timer->stop);
      fprintf (fl, "\t\t<Stop>%s</Stop>\n", str);
      g_free (str);

      fprintf (fl, "\t\t<TimeContinued>%d</TimeContinued>\n",
	       tmp->timer->timecontinued);

      fprintf (fl, "\t\t<SpotController>%d</SpotController>\n\n",
	       tmp->spotcontroller);

      switch (tmp->type)
	{
	case FILES:
	  fprintf (fl, "\t\t<Type>files</Type>\n\n");
	  break;
	case STREAM:
	  fprintf (fl, "\t\t<Type>stream</Type>\n\n");
	  break;
	case MODULE:
	  fprintf (fl, "\t\t<Type>module</Type>\n\n");
	  break;
	case SILENCE:
	  fprintf (fl, "\t\t<Type>silence</Type>\n\n");
	  break;
	}

      str = p_xml_markup (tmp->jingle);
      fprintf (fl, "\t\t<Jingle>%s</Jingle>\n\n", str);
      g_free (str);

      str = p_xml_markup (tmp->prespot);
      fprintf (fl, "\t\t<PreSpot>%s</PreSpot>\n\n", str);
      g_free (str);

      str = p_xml_markup (tmp->postspot);
      fprintf (fl, "\t\t<PostSpot>%s</PostSpot>\n\n", str);
      g_free (str);

      str = p_xml_markup (tmp->module);
      fprintf (fl, "\t\t<Module>%s</Module>\n", str);
      g_free (str);

      str = p_xml_markup (tmp->moduledata);
      fprintf (fl, "\t\t<ModuleData>%s</ModuleData>\n", str), g_free (str);

      str = p_xml_markup (tmp->stream);
      fprintf (fl, "\t\t<Stream>%s</Stream>\n\n", str);
      g_free (str);

      fprintf (fl, "\t\t<RandomItem>%d</RandomItem>\n", tmp->randomitem);
      fprintf (fl, "\t\t<RandomSpot>%d</RandomSpot>\n", tmp->randomspot);
      fprintf (fl, "\t\t<SoftStop>%d</SoftStop>\n\n", tmp->softstop);

      fprintf (fl, "\t\t<RatioItem>%d</RatioItem>\n", tmp->ratioitem);
      fprintf (fl, "\t\t<RatioSpot>%d</RatioSpot>\n\n", tmp->ratiospot);

      fprintf (fl, "\t\t<PathItem>\n");

      list = tmp->pathitem;
      while (list)
	{
	  str = p_xml_markup ((char *) list->data);
	  fprintf (fl, "\t\t\t<item>%s</item>\n", str);
	  g_free (str);

	  list = list->next;
	}
      fprintf (fl, "\t\t</PathItem>\n\n");

      fprintf (fl, "\t\t<PathSpot>\n");

      list = tmp->pathspot;
      while (list)
	{
	  str = p_xml_markup ((char *) list->data);
	  fprintf (fl, "\t\t\t<item>%s</item>\n", str);
	  g_free (str);

	  list = list->next;
	}
      fprintf (fl, "\t\t</PathSpot>\n\n");

      fprintf (fl, "\t</Palinsesto>\n\n");

      tmp = tmp->next;
    }

  fprintf (fl, "\n</PalinsestoXML>\n");
  fclose (fl);

  chmod (file, 0640);

  return 0;
}

/* Trim function */
char *
palinsesto_trim (char *tmp)
{
  int i = 0;
  while (tmp[i] == ' ' || tmp[i] == '\t' || tmp[i] == '\r' || tmp[i] == '\n')
    tmp++;

  i = strlen (tmp);
  i--;

  while (tmp[i] == ' ' || tmp[i] == '\t' || tmp[i] == '\r' || tmp[i] == '\n')
    i--;

  tmp[i + 1] = 0;

  return tmp;
}

struct somad_data *
palinsesto_dump (struct somad_data *dump_pl)
{
  struct somad_data *tmp;
  GList *list;

  tmp = g_malloc (sizeof (struct somad_data));
  memcpy (tmp, dump_pl, sizeof (struct somad_data));

  list = dump_pl->pathitem;
  tmp->pathitem = NULL;
  while (list)
    {
      tmp->pathitem =
	g_list_append (tmp->pathitem, g_strdup ((gchar *) list->data));
      list = list->next;
    }

  list = dump_pl->pathspot;
  tmp->pathspot = NULL;
  while (list)
    {
      tmp->pathspot =
	g_list_append (tmp->pathspot, g_strdup ((gchar *) list->data));
      list = list->next;
    }

  if (dump_pl->stream)
    tmp->stream = g_strdup (dump_pl->stream);
  if (dump_pl->jingle)
    tmp->jingle = g_strdup (dump_pl->jingle);

  if (dump_pl->module)
    tmp->module = g_strdup (dump_pl->module);

  if (dump_pl->moduledata)
    tmp->moduledata = g_strdup (dump_pl->moduledata);

  tmp->timer = g_malloc (sizeof (struct somad_time));
  memcpy (tmp->timer, dump_pl->timer, sizeof (struct somad_time));

  if (dump_pl->timer->start)
    tmp->timer->start = g_strdup (dump_pl->timer->start);

  if (dump_pl->timer->stop)
    tmp->timer->stop = g_strdup (dump_pl->timer->stop);

  if (dump_pl->description)
    tmp->description = g_strdup (dump_pl->description);

  tmp->color = (GdkColor *) g_malloc (sizeof (GdkColor));
  memcpy (tmp->color, dump_pl->color, sizeof (GdkColor));

  tmp->next = NULL;

  return tmp;
}

/* EOF */
