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
#include <glib-object.h>
#include "gtk/gtkinvisible.h"
#include "gtk/gtktree.h"

extern php_gtk_ext_entry gtk_plus_ext_entry;
#define php_gtk_ext_gtk__ptr &gtk_plus_ext_entry

PHP_GTK_API extern zend_object_handlers *php_gtk_handlers;
PHP_GTK_API extern zend_class_entry *php_gtk_exception_ce;
PHP_GTK_API extern int le_gtk_object;
extern int le_php_gtk_wrapper;
extern int le_gobject;
extern int le_gdk_window;
extern int le_gdk_drawable;
extern int le_gdk_image;
extern int le_gdk_screen;

extern int le_gdk_bitmap;
extern int le_gdk_color;
extern int le_gdk_colormap;
extern int le_gdk_cursor;
extern int le_gdk_visual;
extern int le_gdk_font;
extern int le_gdk_gc;
extern int le_gdk_drag_context;
extern int le_gtk_accel_group;
extern int le_gtk_accel_label;
extern int le_gtk_style;
extern int le_gtk_rc_style;

extern int le_gdk_display;
extern int le_gtk_texttag;
extern int le_gtk_text_layout;
extern int le_gtk_box;
extern int le_gtk_button;
extern int le_gtk_calendar;
extern int le_gtk_check_menu_item;
extern int le_gtk_clist;
extern int le_gtk_color_selection;
extern int le_gtk_combo;
extern int le_gtk_container;
extern int le_gtk_ctree;
extern int le_gtk_item;
extern int le_gtk_curve;
extern int le_gtk_dialog;
extern int le_gtk_drawing_area;
extern int le_gtk_entry;
extern int le_gtk_file_selection;
extern int le_gtk_fixed;
extern int le_gtk_layout;
extern int le_gtk_list;
extern int le_gtk_menu;
extern int le_gtk_menu_item;
extern int le_gtk_menu_shell;
extern int le_gtk_notebook;
extern int le_gtk_option_menu;
extern int le_gtk_paned;
extern int le_gtk_pixmap;
extern int le_gtk_preview;
extern int le_gtk_progress;
extern int le_gtk_range;
extern int le_gtk_ruler;
extern int le_gtk_scale;
extern int le_gtk_scrolled_window;
extern int le_gtk_icon_factory;
extern int le_gtk_im_context;
extern int le_gtk_list_store;
extern int le_gtk_settings;
extern int le_gtk_size_group;
extern int le_gtk_text_buffer;
extern int le_gtk_text_anchor;
extern int le_gtk_text_mark;
extern int le_gtk_text_tag;
extern int le_gtk_tree_model_sort;
extern int le_gtk_tree_selection;
extern int le_gtk_tree_store;


extern int le_gtk_menu_shell;
extern int le_gtk_text_tag_table;
extern int le_gtk_text_child_anchor;
extern int le_gtk_cell_renderer;
extern int le_gtk_tree_view_column;




typedef void (*php_gtk_dtor_t)(void *);

typedef struct _php_gtk_object {
	zend_object zobj;
	void *obj;
	php_gtk_dtor_t dtor;
} php_gtk_object;

static inline GdkAtom php_gdk_atom_get(zval *wrapper)
{
	zval **atom;
	TSRMLS_FETCH();
	
	zend_hash_find(Z_OBJPROP_P(wrapper), "atom", sizeof("atom"), (void**)&atom);
	return (GdkAtom)Z_LVAL_PP(atom);
}

#define PHP_GTK_GET(w)			PHP_GTK_GET_GENERIC(w, GObject*, le_gobject)
#define PHP_GTK_OBJECT_GET(w)		PHP_GTK_GET_GENERIC(w, GtkObject*, le_gtk_object)

#define PHP_GDK_EVENT_GET(w)		PHP_GTK_GET_GENERIC(w, GdkEvent*, le_php_gtk_wrapper)
#define PHP_GDK_WINDOW_GET(w)		PHP_GTK_GET_GENERIC(w, GdkWindow*, le_gdk_window)
#define PHP_GDK_DRAWABLE_GET(w)		PHP_GTK_GET_GENERIC(w, GdkDrawable*, le_gdk_drawable)
#define PHP_GDK_IMAGE_GET(w)		PHP_GTK_GET_GENERIC(w, GdkImage*, le_gdk_image)


#define PHP_GDK_PIXMAP_GET(w)		PHP_GTK_GET_GENERIC(w, GdkPixmap*, le_gdk_window)
#define PHP_GDK_BITMAP_GET(w)		PHP_GTK_GET_GENERIC(w, GdkBitmap*, le_gdk_bitmap)
#define PHP_GDK_COLOR_GET(w)		PHP_GTK_GET_GENERIC(w, GdkColor*, le_gdk_color)
#define PHP_GDK_COLORMAP_GET(w)		PHP_GTK_GET_GENERIC(w, GdkColormap*, le_gdk_colormap)

#define PHP_GDK_DISPLAY_GET(w)		PHP_GTK_GET_GENERIC(w, GdkDisplay*, le_gdk_display)
#define PHP_GTK_TEXTTAG_GET(w)		PHP_GTK_GET_GENERIC(w, GtkTextTag*, le_gtk_texttag)
#define PHP_GTK_BOX_GET(w)		PHP_GTK_GET_GENERIC(w, GtkBox*, le_gtk_box)
#define PHP_GTK_BUTTON_GET(w)         PHP_GTK_GET_GENERIC(w, GtkButton*, le_gtk_button)
#define PHP_GTK_CALENDAR_GET(w)         PHP_GTK_GET_GENERIC(w, GtkCalendar*, le_gtk_calendar)
#define PHP_GTK_CHECK_MENU_ITEM_GET(w)         PHP_GTK_GET_GENERIC(w, GtkCheckMenuItem*, le_gtk_check_menu_item)
#define PHP_GTK_CLIST_GET(w)         PHP_GTK_GET_GENERIC(w, GtkCList*, le_gtk_clist)
#define PHP_GTK_COLOR_SELECTION_GET(w)         PHP_GTK_GET_GENERIC(w, GtkColorSelection*, le_gtk_color_selection)
#define PHP_GTK_COMBO_GET(w)         PHP_GTK_GET_GENERIC(w, GtkCombo*, le_gtk_combo)
#define PHP_GTK_CONTAINER_GET(w)         PHP_GTK_GET_GENERIC(w, GtkContainer*, le_gtk_container)
#define PHP_GTK_CTREE_GET(w)         PHP_GTK_GET_GENERIC(w, GtkCTree*, le_gtk_ctree)
#define PHP_GTK_ITEM_GET(w)         PHP_GTK_GET_GENERIC(w, GtkItem*, le_gtk_item)
#define PHP_GTK_CURVE_GET(w)         PHP_GTK_GET_GENERIC(w, GtkCurve*, le_gtk_curve)
#define PHP_GTK_DIALOG_GET(w)         PHP_GTK_GET_GENERIC(w, GtkDialog*, le_gtk_dialog)
#define PHP_GTK_DRAWING_AREA_GET(w)         PHP_GTK_GET_GENERIC(w, GtkDrawingArea*, le_gtk_drawing_area)
#define PHP_GTK_ENTRY_GET(w)         PHP_GTK_GET_GENERIC(w, GtkEntry*, le_gtk_entry)
#define PHP_GTK_FILE_SELECTION_GET(w)         PHP_GTK_GET_GENERIC(w, GtkFileSelection*, le_gtk_file_selection)

#define PHP_GTK_ICON_FACTORY_GET(w)         PHP_GTK_GET_GENERIC(w, GtkIconFactory*, le_gtk_icon_factory)
#define PHP_GTK_IMCONTEXT_GET(w)         PHP_GTK_GET_GENERIC(w, GtkIMContext*, le_gtk_im_context)
#define PHP_GTK_LIST_STORE_GET(w)         PHP_GTK_GET_GENERIC(w, GtkListStore*, le_gtk_list_store)
#define PHP_GTK_SETTINGS_GET(w)         PHP_GTK_GET_GENERIC(w, GtkSettings*, le_gtk_settings)
#define PHP_GTK_SIZE_GROUP_GET(w)         PHP_GTK_GET_GENERIC(w, GtkSizeGroup*, le_gtk_size_group)
#define PHP_GTK_TEXT_BUFFER_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTextBuffer*, le_gtk_text_buffer)
#define PHP_GTK_TEXT_ANCHOR_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTextAnchor*, le_gtk_text_anchor)
#define PHP_GTK_TEXT_TAG_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTextTag*, le_gtk_text_tag)
#define PHP_GTK_TEXT_MARK_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTextMark*, le_gtk_text_mark)
#define PHP_GTK_TREE_MODEL_SORT_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTreeModelSort*, le_gtk_tree_model_sort)
#define PHP_GTK_TREE_SELECTION_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTreeSelection*, le_gtk_tree_selection)
#define PHP_GTK_TREE_STORE_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTreeStore*, le_gtk_tree_store)




#define PHP_GTK_MENU_SHELL_GET(w)         PHP_GTK_GET_GENERIC(w, GtkMenuShell*, le_gtk_menu_shell)
#define PHP_GTK_TEXT_TAG_TABLE_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTextTagTable*, le_gtk_text_tag_table)
#define PHP_GTK_TEXT_CHILD_ANCHOR_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTextChildAnchor*, le_gtk_text_child_anchor)
#define PHP_GTK_CELL_RENDERER_GET(w)         PHP_GTK_GET_GENERIC(w, GtkCellRenderer*, le_gtk_cell_renderer)
#define PHP_GTK_TREE_VIEW_COLUMN_GET(w)         PHP_GTK_GET_GENERIC(w, GtkTreeViewColumn*, le_gtk_tree_view_column)





#define PHP_GDK_ATOM_GET(w)			(php_gdk_atom_get(w))
#define PHP_GDK_CURSOR_GET(w)		PHP_GTK_GET_GENERIC(w, GdkCursor*, le_gdk_cursor)
#define PHP_GDK_VISUAL_GET(w)		PHP_GTK_GET_GENERIC(w, GdkVisual*, le_gdk_visual)
#define PHP_GDK_FONT_GET(w)			PHP_GTK_GET_GENERIC(w, GdkFont*, le_gdk_font)
#define PHP_GDK_GC_GET(w)			PHP_GTK_GET_GENERIC(w, GdkGC*, le_gdk_gc)
#define PHP_GDK_DRAG_CONTEXT_GET(w) PHP_GTK_GET_GENERIC(w, GdkDragContext*, le_gdk_drag_context)
#define PHP_GTK_SELECTION_DATA_GET(w) PHP_GTK_GET_GENERIC(w, GtkSelectionData*, le_php_gtk_wrapper)
#define PHP_GTK_CTREE_NODE_GET(w)   PHP_GTK_GET_GENERIC(w, GtkCTreeNode*, le_php_gtk_wrapper)
#define PHP_GTK_ACCEL_GROUP_GET(w)  PHP_GTK_GET_GENERIC(w, GtkAccelGroup*, le_gtk_accel_group)

#define PHP_GTK_ACCEL_LABEL_GET(w)  PHP_GTK_GET_GENERIC(w, GtkAccelLabel*, le_gtk_accel_label)
#define PHP_GTK_STYLE_GET(w)        PHP_GTK_GET_GENERIC(w, GtkStyle*, le_gtk_style)
#define PHP_GTK_RC_STYLE_GET(w)  PHP_GTK_GET_GENERIC(w, GtkRcStyle*, le_gtk_rc_style)



PHP_GTK_API extern PHP_GTK_EXPORT_CE(gobject_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_event_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_window_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_drawable_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_image_ce);

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

PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_screen_ce);


PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_selection_data_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_ctree_node_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_accel_group_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_accel_label_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_style_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_rc_style_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_box_child_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_table_child_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_fixed_child_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_clist_row_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_allocation_ce);

PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_display_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_texttag_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_text_layout_ce);

PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_icon_factory_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_im_context_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_list_store_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_settings_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_size_group_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_text_buffer_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_text_anchor_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_text_mark_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_text_tag_ce);

PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_tree_model_sort_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_tree_selection_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_tree_store_ce);




PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_menu_shell_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_text_tag_table_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_text_child_anchor_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_cell_renderer_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtk_tree_view_column_ce);









#include "gen_gtk.h"

PHP_GTK_API zval *php_gtk_new(GtkObject *obj);

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
//zval *php_gdk_drag_context_new(GdkDragContext *context);
zval *php_gtk_selection_data_new(GtkSelectionData *data);
zval *php_gtk_ctree_node_new(GtkCTreeNode *node);
zval *php_gtk_accel_group_new(GtkAccelGroup *group);
zval *php_gtk_accel_label_new(GtkAccelLabel *label);
zval *php_gtk_style_new(GtkStyle *style);
zval *php_gtk_box_child_new(GtkBoxChild *box_child);
zval *php_gtk_table_child_new(GtkTableChild *table_child);
zval *php_gtk_fixed_child_new(GtkFixedChild *fixed_child);
zval *php_gtk_clist_row_new(GtkCListRow *clist_row);

void php_gtk_register_constants(int module_number TSRMLS_DC);
void php_gdk_register_constants(int module_number TSRMLS_DC);
void php_gtk_register_classes(void);
void php_gdk_register_classes(void);
void php_gtk_plus_register_types(int module_number);

int php_gtk_array_to_gchar_array(zval *zvalue, gchar ***gchar_array);
void php_gtk_free_gchar_array(gchar **gchar_array);
void php_gtk_free_gchar_ptr_array(gchar ***gchar_ptr_array, int num_elements);

#endif	/* _PHP_GTK_PLUS_H */
