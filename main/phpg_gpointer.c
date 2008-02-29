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
 * GPointer API and helper functions
 */
PHP_GTK_EXPORT_CE(gpointer_ce) = NULL;

static function_entry gpointer_methods[] = {
#if ZEND_EXTENSION_API_NO > 220051025
    PHP_ME_MAPPING(__construct, no_direct_constructor, NULL, 0)
#else
    PHP_ME_MAPPING(__construct, no_direct_constructor, NULL)
#endif
    { NULL, NULL, NULL }
};

/* {{{ static phpg_free_gpointer_storage() */
static void phpg_free_gpointer_storage(phpg_gpointer_t *object TSRMLS_DC)
{
	zend_hash_destroy(object->zobj.properties);
	FREE_HASHTABLE(object->zobj.properties);
    if (object->zobj.guards) {
        zend_hash_destroy(object->zobj.guards);
        FREE_HASHTABLE(object->zobj.guards);     
    }
	efree(object);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_register_pointer() */
PHP_GTK_API zend_class_entry* phpg_register_pointer(const char *class_name,
                                                  function_entry *class_methods,
                                                  create_object_func_t create_obj_func,
                                                  GType gtype TSRMLS_DC)
{
    return phpg_register_class(class_name, class_methods, gpointer_ce, 0, NULL, create_obj_func ? create_obj_func : phpg_create_gpointer, gtype TSRMLS_CC);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_create_gpointer() */
PHP_GTK_API zend_object_value phpg_create_gpointer(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value zov;
	phpg_gpointer_t *object;

	object = emalloc(sizeof(phpg_gpointer_t));
	phpg_init_object(object, ce);

	object->pointer = NULL;
	object->gtype = 0;

	zov.handlers = &php_gtk_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) phpg_free_gpointer_storage, NULL TSRMLS_CC);

	return zov;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gpointer_check() */
PHP_GTK_API zend_bool phpg_gpointer_check(zval *zobj, GType gtype, zend_bool full_check TSRMLS_DC)
{
    phpg_gpointer_t *pobj;

    phpg_return_val_if_fail(zobj != NULL, FALSE);
    if (full_check) {
        phpg_return_val_if_fail_quiet(Z_TYPE_P(zobj) == IS_OBJECT
                                      && instanceof_function(Z_OBJCE_P(zobj), gpointer_ce TSRMLS_CC), FALSE);
    }

    pobj = phpg_gpointer_get(zobj TSRMLS_CC);

    return (pobj->gtype == gtype);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gpointer_new() */
PHP_GTK_API void phpg_gpointer_new(zval **zobj, GType gtype, gpointer pointer TSRMLS_DC)
{
    zend_class_entry *ce = NULL;
    phpg_gpointer_t *pobj = NULL;

    assert(zobj != NULL);
    if (*zobj == NULL) {
        MAKE_STD_ZVAL(*zobj);
    }
    ZVAL_NULL(*zobj);

    phpg_return_if_fail(gtype != 0);
    phpg_return_if_fail(pointer != NULL);

    ce = g_type_get_qdata(gtype, phpg_class_key);
    if (!ce) {
        ce = gpointer_ce;
    }
    object_init_ex(*zobj, ce);

    pobj = zend_object_store_get_object(*zobj TSRMLS_CC);
    pobj->gtype = gtype;
    pobj->pointer = pointer;
}
/* }}} */

/* {{{ phpg_gpointer_register_self() */
void phpg_gpointer_register_self(TSRMLS_D)
{
	if (gpointer_ce) return;

	gpointer_ce = phpg_register_class("GPointer", gpointer_methods, NULL, 0, NULL, phpg_create_gpointer, G_TYPE_POINTER TSRMLS_CC);
    phpg_register_int_constant(gpointer_ce, "gtype", sizeof("gtype")-1, G_TYPE_POINTER);
}
/* }}} */

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */
