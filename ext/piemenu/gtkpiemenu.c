#include <math.h>

/* #include "config.h" */
#include <gdk/gdk.h>
#include "gdkaddons.h"

#include <gtk/gtkmain.h>
#include <gtk/gtkmenushell.h>
#include <gtk/gtkmenu.h>
#include "gtkpiemenu.h"
#include <gtk/gtkmenuitem.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkinvisible.h>

#include "gtkaddons.h"

#include <gtk/gtkprivate.h>

/* uncoment following line if you want visible phantom window */
/* #define PIE_PHANTOM_VISIBLE 	1 */

/* uncoment following line if you do not want allow pie_menu to 
 * grab also keyboard - suitable for debugging */
#define  gdk_keyboard_grab(x,y,z)	0

#ifndef PI
#define PI 3.14159265358979323846264338328
#endif 

#define	PIE_HELPLESS_USER_TIMEOUT	400	/* user do not move - he do not know what to do in 
						 *  (hidden) pie menu... - so we show him it early 
						 */
#define PIE_UNSURE_USER_TIMEOUT		400	/* user who almost knows - but he is not sure whether
						 * he chosed good direction - we show him menu a little
						 * bit later..
						 */

#define	PIE_DEAD_ZONE			 -2
#define PIE_DEAD_ZONE_RADIUS_SQUARED	100

#define PIE_HAS_QUARTERS	1
#define PIE_HAS_EIGHTHS		2
#define	PIE_HAS_SIXTEENTHS	4

#define PIE_MAJOR_MASK		(PIE_HAS_QUARTERS | PIE_HAS_EIGHTHS | PIE_HAS_SIXTEENTHS)

#define	PIE_HAS_NS_POLE		8
#define	PIE_HAS_EW	       16
#define PIE_HAS_POLE_SIXT      32	/* NNE, NNW, SSE, SSW */
#define PIE_HAS_EW_SIXT	       64	/* NEE, NWW, SEE, SWW */

#define PIE_BUTTON_MASK	      (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON4_MASK | \
                               GDK_BUTTON3_MASK | GDK_BUTTON5_MASK)

typedef enum
{
  PIE_MASK_N    = (1 << GTK_PIE_N),
  PIE_MASK_NNE	= (1 << GTK_PIE_NNE),
  PIE_MASK_NE   = (1 << GTK_PIE_NE),
  PIE_MASK_NEE	= (1 << GTK_PIE_NEE),
  PIE_MASK_E    = (1 << GTK_PIE_E),
  PIE_MASK_SEE	= (1 << GTK_PIE_SEE),
  PIE_MASK_SE   = (1 << GTK_PIE_SE),
  PIE_MASK_SSE	= (1 << GTK_PIE_SSE),
  PIE_MASK_S    = (1 << GTK_PIE_S),
  PIE_MASK_SSW	= (1 << GTK_PIE_SSW),
  PIE_MASK_SW   = (1 << GTK_PIE_SW),
  PIE_MASK_SWW	= (1 << GTK_PIE_SWW),
  PIE_MASK_W    = (1 << GTK_PIE_W),
  PIE_MASK_NWW	= (1 << GTK_PIE_NWW),
  PIE_MASK_NW 	= (1 << GTK_PIE_NW),
  PIE_MASK_NNW	= (1 << GTK_PIE_NNW)
} PieCompassMask;

static GtkMenuClass 	*parent_class = NULL;
static const gchar	*position_key = "gtk-pie-menu-position";

static void gtk_pie_menu_class_init (GtkPieMenuClass *class);
static void gtk_pie_menu_init (GtkPieMenu *menu);

static void gtk_pie_menu_destroy (GtkObject *object);
static int  gtk_pie_menu_real_position (GtkPieMenu *menu, int position);
static int  gtk_pie_menu_first_free (GtkPieMenu *menu);
static int  gtk_pie_study (GtkPieMenu *menu);

static void gtk_pie_menu_item_set_pie_position (GtkPieMenu *pmenu, GtkWidget *child, int position);
static int  gtk_pie_menu_item_get_pie_position (GtkPieMenu *pmenu, GtkWidget *child);
static void gtk_pie_menu_realize (GtkWidget *widget);
static void gtk_pie_menu_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gtk_pie_menu_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static gint gtk_pie_menu_expose (GtkWidget *widget, GdkEventExpose *event);
static void gtk_pie_menu_draw (GtkWidget *widget, GdkRectangle *area);
static void gtk_pie_menu_paint (GtkWidget *widget);

static gint gtk_pie_menu_activate (GtkPieMenu *pie_menu, gint id, gint button, guint32 activate_time);
static void gtk_pie_menu_item_select (GtkMenuItem *item);
static void gtk_pie_menu_item_deselect (GtkMenuItem *item);

static gdouble pie_menu_sin [GTK_PIE_MENU_MAX_ITEMS];
static gdouble pie_menu_cos [GTK_PIE_MENU_MAX_ITEMS];

static gint gtk_pie_phantom_motion_notify (GtkWidget *widget, GdkEventMotion *event, GtkPieMenu *pie_menu);
static gint gtk_pie_phantom_press (GtkWidget *widget, GdkEventButton *event, GtkPieMenu	*pie_menu);
static gint gtk_pie_phantom_release (GtkWidget *widget, GdkEventButton *event, GtkPieMenu *pie_menu);

static void gtk_pie_phantom_reset_event_mask (void);
static void gtk_pie_phantom_shutdown (void);
static void gtk_pie_phantom_remove_handlers (void);

static void gtk_pie_menu_real_popup (GtkPieMenu *pie_menu, gint cx, gint cy);
static void gtk_pie_menu_popdown (GtkPieMenu *pie_menu);

static GtkWidget *phantom_window;
static gint phantom_motion_id = -1, 
  phantom_press_id = -1,
  phantom_release_id = -1,
  phantom_timer_id = -1;

static gint phantom_ignore_first_click;
static gint phantom_center_x, phantom_center_y;

GtkType
gtk_pie_menu_get_type (void)
{
  static GtkType pie_menu_type = 0;
  
  if (!pie_menu_type)
    {
      static const GtkTypeInfo pie_menu_info =
      {
	"GtkPieMenu",
	sizeof (GtkPieMenu),
	sizeof (GtkPieMenuClass),
	(GtkClassInitFunc) gtk_pie_menu_class_init,
	(GtkObjectInitFunc) gtk_pie_menu_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
      
      pie_menu_type = gtk_type_unique (gtk_menu_get_type (), &pie_menu_info);
    }
  
  return pie_menu_type;
}

static void
gtk_pie_menu_class_init (GtkPieMenuClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;
  GtkMenuShellClass *menu_shell_class;
  GtkMenuClass *menu_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;
  container_class = (GtkContainerClass*) class;
  menu_shell_class = (GtkMenuShellClass*) class;
  menu_class = (GtkMenuClass*) class;  
  parent_class = gtk_type_class (gtk_menu_get_type ());

  object_class->destroy = gtk_pie_menu_destroy;
  
  widget_class->realize = gtk_pie_menu_realize;
  widget_class->draw = gtk_pie_menu_draw;
  widget_class->size_request = gtk_pie_menu_size_request;
  widget_class->size_allocate = gtk_pie_menu_size_allocate;
  widget_class->expose_event = gtk_pie_menu_expose;

  /* widget_class->key_press_event = gtk_pie_menu_key_press; */
  /* widget_class->button_press_event = gtk_pie_menu_button_press; */
  /* widget_class->button_release_event = gtk_pie_menu_button_release; */
  /* widget_class->motion_notify_event = gtk_pie_menu_motion_notify; */

  /* widget_class->enter_notify_event = gtk_pie_menu_enter_notify; */
  /* widget_class->leave_notify_event = gtk_pie_menu_leave_notify; */

  /*  widget_class->show_all = gtk_pie_menu_show_all; */
  /*  widget_class->hide_all = gtk_pie_menu_hide_all; */

  menu_shell_class->submenu_placement = GTK_LEFT_RIGHT;

  {
    gint 	i;
    gdouble	alfa;

    for (i = 0, alfa = 3.0 * PI / 2.0;
	 i < GTK_PIE_MENU_MAX_ITEMS;
	 i++, alfa += ( 2 * PI / ((gdouble) GTK_PIE_MENU_MAX_ITEMS)))
      {
	pie_menu_cos [i] = cos (alfa);
	pie_menu_sin [i] = sin (alfa);
      }
  }
}

static void
gtk_pie_menu_init (GtkPieMenu *menu)
{
  menu -> position_mask = 0;
  menu -> refinement = 4;
  menu -> spacing = 3;
  menu -> inner_radius = 25;

  menu -> title = NULL;
  menu -> menu_type = 0;

#ifdef HAVE_SHAPE_EXT
  menu -> shape_window = 1;
#else
  menu -> shape_window = 0;
#endif
}

static void
gtk_pie_menu_destroy (GtkObject	    *object)
{
  GtkPieMenu *menu;
  
  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (object));

  menu = GTK_PIE_MENU (object);
  
  gtk_object_ref (object);
  
  if (menu -> title)
    gtk_widget_destroy (menu->title);

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
  
  gtk_object_unref (object);
}

static int
gtk_pie_menu_real_position (GtkPieMenu 	*menu, 
			    int 	position)
{
  gint	i,c, mask;

  mask = menu -> position_mask;

  for (i = 0, c = 0; i < position; i++, mask >>= 1)
    if (mask & 1) c++;

  return c;
} 

static GtkMenuItem *
gtk_pie_get_nth_visible_menu_item (GtkPieMenu 	*menu,
				   int		position)
{
  GList 	*child;
  GtkMenuItem	*menu_item;

  if (position < 0)
    return NULL;

  child = g_list_nth (GTK_MENU_SHELL (menu) -> children, 
		      gtk_pie_menu_real_position (menu, position));
  if (! child)
    return NULL;

  menu_item = child->data;

  if ((menu_item) && GTK_MENU_ITEM (menu_item) &&
      GTK_WIDGET_VISIBLE (menu_item) && 
      GTK_WIDGET_IS_SENSITIVE (menu_item))
    return menu_item;

  return NULL;
}

static int
gtk_pie_menu_first_free (GtkPieMenu *menu)
{
  gint	i, step, mask;

  step = GTK_PIE_MENU_MAX_ITEMS / menu -> refinement;
  mask = menu -> position_mask;

  for (i = 0; i < GTK_PIE_MENU_MAX_ITEMS; i += step, mask >>= step)
    if  ((mask & 1) == 0)
      return i;

  g_return_val_if_fail (step != 1, -1);
  menu -> refinement *= 2;
  
  return gtk_pie_menu_first_free (menu);
}

static int
gtk_pie_study (GtkPieMenu *menu)
{
  int 	mask, ret;


  mask = menu -> position_mask;
  ret = 0;

  if (mask & (PIE_MASK_N | PIE_MASK_S))
    ret = PIE_HAS_NS_POLE | PIE_HAS_QUARTERS;

  if (mask & (PIE_MASK_E | PIE_MASK_W))
    ret |= PIE_HAS_EW | PIE_HAS_QUARTERS;

  if (mask & (PIE_MASK_NW | PIE_MASK_NE | PIE_MASK_SW | PIE_MASK_SE))
    ret |= PIE_HAS_EIGHTHS;

  if (mask & (PIE_MASK_NNW | PIE_MASK_NNE | PIE_MASK_SSW | PIE_MASK_SSE))
    ret |= PIE_HAS_SIXTEENTHS | PIE_HAS_POLE_SIXT;

  if (mask & (PIE_MASK_NWW | PIE_MASK_NEE | PIE_MASK_SWW | PIE_MASK_SEE))
    ret |= PIE_HAS_SIXTEENTHS | PIE_HAS_EW_SIXT;

  return ret;
}

static int
gtk_pie_menu_get_pie_from_xy (GtkPieMenu	*pmenu,
			      gint		px,
			      gint		py)
{
  int		study, pieces, ret;
  gdouble	x, y, a;

  study = gtk_pie_study (pmenu);

  y = (gdouble) (px - phantom_center_x);
  x = (gdouble) (phantom_center_y - py); /* clocks -=>  x,y */

  if ((x*x + y*y) <= ((gdouble) PIE_DEAD_ZONE_RADIUS_SQUARED))
    return PIE_DEAD_ZONE;

  a = atan2 (y,x);

#if GTK_PIE_MENU_MAX_ITEMS != 16
#error GTK_PIE_MENU_MAX_ITEMS != 16 -=> Please update following line
#endif

  if (study & PIE_HAS_SIXTEENTHS)
    pieces = 16;
  else if (study & PIE_HAS_EIGHTHS)
    pieces = 8;
  else if (study & PIE_HAS_QUARTERS)
    pieces = 4;
  else /* nothing in menu */
    return -1;

  a += PI / (gdouble) pieces;

  if (a < 0)		/* <-PI,PI> -=> <0, 2 * PI> */
    a += 2.0 * PI;

  ret = a / (2.0 * PI / (gdouble) pieces);
  ret = (GTK_PIE_MENU_MAX_ITEMS / pieces) * (ret % pieces);

  if (pmenu->position_mask & (1 << ret))
    return ret;
  else
    return -1;
}
			      

static void
gtk_pie_menu_item_set_pie_position (GtkPieMenu 	*pmenu, 
				    GtkWidget  	*child, 
				    int 	position)
{
  if (position >= 0)
    if ((pmenu -> position_mask & (1 << position)) != 0)
      {
	g_warning ("already occupied position");
	return;
      }
    else
      pmenu -> position_mask |= (1 << position);

  if (gtk_object_get_data (GTK_OBJECT (child), position_key) != NULL)
    {
      g_warning ("Pie menu position already set!");
      return;
    }

  gtk_object_set_data (GTK_OBJECT (child), position_key, (gpointer) (position + 11));
}

static int
gtk_pie_menu_item_get_pie_position (GtkPieMenu 	*pmenu, 
				    GtkWidget	*child)
{
  int	position;

  position = (int) gtk_object_get_data (GTK_OBJECT (child), position_key);

  if (position == 0)
    {
      position = gtk_pie_menu_first_free (pmenu);
      gtk_pie_menu_item_set_pie_position (pmenu, child, position);
      return position;
    }

  return position - 11;
}

GtkWidget* 
gtk_pie_menu_new (void)
{
  return GTK_WIDGET (gtk_type_new (gtk_pie_menu_get_type ()));
}


GtkWidget* 
gtk_pie_menu_new_with_refinement (gint refinement)
{
  GtkWidget	*pie_menu;

  g_return_val_if_fail (GTK_PIE_MENU_IS_REFINEMENT (refinement), NULL);

  pie_menu = gtk_pie_menu_new ();
  GTK_PIE_MENU (pie_menu) -> refinement = refinement;

  return pie_menu;
}


void 
gtk_pie_menu_append (GtkPieMenu 	*menu,
		     GtkWidget	       	*child)
{
  int	ff;

  g_return_if_fail (menu != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (menu));

  ff = gtk_pie_menu_first_free (menu);
  return gtk_pie_menu_put_in_refinement (menu, child, ff, GTK_PIE_MENU_MAX_ITEMS);
}

void
gtk_pie_menu_put (GtkPieMenu	       *menu,
		  GtkWidget	       *child,
		  gint			position)
{
  g_return_if_fail (menu != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (menu));

  position *= GTK_PIE_MENU_MAX_ITEMS / menu->refinement;
  return gtk_pie_menu_put_in_refinement (menu, child, position, GTK_PIE_MENU_MAX_ITEMS);
}

void
gtk_pie_menu_put_with_compass (GtkPieMenu	*menu,
			       GtkWidget	*child,
			       GtkPieCompass	 compass)
{
  g_return_if_fail (menu != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (menu));

  return gtk_pie_menu_put_in_refinement (menu, child, (gint) compass, GTK_PIE_MENU_MAX_ITEMS);
}

void
gtk_pie_menu_put_in_refinement (GtkPieMenu	       *menu,
				GtkWidget	       *child,
				gint			position,
				gint			refinement)
{
  int		rpos;

  g_return_if_fail (menu != NULL);
  g_return_if_fail (child != NULL);
  g_return_if_fail (GTK_PIE_MENU_IS_REFINEMENT (refinement));
  g_return_if_fail ((0 <= position) && (position < refinement));
  position *= GTK_PIE_MENU_MAX_ITEMS / refinement;
  g_return_if_fail (((menu -> position_mask) & (1 << position)) == 0);

  gtk_pie_menu_item_set_pie_position (menu, child, position);
  rpos = gtk_pie_menu_real_position (menu, position);

  gtk_menu_shell_insert (GTK_MENU_SHELL (menu), child, rpos);
}	

void
gtk_pie_menu_set_refinement (GtkPieMenu *menu,
			     gint 	refinement)
{
  g_return_if_fail (menu != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (menu));
  g_return_if_fail (GTK_PIE_MENU_IS_REFINEMENT (refinement));

  menu -> refinement = refinement;
}

void
gtk_pie_menu_set_spacing (GtkPieMenu 	*menu,      
			  gint 		spacing)
{
  g_return_if_fail (menu != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (menu));
  g_return_if_fail (spacing >= 0);

  if (menu -> spacing != spacing)
    {
      menu -> spacing = spacing;
      gtk_widget_queue_resize (GTK_WIDGET (menu));
    }
}

void
gtk_pie_menu_set_radius (GtkPieMenu	 *menu,      
			 gint 		radius)
{
  g_return_if_fail (menu != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (menu));
  g_return_if_fail (radius >= 0);

  if (menu -> inner_radius != radius)
    {
      menu -> inner_radius = radius;
      gtk_widget_queue_resize (GTK_WIDGET (menu));
    }
}

void
gtk_pie_menu_set_title (GtkPieMenu	*menu,
			GtkWidget	*child)
{
  g_return_if_fail (menu != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (menu));
  g_return_if_fail (child != NULL);
  g_return_if_fail (GTK_WIDGET (child));

  gtk_widget_set_parent (child, GTK_WIDGET (menu));

  if (GTK_WIDGET_VISIBLE (child->parent))
    {
      if (GTK_WIDGET_REALIZED (child->parent) &&
	  !GTK_WIDGET_REALIZED (child))
	gtk_widget_realize (child);

      if (GTK_WIDGET_MAPPED (child->parent) &&
	  !GTK_WIDGET_MAPPED (child))
	gtk_widget_map (child);
    }

  if (GTK_WIDGET_VISIBLE (menu))
    gtk_widget_queue_resize (GTK_WIDGET (menu));
}

static void
gtk_pie_menu_realize (GtkWidget *widget)
{
  GdkWindowAttr attributes;
  gint attributes_mask;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_MENU (widget));
  
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK |
			    GDK_MOTION_NOTIFY | GDK_BUTTON_PRESS_MASK |
			    GDK_BUTTON_RELEASE_MASK);
  
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);
  
  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
  gtk_pie_menu_paint(widget);
}

static void
gtk_pie_menu_size_request (GtkWidget		*widget,
			   GtkRequisition	*requisition)
{
  GtkPieMenu		*menu;
  GtkMenuShell		*menu_shell;
  GtkWidget		*child;
  GList			*children;

  GtkWidget		*pie_children [GTK_PIE_MENU_MAX_ITEMS];
  GtkRequisition	children_reqs [GTK_PIE_MENU_MAX_ITEMS];
  gint			max_width, max_height, width, height, radius, tmp, id, jd;
  double		start;

#if GTK_PIE_MENU_MAX_ITEMS != 16
#error GTK_PIE_MENU_MAX_ITEMS != 16 -=> Please update following lines
#endif
  gint			occupied [4];

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (widget));
  g_return_if_fail (requisition != NULL);

  menu = GTK_PIE_MENU (widget);
  menu_shell = GTK_MENU_SHELL (widget);

  if (menu -> menu_type == 0)
    {
      (GTK_WIDGET_CLASS (parent_class)->size_request) (widget, requisition);
      return;
    }

  /* g_message ("size_request"); */

  max_height = 0;
  max_width = 0;

  for (id = 0; id < GTK_PIE_MENU_MAX_ITEMS; id++)
    pie_children [id] = NULL;

  occupied [0] = occupied [1] = occupied [2] = occupied [3] = 0;

  children = menu_shell->children;
  while (children)
    {
      child = children->data;
      children = children->next;

      id = gtk_pie_menu_item_get_pie_position (menu, child);
      if (id < 0)
	continue;

      if (GTK_WIDGET_VISIBLE (child))
	{
	  GTK_MENU_ITEM (child)->show_submenu_indicator = FALSE;
	  gtk_widget_size_request (child, children_reqs + id);

	  max_height = MAX (max_height, children_reqs[id].height);
	  pie_children [id] = child;

	  if (id >= (GTK_PIE_MENU_MAX_ITEMS / 2))
	    id = GTK_PIE_MENU_MAX_ITEMS - id;

	  if (id >= (GTK_PIE_MENU_MAX_ITEMS / 4))
	    id = (GTK_PIE_MENU_MAX_ITEMS / 2) - id;

	  occupied [id] = 1;
	}
    }

  radius = menu->inner_radius;
  start = ((double) menu->spacing) + (menu->shape_window ? 2.0 : 0.0);

  for (id = 0; id < (GTK_PIE_MENU_MAX_ITEMS / 4); id++)
    if (occupied [id] == 1)
      for (jd = id + 1;  jd < (GTK_PIE_MENU_MAX_ITEMS / 4); jd++)
	if (occupied [jd] == 1)
	  {
	    tmp = (gint)
	          ((start + ((double) max_height) * (id == 0 ? 0.5 : 1.0))
	        /* sinus are stored according to screen coords so we 	*
		 * had to negate result of substraction... 		*/
	      / (- pie_menu_sin [id] + pie_menu_sin [jd]));
	        
	    if (tmp > radius)
	      radius = radius = tmp;
	  }

  menu->inner_radius = radius;

  width = 0;
  height = 0;

  
  for (id = 0; id < GTK_PIE_MENU_MAX_ITEMS; id++)
    if (pie_children [id] != NULL)
      {
	if ((id == 0) || (id == (GTK_PIE_MENU_MAX_ITEMS / 2)))
	  {
	    width = MAX (children_reqs [id].width / 2, width);

	    tmp = max_height + (gint) (((double) radius) * ABS (pie_menu_sin [id]));
	    height = MAX (tmp, height);
	  }
	else
	  {
	    tmp = children_reqs [id].width + (gint) (ABS (pie_menu_cos [id]) * (gdouble) radius);
	    width = MAX (tmp, width);

	    tmp = max_height / 2 + (gint) (((double) radius) * pie_menu_sin [id]);
	    height = MAX (tmp, height);
	  }
      }

  requisition->width = 2 * width + 2;
  requisition->height = 2 * height + 2;
  
#if 0 
  /* titles not yet implemented :( */
  if (menu->title != NULL && GTK_WIDGET_VISIBLE (menu->title))
    {
      gtk_widget_size_request (menu->title, &child_requisition);
      requisition -> height += child_requisition.height + menu->spacing;
      width = child_requisition.width;
    }
#endif

  requisition->width += (GTK_CONTAINER (menu)->border_width +
			 widget->style->klass->xthickness) * 2;
  requisition->height += (GTK_CONTAINER (menu)->border_width +
			  widget->style->klass->ythickness) * 2;
}

static void
gtk_pie_menu_size_allocate (GtkWidget		*widget,
			    GtkAllocation	*allocation)
{
  GtkPieMenu		*menu;
  GtkMenuShell		*menu_shell;

  GtkWidget		*child;
  GList			*children;

  GtkRequisition 	child_requisition;
  GtkAllocation		child_allocation;

  gint			cw, ch, id, sx, sy;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (widget));
  g_return_if_fail (allocation != NULL);

  /* g_message ("size_allocate"); */

  menu = GTK_PIE_MENU (widget);
  menu_shell = GTK_MENU_SHELL (widget);

  if (menu -> menu_type == 0)
    {
      (GTK_WIDGET_CLASS (parent_class)->size_allocate) (widget, allocation);
      return;
    }

  widget -> allocation = *allocation;
  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_move_resize (widget->window,
			    allocation->x, allocation->y,
			    allocation->width, allocation->height);

  if (menu->title)
    g_warning ("No support for pie menu tiles");

  cw = allocation -> width / 2;
  ch = allocation -> height / 2 ;

  children = menu_shell -> children;
  while (children)
    {
      child = (GtkWidget *) children -> data;
      children = children -> next;

      id = gtk_pie_menu_item_get_pie_position (menu, child);


      if ((id >= 0) && GTK_WIDGET_VISIBLE (child))
	{
	  sx = cw + (pie_menu_cos [id] * (gdouble) (menu->inner_radius));
	  sy = ch + (pie_menu_sin [id] * (gdouble) (menu->inner_radius));

	  gtk_widget_get_child_requisition (child, &child_requisition);

	  child_allocation.width = child_requisition.width;
	  child_allocation.height = child_requisition.height;

	  if ((id == 0) || (id == (GTK_PIE_MENU_MAX_ITEMS / 2)))
	    {
	      child_allocation.x = sx - (child_allocation.width / 2);
	      if (id != 0)
		child_allocation.y = sy;
	      else
		child_allocation.y = sy - child_allocation.height;
	    }
	  else
	    {
	      child_allocation.y = sy - child_allocation.height / 2;

	      if (id < (GTK_PIE_MENU_MAX_ITEMS / 2))
		child_allocation.x = sx;
	      else
		child_allocation.x = sx - child_allocation.width;
	    }

	  gtk_widget_size_allocate (child, &child_allocation);
	}
    }
}

static void
gtk_pie_menu_paint (GtkWidget *widget)
{
  GtkPieMenu	*pie_menu;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (widget));
  
  pie_menu = GTK_PIE_MENU (widget);
  
  if (GTK_WIDGET_DRAWABLE (widget) && 
      (GTK_PIE_MENU (pie_menu) -> shape_window == 0))
    gtk_paint_box (widget->style,
		   widget->window,
		   GTK_STATE_NORMAL,
		   GTK_SHADOW_OUT,
		   NULL, widget, "piemenu",
		   0, 0, -1, -1);
}

static void
gtk_pie_menu_paint_child (GtkWidget *widget, GtkWidget *child, GdkRectangle *area)
{
  GtkAllocation	*allocation;

  /* g_message ("paint child"); */
  allocation = &(child->allocation);

  gtk_paint_box (widget->style,
		 widget->window,
		 GTK_STATE_NORMAL,
		 GTK_SHADOW_OUT,
		 area, widget, "piemenu",
		 allocation->x - 1, allocation->y - 1,
		 allocation->width + 2, allocation->height + 2);
}

static void
gtk_pie_menu_draw (GtkWidget    *widget,
		   GdkRectangle *area)
{
  GtkMenuShell *menu_shell;
  GtkPieMenu   *pie_menu;
  GtkWidget *child;
  GdkRectangle child_area;
  GList *children;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_PIE_MENU (widget));
  g_return_if_fail (area != NULL);
  
  pie_menu = GTK_PIE_MENU (widget);
  if (pie_menu -> menu_type == 0)
    {
      (GTK_WIDGET_CLASS (parent_class)->draw) (widget, area);
      return;
    }

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      gtk_pie_menu_paint (widget);
      
      menu_shell = GTK_MENU_SHELL (widget);
      
      children = menu_shell->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;
	  
	  if (gtk_widget_intersect (child, area, &child_area))
	    {
	      if (pie_menu -> shape_window)
		gtk_pie_menu_paint_child (widget, child, area);

	      gtk_widget_draw (child, &child_area);
	    }
	}
    }
}

static gint
gtk_pie_menu_expose (GtkWidget	*widget,
		     GdkEventExpose *event)
{
  GtkMenuShell 		*menu_shell;
  GtkPieMenu   		*pie_menu;
  GtkWidget 		*child;
  GdkEventExpose 	child_event;
  GList 		*children;
  int			intersects;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_MENU (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
  pie_menu = GTK_PIE_MENU (widget);
  if (pie_menu -> menu_type == 0)
    return (GTK_WIDGET_CLASS (parent_class)->expose_event) (widget, event);


  if (GTK_WIDGET_DRAWABLE (widget))
    {
      gtk_pie_menu_paint (widget);
      
      menu_shell = GTK_MENU_SHELL (widget);
      child_event = *event;
      
      children = menu_shell->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;
	  
	  intersects = -1;

	  if (pie_menu -> shape_window)
	    {
	      GdkRectangle	area;

	      intersects = gtk_widget_intersect (widget, &event->area, &area);
	      gtk_pie_menu_paint_child (widget, child, &area);
	    }
	  
	  if ((intersects != 0) && GTK_WIDGET_NO_WINDOW (child))
	    {
	      gtk_widget_intersect (widget, &event->area, &child_event.area);
	      gtk_widget_event (child, (GdkEvent*) &child_event);
	    }
	}
    }
  
  return FALSE;
}

static void
gtk_pie_menu_shape (GtkPieMenu *pie_menu)
{
  GList			*children;
  GtkWidget		*widget;
  GtkWidget		*child;
  GtkMenu		*menu;
  GtkRequisition	requisition;
  GtkAllocation		*allocation;
  GdkRectangle  	rectangles [GTK_PIE_MENU_MAX_ITEMS];
  gint			count;

  if (pie_menu -> shape_window == 0)
    return;

  menu = GTK_MENU (pie_menu);
  widget = GTK_WIDGET (pie_menu);

  if (! GTK_WIDGET_REALIZED (menu->toplevel))
    gtk_widget_realize (menu->toplevel);

  gtk_pie_menu_size_request (widget, &requisition);
  widget->allocation.x = 0; widget->allocation.y = 0;
  widget->allocation.width = requisition.width;
  widget->allocation.height = requisition.height;
  gtk_pie_menu_size_allocate (widget, &(widget->allocation));

  children = GTK_MENU_SHELL (pie_menu) -> children;
  count = 0;

  while (children)
    {
      child = children->data;
      children = children -> next;

      if (! GTK_WIDGET_VISIBLE (child))
	continue;

      allocation = &(child -> allocation);

      rectangles[count].x = allocation->x - 1;
      rectangles[count].y = allocation->y - 1;
      rectangles[count].width = allocation->width + 2;
      rectangles[count].height = allocation->height + 2;

      /* g_message ("Shaping (%d,%d,%d,%d)", allocation->x, */
      /* 		 allocation->y, */
      /* 		 allocation->width, */
      /* 	         allocation->height); */

      count++;
    }

  if (count)
    gdk_window_shape_combine_rectangles (menu -> toplevel-> window,
					 (count ? rectangles : NULL),
					 count, 0, 0);
}

static void 
gtk_pie_menu_try_select (GtkPieMenu	*pie_menu,
			 gint		x,
			 gint		y)
{
  int		candidate;
  GtkMenuItem 	*menu_item;
  GtkMenuShell 	*menu_shell;
  GtkWidget	*widget;

  menu_shell = GTK_MENU_SHELL (pie_menu);
  widget = GTK_WIDGET (pie_menu);

  candidate = gtk_pie_menu_get_pie_from_xy (pie_menu, x, y);
  menu_item = gtk_pie_get_nth_visible_menu_item (pie_menu, candidate);

  if (menu_shell->active_menu_item != ((GtkWidget *) menu_item))
    {

      if (menu_shell -> active_menu_item != NULL)
	gtk_pie_menu_item_deselect (GTK_MENU_ITEM (menu_shell -> active_menu_item));

      if (menu_item != NULL)
	{
	  gtk_pie_menu_item_select (menu_item);
	  /* g_message ("selection changed"); */
	}

      /* we do not use GTK_WIDGET (menu_item) since 
       * menu_item could be NULL */

      menu_shell->active_menu_item = (GtkWidget *) menu_item;
    }
}

static void
gtk_pie_menu_item_select (GtkMenuItem *item)
{
  if (item -> submenu == NULL)
    gtk_menu_item_select (item);
  else
    { /* gtk_menu_item_select is dangerous since it can
       * pop up submenu window and mess pointer grabs etc...
       * as you can guess we do not want it that way... 
       */

       gtk_widget_set_state (GTK_WIDGET (item), GTK_STATE_PRELIGHT);
       gtk_widget_draw (GTK_WIDGET (item), NULL);
    }
}

static void
gtk_pie_menu_item_deselect (GtkMenuItem *item)
{
  if (item -> submenu == NULL)
    gtk_menu_item_deselect (item);
  else
    { 
       gtk_widget_set_state (GTK_WIDGET (item), GTK_STATE_NORMAL);
       gtk_widget_draw (GTK_WIDGET (item), NULL);
    }
}

static gint
gtk_pie_menu_activate (GtkPieMenu 	*pie_menu, 
		       gint 		id,
		       gint		button,
		       guint32		activate_time)
{
  GtkMenuItem		*menu_item;

  g_return_val_if_fail (pie_menu, -1);
  g_return_val_if_fail (GTK_IS_PIE_MENU (pie_menu), -1);

  menu_item = gtk_pie_get_nth_visible_menu_item (pie_menu, id);

  if (menu_item == NULL)
    return -1;

  if (menu_item -> submenu != NULL)
    {
      g_return_val_if_fail (GTK_IS_PIE_MENU (menu_item -> submenu), FALSE);

      phantom_ignore_first_click = TRUE;
      gtk_pie_phantom_reset_event_mask ();
      gtk_pie_phantom_remove_handlers ();

      gtk_pie_menu_popup (GTK_PIE_MENU (menu_item -> submenu), 
			  button,
			  activate_time);
      return TRUE;
    }

  gtk_widget_activate (GTK_WIDGET (menu_item));
  return FALSE;
}


/*
 *  I have been here....
 *
 *				PHANTOMAS
 */

static gint
gtk_pie_phantom_unsure_timeout_callback (GtkPieMenu *menu)
{
  int			mx, my;
  GdkModifierType 	mask;

  g_return_val_if_fail (menu, FALSE);
  g_return_val_if_fail (GTK_PIE_MENU (menu), FALSE);

  gdk_window_get_pointer (NULL, &mx, &my, &mask);

  phantom_timer_id = -1;
  /* g_message ("Unsurly popuping"); */

  if ((mask & PIE_BUTTON_MASK) != 0)
    gtk_pie_menu_real_popup (menu, -1, -1);
  else
    gtk_pie_menu_real_popup (menu, mx, my);

  /* world is cruel */
  return FALSE;
}

static gint
gtk_pie_phantom_helpless_timeout_callback (GtkPieMenu *menu)
{ 
  gint		mx,my;
  gint		x,y;

  g_return_val_if_fail (menu, FALSE);
  g_return_val_if_fail (GTK_IS_PIE_MENU (menu), FALSE);

  gdk_window_get_pointer (NULL, &mx, &my, NULL);

  /* we have been called -=> pie_menu is still visible */

  x = mx - phantom_center_x;
  y = my - phantom_center_y;

  if ((x*x + y*y) <= PIE_DEAD_ZONE_RADIUS_SQUARED)
    { 
      /* help to helpless users :) - or shock the lazy ones */ 
      /* g_message ("Helplessly popuping"); */
     phantom_timer_id = -1;
     gtk_pie_menu_real_popup (menu, mx, my);
    }
  else
    {
      /* user at least moved mouse pointer a little bit - 
       * so we should be The Patience and give him another try :) 
       */ 
      phantom_timer_id = gtk_timeout_add (PIE_UNSURE_USER_TIMEOUT,
					  (GtkFunction) gtk_pie_phantom_unsure_timeout_callback,
					  menu);
    }

  /* we do not want you any more ! */
  return FALSE;
}

static gint
gtk_pie_phantom_press (GtkWidget	*widget,
			GdkEventButton  *event,
			GtkPieMenu	*pie_menu)
{ 
  g_return_val_if_fail (widget, FALSE);
  g_return_val_if_fail (event, FALSE);
  g_return_val_if_fail (pie_menu, FALSE);
  g_return_val_if_fail (GTK_IS_PIE_MENU (pie_menu), FALSE);

  if (phantom_ignore_first_click)
    phantom_ignore_first_click = FALSE;
  else
    if (! (GTK_WIDGET_VISIBLE (GTK_MENU (pie_menu)->toplevel)))
      gtk_pie_menu_real_popup (pie_menu,
			       (gint) event->x_root, 
			       (gint) event->y_root);

  return TRUE;
}

static gint
gtk_pie_phantom_release (GtkWidget 	 *widget,
			 GdkEventButton  *event,
			 GtkPieMenu	 *pie_menu)
{
  static guint32	my_time = -1;
  int 			x,y, result, visible;

  g_return_val_if_fail (pie_menu != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_PIE_MENU (pie_menu), FALSE);

  /* some time wired things happend and I got one event two times */
  /* huh ... */

  if (my_time == event->time)
    return TRUE; 

  my_time = event->time;

  x = (gint) event->x_root;
  y = (gint) event->y_root;

  result = gtk_pie_menu_get_pie_from_xy (pie_menu, x,y);
  visible = GTK_WIDGET_VISIBLE (GTK_MENU (pie_menu) -> toplevel);
  /* g_message ("Phantom release (result %d) (state %d) (x %d) (y %d)",  */
  /* 	     result, event->state,  */
  /* 	     x, y); */

  switch (result)
    {
    case PIE_DEAD_ZONE:  /* dead zone */

      if (! visible)
	gtk_pie_menu_real_popup (pie_menu, x, y);
      else
	{
	  gtk_pie_menu_popdown (pie_menu);
	  gtk_pie_phantom_shutdown ();
	}

      break;
    case -1:
      if (! GTK_WIDGET_VISIBLE (GTK_MENU (pie_menu) -> toplevel))
	gdk_beep ();
	
      gtk_pie_phantom_shutdown ();
      gtk_pie_menu_popdown (pie_menu);

      break;
    default:
      phantom_center_x = x;
      phantom_center_y = y;

      gtk_pie_menu_popdown (pie_menu);

      switch (gtk_pie_menu_activate (pie_menu, result, 0, event->time))
	{
	case -1:
	  if (! visible)
	    gdk_beep ();
	  /* NO BREAK !!! */
	case FALSE:
	  gtk_pie_phantom_shutdown ();
	  break;
	case TRUE:
	  break;
	}

      break;
    }
      
  return TRUE;
}

static gint
gtk_pie_phantom_motion_notify (GtkWidget 	*widget,
			       GdkEventMotion  	*event,
			       GtkPieMenu	*menu)
{
  gint		x,y;

  g_return_val_if_fail (widget, FALSE);
  g_return_val_if_fail (event, FALSE);
  g_return_val_if_fail (menu, FALSE);

  g_return_val_if_fail (GTK_IS_PIE_MENU (menu), FALSE);
  g_return_val_if_fail (GTK_WIDGET_VISIBLE (GTK_MENU (menu)->toplevel),
			FALSE); 

  if (event -> is_hint)
    gdk_window_get_pointer (NULL, &x, &y, NULL);
  else
    {
      x = (gint) event -> x_root;
      y = (gint) event -> y_root;
    }

  if ((event -> state & PIE_BUTTON_MASK) == 0)
    {
      GtkWidget		*mwidget;

      phantom_center_x = x;
      phantom_center_y = y;
      mwidget = GTK_WIDGET (menu);

      gdk_window_move (GTK_MENU (menu)->toplevel->window, 
		       event->x_root - mwidget->allocation.width / 2,
		       event->y_root - mwidget->allocation.height / 2);
    }

  gtk_pie_menu_try_select (menu, x, y);

  return TRUE;
}

static void
gtk_pie_phantom_shutdown (void)
{
  g_return_if_fail (phantom_window);

  gtk_grab_remove (phantom_window);
  gtk_pie_phantom_remove_handlers ();

  /* hide & destroy clears everything - callbacks, pointer grab,.. */

  gtk_widget_hide (phantom_window);
  gtk_widget_destroy (phantom_window);
  
  phantom_window = NULL;
}

static void
gtk_pie_phantom_reset_event_mask (void)
{
  gint		mask;

  mask = gtk_widget_get_events (phantom_window);
  mask &= ~ (GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
  /* 
   *  gtk_widget_set_events (phantom_window, mask); 
   *    - does not work so I think that nobody complain if
   *	  we bypass this limitation by dirty hack
   */
  gdk_window_set_events (phantom_window -> window, mask);
  gdk_change_active_pointer_grab (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
				  NULL,
				  0 /* current time */);
}

static void
gtk_pie_phantom_remove_handlers (void)
{
  if (phantom_motion_id != -1)
    gtk_signal_disconnect (GTK_OBJECT (phantom_window), phantom_motion_id);
  if (phantom_press_id != -1)
    gtk_signal_disconnect (GTK_OBJECT (phantom_window), phantom_press_id);
  if (phantom_release_id != -1)
    gtk_signal_disconnect (GTK_OBJECT (phantom_window), phantom_release_id);

  if (phantom_timer_id != -1)
    gtk_timeout_remove (phantom_timer_id);

  phantom_motion_id = phantom_press_id = phantom_release_id = phantom_timer_id = -1;
}

void
gtk_pie_menu_popdown (GtkPieMenu *pie_menu)
{
  GtkMenuShell	*menu_shell;
  GtkMenu 	*menu;

  menu = GTK_MENU (pie_menu);

  menu_shell = GTK_MENU_SHELL (pie_menu);
  if (menu_shell -> active_menu_item)
    {
      gtk_pie_menu_item_deselect (GTK_MENU_ITEM (menu_shell->active_menu_item));
      menu_shell -> active_menu_item = NULL;
    }

  if (GTK_WIDGET_VISIBLE (menu->toplevel))
    {
      gtk_widget_hide (GTK_MENU (pie_menu) -> toplevel);
      gtk_widget_hide (GTK_WIDGET (pie_menu));

      if (pie_menu -> shape_window != 0)
	gdk_window_shape_combine_mask (menu -> toplevel-> window,
				       NULL, 0,0);
    }

  if (phantom_motion_id != -1)
    gtk_signal_disconnect (GTK_OBJECT (phantom_window), phantom_motion_id);

  if (phantom_timer_id != -1)
    gtk_timeout_remove (phantom_timer_id);

  phantom_motion_id = phantom_timer_id = -1;

  pie_menu -> menu_type = 0;
}

static void
gtk_pie_menu_real_popup (GtkPieMenu 	*pie_menu, 
			 gint 		cx, 
			 gint		cy)
{
  GtkWidget		*widget;
  GtkMenu		*menu;

  GtkRequisition	 requisition;
  gint			 sx, sy, dmx, dmy, wx, wy;
  gint			 screen_w, screen_h;

  g_return_if_fail (pie_menu);
  g_return_if_fail (GTK_IS_PIE_MENU (pie_menu));
  g_return_if_fail (! GTK_WIDGET_VISIBLE (GTK_MENU (pie_menu) -> toplevel));

  if (phantom_timer_id != -1)
    { gtk_timeout_remove (phantom_timer_id); phantom_timer_id = -1; }

  if (cx >= 0 || cy >= 0)
    {
      phantom_center_x = cx;
      phantom_center_y = cy;
    }

  /* g_message ("Real popup: (%d,%d)", cx,cy); */

  widget = GTK_WIDGET (pie_menu);
  menu = GTK_MENU (pie_menu);

  gtk_widget_size_request (widget, &requisition);

  wx = requisition.width / 2;
  wy = requisition.height / 2;

  sx = phantom_center_x - wx;
  sy = phantom_center_y - wy;

  screen_w = gdk_screen_width ();
  screen_h = gdk_screen_height ();

  dmx = dmy = 0;

  if (sx < 0)
    { dmx = -sx; sx = 0; }

  if (sy < 0)
    { dmy = -sy; sy = 0; }
  
  if (sx + requisition.width > screen_w)
    { dmx = screen_w - sx - requisition.width; sx = screen_w - requisition.width; }

  if (sy + requisition.height > screen_h)
    { dmy = screen_h - sy - requisition.height; sy = screen_h - requisition.height; }

  gtk_window_set_win_position (menu->toplevel, sx, sy);
  gtk_pie_menu_shape (pie_menu);

  gtk_widget_show (widget);
  gtk_widget_show (menu->toplevel);

  if ((dmx | dmy) != 0)
    {
      phantom_center_x += dmx;
      phantom_center_y += dmy;
      gdk_window_warp_pointer (NULL, NULL, 0,0,0,0, dmx, dmy);
    }

  if (((dmx | dmy) != 0) || (cx < 0) || (cy < 0))
    {
      gdk_window_get_pointer (NULL, &dmx, &dmy, NULL);
      gtk_pie_menu_try_select (pie_menu, dmx, dmy);
    }

  phantom_motion_id = 
    gtk_signal_connect (GTK_OBJECT (phantom_window), "motion_notify_event",
			(GtkSignalFunc) gtk_pie_phantom_motion_notify, (gpointer) pie_menu);

  gtk_widget_add_events (phantom_window, 
			 GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
  gdk_change_active_pointer_grab (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | 
				  GDK_POINTER_MOTION_MASK| GDK_POINTER_MOTION_HINT_MASK,
				  NULL,
				  0 /* current time */);
}

void
gtk_pie_menu_popup (GtkPieMenu *pie_menu,
		    guint	button,
		    guint32	activate_time)
{
  g_return_if_fail (pie_menu);
  g_return_if_fail (GTK_IS_PIE_MENU (pie_menu));

  pie_menu -> menu_type = 1;

  if (phantom_window == NULL)
    {
      gint 	have_xgrab = FALSE;

#ifndef PIE_PHANTOM_VISIBLE
      phantom_window = gtk_invisible_new ();
#else
      phantom_window = gtk_window_new (GTK_WINDOW_POPUP);
#endif

      gtk_widget_add_events (phantom_window,
			     /* we do not need MOTION_NOTIFY (yet) */
			     GDK_BUTTON_PRESS_MASK | 
			     GDK_BUTTON_RELEASE_MASK);

      gtk_widget_show (phantom_window);

      if ((gdk_pointer_grab (phantom_window -> window, TRUE,
			     GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
			     NULL, NULL, activate_time)) == 0)
	{
	  if (gdk_keyboard_grab (phantom_window -> window, TRUE, activate_time) == 0)
	    have_xgrab = TRUE;
	  else
	    gdk_pointer_ungrab (activate_time);
	}
  
      if (! have_xgrab)
	{
	  g_warning ("gtk_pie_menu_popup: Can not grab X pointer :(");
	  gtk_widget_destroy (phantom_window);
	  phantom_window = NULL;
	  return;
	}

      gtk_grab_add (GTK_WIDGET (phantom_window));

      gdk_window_get_pointer (NULL, &phantom_center_x, &phantom_center_y, NULL);
      phantom_ignore_first_click = FALSE;
    }
  /*else phantom_window != NULL
   * - phantom menu already exists, it is visible, has grab
   *   and there have been some pie menu which utilized its services
   *   current pie_menu is submenu of such (previous) menu
   */

  /* we set motion notify later.... 
   * when we see it is realy necessary (when pie menu should show up)
   */

  phantom_motion_id = -1;

  phantom_press_id = 
    gtk_signal_connect (GTK_OBJECT (phantom_window), "button_press_event",
			(GtkSignalFunc) gtk_pie_phantom_press, (gpointer) pie_menu);
  phantom_release_id = 
    gtk_signal_connect (GTK_OBJECT (phantom_window), "button_release_event",
			(GtkSignalFunc) gtk_pie_phantom_release, (gpointer) pie_menu);

  phantom_timer_id = gtk_timeout_add (PIE_HELPLESS_USER_TIMEOUT,
				      (GtkFunction) gtk_pie_phantom_helpless_timeout_callback,
				      (gpointer) pie_menu);
}


