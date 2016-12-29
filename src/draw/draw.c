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
#include "draw.h"
#include "../palinsesto/palinsesto.h"

#define DRAW_MAX 20
#define DRAW_SS 20
#define DRAW_DESC 8
#define DRAW_PIXEL 10
#define DRAW_SPAN 10

static struct somad_data *draw_resizable_pl = NULL;
static gboolean draw_resizing = FALSE;
static gint draw_resizing_id = FALSE;
static gint draw_resizing_start = FALSE;
static gint draw_resizing_stop = FALSE;
static gint draw_resizing_x = 0;
static gboolean draw_resizing_right = FALSE;

static void draw_softstop (GtkWidget *, GdkColor *, int, int, int, int);
static void draw_rectangle (GtkWidget *, int, int, GdkGC *, int, char *,
			    GdkColor *, gboolean, gboolean);
static gboolean draw_expose_event (GtkWidget *, GdkEventExpose *,
				   struct somad_data **);
static void draw_refresh_true (GtkWidget *, struct somad_data *, gboolean,
			       time_t, gboolean);
static void draw_zoom_in (GtkWidget *, GtkWidget *);
static void draw_zoom_out (GtkWidget *, GtkWidget *);
static GtkWidget *draw_menu (GtkWidget *);
static void draw_zoom (GtkWidget *, gpointer);
static gint draw_press (GtkWidget *, GdkEventButton *, gpointer);
static gint draw_release (GtkWidget *, GdkEventButton *, gpointer);
static gint draw_motion (GtkWidget *, GdkEventMotion *);

static void draw_tooltip (GtkWidget * widget);
static gint draw_tooltip_expose (GtkWidget * win);
static void draw_tooltip_hide (GtkWidget * draw);

GtkWidget *
draw_new (void *expose_data,
	  gboolean (*button_func) (GtkWidget *, GdkEventButton *, gpointer),
	  void *button_data, gboolean resizable,
	  gboolean (*resize_func) (GtkWidget *, gpointer), void *resize_data)
{
  GtkWidget *box;
  GtkWidget *vbox;
  GtkWidget *widget;
  GtkWidget *draw;
  GtkWidget *image;
  GtkWidget *menu;
  PangoLayout *layout;
  int w, h;

  box = gtk_hbox_new (0, FALSE);

  draw = gtk_drawing_area_new ();
  gtk_widget_show (draw);
  gtk_box_pack_start (GTK_BOX (box), draw, TRUE, TRUE, 0);

  menu = draw_menu (draw);

  g_signal_connect ((gpointer) draw, "expose_event",
		    G_CALLBACK (draw_expose_event), expose_data);

  g_signal_connect (GTK_OBJECT (draw), "button_press_event",
		    GTK_SIGNAL_FUNC (draw_press), button_func);

  g_signal_connect (GTK_OBJECT (draw), "button_release_event",
		    GTK_SIGNAL_FUNC (draw_release), button_func);

  g_object_set_data (G_OBJECT (draw), "resize_func", resize_func);
  g_object_set_data (G_OBJECT (draw), "resize_data", resize_data);

  if (resizable == FALSE)
    gtk_widget_set_events (draw,
			   gtk_widget_get_events (draw) |
			   GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  else
    {
      g_signal_connect (G_OBJECT (draw), "motion_notify_event",
			G_CALLBACK (draw_motion), NULL);

      gtk_widget_set_events (draw,
			     gtk_widget_get_events (draw) |
			     GDK_BUTTON_PRESS_MASK |
			     GDK_BUTTON_RELEASE_MASK |
			     GDK_POINTER_MOTION_MASK | GDK_MOTION_NOTIFY);
    }


  layout = gtk_widget_create_pango_layout (draw, "1");
  pango_layout_get_pixel_size (layout, &w, &h);
  gtk_widget_set_size_request (draw, -1, (h * 3	/* Low and high transmission
						   and the time line */ )
			       + DRAW_PIXEL +
			       /* 2 up and 2 down on timeline + the line down
			          and 2 pixer up and down the line + 4 up and down the 
			          description of trasnmission */ +11);
  g_object_unref (layout);

  vbox = gtk_vbox_new (0, FALSE);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (box), vbox, FALSE, FALSE, 0);

  widget = gtk_button_new ();
  gtk_widget_show (widget);
  gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

  image = gtk_image_new_from_stock ("gtk-zoom-out", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (widget), image);

  g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (draw_zoom_out),
		    draw);

  widget = gtk_button_new ();
  gtk_widget_show (widget);
  gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

  image = gtk_image_new_from_stock ("gtk-zoom-in", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_container_add (GTK_CONTAINER (widget), image);

  g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (draw_zoom_in),
		    draw);

  g_object_set_data (G_OBJECT (box), "draw", draw);
  g_object_set_data (G_OBJECT (draw), "menu", menu);
  g_object_set_data (G_OBJECT (draw), "expose_data", expose_data);
  g_object_set_data (G_OBJECT (draw), "button_data", button_data);
  g_object_set_data (G_OBJECT (draw), "zoom", (void *) 1);

  return box;
}

GdkColor *
draw_color_str (char *str)
{
  static GdkColor *color;

  color = g_malloc (sizeof (GdkColor));
  if (!gdk_color_parse (str, color))
    {
      g_free (color);
      return NULL;
    }

  return color;
}

GdkColor *
draw_color (void)
{
  static GdkColor *color;
  static int k = -1;

  if (k == -1)
    {
      srand (getpid ());
      k++;
    }

  color = g_malloc (sizeof (GdkColor));
  color->red = getrandom (0x00, 0xffff);
  color->blue = getrandom (color->red >= 0x6666 ? 0x00 : 0x6666, 0xffff);
  color->green = getrandom (color->red >= 0x6666
			    && color->blue >= 0x6666 ? 0x00 : 0x6666, 0xffff);

  return color;
}

static void
draw_zoom_in (GtkWidget * w, GtkWidget * draw)
{
  time_t t;
  gboolean now;
  struct somad_data **expose_data;
  int zoom;

  t = (time_t) g_object_get_data (G_OBJECT (draw), "time");
  now = (gboolean) g_object_get_data (G_OBJECT (draw), "now");
  zoom = (int) g_object_get_data (G_OBJECT (draw), "zoom");

  zoom++;
  g_object_set_data (G_OBJECT (draw), "zoom", (void *) zoom);

  expose_data =
    (struct somad_data **) g_object_get_data (G_OBJECT (draw), "expose_data");

  draw_refresh_true (draw, *expose_data, FALSE, t, now);
}

static void
draw_zoom_out (GtkWidget * w, GtkWidget * draw)
{
  time_t t;
  gboolean now;
  struct somad_data **expose_data;
  int zoom;

  t = (time_t) g_object_get_data (G_OBJECT (draw), "time");
  now = (gboolean) g_object_get_data (G_OBJECT (draw), "now");
  zoom = (int) g_object_get_data (G_OBJECT (draw), "zoom");

  if (zoom == 1)
    return;

  zoom--;
  g_object_set_data (G_OBJECT (draw), "zoom", (void *) zoom);

  expose_data = g_object_get_data (G_OBJECT (draw), "expose_data");

  expose_data = (struct somad_data **) g_object_get_data (G_OBJECT (draw),
							  "expose_data");

  draw_refresh_true (draw, *expose_data, FALSE, t, now);
}

static void
draw_zoom (GtkWidget * w, gpointer data)
{
  time_t t;
  gboolean now;
  struct somad_data **expose_data;
  int zoom;
  GtkWidget *draw;

  draw = (GtkWidget *) g_object_get_data (G_OBJECT (w), "draw");

  t = (time_t) g_object_get_data (G_OBJECT (draw), "time");
  now = (gboolean) g_object_get_data (G_OBJECT (draw), "now");
  zoom = (int) g_object_get_data (G_OBJECT (draw), "zoom");

  switch ((int) data)
    {
    case 0:
      zoom++;
      break;
    case -1:
      if (zoom > 1)
	zoom--;
      else
	return;
      break;
    default:
      zoom = (int) data;
    }

  g_object_set_data (G_OBJECT (draw), "zoom", (void *) zoom);

  expose_data =
    (struct somad_data **) g_object_get_data (G_OBJECT (draw), "expose_data");

  draw_refresh_true (draw, *expose_data, FALSE, t, now);
}


static gboolean
draw_expose_event (GtkWidget * draw, GdkEventExpose * event,
		   struct somad_data **data)
{
  time_t t;
  gboolean now;

  t = (time_t) g_object_get_data (G_OBJECT (draw), "time");
  now = (gboolean) g_object_get_data (G_OBJECT (draw), "now");

  draw_refresh_true (draw, *data, FALSE, t, now);

  return TRUE;
}

static void
draw_rectangle (GtkWidget * draw, int a, int b, GdkGC * gc, int h, char *desc,
		GdkColor * color, gboolean softstop, gboolean priority)
{
  PangoLayout *layout;
  int width, height;
  int i;
  char s[1024];
  int p;

  if ((b - a - 2) <= 0)
    b += 2;

  layout = gtk_widget_create_pango_layout (draw, "1");

  p = (priority ? 0 : h + 1);

  gdk_draw_rectangle (draw->window, gc, TRUE, a + 1, DRAW_PIXEL + 1 + p, b -
		      a - 2, h - 1);
  gdk_draw_line (draw->window, draw->style->black_gc, a, DRAW_PIXEL + h + p,
		 b - 1, DRAW_PIXEL + h + p);
  gdk_draw_line (draw->window, draw->style->black_gc, b - 1, DRAW_PIXEL + p,
		 b - 1, DRAW_PIXEL + (h - 1) + p);
  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], a + 1,
		 DRAW_PIXEL + (h - 1) + p, b - 2, DRAW_PIXEL + (h - 1) + p);
  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], b - 2,
		 DRAW_PIXEL + p, b - 2, DRAW_PIXEL + (h - 1) + p);
  gdk_draw_line (draw->window, draw->style->light_gc[draw->state], a,
		 DRAW_PIXEL + p, b - 2, DRAW_PIXEL + p);
  gdk_draw_line (draw->window, draw->style->light_gc[draw->state], a,
		 DRAW_PIXEL + p, a, DRAW_PIXEL + h + p);

  if (desc)
    {
      for (i = strlen (desc); i; i--)
	{
	  snprintf (s, sizeof (s), "%s", desc);
	  s[i] = 0;
	  pango_layout_set_text (layout, s, i);
	  pango_layout_get_pixel_size (layout, &width, &height);

	  if (width <= (b - a) - DRAW_DESC)
	    break;
	}
    }
  else
    i = 0;

  if (i != 0)
    {
      gtk_paint_layout (draw->style, draw->window, GTK_STATE_NORMAL, TRUE,
			NULL, draw, NULL, b - ((b - a) / 2) - (width / 2),
			DRAW_PIXEL + ((h / 2) - (height / 2)) + p, layout);
    }

  /* The indicator is DRAW_PIXEL - 1 pixel of border up and down - 1 pixel up
   * and down the border - 1 pixel as separator form the timeline and 1 as
   * line */
  gdk_draw_line (draw->window, gc, a + 1, 2, a + 1, DRAW_PIXEL - 5);
  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], a, 1, a,
		 DRAW_PIXEL - 4);
  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], a + 2, 1,
		 a + 2, DRAW_PIXEL - 4);
  gdk_draw_point (draw->window, draw->style->dark_gc[draw->state], a + 1, 1);
  gdk_draw_point (draw->window, draw->style->dark_gc[draw->state], a + 1,
		  DRAW_PIXEL - 4);
  gdk_draw_line (draw->window, gc, b - 2, 2, b - 2, DRAW_PIXEL - 5);
  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], b - 3, 1,
		 b - 3, DRAW_PIXEL - 4);
  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], b - 1, 1,
		 b - 1, DRAW_PIXEL - 4);
  gdk_draw_point (draw->window, draw->style->dark_gc[draw->state], b - 2, 1);
  gdk_draw_point (draw->window, draw->style->dark_gc[draw->state], b - 2,
		  DRAW_PIXEL - 4);

  if (softstop)
    draw_softstop (draw, color, a + 1, b, h, p);

  g_object_unref (layout);
}

void
draw_refresh (GtkWidget * draw, struct somad_data *data, gboolean done,
	      time_t stime, gboolean now)
{
  GtkWidget *w;

  w = g_object_get_data (G_OBJECT (draw), "draw");

  if (GTK_WIDGET_DRAWABLE (w) == TRUE)
    draw_refresh_true (w, data, done, stime, now);

  draw_resizable_pl = NULL;
  draw_resizing = FALSE;
}

static void
draw_refresh_true (GtkWidget * draw, struct somad_data *data, gboolean done,
		   time_t stime, gboolean now)
{
  PangoLayout *layout;
  struct somad_data *pl;
  struct tm *k;
  int a, b, x1, x2;
  int w, width, height;
  int id, zoom;
  GdkGC *gc;
  char buf[1024];
  int pango_height, pango_width;
  struct tm check;
  GdkColor color;

  layout = gtk_widget_create_pango_layout (draw, "1");
  pango_layout_get_pixel_size (layout, &pango_width, &pango_height);

  width = draw->allocation.width - DRAW_MAX;
  height = (draw->allocation.height - pango_height - 7 - DRAW_PIXEL) / 2;

  zoom = (int) g_object_get_data (G_OBJECT (draw), "zoom");

  width /= zoom;

  gdk_window_clear (draw->window);
  gdk_draw_rectangle (draw->window, draw->style->bg_gc[draw->state], TRUE, 0,
		      0, width, height);

  gc = gdk_gc_new (draw->window);

  g_object_set_data (G_OBJECT (draw), "now", (void *) now);
  g_object_set_data (G_OBJECT (draw), "time", (void *) stime);

  for (id = 0; id < zoom; id++)
    {

      if (stime)
	k = localtime (&stime);
      else
	k = get_time ();

      if (id)
	{
	  GDate *date;

	  date =
	    g_date_new_dmy (k->tm_mday, k->tm_mon + 1, k->tm_year + 1900);
	  g_date_add_days (date, id);
	  k = g_malloc (sizeof (struct tm));
	  g_date_to_struct_tm (date, k);
	  g_date_free (date);
	}

      w = width * (id);

      pl = data;

      while (pl)
	{
	  if (!timer_check_day
	      (pl->timer, k->tm_year + 1900, k->tm_mon, k->tm_mday,
	       k->tm_wday))
	    {
	      a = pl->timer->start_min + (pl->timer->start_hour * 60);
	      b = pl->timer->stop_min + (pl->timer->stop_hour * 60);

	      if (a == 1440)
		a = 0;
	      if (b == 0)
		b = 1440;

	      /* A < B */
	      if (a <= b)
		{
		  x1 = w + ((a * width / 1440) + (DRAW_MAX / 2));
		  x2 = w + ((b * width / 1440) + (DRAW_MAX / 2));

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = pl->timer->start_min;
		  check.tm_hour = pl->timer->start_hour;

		  if (check.tm_min)
		    check.tm_min--;

		  else if (check.tm_hour)
		    {
		      check.tm_hour--;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mday)
		    {
		      check.tm_mday--;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mon)
		    {
		      check.tm_mon--;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else
		    {
		      check.tm_year--;
		      check.tm_mon = 11;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  if (timer_check (pl->timer, &check) < 1)
		    x1 = w + (DRAW_MAX / 2);

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = pl->timer->stop_min;
		  check.tm_hour = pl->timer->stop_hour;

		  if (pl->timer->stop_min != 59)
		    check.tm_min = pl->timer->stop_min + 1;

		  else if (pl->timer->stop_hour != 23)
		    {
		      check.tm_hour = pl->timer->stop_hour + 1;
		      check.tm_min = 0;
		    }

		  else if (pl->timer->stop_mday != 30)
		    {
		      check.tm_mday = pl->timer->stop_mday + 1;
		      check.tm_hour = 0;
		      check.tm_min = 0;
		    }

		  else if (pl->timer->stop_month != 11)
		    {
		      check.tm_mon = pl->timer->stop_month + 1;
		      check.tm_mday = 0;
		      check.tm_hour = 0;
		      check.tm_min = 0;
		    }

		  else
		    {
		      check.tm_year = (pl->timer->stop_year + 1) - 1900;
		      check.tm_mon = 0;
		      check.tm_mday = 0;
		      check.tm_hour = 0;
		      check.tm_min = 0;
		    }

		  if (timer_check (pl->timer, &check) < 1)
		    x2 = (width * (id + 1)) + (DRAW_MAX / 2);

		  gdk_gc_set_rgb_fg_color (gc, pl->color);

		  draw_rectangle (draw, x1, x2, gc, height, pl->description,
				  pl->color, pl->softstop, pl->priority);
		}

	      /* A > B */
	      else
		{
		  x1 = w + ((a * width / 1440) + (DRAW_MAX / 2));
		  x2 = w + ((b * width / 1440) + (DRAW_MAX / 2));

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = pl->timer->start_min;
		  check.tm_hour = pl->timer->start_hour;

		  if (check.tm_min)
		    check.tm_min--;

		  else if (check.tm_hour)
		    {
		      check.tm_hour--;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mday)
		    {
		      check.tm_mday--;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mon)
		    {
		      check.tm_mon--;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else
		    {
		      check.tm_year--;
		      check.tm_mon = 11;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  if (timer_check (pl->timer, &check) < 1)
		    {
		      x1 = w + (DRAW_MAX / 2);
		      x2 = (width * (id + 1)) + (DRAW_MAX / 2);
		    }

		  gdk_gc_set_rgb_fg_color (gc, pl->color);

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = 23;
		  check.tm_hour = 59;

		  if (timer_check (pl->timer, &check) < 1)
		    {
		      if (((width * (id + 1) + (DRAW_MAX / 2)) - x1) > 2)
			draw_rectangle (draw, x1,
					(width * (id + 1)) + (DRAW_MAX / 2),
					gc, height, pl->description,
					pl->color, pl->softstop,
					pl->priority);
		    }

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = 0;
		  check.tm_hour = 0;

		  if (timer_check (pl->timer, &check) < 1)
		    {
		      if ((x2 - (w + (DRAW_MAX / 2))) > 2)
			draw_rectangle (draw, w + (DRAW_MAX / 2), x2, gc,
					height, pl->description, pl->color,
					pl->softstop, pl->priority);
		    }
		}

	      pl->x1 = x1;
	      pl->x2 = x2;
	      pl->y = height;
	    }

	  else
	    {
	      pl->x1 = pl->x2 = pl->y = -1;
	    }

	  pl = pl->next;

	}

    }

  g_object_unref (gc);

  pango_layout_set_text (layout, "H", 1);
  pango_layout_get_pixel_size (layout, &pango_width, &pango_height);
  gtk_paint_layout (draw->style, draw->window, GTK_STATE_NORMAL, TRUE,
		    NULL, draw, NULL, DRAW_MAX / 2 - (pango_width + 1), 11,
		    layout);

  pango_layout_set_text (layout, "L", 1);
  gtk_paint_layout (draw->style, draw->window, GTK_STATE_NORMAL, TRUE,
		    NULL, draw, NULL, DRAW_MAX / 2 - (pango_width + 1),
		    11 + height + 2, layout);

  /* NUMBERS && LINES */
  if (zoom == 1)
    {
      height = draw->allocation.height - 1;
      for (a = 0; a < 25; a++)
	{
	  b = snprintf (buf, sizeof (buf), "%d", a);
	  pango_layout_set_text (layout, buf, b);
	  pango_layout_get_pixel_size (layout, &pango_width, &pango_height);
	  x1 = (((a * width) / 24) + (DRAW_MAX / 2)) - (pango_width / 2);
	  gtk_paint_layout (draw->style, draw->window, GTK_STATE_NORMAL, TRUE,
			    NULL, draw, NULL, x1, height - pango_height,
			    layout);
	  x1 = (((a * width) / 24) + (DRAW_MAX / 2));
	  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], x1,
			 height - pango_height - 2, x1,
			 height - pango_height);
	  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], x1,
			 height - 2, x1, height);
	}
    }
  else
    {
      height = draw->allocation.height - 1;

      for (a = 0; a <= zoom; a++)
	{
	  x1 = ((a * width) + (DRAW_MAX / 2));
	  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state],
			 x1, height - pango_height - 2, x1, height);

	}

      if (stime)
	k = localtime (&stime);
      else
	k = get_time ();

      b = snprintf (buf, sizeof (buf), "30 September");
      pango_layout_set_text (layout, buf, b);
      pango_layout_get_pixel_size (layout, &pango_width, &pango_height);

      a = 0;

      if (pango_width < width)
	{
	  GDate *date;

	  date =
	    g_date_new_dmy (k->tm_mday, k->tm_mon + 1, k->tm_year + 1900);

	  for (a = 0; a < zoom; a++)
	    {
	      if (a)
		g_date_add_days (date, 1);

	      b =
		snprintf (buf, sizeof (buf), "%d %s", g_date_get_day (date),
			  somax_month (g_date_get_month (date) - 1));

	      pango_layout_set_text (layout, buf, b);
	      pango_layout_get_pixel_size (layout, &pango_width,
					   &pango_height);
	      x1 =
		((a * width) + (DRAW_MAX / 2)) + ((width / 2) -
						  (pango_width / 2));
	      gtk_paint_layout (draw->style, draw->window, GTK_STATE_NORMAL,
				TRUE, NULL, draw, NULL, x1,
				height - pango_height, layout);
	      x1 = ((a * width) + (DRAW_MAX / 2));
	      gdk_draw_line (draw->window, draw->style->dark_gc[draw->state],
			     x1, height - pango_height - 2, x1,
			     height - pango_height);
	      gdk_draw_line (draw->window, draw->style->dark_gc[draw->state],
			     x1, height - 2, x1, height);
	    }

	  g_date_free (date);
	}

      if (a == 0)
	{

	  b = snprintf (buf, sizeof (buf), "30 sep");
	  pango_layout_set_text (layout, buf, b);
	  pango_layout_get_pixel_size (layout, &pango_width, &pango_height);

	  if (pango_width < width)
	    {
	      GDate *date;

	      date =
		g_date_new_dmy (k->tm_mday, k->tm_mon + 1, k->tm_year + 1900);

	      for (a = 0; a < zoom; a++)
		{
		  if (a)
		    g_date_add_days (date, 1);

		  b =
		    snprintf (buf, sizeof (buf), "%d %s",
			      g_date_get_day (date),
			      somax_mini_month (g_date_get_month (date) - 1));

		  pango_layout_set_text (layout, buf, b);
		  pango_layout_get_pixel_size (layout, &pango_width,
					       &pango_height);
		  x1 =
		    ((a * width) + (DRAW_MAX / 2)) + ((width / 2) -
						      (pango_width / 2));
		  gtk_paint_layout (draw->style, draw->window,
				    GTK_STATE_NORMAL, TRUE, NULL, draw, NULL,
				    x1, height - pango_height, layout);
		  x1 = ((a * width) + (DRAW_MAX / 2));
		  gdk_draw_line (draw->window,
				 draw->style->dark_gc[draw->state], x1,
				 height - pango_height - 2, x1,
				 height - pango_height);
		  gdk_draw_line (draw->window,
				 draw->style->dark_gc[draw->state], x1,
				 height - 2, x1, height);
		}

	      g_date_free (date);
	    }
	}

      if (a == 0)
	{
	  b = snprintf (buf, sizeof (buf), "30");
	  pango_layout_set_text (layout, buf, b);
	  pango_layout_get_pixel_size (layout, &pango_width, &pango_height);

	  if (pango_width < width)
	    {
	      GDate *date;

	      date =
		g_date_new_dmy (k->tm_mday, k->tm_mon + 1, k->tm_year + 1900);

	      for (a = 0; a < zoom; a++)
		{
		  if (a)
		    g_date_add_days (date, 1);

		  b =
		    snprintf (buf, sizeof (buf), "%d", g_date_get_day (date));

		  pango_layout_set_text (layout, buf, b);
		  pango_layout_get_pixel_size (layout, &pango_width,
					       &pango_height);
		  x1 =
		    ((a * width) + (DRAW_MAX / 2)) + ((width / 2) -
						      (pango_width / 2));
		  gtk_paint_layout (draw->style, draw->window,
				    GTK_STATE_NORMAL, TRUE, NULL, draw, NULL,
				    x1, height - pango_height, layout);
		  x1 = ((a * width) + (DRAW_MAX / 2));
		  gdk_draw_line (draw->window,
				 draw->style->dark_gc[draw->state], x1,
				 height - pango_height - 2, x1,
				 height - pango_height);
		  gdk_draw_line (draw->window,
				 draw->style->dark_gc[draw->state], x1,
				 height - 2, x1, height);
		}

	      g_date_free (date);
	    }
	}
    }

  /* HSEPARATOR */
  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], 0,
		 DRAW_PIXEL - 2, draw->allocation.width - 2, DRAW_PIXEL - 2);
  gdk_draw_point (draw->window, draw->style->light_gc[draw->state], 0,
		  DRAW_PIXEL - 2);
  gdk_draw_point (draw->window, draw->style->light_gc[draw->state],
		  draw->allocation.width - 3, DRAW_PIXEL - 2 + 1);
  gdk_draw_line (draw->window, draw->style->light_gc[draw->state], 0,
		 DRAW_PIXEL - 1, draw->allocation.width - 2, DRAW_PIXEL - 1);

  height = draw->allocation.height - pango_height - 5;
  gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], 0, height,
		 draw->allocation.width, height);
  gdk_draw_point (draw->window, draw->style->light_gc[draw->state], 0,
		  height);
  gdk_draw_point (draw->window, draw->style->light_gc[draw->state],
		  draw->allocation.width - 1, height + 1);
  gdk_draw_line (draw->window, draw->style->light_gc[draw->state], 0,
		 height + 1, draw->allocation.width, height + 1);

  if (now == TRUE)
    {
      if (stime)
	k = localtime (&stime);
      else
	k = get_time ();

      /* NOW LINE */
      a = k->tm_min + (k->tm_hour * 60);

      x1 = ((a * width / 1440) + (DRAW_MAX / 2)) - (TIME_SIZE / 2);
      b = height + 1;

      gc = gdk_gc_new (draw->window);
      gdk_color_parse ("#C20101", &color);
      gdk_gc_set_rgb_fg_color (gc, &color);

      g_object_set_data (G_OBJECT (draw), "x", (void *) x1);
      g_object_set_data (G_OBJECT (draw), "y", (void *) b);

      gdk_draw_rectangle (draw->window, gc, TRUE, x1, b, TIME_SIZE,
			  TIME_SIZE);
      gdk_draw_line (draw->window, draw->style->black_gc, x1, b + TIME_SIZE,
		     x1 + (TIME_SIZE - 1), b + TIME_SIZE);
      gdk_draw_line (draw->window, draw->style->black_gc,
		     x1 + (TIME_SIZE - 1), b, x1 + (TIME_SIZE - 1),
		     b + (TIME_SIZE - 1));
      gdk_draw_line (draw->window, draw->style->dark_gc[draw->state], x1 + 1,
		     b + (TIME_SIZE - 1), x1 + (TIME_SIZE - 3),
		     b + (TIME_SIZE - 1));
      gdk_draw_line (draw->window, draw->style->dark_gc[draw->state],
		     x1 + (TIME_SIZE - 2), b, x1 + (TIME_SIZE - 2),
		     b + (TIME_SIZE - 1));
      gdk_draw_line (draw->window, draw->style->light_gc[draw->state], x1, b,
		     x1 + (TIME_SIZE - 2), b);
      gdk_draw_line (draw->window, draw->style->light_gc[draw->state], x1, b,
		     x1, b + TIME_SIZE);

      g_object_unref (gc);
    }

  if (done == TRUE)
    gtk_widget_queue_draw (draw);

  g_object_unref (layout);
}

int
draw_get_time_x (GtkWidget * w)
{
  void *data;
  GtkWidget *draw;

  draw = g_object_get_data (G_OBJECT (w), "draw");
  data = g_object_get_data (G_OBJECT (draw), "x");

  return (int) data;
}

int
draw_get_time_y (GtkWidget * w)
{
  void *data;
  GtkWidget *draw;

  draw = g_object_get_data (G_OBJECT (w), "draw");
  data = g_object_get_data (G_OBJECT (draw), "y");

  return (int) data;
}

static void
draw_softstop (GtkWidget * draw, GdkColor * col, int x1, int x2, int h, int p)
{
  int a, b;
  int red, blue, green;
  GdkColor color;
  GdkGC *gc;

  gc = gdk_gc_new (draw->window);
  memcpy (&color, col, sizeof (GdkColor));

  x1++;

  if ((x2 - x1) > DRAW_SS)
    a = x2 - DRAW_SS;
  else
    a = x1;

  b = x2 - 2;

  if (b == a)
    b += 2;

  red = (0xffff - color.red) / (b - a);
  blue = (0xffff - color.blue) / (b - a);
  green = (0xffff - color.green) / (b - a);

  for (; a < b; a++)
    {
      color.red += red;
      color.blue += blue;
      color.green += green;

      gdk_gc_set_rgb_fg_color (gc, &color);

      gdk_draw_line (draw->window, gc, a, DRAW_PIXEL + 1 + p, a,
		     DRAW_PIXEL + (h - 2) + p);
    }

  g_object_unref (gc);
}

GList *
draw_get_xy (GtkWidget * widget, int x, int y, struct somad_data *data)
{
  PangoLayout *layout;
  struct somad_data *pl;
  GtkWidget *draw;
  int width, height, zoom, id;
  int pango_height, pango_width;
  int a, b, x1, x2, w;
  struct tm *k;
  struct tm check;
  time_t stime;
  int priority;
  GList *list = NULL;

  draw = g_object_get_data (G_OBJECT (widget), "draw");
  layout = gtk_widget_create_pango_layout (draw, "1");
  pango_layout_get_pixel_size (layout, &pango_width, &pango_height);
  g_object_unref (layout);

  width = draw->allocation.width - DRAW_MAX;
  height = pango_height + 12;
  zoom = (int) g_object_get_data (G_OBJECT (draw), "zoom");

  if (y < DRAW_PIXEL || y > draw->allocation.height)
    return NULL;

  priority = y < height;
  width /= zoom;

  stime = (time_t) g_object_get_data (G_OBJECT (draw), "time");

  for (id = 0; id < zoom; id++)
    {

      if (stime)
	k = localtime (&stime);
      else
	k = get_time ();

      if (id)
	{
	  GDate *date;

	  date =
	    g_date_new_dmy (k->tm_mday, k->tm_mon + 1, k->tm_year + 1900);
	  g_date_add_days (date, id);
	  k = g_malloc (sizeof (struct tm));
	  g_date_to_struct_tm (date, k);
	}

      w = width * (id);

      pl = data;

      while (pl)
	{
	  if (pl->priority == priority && !timer_check_day
	      (pl->timer, k->tm_year + 1900, k->tm_mon, k->tm_mday,
	       k->tm_wday))
	    {
	      a = pl->timer->start_min + (pl->timer->start_hour * 60);
	      b = pl->timer->stop_min + (pl->timer->stop_hour * 60);

	      if (a == 1440)
		a = 0;
	      if (b == 0)
		b = 1440;

	      /* A < B */
	      if (a <= b)
		{
		  x1 = w + ((a * width / 1440) + (DRAW_MAX / 2));
		  x2 = w + ((b * width / 1440) + (DRAW_MAX / 2));

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = pl->timer->start_min;
		  check.tm_hour = pl->timer->start_hour;

		  if (check.tm_min)
		    check.tm_min--;

		  else if (check.tm_hour)
		    {
		      check.tm_hour--;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mday)
		    {
		      check.tm_mday--;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mon)
		    {
		      check.tm_mon--;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else
		    {
		      check.tm_year--;
		      check.tm_mon = 11;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  if (timer_check (pl->timer, &check) < 1)
		    x1 = w + (DRAW_MAX / 2);

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = pl->timer->stop_min;
		  check.tm_hour = pl->timer->stop_hour;

		  if (pl->timer->stop_min != 59)
		    check.tm_min = pl->timer->stop_min + 1;

		  else if (pl->timer->stop_hour != 23)
		    {
		      check.tm_hour = pl->timer->stop_hour + 1;
		      check.tm_min = 0;
		    }

		  else if (pl->timer->stop_mday != 30)
		    {
		      check.tm_mday = pl->timer->stop_mday + 1;
		      check.tm_hour = 0;
		      check.tm_min = 0;
		    }

		  else if (pl->timer->stop_month != 11)
		    {
		      check.tm_mon = pl->timer->stop_month + 1;
		      check.tm_mday = 0;
		      check.tm_hour = 0;
		      check.tm_min = 0;
		    }

		  else
		    {
		      check.tm_year = (pl->timer->stop_year + 1) - 1900;
		      check.tm_mon = 0;
		      check.tm_mday = 0;
		      check.tm_hour = 0;
		      check.tm_min = 0;
		    }

		  if (timer_check (pl->timer, &check) < 1)
		    x2 = (width * (id + 1)) + (DRAW_MAX / 2);

		  if (x > x1 && x < x2)
		    list = g_list_append (list, pl);
		}

	      /* A > B */
	      else
		{
		  x1 = w + ((a * width / 1440) + (DRAW_MAX / 2));
		  x2 = w + ((b * width / 1440) + (DRAW_MAX / 2));

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = pl->timer->start_min;
		  check.tm_hour = pl->timer->start_hour;

		  if (check.tm_min)
		    check.tm_min--;

		  else if (check.tm_hour)
		    {
		      check.tm_hour--;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mday)
		    {
		      check.tm_mday--;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mon)
		    {
		      check.tm_mon--;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else
		    {
		      check.tm_year--;
		      check.tm_mon = 11;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  if (timer_check (pl->timer, &check) < 1)
		    x2 = x1;

		  if (((width * (id + 1) + (DRAW_MAX / 2)) - x1) > 2 &&
		      x > x1 && x < (width * (id + 1) + (DRAW_MAX / 2)))
		    list = g_list_append (list, pl);

		  else if ((x2 - (w + (DRAW_MAX / 2))) > 2 &&
			   x > (w + (DRAW_MAX / 2)) && x < x2)
		    list = g_list_append (list, pl);

		}
	    }

	  pl = pl->next;
	}
    }

  return list;
}

static GtkWidget *
draw_menu (GtkWidget * draw)
{
  GtkWidget *menu;
  GtkWidget *item;

  menu = gtk_menu_new ();

  item = gtk_image_menu_item_new_from_stock ("gtk-zoom-out", NULL);
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  g_object_set_data (G_OBJECT (item), "draw", draw);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (draw_zoom), (gpointer) 0);

  item = gtk_image_menu_item_new_from_stock ("gtk-zoom-in", NULL);
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  g_object_set_data (G_OBJECT (item), "draw", draw);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (draw_zoom), (gpointer) - 1);

  item = gtk_menu_item_new ();
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  gtk_widget_set_sensitive (item, FALSE);

  item = gtk_menu_item_new_with_mnemonic (_("1 day"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  g_object_set_data (G_OBJECT (item), "draw", draw);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (draw_zoom), (gpointer) 1);

  item = gtk_menu_item_new_with_mnemonic (_("2 days"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  g_object_set_data (G_OBJECT (item), "draw", draw);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (draw_zoom), (gpointer) 2);

  item = gtk_menu_item_new_with_mnemonic (_("1 week"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  g_object_set_data (G_OBJECT (item), "draw", draw);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (draw_zoom), (gpointer) 7);

  item = gtk_menu_item_new_with_mnemonic (_("2 weeks"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  g_object_set_data (G_OBJECT (item), "draw", draw);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (draw_zoom), (gpointer) 14);

  item = gtk_menu_item_new_with_mnemonic (_("1 month"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  g_object_set_data (G_OBJECT (item), "draw", draw);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (draw_zoom), (gpointer) 30);

  item = gtk_menu_item_new_with_mnemonic (_("2 months"));
  gtk_widget_show (item);
  gtk_container_add (GTK_CONTAINER (menu), item);
  g_object_set_data (G_OBJECT (item), "draw", draw);

  g_signal_connect ((gpointer) item, "activate",
		    G_CALLBACK (draw_zoom), (gpointer) 60);

  return menu;
}

static gint
draw_press (GtkWidget * draw, GdkEventButton * event, gpointer bf)
{
  gint (*button_func) (GtkWidget *, GdkEventButton *, gpointer);
  void *data;

  if (event->button == 3)
    {
      void *data = (void *) g_object_get_data (G_OBJECT (draw), "menu");

      gtk_menu_popup (GTK_MENU (data), NULL, NULL, NULL, NULL, event->button,
		      event->time);

      return TRUE;
    }

  if (draw_resizable_pl)
    draw_resizing = TRUE;

  data = (void *) g_object_get_data (G_OBJECT (draw), "button_data");
  button_func = bf;

  return button_func (draw, event, data);
}

static gint
draw_release (GtkWidget * draw, GdkEventButton * event, gpointer bf)
{
  gint (*button_func) (GtkWidget *, GdkEventButton *, gpointer);
  void *data;

  if (draw_resizing == TRUE)
    {
      time_t t;
      gboolean now;
      struct somad_data **expose_data;
      gboolean (*func) (GtkWidget *, gpointer);

      if (draw_resizing_start != -1)
	{
	  draw_resizable_pl->timer->start_min = draw_resizing_start % 60;
	  draw_resizable_pl->timer->start_hour = draw_resizing_start / 60;
	}

      if (draw_resizing_stop != -1)
	{
	  draw_resizable_pl->timer->stop_min = draw_resizing_stop % 60;
	  draw_resizable_pl->timer->stop_hour = draw_resizing_stop / 60;
	}

      draw_resizing = FALSE;
      draw_tooltip_hide (draw);

      t = (time_t) g_object_get_data (G_OBJECT (draw), "time");
      now = (gboolean) g_object_get_data (G_OBJECT (draw), "now");

      expose_data =
	(struct somad_data **) g_object_get_data (G_OBJECT (draw),
						  "expose_data");
      draw_refresh_true (draw, *expose_data, FALSE, t, now);

      data = (void *) g_object_get_data (G_OBJECT (draw), "resize_data");
      func = (void *) g_object_get_data (G_OBJECT (draw), "resize_func");

      if (func)
	return func (draw, data);

      return TRUE;
    }

  data = (void *) g_object_get_data (G_OBJECT (draw), "button_data");
  button_func = bf;

  return button_func (draw, event, data);
}

static gboolean
draw_motion (GtkWidget * draw, GdkEventMotion * event)
{
  int x, y;
  GdkModifierType state;

  PangoLayout *layout;
  struct somad_data *pl, *resizable_pl = NULL;
  gboolean resizing_right = FALSE;
  gint resizing_id = 0;
  int width, height, zoom, id;
  int pango_height, pango_width;
  int a, b, x1, x2, w;
  struct tm *k;
  struct tm check;
  time_t stime;
  int priority;
  int span;

  GdkCursorType cursor = GDK_X_CURSOR;
  GdkCursor *cursor_widget;

  struct somad_data **data =
    (struct somad_data **) g_object_get_data (G_OBJECT (draw), "expose_data");

  gdk_window_get_pointer (event->window, &x, &y, &state);

  layout = gtk_widget_create_pango_layout (draw, "1");
  pango_layout_get_pixel_size (layout, &pango_width, &pango_height);
  g_object_unref (layout);

  width = draw->allocation.width - DRAW_MAX;
  height = pango_height + 12;
  zoom = (int) g_object_get_data (G_OBJECT (draw), "zoom");

  if (y < DRAW_PIXEL || y > draw->allocation.height)
    return TRUE;

  /* SET THE CURSOR: */
  priority = y < height;
  width /= zoom;

  stime = (time_t) g_object_get_data (G_OBJECT (draw), "time");

  /* RESIZING: */
  if (draw_resizable_pl && draw_resizing)
    {
      time_t t;
      gboolean now;

      if (stime)
	k = localtime (&stime);
      else
	k = get_time ();

      if (draw_resizing_id)
	{
	  GDate *date;

	  date =
	    g_date_new_dmy (k->tm_mday, k->tm_mon + 1, k->tm_year + 1900);
	  g_date_add_days (date, draw_resizing_id);
	  k = g_malloc (sizeof (struct tm));
	  g_date_to_struct_tm (date, k);
	}

      w = width * draw_resizing_id;
      pl = draw_resizable_pl;

      a = pl->timer->start_min + (pl->timer->start_hour * 60);
      b = pl->timer->stop_min + (pl->timer->stop_hour * 60);

      if (a == 1440)
	a = 0;
      if (b == 0)
	b = 1440;

      /* A < B */
      if (a <= b)
	{
	  x1 = w + ((a * width / 1440) + (DRAW_MAX / 2));
	  x2 = w + ((b * width / 1440) + (DRAW_MAX / 2));

	  memcpy (&check, k, sizeof (struct tm));
	  check.tm_min = pl->timer->start_min;
	  check.tm_hour = pl->timer->start_hour;

	  if (check.tm_min)
	    check.tm_min--;

	  else if (check.tm_hour)
	    {
	      check.tm_hour--;
	      check.tm_min = 59;
	    }

	  else if (check.tm_mday)
	    {
	      check.tm_mday--;
	      check.tm_hour = 23;
	      check.tm_min = 59;
	    }

	  else if (check.tm_mon)
	    {
	      check.tm_mon--;
	      check.tm_mday = 30;
	      check.tm_hour = 23;
	      check.tm_min = 59;
	    }

	  else
	    {
	      check.tm_year--;
	      check.tm_mon = 11;
	      check.tm_mday = 30;
	      check.tm_hour = 23;
	      check.tm_min = 59;
	    }

	  if (timer_check (pl->timer, &check) < 1)
	    x1 = w + (DRAW_MAX / 2);

	  memcpy (&check, k, sizeof (struct tm));
	  check.tm_min = pl->timer->stop_min;
	  check.tm_hour = pl->timer->stop_hour;

	  if (pl->timer->stop_min != 59)
	    check.tm_min = pl->timer->stop_min + 1;

	  else if (pl->timer->stop_hour != 23)
	    {
	      check.tm_hour = pl->timer->stop_hour + 1;
	      check.tm_min = 0;
	    }

	  else if (pl->timer->stop_mday != 30)
	    {
	      check.tm_mday = pl->timer->stop_mday + 1;
	      check.tm_hour = 0;
	      check.tm_min = 0;
	    }

	  else if (pl->timer->stop_month != 11)
	    {
	      check.tm_mon = pl->timer->stop_month + 1;
	      check.tm_mday = 0;
	      check.tm_hour = 0;
	      check.tm_min = 0;
	    }

	  else
	    {
	      check.tm_year = (pl->timer->stop_year + 1) - 1900;
	      check.tm_mon = 0;
	      check.tm_mday = 0;
	      check.tm_hour = 0;
	      check.tm_min = 0;
	    }

	  if (draw_resizing_right == FALSE)
	    {
	      int new =
		(x - (width * draw_resizing_id) -
		 DRAW_MAX / 2) * 1440 / width;
	      if (new < 0 || new > 1440)
		new = 0;

	      draw_resizing_start = new;
	    }
	  else
	    {
	      int new =
		(x - (width * draw_resizing_id) -
		 DRAW_MAX / 2) * 1440 / width;
	      if (new < 0 || new > 1440)
		new = 0;

	      draw_resizing_stop = new;
	    }
	}

      /* A > B */
      else
	{
	  x1 = w + ((a * width / 1440) + (DRAW_MAX / 2));
	  x2 = w + ((b * width / 1440) + (DRAW_MAX / 2));

	  memcpy (&check, k, sizeof (struct tm));
	  check.tm_min = pl->timer->start_min;
	  check.tm_hour = pl->timer->start_hour;

	  if (check.tm_min)
	    check.tm_min--;

	  else if (check.tm_hour)
	    {
	      check.tm_hour--;
	      check.tm_min = 59;
	    }

	  else if (check.tm_mday)
	    {
	      check.tm_mday--;
	      check.tm_hour = 23;
	      check.tm_min = 59;
	    }

	  else if (check.tm_mon)
	    {
	      check.tm_mon--;
	      check.tm_mday = 30;
	      check.tm_hour = 23;
	      check.tm_min = 59;
	    }

	  else
	    {
	      check.tm_year--;
	      check.tm_mon = 11;
	      check.tm_mday = 30;
	      check.tm_hour = 23;
	      check.tm_min = 59;
	    }

	  if (timer_check (pl->timer, &check) < 1)
	    x2 = x1;

	  if (draw_resizing_right == FALSE)
	    {
	      int new =
		(x - (width * draw_resizing_id) -
		 DRAW_MAX / 2) * 1440 / width;
	      if (new < 0 || new > 1440)
		new = 0;

	      draw_resizing_start = new;
	    }
	  else
	    {
	      int new =
		(x - (width * draw_resizing_id) -
		 DRAW_MAX / 2) * 1440 / width;
	      if (new < 0 || new > 1440)
		new = 0;

	      draw_resizing_stop = new;
	    }
	}

      t = (time_t) g_object_get_data (G_OBJECT (draw), "time");
      now = (gboolean) g_object_get_data (G_OBJECT (draw), "now");
      draw_resizing_x = x;

      draw_tooltip (draw);
      return TRUE;
    }

  for (id = 0; id < zoom; id++)
    {

      if (stime)
	k = localtime (&stime);
      else
	k = get_time ();

      if (id)
	{
	  GDate *date;

	  date =
	    g_date_new_dmy (k->tm_mday, k->tm_mon + 1, k->tm_year + 1900);
	  g_date_add_days (date, id);
	  k = g_malloc (sizeof (struct tm));
	  g_date_to_struct_tm (date, k);
	}

      w = width * (id);

      pl = *data;

      while (pl)
	{
	  if (pl->priority == priority && !timer_check_day
	      (pl->timer, k->tm_year + 1900, k->tm_mon, k->tm_mday,
	       k->tm_wday))
	    {
	      a = pl->timer->start_min + (pl->timer->start_hour * 60);
	      b = pl->timer->stop_min + (pl->timer->stop_hour * 60);

	      if (a == 1440)
		a = 0;
	      if (b == 0)
		b = 1440;

	      /* A < B */
	      if (a <= b || !b)
		{
		  x1 = w + ((a * width / 1440) + (DRAW_MAX / 2));

		  if (!b)
		    x2 = w + (width + (DRAW_MAX / 2));
		  else
		    x2 = w + ((b * width / 1440) + (DRAW_MAX / 2));

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = pl->timer->start_min;
		  check.tm_hour = pl->timer->start_hour;

		  if (check.tm_min)
		    check.tm_min--;

		  else if (check.tm_hour)
		    {
		      check.tm_hour--;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mday)
		    {
		      check.tm_mday--;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mon)
		    {
		      check.tm_mon--;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else
		    {
		      check.tm_year--;
		      check.tm_mon = 11;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  if (timer_check (pl->timer, &check) < 1)
		    x1 = w + (DRAW_MAX / 2);

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = pl->timer->stop_min;
		  check.tm_hour = pl->timer->stop_hour;

		  if (pl->timer->stop_min != 59)
		    check.tm_min = pl->timer->stop_min + 1;

		  else if (pl->timer->stop_hour != 23)
		    {
		      check.tm_hour = pl->timer->stop_hour + 1;
		      check.tm_min = 0;
		    }

		  else if (pl->timer->stop_mday != 30)
		    {
		      check.tm_mday = pl->timer->stop_mday + 1;
		      check.tm_hour = 0;
		      check.tm_min = 0;
		    }

		  else if (pl->timer->stop_month != 11)
		    {
		      check.tm_mon = pl->timer->stop_month + 1;
		      check.tm_mday = 0;
		      check.tm_hour = 0;
		      check.tm_min = 0;
		    }

		  else
		    {
		      check.tm_year = (pl->timer->stop_year + 1) - 1900;
		      check.tm_mon = 0;
		      check.tm_mday = 0;
		      check.tm_hour = 0;
		      check.tm_min = 0;
		    }

		  if (timer_check (pl->timer, &check) < 1)
		    x2 = (width * (id + 1)) + (DRAW_MAX / 2);

		  span = x2 - x1 >= DRAW_SPAN ? DRAW_SPAN : x2 - x1;

		  if (x > x1 && x < x1 + span)
		    {
		      cursor = GDK_LEFT_SIDE;
		      resizable_pl = pl;
		      resizing_right = FALSE;
		      resizing_id = id;
		    }

		  else if (x2 > x && x2 < x + span)
		    {
		      cursor = GDK_RIGHT_SIDE;
		      resizable_pl = pl;
		      resizing_right = TRUE;
		      resizing_id = id;
		    }
		}

	      /* A > B */
	      else
		{
		  x1 = w + ((a * width / 1440) + (DRAW_MAX / 2));
		  x2 = w + ((b * width / 1440) + (DRAW_MAX / 2));

		  memcpy (&check, k, sizeof (struct tm));
		  check.tm_min = pl->timer->start_min;
		  check.tm_hour = pl->timer->start_hour;

		  if (check.tm_min)
		    check.tm_min--;

		  else if (check.tm_hour)
		    {
		      check.tm_hour--;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mday)
		    {
		      check.tm_mday--;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else if (check.tm_mon)
		    {
		      check.tm_mon--;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  else
		    {
		      check.tm_year--;
		      check.tm_mon = 11;
		      check.tm_mday = 30;
		      check.tm_hour = 23;
		      check.tm_min = 59;
		    }

		  if (timer_check (pl->timer, &check) < 1)
		    x2 = x1;

		  if (((width * (id + 1) + (DRAW_MAX / 2)) - x1) > 2
		      && x > x1
		      && x <
		      x1 + (((width * (id + 1) + (DRAW_MAX / 2)) - x1) >=
			    DRAW_SPAN ? DRAW_SPAN
			    : ((width * (id + 1) + (DRAW_MAX / 2)) - x1))
		      && x < (width * (id + 1) + (DRAW_MAX / 2)))
		    {
		      cursor = GDK_LEFT_SIDE;
		      resizing_right = FALSE;
		      resizable_pl = pl;
		      resizing_id = id;
		    }

		  else if ((x2 - (w + (DRAW_MAX / 2))) > 2
			   && x > (w + (DRAW_MAX / 2)) && x < x2
			   && x > x2 - ((x2 - (w + (DRAW_MAX / 2))) >=
					DRAW_SPAN ? DRAW_SPAN : (x2 -
								 (w +
								  (DRAW_MAX
								   / 2)))))
		    {
		      cursor = GDK_RIGHT_SIDE;
		      resizing_right = TRUE;
		      resizable_pl = pl;
		      resizing_id = id;
		    }
		}
	    }

	  pl = pl->next;
	}
    }

  if (cursor != GDK_X_CURSOR)
    {
      cursor_widget = gdk_cursor_new (cursor);
      gdk_window_set_cursor (draw->window, cursor_widget);
      gdk_cursor_unref (cursor_widget);
      draw_resizable_pl = resizable_pl;
      draw_resizing = FALSE;
      draw_resizing_right = resizing_right;
      draw_resizing_id = resizing_id;
      draw_resizing_start = draw_resizing_stop = -1;
    }
  else
    {
      gdk_window_set_cursor (draw->window, NULL);
      draw_resizable_pl = NULL;
      draw_resizing = FALSE;
      draw_resizing_start = draw_resizing_stop = -1;
    }

  draw_tooltip_hide (draw);
  return TRUE;
}

static void
draw_tooltip (GtkWidget * draw)
{
  GdkScreen *screen;
  gint x, y, dx, dy;
  gboolean exist = FALSE;
  GtkWidget *win;
  GtkWidget *label;
  char buf[1024];

  if (draw_resizing_start != -1)
    snprintf (buf, sizeof (buf), "%d:%.2d", draw_resizing_start / 60,
	      draw_resizing_start % 60);
  else
    snprintf (buf, sizeof (buf), "%d:%.2d", draw_resizing_stop / 60,
	      draw_resizing_stop % 60);

  if ((win = g_object_get_data (G_OBJECT (draw), "tooltip")))
    {
      GList *list = gtk_container_get_children (GTK_CONTAINER (win));
      gtk_label_set_text (GTK_LABEL (list->data), buf);
      g_list_free (list);
      exist = TRUE;
    }
  else
    {
      win = gtk_window_new (GTK_WINDOW_POPUP);
      gtk_widget_set_app_paintable (win, TRUE);
      gtk_window_set_resizable (GTK_WINDOW (win), FALSE);
      gtk_widget_set_name (win, "somax-draw-tooltips");
      gtk_container_set_border_width (GTK_CONTAINER (win), 4);

      g_signal_connect (win, "expose_event",
			G_CALLBACK (draw_tooltip_expose), win);

      label = gtk_label_new (buf);
      gtk_widget_show (label);
      gtk_container_add (GTK_CONTAINER (win), label);

      gtk_widget_realize (win);
    }

  screen = gtk_widget_get_screen (draw);
  gdk_window_get_origin (draw->window, &dx, &dy);

  gdk_display_get_pointer (gdk_screen_get_display (screen), NULL, &x, &y,
			   NULL);

  if (draw_resizing_right == FALSE)
    gtk_window_move (GTK_WINDOW (win), x - win->allocation.width - 1,
		     dy + draw->allocation.height - win->allocation.height +
		     2);
  else
    gtk_window_move (GTK_WINDOW (win), x + 1,
		     dy + draw->allocation.height - win->allocation.height +
		     2);

  if (exist == FALSE)
    {
      gtk_widget_show (win);
      g_object_set_data (G_OBJECT (draw), "tooltip", win);
    }
}

static void
draw_tooltip_hide (GtkWidget * draw)
{
  GtkWidget *win;

  if ((win = g_object_get_data (G_OBJECT (draw), "tooltip")))
    {
      g_object_steal_data (G_OBJECT (draw), "tooltip");
      gtk_widget_destroy (win);
    }
}

/* callback for the tooltip showed */
static gint
draw_tooltip_expose (GtkWidget * win)
{
  GtkRequisition req;

  if (!win)
    return TRUE;

  gtk_widget_size_request (win, &req);
  gtk_paint_flat_box (win->style, win->window, GTK_STATE_NORMAL,
		      GTK_SHADOW_OUT, NULL, GTK_WIDGET (win), "tooltip", 0, 0,
		      req.width, req.height);

  return FALSE;
}

/* EOF */
