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
 * GdkAtom API and helper functions
 */
PHP_GTK_EXPORT_CE(gdkatom_ce) = NULL;

static zend_object_handlers phpg_gdkatom_handlers;

static zend_function_entry gdkatom_methods[] = {
#if ZEND_EXTENSION_API_NO > 220051025
    PHP_ME_MAPPING(__construct, no_direct_constructor, NULL, 0)
#else
    PHP_ME_MAPPING(__construct, no_direct_constructor, NULL)
#endif
    { NULL, NULL, NULL }
};

/* {{{ static phpg_free_gdkatom_storage() */
static void phpg_free_gdkatom_storage(phpg_gdkatom_t *object TSRMLS_DC)
{
	zend_hash_destroy(object->zobj.properties);
	FREE_HASHTABLE(object->zobj.properties);
    if (object->zobj.guards) {
        zend_hash_destroy(object->zobj.guards);
        FREE_HASHTABLE(object->zobj.guards);     
    }
	if (object->name) efree(object->name);
	efree(object);
}
/* }}} */

/* {{{ static phpg_gdkatom_compare_objects() */
static int phpg_gdkatom_compare_objects(zval *zobj1, zval *zobj2 TSRMLS_DC)
{
    phpg_gdkatom_t *pobj1, *pobj2;

    pobj1 = zend_object_store_get_object(zobj1 TSRMLS_CC);
    pobj2 = zend_object_store_get_object(zobj2 TSRMLS_CC);

    return (pobj1->atom > pobj2->atom) ? 1 : (pobj1->atom < pobj2->atom ? -1 : 0);
}
/* }}} */

/* {{{ static phpg_gdkatom_cast_object() */
#if ZEND_EXTENSION_API_NO > 220051025
static int phpg_gdkatom_cast_object(zval *readobj, zval *writeobj, int type TSRMLS_DC)
{
    phpg_gdkatom_t *pobj;
    gchar *atom_name = NULL;

    if (type == IS_STRING) {
        pobj = zend_object_store_get_object(readobj TSRMLS_CC);
        if (!pobj->name) {
            atom_name = gdk_atom_name(pobj->atom);
            if (atom_name) {
                pobj->name = estrdup(atom_name);
                ZVAL_STRING(writeobj, pobj->name, 1);
            } else {
                ZVAL_EMPTY_STRING(writeobj);
            }
        }
        return SUCCESS;
    }

    return FAILURE;
}
#else
static int phpg_gdkatom_cast_object(zval *readobj, zval *writeobj, int type, int should_free TSRMLS_DC)
{
    phpg_gdkatom_t *pobj;
    zval free_obj;

    if (type == IS_STRING) {
        pobj = zend_object_store_get_object(readobj TSRMLS_CC);
        if (should_free) {
            free_obj = *writeobj;
        }
        if (!pobj->name) {
            gchar *atom_name = gdk_atom_name(pobj->atom);
            if (atom_name) {
                pobj->name = estrdup(atom_name);
                ZVAL_STRING(writeobj, pobj->name, 1);
            } else {
                ZVAL_EMPTY_STRING(writeobj);
            }
        }
        if (should_free) {
            zval_dtor(&free_obj);
        }
        return SUCCESS;
    }

    return FAILURE;
}
#endif
/* }}} */

/* {{{ PHP_GTK_API phpg_create_gdkatom() */
PHP_GTK_API zend_object_value phpg_create_gdkatom(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value zov;
	phpg_gdkatom_t *object;

	object = emalloc(sizeof(phpg_gdkatom_t));
	phpg_init_object(object, ce);

	object->atom = GDK_NONE;
	object->name = NULL;

	zov.handlers = &phpg_gdkatom_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) phpg_free_gdkatom_storage, NULL TSRMLS_CC);

	return zov;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gdkatom_new() */
PHP_GTK_API void phpg_gdkatom_new(zval **zobj, GdkAtom atom TSRMLS_DC)
{
    phpg_gdkatom_t *pobj = NULL;

    assert(zobj != NULL);
    if (*zobj == NULL) {
        MAKE_STD_ZVAL(*zobj);
    }

    object_init_ex(*zobj, gdkatom_ce);

    pobj = zend_object_store_get_object(*zobj TSRMLS_CC);
    pobj->atom = atom;
    pobj->name = NULL;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gdkatom_from_zval() */
PHP_GTK_API GdkAtom phpg_gdkatom_from_zval(zval *value TSRMLS_DC)
{
    if (value) {
        if (Z_TYPE_P(value) == IS_STRING) {
            return gdk_atom_intern(Z_STRVAL_P(value), FALSE);
        } else if (Z_TYPE_P(value) == IS_OBJECT
                   && instanceof_function(Z_OBJCE_P(value), gdkatom_ce TSRMLS_CC)) {
            return PHPG_GDKATOM(value);
        } else {
            php_error(E_WARNING, "unable to convert to type GdkAtom");
            return NULL;
        }
    } else {
        return NULL;
    }
}
/* }}} */

/* {{{ phpg_gdkatom_register_self() */
void phpg_gdkatom_register_self(TSRMLS_D)
{
	if (gdkatom_ce) return;

    phpg_gdkatom_handlers = php_gtk_handlers;
    phpg_gdkatom_handlers.compare_objects = phpg_gdkatom_compare_objects;
    phpg_gdkatom_handlers.cast_object = phpg_gdkatom_cast_object;

	gdkatom_ce = phpg_register_class("GdkAtom", gdkatom_methods, NULL, 0, NULL, phpg_create_gdkatom, 0 TSRMLS_CC);
}
/* }}} */

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */

