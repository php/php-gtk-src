
/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Scrollpane widget for GTK/GNOME
 * Copyright (C) 1998 - 2001 Redhat Software Inc.
 * Author: Jonathan Blandford <jrb@redhat.com>
 */

#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include "gtkscrollpane.h"

#define SCROLLPANE_CLASS(w) GTK_SCROLLPANE_CLASS (GTK_OBJECT (w)->klass)
#define SCROLLPANE_DEFAULT_SIZE 80;
#define EPSILON 0.00005

enum
  {
    MIDDLE_CLICKED,
    RIGHT_CLICKED,
    LAST_SIGNAL
  };

/* Forward declarations */
static void         gtk_scrollpane_init(GtkScrollpane * sp);
static void         gtk_scrollpane_class_init(GtkScrollpaneClass * klass);
static void         gtk_scrollpane_destroy(GtkObject * object);
static void         gtk_scrollpane_realize(GtkWidget * widget);
static void         gtk_scrollpane_unrealize(GtkWidget * widget);
static void         gtk_scrollpane_draw(GtkWidget * widget, GdkRectangle * area);
static void         gtk_scrollpane_draw_background(GtkScrollpane * sp);
static void         gtk_scrollpane_draw_trough(GtkScrollpane * sp);
static void         gtk_scrollpane_draw_slider(GtkScrollpane * sp);
static void         gtk_real_scrollpane_draw_trough(GtkScrollpane * sp);
static void         gtk_real_scrollpane_draw_slider(GtkScrollpane * sp);
static void         gtk_scrollpane_size_request(GtkWidget * widget, GtkRequisition * requisition);
static void         gtk_scrollpane_size_allocate(GtkWidget * widget, GtkAllocation * allocation);
static void         gtk_scrollpane_update_slider_size(GtkScrollpane * sp);
static void         gtk_scrollpane_style_set(GtkWidget * widget, GtkStyle * previous_style);
static void         gtk_scrollpane_adjustment_changed(GtkAdjustment * adjustment, gpointer data);
static void         gtk_scrollpane_value_adjustment_changed(GtkAdjustment * adjustment, gpointer data);
static gint         gtk_scrollpane_expose(GtkWidget * widget, GdkEventExpose * event);
static gint         gtk_scrollpane_button_press(GtkWidget * widget, GdkEventButton * event);
static gint         gtk_scrollpane_button_release(GtkWidget * widget, GdkEventButton * event);
static gint         gtk_scrollpane_key_press(GtkWidget * widget, GdkEventKey * event);
static gint         gtk_scrollpane_enter_notify(GtkWidget * widget, GdkEventCrossing * event);
static gint         gtk_scrollpane_leave_notify(GtkWidget * widget, GdkEventCrossing * event);
static gint         gtk_scrollpane_motion_notify(GtkWidget * widget, GdkEventMotion * event);
static gint         gtk_scrollpane_focus_in(GtkWidget * widget, GdkEventFocus * event);
static gint         gtk_scrollpane_focus_out(GtkWidget * widget, GdkEventFocus * event);

static GtkWidget   *parent_class = NULL;
static guint        scrollpane_signals[LAST_SIGNAL] = {0};

/* Static, private functions */
guint
gtk_scrollpane_get_type(void)
{
  static guint        gtk_scrollpane_type = 0;

  if (!gtk_scrollpane_type)
    {
      GtkTypeInfo         gtk_scrollpane_info =
      {
	"GtkScrollpane",
	sizeof(GtkScrollpane),
	sizeof(GtkScrollpaneClass),
	(GtkClassInitFunc) gtk_scrollpane_class_init,
	(GtkObjectInitFunc) gtk_scrollpane_init,
	(GtkArgSetFunc) NULL,
	(GtkArgGetFunc) NULL,
      };

      gtk_scrollpane_type = gtk_type_unique(gtk_widget_get_type(), &gtk_scrollpane_info);
    }
  return gtk_scrollpane_type;
}

static void
gtk_scrollpane_init(GtkScrollpane * sp)
{
  GtkWidget          *widget;

  widget = GTK_WIDGET(sp);
  sp->trough = NULL;
  sp->slider = NULL;

  sp->min_slider_width = widget->style->klass->xthickness + widget->style->klass->xthickness + 1;
  sp->min_slider_height = widget->style->klass->ythickness + widget->style->klass->ythickness + 1;
  sp->slider_height = 0;
  sp->slider_width = 0;

  sp->policy = GTK_UPDATE_CONTINUOUS;
  sp->Xadjustment = NULL;
  sp->Yadjustment = NULL;
  sp->moving = 0;
  sp->in_slider = FALSE;
  sp->trough_width = SCROLLPANE_DEFAULT_SIZE;
  sp->trough_height = SCROLLPANE_DEFAULT_SIZE;
}

static void
gtk_scrollpane_class_init(GtkScrollpaneClass * klass)
{
  GtkObjectClass     *object_class;
  GtkWidgetClass     *widget_class;

  object_class = (GtkObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;
  parent_class = gtk_type_class(gtk_widget_get_type());

  scrollpane_signals[MIDDLE_CLICKED] =
    gtk_signal_new("middle_clicked",
		   GTK_RUN_FIRST,
		   object_class->type,
		   GTK_SIGNAL_OFFSET(GtkScrollpaneClass, middle_clicked),
		   gtk_signal_default_marshaller,
		   GTK_TYPE_NONE, 0);
  scrollpane_signals[RIGHT_CLICKED] =
    gtk_signal_new("right_clicked",
		   GTK_RUN_FIRST,
		   object_class->type,
		   GTK_SIGNAL_OFFSET(GtkScrollpaneClass, right_clicked),
		   gtk_signal_default_marshaller,
		   GTK_TYPE_NONE, 0);
  gtk_object_class_add_signals(object_class, scrollpane_signals, LAST_SIGNAL);

  object_class->destroy = gtk_scrollpane_destroy;

  widget_class->realize = gtk_scrollpane_realize;
  widget_class->unrealize = gtk_scrollpane_unrealize;
  widget_class->expose_event = gtk_scrollpane_expose;
  widget_class->draw = gtk_scrollpane_draw;
  widget_class->size_request = gtk_scrollpane_size_request;
  widget_class->size_allocate = gtk_scrollpane_size_allocate;
  widget_class->button_press_event = gtk_scrollpane_button_press;
  widget_class->button_release_event = gtk_scrollpane_button_release;
  widget_class->motion_notify_event = gtk_scrollpane_motion_notify;
  widget_class->key_press_event = gtk_scrollpane_key_press;
  widget_class->enter_notify_event = gtk_scrollpane_enter_notify;
  widget_class->leave_notify_event = gtk_scrollpane_leave_notify;
  widget_class->focus_in_event = gtk_scrollpane_focus_in;
  widget_class->focus_out_event = gtk_scrollpane_focus_out;
  widget_class->style_set = gtk_scrollpane_style_set;

  klass->draw_background = NULL;
  klass->draw_trough = gtk_real_scrollpane_draw_trough;
  klass->draw_slider = gtk_real_scrollpane_draw_slider;
  klass->middle_clicked = NULL;
  klass->right_clicked = NULL;
}

static void
gtk_scrollpane_destroy(GtkObject * object)
{
  GtkScrollpane      *sp;

  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(object));

  sp = GTK_SCROLLPANE(object);

  if (GTK_OBJECT_CLASS(parent_class)->destroy)
    (*GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

static void
gtk_scrollpane_realize(GtkWidget * widget)
{
  GtkScrollpane      *sp;
  GdkWindowAttr       attributes;
  gint                attributes_mask;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(widget));

  sp = GTK_SCROLLPANE(widget);

  /* 
   * set up the widget
   */
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events(widget) |
    GDK_EXPOSURE_MASK |
    GDK_BUTTON_PRESS_MASK |
    GDK_BUTTON_RELEASE_MASK |
    GDK_POINTER_MOTION_MASK |
    GDK_ENTER_NOTIFY_MASK |
    GDK_LEAVE_NOTIFY_MASK;

  attributes.visual = gtk_widget_get_visual(widget);
  attributes.colormap = gtk_widget_get_colormap(widget);

  attributes_mask = GDK_WA_X |
    GDK_WA_Y |
    GDK_WA_VISUAL |
    GDK_WA_COLORMAP;
  widget->window = gdk_window_new(widget->parent->window, &attributes, attributes_mask);

  sp->trough = widget->window;
  gdk_window_ref(sp->trough);

  sp->slider = gdk_window_new(sp->trough,
			      &attributes,
			      attributes_mask);

  gtk_scrollpane_update_slider_size(sp);

  GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
  widget->style = gtk_style_attach(widget->style, widget->window);

  gdk_window_set_user_data(sp->trough, widget);
  gtk_style_set_background(widget->style,
			   sp->trough,
			   GTK_STATE_ACTIVE);
  gdk_window_set_user_data(sp->slider, widget);
  gtk_style_set_background(widget->style,
			   sp->slider,
			   GTK_STATE_NORMAL);
  gdk_window_show(sp->slider);
}

static void
gtk_scrollpane_unrealize(GtkWidget * widget)
{
  GtkScrollpane      *sp;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(widget));

  sp = GTK_SCROLLPANE(widget);
  GTK_WIDGET_UNSET_FLAGS(widget, GTK_REALIZED);
  
  if (sp->slider)
    {
      gdk_window_set_user_data(sp->slider, NULL);
      gdk_window_destroy(sp->slider);
      sp->slider = NULL;
    }

  if (GTK_WIDGET_CLASS(parent_class)->unrealize)
    (*GTK_WIDGET_CLASS(parent_class)->unrealize) (widget);
}

static void
gtk_scrollpane_draw(GtkWidget * widget, GdkRectangle * area)
{
  GtkScrollpane      *sp;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(widget));
  g_return_if_fail(area != NULL);

  if (GTK_WIDGET_DRAWABLE(widget))
    {
      sp = GTK_SCROLLPANE(widget);
      gtk_scrollpane_draw_background(sp);
      gtk_scrollpane_draw_trough(sp);
      gtk_scrollpane_draw_slider(sp);
    }
}

static void
gtk_scrollpane_draw_background(GtkScrollpane * sp)
{
  g_return_if_fail(sp != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(sp));

  if (sp->trough && SCROLLPANE_CLASS(sp)->draw_background)
    (*SCROLLPANE_CLASS(sp)->draw_background) (sp);
}

static void
gtk_scrollpane_draw_trough(GtkScrollpane * sp)
{
  g_return_if_fail(sp != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(sp));

  if (sp->trough && SCROLLPANE_CLASS(sp)->draw_trough)
    (*SCROLLPANE_CLASS(sp)->draw_trough) (sp);
}

static void
gtk_scrollpane_draw_slider(GtkScrollpane * sp)
{
  g_return_if_fail(sp != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(sp));

  if (sp->trough && SCROLLPANE_CLASS(sp)->draw_slider)
    (*SCROLLPANE_CLASS(sp)->draw_slider) (sp);
}

static void
gtk_real_scrollpane_draw_trough(GtkScrollpane * sp)
{
  g_return_if_fail(sp != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(sp));

  if (sp->trough)
    {
            gtk_paint_box (GTK_WIDGET (sp)->style, sp->trough,
                           GTK_STATE_ACTIVE, GTK_SHADOW_IN,
                           NULL, GTK_WIDGET(sp), "trough",
                           0, 0, -1, -1);
            if (GTK_WIDGET_HAS_FOCUS (sp))
                    gtk_paint_focus (GTK_WIDGET (sp)->style,
                                     sp->trough,
                                     NULL, GTK_WIDGET(sp), "trough",
                                     0, 0, -1, -1);
    }
}

static void
gtk_real_scrollpane_draw_slider(GtkScrollpane * sp)
{
  GtkStateType        state_type;

  g_return_if_fail(sp != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(sp));

  if (sp->slider)
    {
      if (sp->in_slider || sp->moving)
	state_type = GTK_STATE_PRELIGHT;              
      else
        state_type = GTK_STATE_NORMAL;

      gtk_paint_box (GTK_WIDGET (sp)->style, sp->slider,
		     state_type, GTK_SHADOW_OUT,
		     NULL, GTK_WIDGET (sp), "slider",
		     0, 0, -1, -1);
    }
}

static void
gtk_scrollpane_size_request(GtkWidget * widget, GtkRequisition * requisition)
{
  GtkScrollpane *sp = GTK_SCROLLPANE(widget);

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(widget));
  g_return_if_fail(requisition != NULL);

  if(sp->trough_width <= 0)
          sp->trough_width = SCROLLPANE_DEFAULT_SIZE;
  if(sp->trough_height <= 0)
          sp->trough_height = SCROLLPANE_DEFAULT_SIZE;

  requisition->width = 
          widget->style->klass->xthickness*2 +
          sp->trough_width;

  requisition->height =
          widget->style->klass->ythickness*2 +
          sp->trough_height;
}

static void
gtk_scrollpane_size_allocate(GtkWidget * widget, GtkAllocation *allocation)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(widget));
  g_return_if_fail(allocation != NULL);

  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED(widget))
    gtk_scrollpane_update_slider_size(GTK_SCROLLPANE(widget));
}

static void
gtk_scrollpane_update_slider_size(GtkScrollpane * sp)
{
  gint                x, y, width, height;
  gint                x_offset, y_offset;
  GtkWidget          *widget;

  g_return_if_fail(sp != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(sp));

  widget = GTK_WIDGET(sp);
  x = widget->allocation.x;
  y = widget->allocation.y;
  width = widget->allocation.width;
  height = widget->allocation.height;
  width -= GTK_WIDGET(widget)->style->klass->xthickness*2;
  height -= GTK_WIDGET(widget)->style->klass->ythickness*2;

  sp->trough_width = width;
  sp->trough_height = height;
  /* we calculate the horizontal offset */
  /* we also do some sanity checking to guarentee that nothing weird has been passed in */
  /* (borrowed in spirit from gtk?scrollbar) */
  x_offset = widget->style->klass->xthickness;
  sp->slider_width = width;
  if (sp->Xadjustment)
    {
      if ((sp->Xadjustment->page_size > 0) && (sp->Xadjustment->lower < sp->Xadjustment->upper))
	{
	  if (sp->Xadjustment->page_size < (sp->Xadjustment->upper - sp->Xadjustment->lower))
	    {
	      /* calculate the X offset... */
	      x_offset += ((gfloat) (width))
                      * ((sp->Xadjustment->value - sp->Xadjustment->page_size / 2 - sp->Xadjustment->lower) /
                         (sp->Xadjustment->upper - sp->Xadjustment->lower));

	      /* scale the width appropriately */
	      sp->slider_width = 1 + ((gfloat) width)*sp->Xadjustment->page_size/(sp->Xadjustment->upper - sp->Xadjustment->lower);
	      if (sp->slider_width < sp->min_slider_width)
		sp->slider_width = sp->min_slider_width;
	      if (sp->slider_width > width)
		{
		  /* this takes care of rounding errors i think.  occasionally, there's a 1 pixel error */
		  x_offset = widget->style->klass->xthickness;
		  sp->slider_width = width;
		}
	    }
	}
    }

  y_offset = widget->style->klass->ythickness;
  sp->slider_height = height;
  if (sp->Yadjustment)
    {
      if ((sp->Yadjustment->page_size > 0) && (sp->Yadjustment->lower < sp->Yadjustment->upper))
	{
	  if (sp->Yadjustment->page_size < (sp->Yadjustment->upper - sp->Yadjustment->lower))
	    {
	      /* calculate the Y offset... */
	      y_offset += ((gfloat) (height))
                      * ((sp->Yadjustment->value - sp->Yadjustment->page_size / 2 - sp->Yadjustment->lower) /
                         (sp->Yadjustment->upper - sp->Yadjustment->lower));

	      /* scale the height appropriately */
	      sp->slider_height = 1 + ((gfloat) height)*sp->Yadjustment->page_size/(sp->Yadjustment->upper - sp->Yadjustment->lower);
	      if (height < sp->min_slider_height)
		sp->slider_height = sp->min_slider_height;
	      if (sp->slider_height > height)
		{
		  /* as per above... */
		  y_offset = widget->style->klass->ythickness;
		  sp->slider_height = height;
		}
	    }
	}
    }

  /* FIXME: This makes the scrollpane grow indefinitely */
  if (widget->window)
          gdk_window_move_resize(widget->window,
                                 x, y,
                                 widget->allocation.width, widget->allocation.height);

  if (sp->slider)
          gdk_window_move_resize(sp->slider,
                                 x_offset, y_offset,
                                 sp->slider_width, sp->slider_height);
}

static void
gtk_scrollpane_style_set(GtkWidget * widget, GtkStyle * previous_style)
{
  GtkScrollpane      *sp;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_SCROLLPANE(widget));

  sp = GTK_SCROLLPANE(widget);

  if (GTK_WIDGET_REALIZED(widget) && !GTK_WIDGET_NO_WINDOW(widget))
    if (sp->trough)
      {
	gtk_style_set_background(widget->style,
				 sp->trough,
				 GTK_STATE_ACTIVE);
	if (GTK_WIDGET_DRAWABLE(widget))
	  gdk_window_clear(sp->trough);
      }
}

/* Events... */
static gint
gtk_scrollpane_expose(GtkWidget * widget, GdkEventExpose * event)
{
  GtkScrollpane      *sp;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_SCROLLPANE(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  sp = GTK_SCROLLPANE(widget);

  /* where did the expose occur??? */
  /* if it did in the trough, then... */
  if (event->window == sp->trough) {
          gtk_scrollpane_draw_trough(sp);
  }
  else if (event->window == widget->window) {
          gtk_scrollpane_draw_background(sp);
  }
  else if (event->window == sp->slider) {
          gtk_scrollpane_draw_slider(sp);
  }
  return FALSE;
}

static gint
gtk_scrollpane_button_press(GtkWidget * widget,
			    GdkEventButton * event)
{
  GtkScrollpane      *sp;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_SCROLLPANE(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  sp = GTK_SCROLLPANE(widget);
  if (!GTK_WIDGET_HAS_FOCUS(widget))
    gdk_pointer_grab(widget->window, FALSE,
		     GDK_BUTTON1_MOTION_MASK
		     | GDK_BUTTON_RELEASE_MASK,
		     NULL, NULL, event->time);
  if (event->button == 1)
    {
      if (event->window == sp->slider)
	{
	  sp->x_offset = event->x;
	  sp->y_offset = event->y;
	  sp->moving = TRUE;
	}
    }

  return TRUE;
}

static gint
gtk_scrollpane_button_release(GtkWidget * widget,
			      GdkEventButton * event)
{
  GtkScrollpane      *sp;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_SCROLLPANE(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  sp = GTK_SCROLLPANE(widget);
  gdk_pointer_ungrab(event->time);

  if (event->button == 1) {
    sp->moving = FALSE;
  }
  else if (event->button == 2)
    gtk_signal_emit(GTK_OBJECT(sp), scrollpane_signals[MIDDLE_CLICKED]);
  else if (event->button == 3)
    gtk_signal_emit(GTK_OBJECT(sp), scrollpane_signals[RIGHT_CLICKED]);

  return TRUE;
}

GtkWidget *
gtk_scrollpane_new(GtkAdjustment * Xadjustment,
		   GtkAdjustment * Yadjustment,
		   gint aspect_ratio)
{
  GtkScrollpane      *retval = gtk_type_new(gtk_scrollpane_get_type());

  retval->Xadjustment = Xadjustment;
  retval->Yadjustment = Yadjustment;
  retval->aspect_ratio = aspect_ratio;

  gtk_signal_connect(GTK_OBJECT(Xadjustment), "changed",
		     (GtkSignalFunc) gtk_scrollpane_adjustment_changed,
		     (gpointer) retval);
  gtk_signal_connect(GTK_OBJECT(Yadjustment), "changed",
		     (GtkSignalFunc) gtk_scrollpane_adjustment_changed,
		     (gpointer) retval);
  gtk_signal_connect(GTK_OBJECT(Xadjustment), "value_changed",
		     (GtkSignalFunc) gtk_scrollpane_value_adjustment_changed,
		     (gpointer) retval);
  gtk_signal_connect(GTK_OBJECT(Yadjustment), "value_changed",
		     (GtkSignalFunc) gtk_scrollpane_value_adjustment_changed,
		     (gpointer) retval);

  return GTK_WIDGET(retval);
}

gboolean 
gtk_scrollpane_goto_edge(GtkScrollpane * sp, gint vertical, gint horizontal)
{
  g_return_val_if_fail(sp != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_SCROLLPANE(sp), FALSE);

  switch (vertical) {
  case GTK_SCROLLPANE_GOTOEDGE_LOWER:
    sp->Yadjustment->value = sp->Yadjustment->lower + sp->Yadjustment->page_size / 2;
    gtk_adjustment_value_changed(sp->Yadjustment);
    break;
  case GTK_SCROLLPANE_GOTOEDGE_UPPER:
    sp->Yadjustment->value = sp->Yadjustment->upper - sp->Yadjustment->page_size / 2;
    break;
  case GTK_SCROLLPANE_GOTOEDGE_NONE:
    break;
  default:
    g_assert(0); /* Illegal parameter error */
  }
  if (vertical != GTK_SCROLLPANE_GOTOEDGE_NONE)
    gtk_adjustment_value_changed(sp->Yadjustment);


  switch (vertical) {
  case GTK_SCROLLPANE_GOTOEDGE_LOWER:
    sp->Xadjustment->value = sp->Xadjustment->lower + sp->Xadjustment->page_size / 2;
    gtk_adjustment_value_changed(sp->Xadjustment);
    break;
  case GTK_SCROLLPANE_GOTOEDGE_UPPER:
    sp->Xadjustment->value = sp->Xadjustment->upper - sp->Xadjustment->page_size / 2;
    break;
  case GTK_SCROLLPANE_GOTOEDGE_NONE:
    break;
  default:
    g_assert(0); /* Illegal parameter error */
  }
  if (vertical != GTK_SCROLLPANE_GOTOEDGE_NONE)
    gtk_adjustment_value_changed(sp->Xadjustment);

  return TRUE;
}


gboolean 
gtk_scrollpane_step(GtkScrollpane * sp, gint direction, gboolean dowrap)
{
  GtkAdjustment *MainAdj; /* We will move this adjustment */
  GtkAdjustment *SecoAdj; /* And this _only_ if we can't move MainAdj (ie. we're edge)
			     and there is wrapping */

  gboolean MoveHorizontal = TRUE; /* Positive if we move horizontal */
  gboolean DirectionFlag = TRUE;  /* Positive if we move towards upper */
  g_return_val_if_fail(sp != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_SCROLLPANE(sp), FALSE);


#define CHECK_THERE_IS_NO_LOWER_SPACE(adj) ((adj)->value - ((adj)->page_size / 2 + EPSILON) <= (adj)->lower)
#define CHECK_THERE_IS_NO_UPPER_SPACE(adj) ((adj)->value + ((adj)->page_size / 2 + EPSILON) >= (adj)->upper)

#define CHECK_THERE_IS_NO_SPACE_FOR_STEP(adj,dir) (dir?CHECK_THERE_IS_NO_UPPER_SPACE(adj):\
						   CHECK_THERE_IS_NO_LOWER_SPACE(adj))

  /* To make code more readable, we make a macro */

#define ADVANCE_TOWARDS_LOWER(adj)         (adj->value -= 0.75 * (adj->page_size))
#define ADVANCE_TOWARDS_UPPER(adj)         (adj->value += 0.75 * (adj->page_size))

#define ADVANCE_STEP(adj,dir)      (dir?ADVANCE_TOWARDS_UPPER(adj):ADVANCE_TOWARDS_LOWER(adj))

#define MOVE_TO_LOWER_EDGE(adj)    (adj->value = adj->lower + adj->page_size / 2)
#define MOVE_TO_UPPER_EDGE(adj)    (adj->value = adj->upper - adj->page_size / 2)

    /* if upper is 1 goto upper, otherwise to lower */

#define MOVE_TO_EDGE(adj,upper) (upper?MOVE_TO_UPPER_EDGE(adj):MOVE_TO_LOWER_EDGE(adj))

  /* These variables make our life easier */

  switch (direction) {
  case GTK_SCROLLPANE_SCROLL_RIGHT:
    MoveHorizontal = TRUE;
    DirectionFlag = TRUE;
    break;
  case GTK_SCROLLPANE_SCROLL_LEFT:
    MoveHorizontal = TRUE;
    DirectionFlag = FALSE;
    break;
  case GTK_SCROLLPANE_SCROLL_DOWN:
    MoveHorizontal = FALSE;
    DirectionFlag = TRUE;
    break;
  case GTK_SCROLLPANE_SCROLL_UP:
    MoveHorizontal = FALSE;
    DirectionFlag = FALSE;
    break;
  default:
    g_assert(0); /* Illegal parameter error */
  }
    
  if (MoveHorizontal) {
    MainAdj = sp->Xadjustment;
    SecoAdj = sp->Yadjustment;
  }
  else {
    MainAdj = sp->Yadjustment;
    SecoAdj = sp->Xadjustment;
  }

  if (CHECK_THERE_IS_NO_SPACE_FOR_STEP(MainAdj, DirectionFlag)) {
    /* g_print("There was no space to move\n"); */
    if (!dowrap)
      return FALSE;

    /* g_print("There was no space to move, but we do wrapping\n"); */
    /* We do wrapping */

    /* Move in the oposite axis */
    if (CHECK_THERE_IS_NO_SPACE_FOR_STEP(SecoAdj, DirectionFlag)) {
      /* there is no place to move, we need a new page */
      return FALSE;
    }
    ADVANCE_STEP(SecoAdj,DirectionFlag);
    
    if (CHECK_THERE_IS_NO_SPACE_FOR_STEP(SecoAdj,DirectionFlag)) {
      /* We move it too far, lets move it to the edge */
      MOVE_TO_EDGE(SecoAdj,DirectionFlag);
    }
    /* now move to edge (other axis) in oposite direction */
    MOVE_TO_EDGE(MainAdj,!DirectionFlag);
    gtk_adjustment_value_changed(SecoAdj);
    return TRUE;
  }
  
  /* Now we know we can move in the direction sought */  
  ADVANCE_STEP(MainAdj,DirectionFlag);
  
  if (CHECK_THERE_IS_NO_SPACE_FOR_STEP(MainAdj,DirectionFlag)) {
    /* We move it too far, lets move it to the edge */
    MOVE_TO_EDGE(MainAdj,DirectionFlag);
  }
  gtk_adjustment_value_changed(MainAdj);
  
  return TRUE;

}


static void
gtk_scrollpane_adjustment_changed(GtkAdjustment * adjustment, gpointer data)
{
  /*
   * we don't want to update if we caused the value change ourselves -- we just hope that
   * no one tries to update while we are updating ourselves...
   */
  if (GTK_SCROLLPANE(data)->moving)
    return;
  gtk_scrollpane_update_slider_size(GTK_SCROLLPANE(data));
}

static void
gtk_scrollpane_value_adjustment_changed(GtkAdjustment * adjustment, gpointer data)
{
  /*
   * we don't want to update if we caused the value change ourselves -- we just hope that
   * no one tries to update while we are updating ourselves...
   */
  if (GTK_SCROLLPANE(data)->moving)
    return;
  gtk_scrollpane_update_slider_size(GTK_SCROLLPANE(data));
}

static gint
gtk_scrollpane_key_press(GtkWidget * widget, GdkEventKey * event)
{
  return TRUE;
}

static gint
gtk_scrollpane_enter_notify(GtkWidget * widget, GdkEventCrossing * event)
{
  GtkScrollpane *pane = GTK_SCROLLPANE(widget);

  if(event->window == pane->slider)
    {
      pane->in_slider = TRUE;
      gtk_scrollpane_draw_slider(pane);
    }

  return FALSE;
}

static gint
gtk_scrollpane_leave_notify(GtkWidget * widget, GdkEventCrossing * event)
{
  GtkScrollpane *pane = GTK_SCROLLPANE(widget);

  if(event->window == pane->slider) {
    pane->in_slider = FALSE;
    gtk_scrollpane_draw_slider(pane);
  }
  return FALSE;
}

static gint
gtk_scrollpane_motion_notify(GtkWidget * widget, GdkEventMotion * event)
{
  GtkScrollpane      *sp;
  gint                x, y, width, height, depth;
  gfloat              value;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_SCROLLPANE(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  sp = GTK_SCROLLPANE(widget);

  if (sp->moving)
    {
      gdk_window_get_geometry(sp->slider, &x, &y, &width, &height, &depth);

      if (event->window == sp->trough)
	{
	  if (width + event->x - sp->x_offset > sp->trough_width + widget->style->klass->xthickness)
	    x = sp->trough_width + widget->style->klass->xthickness - width;
	  else if (event->x - sp->x_offset > widget->style->klass->xthickness)
	    x = event->x - sp->x_offset;
	  else
	    x = widget->style->klass->xthickness;

	  if (height + event->y - sp->y_offset > sp->trough_height + widget->style->klass->ythickness)
	    y = sp->trough_height + widget->style->klass->ythickness - height;
	  else if (event->y - sp->y_offset > widget->style->klass->ythickness)
	    y = event->y - sp->y_offset;
	  else
	    y = widget->style->klass->ythickness;
	  gdk_window_move(sp->slider, x, y);
	}
      else if (event->window == sp->slider)
	{
	  if (x + width + event->x - sp->x_offset > sp->trough_width + widget->style->klass->xthickness)
	    x = sp->trough_width + widget->style->klass->xthickness - width;
	  else if (x + event->x - sp->x_offset > widget->style->klass->xthickness)
	    x += event->x - sp->x_offset;
	  else
	    x = widget->style->klass->xthickness;

	  if (y + height + event->y - sp->y_offset > sp->trough_height + widget->style->klass->ythickness)
	    y = sp->trough_height + widget->style->klass->ythickness - height;
	  else if (y + event->y - sp->y_offset > widget->style->klass->ythickness)
	    y += event->y - sp->y_offset;
	  else
	    y = widget->style->klass->ythickness;

	  gdk_window_move(sp->slider, x, y);
	}

      x -= widget->style->klass->xthickness;
      y -= widget->style->klass->ythickness;

      /* first we check the x adjustment... */
      value = ((sp->Xadjustment->upper - sp->Xadjustment->lower) *
	       (x + ((gfloat) width) / 2) /
	       (sp->trough_width));
      if (value + sp->Xadjustment->page_size / 2 < sp->Xadjustment->upper)
	gtk_adjustment_set_value(sp->Xadjustment, value);
      else
	/*ugh -- yucky hack. to guarentee that value + page_size/2 < upper */
	gtk_adjustment_set_value(sp->Xadjustment, sp->Xadjustment->upper - sp->Xadjustment->page_size / 2 - 0.0001);

      /* then the y adjustment... */
      value = ((sp->Yadjustment->upper - sp->Yadjustment->lower) *
	       (y + ((gfloat) height) / 2) /
	       (sp->trough_height));
      if (value + sp->Yadjustment->page_size / 2 < sp->Yadjustment->upper)
	gtk_adjustment_set_value(sp->Yadjustment, value);
      else
	/*ugh -- yucky hack. to guarentee that value + page_size/2 < upper */
	gtk_adjustment_set_value(sp->Yadjustment, sp->Yadjustment->upper - sp->Yadjustment->page_size / 2 - 0.0001);
    }

  return TRUE;
}

static gint
gtk_scrollpane_focus_in(GtkWidget * widget, GdkEventFocus * event)
{
  return TRUE;
}

static gint
gtk_scrollpane_focus_out(GtkWidget * widget, GdkEventFocus * event)
{
  return TRUE;
}
