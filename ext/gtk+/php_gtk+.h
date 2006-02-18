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

PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtktreemodelrow_ce);
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gtktreemodelrowiter_ce);

typedef enum { STYLE_COLOR_ARRAY, STYLE_GC_ARRAY, STYLE_PIXMAP_ARRAY } style_helper_type;

PHP_GTK_API int phpg_rectangle_from_zval(zval *value, GdkRectangle *rectangle TSRMLS_DC);
PHP_GTK_API void phpg_create_style_helper(zval **zobj, GtkStyle *style, int type, gpointer array TSRMLS_DC);
PHP_GTK_API int phpg_tree_path_from_zval(const zval *value, GtkTreePath **path TSRMLS_DC);
PHP_GTK_API int phpg_tree_path_to_zval(GtkTreePath *path, zval **value TSRMLS_DC);

int phpg_model_set_row(GtkTreeModel *model, GtkTreeIter *iter, zval *items TSRMLS_DC);
int phpg_model_remove_row(GtkTreeModel *model, GtkTreeIter *iter TSRMLS_DC);
zend_object_iterator* phpg_treemodel_get_iterator(zend_class_entry *ce, zval *object TSRMLS_DC);
void phpg_modelrow_new(zval **zobj, GtkTreeModel *model, GtkTreeIter *iter TSRMLS_DC);

void phpg_atk_register_constants(const char *strip_prefix);
void phpg_pango_register_constants(const char *strip_prefix);
void phpg_gdk_register_constants(const char *strip_prefix);
void phpg_gtk_register_constants(const char *strip_prefix);
void phpg_atk_register_classes(void);
void phpg_pango_register_classes(void);
void phpg_gdk_register_classes(void);
void phpg_gtk_register_classes(void);

void php_gtk_plus_register_types(TSRMLS_D);

#endif	/* _PHP_GTK_PLUS_H */
