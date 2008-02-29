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
 * GType PHP class definition
 */

typedef struct {
	PHPG_OBJ_HEADER
	GType type;
} phpg_gtype_t;

PHP_GTK_EXPORT_CE(gtype_ce) = NULL;
static zend_object_handlers phpg_gtype_handlers;

static PHP_METHOD(GType, __construct);
PHPG_PROP_READER(GType, type);
PHPG_PROP_READER(GType, name);

/* XXX possibly make constructor public */
static zend_function_entry gtype_methods[] = {
	ZEND_ME(GType, __construct, NULL, ZEND_ACC_PRIVATE)
	{NULL, NULL, NULL}
};

static prop_info_t gtype_props_info[] = {
	{ "type", PHPG_PROP_READ_FN(GType, type), NULL },
	{ "name", PHPG_PROP_READ_FN(GType, name), NULL },
	{ NULL, NULL, NULL },
};

static void gtype_free_object_storage(phpg_gtype_t *object TSRMLS_DC)
{
	zend_hash_destroy(object->zobj.properties);
	FREE_HASHTABLE(object->zobj.properties);
    if (object->zobj.guards) {
        zend_hash_destroy(object->zobj.guards);
        FREE_HASHTABLE(object->zobj.guards);     
    }
	efree(object);
}

static zend_object_value gtype_create_object(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value zov;
	phpg_gtype_t *object;

	object = emalloc(sizeof(phpg_gtype_t));
	phpg_init_object(object, ce);

	object->type = 0;

	zov.handlers = &php_gtk_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) gtype_free_object_storage, NULL TSRMLS_CC);

	return zov;
}

static int phpg_gtype_cast_object(zval *readobj, zval *writeobj, int type TSRMLS_DC)
{
	phpg_gtype_t *pobj = zend_object_store_get_object(readobj TSRMLS_CC);

	if (type == IS_LONG) {
		INIT_PZVAL(writeobj);
		ZVAL_LONG(writeobj, pobj->type);
		return SUCCESS;
	}

	/*
	 * XXX doesn't work, since ZEND_ECHO opcode calls zend_std_object_cast_to_string()
	 * which invokes __toString() instead
	 */
	if (type == IS_STRING) {
		char *buf;
		int buf_len;
		const char *name = g_type_name(pobj->type);

		buf_len = spprintf(&buf, 128, "[GType %s (%lu)]", name ? name : "invalid", pobj->type);
		INIT_PZVAL(writeobj);
		ZVAL_STRINGL(writeobj, buf, buf_len, 0);
		return SUCCESS;
	}

	return FAILURE;
}

static PHP_METHOD(GType, __construct) {}

PHPG_PROP_READER(GType, type)
{
	ZVAL_LONG(return_value, ((phpg_gtype_t *)object)->type);
	return SUCCESS;
}

PHPG_PROP_READER(GType, name)
{
	ZVAL_STRING(return_value, (char *)g_type_name(((phpg_gtype_t *)object)->type), 1);
	return SUCCESS;
}

/*
 * External API functions
 */ 
PHP_GTK_API void phpg_gtype_new(zval *zobj, GType type TSRMLS_DC)
{
	phpg_gtype_t *object;

    assert(zobj != NULL);
	object_init_ex(zobj, gtype_ce);
	object = zend_object_store_get_object(zobj TSRMLS_CC);
	object->type = type;
}

PHP_GTK_API GType phpg_gtype_from_class(zend_class_entry *ce TSRMLS_DC)
{
	zval **gtype;

    if (!ce || zend_hash_find(&ce->constants_table, "gtype", sizeof("gtype"), (void**)&gtype) == FAILURE) {
		php_error(E_WARNING, "PHP-GTK internal error: could not get typecode from class");
		return 0;
	}

	return Z_LVAL_PP(gtype);
}

/* TODO add support for boxed types */
PHP_GTK_API GType phpg_gtype_from_zval(zval *value)
{
	GType type;
	TSRMLS_FETCH();

	if (!value) {
		php_error(E_WARNING, "PHP-GTK internal error: could not get typecode from value");
		return 0;
	}

	switch (Z_TYPE_P(value)) {
		case IS_NULL:
			return G_TYPE_NONE;

		/*
		 * We check if the number corresponds to a valid GType. Return the
		 * corresponding type, if it does, assume a G_TYPE_INT otherwise.
		 */
		case IS_LONG:
		{
			if (G_TYPE_IS_FUNDAMENTAL(Z_LVAL_P(value)) ||
				G_TYPE_IS_CLASSED(Z_LVAL_P(value)) ||
				G_TYPE_FUNDAMENTAL(Z_LVAL_P(value)) == G_TYPE_BOXED ||
				G_TYPE_IS_INTERFACE(Z_LVAL_P(value))) {
				return Z_LVAL_P(value);
			}
			return G_TYPE_INT;
		}

		case IS_DOUBLE:
			return G_TYPE_DOUBLE;

		/* XXX
		 * Seeing as we lack a proper type system, need to fix this to divorce
		 * pure strings from types specified via strings. Maybe via
		 * implementing GType::from_name().
		 */
		case IS_STRING:
			type = g_type_from_name(Z_STRVAL_P(value));
			if (type == 0)
				return G_TYPE_STRING;
			else
				return type;

		case IS_BOOL:
			return G_TYPE_BOOLEAN;

		case IS_ARRAY:
			return G_TYPE_PHP_VALUE;

		case IS_OBJECT:
			if (Z_OBJCE_P(value) == gtype_ce) {
				phpg_gtype_t *object = zend_object_store_get_object(value TSRMLS_CC);
				if (object) {
					return object->type;
				}
			} else {
				zval **gtype;
				/*
				 * It's either a PHP object representing some Gtk+ object
				 */
				if (zend_hash_find(&Z_OBJCE_P(value)->constants_table, "gtype",
								   sizeof("gtype"), (void**)&gtype) == SUCCESS
					&& Z_TYPE_PP(gtype) == IS_LONG) {
						return Z_LVAL_PP(gtype);
				} else {
					/*
					 * Or it's a regular PHP value, but with our own GType.
					 */
					return G_TYPE_PHP_VALUE;
				}
			}
			break;

		default:
			break;
	}

	php_error(E_WARNING, "PHP-GTK internal error: could not get typecode from value");
	return 0;
}

void phpg_gtype_register_self(TSRMLS_D)
{
	if (gtype_ce) return;

	phpg_gtype_handlers = php_gtk_handlers;
	phpg_gtype_handlers.cast_object = phpg_gtype_cast_object;

	gtype_ce = phpg_register_class("GType", gtype_methods, NULL, 0, gtype_props_info, gtype_create_object, 0 TSRMLS_CC);
}

#endif /* HAVE_PHP_GTK */
