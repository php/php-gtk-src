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

#include "gtkshpaned.h"

static void gtk_shpaned_class_init (GtkSHPanedClass    *klass);
static void gtk_shpaned_init       (GtkSHPaned         *shpaned);
static void gtk_shpaned_realize    (GtkWidget *widget);
static void gtk_shpaned_size_request     (GtkWidget      *widget,
					 GtkRequisition *requisition);
static void gtk_shpaned_size_allocate    (GtkWidget          *widget,
					 GtkAllocation      *allocation);
static void gtk_shpaned_draw             (GtkWidget    *widget,
					 GdkRectangle *area);
static void gtk_shpaned_xor_line         (GtkSPaned *shpaned);
static gint gtk_shpaned_button_press     (GtkWidget *widget,
					 GdkEventButton *event);
static gint gtk_shpaned_button_release   (GtkWidget *widget,
					 GdkEventButton *event);
static gint gtk_shpaned_motion           (GtkWidget *widget,
					 GdkEventMotion *event);

GtkType
gtk_shpaned_get_type (void)
{
  static GtkType shpaned_type = 0;
  
  if (!shpaned_type)
    {
      static const GtkTypeInfo shpaned_info =
      {
	"GtkSHPaned",
	sizeof (GtkSHPaned),
	sizeof (GtkSHPanedClass),
	(GtkClassInitFunc) gtk_shpaned_class_init,
	(GtkObjectInitFunc) gtk_shpaned_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
      
      shpaned_type = gtk_type_unique (gtk_spaned_get_type (), &shpaned_info);
    }
  
  return shpaned_type;
}

static void
gtk_shpaned_class_init (GtkSHPanedClass *class)
{
  GtkWidgetClass *widget_class;
  
  widget_class = (GtkWidgetClass*) class;
 
  widget_class->size_request = gtk_shpaned_size_request;
  widget_class->size_allocate = gtk_shpaned_size_allocate;
  widget_class->draw = gtk_shpaned_draw;
  widget_class->button_press_event = gtk_shpaned_button_press;
  widget_class->button_release_event = gtk_shpaned_button_release;
  widget_class->motion_notify_event = gtk_shpaned_motion;
  widget_class->realize = gtk_shpaned_realize;
}

static void
gtk_shpaned_init (GtkSHPaned *shpaned)
{
}

GtkWidget*
gtk_shpaned_new (void)
{
  GtkSHPaned *shpaned;

  shpaned = gtk_type_new (gtk_shpaned_get_type ());

  return GTK_WIDGET (shpaned);
}

static void
gtk_shpaned_realize (GtkWidget *widget)
{
  GtkSPaned *spaned;
  GdkWindowAttr attributes;
  gint attributes_mask;
  guint16 border_width;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SPANED (widget));
  
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  spaned = GTK_SPANED (widget);
  
  border_width = GTK_CONTAINER (spaned)->border_width;
  
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
  gdk_window_set_user_data (widget->window, spaned);
  
  attributes.x = spaned->handle_xpos;
  attributes.y = spaned->handle_ypos;
  attributes.width = spaned->handle_width;
  attributes.height = spaned->handle_height;
  attributes.cursor = gdk_cursor_new (GDK_SB_H_DOUBLE_ARROW);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
			    GDK_BUTTON_RELEASE_MASK |
			    GDK_POINTER_MOTION_MASK |
			    GDK_POINTER_MOTION_HINT_MASK);
  attributes_mask |= GDK_WA_CURSOR;
  
  spaned->handle = gdk_window_new (widget->window,
				  &attributes, attributes_mask);
  gdk_window_set_user_data (spaned->handle, spaned);
  gdk_cursor_destroy (attributes.cursor);
  
  widget->style = gtk_style_attach (widget->style, widget->window);
  
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
  gtk_style_set_background (widget->style, spaned->handle, GTK_STATE_NORMAL);

  gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
  
  gdk_window_show (spaned->handle);
}

static void
gtk_shpaned_size_request (GtkWidget      *widget,
			 GtkRequisition *requisition)
{
  GtkSPaned *shpaned;
  GtkRequisition child_requisition;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SHPANED (widget));
  g_return_if_fail (requisition != NULL);

  shpaned = GTK_SPANED (widget);
  requisition->width = 0;
  requisition->height = 0;

  if (shpaned->child1 && GTK_WIDGET_VISIBLE (shpaned->child1))
    {
      gtk_widget_size_request (shpaned->child1, &child_requisition);

      requisition->height = child_requisition.height;
      requisition->width = child_requisition.width;
    }

  if (shpaned->child2 && GTK_WIDGET_VISIBLE (shpaned->child2))
    {
      gtk_widget_size_request (shpaned->child2, &child_requisition);

      requisition->height = MAX(requisition->height, child_requisition.height);
      requisition->width += child_requisition.width;
    }

  requisition->width += GTK_CONTAINER (shpaned)->border_width * 2 + shpaned->handle_size;
  requisition->height += GTK_CONTAINER (shpaned)->border_width * 2;
}

static void
gtk_shpaned_size_allocate (GtkWidget     *widget,
			  GtkAllocation *allocation)
{
  GtkSPaned *shpaned;
  GtkRequisition child1_requisition;
  GtkRequisition child2_requisition;
  GtkAllocation child1_allocation;
  GtkAllocation child2_allocation;
  guint16 border_width;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SHPANED (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;

  shpaned = GTK_SPANED (widget);
  border_width = GTK_CONTAINER (shpaned)->border_width;

  if (shpaned->child1)
    gtk_widget_size_request(shpaned->child1, &child1_requisition);
  else
    child1_requisition.width = 0;

  if (shpaned->child2)
    gtk_widget_size_request(shpaned->child2, &child2_requisition);
  else
    child2_requisition.width = 0;
    
  gtk_spaned_compute_position (shpaned,
			      widget->allocation.width
			        - shpaned->handle_size
			        - 2 * border_width,
			      child1_requisition.width,
			      child2_requisition.width);
  
  /* Move the handle before the children so we don't get extra expose events */

  shpaned->handle_xpos = shpaned->child1_size + border_width;
  shpaned->handle_ypos = border_width;
  shpaned->handle_width = shpaned->handle_size;
  shpaned->handle_height = allocation->height;

  if (GTK_WIDGET_REALIZED (widget))
    {
      gdk_window_move_resize (widget->window,
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);
      
      gdk_window_move_resize (shpaned->handle,
                  shpaned->handle_xpos, shpaned->handle_ypos,
				  shpaned->handle_width, shpaned->handle_height);
    }

  child1_allocation.height = child2_allocation.height = MAX (1, (gint)allocation->height - border_width * 2);
  child1_allocation.width = shpaned->child1_size;
  child1_allocation.x = border_width;
  child1_allocation.y = child2_allocation.y = border_width;

  child2_allocation.x = child1_allocation.x + child1_allocation.width + shpaned->handle_size;
  child2_allocation.width = MAX (1, (gint)allocation->width - child2_allocation.x - border_width);
  
  /* Now allocate the childen, making sure, when resizing not to
   * overlap the windows */
  if (GTK_WIDGET_MAPPED(widget) &&
      shpaned->child1 && GTK_WIDGET_VISIBLE (shpaned->child1) &&
      shpaned->child1->allocation.width < child1_allocation.width)
    {
      if (shpaned->child2 && GTK_WIDGET_VISIBLE (shpaned->child2))
	gtk_widget_size_allocate (shpaned->child2, &child2_allocation);
      gtk_widget_size_allocate (shpaned->child1, &child1_allocation);      
    }
  else
    {
      if (shpaned->child1 && GTK_WIDGET_VISIBLE (shpaned->child1))
	gtk_widget_size_allocate (shpaned->child1, &child1_allocation);
      if (shpaned->child2 && GTK_WIDGET_VISIBLE (shpaned->child2))
	gtk_widget_size_allocate (shpaned->child2, &child2_allocation);
    }
}

static void
gtk_shpaned_draw (GtkWidget    *widget,
		GdkRectangle *area)
{
  GtkSPaned *shpaned;
  GdkRectangle child_area;
  guint16 border_width;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SHPANED (widget));

  if (GTK_WIDGET_VISIBLE (widget) && GTK_WIDGET_MAPPED (widget))
    {
      shpaned = GTK_SPANED (widget);
      border_width = GTK_CONTAINER (shpaned)->border_width;

      gdk_window_clear_area (widget->window,
			     area->x, area->y, area->width, area->height);

      if (shpaned->child1 &&
	  gtk_widget_intersect (shpaned->child1, area, &child_area))
        gtk_widget_draw (shpaned->child1, &child_area);
      if (shpaned->child2 &&
	  gtk_widget_intersect (shpaned->child2, area, &child_area))
        gtk_widget_draw (shpaned->child2, &child_area);

    }
}

static void
gtk_shpaned_xor_line (GtkSPaned *shpaned)
{
  GtkWidget *widget;
  GdkGCValues values;
  guint16 xpos;

  widget = GTK_WIDGET(shpaned);

  if (!shpaned->xor_gc)
    {
      values.function = GDK_INVERT;
      values.subwindow_mode = GDK_INCLUDE_INFERIORS;
      shpaned->xor_gc = gdk_gc_new_with_values (widget->window,
					      &values,
					      GDK_GC_FUNCTION |
					      GDK_GC_SUBWINDOW);
    }

  xpos = shpaned->child1_size
    + GTK_CONTAINER (shpaned)->border_width + shpaned->handle_size / 2;

  gdk_draw_line (widget->window, shpaned->xor_gc,
		 xpos,
		 0,
		 xpos,
		 widget->allocation.height - 1);
}

static gint
gtk_shpaned_button_press (GtkWidget *widget, GdkEventButton *event)
{
  GtkSPaned *shpaned;

  g_return_val_if_fail (widget != NULL,FALSE);
  g_return_val_if_fail (GTK_IS_SHPANED (widget),FALSE);

  shpaned = GTK_SPANED (widget);

  if (!shpaned->in_drag &&
      (event->window == shpaned->handle) && (event->button == 1))
    {
      shpaned->in_drag = TRUE;
      /* We need a server grab here, not gtk_grab_add(), since
       * we don't want to pass events on to the widget's children */
      gdk_pointer_grab (shpaned->handle, FALSE,
			GDK_POINTER_MOTION_HINT_MASK 
			| GDK_BUTTON1_MOTION_MASK 
			| GDK_BUTTON_RELEASE_MASK,
			NULL, NULL, event->time);
      shpaned->child1_size += (gint)event->x - shpaned->handle_size / 2;
      shpaned->child1_size = CLAMP ((guint16)shpaned->child1_size, 0,
                                  widget->allocation.width - shpaned->handle_size
                                  - 2 * GTK_CONTAINER (shpaned)->border_width);
      gtk_shpaned_xor_line (shpaned);
    }

  return TRUE;
}

static gint
gtk_shpaned_button_release (GtkWidget *widget, GdkEventButton *event)
{
  GtkSPaned *shpaned;

  g_return_val_if_fail (widget != NULL,FALSE);
  g_return_val_if_fail (GTK_IS_SHPANED (widget),FALSE);

  shpaned = GTK_SPANED (widget);

  if (shpaned->in_drag && (event->button == 1))
    {
      gtk_shpaned_xor_line (shpaned);
      shpaned->in_drag = FALSE;
      shpaned->position_set = TRUE;
      gdk_pointer_ungrab (event->time);
      gtk_widget_queue_resize (GTK_WIDGET (shpaned));
    }

  return TRUE;
}

static gint
gtk_shpaned_motion (GtkWidget *widget, GdkEventMotion *event)
{
  GtkSPaned *shpaned;
  gint x;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_SHPANED (widget), FALSE);

  shpaned = GTK_SPANED (widget);

  if (event->is_hint || event->window != widget->window)
    gtk_widget_get_pointer(widget, &x, NULL);
  else
    x = (gint)event->x;

  if (shpaned->in_drag)
    {
      gint size = x - GTK_CONTAINER (shpaned)->border_width - shpaned->handle_size/2;
      
      gtk_shpaned_xor_line (shpaned);
      shpaned->child1_size = CLAMP (size,
				  shpaned->min_position,
				  shpaned->max_position);
      gtk_shpaned_xor_line (shpaned);
    }

  return TRUE;
}

