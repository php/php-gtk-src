/* GtkComboButton - A ComboButton for GTK
 * Copyright (C) 1999 Iain Holmes
 * 
 * <iain@webtribe.net>
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

#include <gtk/gtksignal.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkpixmap.h>
#include <gtk/gtkmenuitem.h>

#include "gtkcombobutton.h"

#define CHILD_SPACING 2
#define DEFAULT_LEFT_POS 4
#define DEFAULT_TOP_POS 4
#define DEFAULT_SPACING 7
#define COMBOARROW_SIZE 14
#define XOFFSET -4
#define YOFFSET -2

enum {
  MENU_PRESSED,
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_X_OFFSET,
  ARG_Y_OFFSET,
  ARG_ARROW_DIRECTION
};

enum {
  GTK_COMBOBUTTON_PIXMAP,
  GTK_COMBOBUTTON_LABEL
};

static guint combobutton_signals[LAST_SIGNAL] = { 0 };
static GtkButtonClass *parent_class = NULL;

static void gtk_combobutton_class_init (GtkComboButtonClass *klass);
static void gtk_combobutton_init (GtkComboButton *combobutton);
static void gtk_combobutton_set_arg (GtkObject *object,
				     GtkArg *arg,
				     guint arg_id);
static void gtk_combobutton_get_arg (GtkObject *object,
				     GtkArg *arg,
				     guint arg_id);
static void gtk_combobutton_destroy (GtkObject *object);
static void gtk_combobutton_size_request (GtkWidget *widget,
					  GtkRequisition *requisition);
static void gtk_combobutton_size_allocate (GtkWidget *widget,
					   GtkAllocation *allocation);
static void gtk_combobutton_draw (GtkWidget *widget,
				  GdkRectangle *area);
static gint gtk_combobutton_expose (GtkWidget *widget,
				    GdkEventExpose *event);
static gint gtk_combobutton_button_press (GtkWidget *widget,
					  GdkEventButton *event);
static gint gtk_combobutton_button_release (GtkWidget *widget,
					    GdkEventButton *event);
static void gtk_combobutton_clicked (GtkComboButton *combobutton,
				     gpointer data);
static void gtk_combobutton_menu_position (GtkMenu *menu,
                                           gint *x,
                                           gint *y,
                                           gpointer cb_menu);

GtkType
gtk_combobutton_get_type (void)
{
  static GtkType combobutton_type = 0;

  if (!combobutton_type)
    {
      static const GtkTypeInfo combobutton_info =
      {
	"GtkComboButton",
	sizeof (GtkComboButton),
	sizeof (GtkComboButtonClass),
	(GtkClassInitFunc) gtk_combobutton_class_init,
	(GtkObjectInitFunc) gtk_combobutton_init,
	NULL,
	NULL,
	(GtkClassInitFunc) NULL
      };

      combobutton_type = gtk_type_unique (GTK_TYPE_BUTTON, &combobutton_info);
    }

  return combobutton_type;
}

static void
gtk_combobutton_class_init (GtkComboButtonClass *klass)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) klass;
  widget_class = (GtkWidgetClass*) klass;

  parent_class = gtk_type_class (GTK_TYPE_BUTTON);

  gtk_object_add_arg_type ("GtkComboButton::xoffset", GTK_TYPE_INT, GTK_ARG_READWRITE, ARG_X_OFFSET);
  gtk_object_add_arg_type ("GtkComboButton::yoffset", GTK_TYPE_INT, GTK_ARG_READWRITE, ARG_Y_OFFSET);
  gtk_object_add_arg_type ("GtkComboButton::arrowdirection", GTK_TYPE_ENUM, GTK_ARG_READWRITE, ARG_ARROW_DIRECTION);

  combobutton_signals[MENU_PRESSED] = 
    gtk_signal_new ("menu_pressed",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GtkComboButtonClass, menu_pressed),
		    gtk_marshal_NONE__NONE,
		    GTK_TYPE_NONE, 0);

  gtk_object_class_add_signals (object_class, combobutton_signals, LAST_SIGNAL);

  object_class->set_arg = gtk_combobutton_set_arg;
  object_class->get_arg = gtk_combobutton_get_arg;
  object_class->destroy = gtk_combobutton_destroy;

  widget_class->draw = gtk_combobutton_draw;
  widget_class->size_request = gtk_combobutton_size_request;
  widget_class->size_allocate = gtk_combobutton_size_allocate;
  widget_class->expose_event = gtk_combobutton_expose;
  widget_class->button_press_event = gtk_combobutton_button_press;

  klass->menu_pressed = NULL;
}

static void
gtk_combobutton_init (GtkComboButton *combobutton)
{
  GTK_WIDGET_SET_FLAGS (combobutton, GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT);
  GTK_WIDGET_UNSET_FLAGS (combobutton, GTK_NO_WINDOW);

  GTK_BUTTON (combobutton)->child = NULL;
  GTK_BUTTON (combobutton)->in_button = FALSE;
  GTK_BUTTON (combobutton)->button_down = FALSE;
  GTK_BUTTON (combobutton)->relief = GTK_RELIEF_NORMAL;

  combobutton->menu = NULL;
  combobutton->x_offset = XOFFSET;
  combobutton->y_offset = YOFFSET;
  combobutton->arrowdir = GTK_ARROW_DOWN;
  combobutton->changing = FALSE;
}

static void
gtk_combobutton_set_arg (GtkObject *object,
			 GtkArg *arg,
			 guint arg_id)
{
  GtkComboButton *combobutton;

  combobutton = GTK_COMBOBUTTON (object);

  switch (arg_id)
    {
    case ARG_X_OFFSET:
      combobutton->x_offset = GTK_VALUE_INT (*arg);
      break;
    case ARG_Y_OFFSET:
      combobutton->y_offset = GTK_VALUE_INT (*arg);
      break;
    case ARG_ARROW_DIRECTION:
      combobutton->arrowdir = GTK_VALUE_ENUM (*arg);
      gtk_widget_draw (GTK_WIDGET (combobutton), NULL);
      break;
    default:
      break;
    }
}

static void
gtk_combobutton_get_arg (GtkObject *object,
			 GtkArg *arg,
			 guint arg_id)
{
  GtkComboButton *combobutton;

  combobutton = GTK_COMBOBUTTON (object);

  switch (arg_id)
    {
    case ARG_X_OFFSET:
      GTK_VALUE_INT (*arg) = combobutton->x_offset;
      break;
    case ARG_Y_OFFSET:
      GTK_VALUE_INT (*arg) = combobutton->y_offset;
      break;
    case ARG_ARROW_DIRECTION:
      GTK_VALUE_ENUM (*arg) = combobutton->arrowdir;
      break;
    default:
      break;
    }
}

/**
 * gtk_combobutton_new:
 * 
 * Creates a new Combobutton widget. 
 * The default direction for the arrow is down.
 *
 * Returns: A pointer to the newly allocated #GtkComboButton.
 */
GtkWidget*
gtk_combobutton_new (void)
{
  return GTK_WIDGET (gtk_type_new (gtk_combobutton_get_type ()));
}

/**
 * gtk_combobutton_new_with_label:
 * @label:  The label that will appear in the #GtkComboButton. 
 * The default direction for the arrow is down.
 *
 * Creates a new #GtkComboButton widget with the text specified in label.
 *
 * Returns: A pointer to the newly allocated #GtkComboButton.
 */
GtkWidget*
gtk_combobutton_new_with_label (const gchar *label)
{
  GtkWidget *combobutton;
  GtkWidget *label_widget;

  combobutton = gtk_combobutton_new ();
  label_widget = gtk_label_new (label);
  gtk_misc_set_alignment (GTK_MISC (label_widget), 0.5, 0.5);

  gtk_container_add (GTK_CONTAINER (combobutton), label_widget);
  gtk_widget_show (label_widget);

  return combobutton;
}

GtkWidget*
gtk_combobutton_copy_item (GtkWidget *widget,
			   gint type)
{
  GtkWidget *clone;

  switch (type)
    {
    case GTK_COMBOBUTTON_PIXMAP:
      {
	GdkPixmap *val;
	GdkBitmap *mask;

	g_return_val_if_fail (GTK_IS_PIXMAP (widget), NULL);
	gtk_pixmap_get (GTK_PIXMAP (widget), &val, &mask);
	clone = gtk_pixmap_new (val, mask);
	break;
      }
    case GTK_COMBOBUTTON_LABEL:
      {
	gchar *str[512];

	g_return_val_if_fail (GTK_IS_LABEL (widget), NULL);
	gtk_label_get (GTK_LABEL (widget), str);
	clone = gtk_label_new (*str);
	break;
      }
    default:
      clone = NULL;
    }

  return clone;
}

static void
gtk_combobutton_item_verifier (GtkWidget *widget,
			       gpointer data)
{
  GtkWidget *content;

  content = GTK_BIN (widget)->child;

  if (!( GTK_IS_PIXMAP (content) || GTK_IS_LABEL (content)))
    (*(gint*)data)++;
}

static void
gtk_combobutton_swapper (GtkWidget *widget,
			 GtkComboButton *combobutton)
{
  GtkWidget *content;

  content = GTK_BIN (widget)->child;
  
  g_return_if_fail (content != NULL);

  if (GTK_IS_PIXMAP (content))
    {
      GtkWidget *newpix;
      
      newpix = gtk_combobutton_copy_item (content, GTK_COMBOBUTTON_PIXMAP);
      gtk_widget_show (newpix);
      gtk_container_remove (GTK_CONTAINER (combobutton), 
			    GTK_BIN (combobutton)->child);
      gtk_container_add (GTK_CONTAINER (combobutton), newpix);
    }
  else
    if (GTK_IS_LABEL (content))
      {
	GtkWidget *newlab;

	newlab = gtk_combobutton_copy_item (content, GTK_COMBOBUTTON_LABEL);
	gtk_widget_show (newlab);
	gtk_container_remove (GTK_CONTAINER (combobutton),
			      GTK_BIN (combobutton)->child);
	gtk_container_add (GTK_CONTAINER (combobutton), newlab);
      }
}

static void
gtk_combobutton_install_swapper (GtkWidget *item,
				 GtkComboButton *combobutton)
{
  gtk_signal_connect (GTK_OBJECT (item), "activate",
		      GTK_SIGNAL_FUNC (gtk_combobutton_swapper),
		      combobutton);
}

/**
 * gtk_combobutton_new_with_menu:
 * @menu: The menu that will appear when the arrow is pressed.
 * @changing: A #gboolean to determine whether the #GtkComboButton content changes when you select a menuitem.
 *
 * Creates a new #GtkComboButton widget with the menu. The image or text in the #GtkComboButton can either be set to change as you select menuitems.
 * Menuitems can only contain labels OR pixmaps, not both or anything else. If you want to use combinations of both then use gtk_combobutton_new(), and gtk_combobutton_set_menu().
 *
 * Returns: A pointer to the newly allocated #GtkComboButton.
 */
GtkWidget*
gtk_combobutton_new_with_menu (GtkWidget *menu,
			       gboolean changing)
{
  GtkWidget *combobutton;
  GtkWidget *menuitem;
  GtkWidget *content;
  GtkWidget *content_clone;
  gint result;

  g_return_val_if_fail (menu != NULL, NULL);
  g_return_val_if_fail (GTK_IS_MENU (menu), NULL);

  combobutton = gtk_combobutton_new ();
  GTK_COMBOBUTTON (combobutton)->changing = changing;

  result = 0;
  gtk_container_foreach (GTK_CONTAINER (menu),
			 gtk_combobutton_item_verifier,
			 &result);
  g_return_val_if_fail (result == 0, NULL);

  menuitem = gtk_menu_get_active (GTK_MENU (menu));
  g_return_val_if_fail (menuitem != NULL, NULL);

  content = GTK_BIN (menuitem)->child;
  g_return_val_if_fail (content != NULL, NULL);
  g_return_val_if_fail ( GTK_IS_PIXMAP (content) || GTK_IS_LABEL (content), NULL);

  if (GTK_IS_PIXMAP (content))
    content_clone = gtk_combobutton_copy_item (content , GTK_COMBOBUTTON_PIXMAP);
  else
    if (GTK_IS_LABEL (content))
      content_clone = gtk_combobutton_copy_item (content, GTK_COMBOBUTTON_LABEL);

  gtk_container_add (GTK_CONTAINER (combobutton), content_clone);
  gtk_combobutton_set_menu (GTK_COMBOBUTTON (combobutton), menu);

  if (changing)
    {
      gtk_container_foreach (GTK_CONTAINER (menu),
			     (GtkCallback)gtk_combobutton_install_swapper,
			     combobutton);
      gtk_signal_connect (GTK_OBJECT (combobutton), "clicked",
			  GTK_SIGNAL_FUNC (gtk_combobutton_clicked), NULL);
    }

  return combobutton;
}

void
gtk_combobutton_menu_pressed (GtkComboButton *combobutton)
{
  g_return_if_fail (combobutton != NULL);
  g_return_if_fail (GTK_IS_COMBOBUTTON (combobutton));

  gtk_signal_emit (GTK_OBJECT (combobutton), combobutton_signals[MENU_PRESSED]);
}

static void
gtk_combobutton_destroy (GtkObject *object)
{
  GtkComboButton *button;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_COMBOBUTTON (object));

  button = GTK_COMBOBUTTON (object);

  if (button->menu)
    gtk_object_unref (GTK_OBJECT (button->menu));

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/**
 * gtk_combobutton_get_menu:
 * @combobutton:  A pointer to a #GtkComboButton.
 *
 * Retrieve the menu structure of this #GtkComboButton.
 *
 * Returns: A pointer to the menu of the #GtkComboButton or NULL if a menu has not been set.
 */
GtkWidget*
gtk_combobutton_get_menu (GtkComboButton *combobutton)
{
  g_return_val_if_fail (combobutton != NULL, NULL);
  g_return_val_if_fail (GTK_IS_COMBOBUTTON (combobutton), NULL);

  return combobutton->menu;
}

/**
 * gtk_combobutton_set_menu:
 * @combobutton: A pointer to a #GtkComboButton.
 * @combomenu: A pointer to a #GtkWidget menu.
 *
 * Set the menu associated with the #GtkComboButton in @combobutton.
 */
void 
gtk_combobutton_set_menu (GtkComboButton *combobutton,
			  GtkWidget *combomenu)
{
  g_return_if_fail (combobutton != NULL);
  g_return_if_fail (GTK_IS_COMBOBUTTON (combobutton));
  g_return_if_fail (GTK_IS_MENU (combomenu));

  if (combobutton->menu)
    {
      gtk_object_unref (GTK_OBJECT (combobutton->menu));
    }

  combobutton->menu = combomenu;
  gtk_object_ref (GTK_OBJECT (combobutton->menu));

}

/**
 * gtk_combobutton_set_arrow_direction:
 * @combobutton: A pointer to a #GtkComboButton.
 * @arrowtype: A #GtkArrowType signifying the direction of the arrow.
 *
 * Sets the direction of the arrow in the #GtkcomboButton.
 */
void 
gtk_combobutton_set_arrow_direction (GtkComboButton *combobutton,
				     GtkArrowType arrowtype)
{
  g_return_if_fail (combobutton != NULL);
  g_return_if_fail (GTK_IS_COMBOBUTTON (combobutton));

  gtk_object_set (GTK_OBJECT (combobutton),
		  "GtkComboButton::arrowdirection", (GtkArrowType)arrowtype, 
		  NULL);
}

/**
 * gtk_combobutton_get_arrow_direction:
 * @combobutton: A pointer to a #GtkComboButton.
 *
 * Retrieves the direction of the arrow in the #GtkComboButton.
 *
 * Returns: A #GtkArrowType.
 */
GtkArrowType
gtk_combobutton_get_arrow_direction (GtkComboButton *combobutton)
{
  GtkArg args[1];

  g_return_val_if_fail (combobutton != NULL, 0);
  g_return_val_if_fail (GTK_IS_COMBOBUTTON (combobutton), 0);

  args[0].name = "GtkComboButton::arrowdirection";
  gtk_object_getv (GTK_OBJECT (combobutton),
		   1,
		   args);

  g_assert (GTK_FUNDAMENTAL_TYPE (args[0].type) == GTK_TYPE_ENUM);

  return GTK_VALUE_ENUM (args[0]);
}

static void
gtk_combobutton_size_request (GtkWidget *widget,
			      GtkRequisition *requisition)
{
  GtkComboButton *combobutton;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_COMBOBUTTON (widget));
  g_return_if_fail (requisition != NULL);

  combobutton = GTK_COMBOBUTTON (widget);

  requisition->width = ((GTK_CONTAINER (widget)->border_width + CHILD_SPACING +
			 GTK_WIDGET (widget)->style->klass->xthickness) * 2 +
			(GTK_CONTAINER (widget)->border_width + (CHILD_SPACING * 5)) * 2 +
			COMBOARROW_SIZE);
  requisition->height = (GTK_CONTAINER (widget)->border_width + CHILD_SPACING +
			 GTK_WIDGET (widget)->style->klass->ythickness) * 2;
			 
  if (GTK_WIDGET_CAN_DEFAULT (widget))
    {
      requisition->width += (GTK_WIDGET (widget)->style->klass->xthickness * 2 +
			     DEFAULT_SPACING);
      requisition->height += (GTK_WIDGET (widget)->style->klass->ythickness * 2 +
			      DEFAULT_SPACING);
    }

  if (GTK_BIN (combobutton)->child && GTK_WIDGET_VISIBLE (GTK_BIN (combobutton)->child))
    {
      GtkRequisition child_requisition;
      
      gtk_widget_size_request (GTK_BIN (combobutton)->child, &child_requisition);
      
      requisition->width += child_requisition.width;
      requisition->height += child_requisition.height;
    }
}

static void
gtk_combobutton_size_allocate (GtkWidget *widget,
			       GtkAllocation *allocation)
{
  GtkComboButton *combobutton;
  GtkAllocation child_allocation;
  gint border_width;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_COMBOBUTTON (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  border_width = GTK_CONTAINER (widget)->border_width;

  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_move_resize (widget->window,
			    widget->allocation.x + border_width,
			    widget->allocation.y + border_width,
			    widget->allocation.width - border_width * 2,
			    widget->allocation.height - border_width * 2);

  combobutton = GTK_COMBOBUTTON (widget);

  if (GTK_BIN (combobutton)->child && 
      GTK_WIDGET_VISIBLE (GTK_BIN (combobutton)->child))
    {
      child_allocation.x = (CHILD_SPACING + GTK_WIDGET (widget)->style->klass->xthickness);
      child_allocation.y = (CHILD_SPACING + GTK_WIDGET (widget)->style->klass->ythickness);

      child_allocation.width = MAX (1, (gint)widget->allocation.width - child_allocation.x * 2 -
				    border_width * 2 -
				    (GTK_CONTAINER (widget)->border_width * 2 +
				    (CHILD_SPACING * 5) +
				    COMBOARROW_SIZE));
      child_allocation.height = MAX (1, (gint)widget->allocation.height - child_allocation.y * 2 -
				     border_width * 2);

      if (GTK_WIDGET_CAN_DEFAULT (combobutton))
	{
	  child_allocation.x += (GTK_WIDGET (widget)->style->klass->xthickness +
				 DEFAULT_LEFT_POS);
	  child_allocation.y += (GTK_WIDGET (widget)->style->klass->ythickness +
				 DEFAULT_TOP_POS);
	  child_allocation.width = MAX (1, (gint)child_allocation.width -
					(gint)(GTK_WIDGET (widget)->style->klass->xthickness * 2 + DEFAULT_SPACING));
	  child_allocation.height = MAX (1, (gint)child_allocation.height -
					 (gint)(GTK_WIDGET (widget)->style->klass->ythickness * 2 + DEFAULT_SPACING));
	}

      gtk_widget_size_allocate (GTK_BIN (combobutton)->child, &child_allocation);
    }
}

static void
gtk_combobutton_paint (GtkWidget *widget,
		       GdkRectangle *area)
{
  GtkComboButton *combobutton;
  GtkShadowType shadow_type;
  gint width;
  gint height;
  gint x;
  gint y;

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      combobutton = GTK_COMBOBUTTON (widget);

      x = 0;
      y = 0;
      width = widget->allocation.width - GTK_CONTAINER (widget)->border_width * 2;
      height = widget->allocation.height - GTK_CONTAINER (widget)->border_width * 2;

      gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
      gdk_window_clear_area (widget->window, area->x, area->y, area->width, area->height);

      if (GTK_WIDGET_HAS_DEFAULT (widget) &&
	  GTK_BUTTON (widget)->relief == GTK_RELIEF_NORMAL)
	{
	  gtk_paint_box (widget->style, widget->window,
			 GTK_STATE_NORMAL, GTK_SHADOW_IN,
			 area, widget, "buttondefault",
			 x, y, width, height);
	  gtk_paint_vline (widget->style, widget->window,
			   GTK_STATE_NORMAL,
			   area, widget, "buttondefault",
			   y+8, y + height - 8,
			   x + width - (GTK_CONTAINER (widget)->border_width + 
				(CHILD_SPACING * 6) + COMBOARROW_SIZE));
	  gtk_paint_arrow (widget->style, widget->window,
			   GTK_STATE_NORMAL, GTK_SHADOW_IN,
			   area, widget, "buttondefault",
			   combobutton->arrowdir, TRUE,
			   x + width - (GTK_CONTAINER (widget)->border_width +
					(CHILD_SPACING * 3) + COMBOARROW_SIZE),
			   y + ((gint)(height - COMBOARROW_SIZE)/2),
			   COMBOARROW_SIZE, COMBOARROW_SIZE);
	}

      if (GTK_BUTTON (widget)->relief == GTK_RELIEF_NONE)
	{
	  gtk_paint_arrow (widget->style, widget->window,
			   GTK_STATE_NORMAL, GTK_SHADOW_IN,
			   area, widget, "buttondefault",
			   combobutton->arrowdir, TRUE,
			   x + width - (GTK_CONTAINER (widget)->border_width +
					(CHILD_SPACING * 3) + COMBOARROW_SIZE),
			   y + ((gint)(height - COMBOARROW_SIZE)/2),
			   COMBOARROW_SIZE, COMBOARROW_SIZE); 
	}

      if (GTK_WIDGET_CAN_DEFAULT (widget))
	{
	  x += widget->style->klass->xthickness;
	  y += widget->style->klass->ythickness;
	  width -= 2 * x + DEFAULT_SPACING;
	  height -= 2 * y + DEFAULT_SPACING;
	  x += DEFAULT_LEFT_POS;
	  y += DEFAULT_TOP_POS;
	}

      if (GTK_WIDGET_HAS_FOCUS (widget))
	{
	  x += 1;
	  y += 1;
	  width -= 2;
	  height -= 2;
	}

      if (GTK_WIDGET_STATE (widget) == GTK_STATE_ACTIVE)
	shadow_type = GTK_SHADOW_IN;
      else
	shadow_type = GTK_SHADOW_OUT;

      if ((GTK_BUTTON (widget)->relief != GTK_RELIEF_NONE) ||
	  ((GTK_WIDGET_STATE (widget) != GTK_STATE_NORMAL) &&
	   (GTK_WIDGET_STATE (widget) != GTK_STATE_INSENSITIVE)))
	{
	  gtk_paint_box (widget->style, widget->window,
			 GTK_WIDGET_STATE (widget), shadow_type,
			 area, widget, "button",
			 x, y, width, height);
	  gtk_paint_vline (widget->style, widget->window,
			   GTK_WIDGET_STATE (widget),
			   area, widget, "button",
			   y+8, y + height - 8,
			   x + width - (GTK_CONTAINER (widget)->border_width + 
				(CHILD_SPACING * 6) + COMBOARROW_SIZE));
	  gtk_paint_arrow (widget->style, widget->window,
			   GTK_WIDGET_STATE (widget), shadow_type,
			   area, widget, "button",
			   combobutton->arrowdir, TRUE,
			   x + width - (GTK_CONTAINER (widget)->border_width +
					(CHILD_SPACING * 3) + COMBOARROW_SIZE),
			   y + ((gint)(height - COMBOARROW_SIZE)/2),
			   COMBOARROW_SIZE, COMBOARROW_SIZE);
	}

      if (GTK_WIDGET_HAS_FOCUS (widget))
	{
	  x -= 1;
	  y -= 1;
	  width += 2;
	  height += 2;

	  gtk_paint_focus (widget->style, widget->window,
			   area, widget, "button",
			   x, y, width - 1, height - 1);
	}
    }
}

static void
gtk_combobutton_draw (GtkWidget *widget,
		      GdkRectangle *area)
{
  GtkComboButton *combobutton;
  GdkRectangle child_area;
  GdkRectangle tmp_area;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_COMBOBUTTON (widget));
  g_return_if_fail (area != NULL);

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      combobutton = GTK_COMBOBUTTON (widget);

      tmp_area = *area;
      tmp_area.x -= GTK_CONTAINER (combobutton)->border_width;
      tmp_area.y -= GTK_CONTAINER (combobutton)->border_width;

      gtk_combobutton_paint (widget, &tmp_area);
      
      if (GTK_BIN (combobutton)->child && 
	  gtk_widget_intersect (GTK_BIN (combobutton)->child, &tmp_area, &child_area))
	gtk_widget_draw (GTK_BIN (combobutton)->child, &child_area);
    }
}

static gint
gtk_combobutton_expose (GtkWidget *widget,
			GdkEventExpose *event)
{
  GtkBin *bin;
  GdkEventExpose child_event;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_COMBOBUTTON (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      bin = GTK_BIN (widget);

      gtk_combobutton_paint (widget, &event->area);
      
      child_event = *event;
      if (bin->child && GTK_WIDGET_NO_WINDOW (bin->child) &&
	  gtk_widget_intersect (bin->child, &event->area, &child_event.area))
	gtk_widget_event (bin->child, (GdkEvent*) &child_event);
    }

  return FALSE;
}

static gint
gtk_combobutton_button_press (GtkWidget *widget,
			      GdkEventButton *event)
{
  GtkComboButton *combobutton;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_COMBOBUTTON (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->type == GDK_BUTTON_PRESS)
    {
      combobutton = GTK_COMBOBUTTON (widget);

      if (!GTK_WIDGET_HAS_FOCUS (widget))
	gtk_widget_grab_focus (widget);

      if (event->button == 1)
	{
	  gint x;
	  gint y;
	  GdkModifierType mods;

	  x = event->x;
	  y = event->y;

	  if (event->window != widget->window)
	    gdk_window_get_pointer (widget->window, &x, &y, &mods);

	  if (x < (gint)(widget->allocation.width -
		   GTK_CONTAINER (widget)->border_width * 3 -
		   (CHILD_SPACING * 6) - COMBOARROW_SIZE))
	    {
	      gtk_grab_add (widget);
	      gtk_button_pressed (GTK_BUTTON (combobutton));
	    }
	  else
	    {
	      g_return_val_if_fail (combobutton->menu != NULL, FALSE);

	      gtk_combobutton_menu_pressed (combobutton);
	      gtk_menu_popup (GTK_MENU (combobutton->menu), NULL, NULL,
			      gtk_combobutton_menu_position, combobutton,
			      event->button, event->time);
	    }
	}
    }

  return TRUE;
}

/*static gint
gtk_combobutton_button_release (GtkWidget *widget,
				GdkEventButton *event)
{
  GtkComboButton *combobutton;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_COMBOBUTTON (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->button == 1)
    {
      combobutton = GTK_COMBOBUTTON (widget);
      gtk_grab_remove (widget);
      gtk_button_released (button);
    }
}*/

static void
gtk_combobutton_clicked (GtkComboButton *combobutton,
			 gpointer data)
{
  GtkWidget *menuitem;
  GtkWidget *menu;

  menu = combobutton->menu;
  g_return_if_fail (menu != NULL);

  menuitem = gtk_menu_get_active (GTK_MENU (menu));
  g_return_if_fail (menu != NULL);

  gtk_menu_item_activate (GTK_MENU_ITEM (menuitem));
}

static void
gtk_combobutton_menu_position (GtkMenu *menu,
			       gint *x,
			       gint *y,
			       gpointer cb_menu)
{
  GtkComboButton *combobutton;
  gint screen_width;
  gint screen_height;
  gint ox;
  gint oy;
  gint mx;
  gint my;
  gint width;
  gint height;
  GtkRequisition request;

  g_return_if_fail (cb_menu != NULL);
  g_return_if_fail (GTK_IS_COMBOBUTTON (cb_menu));

  combobutton = GTK_COMBOBUTTON (cb_menu);

  gtk_widget_size_request (GTK_WIDGET (menu), &request);
  width = request.width;
  height = request.height;

  gtk_widget_realize (GTK_WIDGET (menu));
  gdk_window_get_origin (GTK_WIDGET (combobutton)->window, &mx, &my);

  switch (combobutton->arrowdir)
    {
    case GTK_ARROW_DOWN:
      mx += GTK_WIDGET (combobutton)->allocation.width - width + 
	combobutton->x_offset;
      my += GTK_WIDGET (combobutton)->allocation.height + combobutton->y_offset;
      break;
    case GTK_ARROW_UP:
      mx += GTK_WIDGET (combobutton)->allocation.width - width +
	combobutton->x_offset;
      my -= height + combobutton->y_offset;
      break;
    case GTK_ARROW_LEFT:
      mx -= width + combobutton->x_offset;
      my -= combobutton->y_offset;
      break;
    case GTK_ARROW_RIGHT:
      mx += GTK_WIDGET (combobutton)->allocation.width + combobutton->x_offset;
      my += GTK_WIDGET (combobutton)->allocation.height - height + 
	combobutton->y_offset;
      break;
    default:
      mx += GTK_WIDGET (combobutton)->allocation.width - width +
	combobutton->x_offset;
      my += GTK_WIDGET (combobutton)->allocation.height + combobutton->y_offset;
      break;
    }

  screen_width = gdk_screen_width ();
  screen_height = gdk_screen_height ();

  if (my+height > screen_height)
    my = screen_height - height;
  if (mx+width > screen_width)
    mx = screen_width - width;
  if (my < 0)
    my = 0;
  if (mx < 0)
    mx = 0;

  *x = mx;
  *y = my;
}
