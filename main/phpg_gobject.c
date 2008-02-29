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
#include "zend_interfaces.h"

#if HAVE_PHP_GTK

/*
 * GObject API and helper functions
 */

static GQuark gobject_wrapper_handle_key    = 0;
static GQuark gobject_wrapper_handlers_key  = 0;
static GQuark gobject_wrapper_owned_key     = 0;

PHP_GTK_API zend_object_handlers phpg_gobject_handlers;

HashTable phpg_prop_info;

/* {{{ static      phpg_free_gobject_storage() */
static inline void phpg_free_gobject_storage(phpg_gobject_t *object, zend_object_handle handle TSRMLS_DC)
{
    GSList *tmp;

    zend_hash_destroy(object->zobj.properties);
    FREE_HASHTABLE(object->zobj.properties);
    if (object->zobj.guards) {
        zend_hash_destroy(object->zobj.guards);
        FREE_HASHTABLE(object->zobj.guards);     
    }

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

    if (EG(objects_store).object_buckets[handle].valid && Z_REFCOUNT_P(stored) == 1) {	
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
zend_bool phpg_gobject_construct(zval *this_ptr, GType object_type, zval *props TSRMLS_DC)
{
    guint n_params = 0;
    GParameter *params = NULL;
    GObjectClass *klass;
    GObject *obj;
    char buf [128];
    int i;

    if (G_TYPE_IS_ABSTRACT(object_type)) {
        snprintf(buf, 128, "Cannot instantiate abstract class %s", g_type_name(object_type));
        PHPG_THROW_EXCEPTION_WITH_RETURN(phpg_construct_exception, buf, 0);
    }

    if ((klass = g_type_class_ref(object_type)) == NULL) {
        PHPG_THROW_EXCEPTION_WITH_RETURN(phpg_construct_exception, "Could not get a reference to type class", 0);
    }

    if (props) {
        int key_type;
        char *key;
        ulong num_key;
        zval **value;
        GParamSpec *pspec;

        params = ecalloc(zend_hash_num_elements(Z_ARRVAL_P(props)), sizeof(GParameter));
        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(props));
             zend_hash_get_current_data(Z_ARRVAL_P(props), (void**)&value) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(props))) {

            key_type = zend_hash_get_current_key(Z_ARRVAL_P(props), &key, &num_key, 0);
            if (key_type != HASH_KEY_IS_STRING) {
                zend_throw_exception(phpg_construct_exception, "parameter names have to be keys", 0 TSRMLS_CC);
                goto cleanup;
            }

            pspec = g_object_class_find_property(klass, key);
            if (!pspec) {
                snprintf(buf, 128, "Class %s does not support property '%s'", g_type_name(object_type), key);
                zend_throw_exception(phpg_construct_exception, buf, 0 TSRMLS_CC);
                goto cleanup;
            }

            g_value_init(&params[n_params].value, G_PARAM_SPEC_VALUE_TYPE(pspec));
            if (phpg_gvalue_from_zval(&params[n_params].value, value, TRUE TSRMLS_CC) == FAILURE) {
                snprintf(buf, 128, "Could not convert value for property '%s'", key);
                zend_throw_exception(phpg_construct_exception, buf, 0 TSRMLS_CC);
                goto cleanup;
            }

            params[n_params].name = estrdup(key);

            n_params++;
        }
    }

    obj = g_object_newv(object_type, n_params, params);
    if (obj) {
        phpg_gobject_set_wrapper(this_ptr, obj TSRMLS_CC);
    } else {
        snprintf(buf, 128, "Could not construct %s object", Z_OBJCE_P(this_ptr)->name);
        zend_throw_exception(phpg_construct_exception, buf, 0 TSRMLS_CC);
    }

cleanup:
    if (params) {
        for (i = 0; i < (int)n_params; i++) {
            efree((void *)params[i].name);
            g_value_unset(&params[i].value);
        }
        efree(params);
    }
    g_type_class_unref(klass);

    return (obj) ? 1 : 0;
}

/* }}} */

/*
 * GObject PHP class definition
 */

PHP_GTK_EXPORT_CE(gobject_ce) = NULL;

/* {{{ GObject::__construct */
static PHP_METHOD(GObject, __construct)
{
    GType my_type;
    zval *php_type = NULL;
    zval *php_props = NULL;
    char buf[128];

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "|Va", &php_type, &php_props)) {
        snprintf(buf, 128, "Could not construct %s object", Z_OBJCE_P(this_ptr)->name);
        PHPG_THROW_EXCEPTION(phpg_construct_exception, buf);
    }

    if (php_type) {
        my_type = phpg_gtype_from_zval(php_type);
    } else {
        my_type = phpg_gtype_from_zval(this_ptr);
    }

    phpg_gobject_construct(this_ptr, my_type, php_props TSRMLS_CC);
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
static void phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAMETERS, int connect_type, zend_bool after)
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
        if (extra) zval_ptr_dtor(&extra);
        return;
    }

    closure = phpg_closure_new(callback, extra, connect_type, NULL TSRMLS_CC);
    if (extra) {
        zval_ptr_dtor(&extra);
    }
    phpg_gobject_watch_closure(this_ptr, closure TSRMLS_CC);
    handler_id = g_signal_connect_closure_by_id(obj, signal_id, detail, closure, after);
    RETURN_LONG(handler_id);
}
/* }}} */
/* {{{ GObject::connect */
static PHP_METHOD(GObject, connect)
{
    phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHPG_CONNECT_NORMAL, FALSE);
}
/* }}} */
/* {{{ GObject::connect_after */
static PHP_METHOD(GObject, connect_after)
{
    phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHPG_CONNECT_NORMAL, TRUE);
}
/* }}} */
/* {{{ GObject::connect_simple */
static PHP_METHOD(GObject, connect_simple)
{
    phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHPG_CONNECT_SIMPLE, FALSE);
}
/* }}} */
/* {{{ GObject::connect_simple_after */
static PHP_METHOD(GObject, connect_simple_after)
{
    phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHPG_CONNECT_SIMPLE, TRUE);
}
/* }}} */
/* {{{ GObject::connect_object */
static PHP_METHOD(GObject, connect_object)
{
    phpg_warn_deprecated("use connect() or connect_simple()" TSRMLS_CC);
    phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHPG_CONNECT_OBJECT, FALSE);
}
/* }}} */
/* {{{ GObject::connect_object_after */
static PHP_METHOD(GObject, connect_object_after)
{
    phpg_warn_deprecated("use connect_after() or connect_simple_after()" TSRMLS_CC);
    phpg_signal_connect_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHPG_CONNECT_OBJECT, TRUE);
}
/* }}} */

/* {{{ GObject::notify stuff */
static PHP_METHOD(GObject, notify)
{
    char *property;
    GParamSpec *pspec;
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

    g_object_notify(obj, property);
}

static PHP_METHOD(GObject, freeze_notify)
{
    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "")) {
        return;
    }

    g_object_freeze_notify(PHPG_GOBJECT(this_ptr));
}

static PHP_METHOD(GObject, thaw_notify)
{
    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "")) {
        return;
    }

    g_object_thaw_notify(PHPG_GOBJECT(this_ptr));
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
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "class '%s' does not support property '%s'",
                         g_type_name(G_TYPE_FROM_INSTANCE(obj)), property);
        return;
    }

    if (!(pspec->flags & G_PARAM_READABLE)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "property '%s' is not readable", property);
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
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "class '%s' does not support property '%s'",
                         g_type_name(G_TYPE_FROM_INSTANCE(obj)), property);
        return;
    }

    if (!(pspec->flags & G_PARAM_WRITABLE)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "property '%s' is not writable", property);
        return;
    }

    g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
    if (phpg_param_gvalue_from_zval(&value, &php_value, pspec TSRMLS_CC) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not convert value to property type");
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

/* {{{ GObject::stop_emission */
static PHP_METHOD(GObject, stop_emission)
{
    char *signal;
    guint signal_id;
    GQuark detail;
    GObject *obj;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &signal)) {
        return;
    }

    obj = PHPG_GOBJECT(this_ptr);
    if (!g_signal_parse_name(signal, G_OBJECT_TYPE(obj), &signal_id, &detail, TRUE)) {
        php_error(E_WARNING, "%s(): unknown signal name '%s'", get_active_function_name(TSRMLS_C), signal);
        return;
    }

    g_signal_stop_emission(obj, signal_id, detail);
}
/* }}} */

/* {{{ GObject::emit */
static PHP_METHOD(GObject, emit)
{
    char *signal;
    zval *extra = NULL, **item;
    guint signal_id;
    GQuark detail;
    GObject *obj;
    GSignalQuery query;
    GValue *params, ret = { 0, };
    int i;

    NOT_STATIC_METHOD();

    if (!php_gtk_parse_varargs(ZEND_NUM_ARGS(), 1, &extra, "s", &signal)) {
        return;
    }

    obj = PHPG_GOBJECT(this_ptr);
    if (!g_signal_parse_name(signal, G_OBJECT_TYPE(obj), &signal_id, &detail, TRUE)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown signal name '%s'", signal);
        if (extra) zval_ptr_dtor(&extra);
        return;
    }

    g_signal_query(signal_id, &query);
    if (extra && zend_hash_num_elements(Z_ARRVAL_P(extra)) != (int)query.n_params) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "%d parameters needed for signal '%s', %d given", query.n_params, signal, zend_hash_num_elements(Z_ARRVAL_P(extra)));
        zval_ptr_dtor(&extra);
        return;
    }

    params = ecalloc(query.n_params+1, sizeof(GValue));
    g_value_init(&params[0], G_OBJECT_TYPE(obj));
    g_value_set_object(&params[0], G_OBJECT(obj));
    i = 1;

    if (extra) {
        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(extra));
             zend_hash_get_current_data(Z_ARRVAL_P(extra), (void**) &item) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(extra)), i++)
        {
            g_value_init(&params[i], query.param_types[i-1] & ~G_SIGNAL_TYPE_STATIC_SCOPE);

            if (phpg_gvalue_from_zval(&params[i], item, TRUE TSRMLS_CC) == FAILURE) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING,
                                 "could not convert value to %s for parameter %d",
                                 g_type_name(G_VALUE_TYPE(&params[i])), i);
                goto cleanup;
            }
        }
    }
    
    if (query.return_type != G_TYPE_NONE) {
        g_value_init(&ret, query.return_type & ~G_SIGNAL_TYPE_STATIC_SCOPE);
    }

    g_signal_emitv(params, signal_id, detail, &ret);

    if ((query.return_type & ~G_SIGNAL_TYPE_STATIC_SCOPE) != G_TYPE_NONE) {
        phpg_gvalue_to_zval(&ret, &return_value, TRUE, TRUE TSRMLS_CC);
        g_value_unset(&ret);
    }

cleanup:
    for (--i ; i >= 0; i--) {
        g_value_unset(&params[i]);
    }
    efree(params);
    if (extra) {
        zval_ptr_dtor(&extra);
    }
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
    phpg_gtype_new(temp, query.itype TSRMLS_CC);
    add_next_index_zval(return_value, temp);
    add_next_index_long(return_value, query.signal_flags);
    MAKE_STD_ZVAL(temp);
    phpg_gtype_new(temp, query.return_type TSRMLS_CC);
    add_next_index_zval(return_value, temp);
    MAKE_STD_ZVAL(params);
    array_init(params);
    for (i = 0; i < query.n_params; i++) {
        MAKE_STD_ZVAL(temp);
        phpg_gtype_new(temp, query.param_types[i] TSRMLS_CC);
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

static PHP_METHOD(GObject, list_properties)
{
    GParamSpec **specs;
    guint n_specs;
    zval *php_type;
    GType type;
    GObjectClass *oclass;
    guint i;

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "V", &php_type)) {
        return;
    }

    if ((type = phpg_gtype_from_zval(php_type)) == 0) {
        return;
    }

    if (!g_type_is_a(type, G_TYPE_OBJECT)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "type must be derived from GObject");
        return;
    }

    oclass = g_type_class_ref(type);
    if (!oclass) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "couuld not get a reference to type class");
        return;
    }

    specs = g_object_class_list_properties(oclass, &n_specs);
    array_init(return_value);
    for (i = 0; i < n_specs; i++) {
        zval *item = NULL;
        phpg_paramspec_new(&item, specs[i] TSRMLS_CC);
        add_next_index_zval(return_value, item);
    }

    g_free(specs);
    g_type_class_unref(oclass);
}

/* {{{ GObject::register_type */

static void phpg_object_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    zval *php_object = NULL;
    zval *php_pspec = NULL;
    zval *retval = NULL;

	TSRMLS_FETCH();
    phpg_gobject_new(&php_object, object TSRMLS_CC);
    phpg_paramspec_new(&php_pspec, pspec TSRMLS_CC);

    zend_call_method_with_1_params(&php_object, Z_OBJCE_P(php_object), NULL, "__get_gproperty", &retval, php_pspec);
    SEPARATE_ZVAL(&retval);

    if (retval) {
        if (phpg_gvalue_from_zval(value, &retval, TRUE TSRMLS_CC) == FAILURE) {
            php_error(E_WARNING, "phpg_object_get_property: could not convert PHP value to GValue");
        }
        zval_ptr_dtor(&retval);
    } else {
        php_error(E_WARNING, "phpg_object_get_property: error invoking do_get_property");
    }
    zval_ptr_dtor(&php_object);
    zval_ptr_dtor(&php_pspec);
}

static void phpg_object_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    zval *php_object = NULL;
    zval *php_pspec = NULL;
    zval *php_value = NULL;
    zval *retval = NULL;

	TSRMLS_FETCH();
    if (phpg_gvalue_to_zval(value, &php_value, TRUE, TRUE TSRMLS_CC) == FAILURE) {
        php_error(E_WARNING, "phpg_object_set_property: could not convert GValue to PHP value");
        zval_ptr_dtor(&php_value);
        return;
    }

    phpg_gobject_new(&php_object, object TSRMLS_CC);
    phpg_paramspec_new(&php_pspec, pspec TSRMLS_CC);

    zend_call_method_with_2_params(&php_object, Z_OBJCE_P(php_object), NULL, "__set_gproperty", &retval, php_pspec, php_value);

    if (retval) {
        zval_ptr_dtor(&retval);
    }
    zval_ptr_dtor(&php_object);
    zval_ptr_dtor(&php_pspec);
    zval_ptr_dtor(&php_value);
}

static void phpg_object_class_init(GObjectClass *class, zend_class_entry *ce)
{
    class->get_property = phpg_object_get_property;
    class->set_property = phpg_object_set_property;
}

static GParamSpec* phpg_create_property(const char *prop_name, GType prop_type, const char *nick, const char *blurb, zval *type_args, GParamFlags flags)
{
    GParamSpec *pspec = NULL;
	TSRMLS_FETCH();

    switch (G_TYPE_FUNDAMENTAL(prop_type)) {
        case G_TYPE_CHAR:
        {
            gchar minimum, maximum, default_val;
            if (!php_gtk_parse_args_hash(type_args, "ccc", &minimum, &maximum, &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_char(prop_name, nick, blurb, minimum, maximum, default_val, flags);
        }
        break;

        case G_TYPE_UCHAR:
        {
            gchar minimum, maximum, default_val;
            if (!php_gtk_parse_args_hash(type_args, "ccc", &minimum, &maximum, &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_uchar(prop_name, nick, blurb, minimum, maximum, default_val, flags);
        }
        break;

        case G_TYPE_BOOLEAN:
        {
            zend_bool default_val;
            if (!php_gtk_parse_args_hash(type_args, "b", &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_boolean(prop_name, nick, blurb, (gboolean)default_val, flags);
        }
        break;

        case G_TYPE_INT:
        {
            int minimum, maximum, default_val;
            if (!php_gtk_parse_args_hash(type_args, "iii", &minimum, &maximum, &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_int(prop_name, nick, blurb, (gint)minimum, (gint)maximum, (gint)default_val, flags);
        }
        break;

        case G_TYPE_UINT:
        {
            int minimum, maximum, default_val;
            if (!php_gtk_parse_args_hash(type_args, "iii", &minimum, &maximum, &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_uint(prop_name, nick, blurb, (guint)minimum, (guint)maximum, (guint)default_val, flags);
        }
        break;

        case G_TYPE_LONG:
        {
            long minimum, maximum, default_val;
            if (!php_gtk_parse_args_hash(type_args, "lll", &minimum, &maximum, &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_long(prop_name, nick, blurb, (glong)minimum, (glong)maximum, (glong)default_val, flags);
        }
        break;

        case G_TYPE_ULONG:
        {
            long minimum, maximum, default_val;
            if (!php_gtk_parse_args_hash(type_args, "lll", &minimum, &maximum, &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_ulong(prop_name, nick, blurb, (gulong)minimum, (gulong)maximum, (gulong)default_val, flags);
        }
        break;

        case G_TYPE_ENUM:
        {
            long default_val;
            if (!php_gtk_parse_args_hash(type_args, "l", &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_enum(prop_name, nick, blurb, prop_type, default_val, flags);
        }
        break;

        case G_TYPE_FLAGS:
        {
            long default_val;
            if (!php_gtk_parse_args_hash(type_args, "l", &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_flags(prop_name, nick, blurb, prop_type, default_val, flags);
        }
        break;

        case G_TYPE_FLOAT:
        {
            double minimum, maximum, default_val;
            if (!php_gtk_parse_args_hash(type_args, "ddd", &minimum, &maximum, &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_float(prop_name, nick, blurb, (float)minimum,
                                       (float)maximum, (float)default_val, flags);
        }
        break;

        case G_TYPE_DOUBLE:
        {
            double minimum, maximum, default_val;
            if (!php_gtk_parse_args_hash(type_args, "ddd", &minimum, &maximum, &default_val)) {
                return NULL;
            }
            pspec = g_param_spec_double(prop_name, nick, blurb, minimum, maximum, default_val, flags);
        }
        break;

        case G_TYPE_STRING:
        {
            char *default_val;
            zend_bool free_default_val = FALSE;

            if (!php_gtk_parse_args_hash(type_args, "u", &default_val, &free_default_val)) {
                return NULL;
            }
            pspec = g_param_spec_string(prop_name, nick, blurb, default_val, flags);
            if (free_default_val) g_free(default_val);
        }
        break;

        case G_TYPE_PARAM:
        {
            pspec = g_param_spec_param(prop_name, nick, blurb, prop_type, flags);
        }
        break;

        case G_TYPE_BOXED:
        {
            pspec = g_param_spec_boxed(prop_name, nick, blurb, prop_type, flags);
        }
        break;

        case G_TYPE_POINTER:
        {
            pspec = g_param_spec_pointer(prop_name, nick, blurb, flags);
        }
        break;

        case G_TYPE_OBJECT:
        {
            pspec = g_param_spec_object(prop_name, nick, blurb, prop_type, flags);
        }
        break;

        default:
            /* unhandled pspec type */
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not create param spec for type '%s'", g_type_name(prop_type));
            return NULL;

    }

    return pspec;
}

static int phpg_register_properties(GType type, zval *properties)
{
    GObjectClass *oclass;
    GParamFlags flags;
    GParamSpec *pspec;
    GType prop_type;
    char *str_key;
    const char *nick, *blurb;
    int data_len;
    int retval = SUCCESS;
    uint str_key_len;
    ulong num_key;
    zval **data;
    zval *type_args = NULL;
    zval *php_prop_type;

	TSRMLS_FETCH();

    oclass = g_type_class_ref(type);
    if (!oclass) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "couuld not get a reference to type class");
        return FAILURE;
    }

    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(properties));
         zend_hash_get_current_data(Z_ARRVAL_P(properties), (void**)&data) == SUCCESS;
            zend_hash_move_forward(Z_ARRVAL_P(properties))) {

        if (zend_hash_get_current_key_ex(Z_ARRVAL_P(properties), &str_key, &str_key_len,
                                         &num_key, 0, NULL) != HASH_KEY_IS_STRING) {
            continue;
        }
        
        if (Z_TYPE_PP(data) != IS_ARRAY) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "property specs have to be arrays");
            retval = FAILURE;
            break;
        }

        if ((data_len = zend_hash_num_elements(Z_ARRVAL_PP(data))) < 4) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "property specs must have at least 4 elements");
            retval = FAILURE;
            break;
        }

        if (!php_gtk_parse_varargs_hash(*data, 4, &type_args, "Vssi", &php_prop_type, &nick, &blurb, &flags)) {
            retval = FAILURE;
            break;
        }

        prop_type = phpg_gtype_from_zval(php_prop_type);
        if (prop_type == 0) {
            retval = FAILURE;
            break;
        }
        if (type_args == NULL &&
            (prop_type != G_TYPE_PARAM   &&
             prop_type != G_TYPE_BOXED   &&
             prop_type != G_TYPE_POINTER &&
             prop_type != G_TYPE_OBJECT)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "property specs require type-specific args");
            retval = FAILURE;
            break;
        }
        
        pspec = phpg_create_property(str_key, prop_type, nick, blurb, type_args, flags);
        if (pspec) {
            g_object_class_install_property(oclass, 1, pspec);
        } else {
            retval = FAILURE;
            break;
        }

        if (type_args) {
            zval_ptr_dtor(&type_args);
        }
    }

    if (type_args) {
        zval_ptr_dtor(&type_args);
    }
    g_type_class_unref(oclass);
    return retval;
}

static int phpg_override_signal(GType type, const char *signal_name)
{
    guint signal_id;

	TSRMLS_FETCH();

    signal_id = g_signal_lookup(signal_name, type);
    if (!signal_id) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not find signal '%s'", signal_name);
        return FAILURE;
    }

    g_signal_override_class_closure(signal_id, type, phpg_get_signal_class_closure());
    return SUCCESS;
}

static int phpg_create_signal(GType type, const char *signal_name, zval *data)
{
    long signal_flags;
    zval *php_return_type;
    zval *php_param_types;
    GType return_type;
    GType *param_types;
    guint n_params, i, signal_id;
    zval **item;

	TSRMLS_FETCH();

    if (!php_gtk_parse_args_hash(data, "lVa", &signal_flags, &php_return_type, &php_param_types)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "incorrect format for '%s' signal spec", signal_name);
        return FAILURE;
    }

    return_type = phpg_gtype_from_zval(php_return_type);
    if (0 == return_type) {
        return FAILURE;
    }

    n_params = zend_hash_num_elements(Z_ARRVAL_P(php_param_types));
    param_types = safe_emalloc(n_params, sizeof(GType), 0);
    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(php_param_types)), i = 0;
         zend_hash_get_current_data(Z_ARRVAL_P(php_param_types), (void**)&item) == SUCCESS;
         zend_hash_move_forward(Z_ARRVAL_P(php_param_types)), i++) {

        param_types[i] = phpg_gtype_from_zval(*item);
        if (0 == param_types[i]) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not get type of param #%d gor signal '%s'", i+1, signal_name);
            efree(param_types);
            return FAILURE;
        }
    }

    signal_id = g_signal_newv(signal_name, type, signal_flags, phpg_get_signal_class_closure(), 
                              (GSignalAccumulator)0, NULL, (GSignalCMarshaller)0,
                              return_type, n_params, param_types);
    efree(param_types);

    if (0 == signal_id) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not create signal '%s'", signal_name);
        return FAILURE;
    }

    return SUCCESS;
}

static int phpg_register_signals(GType type, zval *signals)
{
    GObjectClass *oclass;
    zval **data;
    char *str_key;
    uint str_key_len;
    ulong num_key;
    int retval = SUCCESS;

	TSRMLS_FETCH();

    oclass = g_type_class_ref(type);
    if (!oclass) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "couuld not get a reference to type class");
        return FAILURE;
    }

    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(signals));
         zend_hash_get_current_data(Z_ARRVAL_P(signals), (void**)&data) == SUCCESS;
         zend_hash_move_forward(Z_ARRVAL_P(signals))) {

        if (zend_hash_get_current_key_ex(Z_ARRVAL_P(signals), &str_key, &str_key_len,
                                         &num_key, 0, NULL) != HASH_KEY_IS_STRING) {
            continue;
        }

        if (Z_TYPE_PP(data) == IS_NULL ||
            (Z_TYPE_PP(data) == IS_STRING &&
             !strcmp(Z_STRVAL_PP(data), "override"))) {
            retval = phpg_override_signal(type, str_key);
        } else {
            retval = phpg_create_signal(type, str_key, *data);
        }

        if (retval == FAILURE) {
            break;
        }
    }

    g_type_class_unref(oclass);
    return retval;
}

static PHP_METHOD(GObject, register_type)
{
    zend_class_entry *class = gobject_ce;
    GType parent_type, new_type;
    GTypeQuery query;
    const char *type_name;
    zval **prop_decls, **signal_decls;

    GTypeInfo type_info = {
        0,    /* class_size */

        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,

        (GClassInitFunc) phpg_object_class_init,
        (GClassFinalizeFunc) NULL,
        NULL, /* class_data */

        0,    /* instance_size */
        0,    /* n_preallocs */
        (GInstanceInitFunc) NULL
    };

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "C", &class)) {
        return;
    }

    parent_type = phpg_gtype_from_class(class TSRMLS_CC);
    if (!parent_type) {
        return;
    }

    type_name = class->name;
    if (g_type_from_name(type_name) != 0) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "type '%s' already exists?", type_name);
        return;
    }

    type_info.class_data = class;
    g_type_query(parent_type, &query);
    type_info.class_size = query.class_size;
    type_info.instance_size = query.instance_size;

    new_type = g_type_register_static(parent_type, type_name, &type_info, 0);
    if (new_type == 0) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not create new GType");
        return;
    }

	if (!phpg_class_key) {
		phpg_class_key = g_quark_from_static_string(phpg_class_id);
	}

    g_type_set_qdata(new_type, phpg_class_key, class);
    zend_declare_class_constant_long(class, "gtype", sizeof("gtype")-1, new_type TSRMLS_CC);

    zend_update_class_constants(class TSRMLS_CC);

    /* register properties */
    if (zend_hash_find(&class->default_properties, "__gproperties", sizeof("__gproperties"), (void**)&prop_decls) == SUCCESS) {
        if (Z_TYPE_PP(prop_decls) != IS_ARRAY) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "__gproperties variable has to be an array");
            return;
        }
        if (phpg_register_properties(new_type, *prop_decls) == FAILURE) {
            return;
        }
        zend_hash_del(&class->default_properties, "__gproperties", sizeof("__gproperties"));
    }

    /* register signals */
    if (zend_hash_find(&class->default_properties, "__gsignals", sizeof("__gsignals"), (void**)&signal_decls) == SUCCESS) {
        if (Z_TYPE_PP(signal_decls) != IS_ARRAY) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "__gsignals variable has to be an array");
            return;
        }
        if (phpg_register_signals(new_type, *signal_decls) == FAILURE) {
            return;
        }
        zend_hash_del(&class->default_properties, "__gsignals", sizeof("__gsignals"));
    }

    /*TODO interface registration */
#if 0
    if (class->num_interfaces > 0) {
        int i;
        GType itype;
        zend_classs_entry *iface;
        const GInterfaceInfo *iinfo;
        
        for (i = 0; i < class->num_interfaces; i++) {

            /* TODO might have to create GInterface type and check if interface class
             * inherits from it
             */
            itype = phpg_gtype_from_class(class->interfaces[i]);
            /* TODO get interface info */
        }
    }
#endif

    /* run class init */
    /* TODO virtual method stuff */

    RETURN_TRUE;
}
/* }}} */

/* {{{ GObject reflection info */
static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_block, 0)
    ZEND_ARG_INFO(0, handler_id)
ZEND_END_ARG_INFO();

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
ZEND_BEGIN_ARG_INFO_EX(arginfo_gobject_connect_simple, 0, 0, 2)
    ZEND_ARG_INFO(0, signal)
    ZEND_ARG_INFO(0, callback)
    ZEND_ARG_INFO(0, userparam)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO_EX(arginfo_gobject_connect_simple_after, 0, 0, 2)
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
ZEND_BEGIN_ARG_INFO(arginfo_gobject_disconnect, 0)
    ZEND_ARG_INFO(0, handler_id)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_notify, 0)
    ZEND_ARG_INFO(0, property_name)
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
ZEND_BEGIN_ARG_INFO(arginfo_gobject_is_connected, 0)
    ZEND_ARG_INFO(0, handler_id)
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

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_unblock, 0)
    ZEND_ARG_INFO(0, handler_id)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_list_properties, 0)
    ZEND_ARG_INFO(0, gtype)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_stop_emission, 0)
    ZEND_ARG_INFO(0, signal)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_gobject_register_type, 0)
    ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO();

/* }}} */

static zend_function_entry gobject_methods[] = {
    PHP_ME(GObject, __construct,          NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, __tostring,           NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect,              arginfo_gobject_connect               , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect_after,        arginfo_gobject_connect_after         , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect_object,       arginfo_gobject_connect_object        , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect_object_after, arginfo_gobject_connect_object_after  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect_simple,       arginfo_gobject_connect_simple        , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, connect_simple_after, arginfo_gobject_connect_simple_after  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, notify,               arginfo_gobject_notify                , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, freeze_notify,        NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, thaw_notify,          NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, get_property,         arginfo_gobject_get_property          , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, set_property,         arginfo_gobject_set_property          , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, get_data,             arginfo_gobject_get_data              , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, set_data,             arginfo_gobject_set_data              , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, emit,                 NULL                                  , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, block,                arginfo_gobject_block                 , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, unblock,              arginfo_gobject_unblock               , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, disconnect,           arginfo_gobject_disconnect            , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, is_connected,         arginfo_gobject_is_connected          , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, signal_query,         arginfo_gobject_signal_query          , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(GObject, stop_emission,        arginfo_gobject_stop_emission         , ZEND_ACC_PUBLIC)
    PHP_ME(GObject, register_type,        arginfo_gobject_register_type         , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(GObject, list_properties,      arginfo_gobject_list_properties       , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(GObject, signal_list_ids,      arginfo_gobject_signal_list_ids       , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(GObject, signal_list_names,    arginfo_gobject_signal_list_names     , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_MALIAS(GObject, emit_stop_by_name, stop_emission, arginfo_gobject_stop_emission, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};


PHPG_PROP_READER(GObject, gtype)
{
    ZVAL_LONG(return_value, G_OBJECT_TYPE(((phpg_gobject_t *)object)->obj));
    return SUCCESS;
}

static prop_info_t gobject_props_info[] = {
    { "gtype", PHPG_PROP_READ_FN(GObject, gtype), NULL },
    { NULL, NULL, NULL },
};


void phpg_gobject_register_self(TSRMLS_D)
{
    if (gobject_ce) return;

    gobject_wrapper_handle_key   = g_quark_from_static_string("phpg-wrapper-handle");
    gobject_wrapper_handlers_key = g_quark_from_static_string("phpg-wrapper-handlers");
    gobject_wrapper_owned_key    = g_quark_from_static_string("phpg-wrapper-owned");

    phpg_gobject_handlers = php_gtk_handlers;
    phpg_gobject_handlers.del_ref = phpg_gobject_del_ref;

    gobject_ce = phpg_register_class("GObject", gobject_methods, NULL, 0, gobject_props_info, NULL, G_TYPE_OBJECT TSRMLS_CC);
    phpg_register_int_constant(gobject_ce, "gtype", sizeof("gtype")-1, G_TYPE_OBJECT);

    /* Fundamental GTypes */
    phpg_register_int_constant(gobject_ce, "TYPE_INVALID", sizeof("TYPE_INVALID")-1, G_TYPE_INVALID);
    phpg_register_int_constant(gobject_ce, "TYPE_NONE", sizeof("TYPE_NONE")-1, G_TYPE_NONE);
    phpg_register_int_constant(gobject_ce, "TYPE_INTERFACE", sizeof("TYPE_INTERFACE")-1, G_TYPE_INTERFACE);
    phpg_register_int_constant(gobject_ce, "TYPE_CHAR", sizeof("TYPE_CHAR")-1, G_TYPE_CHAR);
    phpg_register_int_constant(gobject_ce, "TYPE_BOOLEAN", sizeof("TYPE_BOOLEAN")-1, G_TYPE_BOOLEAN);
    phpg_register_int_constant(gobject_ce, "TYPE_LONG", sizeof("TYPE_LONG")-1, G_TYPE_LONG);
    phpg_register_int_constant(gobject_ce, "TYPE_ENUM", sizeof("TYPE_ENUM")-1, G_TYPE_ENUM);
    phpg_register_int_constant(gobject_ce, "TYPE_FLAGS", sizeof("TYPE_FLAGS")-1, G_TYPE_FLAGS);
    phpg_register_int_constant(gobject_ce, "TYPE_DOUBLE", sizeof("TYPE_DOUBLE")-1, G_TYPE_DOUBLE);
    phpg_register_int_constant(gobject_ce, "TYPE_STRING", sizeof("TYPE_STRING")-1, G_TYPE_STRING);
    phpg_register_int_constant(gobject_ce, "TYPE_POINTER", sizeof("TYPE_POINTER")-1, G_TYPE_POINTER);
    phpg_register_int_constant(gobject_ce, "TYPE_BOXED", sizeof("TYPE_BOXED")-1, G_TYPE_BOXED);
    phpg_register_int_constant(gobject_ce, "TYPE_PARAM", sizeof("TYPE_PARAM")-1, G_TYPE_PARAM);
    phpg_register_int_constant(gobject_ce, "TYPE_OBJECT", sizeof("TYPE_OBJECT")-1, G_TYPE_OBJECT);
    phpg_register_int_constant(gobject_ce, "TYPE_PHP_VALUE", sizeof("TYPE_PHP_VALUE")-1, G_TYPE_PHP_VALUE);

	/* Priority Constants */
    phpg_register_int_constant(gobject_ce, "PRIORITY_HIGH", sizeof("PRIORITY_HIGH")-1, G_PRIORITY_HIGH);
    phpg_register_int_constant(gobject_ce, "PRIORITY_DEFAULT", sizeof("PRIORITY_DEFAULT")-1, G_PRIORITY_DEFAULT);
    phpg_register_int_constant(gobject_ce, "PRIORITY_HIGH_IDLE", sizeof("PRIORITY_HIGH_IDLE")-1, G_PRIORITY_HIGH_IDLE);
    phpg_register_int_constant(gobject_ce, "PRIORITY_DEFAULT_IDLE", sizeof("PRIORITY_DEFAULT_IDLE")-1, G_PRIORITY_DEFAULT_IDLE);
    phpg_register_int_constant(gobject_ce, "PRIORITY_LOW", sizeof("PRIORITY_LOW")-1, G_PRIORITY_LOW);

    /* IO Condition Constants */
    phpg_register_int_constant(gobject_ce, "IO_IN", sizeof("IO_IN")-1, G_IO_IN);
    phpg_register_int_constant(gobject_ce, "IO_OUT", sizeof("IO_OUT")-1, G_IO_OUT);
    phpg_register_int_constant(gobject_ce, "IO_PRI", sizeof("IO_PRI")-1, G_IO_PRI);
    phpg_register_int_constant(gobject_ce, "IO_ERR", sizeof("IO_ERR")-1, G_IO_ERR);
    phpg_register_int_constant(gobject_ce, "IO_HUP", sizeof("IO_HUP")-1, G_IO_HUP);
    phpg_register_int_constant(gobject_ce, "IO_NVAL", sizeof("IO_NVAL")-1, G_IO_NVAL);

	/* Signal type constants */
	phpg_register_int_constant(gobject_ce, "SIGNAL_RUN_FIRST", sizeof("SIGNAL_RUN_FIRST")-1, G_SIGNAL_RUN_FIRST);
    phpg_register_int_constant(gobject_ce, "SIGNAL_RUN_LAST", sizeof("SIGNAL_RUN_LAST")-1, G_SIGNAL_RUN_LAST);
    phpg_register_int_constant(gobject_ce, "SIGNAL_RUN_CLEANUP", sizeof("SIGNAL_RUN_CLEANUP")-1, G_SIGNAL_RUN_CLEANUP);
    phpg_register_int_constant(gobject_ce, "SIGNAL_NO_RECURSE", sizeof("SIGNAL_NO_RECURSE")-1, G_SIGNAL_NO_RECURSE);
    phpg_register_int_constant(gobject_ce, "SIGNAL_DETAILED", sizeof("SIGNAL_DETAILED")-1, G_SIGNAL_DETAILED);
    phpg_register_int_constant(gobject_ce, "SIGNAL_ACTION", sizeof("SIGNAL_ACTION")-1, G_SIGNAL_ACTION);
    phpg_register_int_constant(gobject_ce, "SIGNAL_NO_HOOKS", sizeof("SIGNAL_NO_HOOKS")-1, G_SIGNAL_NO_HOOKS);
    
    /* Paramspec constants */
    phpg_register_int_constant(gobject_ce, "PARAM_READABLE", sizeof("PARAM_READABLE")-1, G_PARAM_READABLE);
    phpg_register_int_constant(gobject_ce, "PARAM_WRITABLE", sizeof("PARAM_WRITABLE")-1, G_PARAM_WRITABLE);
    phpg_register_int_constant(gobject_ce, "PARAM_CONSTRUCT", sizeof("PARAM_CONSTRUCT")-1, G_PARAM_CONSTRUCT);
    phpg_register_int_constant(gobject_ce, "PARAM_CONSTRUCT_ONLY", sizeof("PARAM_CONSTRUCT_ONLY")-1, G_PARAM_CONSTRUCT_ONLY);
    phpg_register_int_constant(gobject_ce, "PARAM_LAX_VALIDATION", sizeof("PARAM_LAX_VALIDATION")-1, G_PARAM_LAX_VALIDATION);
    phpg_register_int_constant(gobject_ce, "PARAM_READWRITE", sizeof("PARAM_READWRITE")-1, G_PARAM_READWRITE);
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */
