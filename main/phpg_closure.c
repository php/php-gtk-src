/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001-2004 Andrei Zmievski <andrei@php.net>
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

#include "php_gtk.h"

#if HAVE_PHP_GTK

/*
 * Closures API and helper functions
 */

struct _phpg_closure_t {
	GClosure closure;
	zval *callback;
	zval *user_args;
	zend_bool use_signal_object;
	char *src_filename;
	uint src_lineno;
};

static void phpg_closure_invalidate(gpointer data, GClosure *closure)
{
    phpg_closure_t *phpg_closure = (phpg_closure_t *)closure;

    zval_ptr_dtor(&phpg_closure->callback);
    if (phpg_closure->user_args) {
        zval_ptr_dtor(&phpg_closure->user_args);
    }
    efree(phpg_closure->src_filename);

    phpg_closure->callback = NULL;
    phpg_closure->user_args = NULL;
    phpg_closure->use_signal_object = 0;
    phpg_closure->src_filename = NULL;
    phpg_closure->src_lineno = 0;
}

static void phpg_closure_marshal(GClosure *closure,
                                 GValue *return_value,
                                 guint n_param_values,
                                 const GValue *param_values,
                                 gpointer invocation_hint,
                                 gpointer marshal_data)
{
    phpg_closure_t *phpg_closure = (phpg_closure_t *)closure;

    /* TODO */
}

PHP_GTK_API GClosure* phpg_closure_new(zval *callback, zval *user_args, zend_bool use_signal_object TSRMLS_DC)
{
    GClosure *closure;
    phpg_closure_t *phpg_closure;

    g_return_val_if_fail(callback != NULL, NULL);
    closure = g_closure_new_simple(sizeof(phpg_closure_t), NULL);
    g_closure_add_invalidate_notifier(closure, NULL, phpg_closure_invalidate);
    g_closure_set_marshal(closure, phpg_closure_marshal);

    phpg_closure = (phpg_closure_t *) closure;
    zval_add_ref(&callback);
    phpg_closure->callback = callback;
	phpg_closure->src_filename = estrdup(zend_get_executed_filename(TSRMLS_C));
	phpg_closure->src_lineno = zend_get_executed_lineno(TSRMLS_C);

    if (user_args) {
        zval_add_ref(&user_args);
        if (Z_TYPE_P(user_args) != IS_ARRAY) {
            convert_to_array(user_args);
        }
        phpg_closure->user_args = user_args;
    } else {
        phpg_closure->user_args = NULL;
    }

    phpg_closure->use_signal_object = use_signal_object;

    return closure;
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */

