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
 * Support functions.
 */

/* {{{ phpg_read_property() */
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
		ret = pi->read(poh, &result TSRMLS_CC);
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

/* {{{ phpg_write_property() */
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
            pi->write(poh, value TSRMLS_CC);
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

/* {{{ phpg_get_properties() */
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

/* {{{ phpg_handle_gerror */
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
/* }}} */

/* {{{ phpg_get_properties_helper */
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
        /*
         * zend_read_property will return a temporary zval, so we need to
         * initialize it in order to keep it around.
         */
        INIT_PZVAL(result);
        zend_hash_update(ht, prop, prop_len+1, &result, sizeof(zval *), NULL);
    }

    va_end(va);
}
/* }}} */

/* {{{ phpg_destroy_notify */
PHP_GTK_API void phpg_destroy_notify(gpointer data)
{
    zval *value = (zval *) data;
    zval_ptr_dtor(&value);
}
/* }}} */

/* {{{ phpg_init_object() */
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

/* {{{ phpg_register_class() */
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

/* {{{ phpg_register_enum() */
void phpg_register_enum(GType gtype, const char *strip_prefix, zend_class_entry *ce)
{
    GEnumClass *eclass;
    char *enum_name;
    int i, j;
    int prefix_len = 0;

    phpg_return_if_fail(ce != NULL);
    phpg_return_if_fail(g_type_is_a(gtype, G_TYPE_ENUM));

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

/* {{{ phpg_register_flags() */
void phpg_register_flags(GType gtype, const char *strip_prefix, zend_class_entry *ce)
{
    GFlagsClass *eclass;
    char *enum_name;
    int i, j;
    int prefix_len = 0;

    phpg_return_if_fail(ce != NULL);
    phpg_return_if_fail(g_type_is_a(gtype, G_TYPE_FLAGS));

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

#endif

/* vim: set fdm=marker et sts=4: */
