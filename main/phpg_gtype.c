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
 * GType PHP class definition
 */

typedef struct {
	PHPG_OBJ_HEADER
	GType type;
} php_gtype_t;

static PHP_METHOD(GType, __construct);
static int gtype_type_read(void *object, zval *return_value);

static zend_function_entry gtype_methods[] = {
	ZEND_ME(GType, __construct, NULL, ZEND_ACC_PRIVATE)
	{NULL, NULL, NULL}
};

static prop_info_t gtype_props_info[] = {
	{ "type", gtype_type_read, NULL },
	{ NULL, NULL, NULL },
};

PHP_GTK_EXPORT_CE(gtype_ce) = NULL;

static void gtype_destroy_object(php_gtype_t *object, zend_object_handle handle TSRMLS_DC)
{
	zend_hash_destroy(object->zobj.properties);
	FREE_HASHTABLE(object->zobj.properties);
	efree(object);
}

static zend_object_value gtype_create_object(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value zov;
	php_gtype_t *object;

	object = emalloc(sizeof(php_gtype_t));
	phpg_init_object(object, ce);

	object->type = 0;

	zov.handlers = php_gtk_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) gtype_destroy_object, NULL TSRMLS_CC);

	return zov;
}

static PHP_METHOD(GType, __construct) {}

static int gtype_type_read(void *object, zval *return_value)
{
	ZVAL_LONG(return_value, ((php_gtype_t *)object)->type);
	return SUCCESS;
}

/*
 * External API functions
 */ 
PHP_GTK_API zval* php_gtype_new(GType type)
{
	zval *wrapper;
	php_gtype_t *object;
	TSRMLS_FETCH();

	MAKE_STD_ZVAL(wrapper);
	object_init_ex(wrapper, gtype_ce);
	object = zend_object_store_get_object(wrapper TSRMLS_CC);
	object->type = type;

	return wrapper;
}

void php_gtype_register_self()
{
	if (gtype_ce) return;

	gtype_ce = phpg_register_class("GType", gtype_methods, NULL, gtype_props_info, gtype_create_object TSRMLS_CC);
}

#endif /* HAVE_PHP_GTK */
