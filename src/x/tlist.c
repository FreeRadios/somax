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
#include "../palinsesto/palinsesto.h"

struct tlist_tree_t
{
  gchar *value;
  gchar *full;
  GList *children;
};

struct tlist_data_t
{
  GStaticMutex mutex;

  GList *item_list;
  GList *item_work;
  gboolean item_done;
  GList *item_tree;
  GThread *item_thread;
  gboolean item_expanded;

  GList *spot_list;
  GList *spot_work;
  gboolean spot_done;
  GList *spot_tree;
  GThread *spot_thread;
  gboolean spot_expanded;
};

static struct tlist_data_t tlist_data =
  { G_STATIC_MUTEX_INIT, NULL, NULL, FALSE, NULL, NULL, TRUE, NULL, NULL,
  FALSE, NULL, NULL, TRUE
};

static GtkWidget *tlist_popup (GtkWidget *, gboolean);
static void tlist_popup_remove (GtkWidget *, GtkWidget * tv);
static void tlist_popup_ns (GtkWidget *, GtkWidget * tv);
static void tlist_popup_show (GtkWidget *, GtkWidget * tv);
static void tlist_popup_expand (GtkWidget *, GtkWidget * tv);
static void tlist_popup_collapse (GtkWidget *, GtkWidget * tv);

static gpointer tlist_thread (gboolean);
static gboolean tlist_compare (GList * new, GList ** old);
static void tlist_tree_free (GList * data);
static GList *tlist_tree_new (GList * list);

static void tlist_tree_set (GList ** root, GList * path, gchar * full);
static GList *tlist_tree_split (gchar * file);
static void tlist_tree_sort (GList ** list);
static gint tlist_tree_compare (struct tlist_tree_t *a,
				struct tlist_tree_t *b);
static void tlist_popule (GtkTreeView * tv, GtkWidget * label, GList * tree);
static void tlist_popule_stat (GtkTreeView * tv, GtkWidget * label);
static gint tlist_get_index (gchar * file, gboolean type);

static GList *
tlist_get_glist (char **list)
{
  GList *l = NULL;
  gchar *tmp;

  if (!list)
    return NULL;

  while (*list)
    {
      tmp = somax_to_utf8 (*list);
      l = g_list_append (l, tmp);
      list++;
    }

  return l;
}

static void
tlist_glist_free (GList * list)
{
  g_list_foreach (list, (GFunc) g_free, NULL);
  g_list_free (list);
}

void
tlist_new (char **what, gboolean type)
{
  if (!what)
    return;

  g_static_mutex_lock (&tlist_data.mutex);

  if (type == TRUE)
    {
      if (!tlist_data.item_thread)
	{
	  if ((tlist_data.item_work = tlist_get_glist (what))
	      && !(tlist_data.item_thread =
		   g_thread_create ((GThreadFunc) tlist_thread,
				    (gpointer) TRUE, FALSE, NULL)))
	    tlist_glist_free (tlist_data.item_work);
	}
    }

  else
    {
      if (!tlist_data.spot_thread)
	{
	  if ((tlist_data.spot_work = tlist_get_glist (what))
	      && !(tlist_data.spot_thread =
		   g_thread_create ((GThreadFunc) tlist_thread,
				    (gpointer) FALSE, FALSE, NULL)))
	    tlist_glist_free (tlist_data.spot_work);
	}
    }

  g_static_mutex_unlock (&tlist_data.mutex);
}

gpointer
tlist_thread (gboolean type)
{
  GList **old, *new;

  g_static_mutex_lock (&tlist_data.mutex);

  if (type == TRUE)
    {
      new = tlist_data.item_work;
      old = &tlist_data.item_list;
      tlist_data.item_work = NULL;
    }

  else
    {
      new = tlist_data.spot_work;
      old = &tlist_data.spot_list;
      tlist_data.spot_work = NULL;
    }

  g_static_mutex_unlock (&tlist_data.mutex);

  /* Compare and free: */
  if (tlist_compare (new, old) == TRUE)
    {
      GList *data = NULL;
      GList *prev;

      /* New tree: */
      data = tlist_tree_new (*old);

      /* Set new data: */
      g_static_mutex_lock (&tlist_data.mutex);

      if (type == TRUE)
	{
	  prev = tlist_data.item_tree;
	  tlist_data.item_tree = data;
	  tlist_data.item_done = TRUE;
	}
      else
	{
	  prev = tlist_data.spot_tree;
	  tlist_data.spot_tree = data;
	  tlist_data.spot_done = TRUE;
	}

      g_static_mutex_unlock (&tlist_data.mutex);

      /* Free old data: */
      if (prev)
	tlist_tree_free (prev);
    }

  g_static_mutex_lock (&tlist_data.mutex);

  if (type == TRUE)
    tlist_data.item_thread = NULL;
  else
    tlist_data.spot_thread = NULL;

  g_static_mutex_unlock (&tlist_data.mutex);

  g_thread_exit (NULL);
  return NULL;
}

static gboolean
tlist_compare (GList * new, GList ** old)
{
  GList *a, *b;
  int ret = 0;

  if (new)
    ret++;
  if (*old)
    ret++;

  switch (ret)
    {
    case 0:
      return FALSE;

    case 1:
      if (new)
	{
	  g_static_mutex_lock (&tlist_data.mutex);
	  *old = new;
	  g_static_mutex_unlock (&tlist_data.mutex);
	}

      else
	{
	  g_static_mutex_lock (&tlist_data.mutex);
	  tlist_glist_free (*old);
	  *old = NULL;
	  g_static_mutex_unlock (&tlist_data.mutex);
	}
      return TRUE;

    case 2:
    default:
      break;
    }

  b = *old;
  a = new;
  ret = 0;

  while (a && b)
    {
      if (strcmp ((char *) a->data, (char *) b->data))
	{
	  ret = 1;
	  break;
	}

      a = a->next;
      b = b->next;
    }

  if (ret || (a && !b) || (!a && b))
    {
      g_static_mutex_lock (&tlist_data.mutex);
      tlist_glist_free (*old);
      *old = new;
      g_static_mutex_unlock (&tlist_data.mutex);
      return TRUE;
    }

  tlist_glist_free (new);
  return FALSE;
}

static GList *
tlist_tree_new (GList * list)
{
  GList *path;
  GList *root = NULL;

  for (; list; list = list->next)
    {
      if (!(path = tlist_tree_split (list->data)))
	continue;

      tlist_tree_set (&root, path, list->data);
      tlist_glist_free (path);
    }

  tlist_tree_sort (&root);

  return root;
}

static void
tlist_tree_set (GList ** root, GList * path, gchar * full)
{
  GList *list;
  struct tlist_tree_t *tree;
  GList *parent = NULL;

  for (list = *root; list; list = list->next)
    {
      tree = list->data;

      if (!strcmp (tree->value, path->data))
	{
	  if (path->next)
	    {
	      tlist_tree_set (&tree->children, path->next, full);
	      return;
	    }
	  else
	    break;
	}
    }

  for (; path; path = path->next)
    {
      tree = g_malloc0 (sizeof (struct tlist_tree_t));
      tree->value = g_strdup (path->data);
      tree->full = g_strdup (full);

      if (!parent)
	{
	  *root = g_list_prepend (*root, tree);
	  parent = *root;
	}

      else
	{
	  struct tlist_tree_t *tmp = parent->data;
	  tmp->children = g_list_prepend (tmp->children, tree);
	  parent = tmp->children;
	}
    }
}

static void
tlist_tree_sort (GList ** list)
{
  GList *l;
  struct tlist_tree_t *tree;

  if (!*list)
    return;

  *list = g_list_sort (*list, (GCompareFunc) tlist_tree_compare);

  for (l = *list; l; l = l->next)
    {
      tree = l->data;
      tlist_tree_sort (&tree->children);
    }
}

static gint
tlist_tree_compare (struct tlist_tree_t *a, struct tlist_tree_t *b)
{
  return g_ascii_strcasecmp (a->value, b->value);
}

/* Splitto i singoli elementi della path e li metto in una lista: */
static GList *
tlist_tree_split (gchar * file)
{
  gint len;
  GList *list = NULL;
  gchar *path;
  gchar **a;
  GString *str = g_string_new (NULL);

  a = g_strsplit (file, "/", 0);
  for (len = 0; a && a[len]; len++)
    if (*a[len])
      g_string_append_printf (str, "/%s", a[len]);

  file = g_string_free (str, FALSE);
  len = strlen (file);

  while (file[len - 1] == '/')
    len--;
  file[len] = 0;

  while (len > 2 && (path = g_path_get_basename (file)))
    {
      list = g_list_prepend (list, path);
      len -= strlen (path) + 1;

      while (file[len - 1] == '/')
	len--;
      file[len] = 0;
    }

  g_free (file);
  return list;
}

static void
tlist_tree_free (GList * data)
{
  struct tlist_tree_t *t;

  for (; data; data = data->next)
    {
      t = data->data;

      if (t->children)
	tlist_tree_free (t->children);

      if (t->value)
	g_free (t->value);

      if (t->full)
	g_free (t->full);

      g_free (t);
    }

  g_list_free (data);
}

void
tlist_refresh (gboolean type)
{
  gboolean done = FALSE;
  GList *tree = NULL;
  GtkTreeView *tv = NULL;
  GtkWidget *label = NULL;
  gboolean expanded;

  g_static_mutex_lock (&tlist_data.mutex);

  if (type == TRUE)
    {
      if (tlist_data.item_done == TRUE)
	{
	  tlist_data.item_done = FALSE;
	  tree = tlist_data.item_tree;
	  done = TRUE;
	}
    }
  else
    {
      if (tlist_data.spot_done == TRUE)
	{
	  tlist_data.spot_done = FALSE;
	  tree = tlist_data.spot_tree;
	  done = TRUE;
	}
    }

  g_static_mutex_unlock (&tlist_data.mutex);

  if (done == FALSE)
    return;

  if (type == TRUE)
    {
      tv = GTK_TREE_VIEW (tlist_item_tv);
      label = tlist_item_label;
      expanded = tlist_data.item_expanded;
    }
  else
    {
      tv = GTK_TREE_VIEW (tlist_spot_tv);
      label = tlist_spot_label;
      expanded = tlist_data.spot_expanded;
    }

  if (done == TRUE)
    tlist_popule (tv, label, tree);
  else
    tlist_popule_stat (tv, label);

  if (expanded == TRUE)
    gtk_tree_view_expand_all (GTK_TREE_VIEW (tv));
  else
    gtk_tree_view_collapse_all (GTK_TREE_VIEW (tv));
}

static void
tlist_popule_remove_rec (GtkTreeModel * model, GtkTreeIter * iter)
{
  GtkTreeIter child;

  while (gtk_tree_model_iter_nth_child
	 (GTK_TREE_MODEL (model), &child, iter, 0))
    {
      tlist_popule_remove_rec (model, &child);
      gtk_tree_store_remove (GTK_TREE_STORE (model), &child);
    }
}

static void
tlist_popule_rec (GtkTreeModel * model, GtkTreeIter * iter, GList * tree,
		  gboolean * over, int64_t * duration)
{
  struct tlist_tree_t *t;
  GtkTreeIter child;
  somax_stat_t *stat;

  for (; tree; tree = tree->next)
    {
      t = tree->data;

      gtk_tree_store_append (GTK_TREE_STORE (model), &child, iter);

      if (!t->children && (stat = stat_get (t->full)))
	{
	  gtk_tree_store_set (GTK_TREE_STORE (model), &child, LIST_STYLE, 0,
			      LIST_FILENAME, t->full,
			      LIST_DURATION,
			      stat_duration (stat->stat->duration),
			      LIST_FILE,
			      stat->stat->title[0] ? stat->stat->
			      title : stat_make_name (t->full), -1);

	  (*duration) += stat->stat->duration;

	  stat_unref (stat);
	}

      else if (t->children)
	{
	  gtk_tree_store_set (GTK_TREE_STORE (model), &child, LIST_STYLE, 0,
			      LIST_FILE, t->value, -1);
	}

      else
	{
	  gtk_tree_store_set (GTK_TREE_STORE (model), &child, LIST_STYLE, 0,
			      LIST_FILENAME, t->full, LIST_FILE,
			      stat_make_name (t->full), -1);
	  *over = TRUE;
	}

      tlist_popule_rec (model, &child, t->children, over, duration);
    }
}

static void
tlist_popule (GtkTreeView * tv, GtkWidget * label, GList * tree)
{
  GtkTreeModel *model;
  gboolean over = FALSE;
  int64_t duration = 0;
  gchar s[1024];

  model = gtk_tree_view_get_model (tv);
  tlist_popule_remove_rec (model, NULL);

  tlist_popule_rec (model, NULL, tree, &over, &duration);

  snprintf (s, sizeof (s), "%s%s", over ? "> " : "",
	    stat_duration (duration));
  gtk_label_set_text (GTK_LABEL (label), s);
}

static void
tlist_popule_stat_rec (GtkTreeModel * model, GtkTreeIter * iter,
		       gboolean * over, int64_t * duration)
{
  GtkTreeIter child;
  somax_stat_t *stat;
  gchar *file;

  do
    {
      gtk_tree_model_get (model, iter, LIST_FILENAME, &file, -1);

      if (file && (stat = stat_get (file)))
	{
	  gtk_tree_store_set (GTK_TREE_STORE (model), iter, LIST_STYLE, 0,
			      LIST_FILENAME, file,
			      LIST_DURATION,
			      stat_duration (stat->stat->duration),
			      LIST_FILE,
			      stat->stat->title[0] ? stat->stat->
			      title : stat_make_name (file), -1);

	  (*duration) += stat->stat->duration;

	  stat_unref (stat);
	}

      else
	*over = TRUE;

      if (file)
	g_free (file);

      if (gtk_tree_model_iter_has_child (model, iter) == TRUE
	  && gtk_tree_model_iter_children (model, &child, iter) == TRUE)
	tlist_popule_stat_rec (model, &child, over, duration);

    }
  while (gtk_tree_model_iter_next (model, iter));
}

static void
tlist_popule_stat (GtkTreeView * tv, GtkWidget * label)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean over = FALSE;
  int64_t duration = 0;
  gchar s[1024];

  model = gtk_tree_view_get_model (tv);
  if (gtk_tree_model_get_iter_first (model, &iter) == TRUE)
    tlist_popule_stat_rec (model, &iter, &over, &duration);

  snprintf (s, sizeof (s), "%s%s", over ? "> " : "",
	    stat_duration (duration));
  gtk_label_set_text (GTK_LABEL (label), s);
}

void
tlist_selected (GtkTreeView * tree, GtkTreePath * path,
		GtkTreeViewColumn * column)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *file;

  model = gtk_tree_view_get_model (tree);
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LIST_FILENAME, &file, -1);

  if (file)
    {
      stat_popup (file);
      g_free (file);
    }
}

gint
tlist_popup_menu (GtkWidget * tv, GdkEventButton * event, gpointer dummy)
{
  static GtkWidget *menu_item = NULL;
  static GtkWidget *menu_spot = NULL;

  if (event->button == 3)
    {
      GtkTreeSelection *selection;
      GtkTreeModel *model;
      GList *list;

      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
      list = gtk_tree_selection_get_selected_rows (selection, &model);

      if (!list)
	return FALSE;

      g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (list);

      if (!dummy)
	{
	  if (!menu_item)
	    menu_item = tlist_popup (tv, 0);
	  gtk_menu_popup (GTK_MENU (menu_item), NULL, NULL, NULL, NULL,
			  event->button, event->time);
	}
      else
	{
	  if (!menu_spot)
	    menu_spot = tlist_popup (tv, 1);
	  gtk_menu_popup (GTK_MENU (menu_spot), NULL, NULL, NULL, NULL,
			  event->button, event->time);
	}

      return TRUE;
    }

  return FALSE;
}

static GtkWidget *
tlist_popup (GtkWidget * tv, gboolean sp)
{
  GtkWidget *menu;
  GtkWidget *item;

  menu = gtk_menu_new ();

  item = gtk_image_menu_item_new_with_mnemonic (_("Show info"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (tlist_popup_show), tv);

  item = gtk_image_menu_item_new ();
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  item = gtk_image_menu_item_new_with_mnemonic (_("Insert in NextItem list"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (tlist_popup_ns), tv);

  item = gtk_image_menu_item_new_with_mnemonic (_("Remove item"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  g_object_set_data (G_OBJECT (item), "sp", (gpointer) sp);
  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (tlist_popup_remove), tv);

  item = gtk_image_menu_item_new ();
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  item = gtk_image_menu_item_new_with_mnemonic (_("Expand tree"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (tlist_popup_expand), tv);

  item = gtk_image_menu_item_new_with_mnemonic (_("Collapse tree"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (tlist_popup_collapse), tv);
  g_object_set_data (G_OBJECT (tv), "expanded",
		     !sp ? &tlist_data.item_expanded : &tlist_data.
		     spot_expanded);

  return menu;
}

static void
tlist_popup_show (GtkWidget * w, GtkWidget * tv)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *list;
  char *name;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));

  if (!
      (list =
       gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection),
					     &model)))
    return;

  if (gtk_tree_model_get_iter (model, &iter, list->data) == TRUE)
    {
      gtk_tree_model_get (model, &iter, LIST_FILENAME, &name, -1);

      if (name)
	{
	  stat_popup (name);
	  g_free (name);
	}
    }

  g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (list);
}

static void
tlist_popup_ns (GtkWidget * w, GtkWidget * tv)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *list, *b;
  int len, i;
  GtkWidget *tmp_win;
  char *name;
  GdkCursor *cursor;
  char s[1024];

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));

  if (!
      (list =
       gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection),
					     &model)))
    return;

  b = list;

  len = g_list_length (list);
  i = 1;

  tmp_win = somax_win ();
  gtk_widget_show_all (tmp_win);

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (tmp_win->window, cursor);

  while (list)
    {
      if (gtk_tree_model_get_iter (model, &iter, list->data) == TRUE)
	{
	  gtk_tree_model_get (model, &iter, LIST_FILENAME, &name, -1);

	  if (name)
	    {
	      snprintf (s, sizeof (s), "Adding... %d of %d", i++, len);
	      somax_nextitem_set (tmp_win, s, controller, name);

	      g_free (name);
	    }
	}

      gtk_tree_path_free (list->data);
      list = list->next;
    }

  g_list_free (b);

  gdk_cursor_unref (cursor);
  gtk_widget_destroy (tmp_win);
}

static void
tlist_popup_remove (GtkWidget * w, GtkWidget * tv)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *list, *b;
  int len, i, k, j;
  GtkWidget *tmp_win;
  GdkCursor *cursor;
  char s[1024];
  gboolean sp;
  gchar *file;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
  sp = (gboolean) g_object_get_data (G_OBJECT (w), "sp");

  if (!
      (list =
       gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection),
					     &model)))
    return;

  b = list;

  len = g_list_length (list);
  i = 0;
  j = 0;

  tmp_win = somax_win ();
  gtk_widget_show_all (tmp_win);

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (tmp_win->window, cursor);

  while (list)
    {
      i++;

      if (gtk_tree_model_get_iter (model, &iter, list->data) == TRUE)
	{
	  gtk_tree_model_get (model, &iter, LIST_FILENAME, &file, -1);

	  if (file && (k = tlist_get_index (file, !sp ? TRUE : FALSE)) >= 0)
	    {
	      snprintf (s, sizeof (s), "Removing... %d of %d", i, len);

	      if (!sp)
		somax_remove_item (tmp_win, s, controller, k - j);
	      else
		somax_remove_spot (tmp_win, s, controller, k - j);

	      j++;
	    }

	  if (file)
	    g_free (file);
	}

      gtk_tree_path_free (list->data);
      list = list->next;
    }

  g_list_free (b);

  gdk_cursor_unref (cursor);
  gtk_widget_destroy (tmp_win);
}

static gint
tlist_get_index (gchar * file, gboolean type)
{
  gint id = 0;
  GList *list;

  g_static_mutex_lock (&tlist_data.mutex);
  if (type)
    list = tlist_data.item_list;
  else
    list = tlist_data.spot_list;

  for (; list; list = list->next, id++)
    if (!strcmp (list->data, file))
      {
	g_static_mutex_unlock (&tlist_data.mutex);
	return id;
      }

  g_static_mutex_unlock (&tlist_data.mutex);

  return -1;
}

static void
tlist_popup_expand (GtkWidget * w, GtkWidget * tv)
{
  gboolean *expanded = g_object_get_data (G_OBJECT (tv), "expanded");
  *expanded = TRUE;
  gtk_tree_view_expand_all (GTK_TREE_VIEW (tv));
}

static void
tlist_popup_collapse (GtkWidget * w, GtkWidget * tv)
{
  gboolean *expanded = g_object_get_data (G_OBJECT (tv), "expanded");
  *expanded = FALSE;
  gtk_tree_view_collapse_all (GTK_TREE_VIEW (tv));
}

/* EOF */
