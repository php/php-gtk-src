#include <gtk/gtkmain.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>
#include "gtkaddons.h"

void
gtk_window_set_win_position (GtkWidget *widget,
			     gint	     x,
			     gint	     y)
{
  /* this is an quick & dirty hack */

  g_return_if_fail (widget);
  g_return_if_fail (GTK_WINDOW (widget));

  if ((x >= 0) && (y >= 0))
    gtk_widget_set_uposition (widget, x,y);
  else
    { 
      /* g_message ("Negative warp to %d,%d", x,y); */

      if (! GTK_WIDGET_REALIZED (widget))
	gtk_widget_realize (widget);

      widget -> allocation.x = x;
      widget -> allocation.y = y;

      gdk_window_set_hints (widget->window, x, y, 0, 0, 0, 0, GDK_HINT_POS);
      gdk_window_move (widget->window, x,y);
    }
}
