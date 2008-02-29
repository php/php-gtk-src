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
        memset(&result, 0, sizeof(zval));
        ZVAL_NULL(&result);
		ret = pi->read(poh, &result TSRMLS_CC);
		if (ret == SUCCESS) {
			ALLOC_ZVAL(result_ptr);
			*result_ptr = result;
            //INIT_PZVAL(result_ptr);
			Z_SET_REFCOUNT_P(result_ptr, 0);
			Z_UNSET_ISREF_P(result_ptr);
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

/* {{{ phpg_get_property_ptr_ptr() */
zval **phpg_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC)
{
    phpg_head_t *poh = NULL;
    zval tmp_member;
    zval **result = NULL;
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
        /*
         * We cannot return a zval** for the internal properties, because we are not
         * actually storing zval*'s. We could potentially return a proxy object here, but
         * it may be simpler just to fail and let the engine retry by calling
         * read_property handler. This should work since we don't really care about
         * assigning to internal properties by reference or doing some other weird stuff.
         */
        result = NULL;
    } else {
        result = zend_get_std_object_handlers()->get_property_ptr_ptr(object, member TSRMLS_CC);
    }

    if (member == &tmp_member) {
        zval_dtor(member);
    }

    return result;
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

        ret = pi->read(poh, &result TSRMLS_CC);
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
PHP_GTK_API zend_bool phpg_handle_gerror(GError **error TSRMLS_DC)
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


PHP_GTK_API void phpg_handle_marshaller_exception(TSRMLS_D)
{
    /*
     * Quit current main loop on exception. This will not help with GtkDialog::run()
     * though, since it has its own main loop.
     */
    if (EG(exception)) {
        gtk_main_quit();
    }
}


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
	poh->zobj.guards = NULL;
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
        g_type_set_qdata(gtype, phpg_class_key, real_ce);
    }

	return real_ce;
}
/* }}} */

/* {{{ phpg_register_interface() */
PHP_GTK_API zend_class_entry* phpg_register_interface(const char *iface_name,
                                                      function_entry *iface_methods,
                                                      GType gtype TSRMLS_DC)
{
	zend_class_entry ce, *real_ce;

	if (!phpg_class_key) {
		phpg_class_key = g_quark_from_static_string(phpg_class_id);
	}

    memset(&ce, 0, sizeof(ce));
    ce.name = strdup(iface_name);
    ce.name_length = strlen(iface_name);
    ce.builtin_functions = iface_methods;

    real_ce = zend_register_internal_interface(&ce TSRMLS_CC);

    if (gtype) {
        g_type_set_qdata(gtype, phpg_class_key, real_ce);
    }

    return real_ce;
}
/* }}} */

/* {{{ phpg_register_enum() */
PHP_GTK_API void phpg_register_enum(GType gtype, const char *strip_prefix, zend_class_entry *ce)
{
    GEnumClass *eclass;
    char *enum_name;
    int i, j;
    int prefix_len = 0;
    int value_count;

    phpg_return_if_fail(ce != NULL);
    phpg_return_if_fail(g_type_is_a(gtype, G_TYPE_ENUM));

    if (strip_prefix) {
        prefix_len = strlen(strip_prefix);
    }

    eclass = G_ENUM_CLASS(g_type_class_ref(gtype));
    value_count = eclass->n_values;

    for (i = 0; i < value_count; i++) {
        zval *val;
        
        val = (zval *)malloc(sizeof(zval));
        INIT_PZVAL(val);
        ZVAL_LONG(val, eclass->values[i].value);
        enum_name = (char *)eclass->values[i].value_name;
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
PHP_GTK_API void phpg_register_flags(GType gtype, const char *strip_prefix, zend_class_entry *ce)
{
    GFlagsClass *eclass;
    char *enum_name;
    int i, j;
    int prefix_len = 0;
	int value_count;

    phpg_return_if_fail(ce != NULL);
    phpg_return_if_fail(g_type_is_a(gtype, G_TYPE_FLAGS));

    if (strip_prefix) {
        prefix_len = strlen(strip_prefix);
    }

    eclass = G_FLAGS_CLASS(g_type_class_ref(gtype));
    value_count = eclass->n_values;

    for (i = 0; i < value_count; i++) {
        zval *val;
        
        val = (zval *)malloc(sizeof(zval));
        INIT_PZVAL(val);
        ZVAL_LONG(val, eclass->values[i].value);
        enum_name = (char *)eclass->values[i].value_name;
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

/* {{{ phpg_register_int_constant */
PHP_GTK_API void phpg_register_int_constant(zend_class_entry *ce, char *name, int name_len, long value)
{
    zval *zvalue;

    phpg_return_if_fail(ce != NULL);
    phpg_return_if_fail(name != NULL);

    zvalue = (zval *)malloc(sizeof(zval));
    INIT_PZVAL(zvalue);
    ZVAL_LONG(zvalue, value);
    zend_hash_update(&ce->constants_table, name, name_len+1, &zvalue, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ phpg_register_string_constant */
PHP_GTK_API void phpg_register_string_constant(zend_class_entry *ce, char *name, int name_len, char *value, int value_len)
{
    zval *zvalue;

    phpg_return_if_fail(ce != NULL);
    phpg_return_if_fail(name != NULL);

    zvalue = (zval *)malloc(sizeof(zval));
    INIT_PZVAL(zvalue);
    Z_TYPE_P(zvalue) = IS_STRING;
    Z_STRLEN_P(zvalue) = value_len;
    Z_STRVAL_P(zvalue) = zend_strndup(value, value_len);
    zend_hash_update(&ce->constants_table, name, name_len+1, &zvalue, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ phpg_create_class */

static int unset_abstract_flag(zend_function *func, int num_args, va_list args, zend_hash_key *hash_key)
{
    zend_class_entry *iface_ce = va_arg(args, zend_class_entry *);

    if (func->type == ZEND_INTERNAL_FUNCTION) {
        if (zend_hash_quick_exists(&iface_ce->function_table, hash_key->arKey, hash_key->nKeyLength, hash_key->h)) {
            ((zend_internal_function*)func)->fn_flags &= ~ZEND_ACC_ABSTRACT;
        }
    }

    return ZEND_HASH_APPLY_KEEP;
}

PHP_GTK_API zend_class_entry* phpg_create_class(GType gtype)
{
    zend_class_entry *parent_ce, *iface_ce, *ce;
    gchar* gtype_name;
    GType parent_type, *ifaces;
    guint i, n_ifaces = 0;
    TSRMLS_FETCH();

    parent_type = g_type_parent(gtype);
    parent_ce = phpg_class_from_gtype(parent_type);

    gtype_name = (gchar *) g_type_name(gtype);
    ce = phpg_register_class(gtype_name, NULL, parent_ce, 0, NULL, NULL, gtype TSRMLS_CC);

    ifaces = g_type_interfaces(gtype, &n_ifaces);
    if (n_ifaces) {
        for (i = 0; i < n_ifaces; i++) {
            iface_ce = phpg_class_from_gtype(ifaces[i]);
            zend_class_implements(ce TSRMLS_CC, 1, iface_ce);
            if (!G_TYPE_IS_INTERFACE(gtype)) {
                zend_hash_apply_with_arguments(&ce->function_table, (apply_func_args_t) unset_abstract_flag, 1, iface_ce TSRMLS_CC);
            }
        }
        if (!G_TYPE_IS_INTERFACE(gtype)) {
            ce->ce_flags &= ~ZEND_ACC_IMPLICIT_ABSTRACT_CLASS;
        }
        g_free(ifaces);
    }

    phpg_register_int_constant(ce, "gtype", sizeof("gtype")-1, gtype);

    /*
     * This is required to trick Zend into performing the global class table cleanup in a
     * different manner. EG(full_table_cleanup) is 0 normally, and is set to 1 if any
     * module is loaded via dl(), because the module may register internal classes. When
     * we register an internal class at runtime, it is added to the end of the class list.
     * shutdown_executor() has to remove user-declared classes from the class list. It
     * does it by iterating through the list in reverse order and cleaning up all user
     * classes until it encounters an internal class, but since we have just added an
     * internal class at the very end, it stops right away and does not clean up the user
     * ones. We circumvent it by pretending to be a dynamically loaded module so that the
     * hash cleanup does not stop on the first internal class and proceeds through the
     * whole table.
     *
     * Whether there is a better way to do it is still to be seen.
     */
    EG(full_tables_cleanup) = 1;

    return ce;
}
/* }}} */

/* {{{ phpg_handler_marshal */
gboolean phpg_handler_marshal(gpointer user_data)
{
    zval *callback_data = (zval *) user_data;
    zval **callback, **extra = NULL;
    zval **callback_filename = NULL, **callback_lineno = NULL;
    zval ***handler_args = NULL;
    int num_handler_args = 0;
    zval *retval = NULL;
    char *callback_name;
    gboolean result;
    TSRMLS_FETCH();

    /* Callback is always passed as the first element. */
    zend_hash_index_find(Z_ARRVAL_P(callback_data), 0, (void **)&callback);
    zend_hash_index_find(Z_ARRVAL_P(callback_data), 1, (void **)&extra);
    zend_hash_index_find(Z_ARRVAL_P(callback_data), 2, (void **)&callback_filename);
    zend_hash_index_find(Z_ARRVAL_P(callback_data), 3, (void **)&callback_lineno);

    if (!zend_is_callable(*callback, 0, &callback_name)) {
        php_error(E_WARNING, "Unable to invoke handler callback '%s' specified in %s on line %ld", callback_name, Z_STRVAL_PP(callback_filename), Z_LVAL_PP(callback_lineno));
        efree(callback_name);
        return 0;
    }

    handler_args = php_gtk_hash_as_array(*extra);
    num_handler_args = zend_hash_num_elements(Z_ARRVAL_PP(extra));

    call_user_function_ex(EG(function_table), NULL, *callback, &retval, num_handler_args, handler_args, 0, NULL TSRMLS_CC);

    result = FALSE;
    if (retval) {
        result = zval_is_true(retval);
        zval_ptr_dtor(&retval);
    }

    efree(callback_name);
    if (handler_args)
        efree(handler_args);

    phpg_handle_marshaller_exception(TSRMLS_C);

    return result;
}
/* }}} */

/* {{{ phpg_parse_ctor_props */
PHP_GTK_API zend_bool phpg_parse_ctor_props(GType gtype, zval **php_args, GParameter *params, guint *n_params, char **prop_names TSRMLS_DC)
{
    GObjectClass *klass;
    GParamSpec *spec;
    int i, n;

    klass = g_type_class_ref(gtype);
    phpg_return_val_if_fail_quiet(klass != NULL, FALSE);

    for (n = i = 0; php_args[i]; i++) {
        spec = g_object_class_find_property(klass, prop_names[i]);
        params[i].name = prop_names[i];
        g_value_init(&params[i].value, spec->value_type);

        if (phpg_gvalue_from_zval(&params[i].value, &php_args[i], FALSE TSRMLS_CC) == FAILURE) {
            php_error(E_WARNING, "Could not convert value for parameter '%s' of type '%s'",
                      prop_names[i], g_type_name(spec->value_type));
            g_type_class_unref(klass);
            for (; i >= 0; i--) {
                g_value_unset(&params[i].value);
            }
            return FALSE;
        }

        n++;
    }

    g_type_class_unref(klass);
    *n_params = n;

    return TRUE;
}
/* }}} */

#endif

/* vim: set fdm=marker et sts=4: */
