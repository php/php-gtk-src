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

#include "gtkspaned.h"

enum {
  ARG_0,
  ARG_HANDLE_SIZE,
  ARG_FIXED,
};


static void gtk_spaned_class_init (GtkSPanedClass    *klass);
static void gtk_spaned_init       (GtkSPaned         *spaned);
static void gtk_spaned_set_arg	 (GtkObject        *object,
				  GtkArg           *arg,
				  guint             arg_id);
static void gtk_spaned_get_arg	 (GtkObject        *object,
				  GtkArg           *arg,
				  guint             arg_id);
static void gtk_spaned_map        (GtkWidget      *widget);
static void gtk_spaned_unmap      (GtkWidget      *widget);
static void gtk_spaned_unrealize  (GtkWidget *widget);
static gint gtk_spaned_expose     (GtkWidget      *widget,
				  GdkEventExpose *event);
static void gtk_spaned_add        (GtkContainer   *container,
				  GtkWidget      *widget);
static void gtk_spaned_remove     (GtkContainer   *container,
				  GtkWidget      *widget);
static void gtk_spaned_forall     (GtkContainer   *container,
			   	  gboolean	  include_internals,
			          GtkCallback     callback,
			          gpointer        callback_data);
static GtkType gtk_spaned_child_type (GtkContainer *container);


static GtkContainerClass *parent_class = NULL;


GtkType
gtk_spaned_get_type (void)
{
  static GtkType spaned_type = 0;
  
  if (!spaned_type)
    {
      static const GtkTypeInfo spaned_info =
      {
	"GtkSPaned",
	sizeof (GtkSPaned),
	sizeof (GtkSPanedClass),
	(GtkClassInitFunc) gtk_spaned_class_init,
	(GtkObjectInitFunc) gtk_spaned_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
      
      spaned_type = gtk_type_unique (GTK_TYPE_CONTAINER, &spaned_info);
    }
  
  return spaned_type;
}

static void
gtk_spaned_class_init (GtkSPanedClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;
  
  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;
  container_class = (GtkContainerClass*) class;
  
  parent_class = gtk_type_class (GTK_TYPE_CONTAINER);
  
  object_class->set_arg = gtk_spaned_set_arg;
  object_class->get_arg = gtk_spaned_get_arg;

  widget_class->map = gtk_spaned_map;
  widget_class->unmap = gtk_spaned_unmap;
  widget_class->unrealize = gtk_spaned_unrealize;
  widget_class->expose_event = gtk_spaned_expose;
 
  container_class->add = gtk_spaned_add;
  container_class->remove = gtk_spaned_remove;
  container_class->forall = gtk_spaned_forall;
  container_class->child_type = gtk_spaned_child_type;

  gtk_object_add_arg_type ("GtkSPaned::handle_size", GTK_TYPE_UINT,
			   GTK_ARG_READWRITE, ARG_HANDLE_SIZE);
  gtk_object_add_arg_type ("GtkSPaned::fixed", GTK_TYPE_BOOL,
			   GTK_ARG_READWRITE, ARG_FIXED);
}

static GtkType
gtk_spaned_child_type (GtkContainer *container)
{
  if (!GTK_SPANED (container)->child1 || !GTK_SPANED (container)->child2)
    return GTK_TYPE_WIDGET;
  else
    return GTK_TYPE_NONE;
}

static void
gtk_spaned_init (GtkSPaned *spaned)
{
  GTK_WIDGET_UNSET_FLAGS (spaned, GTK_NO_WINDOW);
  
  spaned->child1 = NULL;
  spaned->child2 = NULL;
  spaned->handle = NULL;
  spaned->xor_gc = NULL;
  
  spaned->handle_size = 6;
  spaned->fixed = TRUE;
  spaned->position_set = FALSE;
  spaned->last_allocation = -1;
  spaned->in_drag = FALSE;
  
  spaned->handle_xpos = -1;
  spaned->handle_ypos = -1;
  spaned->handle_width = 6;
  spaned->handle_height = 6;
}

static void
gtk_spaned_set_arg (GtkObject *object,
		   GtkArg    *arg,
		   guint      arg_id)
{
  GtkSPaned *spaned = GTK_SPANED (object);
  
  switch (arg_id)
    {
    case ARG_HANDLE_SIZE:
      gtk_spaned_set_handle_size (spaned, GTK_VALUE_UINT (*arg));
      break;
    case ARG_FIXED:
      gtk_spaned_set_fixed (spaned, GTK_VALUE_BOOL (*arg));
      break;
    }
}

static void
gtk_spaned_get_arg (GtkObject *object,
		   GtkArg    *arg,
		   guint      arg_id)
{
  GtkSPaned *spaned = GTK_SPANED (object);
  
  switch (arg_id)
    {
    case ARG_HANDLE_SIZE:
      GTK_VALUE_UINT (*arg) = spaned->handle_size;
      break;
    case ARG_FIXED:
      GTK_VALUE_BOOL (*arg) = spaned->fixed;
      break;
    default:
      arg->type = GTK_TYPE_INVALID;
      break;
    }
}

static void
gtk_spaned_map (GtkWidget *widget)
{
  GtkSPaned *spaned;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SPANED (widget));
  
  GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);
  spaned = GTK_SPANED (widget);
  
  if (spaned->child1 &&
      GTK_WIDGET_VISIBLE (spaned->child1) &&
      !GTK_WIDGET_MAPPED (spaned->child1))
    gtk_widget_map (spaned->child1);
  if (spaned->child2 &&
      GTK_WIDGET_VISIBLE (spaned->child2) &&
      !GTK_WIDGET_MAPPED (spaned->child2))
    gtk_widget_map (spaned->child2);

  gdk_window_show (widget->window);
}

static void
gtk_spaned_unmap (GtkWidget *widget)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SPANED (widget));
  
  GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);
  
  gdk_window_hide (widget->window);
}

static void
gtk_spaned_unrealize (GtkWidget *widget)
{
  GtkSPaned *spaned;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SPANED (widget));
  
  spaned = GTK_SPANED (widget);
  
  if (spaned->xor_gc)
    {
      gdk_gc_destroy (spaned->xor_gc);
      spaned->xor_gc = NULL;
    }
  
  if (spaned->handle)
    {
      gdk_window_set_user_data (spaned->handle, NULL);
      gdk_window_destroy (spaned->handle);
      spaned->handle = NULL;
    }
  
  if (GTK_WIDGET_CLASS (parent_class)->unrealize)
    (* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static gint
gtk_spaned_expose (GtkWidget      *widget,
		  GdkEventExpose *event)
{
  GtkSPaned *spaned;
  GdkEventExpose child_event;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_SPANED (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
  if (GTK_WIDGET_DRAWABLE (widget))
    {
      spaned = GTK_SPANED (widget);
      
      /* An expose event for the handle */
      if (event->window == spaned->handle)
	{
	   /*gtk_paint_box (widget->style, spaned->handle,
			  GTK_WIDGET_STATE(widget),
			  GTK_SHADOW_OUT, 
			  &event->area, widget, "spaned",
			  0, 0,
			  spaned->handle_size, spaned->handle_size);*/
	}
      else
	{
	  child_event = *event;
	  if (spaned->child1 &&
	      GTK_WIDGET_NO_WINDOW (spaned->child1) &&
	      gtk_widget_intersect (spaned->child1, &event->area, &child_event.area))
	    gtk_widget_event (spaned->child1, (GdkEvent*) &child_event);
	  
	  if (spaned->child2 &&
	      GTK_WIDGET_NO_WINDOW (spaned->child2) &&
	      gtk_widget_intersect (spaned->child2, &event->area, &child_event.area))
	    gtk_widget_event (spaned->child2, (GdkEvent*) &child_event);
	  
	  /* redraw the groove if necessary */
	  /*if (gdk_rectangle_intersect (&spaned->groove_rectangle, 
				       &event->area, 
				       &child_event.area))
	    gtk_widget_draw (widget, &child_event.area);*/
	}
    }
  return FALSE;
}

void
gtk_spaned_add1 (GtkSPaned     *spaned,
		GtkWidget    *widget)
{
  gtk_spaned_pack1 (spaned, widget, TRUE, TRUE);//FALSE, TRUE);
}

void
gtk_spaned_add2 (GtkSPaned  *spaned,
		GtkWidget *widget)
{
  gtk_spaned_pack2 (spaned, widget, TRUE, TRUE);
}

void
gtk_spaned_pack1 (GtkSPaned     *spaned,
		 GtkWidget    *child,
		 gboolean      resize,
		 gboolean      shrink)
{
  g_return_if_fail (spaned != NULL);
  g_return_if_fail (GTK_IS_SPANED (spaned));
  g_return_if_fail (GTK_IS_WIDGET (child));
  
  if (!spaned->child1)
    {
      spaned->child1 = child;

      gtk_widget_set_parent (child, GTK_WIDGET (spaned));

      if (GTK_WIDGET_REALIZED (child->parent))
	gtk_widget_realize (child);

      if (GTK_WIDGET_VISIBLE (child->parent) && GTK_WIDGET_VISIBLE (child))
	{
	  if (GTK_WIDGET_MAPPED (child->parent))
	    gtk_widget_map (child);

	  gtk_widget_queue_resize (child);
	}
    }
}

void
gtk_spaned_pack2 (GtkSPaned  *spaned,
		 GtkWidget *child,
		 gboolean   resize,
		 gboolean   shrink)
{
  g_return_if_fail (spaned != NULL);
  g_return_if_fail (GTK_IS_SPANED (spaned));
  g_return_if_fail (GTK_IS_WIDGET (child));
  
  if (!spaned->child2)
    {
      spaned->child2 = child;
      
      gtk_widget_set_parent (child, GTK_WIDGET (spaned));

      if (GTK_WIDGET_REALIZED (child->parent))
	gtk_widget_realize (child);

      if (GTK_WIDGET_VISIBLE (child->parent) && GTK_WIDGET_VISIBLE (child))
	{
	  if (GTK_WIDGET_MAPPED (child->parent))
	    gtk_widget_map (child);

	  gtk_widget_queue_resize (child);
	}
    }
}

static void
gtk_spaned_add (GtkContainer *container,
	       GtkWidget    *widget)
{
  GtkSPaned *spaned;
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_SPANED (container));
  g_return_if_fail (widget != NULL);
  
  spaned = GTK_SPANED (container);
  
  if (!spaned->child1)
    gtk_spaned_add1 (GTK_SPANED (container),widget);
  else if (!spaned->child2)
    gtk_spaned_add2 (GTK_SPANED (container),widget);
}

static void
gtk_spaned_remove (GtkContainer *container,
		  GtkWidget    *widget)
{
  GtkSPaned *spaned;
  gboolean was_visible;
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_SPANED (container));
  g_return_if_fail (widget != NULL);
  
  spaned = GTK_SPANED (container);
  was_visible = GTK_WIDGET_VISIBLE (widget);
  
  if (spaned->child1 == widget)
    {
      gtk_widget_unparent (widget);
      
      spaned->child1 = NULL;
      
      if (was_visible && GTK_WIDGET_VISIBLE (container))
        gtk_widget_queue_resize (GTK_WIDGET (container));
    }
  else if (spaned->child2 == widget)
    {
      gtk_widget_unparent (widget);
      
      spaned->child2 = NULL;
      
      if (was_visible && GTK_WIDGET_VISIBLE (container))
        gtk_widget_queue_resize (GTK_WIDGET (container));
    }
}

static void
gtk_spaned_forall (GtkContainer *container,
		  gboolean	include_internals,
		  GtkCallback   callback,
		  gpointer      callback_data)
{
  GtkSPaned *spaned;
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_SPANED (container));
  g_return_if_fail (callback != NULL);
  
  spaned = GTK_SPANED (container);
  
  if (spaned->child1)
    (* callback) (spaned->child1, callback_data);
  if (spaned->child2)
    (* callback) (spaned->child2, callback_data);
}

void
gtk_spaned_set_position    (GtkSPaned  *spaned,
			   gint       position)
{
  g_return_if_fail (spaned != NULL);
  g_return_if_fail (GTK_IS_SPANED (spaned));

  if (position >= 0)
    {
      /* We don't clamp here - the assumption is that
       * if the total allocation changes at the same time
       * as the position, the position set is with reference
       * to the new total size. If only the position changes,
       * then clamping will occur in gtk_spaned_compute_position()
       */
      spaned->child1_size = position;
      spaned->position_set = TRUE;
    }
  else
    spaned->position_set = FALSE;

  gtk_widget_queue_resize (GTK_WIDGET (spaned));
}

void
gtk_spaned_set_handle_size (GtkSPaned *spaned,
			   guint   size)
{
  g_return_if_fail (spaned != NULL);
  g_return_if_fail (GTK_IS_SPANED (spaned));

  spaned->handle_size = size;
  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (spaned)))
    gtk_widget_queue_resize (GTK_WIDGET (spaned));
}

void
gtk_spaned_set_fixed (GtkSPaned *spaned,
			   gboolean   fixed)
{
  g_return_if_fail (spaned != NULL);
  g_return_if_fail (GTK_IS_SPANED (spaned));

  spaned->fixed = fixed;
}


void
gtk_spaned_compute_position (GtkSPaned *spaned,
			    gint      allocation,
			    gint      child1_req,
			    gint      child2_req)
{
  g_return_if_fail (spaned != NULL);
  g_return_if_fail (GTK_IS_SPANED (spaned));

  spaned->min_position = 0;

  spaned->max_position = allocation;

  if (!spaned->position_set)
    {
	spaned->child1_size = child1_req;//allocation /2;
    spaned->position_set = TRUE;
    }
  else
    {
    if (spaned->last_allocation>0)
        {
		if(spaned->fixed)
        spaned->child1_size = spaned->child1_size;//* ((gdouble)child1_req / (child1_req + child2_req));
		else
        spaned->child1_size = spaned->child1_size * allocation / spaned->last_allocation;//* ((gdouble)child1_req / (child1_req + child2_req));
        }
    }

  spaned->child1_size = CLAMP (spaned->child1_size,
			      spaned->min_position,
			      spaned->max_position);

  spaned->last_allocation = allocation;
  
}


