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

static int spot_parser_real (char *, struct somad_spot_data **, gboolean);

void
spot_free (struct somad_spot_data *spot)
{
  struct somad_spot_data *tmp, *old;
  GList *list;

  if (!spot)
    return;

  tmp = spot;

  while (tmp)
    {

      if (tmp->path)
	{
	  list = tmp->path;
	  while (list)
	    {
	      g_free (list->data);
	      list = list->next;
	    }

	  g_list_free (tmp->path);
	}

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
spot_null (void *fl, const char *str, ...)
{
  /* void... */
}

int
spot_check_file (char *file)
{
  xmlDocPtr doc;
  xmlNodePtr cur;

  xmlSetGenericErrorFunc (xmlGenericErrorContext, spot_null);

  if (!(doc = xmlParseFile (file)))
    return 1;

  if (!(cur = xmlDocGetRootElement (doc)))
    {
      xmlFreeDoc (doc);
      return 1;
    }

  if (xmlStrcmp (cur->name, (xmlChar *) "SpotXML"))
    {
      xmlFreeDoc (doc);
      return 1;
    }

  xmlFreeDoc (doc);
  return 0;
}

int
spot_parser_file (char *file, struct somad_spot_data **father)
{
  return spot_parser_real (file, father, TRUE);
}

int
spot_parser (char *pl, struct somad_spot_data **father)
{
  return spot_parser_real (pl, father, FALSE);
}

static int
spot_parser_real (char *pl, struct somad_spot_data **father,
		      gboolean file_buf)
{
  struct somad_spot_data *tmp;
  xmlDocPtr doc;
  xmlNodePtr cur, cur_pl, cur_item;
  char *tmp_start = NULL;
  char *tmp_stop = NULL;
  int tmp_timecontinued = 0;
  char *c;

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

  if (xmlStrcmp (cur->name, (xmlChar *) "SpotXML"))
    {
      xmlFreeDoc (doc);
      return 1;
    }

  for (cur = cur->children; cur != NULL; cur = cur->next)
    {

      if (!xmlStrcmp (cur->name, (xmlChar *) "Spot"))
	{
	  tmp = g_malloc0 (sizeof (struct somad_spot_data));

	  for (cur_pl = cur->children; cur_pl != NULL; cur_pl = cur_pl->next)
	    {
	      if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Description"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)))
		    tmp->description = g_strdup (_("No description"));
		  else
		    {
		      tmp->description = p_utf8 (c);
		      free (c);
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Path"))
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
			  tmp->path = g_list_append (tmp->path, p_utf8 (c));
			  free (c);
			}
		    }
		}

	      else if (!xmlStrcmp (cur_pl->name, (xmlChar *) "Repeat"))
		{
		  if (!
		      (c =
		       (char *) xmlNodeListGetString (doc,
						      cur_pl->xmlChildrenNode,
						      1)) || !atoi (c))
		    tmp->repeat = 0;
		  else
		    tmp->repeat = atoi (c);

		  if (c)
		    free (c);
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

	      spot_free (*father);
	      *father = NULL;
	      return 1;
	    }

	  spot_insert (tmp, father);
	}
    }

  xmlFreeDoc (doc);

  return 0;
}

/* Check of the correct Spot struct */
struct somad_spot_data *
spot_now (struct somad_spot_data *data)
{
  struct somad_spot_data *tmp = NULL, *spot;
  int i, j;

  i = j = 0;

  spot = data;

  while (spot)
    {
      i = timer_check (spot->timer, get_time ());

      if (!i)
	return spot;

      if (!tmp || i < j)
	{
	  j = i;
	  tmp = spot;
	}

      spot = spot->next;
    }

  return tmp;
}

void
spot_insert (struct somad_spot_data *data, struct somad_spot_data **father)
{
  struct somad_spot_data *tmp;

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
spot_save_file (char *file, struct somad_spot_data *tmp)
{
  FILE *fl;
  GList *list;
  char *str;

  if (!(fl = fopen (file, "w")))
    return 1;

  fprintf (fl,
	   "<?xml version=\"1.0\" standalone=\"no\"?>\n"
	   "<!DOCTYPE SpotXML SYSTEM \"http://www.somasuite.org/xml/soma-xml-spot.dtd\">\n\n");

  fprintf (fl, "<!-- This Spot file is generated by %s %s. -->\n\n", PACKAGE,
	   VERSION);

  fprintf (fl, "<SpotXML>\n\n");

  while (tmp)
    {
      fprintf (fl, "\t<Spot>\n");

      str = p_xml_markup (tmp->description);
      fprintf (fl, "\t\t<Description>%s</Description>\n\n", str);
      g_free (str);

      str = p_xml_markup (tmp->timer->start);
      fprintf (fl, "\t\t<Start>%s</Start>\n", str);
      g_free (str);

      str = p_xml_markup (tmp->timer->stop);
      fprintf (fl, "\t\t<Stop>%s</Stop>\n", str);
      g_free (str);

      fprintf (fl, "\t\t<TimeContinued>%d</TimeContinued>\n\n",
	       tmp->timer->timecontinued);
      fprintf (fl, "\t\t<Repeat>%d</Repeat>\n\n", tmp->repeat);

      fprintf (fl, "\t\t<Path>\n");

      list = tmp->path;
      while (list)
	{
	  str = p_xml_markup ((char *) list->data);
	  fprintf (fl, "\t\t\t<item>%s</item>\n", str);
	  g_free (str);

	  list = list->next;
	}

      fprintf (fl, "\t\t</Path>\n\n");
      fprintf (fl, "\t</Spot>\n\n");

      tmp = tmp->next;
    }

  fprintf (fl, "\n</SpotXML>\n");
  fclose (fl);
  chmod (file, 0640);

  return 0;
}

struct somad_spot_data *
spot_dump (struct somad_spot_data *dump_spot)
{
  struct somad_spot_data *tmp;
  GList *list;

  tmp = g_malloc (sizeof (struct somad_spot_data));
  memcpy (tmp, dump_spot, sizeof (struct somad_spot_data));

  list = dump_spot->path;
  tmp->path = NULL;
  while (list)
    {
      tmp->path = g_list_append (tmp->path, g_strdup ((gchar *) list->data));
      list = list->next;
    }

  tmp->timer = g_malloc (sizeof (struct somad_time));
  memcpy (tmp->timer, dump_spot->timer, sizeof (struct somad_time));

  if (dump_spot->timer->start)
    tmp->timer->start = g_strdup (dump_spot->timer->start);

  if (dump_spot->timer->stop)
    tmp->timer->stop = g_strdup (dump_spot->timer->stop);

  if (dump_spot->description)
    tmp->description = g_strdup (dump_spot->description);

  tmp->next = NULL;

  return tmp;
}

/* EOF */
