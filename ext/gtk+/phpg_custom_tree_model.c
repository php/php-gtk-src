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

#define ITER_IS_VALID(iter, tree_model) \
	 (iter != NULL && iter->stamp == PHPG_CUSTOM_TREE_MODEL(tree_model)->stamp)

static void phpg_custom_tree_model_class_init(PhpGtkCustomTreeModelClass *klass);
static void phpg_custom_tree_model_init(PhpGtkCustomTreeModel *self);
static void phpg_custom_tree_model_iface_init(GtkTreeModelIface *iface);

static GObjectClass *parent_class = NULL;

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
static void phpg_custom_tree_model_finalize(GObject *object);


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

	zend_hash_init(&self->owned_nodes, 10, NULL, ZVAL_PTR_DTOR, 0);
}

static void
phpg_custom_tree_model_class_init(PhpGtkCustomTreeModelClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = phpg_custom_tree_model_finalize;
}

static void phpg_custom_tree_model_finalize(GObject *object)
{
	PhpGtkCustomTreeModel *model = PHPG_CUSTOM_TREE_MODEL(object);

	zend_hash_destroy(&model->owned_nodes);

	/* must chain up */
	(* parent_class->finalize) (object);
}

PhpGtkCustomTreeModel* phpg_custom_tree_model_new(void)
{
	return PHPG_CUSTOM_TREE_MODEL(g_object_new(PHPG_TYPE_CUSTOM_TREE_MODEL, NULL));
}


static guint phpg_custom_tree_model_get_flags(GtkTreeModel *tree_model)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval method_name;
	guint result = 0;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, 0);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), 0);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_get_flags", 0);

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 0, NULL, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		convert_to_long(retval);
		result = Z_LVAL_P(retval);
		zval_ptr_dtor(&retval);
	} else {
		php_error(E_WARNING, "Could not invoke on_get_flags handler");
	}
	zval_ptr_dtor(&wrapper);

	return result;
}


static gint phpg_custom_tree_model_get_n_columns(GtkTreeModel *tree_model)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval method_name;
	gint result = 0;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, 0);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), 0);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_get_n_columns", 0);

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 0, NULL, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		convert_to_long(retval);
		result = Z_LVAL_P(retval);
		zval_ptr_dtor(&retval);
	} else {
		php_error(E_WARNING, "Could not invoke on_get_n_columns handler");
	}
	zval_ptr_dtor(&wrapper);

	return result;
}


static GType phpg_custom_tree_model_get_column_type(GtkTreeModel *tree_model, gint index)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1;
	zval method_name;
	GType result = G_TYPE_INVALID;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, G_TYPE_INVALID);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), G_TYPE_INVALID);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_get_column_type", 0);
	MAKE_STD_ZVAL(arg1);
	ZVAL_LONG(arg1, index);

	args[0] = &arg1;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		result = phpg_gtype_from_zval(retval);
		zval_ptr_dtor(&retval);
	} else {
		php_error(E_WARNING, "Could not invoke on_get_column_type handler");
	}
	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	return result;
}


static gboolean phpg_custom_tree_model_get_iter(GtkTreeModel *tree_model,
												GtkTreeIter *iter,
												GtkTreePath *path)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1 = NULL;
	zval method_name;
	gboolean result = FALSE;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, FALSE);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), FALSE);
	phpg_return_val_if_fail(iter != NULL, FALSE);
	phpg_return_val_if_fail(path != NULL, FALSE);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_get_iter", 0);

	phpg_tree_path_to_zval(path, &arg1 TSRMLS_CC);

	args[0] = &arg1;

	iter->stamp = PHPG_CUSTOM_TREE_MODEL(tree_model)->stamp;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		if (Z_TYPE_P(retval) != IS_NULL) {
			iter->user_data = retval;
			result = TRUE;
			zend_hash_index_update(&PHPG_CUSTOM_TREE_MODEL(tree_model)->owned_nodes,
								   (long)retval, (void *)&retval, sizeof(zval *), NULL);
		} else {
			iter->user_data = NULL;
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error(E_WARNING, "Could not invoke on_get_iter handler");
		iter->user_data = NULL;
	}
	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	return result;
}


static GtkTreePath *phpg_custom_tree_model_get_path(GtkTreeModel *tree_model,
													GtkTreeIter *iter)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1 = NULL;
	zval method_name;
	GtkTreePath *path = NULL;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, NULL);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), NULL);
	phpg_return_val_if_fail(ITER_IS_VALID(iter, tree_model), NULL);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_get_path", 0);

	arg1 = (zval *) iter->user_data;
	if (arg1) {
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	args[0] = &arg1;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		if (phpg_tree_path_from_zval(retval, &path TSRMLS_CC) == FAILURE) {
			php_error(E_WARNING, "Could not convert return value to tree path");
		}
		zval_ptr_dtor(&retval);
	} else {
		php_error(E_WARNING, "Could not invoke on_get_path handler");
	}
	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	return path;
}


static void phpg_custom_tree_model_get_value(GtkTreeModel*tree_model,
											 GtkTreeIter *iter,
											 gint column, GValue *value)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[2], *arg1 = NULL, *arg2 = NULL;
	zval method_name;

	TSRMLS_FETCH();

	phpg_return_if_fail(tree_model != NULL);
	phpg_return_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model));
	phpg_return_if_fail(ITER_IS_VALID(iter, tree_model));

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_get_value", 0);

	g_value_init(value, phpg_custom_tree_model_get_column_type(tree_model, column));

	arg1 = (zval *) iter->user_data;
	if (arg1) {
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	MAKE_STD_ZVAL(arg2);
	ZVAL_LONG(arg2, column);

	args[0] = &arg1;
	args[1] = &arg2;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 2, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		if (Z_TYPE_P(retval) != IS_NULL) {
			if (phpg_gvalue_from_zval(value, retval, 1 TSRMLS_CC) == FAILURE) {
				php_error(E_WARNING, "Could not convert return value to appropriate type");
			}
		}
		zval_ptr_dtor(&retval);
	} else {
		php_error(E_WARNING, "Could not invoke on_get_path handler");
	}

	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);
	zval_ptr_dtor(&arg2);
}


static gboolean phpg_custom_tree_model_iter_next(GtkTreeModel *tree_model,
												 GtkTreeIter *iter)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1 = NULL;
	zval method_name;
	gboolean result = FALSE;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, FALSE);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), FALSE);
	phpg_return_val_if_fail(ITER_IS_VALID(iter, tree_model), FALSE);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_iter_next", 0);

	arg1 = (zval *) iter->user_data;
	if (arg1) {
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	args[0] = &arg1;

	iter->stamp = PHPG_CUSTOM_TREE_MODEL(tree_model)->stamp;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		if (Z_TYPE_P(retval) != IS_NULL) {
			iter->user_data = retval;
			result = TRUE;
			zend_hash_index_update(&PHPG_CUSTOM_TREE_MODEL(tree_model)->owned_nodes,
								   (long)retval, (void *)&retval, sizeof(zval *), NULL);
		} else {
			iter->user_data = NULL;
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error(E_WARNING, "Could not invoke on_iter_next handler");
		iter->user_data = NULL;
	}
	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	return result;
}


static gboolean phpg_custom_tree_model_iter_children(GtkTreeModel *tree_model,
													 GtkTreeIter *iter,
													 GtkTreeIter *parent)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1 = NULL;
	zval method_name;
	gboolean result = FALSE;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, FALSE);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), FALSE);
	phpg_return_val_if_fail(iter != NULL, FALSE);
	phpg_return_val_if_fail(parent == NULL || parent->stamp == PHPG_CUSTOM_TREE_MODEL(tree_model)->stamp, FALSE);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_iter_children", 0);

	if (parent && parent->user_data != NULL) {
		arg1 = (zval *) parent->user_data;
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	args[0] = &arg1;

	iter->stamp = PHPG_CUSTOM_TREE_MODEL(tree_model)->stamp;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		if (Z_TYPE_P(retval) != IS_NULL) {
			iter->user_data = retval;
			result = TRUE;
			zend_hash_index_update(&PHPG_CUSTOM_TREE_MODEL(tree_model)->owned_nodes,
								   (long)retval, (void *)&retval, sizeof(zval *), NULL);
		} else {
			iter->user_data = NULL;
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error(E_WARNING, "Could not invoke on_iter_children handler");
		iter->user_data = NULL;
	}
	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	return result;
}


static gboolean phpg_custom_tree_model_iter_has_child(GtkTreeModel *tree_model,
													  GtkTreeIter *iter)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1 = NULL;
	zval method_name;
	gboolean result = FALSE;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, FALSE);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), FALSE);
	phpg_return_val_if_fail(ITER_IS_VALID(iter, tree_model), FALSE);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_iter_has_child", 0);

	arg1 = (zval *) iter->user_data;
	if (arg1) {
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	args[0] = &arg1;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		result = zval_is_true(retval);
		zval_ptr_dtor(&retval);
	} else {
		php_error(E_WARNING, "Could not invoke on_iter_has_child handler");
	}
	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	return result;
}


static gint phpg_custom_tree_model_iter_n_children(GtkTreeModel *tree_model,
												   GtkTreeIter *iter)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1 = NULL;
	zval method_name;
	guint result = 0;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, 0);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), 0);
	phpg_return_val_if_fail(iter == NULL || iter->stamp == PHPG_CUSTOM_TREE_MODEL(tree_model)->stamp, 0);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_iter_n_children", 0);

	if (iter && iter->user_data != NULL) {
		arg1 = (zval *) iter->user_data;
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	args[0] = &arg1;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		convert_to_long(retval);
		result = Z_LVAL_P(retval);
		zval_ptr_dtor(&retval);
	} else {
		php_error(E_WARNING, "Could not invoke on_iter_n_children handler");
	}
	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	return result;
}


static gboolean phpg_custom_tree_model_iter_nth_child(GtkTreeModel *tree_model,
													  GtkTreeIter  *iter,
													  GtkTreeIter  *parent,
													  gint n)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[2], *arg1 = NULL, *arg2 = NULL;
	zval method_name;
	gboolean result = FALSE;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, FALSE);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), FALSE);
	phpg_return_val_if_fail(iter != NULL, FALSE);
	phpg_return_val_if_fail(parent == NULL || parent->stamp == PHPG_CUSTOM_TREE_MODEL(tree_model)->stamp, FALSE);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_iter_nth_child", 0);

	if (parent && parent->user_data != NULL) {
		arg1 = (zval *) parent->user_data;
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	MAKE_STD_ZVAL(arg2);
	ZVAL_LONG(arg2, n);

	args[0] = &arg1;
	args[1] = &arg2;

	iter->stamp = PHPG_CUSTOM_TREE_MODEL(tree_model)->stamp;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 2, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		if (Z_TYPE_P(retval) != IS_NULL) {
			iter->user_data = retval;
			result = TRUE;
			zend_hash_index_update(&PHPG_CUSTOM_TREE_MODEL(tree_model)->owned_nodes,
								   (long)retval, (void *)&retval, sizeof(zval *), NULL);
		} else {
			iter->user_data = NULL;
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error(E_WARNING, "Could not invoke on_nth_child handler");
		iter->user_data = NULL;
	}
	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);
	zval_ptr_dtor(&arg2);

	return result;
}


static gboolean phpg_custom_tree_model_iter_parent(GtkTreeModel *tree_model,
												   GtkTreeIter *iter,
												   GtkTreeIter *child)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1 = NULL;
	zval method_name;
	gboolean result = FALSE;

	TSRMLS_FETCH();

	phpg_return_val_if_fail(tree_model != NULL, FALSE);
	phpg_return_val_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model), FALSE);
	phpg_return_val_if_fail(iter != NULL, FALSE);
	phpg_return_val_if_fail(ITER_IS_VALID(child, tree_model), FALSE);

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_iter_parent", 0);

	if (child && child->user_data != NULL) {
		arg1 = (zval *) child->user_data;
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	args[0] = &arg1;

	iter->stamp = PHPG_CUSTOM_TREE_MODEL(tree_model)->stamp;

	if (call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC) == SUCCESS
		&& retval) {
		if (Z_TYPE_P(retval) != IS_NULL) {
			iter->user_data = retval;
			result = TRUE;
			zend_hash_index_update(&PHPG_CUSTOM_TREE_MODEL(tree_model)->owned_nodes,
								   (long)retval, (void *)&retval, sizeof(zval *), NULL);
		} else {
			iter->user_data = NULL;
			zval_ptr_dtor(&retval);
		}
	} else {
		php_error(E_WARNING, "Could not invoke on_iter_parent handler");
		iter->user_data = NULL;
	}
	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	return result;
}


static void phpg_custom_tree_model_unref_node(GtkTreeModel *tree_model,
											  GtkTreeIter *iter)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1 = NULL;
	zval method_name;

	TSRMLS_FETCH();

	phpg_return_if_fail(tree_model != NULL);
	phpg_return_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model));
	phpg_return_if_fail(ITER_IS_VALID(iter, tree_model));

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_unref_node", 0);

	if (iter && iter->user_data != NULL) {
		arg1 = (zval *) iter->user_data;
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	args[0] = &arg1;

	call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC);

	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	if (retval) {
		zval_ptr_dtor(&retval);
	}
}


static void phpg_custom_tree_model_ref_node(GtkTreeModel *tree_model,
											GtkTreeIter *iter)
{
	zval *wrapper = NULL;
	zval *retval = NULL;
	zval **args[1], *arg1 = NULL;
	zval method_name;

	TSRMLS_FETCH();

	phpg_return_if_fail(tree_model != NULL);
	phpg_return_if_fail(PHPG_IS_CUSTOM_TREE_MODEL(tree_model));
	phpg_return_if_fail(ITER_IS_VALID(iter, tree_model));

	phpg_gobject_new(&wrapper, (GObject *) tree_model TSRMLS_CC);
	ZVAL_STRING(&method_name, "on_ref_node", 0);

	if (iter && iter->user_data != NULL) {
		arg1 = (zval *) iter->user_data;
		zval_add_ref(&arg1);
	} else {
		MAKE_ZVAL_IF_NULL(arg1);
		ZVAL_NULL(arg1);
	}

	args[0] = &arg1;

	call_user_function_ex(EG(function_table), &wrapper, &method_name, &retval, 1, args, 0, NULL TSRMLS_CC);

	zval_ptr_dtor(&wrapper);
	zval_ptr_dtor(&arg1);

	if (retval) {
		zval_ptr_dtor(&retval);
	}
}


#endif
