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
 * Boxed types API and helper functions
 */

static GQuark custom_boxed_marshal_key = 0;

PHP_GTK_EXPORT_CE(gboxed_ce) = NULL;

/* {{{ PHP_METHOD(GBoxed, copy) */
PHP_METHOD(GBoxed, copy)
{
    phpg_gboxed_t *pobj = NULL;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "")) {
        return;
    }

    pobj = zend_object_store_get_object(this_ptr TSRMLS_CC);
    phpg_gboxed_new(&return_value, pobj->gtype, pobj->boxed, TRUE, TRUE TSRMLS_CC);
}
/* }}} */

static zend_function_entry gboxed_methods[] = {
    PHP_ME(GBoxed, copy, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ static phpg_free_gboxed_storage() */
static void phpg_free_gboxed_storage(phpg_gboxed_t *object TSRMLS_DC)
{
	zend_hash_destroy(object->zobj.properties);
	FREE_HASHTABLE(object->zobj.properties);
    if (object->zobj.guards) {
        zend_hash_destroy(object->zobj.guards);
        FREE_HASHTABLE(object->zobj.guards);     
    }
	if (object->boxed && object->free_on_destroy) {
		g_boxed_free(object->gtype, object->boxed);
		object->boxed = NULL;
	}
	efree(object);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_create_gboxed() */
PHP_GTK_API zend_object_value phpg_create_gboxed(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value zov;
	phpg_gboxed_t *object;

	object = emalloc(sizeof(phpg_gboxed_t));
	phpg_init_object(object, ce);

	object->boxed = NULL;
	object->gtype = 0;
	object->free_on_destroy = FALSE;

	zov.handlers = &php_gtk_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) phpg_free_gboxed_storage, NULL TSRMLS_CC);

	return zov;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_register_boxed() */
PHP_GTK_API zend_class_entry* phpg_register_boxed(const char *class_name,
                                                  function_entry *class_methods,
                                                  prop_info_t *prop_info,
                                                  create_object_func_t create_obj_func,
                                                  GType gtype TSRMLS_DC)
{
    return phpg_register_class(class_name, class_methods, gboxed_ce, 0, prop_info, create_obj_func ? create_obj_func : phpg_create_gboxed, gtype TSRMLS_CC);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gboxed_check() */
PHP_GTK_API zend_bool phpg_gboxed_check(zval *zobj, GType gtype, zend_bool full_check TSRMLS_DC)
{
    phpg_gboxed_t *pobj;

    phpg_return_val_if_fail(zobj != NULL, FALSE);
    if (full_check) {
        phpg_return_val_if_fail_quiet(Z_TYPE_P(zobj) == IS_OBJECT
                                      && instanceof_function(Z_OBJCE_P(zobj), gboxed_ce TSRMLS_CC), FALSE);
    }

    pobj = phpg_gboxed_get(zobj TSRMLS_CC);

    return (pobj->gtype == gtype);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gboxed_new() */
PHP_GTK_API void phpg_gboxed_new(zval **zobj, GType gtype, gpointer boxed, gboolean copy, gboolean own_ref TSRMLS_DC)
{
    zend_class_entry *ce = NULL;
    phpg_gboxed_t *pobj = NULL;

    assert(zobj != NULL);
    if (*zobj == NULL) {
        MAKE_STD_ZVAL(*zobj);
    }
    ZVAL_NULL(*zobj);

    phpg_return_if_fail(gtype != 0);
    phpg_return_if_fail_quiet(boxed != NULL);
    phpg_return_if_fail(!copy || (copy && own_ref));

    ce = g_type_get_qdata(gtype, phpg_class_key);
    if (!ce) {
        ce = gboxed_ce;
    }
    object_init_ex(*zobj, ce);

    if (copy) {
        boxed = g_boxed_copy(gtype, boxed);
    }
    pobj = zend_object_store_get_object(*zobj TSRMLS_CC);
    pobj->gtype = gtype;
    pobj->boxed = boxed;
    pobj->free_on_destroy = own_ref;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gboxed_register_custom */
PHP_GTK_API void phpg_gboxed_register_custom(GType type, boxed_from_zval_t from_func, boxed_to_zval_t to_func)
{
    phpg_gboxed_marshal_t *gbm;

    if (!custom_boxed_marshal_key) {
        custom_boxed_marshal_key = g_quark_from_static_string("phpg-custom-boxed-marshal");
    }

    /*
     * Use GLib's memory allocation so that it gets cleaned up, since there is
     * no destroy notify callback for type qdata.
     */
    gbm = g_new(phpg_gboxed_marshal_t, 1);
    gbm->to_zval   = to_func;
    gbm->from_zval = from_func;

    g_type_set_qdata(type, custom_boxed_marshal_key, gbm);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gboxed_lookup_custom */
PHP_GTK_API phpg_gboxed_marshal_t* phpg_gboxed_lookup_custom(GType type)
{
    return (phpg_gboxed_marshal_t *) g_type_get_qdata(type, custom_boxed_marshal_key);
}
/* }}} */

/* {{{ phpg_gboxed_register_self() */
void phpg_gboxed_register_self(TSRMLS_D)
{
	if (gboxed_ce) return;

	gboxed_ce = phpg_register_class("GBoxed", gboxed_methods, NULL, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS, NULL, phpg_create_gboxed, G_TYPE_BOXED TSRMLS_CC);
    phpg_register_int_constant(gboxed_ce, "gtype", sizeof("gtype")-1, G_TYPE_BOXED);
}
/* }}} */

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */
