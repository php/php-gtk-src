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

#ifndef __GTK_SPANED_H__
#define __GTK_SPANED_H__


#include <gdk/gdk.h>
#include <gtk/gtkcontainer.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_SPANED                  (gtk_spaned_get_type ())
#define GTK_SPANED(obj)                  (GTK_CHECK_CAST ((obj), GTK_TYPE_SPANED, GtkSPaned))
#define GTK_SPANED_CLASS(klass)          (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SPANED, GtkSPanedClass))
#define GTK_IS_SPANED(obj)               (GTK_CHECK_TYPE ((obj), GTK_TYPE_SPANED))
#define GTK_IS_SPANED_CLASS(klass)       (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SPANED))


typedef struct _GtkSPaned       GtkSPaned;
typedef struct _GtkSPanedClass  GtkSPanedClass;

struct _GtkSPaned
{
  GtkContainer container;
  
  GtkWidget *child1;
  GtkWidget *child2;
  
  GdkWindow *handle;
  GdkGC *xor_gc;

  /*< public >*/
  guint16 handle_size;
  gboolean fixed;

  /*< private >*/
  gint child1_size;
  gint last_allocation;
  gint min_position;
  gint max_position;
  
  guint position_set : 1;
  guint in_drag : 1;
  
  gint16 handle_xpos;
  gint16 handle_ypos;
  gint16 handle_width;
  gint16 handle_height;
};

struct _GtkSPanedClass
{
  GtkContainerClass parent_class;
};


GtkType gtk_spaned_get_type        (void);
void    gtk_spaned_add1            (GtkSPaned  *shpaned,
				   GtkWidget *child);
void    gtk_spaned_add2            (GtkSPaned  *shpaned,
				   GtkWidget *child);
void    gtk_spaned_pack1           (GtkSPaned  *shpaned,
				   GtkWidget *child,
				   gboolean   resize,
				   gboolean   shrink);
void    gtk_spaned_pack2           (GtkSPaned  *shpaned,
				   GtkWidget *child,
				   gboolean   resize,
				   gboolean   shrink);
void    gtk_spaned_set_position    (GtkSPaned  *shpaned,
				   gint       position);
void    gtk_spaned_set_handle_size (GtkSPaned *shpaned,
				   guint   size);
void    gtk_spaned_set_fixed (GtkSPaned *shpaned,
				   gboolean   fixed);

/* Internal function */
void    gtk_spaned_compute_position (GtkSPaned *shpaned,
				    gint      allocation,
				    gint      child1_req,
				    gint      child2_req);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_SPANED_H__ */
