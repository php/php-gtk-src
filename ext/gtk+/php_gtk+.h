/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001,2002 Andrei Zmievski <andrei@php.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
/* $Id$: */

#ifndef _PHP_GTK_PLUS_H
#define _PHP_GTK_PLUS_H

#include <gtk/gtk.h>
#include "gtk/gtkinvisible.h"

extern php_gtk_ext_entry gtk_plus_ext_entry;
#define php_gtk_ext_gtk__ptr &gtk_plus_ext_entry

typedef enum { STYLE_COLOR_ARRAY, STYLE_GC_ARRAY, STYLE_PIXMAP_ARRAY} style_helper_type;

PHP_GTK_API int phpg_rectangle_from_zval(zval *value, GdkRectangle *rectangle TSRMLS_DC);
PHP_GTK_API void phpg_create_style_helper(zval **zobj, GtkStyle *style, int type, gpointer array TSRMLS_DC);

void phpg_atk_register_constants(const char *strip_prefix);
void phpg_pango_register_constants(const char *strip_prefix);
void phpg_gdk_register_constants(const char *strip_prefix);
void phpg_gtk_register_constants(const char *strip_prefix);
void phpg_atk_register_classes(void);
void phpg_pango_register_classes(void);
void phpg_gdk_register_classes(void);
void phpg_gtk_register_classes(void);

void php_gtk_plus_register_types();

#if 0
PHP_GTK_API extern int le_gtk_object;
PHP_GTK_API extern int le_php_gtk_wrapper;
PHP_GTK_API extern int le_gdk_window;
PHP_GTK_API extern int le_gdk_bitmap;
PHP_GTK_API extern int le_gdk_color;
PHP_GTK_API extern int le_gdk_colormap;
PHP_GTK_API extern int le_gdk_cursor;
PHP_GTK_API extern int le_gdk_visual;
PHP_GTK_API extern int le_gdk_font;
PHP_GTK_API extern int le_gdk_gc;
PHP_GTK_API extern int le_gdk_drag_context;
PHP_GTK_API extern int le_gtk_accel_group;
PHP_GTK_API extern int le_gtk_style;

static inline GdkAtom php_gdk_atom_get(zval *wrapper)
{
	zval **atom;
	TSRMLS_FETCH();
	
	zend_hash_find(Z_OBJPROP_P(wrapper), "atom", sizeof("atom"), (void**)&atom);
	return (GdkAtom)Z_LVAL_PP(atom);
}

#define PHP_GDK_EVENT_GET(w)		PHP_GTK_GET_GENERIC(w, GdkEvent*, le_php_gtk_wrapper)
#define PHP_GDK_WINDOW_GET(w)		PHP_GTK_GET_GENERIC(w, GdkWindow*, le_gdk_window)
#define PHP_GDK_PIXMAP_GET(w)		PHP_GTK_GET_GENERIC(w, GdkPixmap*, le_gdk_window)
#define PHP_GDK_BITMAP_GET(w)		PHP_GTK_GET_GENERIC(w, GdkBitmap*, le_gdk_bitmap)
#define PHP_GDK_COLOR_GET(w)		PHP_GTK_GET_GENERIC(w, GdkColor*, le_gdk_color)
#define PHP_GDK_COLORMAP_GET(w)		PHP_GTK_GET_GENERIC(w, GdkColormap*, le_gdk_colormap)
#define PHP_GDK_ATOM_GET(w)			(php_gdk_atom_get(w))
#define PHP_GDK_CURSOR_GET(w)		PHP_GTK_GET_GENERIC(w, GdkCursor*, le_gdk_cursor)
#define PHP_GDK_VISUAL_GET(w)		PHP_GTK_GET_GENERIC(w, GdkVisual*, le_gdk_visual)
#define PHP_GDK_FONT_GET(w)			PHP_GTK_GET_GENERIC(w, GdkFont*, le_gdk_font)
#define PHP_GDK_GC_GET(w)			PHP_GTK_GET_GENERIC(w, GdkGC*, le_gdk_gc)
#define PHP_GDK_DRAG_CONTEXT_GET(w) PHP_GTK_GET_GENERIC(w, GdkDragContext*, le_gdk_drag_context)
#define PHP_GTK_SELECTION_DATA_GET(w) PHP_GTK_GET_GENERIC(w, GtkSelectionData*, le_php_gtk_wrapper)
#define PHP_GTK_CTREE_NODE_GET(w)   PHP_GTK_GET_GENERIC(w, GtkCTreeNode*, le_php_gtk_wrapper)
#define PHP_GTK_ACCEL_GROUP_GET(w)  PHP_GTK_GET_GENERIC(w, GtkAccelGroup*, le_gtk_accel_group)
#define PHP_GTK_STYLE_GET(w)        PHP_GTK_GET_GENERIC(w, GtkStyle*, le_gtk_style)

PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_event_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_window_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_pixmap_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_bitmap_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_color_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_colormap_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_atom_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_cursor_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_visual_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_font_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_gc_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_drag_context_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_selection_data_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_ctree_node_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_accel_group_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_style_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_box_child_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_table_child_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_fixed_child_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_clist_row_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_allocation_ce);

/* #include "gen_gtk.h" */

zval *php_gdk_event_new(GdkEvent *event);
zval *php_gdk_window_new(GdkWindow *window);
zval *php_gdk_pixmap_new(GdkPixmap *pixmap);
zval *php_gdk_bitmap_new(GdkBitmap *bitmap);
zval *php_gdk_color_new(GdkColor *color);
zval *php_gdk_colormap_new(GdkColormap *cmap);
zval *php_gdk_atom_new(GdkAtom atom);
zval *php_gdk_cursor_new(GdkCursor *cursor);
zval *php_gdk_visual_new(GdkVisual *visual);
zval *php_gdk_font_new(GdkFont *font);
zval *php_gdk_gc_new(GdkGC *gc);
zval *php_gdk_drag_context_new(GdkDragContext *context);
zval *php_gtk_selection_data_new(GtkSelectionData *data);
zval *php_gtk_ctree_node_new(GtkCTreeNode *node);
zval *php_gtk_accel_group_new(GtkAccelGroup *group);
zval *php_gtk_style_new(GtkStyle *style);
zval *php_gtk_box_child_new(GtkBoxChild *box_child);
zval *php_gtk_table_child_new(GtkTableChild *table_child);
zval *php_gtk_fixed_child_new(GtkFixedChild *fixed_child);
zval *php_gtk_clist_row_new(GtkCListRow *clist_row);

int php_gtk_array_to_gchar_array(zval *zvalue, gchar ***gchar_array);
void php_gtk_free_gchar_array(gchar **gchar_array);
void php_gtk_free_gchar_ptr_array(gchar ***gchar_ptr_array, int num_elements);

#endif

#endif	/* _PHP_GTK_PLUS_H */
