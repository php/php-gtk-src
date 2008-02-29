/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001-2008 Andrei Zmievski <andrei@php.net>
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
    zval *replace_object;
	int connect_type;
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
    if (phpg_closure->replace_object) {
        zval_ptr_dtor(&phpg_closure->replace_object);
    }
    efree(phpg_closure->src_filename);

    phpg_closure->callback = NULL;
    phpg_closure->user_args = NULL;
    phpg_closure->replace_object = NULL;
    phpg_closure->connect_type = PHPG_CONNECT_NORMAL;
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
	char *callback_name;
    zval ***params = NULL;
    zval *retval = NULL;
    uint n_params = 0, i;
	TSRMLS_FETCH();

	if (!zend_is_callable(phpg_closure->callback, 0, &callback_name)) {
		if (phpg_closure->src_filename)
            php_error(E_WARNING,
                      "Unable to invoke signal callback '%s' specified in %s on line %d",
                      callback_name, phpg_closure->src_filename, phpg_closure->src_lineno);
		else
			php_error(E_WARNING, "Unable to invoke signal callback '%s'", callback_name);
		efree(callback_name);
		return;
	}

    if (phpg_closure->connect_type == PHPG_CONNECT_SIMPLE) {
        /* we don't use any signal params for simple connections */
        n_param_values = 0;
    } else {
        if (phpg_closure->connect_type == PHPG_CONNECT_OBJECT) {
            /* skip first parameter */
            n_param_values--;
            param_values++;
        }
        n_params = n_param_values;
    }
    
    if (phpg_closure->user_args) {
        n_params += zend_hash_num_elements(Z_ARRVAL_P(phpg_closure->user_args));
    }

	params = (zval ***)emalloc(n_params * sizeof(zval **));
    i = 0;

    if (phpg_closure->connect_type == PHPG_CONNECT_REPLACE) {
        params[i++] = &phpg_closure->replace_object;
    }

    for ( ; i < n_param_values; i++) {
        params[i] = (zval **) emalloc(sizeof(zval *));
        *(params[i]) = NULL;
        if (phpg_gvalue_to_zval(&param_values[i], params[i], FALSE, TRUE TSRMLS_CC) != SUCCESS) {
            goto err_marshal;
        }
    }

    if (phpg_closure->user_args) {
        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(phpg_closure->user_args));
             zend_hash_get_current_data(Z_ARRVAL_P(phpg_closure->user_args), (void **)&params[i]) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(phpg_closure->user_args)), i++);
        /* empty body */
    }
    assert(i == n_params);

    call_user_function_ex(EG(function_table), NULL, phpg_closure->callback,
                          &retval, n_params, params, 0, NULL TSRMLS_CC);

	if (retval) {
		if (return_value) {
			if (phpg_gvalue_from_zval(return_value, &retval, TRUE TSRMLS_CC) == FAILURE) {
                php_error(E_WARNING, "Could not convert return value of signal callback '%s' to '%s'",
                          callback_name, g_type_name(G_VALUE_TYPE(return_value)));
            }
        }
		zval_ptr_dtor(&retval);
	}

err_marshal:
    efree(callback_name);
    i = (phpg_closure->connect_type == PHPG_CONNECT_REPLACE) ? 1 : 0;
    for ( ; i < n_param_values; i++) {
        zval_ptr_dtor(params[i]);
        efree(params[i]);
    }
    efree(params);

    phpg_handle_marshaller_exception(TSRMLS_C);
}

PHP_GTK_API GClosure* phpg_closure_new(zval *callback, zval *user_args, int connect_type, zval *replace_object TSRMLS_DC)
{
    GClosure *closure;
    phpg_closure_t *phpg_closure;

    phpg_return_val_if_fail(callback != NULL, NULL);
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

    if (replace_object) {
        zval_add_ref(&replace_object);
        phpg_closure->replace_object = replace_object;
    } else {
        phpg_closure->replace_object = NULL;
    }

    phpg_closure->connect_type = connect_type;

    return closure;
}

PHP_GTK_API void phpg_cb_data_destroy(gpointer data)
{
    phpg_cb_data_t *cbd = (phpg_cb_data_t *) data;

    if (!cbd) return;

    zval_ptr_dtor(&cbd->callback);
    if (cbd->user_args) {
        zval_ptr_dtor(&cbd->user_args);
    }
    efree(cbd->src_filename);
    efree(cbd);
}

static void phpg_signal_class_closure_marshal(GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data)
{
    GObject *object;
    GSignalInvocationHint *hint = (GSignalInvocationHint *)invocation_hint;
    zval *php_object = NULL;
    gchar *method_name, *tmp, *lc_method_name;
    gsize method_name_len;
    zval ***params = NULL;
    zval *retval = NULL;
    zval *item, z_method_name;
    phpg_gboxed_t *boxed_item;
    int i, k;

	TSRMLS_FETCH();

    phpg_return_if_fail(invocation_hint != NULL);
    /* verify that first parameter is an object */
    object = g_value_get_object(&param_values[0]);
    phpg_return_if_fail(object != NULL && G_IS_OBJECT(object));

    /* create wrapper for the object */
    phpg_gobject_new(&php_object, object TSRMLS_CC);
    if (Z_TYPE_P(php_object) == IS_NULL) {
        zval_ptr_dtor(&php_object);
        return;
    }

    method_name = g_strconcat("__do_", g_signal_name(hint->signal_id), NULL);
    for (tmp = method_name; *tmp != '\0'; tmp++) {
        if (*tmp == '-') *tmp = '_';
    }

    method_name_len = strlen(method_name);
    lc_method_name = g_ascii_strdown(method_name, method_name_len);
    if (!zend_hash_exists(&Z_OBJCE_P(php_object)->function_table, lc_method_name, method_name_len+1)) {
        union _zend_function *func = NULL;

        g_free(lc_method_name);
        
        if (Z_OBJ_HT_P(php_object)->get_method != NULL
            && (func = Z_OBJ_HT_P(php_object)->get_method(&php_object, method_name, method_name_len TSRMLS_CC)) != NULL) {
            if (func->type == ZEND_INTERNAL_FUNCTION
                && ((zend_internal_function*)func)->handler == zend_std_call_user_call
               ) {
                efree(((zend_internal_function*)func)->function_name);
                efree(func);
                zval_ptr_dtor(&php_object);
                return;
            }
        }
        zval_ptr_dtor(&php_object);
        return;
    }
    g_free(lc_method_name);

	params = (zval ***)emalloc((n_param_values-1) * sizeof(zval **));
    for (i = 0; i < (int)n_param_values-1; i++) {
        params[i] = (zval **) emalloc(sizeof(zval *));
        *(params[i]) = NULL;
        if (phpg_gvalue_to_zval(&param_values[i+1], params[i], FALSE, TRUE TSRMLS_CC) != SUCCESS) {
            goto err_class_closure_marshal;
        }
    }

    ZVAL_STRINGL(&z_method_name, method_name, method_name_len, 0);
    call_user_function_ex(EG(function_table), &php_object, &z_method_name,
                          &retval, n_param_values-1, params, 0, NULL TSRMLS_CC);


    for (k = 0; k < i; k++) {
        item = *params[k];
        if (Z_TYPE_P(item) == IS_OBJECT &&
            instanceof_function(Z_OBJCE_P(item), gboxed_ce TSRMLS_CC) && Z_REFCOUNT_P(item) > 1) {
            boxed_item = phpg_gboxed_get(item TSRMLS_CC);
            if (!boxed_item->free_on_destroy) {
                boxed_item->boxed = g_boxed_copy(boxed_item->gtype, boxed_item->boxed);
                boxed_item->free_on_destroy = TRUE;
            }
        }
    }

	if (retval) {
		if (return_value) {
			if (phpg_gvalue_from_zval(return_value, &retval, TRUE TSRMLS_CC) == FAILURE) {
                php_error(E_WARNING, "Could not convert return value of custom signal action '%s' to '%s'",
                          method_name, g_type_name(G_VALUE_TYPE(return_value)));
            }
        }
		zval_ptr_dtor(&retval);
	}

    i--; // adjust for cleanup

err_class_closure_marshal:
    for ( ; i >= 0; i--) {
        zval_ptr_dtor(params[i]);
        efree(params[i]);
    }
    efree(params);
    g_free(method_name);
    zval_ptr_dtor(&php_object);
}


PHP_GTK_API GClosure* phpg_get_signal_class_closure()
{
    static GClosure *closure = NULL;

    if (closure == NULL) {
        closure = g_closure_new_simple(sizeof(GClosure), NULL);
        g_closure_set_marshal(closure, phpg_signal_class_closure_marshal);

        g_closure_ref(closure);
        g_closure_sink(closure);
    }

    return closure;
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */

