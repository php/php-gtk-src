#include <gdk/gdkprivate.h>
#include <gdk/gdk.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>

#include "gdkaddons.h"
/* #include "config.h" */

gint
gdk_change_active_pointer_grab (GdkEventMask 	event_mask,
				GdkCursor *	cursor,
				guint32		time)
{
  GdkCursorPrivate 	*cursor_private;
  guint  		xevent_mask;
  Cursor 		xcursor;

  int	 		i;

  g_return_val_if_fail (gdk_xgrab_window, 1); /* GrabSuccess == 0 */

  cursor_private = (GdkCursorPrivate*) cursor;

  if (! cursor)
    xcursor = None;
  else
    xcursor = cursor_private ->xcursor;

  xevent_mask = 0;
  for (i = 0; i < gdk_nevent_masks; i++)
    if (event_mask & (1 << (i + 1)))
      xevent_mask |= gdk_event_mask_table [i];

  return XChangeActivePointerGrab (gdk_xgrab_window -> xdisplay,
				   xevent_mask,
				   xcursor,
				   time);
}

void
gdk_window_warp_pointer (GdkWindow     *src_window,
			 GdkWindow     *dest_window,
			 gint 	    	src_x,
			 gint		src_y,
			 guint		src_width,
			 guint		src_height,
			 guint		dest_x,
			 guint		dest_y)
{
  GdkWindowPrivate *private;
  Window	src_w;
  Window	dest_w;


  private = NULL;

  if (src_window == NULL)
    src_w = None;
  else
    {
      private = (GdkWindowPrivate *) src_window;
      src_w = private->xwindow;
    }

  if (dest_window == NULL)
    dest_w = None;
  else
    {
      private = (GdkWindowPrivate *) dest_window;
      dest_w = private->xwindow;
    }

  if (private == NULL)
    private = &gdk_root_parent;

  XWarpPointer (private->xdisplay, src_w, dest_w,
		src_x, src_y, src_width, src_height,
		dest_x, dest_y);
}

gboolean
gdk_window_have_shape_ext (void)
{
  enum { UNKNOWN, NO, YES };
  static gint have_shape = UNKNOWN;
  
  if (have_shape == UNKNOWN)
    {
      int ignore;
      if (XQueryExtension (gdk_display, "SHAPE", &ignore, &ignore, &ignore))
	have_shape = YES;
      else
	have_shape = NO;
    }
  
  return (have_shape == YES);
}


void
gdk_window_shape_combine_rectangles (GdkWindow 		*window,
				     GdkRectangle 	*rectangles,
				     gint		n_rectangles,
				     gint		x,
				     gint		y)
{
  GdkWindowPrivate *window_private;

  g_return_if_fail (window != NULL);

#ifdef HAVE_SHAPE_EXT
  window_private = (GdkWindowPrivate *) window;
  if (window_private -> destroyed)
    return;

  if (gdk_window_have_shape_ext ())
    XShapeCombineRectangles (window_private -> xdisplay,
			     window_private -> xwindow,
			     ShapeBounding,
			     x,y,
			     (XRectangle *) rectangles,
			     n_rectangles,
			     ShapeSet,
			     Unsorted);
#endif HAVE_SHAPE_EXT
}

