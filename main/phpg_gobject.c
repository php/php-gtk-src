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
 * GObject API and helper functions
 */

static GQuark gobject_wrapper_handle_key    = 0;
static GQuark gobject_wrapper_handlers_key  = 0;
static GQuark gobject_wrapper_owned_key     = 0;

static zend_object_handlers phpg_gobject_handlers;

HashTable phpg_prop_info;

/* {{{ static      phpg_free_gobject_storage() */
static inline void phpg_free_gobject_storage(phpg_gobject_t *object, zend_object_handle handle TSRMLS_DC)
{
    GSList *tmp;

    zend_hash_destroy(object->zobj.properties);
    FREE_HASHTABLE(object->zobj.properties);

    /*
     * Remove cached handle information, since the object wrapper is going away.
     */
    /*
    g_object_set_qdata(object->obj, gobject_wrapper_handle_key, NULL);
    g_object_set_qdata(object->obj, gobject_wrapper_handlers_key, NULL);
    */

	if (object->obj && object->dtor && !object->is_owned)
		object->dtor(object->obj);
    object->obj = NULL;

    tmp = object->closures;
    while (tmp) {
        GClosure *closure = (GClosure *) tmp->data;
        /*
         * Save the pointer, because phpg_gobject_unwatch_closure() will remove
         * the closure from the list.
         */
        tmp = tmp->next;
        g_closure_invalidate(closure);
    }
    object->closures = NULL;

    efree(object);
}
/* }}} */

/* {{{ static      phpg_sink_object() */
static inline void phpg_sink_object(GObject *obj)
{
    /*
     * For now, do a check for GTK_TYPE_OBJECT's and sink them. More generic
     * system can be implemented later.
     */
    if (g_type_is_a(G_OBJECT_TYPE(obj), GTK_TYPE_OBJECT)
        && GTK_OBJECT_FLOATING(obj)) {
        g_object_ref(obj);
        gtk_object_sink(GTK_OBJECT(obj));
        return;
    }

    if (g_type_is_a(G_OBJECT_TYPE(obj), GTK_TYPE_WINDOW)
        && obj->ref_count == 1
        && GTK_WINDOW(obj)->has_user_ref_count) {
        g_object_ref(obj);
        return;
    }

    if (g_type_is_a(G_OBJECT_TYPE(obj), GTK_TYPE_INVISIBLE)
        && obj->ref_count == 1
        && GTK_INVISIBLE(obj)->has_user_ref_count) {
        g_object_ref(obj);
        return;
    }
}
/* }}} */

/* {{{ static phpg_unref_by_handle */
static void phpg_unref_by_handle(void *data)
{
    zend_object_handle handle = (zend_object_handle) data;
    zval zobj;
    TSRMLS_FETCH();

    Z_OBJ_HANDLE(zobj) = handle;
    php_gtk_handlers.del_ref(&zobj TSRMLS_CC);
}
/* }}} */

void phpg_gobject_del_ref(zval *zobject TSRMLS_DC)
{
    zend_object_handle handle = Z_OBJ_HANDLE_P(zobject);
	struct _store_object *stored = &EG(objects_store).object_buckets[handle].bucket.obj;
    phpg_gobject_t *pobj = (phpg_gobject_t *) stored->object;

	if (EG(objects_store).object_buckets[handle].valid && stored->refcount == 1) {
        if (pobj->obj && pobj->obj->ref_count > 1) {
            pobj->is_owned = TRUE;
            g_object_set_qdata_full(pobj->obj, gobject_wrapper_owned_key, (void *)handle, phpg_unref_by_handle);
            g_object_unref(pobj->obj);
            return;
        }
    }

    php_gtk_handlers.del_ref(zobject TSRMLS_CC);
}

/* {{{ PHP_GTK_API phpg_create_gobject() */
PHP_GTK_API zend_object_value phpg_create_gobject(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value zov;
	phpg_gobject_t *object;

	object = emalloc(sizeof(phpg_gobject_t));
	phpg_init_object(object, ce);

	object->obj  = NULL;
	object->dtor = NULL;
	object->closures = NULL;
	object->is_owned = FALSE;

	zov.handlers = &phpg_gobject_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) phpg_free_gobject_storage, NULL TSRMLS_CC);

	return zov;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gobject_set_wrapper() */
PHP_GTK_API void phpg_gobject_set_wrapper(zval *zobj, GObject *obj TSRMLS_DC)
{
    phpg_gobject_t *pobj = NULL;

    phpg_sink_object(obj);
    pobj = (phpg_gobject_t *) zend_object_store_get_object(zobj TSRMLS_CC);
    pobj->obj = obj;
    pobj->dtor = (phpg_dtor_t) g_object_unref;
    pobj->is_owned = FALSE;
    g_object_set_qdata(pobj->obj, gobject_wrapper_handle_key, (void*)Z_OBJ_HANDLE_P(zobj));
    g_object_set_qdata(pobj->obj, gobject_wrapper_handlers_key, (void*)Z_OBJ_HT_P(zobj));
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gobject_new() */
PHP_GTK_API void phpg_gobject_new(zval **zobj, GObject *obj TSRMLS_DC)
{
	zend_class_entry *ce = NULL;
	phpg_gobject_t *pobj = NULL;
    zend_object_handle handle;
    zend_object_handlers *handlers;

    assert(zobj != NULL);
    if (*zobj == NULL) {
        MAKE_STD_ZVAL(*zobj);
    }
	ZVAL_NULL(*zobj);

	if (obj == NULL) {
		return;
	}

	/* 
	 * 1. Lookup stored object handle based on wrapper key.
	 * 2. Make zval refer to this handle.
	 * 3. If not found, create new object and store it.
	 * 4. Install wrapper key.
	 */

	handle = (zend_object_handle) g_object_get_qdata(obj, gobject_wrapper_handle_key);
	if ((void*)handle != NULL) {
        phpg_gobject_t *pobj;
        handlers = (zend_object_handlers*) g_object_get_qdata(obj, gobject_wrapper_handlers_key);
		Z_TYPE_PP(zobj) = IS_OBJECT;
		Z_OBJ_HANDLE_PP(zobj) = handle;
		Z_OBJ_HT_PP(zobj) = handlers;
        pobj = zend_object_store_get_object(*zobj TSRMLS_CC);
        if (pobj->is_owned) {
            pobj->is_owned = FALSE;
            g_object_steal_qdata(pobj->obj, gobject_wrapper_owned_key);
            g_object_ref(pobj->obj);
        } else {
            zend_objects_store_add_ref(*zobj TSRMLS_CC);
        }
	} else {
		ce = phpg_class_from_gtype(G_OBJECT_TYPE(obj));
		object_init_ex(*zobj, ce);

		g_object_ref(obj);
        phpg_sink_object(obj);

		pobj = (phpg_gobject_t *) zend_object_store_get_object(*zobj TSRMLS_CC);
		pobj->obj = obj;
        pobj->dtor = (phpg_dtor_t) g_object_unref;
        pobj->is_owned = FALSE;
		g_object_set_qdata(obj, gobject_wrapper_handle_key, (void*)Z_OBJ_HANDLE_PP(zobj));
		g_object_set_qdata(obj, gobject_wrapper_handlers_key, (void*)Z_OBJ_HT_PP(zobj));
	}
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gobject_watch_closure() */
static void phpg_gobject_unwatch_closure(gpointer data, GClosure *closure)
{
    phpg_gobject_t *pobj = (phpg_gobject_t *) data;

    pobj->closures = g_slist_remove(pobj->closures, closure);
}

PHP_GTK_API void phpg_gobject_watch_closure(zval *zobj, GClosure *closure TSRMLS_DC)
{
    phpg_gobject_t *pobj = NULL;

    phpg_return_if_fail(zobj != NULL);
    phpg_return_if_fail(closure != NULL);
    phpg_return_if_fail_quiet(Z_TYPE_P(zobj) == IS_OBJECT && instanceof_function(Z_OBJCE_P(zobj), gobject_ce TSRMLS_CC));

    pobj = zend_object_store_get_object(zobj TSRMLS_CC);
    phpg_return_if_fail_quiet(g_slist_find(pobj->closures, closure) == NULL);

    pobj->closures = g_slist_prepend(pobj->closures, closure);
    g_closure_add_invalidate_notifier(closure, pobj, phpg_gobject_unwatch_closure);
}
/* }}} */

/*
 * GObject PHP class definition
 */

PHP_GTK_EXPORT_CE(gobject_ce) = NULL;

/* {{{ static phpg_signal_connect_impl() */
static void phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAMETERS, zend_bool use_signal_object, zend_bool after)
{
    char *signal = NULL;
    zval *callback;
    zval *extra = NULL;
    GObject *obj = NULL;
    guint signal_id, handler_id;
    GQuark detail = 0;
    GClosure *closure = NULL;

    NOT_STATIC_METHOD();

    if (ZEND_NUM_ARGS() < 2) {
        php_error(E_WARNING, "%s() requires at least 2 arguments, %d given",
                  get_active_function_name(TSRMLS_C), ZEND_NUM_ARGS());
        return;
    }

    if (!php_gtk_parse_args(2, "sV", &signal, &callback)) {
        return;
    }

    obj = PHPG_GOBJECT(this_ptr);
    if (!g_signal_parse_name(signal, G_OBJECT_TYPE(obj), &signal_id, &detail, TRUE)) {
        php_error(E_WARNING, "%s(): unknown signal name", get_active_function_name(TSRMLS_C));
        return;
    }

    extra = php_gtk_func_args_as_hash(ZEND_NUM_ARGS(), 2, ZEND_NUM_ARGS());
    closure = phpg_closure_new(callback, extra, use_signal_object TSRMLS_CC);
    if (extra) {
        zval_ptr_dtor(&extra);
    }
    phpg_gobject_watch_closure(this_ptr, closure);
    handler_id = g_signal_connect_closure_by_id(obj, signal_id, detail, closure, after);
    RETURN_LONG(handler_id);
}
/* }}} */
/* {{{ GObject::connect */
static PHP_METHOD(GObject, connect)
{
	phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE, FALSE);
}
/* }}} */
/* {{{ GObject::connect_after */
static PHP_METHOD(GObject, connect_after)
{
	phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRUE, TRUE);
}
/* }}} */
/* {{{ GObject::connect_object */
static PHP_METHOD(GObject, connect_object)
{
	phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE, FALSE);
}
/* }}} */
/* {{{ GObject::connect_object_after */
static PHP_METHOD(GObject, connect_object_after)
{
	phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, FALSE, TRUE);
}
/* }}} */

/* {{{ GObject::__tostring() */
static PHP_METHOD(GObject, __tostring)
{
    char buf[256];
    GObject *obj = NULL;
    int numc = 0;

    NOT_STATIC_METHOD();

    obj = PHPG_GOBJECT(this_ptr);
    numc = snprintf(buf, sizeof(buf),
                    "[%s object (%s Gtk+ type)]", Z_OBJCE_P(this_ptr)->name,
                    obj ? G_OBJECT_TYPE_NAME(obj) : "uninitialized");
    RETURN_STRINGL(buf, numc, 1);
}
/* }}} */

static zend_function_entry gobject_methods[] = {
	PHP_ME(GObject, connect, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(GObject, connect_after, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(GObject, connect_object, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(GObject, connect_object_after, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(GObject, __tostring, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

void phpg_gobject_register_self(TSRMLS_D)
{
	if (gobject_ce) return;

    gobject_wrapper_handle_key   = g_quark_from_static_string("phpg-wrapper-handle");
    gobject_wrapper_handlers_key = g_quark_from_static_string("phpg-wrapper-handlers");
    gobject_wrapper_owned_key    = g_quark_from_static_string("phpg-wrapper-owned");

    phpg_gobject_handlers = php_gtk_handlers;
    phpg_gobject_handlers.del_ref = phpg_gobject_del_ref;

	gobject_ce = phpg_register_class("GObject", gobject_methods, NULL, 0, NULL, NULL, G_TYPE_OBJECT TSRMLS_CC);
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */
