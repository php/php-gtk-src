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

#include "gtksqpane.h"

enum {
  ARG_0,
  ARG_HANDLE_SIZE,
};

static void gtk_sqpane_class_init (GtkSQPaneClass    *klass);
static void gtk_sqpane_init       (GtkSQPane         *sqpane);
static void gtk_sqpane_set_arg	 (GtkObject        *object,
				  GtkArg           *arg,
				  guint             arg_id);
static void gtk_sqpane_get_arg	 (GtkObject        *object,
				  GtkArg           *arg,
				  guint             arg_id);
static void gtk_sqpane_realize    (GtkWidget *widget);
static void gtk_sqpane_map        (GtkWidget      *widget);
static void gtk_sqpane_unmap      (GtkWidget      *widget);
static void gtk_sqpane_unrealize  (GtkWidget *widget);
static gint gtk_sqpane_expose     (GtkWidget      *widget,
				  GdkEventExpose *event);
static void gtk_sqpane_add        (GtkContainer   *container,
				  GtkWidget      *widget);
static void gtk_sqpane_remove     (GtkContainer   *container,
				  GtkWidget      *widget);
static void gtk_sqpane_forall     (GtkContainer   *container,
			   	  gboolean	  include_internals,
			          GtkCallback     callback,
			          gpointer        callback_data);
static GtkType gtk_sqpane_child_type (GtkContainer *container);

static void gtk_sqpane_size_request     (GtkWidget      *widget,
					 GtkRequisition *requisition);
static void gtk_sqpane_size_allocate    (GtkWidget          *widget,
					 GtkAllocation      *allocation);
static void gtk_sqpane_draw             (GtkWidget    *widget,
					 GdkRectangle *area);
static void gtk_sqpane_hxor_line         (GtkSQPane *sqpane);
static void gtk_sqpane_vxor_line         (GtkSQPane *sqpane);
static gint gtk_sqpane_button_press     (GtkWidget *widget,
					 GdkEventButton *event);
static gint gtk_sqpane_button_release   (GtkWidget *widget,
					 GdkEventButton *event);
static gint gtk_sqpane_motion           (GtkWidget *widget,
					 GdkEventMotion *event);


static GtkContainerClass *parent_class = NULL;


GtkType
gtk_sqpane_get_type (void)
{
  static GtkType sqpane_type = 0;
  
  if (!sqpane_type)
    {
      static const GtkTypeInfo sqpane_info =
      {
	"GtkSQPane",
	sizeof (GtkSQPane),
	sizeof (GtkSQPaneClass),
	(GtkClassInitFunc) gtk_sqpane_class_init,
	(GtkObjectInitFunc) gtk_sqpane_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
      
      sqpane_type = gtk_type_unique (GTK_TYPE_CONTAINER, &sqpane_info);
    }
  
  return sqpane_type;
}

static void
gtk_sqpane_class_init (GtkSQPaneClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;
  
  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;
  container_class = (GtkContainerClass*) class;
  
  parent_class = gtk_type_class (GTK_TYPE_CONTAINER);
  
  object_class->set_arg = gtk_sqpane_set_arg;
  object_class->get_arg = gtk_sqpane_get_arg;

  widget_class->realize = gtk_sqpane_realize;
  widget_class->map = gtk_sqpane_map;
  widget_class->unmap = gtk_sqpane_unmap;
  widget_class->unrealize = gtk_sqpane_unrealize;
  widget_class->expose_event = gtk_sqpane_expose;
 
  widget_class->size_request = gtk_sqpane_size_request;
  widget_class->size_allocate = gtk_sqpane_size_allocate;
  widget_class->draw = gtk_sqpane_draw;
  widget_class->button_press_event = gtk_sqpane_button_press;
  widget_class->button_release_event = gtk_sqpane_button_release;
  widget_class->motion_notify_event = gtk_sqpane_motion;
  
  container_class->add = gtk_sqpane_add;
  container_class->remove = gtk_sqpane_remove;
  container_class->forall = gtk_sqpane_forall;
  container_class->child_type = gtk_sqpane_child_type;

  gtk_object_add_arg_type ("GtkSQPane::handle_size", GTK_TYPE_UINT,
			   GTK_ARG_READWRITE, ARG_HANDLE_SIZE);
}

static GtkType
gtk_sqpane_child_type (GtkContainer *container)
{
  if (!GTK_SQPANE (container)->child1 || !GTK_SQPANE (container)->child2)
    return GTK_TYPE_WIDGET;
  else
    return GTK_TYPE_NONE;
}

static void
gtk_sqpane_init (GtkSQPane *sqpane)
{
  GTK_WIDGET_UNSET_FLAGS (sqpane, GTK_NO_WINDOW);
  
  sqpane->child1 = NULL;
  sqpane->child2 = NULL;
  sqpane->child3 = NULL;
  sqpane->child4 = NULL;
  sqpane->vhandle = NULL;
  sqpane->hhandle = NULL;
  sqpane->chandle = NULL;
  sqpane->xor_gc = NULL;
  
  sqpane->handle_size = 6;
  sqpane->position_set = FALSE;
  sqpane->last_width = -1;
  sqpane->last_height = -1;
  sqpane->in_drag = FALSE;
  
  sqpane->handle_xpos = -1;
  sqpane->handle_ypos = -1;
}

GtkWidget*
gtk_sqpane_new (void)
{
  GtkSQPane *sqpane;

  sqpane = gtk_type_new (gtk_sqpane_get_type ());

  return GTK_WIDGET (sqpane);
}

static void
gtk_sqpane_set_arg (GtkObject *object,
		   GtkArg    *arg,
		   guint      arg_id)
{
  GtkSQPane *sqpane = GTK_SQPANE (object);
  
  switch (arg_id)
    {
    case ARG_HANDLE_SIZE:
      gtk_sqpane_set_handle_size (sqpane, GTK_VALUE_UINT (*arg));
      break;
    }
}

static void
gtk_sqpane_get_arg (GtkObject *object,
		   GtkArg    *arg,
		   guint      arg_id)
{
  GtkSQPane *sqpane = GTK_SQPANE (object);
  
  switch (arg_id)
    {
    case ARG_HANDLE_SIZE:
      GTK_VALUE_UINT (*arg) = sqpane->handle_size;
      break;
    default:
      arg->type = GTK_TYPE_INVALID;
      break;
    }
}

static void
gtk_sqpane_realize (GtkWidget *widget)
{
  GtkSQPane *sqpane;
  GdkWindowAttr attributes;
  gint attributes_mask;
  guint16 border_width;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SQPANE (widget));
  
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  sqpane = GTK_SQPANE (widget);
  
  border_width = GTK_CONTAINER (sqpane)->border_width;
  
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, sqpane);
  
  //
  attributes.x = border_width;//sqpane->handle_xpos;
  attributes.y = sqpane->handle_ypos;
  attributes.width = widget->allocation.width;
  attributes.height = sqpane->handle_size;//widget->allocation.height;
  attributes.cursor = gdk_cursor_new (GDK_SB_V_DOUBLE_ARROW);//gdk_cursor_new (GDK_CROSS);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
			    GDK_BUTTON_RELEASE_MASK |
			    GDK_POINTER_MOTION_MASK |
			    GDK_POINTER_MOTION_HINT_MASK);
  attributes_mask |= GDK_WA_CURSOR;
  
  sqpane->vhandle = gdk_window_new (widget->window,
				  &attributes, attributes_mask);
  gdk_window_set_user_data (sqpane->vhandle, sqpane);
  gdk_cursor_destroy (attributes.cursor);
  //
  attributes.x = sqpane->handle_xpos;
  attributes.y = border_width;//sqpane->handle_ypos;
  attributes.width = sqpane->handle_size;//widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.cursor = gdk_cursor_new (GDK_SB_H_DOUBLE_ARROW);//gdk_cursor_new (GDK_CROSS);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
			    GDK_BUTTON_RELEASE_MASK |
			    GDK_POINTER_MOTION_MASK |
			    GDK_POINTER_MOTION_HINT_MASK);
  attributes_mask |= GDK_WA_CURSOR;
  
  sqpane->hhandle = gdk_window_new (widget->window,
				  &attributes, attributes_mask);
  gdk_window_set_user_data (sqpane->hhandle, sqpane);
  gdk_cursor_destroy (attributes.cursor);
  //
  attributes.x = sqpane->handle_xpos;
  attributes.y = sqpane->handle_ypos;
  attributes.width = sqpane->handle_size;
  attributes.height = sqpane->handle_size;
  attributes.cursor = gdk_cursor_new (GDK_CROSS);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
			    GDK_BUTTON_RELEASE_MASK |
			    GDK_POINTER_MOTION_MASK |
			    GDK_POINTER_MOTION_HINT_MASK);
  attributes_mask |= GDK_WA_CURSOR;
  
  sqpane->chandle = gdk_window_new (widget->window,
				  &attributes, attributes_mask);
  gdk_window_set_user_data (sqpane->chandle, sqpane);
  gdk_cursor_destroy (attributes.cursor);
  //
  
  widget->style = gtk_style_attach (widget->style, widget->window);
  
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
  gtk_style_set_background (widget->style, sqpane->vhandle, GTK_STATE_NORMAL);
  gtk_style_set_background (widget->style, sqpane->hhandle, GTK_STATE_NORMAL);
  gtk_style_set_background (widget->style, sqpane->chandle, GTK_STATE_NORMAL);

  gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
  
  gdk_window_show (sqpane->vhandle);
  gdk_window_show (sqpane->hhandle);
  gdk_window_show (sqpane->chandle);
}

static void
gtk_sqpane_map (GtkWidget *widget)
{
  GtkSQPane *sqpane;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SQPANE (widget));
  
  GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);
  sqpane = GTK_SQPANE (widget);
  
  if (sqpane->child1 &&
      GTK_WIDGET_VISIBLE (sqpane->child1) &&
      !GTK_WIDGET_MAPPED (sqpane->child1))
    gtk_widget_map (sqpane->child1);
  if (sqpane->child2 &&
      GTK_WIDGET_VISIBLE (sqpane->child2) &&
      !GTK_WIDGET_MAPPED (sqpane->child2))
    gtk_widget_map (sqpane->child2);
  if (sqpane->child3 &&
      GTK_WIDGET_VISIBLE (sqpane->child3) &&
      !GTK_WIDGET_MAPPED (sqpane->child3))
    gtk_widget_map (sqpane->child3);
  if (sqpane->child4 &&
      GTK_WIDGET_VISIBLE (sqpane->child4) &&
      !GTK_WIDGET_MAPPED (sqpane->child4))
    gtk_widget_map (sqpane->child4);

  gdk_window_show (widget->window);
}

static void
gtk_sqpane_unmap (GtkWidget *widget)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SQPANE (widget));
  
  GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);
  
  gdk_window_hide (widget->window);
}

static void
gtk_sqpane_unrealize (GtkWidget *widget)
{
  GtkSQPane *sqpane;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SQPANE (widget));
  
  sqpane = GTK_SQPANE (widget);
  
  if (sqpane->xor_gc)
    {
      gdk_gc_destroy (sqpane->xor_gc);
      sqpane->xor_gc = NULL;
    }

  if (sqpane->vhandle)
    {
      gdk_window_set_user_data (sqpane->vhandle, NULL);
      gdk_window_destroy (sqpane->vhandle);
      sqpane->vhandle = NULL;
    }
  if (sqpane->hhandle)
    {
      gdk_window_set_user_data (sqpane->hhandle, NULL);
      gdk_window_destroy (sqpane->hhandle);
      sqpane->hhandle = NULL;
    }
  if (sqpane->chandle)
    {
      gdk_window_set_user_data (sqpane->chandle, NULL);
      gdk_window_destroy (sqpane->chandle);
      sqpane->chandle = NULL;
    }
  
  if (GTK_WIDGET_CLASS (parent_class)->unrealize)
    (* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static gint
gtk_sqpane_expose (GtkWidget      *widget,
		  GdkEventExpose *event)
{
  GtkSQPane *sqpane;
  GdkEventExpose child_event;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_SQPANE (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
  if (GTK_WIDGET_DRAWABLE (widget))
    {
      sqpane = GTK_SQPANE (widget);
      
      /* An expose event for the handle */
	  child_event = *event;
	  if (sqpane->child1 &&
	      GTK_WIDGET_NO_WINDOW (sqpane->child1) &&
	      gtk_widget_intersect (sqpane->child1, &event->area, &child_event.area))
	    gtk_widget_event (sqpane->child1, (GdkEvent*) &child_event);
	  
	  if (sqpane->child2 &&
	      GTK_WIDGET_NO_WINDOW (sqpane->child2) &&
	      gtk_widget_intersect (sqpane->child2, &event->area, &child_event.area))
	    gtk_widget_event (sqpane->child2, (GdkEvent*) &child_event);
	  
	  if (sqpane->child3 &&
	      GTK_WIDGET_NO_WINDOW (sqpane->child3) &&
	      gtk_widget_intersect (sqpane->child3, &event->area, &child_event.area))
	    gtk_widget_event (sqpane->child3, (GdkEvent*) &child_event);
	  
	  if (sqpane->child4 &&
	      GTK_WIDGET_NO_WINDOW (sqpane->child4) &&
	      gtk_widget_intersect (sqpane->child4, &event->area, &child_event.area))
	    gtk_widget_event (sqpane->child4, (GdkEvent*) &child_event);
    }
  return FALSE;
}

void
gtk_sqpane_add1 (GtkSQPane     *sqpane,
		GtkWidget    *widget)
{
  gtk_sqpane_pack1 (sqpane, widget, TRUE, TRUE);
}

void
gtk_sqpane_add2 (GtkSQPane  *sqpane,
		GtkWidget *widget)
{
  gtk_sqpane_pack2 (sqpane, widget, TRUE, TRUE);
}

void
gtk_sqpane_add3 (GtkSQPane  *sqpane,
		GtkWidget *widget)
{
  gtk_sqpane_pack3 (sqpane, widget, TRUE, TRUE);
}

void
gtk_sqpane_add4 (GtkSQPane  *sqpane,
		GtkWidget *widget)
{
  gtk_sqpane_pack4 (sqpane, widget, TRUE, TRUE);
}

void
gtk_sqpane_pack1 (GtkSQPane     *sqpane,
		 GtkWidget    *child,
		 gboolean      resize,
		 gboolean      shrink)
{
  g_return_if_fail (sqpane != NULL);
  g_return_if_fail (GTK_IS_SQPANE (sqpane));
  g_return_if_fail (GTK_IS_WIDGET (child));
  
  if (!sqpane->child1)
    {
      sqpane->child1 = child;

      gtk_widget_set_parent (child, GTK_WIDGET (sqpane));

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
gtk_sqpane_pack2 (GtkSQPane  *sqpane,
		 GtkWidget *child,
		 gboolean   resize,
		 gboolean   shrink)
{
  g_return_if_fail (sqpane != NULL);
  g_return_if_fail (GTK_IS_SQPANE (sqpane));
  g_return_if_fail (GTK_IS_WIDGET (child));
  
  if (!sqpane->child2)
    {
      sqpane->child2 = child;
      
      gtk_widget_set_parent (child, GTK_WIDGET (sqpane));

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
gtk_sqpane_pack3 (GtkSQPane  *sqpane,
		 GtkWidget *child,
		 gboolean   resize,
		 gboolean   shrink)
{
  g_return_if_fail (sqpane != NULL);
  g_return_if_fail (GTK_IS_SQPANE (sqpane));
  g_return_if_fail (GTK_IS_WIDGET (child));
  
  if (!sqpane->child3)
    {
      sqpane->child3 = child;
      
      gtk_widget_set_parent (child, GTK_WIDGET (sqpane));

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
gtk_sqpane_pack4 (GtkSQPane  *sqpane,
		 GtkWidget *child,
		 gboolean   resize,
		 gboolean   shrink)
{
  g_return_if_fail (sqpane != NULL);
  g_return_if_fail (GTK_IS_SQPANE (sqpane));
  g_return_if_fail (GTK_IS_WIDGET (child));
  
  if (!sqpane->child4)
    {
      sqpane->child4 = child;
      
      gtk_widget_set_parent (child, GTK_WIDGET (sqpane));

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
gtk_sqpane_add (GtkContainer *container,
	       GtkWidget    *widget)
{
  GtkSQPane *sqpane;
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_SQPANE (container));
  g_return_if_fail (widget != NULL);
  
  sqpane = GTK_SQPANE (container);
  
  if (!sqpane->child1)
    gtk_sqpane_add1 (GTK_SQPANE (container),widget);
  else if (!sqpane->child2)
    gtk_sqpane_add2 (GTK_SQPANE (container),widget);
  else if (!sqpane->child3)
    gtk_sqpane_add3 (GTK_SQPANE (container),widget);
  else if (!sqpane->child4)
    gtk_sqpane_add4 (GTK_SQPANE (container),widget);
}

static void
gtk_sqpane_remove (GtkContainer *container,
		  GtkWidget    *widget)
{
  GtkSQPane *sqpane;
  gboolean was_visible;
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_SQPANE (container));
  g_return_if_fail (widget != NULL);
  
  sqpane = GTK_SQPANE (container);
  was_visible = GTK_WIDGET_VISIBLE (widget);
  
  if (sqpane->child1 == widget)
    {
      gtk_widget_unparent (widget);
      
      sqpane->child1 = NULL;
      
      if (was_visible && GTK_WIDGET_VISIBLE (container))
        gtk_widget_queue_resize (GTK_WIDGET (container));
    }
  else if (sqpane->child2 == widget)
    {
      gtk_widget_unparent (widget);
      
      sqpane->child2 = NULL;
      
      if (was_visible && GTK_WIDGET_VISIBLE (container))
        gtk_widget_queue_resize (GTK_WIDGET (container));
    }
  else if (sqpane->child3 == widget)
    {
      gtk_widget_unparent (widget);
      
      sqpane->child3 = NULL;
      
      if (was_visible && GTK_WIDGET_VISIBLE (container))
        gtk_widget_queue_resize (GTK_WIDGET (container));
    }
  else if (sqpane->child4 == widget)
    {
      gtk_widget_unparent (widget);
      
      sqpane->child4 = NULL;
      
      if (was_visible && GTK_WIDGET_VISIBLE (container))
        gtk_widget_queue_resize (GTK_WIDGET (container));
    }
}

static void
gtk_sqpane_forall (GtkContainer *container,
		  gboolean	include_internals,
		  GtkCallback   callback,
		  gpointer      callback_data)
{
  GtkSQPane *sqpane;
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_SQPANE (container));
  g_return_if_fail (callback != NULL);
  
  sqpane = GTK_SQPANE (container);
  
  if (sqpane->child1)
    (* callback) (sqpane->child1, callback_data);
  if (sqpane->child2)
    (* callback) (sqpane->child2, callback_data);
  if (sqpane->child3)
    (* callback) (sqpane->child3, callback_data);
  if (sqpane->child4)
    (* callback) (sqpane->child4, callback_data);
}

void
gtk_sqpane_set_position    (GtkSQPane  *sqpane,
			   gint       x,
			   gint       y)
{
  g_return_if_fail (sqpane != NULL);
  g_return_if_fail (GTK_IS_SQPANE (sqpane));

  if (x >= 0 && y >= 0)
    {
      /* We don't clamp here - the assumption is that
       * if the total allocation changes at the same time
       * as the position, the position set is with reference
       * to the new total size. If only the position changes,
       * then clamping will occur in gtk_sqpane_compute_position()
       */
      sqpane->child1_width = x;
      sqpane->child1_height = y;
      sqpane->position_set = TRUE;
    }
  else
    sqpane->position_set = FALSE;

  gtk_widget_queue_resize (GTK_WIDGET (sqpane));
}

void
gtk_sqpane_set_handle_size (GtkSQPane *sqpane,
			   guint16   size)
{
  gint x,y,width,height;
  
  g_return_if_fail (sqpane != NULL);
  g_return_if_fail (GTK_IS_SQPANE (sqpane));

  if (sqpane->vhandle)
    {
      gdk_window_get_geometry (sqpane->vhandle, &x, &y, &width, &height, NULL);
      gdk_window_move_resize (sqpane->vhandle,
                              x + sqpane->handle_size / 2 - size / 2,
                              y,
                              size, height);
    }
  if (sqpane->hhandle)
    {
      gdk_window_get_geometry (sqpane->hhandle, &x, &y, &width, &height, NULL);
      gdk_window_move_resize (sqpane->hhandle,
                              x,
                              y + sqpane->handle_size / 2 - size / 2,
                              width, size);
    }
  if (sqpane->chandle)
    {
      gdk_window_get_geometry (sqpane->chandle, &x, &y, NULL, NULL, NULL);
      gdk_window_move_resize (sqpane->chandle,
                              x + sqpane->handle_size / 2 - size / 2,
                              y + sqpane->handle_size / 2 - size / 2,
                              size, size);
    }
  sqpane->handle_size = size;
}

void
gtk_sqpane_compute_position (GtkSQPane *sqpane,
			    gint      width,
			    gint      height)
{
  g_return_if_fail (GTK_IS_SQPANE (sqpane));

  sqpane->min_x = 0;
  sqpane->max_x = width;
  sqpane->min_y = 0;
  sqpane->max_y = height;

  if (!sqpane->position_set)
    {
	sqpane->child1_width = width /2;
	sqpane->child1_height = height /2;
    }
  else
    {
    if (sqpane->last_width>0)
	sqpane->child1_width = sqpane->child1_width * width / sqpane->last_width;//* ((gdouble)child1_req / (child1_req + child2_req));
    if (sqpane->last_height>0)
	sqpane->child1_height = sqpane->child1_height * height / sqpane->last_height;//* ((gdouble)child1_req / (child1_req + child2_req));
    }

  sqpane->child1_width = CLAMP (sqpane->child1_width,
			      sqpane->min_x,
			      sqpane->max_x);
  sqpane->child1_height = CLAMP (sqpane->child1_height,
			      sqpane->min_y,
			      sqpane->max_y);

  sqpane->last_width = width;
  sqpane->last_height = height;
  
}

//

static void
gtk_sqpane_size_request (GtkWidget      *widget,
			 GtkRequisition *requisition)
{
  GtkSQPane *sqpane;
  GtkRequisition child1_requisition;
  GtkRequisition child2_requisition;
  GtkRequisition child3_requisition;
  GtkRequisition child4_requisition;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SQPANE (widget));
  g_return_if_fail (requisition != NULL);

  sqpane = GTK_SQPANE (widget);
  requisition->width = 0;
  requisition->height = 0;

  if (sqpane->child1 && GTK_WIDGET_VISIBLE (sqpane->child1))
    {
      gtk_widget_size_request (sqpane->child1, &child1_requisition);
    }
  else
    {
      child1_requisition.width = 0;
      child1_requisition.height = 0;
    }

  if (sqpane->child2 && GTK_WIDGET_VISIBLE (sqpane->child2))
    {
      gtk_widget_size_request (sqpane->child2, &child2_requisition);
    }
  else
    {
      child2_requisition.width = 0;
      child2_requisition.height = 0;
    }

  if (sqpane->child3 && GTK_WIDGET_VISIBLE (sqpane->child3))
    {
      gtk_widget_size_request (sqpane->child3, &child3_requisition);
    }
  else
    {
      child3_requisition.width = 0;
      child3_requisition.height = 0;
    }

  if (sqpane->child4 && GTK_WIDGET_VISIBLE (sqpane->child4))
    {
      gtk_widget_size_request (sqpane->child4, &child4_requisition);
    }
  else
    {
      child4_requisition.width = 0;
      child4_requisition.height = 0;
    }

  requisition->width = MAX(child1_requisition.width, child3_requisition.width)
	  + MAX(child2_requisition.width, child4_requisition.width);
  requisition->height = MAX(child1_requisition.height, child2_requisition.height)
	  + MAX(child3_requisition.height, child4_requisition.height);

  requisition->width += GTK_CONTAINER (sqpane)->border_width * 2 + sqpane->handle_size;
  requisition->height += GTK_CONTAINER (sqpane)->border_width * 2 + sqpane->handle_size;
}

static void
gtk_sqpane_size_allocate (GtkWidget     *widget,
			  GtkAllocation *allocation)
{
  GtkSQPane *sqpane;
  GtkRequisition child1_requisition;
  GtkRequisition child2_requisition;
  GtkRequisition child3_requisition;
  GtkRequisition child4_requisition;
  GtkAllocation child1_allocation;
  GtkAllocation child2_allocation;
  GtkAllocation child3_allocation;
  GtkAllocation child4_allocation;
  guint16 border_width;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SQPANE (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;

  sqpane = GTK_SQPANE (widget);
  border_width = GTK_CONTAINER (sqpane)->border_width;

  if (sqpane->child1)
    gtk_widget_get_child_requisition (sqpane->child1, &child1_requisition);
  else
    {
      child1_requisition.width = 0;
      child1_requisition.height = 0;
    }

  if (sqpane->child2)
    gtk_widget_get_child_requisition (sqpane->child2, &child2_requisition);
  else
    {
      child2_requisition.width = 0;
      child2_requisition.height = 0;
    }

  if (sqpane->child3)
    gtk_widget_get_child_requisition (sqpane->child3, &child3_requisition);
  else
    {
      child3_requisition.width = 0;
      child3_requisition.height = 0;
    }

  if (sqpane->child4)
    gtk_widget_get_child_requisition (sqpane->child4, &child4_requisition);
  else
    {
      child4_requisition.width = 0;
      child4_requisition.height = 0;
    }
    
  gtk_sqpane_compute_position (sqpane,
			      widget->allocation.width - sqpane->handle_size - 2 * border_width,
			      widget->allocation.height - sqpane->handle_size - 2 * border_width);//,
			      //child1_requisition.width,
			      //child2_requisition.width);
  
  /* Move the handle before the children so we don't get extra expose events */

  sqpane->handle_xpos = sqpane->child1_width + border_width;
  sqpane->handle_ypos = sqpane->child1_height + border_width;

  if (GTK_WIDGET_REALIZED (widget))
    {
      gdk_window_move_resize (widget->window,
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);
      
      gdk_window_move_resize (sqpane->vhandle,
                  border_width, sqpane->handle_ypos,
				   allocation->width, sqpane->handle_size);
      
      gdk_window_move_resize (sqpane->hhandle,
                  sqpane->handle_xpos, border_width,
				  sqpane->handle_size, allocation->height);
      
      gdk_window_move_resize (sqpane->chandle,
                  sqpane->handle_xpos, sqpane->handle_ypos,
				  sqpane->handle_size, sqpane->handle_size);
    }

  child1_allocation.x = border_width;
  child1_allocation.y = border_width;
  child1_allocation.width = sqpane->child1_width;
  child1_allocation.height = sqpane->child1_height;

  child2_allocation.x = child1_allocation.x + child1_allocation.width + sqpane->handle_size;
  child2_allocation.y = border_width;
  child2_allocation.width = MAX (1, (gint)allocation->width - child2_allocation.x - border_width);
  child2_allocation.height = sqpane->child1_height;

  child3_allocation.x = border_width;
  child3_allocation.y = child1_allocation.y + child1_allocation.height + sqpane->handle_size;
  child3_allocation.width = sqpane->child1_width;
  child3_allocation.height = MAX (1, (gint)allocation->height - child3_allocation.y - border_width);

  child4_allocation.x = child1_allocation.x + child1_allocation.width + sqpane->handle_size;
  child4_allocation.y = child1_allocation.y + child1_allocation.height + sqpane->handle_size;
  child4_allocation.width = MAX (1, (gint)allocation->width - child4_allocation.x - border_width);
  child4_allocation.height = MAX (1, (gint)allocation->height - child4_allocation.y - border_width);

  /* Now allocate the childen, making sure, when resizing not to
   * overlap the windows */
  if (GTK_WIDGET_MAPPED(widget) &&
      sqpane->child1 && GTK_WIDGET_VISIBLE (sqpane->child1) &&
      sqpane->child1->allocation.width < child1_allocation.width &&
      sqpane->child1->allocation.height < child1_allocation.height)
    {
      if (sqpane->child4 && GTK_WIDGET_VISIBLE (sqpane->child4))
        gtk_widget_size_allocate (sqpane->child4, &child4_allocation);
      if (sqpane->child2 && GTK_WIDGET_VISIBLE (sqpane->child2))
        gtk_widget_size_allocate (sqpane->child2, &child2_allocation);
      if (sqpane->child3 && GTK_WIDGET_VISIBLE (sqpane->child3))
        gtk_widget_size_allocate (sqpane->child3, &child3_allocation);
      gtk_widget_size_allocate (sqpane->child1, &child1_allocation);      
    }
  else if (GTK_WIDGET_MAPPED(widget) &&
      sqpane->child1 && GTK_WIDGET_VISIBLE (sqpane->child1) &&
      sqpane->child1->allocation.width > child1_allocation.width &&
      sqpane->child1->allocation.height < child1_allocation.height)
    {
      if (sqpane->child3 && GTK_WIDGET_VISIBLE (sqpane->child3))
        gtk_widget_size_allocate (sqpane->child3, &child3_allocation);
      gtk_widget_size_allocate (sqpane->child1, &child1_allocation);      
      if (sqpane->child4 && GTK_WIDGET_VISIBLE (sqpane->child4))
        gtk_widget_size_allocate (sqpane->child4, &child4_allocation);
      if (sqpane->child2 && GTK_WIDGET_VISIBLE (sqpane->child2))
        gtk_widget_size_allocate (sqpane->child2, &child2_allocation);
    }
  else if (GTK_WIDGET_MAPPED(widget) &&
      sqpane->child1 && GTK_WIDGET_VISIBLE (sqpane->child1) &&
      sqpane->child1->allocation.width < child1_allocation.width &&
      sqpane->child1->allocation.height > child1_allocation.height)
    {
      if (sqpane->child2 && GTK_WIDGET_VISIBLE (sqpane->child2))
        gtk_widget_size_allocate (sqpane->child2, &child2_allocation);
      if (sqpane->child4 && GTK_WIDGET_VISIBLE (sqpane->child4))
        gtk_widget_size_allocate (sqpane->child4, &child4_allocation);
      gtk_widget_size_allocate (sqpane->child1, &child1_allocation);      
      if (sqpane->child3 && GTK_WIDGET_VISIBLE (sqpane->child3))
        gtk_widget_size_allocate (sqpane->child3, &child3_allocation);
    }
  else
    {
      if (sqpane->child1 && GTK_WIDGET_VISIBLE (sqpane->child1))
	gtk_widget_size_allocate (sqpane->child1, &child1_allocation);
      if (sqpane->child2 && GTK_WIDGET_VISIBLE (sqpane->child2))
	gtk_widget_size_allocate (sqpane->child2, &child2_allocation);
      if (sqpane->child3 && GTK_WIDGET_VISIBLE (sqpane->child3))
	gtk_widget_size_allocate (sqpane->child3, &child3_allocation);
      if (sqpane->child4 && GTK_WIDGET_VISIBLE (sqpane->child4))
	gtk_widget_size_allocate (sqpane->child4, &child4_allocation);
    }
}

static void
gtk_sqpane_draw (GtkWidget    *widget,
		GdkRectangle *area)
{
  GtkSQPane *sqpane;
  GdkRectangle handle_area, child_area;
  guint16 border_width;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SQPANE (widget));

  if (GTK_WIDGET_VISIBLE (widget) && GTK_WIDGET_MAPPED (widget))
    {
      gint width, height;
      
      sqpane = GTK_SQPANE (widget);
      border_width = GTK_CONTAINER (sqpane)->border_width;

      gdk_window_clear_area (widget->window,
			     area->x, area->y, area->width, area->height);

      if (sqpane->child1 &&
	  gtk_widget_intersect (sqpane->child1, area, &child_area))
        gtk_widget_draw (sqpane->child1, &child_area);
      if (sqpane->child2 &&
	  gtk_widget_intersect (sqpane->child2, area, &child_area))
        gtk_widget_draw (sqpane->child2, &child_area);
      if (sqpane->child3 &&
	  gtk_widget_intersect (sqpane->child3, area, &child_area))
        gtk_widget_draw (sqpane->child3, &child_area);
      if (sqpane->child4 &&
	  gtk_widget_intersect (sqpane->child4, area, &child_area))
        gtk_widget_draw (sqpane->child4, &child_area);

    }
}

static void
gtk_sqpane_vxor_line (GtkSQPane *sqpane)
{
  GtkWidget *widget;
  GdkGCValues values;
  guint16 ypos;

  widget = GTK_WIDGET(sqpane);

  if (!sqpane->xor_gc)
    {
      values.function = GDK_INVERT;
      values.subwindow_mode = GDK_INCLUDE_INFERIORS;
      sqpane->xor_gc = gdk_gc_new_with_values (widget->window,
					      &values,
					      GDK_GC_FUNCTION |
					      GDK_GC_SUBWINDOW);
    }

  ypos = sqpane->child1_height
    + GTK_CONTAINER (sqpane)->border_width + sqpane->handle_size / 2;

  gdk_draw_line (widget->window, sqpane->xor_gc,
		 0,
		 ypos,
		 widget->allocation.width - 1,
		 ypos);
}

static void
gtk_sqpane_hxor_line (GtkSQPane *sqpane)
{
  GtkWidget *widget;
  GdkGCValues values;
  guint16 xpos;

  widget = GTK_WIDGET(sqpane);

  if (!sqpane->xor_gc)
    {
      values.function = GDK_INVERT;
      values.subwindow_mode = GDK_INCLUDE_INFERIORS;
      sqpane->xor_gc = gdk_gc_new_with_values (widget->window,
					      &values,
					      GDK_GC_FUNCTION |
					      GDK_GC_SUBWINDOW);
    }

  xpos = sqpane->child1_width
    + GTK_CONTAINER (sqpane)->border_width + sqpane->handle_size / 2;

  gdk_draw_line (widget->window, sqpane->xor_gc,
		 xpos,
		 0,
		 xpos,
		 widget->allocation.height - 1);
}

static gint
gtk_sqpane_button_press (GtkWidget *widget, GdkEventButton *event)
{
  GtkSQPane *sqpane;

  g_return_val_if_fail (widget != NULL,FALSE);
  g_return_val_if_fail (GTK_IS_SQPANE (widget),FALSE);

  sqpane = GTK_SQPANE (widget);

  if (!sqpane->in_drag && (event->button == 1))
    {
      if(event->window == sqpane->vhandle)
        {
          sqpane->in_drag = 1;
          //* We need a server grab here, not gtk_grab_add(), since
          //* we don't want to pass events on to the widget's children
          gdk_pointer_grab (sqpane->vhandle, FALSE,
			GDK_POINTER_MOTION_HINT_MASK 
			| GDK_BUTTON1_MOTION_MASK 
			| GDK_BUTTON_RELEASE_MASK,
			NULL, NULL, event->time);
          sqpane->child1_height += event->y - sqpane->handle_size / 2;
          sqpane->child1_height = CLAMP ((guint16)sqpane->child1_height, 0,
                                  widget->allocation.height - sqpane->handle_size
                                  - 2 * GTK_CONTAINER (sqpane)->border_width);
          gtk_sqpane_vxor_line (sqpane);
        }
	  else if(event->window == sqpane->hhandle)
        {
          sqpane->in_drag = 2;
          //* We need a server grab here, not gtk_grab_add(), since
          //* we don't want to pass events on to the widget's children
          gdk_pointer_grab (sqpane->hhandle, FALSE,
			GDK_POINTER_MOTION_HINT_MASK 
			| GDK_BUTTON1_MOTION_MASK 
			| GDK_BUTTON_RELEASE_MASK,
			NULL, NULL, event->time);
          sqpane->child1_width += event->x - sqpane->handle_size / 2;
          sqpane->child1_width = CLAMP ((guint16)sqpane->child1_width, 0,
                                  widget->allocation.width - sqpane->handle_size
                                  - 2 * GTK_CONTAINER (sqpane)->border_width);
          gtk_sqpane_hxor_line (sqpane);
        }
	  else if(event->window == sqpane->chandle)
        {
          sqpane->in_drag = 3;
          //* We need a server grab here, not gtk_grab_add(), since
          //* we don't want to pass events on to the widget's children
          gdk_pointer_grab (sqpane->chandle, FALSE,
			GDK_POINTER_MOTION_HINT_MASK 
			| GDK_BUTTON1_MOTION_MASK 
			| GDK_BUTTON_RELEASE_MASK,
			NULL, NULL, event->time);
          sqpane->child1_width += event->x - sqpane->handle_size / 2;
          sqpane->child1_width = CLAMP ((guint16)sqpane->child1_width, 0,
                                  widget->allocation.width - sqpane->handle_size
                                  - 2 * GTK_CONTAINER (sqpane)->border_width);
          sqpane->child1_height += event->y - sqpane->handle_size / 2;
          sqpane->child1_height = CLAMP ((guint16)sqpane->child1_height, 0,
                                  widget->allocation.height - sqpane->handle_size
                                  - 2 * GTK_CONTAINER (sqpane)->border_width);
          gtk_sqpane_vxor_line (sqpane);
          gtk_sqpane_hxor_line (sqpane);
        }
    }

  return TRUE;
}

static gint
gtk_sqpane_button_release (GtkWidget *widget, GdkEventButton *event)
{
  GtkSQPane *sqpane;

  g_return_val_if_fail (widget != NULL,FALSE);
  g_return_val_if_fail (GTK_IS_SQPANE (widget),FALSE);

  sqpane = GTK_SQPANE (widget);

  if (sqpane->in_drag>0 && (event->button == 1))
    {
	  if(sqpane->in_drag&1==1)
        gtk_sqpane_vxor_line (sqpane);
	  if((sqpane->in_drag&2)==2)
        gtk_sqpane_hxor_line (sqpane);
      sqpane->in_drag = 0;
      sqpane->position_set = TRUE;
      gdk_pointer_ungrab (event->time);
      gtk_widget_queue_resize (GTK_WIDGET (sqpane));
    }

  return TRUE;
}

static gint
gtk_sqpane_motion (GtkWidget *widget, GdkEventMotion *event)
{
  GtkSQPane *sqpane;
  gint x;
  gint y;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_SQPANE (widget), FALSE);

  sqpane = GTK_SQPANE (widget);

  if (event->is_hint || event->window != widget->window)
    gtk_widget_get_pointer(widget, &x, &y);
  else
    {
    x = event->x;
    y = event->y;
    }

  if (sqpane->in_drag==1)
    {
      y = y - GTK_CONTAINER (sqpane)->border_width - sqpane->handle_size/2;
      
      gtk_sqpane_vxor_line (sqpane);
      sqpane->child1_height = CLAMP (y,
				  sqpane->min_y,
				  sqpane->max_y);
      gtk_sqpane_vxor_line (sqpane);
    }
  else if (sqpane->in_drag==2)
    {
      x = x - GTK_CONTAINER (sqpane)->border_width - sqpane->handle_size/2;
      
      gtk_sqpane_hxor_line (sqpane);
      sqpane->child1_width = CLAMP (x,
				  sqpane->min_x,
				  sqpane->max_x);
      gtk_sqpane_hxor_line (sqpane);
    }
  else if (sqpane->in_drag==3)
    {
      x = x - GTK_CONTAINER (sqpane)->border_width - sqpane->handle_size/2;
      y = y - GTK_CONTAINER (sqpane)->border_width - sqpane->handle_size/2;
      
      gtk_sqpane_vxor_line (sqpane);
      gtk_sqpane_hxor_line (sqpane);
      sqpane->child1_width = CLAMP (x,
				  sqpane->min_x,
				  sqpane->max_x);
      sqpane->child1_height = CLAMP (y,
				  sqpane->min_y,
				  sqpane->max_y);
      gtk_sqpane_vxor_line (sqpane);
      gtk_sqpane_hxor_line (sqpane);
    }

  return TRUE;
}

