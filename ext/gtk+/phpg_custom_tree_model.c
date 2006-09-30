/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001-2006 Andrei Zmievski <andrei@php.net>
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

#include "php_gtk.h"

#if HAVE_PHP_GTK

#include "phpg_custom_tree_model.h"

static void phpg_custom_tree_model_class_init(PhpGtkCustomTreeModelClass *klass);
static void phpg_custom_tree_model_init(PhpGtkCustomTreeModel *self);
static void phpg_custom_tree_model_iface_init(GtkTreeModelIface *iface);

GType
phpg_custom_tree_model_get_type(void)
{   
    static GType object_type = 0;

    if (!object_type) {
    static const GTypeInfo object_info = {
        sizeof(PhpGtkCustomTreeModelClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) phpg_custom_tree_model_class_init,
        NULL, /* class_finalize */
        NULL, /* class_data */
        sizeof(PhpGtkCustomTreeModel),
        0, /* n_preallocs */
        (GInstanceInitFunc) phpg_custom_tree_model_init,
    };
    static const GInterfaceInfo tree_model_info = {
        (GInterfaceInitFunc) phpg_custom_tree_model_iface_init,
        NULL,
        NULL,
    };

    object_type = g_type_register_static(G_TYPE_OBJECT,
                         "PhpGtkCustomTreeModel",
                         &object_info, 0);
    g_type_add_interface_static(object_type,
                    GTK_TYPE_TREE_MODEL,
                    &tree_model_info);
    }
    return object_type;
}

static guint phpg_custom_tree_model_get_flags(GtkTreeModel *tree_model);
static gint phpg_custom_tree_model_get_n_columns(GtkTreeModel *tree_model);
static GType phpg_custom_tree_model_get_column_type(GtkTreeModel *tree_model, gint index);
static gboolean phpg_custom_tree_model_get_iter(GtkTreeModel *tree_model,
												GtkTreeIter *iter,
												GtkTreePath *path);
static GtkTreePath *phpg_custom_tree_model_get_path(GtkTreeModel *tree_model,
													GtkTreeIter *iter);
static void phpg_custom_tree_model_get_value(GtkTreeModel*tree_model,
											 GtkTreeIter *iter,
											 gint column, GValue *value);
static gboolean phpg_custom_tree_model_iter_next(GtkTreeModel *tree_model,
												 GtkTreeIter *iter);
static gboolean phpg_custom_tree_model_iter_children(GtkTreeModel *tree_model,
													 GtkTreeIter *iter,
													 GtkTreeIter *parent);
static gboolean phpg_custom_tree_model_iter_has_child(GtkTreeModel *tree_model,
													  GtkTreeIter *iter);
static gint phpg_custom_tree_model_iter_n_children(GtkTreeModel *tree_model,
												   GtkTreeIter *iter);
static gboolean phpg_custom_tree_model_iter_nth_child(GtkTreeModel *tree_model,
													  GtkTreeIter  *iter,
													  GtkTreeIter  *parent,
													  gint n);
static gboolean phpg_custom_tree_model_iter_parent(GtkTreeModel *tree_model,
												   GtkTreeIter *iter,
												   GtkTreeIter *child);
static void phpg_custom_tree_model_unref_node(GtkTreeModel *tree_model,
											  GtkTreeIter *iter);
static void phpg_custom_tree_model_ref_node(GtkTreeModel *tree_model,
											GtkTreeIter *iter);

static void
phpg_custom_tree_model_iface_init(GtkTreeModelIface *iface)
{ 
  iface->get_flags = phpg_custom_tree_model_get_flags;
  iface->get_n_columns = phpg_custom_tree_model_get_n_columns;
  iface->get_column_type = phpg_custom_tree_model_get_column_type;
  iface->get_iter = phpg_custom_tree_model_get_iter;
  iface->get_path = phpg_custom_tree_model_get_path;
  iface->get_value = phpg_custom_tree_model_get_value;
  iface->iter_next = phpg_custom_tree_model_iter_next;
  iface->iter_children = phpg_custom_tree_model_iter_children;
  iface->iter_has_child = phpg_custom_tree_model_iter_has_child;
  iface->iter_n_children = phpg_custom_tree_model_iter_n_children;
  iface->iter_nth_child = phpg_custom_tree_model_iter_nth_child;
  iface->iter_parent = phpg_custom_tree_model_iter_parent;
  iface->ref_node = phpg_custom_tree_model_ref_node;
  iface->unref_node = phpg_custom_tree_model_unref_node;
}

static void
phpg_custom_tree_model_init(PhpGtkCustomTreeModel *self)
{
	do {
		self->stamp = g_random_int();
	} while (self->stamp == 0);
}

static void
phpg_custom_tree_model_class_init(PhpGtkCustomTreeModelClass *klass)
{
    return NULL;
}

#endif
