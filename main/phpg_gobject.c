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

static const gchar *gobject_wrapper_handle_id  = "phpg_wrapper_handle";
static GQuark       gobject_wrapper_handle_key = 0;
static const gchar *gobject_wrapper_handlers_id   = "phpg_wrapper_handlers";
static GQuark       gobject_wrapper_handlers_key  = 0;

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
    g_object_set_qdata(object->obj, gobject_wrapper_handle_key, NULL);
    g_object_set_qdata(object->obj, gobject_wrapper_handlers_key, NULL);

	if (object->obj && object->dtor)
		object->dtor(object->obj);
    object->obj = NULL;

    tmp = object->closures;
    while (tmp) {
        g_closure_invalidate((GClosure *)tmp->data);
        tmp = tmp->next;
    }
    object->closures = NULL;

    efree(object);
}
/* }}} */

/* {{{ static      phpg_class_from_gtype() */
static zend_class_entry* phpg_class_from_gtype(GType gtype)
{
	zend_class_entry *ce = NULL;

	while (gtype != G_TYPE_INVALID
		   && (ce = g_type_get_qdata(gtype, phpg_class_key)) == NULL) {
		gtype = g_type_parent(gtype);
	}
	
	assert(ce != NULL);
	return ce;
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


/* {{{ zval*       phpg_read_property() */
zval* phpg_read_property(zval *object, zval *member, int type TSRMLS_DC)
{
	phpg_head_t *poh = NULL;
	zval tmp_member;
	zval result, *result_ptr = NULL;
	prop_info_t *pi = NULL;
	int ret;

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	ret = FAILURE;

	poh = (phpg_head_t *) zend_object_store_get_object(object TSRMLS_CC);
	if (poh->pi_hash) {
		ret = zend_hash_find(poh->pi_hash, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &pi);
	}

	if (ret == SUCCESS) {
        ZVAL_NULL(&result);
		ret = pi->read(poh, &result);
		if (ret == SUCCESS) {
			ALLOC_ZVAL(result_ptr);
			*result_ptr = result;
            //INIT_PZVAL(result_ptr);
            result_ptr->refcount = 0;
            result_ptr->is_ref = 0;
        } else {
            result_ptr = EG(uninitialized_zval_ptr);
        }
	} else {
		result_ptr = zend_get_std_object_handlers()->read_property(object, member, type TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}

	return result_ptr;
}
/* }}} */

/* {{{ void        phpg_write_property() */
void phpg_write_property(zval *object, zval *member, zval *value TSRMLS_DC)
{
	phpg_head_t *poh = NULL;
	zval tmp_member;
	prop_info_t *pi;
	int ret;

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

    ret = FAILURE;
	poh = (phpg_head_t *) zend_object_store_get_object(object TSRMLS_CC);
	if (poh->pi_hash) {
		ret = zend_hash_find(poh->pi_hash, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &pi);
	}

    if (ret == SUCCESS) {
        if (pi->write) {
            pi->write(poh, value);
        } else {
            php_error(E_NOTICE, "PHP-GTK: ignoring write attempt to the read only property");
        }
    } else {
		zend_get_std_object_handlers()->write_property(object, member, value TSRMLS_CC);
    }

	if (member == &tmp_member) {
		zval_dtor(member);
	}
}
/* }}} */

/* {{{ HashTable*  phpg_get_properties() */
HashTable* phpg_get_properties(zval *object TSRMLS_DC)
{
	HashTable *pi_hash;
	phpg_head_t *poh = NULL;
	prop_info_t *pi;
	zval result, *result_ptr;
	int ret;

	poh = (phpg_head_t *) zend_object_store_get_object(object TSRMLS_CC);
    pi_hash = poh->pi_hash;
    for (zend_hash_internal_pointer_reset(pi_hash);
         zend_hash_get_current_data(pi_hash, (void **)&pi) == SUCCESS;
         zend_hash_move_forward(pi_hash)) {

        ret = pi->read(poh, &result);
        if (ret == SUCCESS) {
            ALLOC_ZVAL(result_ptr);
            *result_ptr = result;
            INIT_PZVAL(result_ptr);
            zend_hash_update(poh->zobj.properties, (char *)pi->name, strlen(pi->name)+1, &result_ptr, sizeof(zval *), NULL);
        }
    }

	return poh->zobj.properties;
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

	zov.handlers = &php_gtk_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) phpg_free_gobject_storage, NULL TSRMLS_CC);

	return zov;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_init_object() */
PHP_GTK_API void phpg_init_object(void *object, zend_class_entry *ce)
{
	zval *tmp;
	zend_class_entry *prop_ce;
	phpg_head_t *poh = (phpg_head_t *) object;

	poh->zobj.ce = ce;
	poh->zobj.in_get = 0;
	poh->zobj.in_set = 0;
	poh->pi_hash = NULL;

	ALLOC_HASHTABLE(poh->zobj.properties);
	zend_hash_init(poh->zobj.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(poh->zobj.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	/*
	 * Find the nearest internal parent class and use its property handler
     * information
	 */
	prop_ce = ce;
	while (prop_ce->type != ZEND_INTERNAL_CLASS && prop_ce->parent != NULL) {
		prop_ce = prop_ce->parent;
	}

    zend_hash_find(&phpg_prop_info, prop_ce->name, prop_ce->name_length+1, (void **) &poh->pi_hash);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_register_class() */
PHP_GTK_API zend_class_entry* phpg_register_class(const char *class_name,
                                                  function_entry *class_methods,
                                                  zend_class_entry *parent,
                                                  zend_uint ce_flags,
                                                  prop_info_t *prop_info,
                                                  create_object_func_t create_obj_func,
                                                  GType gtype
                                                  TSRMLS_DC)
{
	zend_class_entry ce, *real_ce;
	HashTable pi_hash;
    HashTable *parent_pi_hash = NULL;
	prop_info_t *pi;

	if (!phpg_class_key) {
		phpg_class_key = g_quark_from_static_string(phpg_class_id);
	}

	memset(&ce, 0, sizeof(ce));

	ce.name = strdup(class_name);
	ce.name_length = strlen(class_name);
	ce.builtin_functions = class_methods;

	real_ce = zend_register_internal_class_ex(&ce, parent, NULL TSRMLS_CC);

    real_ce->ce_flags = ce_flags;
	if (create_obj_func) {
		real_ce->create_object = create_obj_func;
	} else {
		real_ce->create_object = phpg_create_gobject;
	}

    zend_hash_init(&pi_hash, 1, NULL, NULL, 1);
    if (prop_info) {
        pi = prop_info;
        /*
         * Only register properties with reader functions.
         */
        while (pi->name && pi->read) {
            zend_hash_update(&pi_hash, (char *)pi->name, strlen(pi->name)+1, pi, sizeof(prop_info_t), NULL);
            pi++;
        }
    }

    /*
     * Merge in parent's properties.
     */
    if (parent && zend_hash_find(&phpg_prop_info, parent->name, parent->name_length+1, (void **)&parent_pi_hash) == SUCCESS) {
        zend_hash_merge(&pi_hash, parent_pi_hash, NULL, NULL, sizeof(prop_info_t), 0);
    }
    zend_hash_add(&phpg_prop_info, ce.name, ce.name_length+1, &pi_hash, sizeof(HashTable), NULL);

    if (gtype) {
        /*
         * Since Zend engine does not let internal classes have constants or
         * static variables that are objects, we are forced to store the gtype
         * integer instead of the wrapper. What a fecking shame.
         */
        zval *g;
        g = (zval *)malloc(sizeof(zval));
        INIT_PZVAL(g);
        ZVAL_LONG(g, gtype);
        zend_hash_update(&real_ce->constants_table, "gtype", sizeof("gtype"), &g, sizeof(zval *), NULL);

        g_type_set_qdata(gtype, phpg_class_key, real_ce);
    }

	return real_ce;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_register_enum() */
void phpg_register_enum(GType gtype, const char *strip_prefix, zend_class_entry *ce)
{
    GEnumClass *eclass;
    char *enum_name;
    int i, j;
    int prefix_len = 0;

    g_return_if_fail(ce != NULL);
    g_return_if_fail(g_type_is_a(gtype, G_TYPE_ENUM));

    if (strip_prefix) {
        prefix_len = strlen(strip_prefix);
    }

    eclass = G_ENUM_CLASS(g_type_class_ref(gtype));
    for (i = 0; i < eclass->n_values; i++) {
        zval *val;
        
        val = (zval *)malloc(sizeof(zval));
        INIT_PZVAL(val);
        ZVAL_LONG(val, eclass->values[i].value);
        enum_name = eclass->values[i].value_name;
        if (strip_prefix) {
            for (j = prefix_len; j >= 0; j--) {
                if (g_ascii_isalpha(enum_name[j]) || enum_name[j] == '_') {
                    enum_name = &enum_name[j];
                    break;
                }
            }
        }
        zend_hash_update(&ce->constants_table, enum_name, strlen(enum_name)+1, &val, sizeof(zval *), NULL);
    }
    g_type_class_unref(eclass);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_register_flags() */
void phpg_register_flags(GType gtype, const char *strip_prefix, zend_class_entry *ce)
{
    GFlagsClass *eclass;
    char *enum_name;
    int i, j;
    int prefix_len = 0;

    g_return_if_fail(ce != NULL);
    g_return_if_fail(g_type_is_a(gtype, G_TYPE_FLAGS));

    if (strip_prefix) {
        prefix_len = strlen(strip_prefix);
    }

    eclass = G_FLAGS_CLASS(g_type_class_ref(gtype));
    for (i = 0; i < eclass->n_values; i++) {
        zval *val;
        
        val = (zval *)malloc(sizeof(zval));
        INIT_PZVAL(val);
        ZVAL_LONG(val, eclass->values[i].value);
        enum_name = eclass->values[i].value_name;
        if (strip_prefix) {
            for (j = prefix_len; j >= 0; j--) {
                if (g_ascii_isalpha(enum_name[j]) || enum_name[j] == '_') {
                    enum_name = &enum_name[j];
                    break;
                }
            }
        }
        zend_hash_update(&ce->constants_table, enum_name, strlen(enum_name)+1, &val, sizeof(zval *), NULL);
    }
    g_type_class_unref(eclass);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gobject_set_wrapper() */
PHP_GTK_API void phpg_gobject_set_wrapper(zval *zobj, GObject *obj TSRMLS_DC)
{
    phpg_gobject_t *pobj = NULL;

	if (!gobject_wrapper_handle_key) {
		gobject_wrapper_handle_key   = g_quark_from_static_string(gobject_wrapper_handle_id);
		gobject_wrapper_handlers_key = g_quark_from_static_string(gobject_wrapper_handlers_id);
	}

    phpg_sink_object(obj);
    pobj = (phpg_gobject_t *) zend_object_store_get_object(zobj TSRMLS_CC);
    pobj->obj = obj;
    pobj->dtor = (phpg_dtor_t) g_object_unref;
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

	if (!gobject_wrapper_handle_key) {
		gobject_wrapper_handle_key   = g_quark_from_static_string(gobject_wrapper_handle_id);
		gobject_wrapper_handlers_key = g_quark_from_static_string(gobject_wrapper_handlers_id);
	}

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
        handlers = (zend_object_handlers*) g_object_get_qdata(obj, gobject_wrapper_handlers_key);
		Z_TYPE_PP(zobj) = IS_OBJECT;
		Z_OBJ_HANDLE_PP(zobj) = handle;
		Z_OBJ_HT_PP(zobj) = handlers;
		zend_objects_store_add_ref(*zobj TSRMLS_CC);
	} else {
		ce = phpg_class_from_gtype(G_OBJECT_TYPE(obj));
		object_init_ex(*zobj, ce);
		g_object_ref(obj);

        phpg_sink_object(obj);
		pobj = (phpg_gobject_t *) zend_object_store_get_object(*zobj TSRMLS_CC);
		pobj->obj = obj;
		pobj->dtor = (phpg_dtor_t) g_object_unref;
		g_object_set_qdata(obj, gobject_wrapper_handle_key, (void*)Z_OBJ_HANDLE_PP(zobj));
		g_object_set_qdata(obj, gobject_wrapper_handlers_key, (void*)Z_OBJ_HT_PP(zobj));
	}
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gobject_watch_closure() */
PHP_GTK_API void phpg_gobject_watch_closure(zval *zobj, GClosure *closure TSRMLS_DC)
{
    phpg_gobject_t *pobj = NULL;

    g_return_if_fail(zobj != NULL);
    g_return_if_fail(Z_TYPE_P(zobj) == IS_OBJECT && instanceof_function(Z_OBJCE_P(zobj), gobject_ce TSRMLS_CC));
    g_return_if_fail(closure != NULL);

    pobj = zend_object_store_get_object(zobj TSRMLS_CC);
    g_return_if_fail(g_slist_find(pobj->closures, closure) == NULL);

    pobj->closures = g_slist_prepend(pobj->closures, closure);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_get_properties_helper */
PHP_GTK_API void phpg_get_properties_helper(zval *object, HashTable *ht TSRMLS_DC, ...)
{
    va_list va;
    char *prop;
    int prop_len;
    zval *result;
    zend_class_entry *ce = Z_OBJCE_P(object);

#ifdef ZTS
    va_start(va, tsrm_ls);
#else
    va_start(va, ht);
#endif
    while ((prop = va_arg(va, char *)) != NULL) {
        prop_len = va_arg(va, int);
        result = zend_read_property(ce, object, prop, prop_len, 1 TSRMLS_CC);
        zend_hash_update(ht, prop, prop_len+1, &result, sizeof(zval *), NULL);
    }

    va_end(va);
}
/* }}} */

zend_bool phpg_handle_gerror(GError **error TSRMLS_DC)
{
    if (error == NULL || *error == NULL)
        return FALSE;

    phpg_throw_gerror_exception(g_quark_to_string((*error)->domain),
                                (*error)->code,
                                (*error)->message TSRMLS_CC);
    g_clear_error(error);

    return TRUE;
}

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

void phpg_gobject_register_self()
{
	if (gobject_ce) return;

	gobject_ce = phpg_register_class("GObject", gobject_methods, NULL, 0, NULL, NULL, G_TYPE_OBJECT TSRMLS_CC);
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */
