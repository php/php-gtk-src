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
 * GParamSpec PHP class definition
 */

PHP_GTK_EXPORT_CE(gparamspec_ce) = NULL;

static void phpg_paramspec_free_object_storage(phpg_paramspec_t *object TSRMLS_DC)
{
	zend_hash_destroy(object->zobj.properties);
	FREE_HASHTABLE(object->zobj.properties);
    if (object->zobj.guards) {
        zend_hash_destroy(object->zobj.guards);
        FREE_HASHTABLE(object->zobj.guards);     
    }
    if (object->pspec) {
        g_param_spec_unref(object->pspec);
    }
	efree(object);
}

/* {{{ PHP_GTK_API phpg_create_paramspec() */
PHP_GTK_API zend_object_value phpg_create_paramspec(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value zov;
    phpg_paramspec_t *object;

    object = emalloc(sizeof(phpg_paramspec_t));
    phpg_init_object(object, ce);

	object->pspec = NULL;

	zov.handlers = &php_gtk_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) phpg_paramspec_free_object_storage, NULL TSRMLS_CC);

    return zov;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_paramspec_new() */
PHP_GTK_API void phpg_paramspec_new(zval **zobj, GParamSpec *pspec TSRMLS_DC)
{
    phpg_paramspec_t *pobj = NULL;

    assert(zobj != NULL);
    if (*zobj == NULL) {
        MAKE_STD_ZVAL(*zobj);
    }
    ZVAL_NULL(*zobj);

    if (pspec == NULL) {
        return;
    }

    object_init_ex(*zobj, gparamspec_ce);
    pobj = zend_object_store_get_object(*zobj TSRMLS_CC);
    pobj->pspec = g_param_spec_ref(pspec);
}
/* }}} */

static PHP_METHOD(GParamSpec, __construct) {}

/* {{{ GParamSpec::__tostring() */
static PHP_METHOD(GParamSpec, __tostring)
{
    char buf[256];
    GParamSpec *pspec = NULL;
    int numc = 0;

    NOT_STATIC_METHOD();

    pspec = PHPG_GPARAMSPEC(this_ptr);
    numc = snprintf(buf, sizeof(buf),
                    "[%s '%s']", G_PARAM_SPEC_TYPE_NAME(pspec),
                    g_param_spec_get_name(pspec));
    RETURN_STRINGL(buf, numc, 1);
}
/* }}} */

/* {{{ GParamSpec properties */
PHPG_PROP_READER(GParamSpec, gtype)
{
	phpg_gtype_new(return_value, G_PARAM_SPEC_TYPE(((phpg_paramspec_t *)object)->pspec) TSRMLS_CC);
    return SUCCESS;
}

PHPG_PROP_READER(GParamSpec, name)
{
    const char *name = g_param_spec_get_name(((phpg_paramspec_t *)object)->pspec);
    if (name) {
        ZVAL_STRING(return_value, (char*)name, 1);
    } else {
        ZVAL_NULL(return_value);
    }
    return SUCCESS;
}

PHPG_PROP_READER(GParamSpec, nick)
{
    const char *nick = g_param_spec_get_nick(((phpg_paramspec_t *)object)->pspec);
    if (nick) {
        ZVAL_STRING(return_value, (char*)nick, 1);
    } else {
        ZVAL_NULL(return_value);
    }
    return SUCCESS;
}

PHPG_PROP_READER(GParamSpec, blurb)
{
    const char *blurb = g_param_spec_get_blurb(((phpg_paramspec_t *)object)->pspec);
    if (blurb) {
        ZVAL_STRING(return_value, (char*)blurb, 1);
    } else {
        ZVAL_NULL(return_value);
    }
    return SUCCESS;
}

PHPG_PROP_READER(GParamSpec, flags)
{
	ZVAL_LONG(return_value, ((phpg_paramspec_t *)object)->pspec->flags);
    return SUCCESS;
}

PHPG_PROP_READER(GParamSpec, owner_type)
{
	phpg_gtype_new(return_value, ((phpg_paramspec_t *)object)->pspec->owner_type TSRMLS_CC);
    return SUCCESS;
}

PHPG_PROP_READER(GParamSpec, value_type)
{
	phpg_gtype_new(return_value, ((phpg_paramspec_t *)object)->pspec->value_type TSRMLS_CC);
    return SUCCESS;
}

PHPG_PROP_READER(GParamSpec, default_value)
{
    GParamSpec *pspec = ((phpg_paramspec_t *)object)->pspec;

    ZVAL_NULL(return_value);
    if (G_IS_PARAM_SPEC_CHAR(pspec)) {
        char buf[2];
        snprintf(buf, 2, "%c", G_PARAM_SPEC_CHAR(pspec)->default_value);
        ZVAL_STRINGL(return_value, buf, 1, 1);
    } else if (G_IS_PARAM_SPEC_UCHAR(pspec)) {
        char buf[2];
        snprintf(buf, 2, "%c", G_PARAM_SPEC_UCHAR(pspec)->default_value);
        ZVAL_STRINGL(return_value, buf, 1, 1);
    } else if (G_IS_PARAM_SPEC_BOOLEAN(pspec)) {
        ZVAL_BOOL(return_value, G_PARAM_SPEC_BOOLEAN(pspec)->default_value);
    } else if (G_IS_PARAM_SPEC_INT(pspec)) {
        ZVAL_LONG(return_value, G_PARAM_SPEC_INT(pspec)->default_value);
    } else if (G_IS_PARAM_SPEC_UINT(pspec)) {
        ZVAL_LONG(return_value, G_PARAM_SPEC_UINT(pspec)->default_value);
    } else if (G_IS_PARAM_SPEC_LONG(pspec)) {
        ZVAL_LONG(return_value, G_PARAM_SPEC_LONG(pspec)->default_value);
    } else if (G_IS_PARAM_SPEC_ULONG(pspec)) {
        ZVAL_LONG(return_value, G_PARAM_SPEC_ULONG(pspec)->default_value);
    } else if (G_IS_PARAM_SPEC_UNICHAR(pspec)) {
        char buf[2];
        snprintf(buf, 2, "%c", G_PARAM_SPEC_UCHAR(pspec)->default_value);
        ZVAL_STRINGL(return_value, buf, 1, 1);
    } else if (G_IS_PARAM_SPEC_ENUM(pspec)) {
        ZVAL_LONG(return_value, G_PARAM_SPEC_ENUM(pspec)->default_value);
    } else if (G_IS_PARAM_SPEC_FLAGS(pspec)) {
        ZVAL_LONG(return_value, G_PARAM_SPEC_FLAGS(pspec)->default_value);
    } else if (G_IS_PARAM_SPEC_FLOAT(pspec)) {
        ZVAL_DOUBLE(return_value, G_PARAM_SPEC_FLOAT(pspec)->default_value);
    } else if (G_IS_PARAM_SPEC_DOUBLE(pspec)) {
        ZVAL_DOUBLE(return_value, G_PARAM_SPEC_DOUBLE(pspec)->default_value);
    } else if (G_IS_PARAM_SPEC_STRING(pspec)) {
        if (G_PARAM_SPEC_STRING(pspec)->default_value) {
            ZVAL_STRING(return_value, G_PARAM_SPEC_STRING(pspec)->default_value, 1);
        }
    }

    return SUCCESS;
}
/* }}} */

static zend_function_entry gparamspec_methods[] = {
    PHP_ME(GParamSpec, __construct, NULL, ZEND_ACC_PRIVATE)
    PHP_ME(GParamSpec, __tostring,  NULL, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

static prop_info_t gparamspec_props_info[] = {
	{ "gtype",         PHPG_PROP_READ_FN(GParamSpec, gtype),         NULL },
	{ "name",          PHPG_PROP_READ_FN(GParamSpec, name),          NULL },
	{ "nick",          PHPG_PROP_READ_FN(GParamSpec, nick),          NULL },
	{ "blurb",         PHPG_PROP_READ_FN(GParamSpec, blurb),         NULL },
	{ "flags",         PHPG_PROP_READ_FN(GParamSpec, flags),         NULL },
	{ "owner_type",    PHPG_PROP_READ_FN(GParamSpec, owner_type),    NULL },
	{ "value_type",    PHPG_PROP_READ_FN(GParamSpec, value_type),    NULL },
	{ "default_value", PHPG_PROP_READ_FN(GParamSpec, default_value), NULL },
	{ NULL, NULL, NULL },
};

void phpg_paramspec_register_self(TSRMLS_D)
{
    if (gparamspec_ce) return;

	gparamspec_ce = phpg_register_class("GParamSpec", gparamspec_methods, NULL, 0, gparamspec_props_info, phpg_create_paramspec, G_TYPE_PARAM TSRMLS_CC);
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */
