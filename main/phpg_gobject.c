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

static const gchar *gobject_wrapper_id   = "phpg_wrapper";
static GQuark       gobject_wrapper_key  = 0;

HashTable phpg_prop_info;
/*
 * TODO: remove?
HashTable php_gtk_prop_getters;
HashTable php_gtk_prop_setters;
HashTable php_gtk_rsrc_hash;
HashTable php_gtk_type_hash;
HashTable php_gtk_prop_desc;
HashTable php_gtk_callback_hash;
*/

/* {{{ static      phpg_free_object_storage() */
static inline void phpg_free_object_storage(phpg_gobject_t *object, zend_object_handle handle TSRMLS_DC)
{
    zend_hash_destroy(object->zobj.properties);
    FREE_HASHTABLE(object->zobj.properties);
	if (object->obj && object->dtor)
		object->dtor(object->obj);
    object->obj = NULL;
    efree(object);
}
/* }}} */

/* {{{ static      phpg_create_object() */
static zend_object_value phpg_create_object(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value zov;
	phpg_gobject_t *object;

	object = emalloc(sizeof(phpg_gobject_t));
	phpg_init_object(object, ce);
	object->obj  = NULL;
	object->dtor = NULL;

	zov.handlers = &php_gtk_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) phpg_free_object_storage, NULL TSRMLS_CC);

	return zov;
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
	phpg_head_t *poh;
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

	poh = (phpg_head_t *) zend_objects_get_address(object TSRMLS_CC);
	if (poh->pi_hash) {
		ret = zend_hash_find(poh->pi_hash, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &pi);
	}

	if (ret == SUCCESS) {
        ZVAL_NULL(&result);
		ret = pi->read(poh, &result);
		if (ret == SUCCESS) {
			ALLOC_ZVAL(result_ptr);
			*result_ptr = result;
            INIT_PZVAL(result_ptr);
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
	phpg_head_t *poh;
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
	poh = (phpg_head_t *) zend_objects_get_address(object TSRMLS_CC);
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
	phpg_head_t *poh;
	prop_info_t *pi;
	zval result, *result_ptr;
	int ret;

	poh = (phpg_head_t *)zend_objects_get_address(object TSRMLS_CC);
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
	 * Find the nearest internal parent class
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
	if (create_obj_func) {
		ce.create_object = create_obj_func;
	} else {
		ce.create_object = phpg_create_object;
	}

	real_ce = zend_register_internal_class_ex(&ce, parent, NULL TSRMLS_CC);
    real_ce->ce_flags = ce_flags;

    zend_hash_init(&pi_hash, 1, NULL, NULL, 0);
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
        /* TODO store __gtype object. This is problematic since the shutdown
         * destructor does not know what to do with internal properties that are
         * objects and exits badly. */
        /*
        zval *g = phpg_gtype_new(gtype);
        zend_hash_update(real_ce->static_members, "__gtype", sizeof("__gtype"), &g, sizeof(zval *), NULL);
        */
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
    int prefix_len;

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
    int prefix_len;

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

	if (!gobject_wrapper_key) {
		gobject_wrapper_key = g_quark_from_static_string(gobject_wrapper_id);
	}

    phpg_sink_object(obj);
    zend_objects_store_add_ref(zobj TSRMLS_CC);
    pobj = zend_object_store_get_object(zobj TSRMLS_CC);
    pobj->obj = obj;
    pobj->dtor = (phpg_dtor_t) g_object_unref;
    g_object_set_qdata(pobj->obj, gobject_wrapper_key, (void*)Z_OBJ_HANDLE_P(zobj));
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gobject_new() */
PHP_GTK_API void phpg_gobject_new(zval **zobj, GObject *obj TSRMLS_DC)
{
	zend_class_entry *ce = NULL;
	phpg_gobject_t *pobj = NULL;
    zend_object_handle handle;

	if (!gobject_wrapper_key) {
		gobject_wrapper_key = g_quark_from_static_string(gobject_wrapper_id);
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

	handle = (zend_object_handle)g_object_get_qdata(obj, gobject_wrapper_key);
	if ((void*)handle != NULL) {
		Z_TYPE_PP(zobj) = IS_OBJECT;
		Z_OBJ_HANDLE_PP(zobj) = handle;
		Z_OBJ_HT_PP(zobj) = &php_gtk_handlers;
		zend_objects_store_add_ref(*zobj TSRMLS_CC);
	} else {
		ce = phpg_class_from_gtype(G_OBJECT_TYPE(obj));
		object_init_ex(*zobj, ce);
		g_object_ref(obj);

        phpg_sink_object(obj);
		pobj = zend_object_store_get_object(*zobj TSRMLS_CC);
		pobj->obj = obj;
		pobj->dtor = (phpg_dtor_t) g_object_unref;
		g_object_set_qdata(obj, gobject_wrapper_key, (void*)Z_OBJ_HANDLE_PP(zobj));
	}
}
/* }}} */

/*
 * GObject PHP class definition
 */

static PHP_METHOD(GObject, __construct)
{
}

/* {{{ GObject::__tostring() */
static PHP_METHOD(GObject, __tostring)
{
    char buf[256];
    GObject *obj = NULL;
    int numc = 0;

    NOT_STATIC_METHOD();

    obj = PHPG_GET(this_ptr);
    numc = snprintf(buf, sizeof(buf),
                    "[%s object (%s Gtk+ type)]", Z_OBJCE_P(this_ptr)->name,
                    obj ? G_OBJECT_TYPE_NAME(obj) : "uninitialized");
    RETURN_STRINGL(buf, numc, 1);
}
/* }}} */

static zend_function_entry gobject_methods[] = {
	ZEND_ME(GObject, __construct, NULL, ZEND_ACC_PUBLIC)
	ZEND_ME(GObject, __tostring, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

PHP_GTK_EXPORT_CE(gobject_ce) = NULL;

void phpg_gobject_register_self()
{
	if (gobject_ce) return;

	gobject_ce = phpg_register_class("GObject", gobject_methods, NULL, 0, NULL, NULL, G_TYPE_OBJECT TSRMLS_CC);
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */
