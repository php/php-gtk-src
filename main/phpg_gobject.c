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

/* {{{ phpg_gobject_del_ref */
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
/* }}} */

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

/* {{{ phpg_gobject_construct */
zend_bool phpg_gobject_construct(zval *this_ptr TSRMLS_DC)
{
    GType my_type;
    guint n_params = 0;
    GParameter *params = NULL;
    GObjectClass *class;
    GObject *obj;

    my_type = phpg_gtype_from_zval(this_ptr);
    
    if (G_TYPE_IS_ABSTRACT(my_type)) {
        zend_error(E_ERROR, "Cannot instantiate abstract class %s", Z_OBJCE_P(this_ptr)->name);
        return 0;
    }

    if ((class = g_type_class_ref(my_type)) == NULL) {
        zend_error(E_ERROR, "Could not get a reference to type class");
        return 0;
    }

    obj = g_object_newv(my_type, n_params, params);
    if (!obj) {
        char buf [128];
        snprintf(buf, 128, "could not construct %s object", Z_OBJCE_P(this_ptr)->name);
        PHPG_THROW_EXCEPTION_WITH_RETURN(phpg_construct_exception, buf, 0);
    }

    phpg_gobject_set_wrapper(this_ptr, obj TSRMLS_CC);

    return 1;
}

/* }}} */

/*
 * GObject PHP class definition
 */

PHP_GTK_EXPORT_CE(gobject_ce) = NULL;

/* {{{ GObject::__construct */
static PHP_METHOD(GObject, __construct)
{
    phpg_gobject_construct(this_ptr TSRMLS_CC);
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

    if (!php_gtk_parse_varargs(ZEND_NUM_ARGS(), 2, &extra, "sV", &signal, &callback)) {
        return;
    }

    obj = PHPG_GOBJECT(this_ptr);
    if (!g_signal_parse_name(signal, G_OBJECT_TYPE(obj), &signal_id, &detail, TRUE)) {
        php_error(E_WARNING, "%s(): unknown signal name '%s'", get_active_function_name(TSRMLS_C), signal);
        return;
    }

    //extra = php_gtk_func_args_as_hash(ZEND_NUM_ARGS(), 2, ZEND_NUM_ARGS());
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

/* {{{ GObject::get/set_property */
static PHP_METHOD(GObject, get_property)
{
    char *property;
    GParamSpec *pspec;
    GValue value = { 0, };
    GObject *obj;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &property))
        return;

    obj = PHPG_GOBJECT(this_ptr);
    pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(obj), property);
    if (pspec == NULL) {
        php_error(E_WARNING, "Class '%s' does not support property '%s'",
                  g_type_name(G_TYPE_FROM_INSTANCE(obj)), property);
        return;
    }

    if (!(pspec->flags & G_PARAM_READABLE)) {
        php_error(E_WARNING, "Property '%s' is not readable", property);
        return;
    }

    g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
    g_object_get_property(obj, property, &value);
    phpg_param_gvalue_to_zval(&value, &return_value, TRUE, pspec TSRMLS_CC);
    g_value_unset(&value);
}

static PHP_METHOD(GObject, set_property)
{
    char *property;
    zval *php_value;
    GParamSpec *pspec;
    GValue value = { 0, };
    GObject *obj;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "sV", &property, &php_value))
        return;

    obj = PHPG_GOBJECT(this_ptr);
    pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(obj), property);
    if (pspec == NULL) {
        php_error(E_WARNING, "Class '%s' does not support property '%s'",
                  g_type_name(G_TYPE_FROM_INSTANCE(obj)), property);
        return;
    }

    if (!(pspec->flags & G_PARAM_WRITABLE)) {
        php_error(E_WARNING, "Property '%s' is not writable", property);
        return;
    }

    g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
    if (phpg_param_gvalue_from_zval(&value, php_value, pspec TSRMLS_CC) == FAILURE) {
        php_error(E_WARNING, "%s::%s(): could not convert value to property type",
                  get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        return;
    }
    g_object_set_property(obj, property, &value);
    g_value_unset(&value);
}
/* }}} */

/* {{{ GObject::get/set_data */
static PHP_METHOD(GObject, get_data)
{
    char *key;
    zval *data;
    GQuark quark;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &key))
        return;

    quark = g_quark_from_string(key);
    data = g_object_get_qdata(PHPG_GOBJECT(this_ptr), quark);
    if (data) {
        RETURN_ZVAL(data, 1, 0);
    }
}

static PHP_METHOD(GObject, set_data)
{
    char *key;
    zval *data;
    GQuark quark;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "sV", &key, &data))
        return;

    quark = g_quark_from_string(key);
    zval_add_ref(&data);
    g_object_set_qdata_full(PHPG_GOBJECT(this_ptr), quark, data, phpg_destroy_notify);
}
/* }}} */

/* {{{ GObject::handler_* */
static PHP_METHOD(GObject, block)
{
    guint handler_id;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &handler_id)) {
        return;
    }

    g_signal_handler_block(PHPG_GOBJECT(this_ptr), handler_id);
}

static PHP_METHOD(GObject, unblock)
{
    guint handler_id;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &handler_id)) {
        return;
    }

    g_signal_handler_unblock(PHPG_GOBJECT(this_ptr), handler_id);
}

static PHP_METHOD(GObject, disconnect)
{
    guint handler_id;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &handler_id)) {
        return;
    }

    g_signal_handler_disconnect(PHPG_GOBJECT(this_ptr), handler_id);
}

static PHP_METHOD(GObject, is_connected)
{
    guint handler_id;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &handler_id)) {
        return;
    }

    RETURN_BOOL(g_signal_handler_is_connected(PHPG_GOBJECT(this_ptr), handler_id));
}
/* }}} */

/* {{{ GObject::signal_list_ids/names */
static void phpg_signal_list_impl(INTERNAL_FUNCTION_PARAMETERS, zend_bool list_names)
{
    zval *php_type;
    GType type;
    gpointer *klass;
    guint *ids, i, n;

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "V", &php_type))
        return;

    type = phpg_gtype_from_zval(php_type);
    if (!type)
        return;

    if (!G_TYPE_IS_INSTANTIATABLE(type) && !G_TYPE_IS_INTERFACE(type)) {
        php_error(E_WARNING, "%s::%s() requires the type to be instantiable or an interface", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        return;
    }

    if (G_TYPE_IS_INTERFACE(type)) {
        klass = g_type_default_interface_ref(type);
    } else {
        klass = g_type_class_ref(type);
    }
    if (!klass) {
        php_error(E_WARNING, "%s::%s() could not get a reference to type class", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
    }

    ids = g_signal_list_ids(type, &n);

    array_init(return_value);
    if (list_names) {
        for (i = 0; i < n; i++) {
            add_next_index_string(return_value, (char *)g_signal_name(ids[i]), 1);
        }
    } else {
        for (i = 0; i < n; i++) {
            add_next_index_long(return_value, ids[i]);
        }
    }

    g_free(ids);
    if (G_TYPE_IS_INTERFACE(type)) {
        g_type_default_interface_unref(klass);
    } else {
        g_type_class_unref(klass);
    }
}

static PHP_METHOD(GObject, signal_list_ids)
{
    phpg_signal_list_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}

static PHP_METHOD(GObject, signal_list_names)
{
    phpg_signal_list_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ GObject::signal_query */
static PHP_METHOD(GObject, signal_query)
{
    zval *php_type, *temp, *params;
    GType gtype;
    gpointer *klass = NULL;
    GSignalQuery query;
    char *signal_name = NULL;
    guint signal_id = 0, i;

    if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "iV", &signal_id, &php_type) &&
        !php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "sV", &signal_name, &php_type)) {
        php_error(E_WARNING, "%s::%s() requires the arguments to be either a valid signal_id or a signal name, followed by the object type", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        return;
    }

    gtype = phpg_gtype_from_zval(php_type);
    if (!gtype)
        return;

    if (!G_TYPE_IS_INSTANTIATABLE(gtype) && !G_TYPE_IS_INTERFACE(gtype)) {
        php_error(E_WARNING, "%s::%s() requires the type to be instantiable or an interface", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        return;
    }

    if (G_TYPE_IS_INTERFACE(gtype)) {
        klass = g_type_default_interface_ref(gtype); 
    } else {
        klass = g_type_class_ref(gtype); 
    }
    if (!klass) {
        php_error(E_WARNING, "%s::%s() could not get a reference to type class", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        return;
    }

    if (signal_name) {
        signal_id = g_signal_lookup(signal_name, gtype);
    }

    g_signal_query(signal_id, &query);

    if (query.signal_id == 0) {
        goto signal_query_done;
    }

    array_init(return_value);

    add_next_index_long(return_value, query.signal_id);
    add_next_index_string(return_value, (char *)query.signal_name, 1);
    MAKE_STD_ZVAL(temp);
    phpg_gtype_new(temp, query.itype);
    add_next_index_zval(return_value, temp);
    add_next_index_long(return_value, query.signal_flags);
    MAKE_STD_ZVAL(temp);
    phpg_gtype_new(temp, query.return_type);
    add_next_index_zval(return_value, temp);
    MAKE_STD_ZVAL(params);
    array_init(params);
    for (i = 0; i < query.n_params; i++) {
        MAKE_STD_ZVAL(temp);
        phpg_gtype_new(temp, query.param_types[i]);
        add_next_index_zval(params, temp);
    }
    add_next_index_zval(return_value, params);

signal_query_done:
    if (G_TYPE_IS_INTERFACE(gtype)) {
        g_type_default_interface_unref(klass); 
    } else {
        g_type_class_unref(klass); 
    }
}
/* }}} */


/* {{{ GObject reflection info */
static
ZEND_BEGIN_ARG_INFO_EX(arginfo_gobject_connect, 0, 0, 2)
    ZEND_ARG_INFO(0, signal)
    ZEND_ARG_INFO(0, callback)
    ZEND_ARG_INFO(0, userparam)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO_EX(arginfo_gobject_connect_after, 0, 0, 2)
    ZEND_ARG_INFO(0, signal)
    ZEND_ARG_INFO(0, callback)
    ZEND_ARG_INFO(0, userparam)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO_EX(arginfo_gobject_connect_object, 0, 0, 2)
    ZEND_ARG_INFO(0, signal)
    ZEND_ARG_INFO(0, callback)
    ZEND_ARG_INFO(0, userparam)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO_EX(arginfo_gobject_connect_object_after, 0, 0, 2)
    ZEND_ARG_INFO(0, signal)
    ZEND_ARG_INFO(0, callback)
    ZEND_ARG_INFO(0, userparam)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_get_property, 0)
    ZEND_ARG_INFO(0, property_name)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_set_property, 0)
    ZEND_ARG_INFO(0, property_name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_get_data, 0)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_set_data, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_signal_list_ids, 0)
    ZEND_ARG_INFO(0, gtype)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_signal_list_names, 0)
    ZEND_ARG_INFO(0, gtype)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_signal_query, 0)
    ZEND_ARG_INFO(0, signal)
    ZEND_ARG_INFO(0, gtype)
ZEND_END_ARG_INFO();


static zend_function_entry gobject_methods[] = {
    PHP_ME(GObject, __construct,          NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, __tostring,           NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect,              arginfo_gobject_connect               , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect_after,        arginfo_gobject_connect_after         , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect_object,       arginfo_gobject_connect_object        , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect_object_after, arginfo_gobject_connect_object_after  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, get_property,         arginfo_gobject_get_property          , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, set_property,         arginfo_gobject_set_property          , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, get_data,             arginfo_gobject_get_data              , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, set_data,             arginfo_gobject_set_data              , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, block,                NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, unblock,              NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, disconnect,           NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, is_connected,         NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, signal_list_ids,      arginfo_gobject_signal_list_ids       , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(GObject, signal_list_names,    arginfo_gobject_signal_list_names     , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(GObject, signal_query,         arginfo_gobject_signal_query          , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};
/* }}} */


void phpg_gobject_register_self(TSRMLS_D)
{
	if (gobject_ce) return;

    gobject_wrapper_handle_key   = g_quark_from_static_string("phpg-wrapper-handle");
    gobject_wrapper_handlers_key = g_quark_from_static_string("phpg-wrapper-handlers");
    gobject_wrapper_owned_key    = g_quark_from_static_string("phpg-wrapper-owned");

    phpg_gobject_handlers = php_gtk_handlers;
    phpg_gobject_handlers.del_ref = phpg_gobject_del_ref;

	gobject_ce = phpg_register_class("GObject", gobject_methods, NULL, 0, NULL, NULL, G_TYPE_OBJECT TSRMLS_CC);
    phpg_register_int_constant(gobject_ce, "gtype", sizeof("gtype")-1, G_TYPE_OBJECT);
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */
