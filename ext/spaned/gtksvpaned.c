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

#include "gtksvpaned.h"

static void gtk_svpaned_class_init (GtkSVPanedClass    *klass);
static void gtk_svpaned_init       (GtkSVPaned         *svpaned);
static void gtk_svpaned_realize    (GtkWidget *widget);
static void gtk_svpaned_size_request     (GtkWidget      *widget,
					 GtkRequisition *requisition);
static void gtk_svpaned_size_allocate    (GtkWidget          *widget,
					 GtkAllocation      *allocation);
static void gtk_svpaned_draw             (GtkWidget    *widget,
					 GdkRectangle *area);
static void gtk_svpaned_xor_line         (GtkSPaned *svpaned);
static gint gtk_svpaned_button_press     (GtkWidget *widget,
					 GdkEventButton *event);
static gint gtk_svpaned_button_release   (GtkWidget *widget,
					 GdkEventButton *event);
static gint gtk_svpaned_motion           (GtkWidget *widget,
					 GdkEventMotion *event);

GtkType
gtk_svpaned_get_type (void)
{
  static GtkType svpaned_type = 0;
  
  if (!svpaned_type)
    {
      static const GtkTypeInfo svpaned_info =
      {
	"GtkSVPaned",
	sizeof (GtkSVPaned),
	sizeof (GtkSVPanedClass),
	(GtkClassInitFunc) gtk_svpaned_class_init,
	(GtkObjectInitFunc) gtk_svpaned_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
      
      svpaned_type = gtk_type_unique (gtk_spaned_get_type (), &svpaned_info);
    }
  
  return svpaned_type;
}

static void
gtk_svpaned_class_init (GtkSVPanedClass *class)
{
  GtkWidgetClass *widget_class;
  
  widget_class = (GtkWidgetClass*) class;
 
  widget_class->size_request = gtk_svpaned_size_request;
  widget_class->size_allocate = gtk_svpaned_size_allocate;
  widget_class->draw = gtk_svpaned_draw;
  widget_class->button_press_event = gtk_svpaned_button_press;
  widget_class->button_release_event = gtk_svpaned_button_release;
  widget_class->motion_notify_event = gtk_svpaned_motion;
  widget_class->realize = gtk_svpaned_realize;
}

static void
gtk_svpaned_init (GtkSVPaned *svpaned)
{
}

GtkWidget*
gtk_svpaned_new (void)
{
  GtkSVPaned *svpaned;

  svpaned = gtk_type_new (gtk_svpaned_get_type ());

  return GTK_WIDGET (svpaned);
}

static void
gtk_svpaned_realize (GtkWidget *widget)
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
  attributes.cursor = gdk_cursor_new (GDK_SB_V_DOUBLE_ARROW);
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
gtk_svpaned_size_request (GtkWidget      *widget,
			 GtkRequisition *requisition)
{
  GtkSPaned *svpaned;
  GtkRequisition child_requisition;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SVPANED (widget));
  g_return_if_fail (requisition != NULL);

  svpaned = GTK_SPANED (widget);
  requisition->width = 0;
  requisition->height = 0;

  if (svpaned->child1 && GTK_WIDGET_VISIBLE (svpaned->child1))
    {
      gtk_widget_size_request (svpaned->child1, &child_requisition);

      requisition->height = child_requisition.height;
      requisition->width = child_requisition.width;
    }

  if (svpaned->child2 && GTK_WIDGET_VISIBLE (svpaned->child2))
    {
      gtk_widget_size_request (svpaned->child2, &child_requisition);

      requisition->height += child_requisition.height;
      requisition->width = MAX(requisition->width, child_requisition.width);
    }

  requisition->width += GTK_CONTAINER (svpaned)->border_width * 2;
  requisition->height += GTK_CONTAINER (svpaned)->border_width * 2 + svpaned->handle_size;
}

static void
gtk_svpaned_size_allocate (GtkWidget     *widget,
			  GtkAllocation *allocation)
{
  GtkSPaned *svpaned;
  GtkRequisition child1_requisition;
  GtkRequisition child2_requisition;
  GtkAllocation child1_allocation;
  GtkAllocation child2_allocation;
  guint16 border_width;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SVPANED (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;

  svpaned = GTK_SPANED (widget);
  border_width = GTK_CONTAINER (svpaned)->border_width;

  if (svpaned->child1)
    gtk_widget_size_request(svpaned->child1, &child1_requisition);
  else
    child1_requisition.height = 0;

  if (svpaned->child2)
    gtk_widget_size_request(svpaned->child1, &child1_requisition);
  else
    child2_requisition.height = 0;
    
  gtk_spaned_compute_position (svpaned,
			      widget->allocation.height
			        - svpaned->handle_size
			        - 2 * border_width,
			      child1_requisition.height,
			      child2_requisition.height);
  
  /* Move the handle before the children so we don't get extra expose events */

  svpaned->handle_xpos = border_width;
  svpaned->handle_ypos = svpaned->child1_size + border_width;
  svpaned->handle_width = allocation->width;
  svpaned->handle_height = svpaned->handle_size;

  if (GTK_WIDGET_REALIZED (widget))
    {
      gdk_window_move_resize (widget->window,
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);
      
      gdk_window_move_resize (svpaned->handle,
                  svpaned->handle_xpos, svpaned->handle_ypos,
				  svpaned->handle_width, svpaned->handle_height);
    }

  child1_allocation.width = child2_allocation.width = MAX (1, (gint)allocation->width - border_width * 2);
  child1_allocation.height = svpaned->child1_size;
  child1_allocation.x = child2_allocation.x = border_width;
  child1_allocation.y = border_width;

  child2_allocation.y = child1_allocation.y + child1_allocation.height + svpaned->handle_size;
  child2_allocation.height = MAX (1, (gint)allocation->height - child2_allocation.y - border_width);
  
  /* Now allocate the childen, making sure, when resizing not to
   * overlap the windows */
  if (GTK_WIDGET_MAPPED(widget) &&
      svpaned->child1 && GTK_WIDGET_VISIBLE (svpaned->child1) &&
      svpaned->child1->allocation.height < child1_allocation.height)
    {
      if (svpaned->child2 && GTK_WIDGET_VISIBLE (svpaned->child2))
	gtk_widget_size_allocate (svpaned->child2, &child2_allocation);
      gtk_widget_size_allocate (svpaned->child1, &child1_allocation);      
    }
  else
    {
      if (svpaned->child1 && GTK_WIDGET_VISIBLE (svpaned->child1))
	gtk_widget_size_allocate (svpaned->child1, &child1_allocation);
      if (svpaned->child2 && GTK_WIDGET_VISIBLE (svpaned->child2))
	gtk_widget_size_allocate (svpaned->child2, &child2_allocation);
    }
}

static void
gtk_svpaned_draw (GtkWidget    *widget,
		GdkRectangle *area)
{
  GtkSPaned *svpaned;
  GdkRectangle handle_area, child_area;
  guint16 border_width;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SVPANED (widget));

  if (GTK_WIDGET_VISIBLE (widget) && GTK_WIDGET_MAPPED (widget))
    {
      gint width, height;
      
      svpaned = GTK_SPANED (widget);
      border_width = GTK_CONTAINER (svpaned)->border_width;

      gdk_window_clear_area (widget->window,
			     area->x, area->y, area->width, area->height);

      if (svpaned->child1 &&
	  gtk_widget_intersect (svpaned->child1, area, &child_area))
        gtk_widget_draw (svpaned->child1, &child_area);
      if (svpaned->child2 &&
	  gtk_widget_intersect (svpaned->child2, area, &child_area))
        gtk_widget_draw (svpaned->child2, &child_area);

    }
}

static void
gtk_svpaned_xor_line (GtkSPaned *svpaned)
{
  GtkWidget *widget;
  GdkGCValues values;
  guint16 ypos;

  widget = GTK_WIDGET(svpaned);

  if (!svpaned->xor_gc)
    {
      values.function = GDK_INVERT;
      values.subwindow_mode = GDK_INCLUDE_INFERIORS;
      svpaned->xor_gc = gdk_gc_new_with_values (widget->window,
					      &values,
					      GDK_GC_FUNCTION |
					      GDK_GC_SUBWINDOW);
    }

  ypos = svpaned->child1_size
    + GTK_CONTAINER (svpaned)->border_width + svpaned->handle_size / 2;

  gdk_draw_line (widget->window, svpaned->xor_gc,
		 0,
		 ypos,
		 widget->allocation.width - 1,
		 ypos);
}

static gint
gtk_svpaned_button_press (GtkWidget *widget, GdkEventButton *event)
{
  GtkSPaned *svpaned;

  g_return_val_if_fail (widget != NULL,FALSE);
  g_return_val_if_fail (GTK_IS_SVPANED (widget),FALSE);

  svpaned = GTK_SPANED (widget);

  if (!svpaned->in_drag &&
      (event->window == svpaned->handle) && (event->button == 1))
    {
      svpaned->in_drag = TRUE;
      /* We need a server grab here, not gtk_grab_add(), since
       * we don't want to pass events on to the widget's children */
      gdk_pointer_grab (svpaned->handle, FALSE,
			GDK_POINTER_MOTION_HINT_MASK 
			| GDK_BUTTON1_MOTION_MASK 
			| GDK_BUTTON_RELEASE_MASK,
			NULL, NULL, event->time);
      svpaned->child1_size += event->y - svpaned->handle_size / 2;
      svpaned->child1_size = CLAMP (svpaned->child1_size, 0,
                                  widget->allocation.height - svpaned->handle_size
                                  - 2 * GTK_CONTAINER (svpaned)->border_width);
      gtk_svpaned_xor_line (svpaned);
    }

  return TRUE;
}

static gint
gtk_svpaned_button_release (GtkWidget *widget, GdkEventButton *event)
{
  GtkSPaned *svpaned;

  g_return_val_if_fail (widget != NULL,FALSE);
  g_return_val_if_fail (GTK_IS_SVPANED (widget),FALSE);

  svpaned = GTK_SPANED (widget);

  if (svpaned->in_drag && (event->button == 1))
    {
      gtk_svpaned_xor_line (svpaned);
      svpaned->in_drag = FALSE;
      svpaned->position_set = TRUE;
      gdk_pointer_ungrab (event->time);
      gtk_widget_queue_resize (GTK_WIDGET (svpaned));
    }

  return TRUE;
}

static gint
gtk_svpaned_motion (GtkWidget *widget, GdkEventMotion *event)
{
  GtkSPaned *svpaned;
  gint y;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_SVPANED (widget), FALSE);

  svpaned = GTK_SPANED (widget);

  if (event->is_hint || event->window != widget->window)
    gtk_widget_get_pointer(widget, NULL, &y);
  else
    y = event->y;

  if (svpaned->in_drag)
    {
      gint size = y - GTK_CONTAINER (svpaned)->border_width - svpaned->handle_size/2;
      
      gtk_svpaned_xor_line (svpaned);
      svpaned->child1_size = CLAMP (size,
				  svpaned->min_position,
				  svpaned->max_position);
      gtk_svpaned_xor_line (svpaned);
    }

  return TRUE;
}

