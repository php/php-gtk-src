/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __GTK_SQPANE_H__
#define __GTK_SQPANE_H__


#include <gdk/gdk.h>
#include <gtk/gtkcontainer.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TYPE_SQPANE                  (gtk_sqpane_get_type ())
#define GTK_SQPANE(obj)                  (GTK_CHECK_CAST ((obj), GTK_TYPE_SQPANE, GtkSQPane))
#define GTK_SQPANE_CLASS(klass)          (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SQPANE, GtkSQPaneClass))
#define GTK_IS_SQPANE(obj)               (GTK_CHECK_TYPE ((obj), GTK_TYPE_SQPANE))
#define GTK_IS_SQPANE_CLASS(klass)       (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SQPANE))


typedef struct _GtkSQPane       GtkSQPane;
typedef struct _GtkSQPaneClass  GtkSQPaneClass;

struct _GtkSQPane
{
  GtkContainer container;
  
  GtkWidget *child1;
  GtkWidget *child2;
  GtkWidget *child3;
  GtkWidget *child4;
  
  GdkWindow *vhandle;
  GdkWindow *hhandle;
  GdkWindow *chandle;
  GdkGC *xor_gc;

  /*< public >*/
  guint16 handle_size;

  /*< private >*/
  gint child1_width;
  gint child1_height;
  gint last_width;
  gint last_height;
  gint min_x;
  gint max_x;
  gint min_y;
  gint max_y;
  
  guint position_set : 1;
  guint in_drag;
  
  gint16 handle_xpos;
  gint16 handle_ypos;
};

struct _GtkSQPaneClass
{
  GtkContainerClass parent_class;
};


GtkType gtk_sqpane_get_type        (void);
GtkWidget* gtk_sqpane_new      (void);
void    gtk_sqpane_add1            (GtkSQPane  *sqpane,
				   GtkWidget *child);
void    gtk_sqpane_add2            (GtkSQPane  *sqpane,
				   GtkWidget *child);
void    gtk_sqpane_add3            (GtkSQPane  *sqpane,
				   GtkWidget *child);
void    gtk_sqpane_add4            (GtkSQPane  *sqpane,
				   GtkWidget *child);
void    gtk_sqpane_pack1           (GtkSQPane  *sqpane,
				   GtkWidget *child,
				   gboolean   resize,
				   gboolean   shrink);
void    gtk_sqpane_pack2           (GtkSQPane  *sqpane,
				   GtkWidget *child,
				   gboolean   resize,
				   gboolean   shrink);
void    gtk_sqpane_pack3           (GtkSQPane  *sqpane,
				   GtkWidget *child,
				   gboolean   resize,
				   gboolean   shrink);
void    gtk_sqpane_pack4           (GtkSQPane  *sqpane,
				   GtkWidget *child,
				   gboolean   resize,
				   gboolean   shrink);
void    gtk_sqpane_set_position    (GtkSQPane  *sqpane,
				   gint       x,
				   gint       y);
void    gtk_sqpane_set_handle_size (GtkSQPane *sqpane,
				   guint   size);

/* Internal function */
void    gtk_sqpane_compute_position (GtkSQPane *sqpane,
				    gint      width,
				    gint      height);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_SQPANE_H__ */
