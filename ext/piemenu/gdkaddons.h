#ifndef __GDK_ADDONS__
#define __GDK_ADDONS__


gboolean gdk_window_have_shape_ext (void);

void
gdk_window_warp_pointer (GdkWindow     *src_window,
			 GdkWindow     *dest_window,
			 gint 	    	src_x,
			 gint		src_y,
			 guint		src_width,
			 guint		src_height,
			 guint		dest_width,
			 guint		dest_height);

gint
gdk_change_active_pointer_grab (GdkEventMask 	event_mask,
				GdkCursor *	cursor,
				guint32		time);

void gdk_window_shape_combine_rectangles (GdkWindow 		*window,
					  GdkRectangle 		*rectangles,
					  gint			n_rectangles,
					  gint			x,
					  gint			y);


#endif
