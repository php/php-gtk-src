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

#ifndef __GTK_COMBOBUTTON_H__
#define __GTK_COMBOBUTTON_H__

#include <gtk/gtkwidget.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkarrow.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GTK_TYPE_COMBOBUTTON			(gtk_combobutton_get_type ())
#define GTK_COMBOBUTTON(obj)			(GTK_CHECK_CAST ((obj), GTK_TYPE_COMBOBUTTON, GtkComboButton))
#define GTK_COMBOBUTTON_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_COMBOBUTTON, GtkComboButtonClass))
#define GTK_IS_COMBOBUTTON(obj)			(GTK_CHECK_TYPE ((obj), GTK_TYPE_COMBOBUTTON))
#define GTK_IS_COMBOBUTTON_CLASS(klass)		(GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_COMBOBUTTON))

typedef struct _GtkComboButton		GtkComboButton;
typedef struct _GtkComboButtonClass	GtkComboButtonClass;

struct _GtkComboButton
{
	GtkButton button;

	GtkWidget *menu;
	guint x_offset;		/* Menu is offset from x by this value */
	guint y_offset;		/* Menu is offset from y by this value */
	GtkArrowType arrowdir;	/* Direction the arrow is in */

	gboolean changing;	/* Does the contents of the button change */
};

struct _GtkComboButtonClass
{
	GtkButtonClass parent_class;

	void (* menu_pressed) (GtkComboButton *combobutton);
};

GtkType		gtk_combobutton_get_type	(void);
GtkWidget*	gtk_combobutton_new		(void);
GtkWidget*	gtk_combobutton_new_with_label	(const gchar *label);
GtkWidget*	gtk_combobutton_new_with_menu	(GtkWidget *menu,
						 gboolean changing);
GtkWidget*	gtk_combobutton_get_menu	(GtkComboButton *combobutton);
void		gtk_combobutton_set_menu	(GtkComboButton *combobutton,
						 GtkWidget *combomenu);
GtkArrowType	gtk_combobutton_get_arrow_direction	(GtkComboButton *combobutton);
void		gtk_combobutton_set_arrow_direction	(GtkComboButton *combobutton,
							 GtkArrowType arrowtype);
#ifdef __cplusplus
}
#endif


#endif
