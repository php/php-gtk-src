#ifndef __GTK_PIE_MENU_H__
#define __GTK_PIE_MENU_H__

#include <gtk/gtkmenu.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define  GTK_PIE_MENU_MAX_ITEMS		16
#define  GTK_PIE_MENU_IS_REFINEMENT(X)  (((X) == 1) || ((X) == 2) || ((X) == 4) || \
                                         ((X) == 8) || ((X) == 16))


#define GTK_TYPE_PIE_MENU		(gtk_pie_menu_get_type ())
#define GTK_PIE_MENU(obj)		(GTK_CHECK_CAST ((obj), GTK_TYPE_PIE_MENU, GtkPieMenu))
#define GTK_PIE_MENU_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PIE_MENU, GtkPieMenuClass))
#define GTK_IS_PIE_MENU(obj)		(GTK_CHECK_TYPE ((obj), GTK_TYPE_PIE_MENU))
#define GTK_IS_PIE_MENU_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PIE_MENU))


/*
 * And now - somethink completely different.
 * My humble ascii art.
 *
 *  	     0                          N
 *     15          1	          NNW        NNE
 *   14              2	        NW             NE
 *  13                3	       NWW              NEE
 *  12                4	         W              E
 *  11                5	       SWW              SEE
 *   10              6	        SW             SE
 *  	9          7	          WW         SSE
 *  	     8		                S
 *
 * End of humble ascii art
 * In case you have not recognised: it is compass! ":)"
 */

#if GTK_PIE_MENU_MAX_ITEMS != 16
#error If you change GTK_PIE_MAX_ITEMS constant you have to rewrite GtkPieCompass enum
#endif

typedef enum
{
  GTK_PIE_N		=  0,
  GTK_PIE_NNE		=  1,
  GTK_PIE_NE		=  2,
  GTK_PIE_NEE		=  3,
  GTK_PIE_E		=  4,
  GTK_PIE_SEE		=  5,
  GTK_PIE_SE		=  6,
  GTK_PIE_SSE		=  7,
  GTK_PIE_S		=  8,
  GTK_PIE_SSW		=  9,
  GTK_PIE_SW		= 10,
  GTK_PIE_SWW		= 11,
  GTK_PIE_W 		= 12,
  GTK_PIE_NWW		= 13,
  GTK_PIE_NW 		= 14,
  GTK_PIE_NNW		= 15
} GtkPieCompass;

typedef struct _GtkPieMenu 	GtkPieMenu;
typedef struct _GtkPieMenuClass GtkPieMenuClass;

struct _GtkPieMenu
{
  GtkMenu 	menu;

  GtkWidget	*title;

  guint32	position_mask;
  guint16	refinement;
  guint16	spacing;
  guint16	inner_radius;

  guint		shape_window:1;
  guint		menu_type:1;
};

struct _GtkPieMenuClass
{
  GtkMenuClass 	parent_class;
};


GtkType	   gtk_pie_menu_get_type		  (void);
GtkWidget* gtk_pie_menu_new			  (void);
GtkWidget* gtk_pie_menu_new_with_refinement	  (gint refinement);

/* Wrappers for the Menu Shell operations */

void	   gtk_pie_menu_append		  (GtkPieMenu	       *menu,
					   GtkWidget	       *child);
void	   gtk_pie_menu_put		  (GtkPieMenu	       *menu,
					   GtkWidget	       *child,
					   gint			position);
void	   gtk_pie_menu_put_with_compass  (GtkPieMenu	       *menu,
					   GtkWidget	       *child,
					   GtkPieCompass	compass);
void	   gtk_pie_menu_put_in_refinement (GtkPieMenu	       *menu,
					   GtkWidget	       *child,
					   gint			position,
					   gint			refinement);

void	   gtk_pie_menu_set_refinement 	  (GtkPieMenu		*menu,
					   gint			refinement);
void	   gtk_pie_menu_set_spacing	  (GtkPieMenu           *menu,      
					   gint                 spacing);
void	   gtk_pie_menu_set_radius	  (GtkPieMenu           *menu,      
					   gint                 radius);

/* Display the menu onscreen */
void	   gtk_pie_menu_popup		  (GtkPieMenu	       *menu,
					   guint		button,
					   guint32		activate_time);

void       gtk_pie_menu_set_title         (GtkPieMenu 	       *menu,
					   GtkWidget	       *child);	

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_PIE_MENU_H__ */
