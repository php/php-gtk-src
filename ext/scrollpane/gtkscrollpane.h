/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef __GTK_SCROLLPANE_H__
#define __GTK_SCROLLPANE_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gtk/gtkobject.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
        

#define GTK_SCROLLPANE_SCROLL_DOWN   1
#define GTK_SCROLLPANE_SCROLL_UP     2
#define GTK_SCROLLPANE_SCROLL_LEFT   3
#define GTK_SCROLLPANE_SCROLL_RIGHT  4

#define GTK_SCROLLPANE_GOTOEDGE_NONE  0
#define GTK_SCROLLPANE_GOTOEDGE_LOWER 1
#define GTK_SCROLLPANE_GOTOEDGE_UPPER 2

        
#define GTK_SCROLLPANE(obj)          GTK_CHECK_CAST (obj, gtk_scrollpane_get_type (), GtkScrollpane)
#define GTK_SCROLLPANE_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_scrollpane_get_type (), GtkScrollpaneClass)
#define GTK_IS_SCROLLPANE(obj)       GTK_CHECK_TYPE (obj, gtk_scrollpane_get_type ())
        
typedef struct _GtkScrollpane 		GtkScrollpane;
typedef struct _GtkScrollpaneClass	GtkScrollpaneClass;
        
struct _GtkScrollpane
{
        GtkWidget widget;
        
        GdkWindow *trough;
        GdkWindow *slider;
        
        /* width/heigth */
        gfloat aspect_ratio; 

        /* update policy (GTK_UPDATE_[CONTINUOUS/DELAYED/DISCONTINUOUS]) */
        guint policy : 2;
        
        gint min_slider_width;
        gint min_slider_height;
        
        gint slider_width;
        gint slider_height;
        gint trough_width;
        gint trough_height;

        gint moving;
        gboolean in_slider;
        gdouble x_offset;
        gdouble y_offset;

        GtkAdjustment *Xadjustment;
        GtkAdjustment *Yadjustment;
};
        
struct _GtkScrollpaneClass
{
        GtkWidgetClass parent_class;
        
        void (* draw_background) (GtkScrollpane *sp);
        void (* draw_trough)     (GtkScrollpane *sp);
        void (* draw_slider)     (GtkScrollpane *sp);
        void (* middle_clicked)  (GtkScrollpane *sp);
        void (* right_clicked)   (GtkScrollpane *sp);
};
        
/*  Member functions */
guint 		gtk_scrollpane_get_type		       (void);
GtkWidget 	*gtk_scrollpane_new		       (GtkAdjustment *Xadjustment,
                                                        GtkAdjustment *Yadjustment,
                                                        gint aspect_ratio);

gboolean        gtk_scrollpane_goto_edge               (GtkScrollpane *sp, gint vertical, gint horizontal);

gboolean        gtk_scrollpane_step(GtkScrollpane *sp, gint direction, gboolean dowrap);

guint          	gtk_range_get_type	               (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
