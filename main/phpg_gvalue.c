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
 * GValue related functions
 */

/* TODO
 * G_TYPE_POINTER
 * G_TYPE_INTERFACE
 * G_TYPE_PARAM
 */


/* {{{ PHP_GTK_API phpg_gvalue_to_zval() */
PHP_GTK_API int phpg_gvalue_to_zval(const GValue *gval, zval **value, zend_bool copy_boxed TSRMLS_DC)
{
    assert(value != NULL);

    switch (G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(gval))) {
        case G_TYPE_NONE:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_NULL(*value);
            break;

        case G_TYPE_CHAR:
            {
                gchar val = g_value_get_char(gval);
                MAKE_ZVAL_IF_NULL(*value);
                ZVAL_STRINGL(*value, (char *)&val, 1, 1);
            }

        case G_TYPE_UCHAR:
            {
                guchar val = g_value_get_uchar(gval);
                MAKE_ZVAL_IF_NULL(*value);
                ZVAL_STRINGL(*value, (char *)&val, 1, 1);
            }
            break;

        case G_TYPE_BOOLEAN:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_BOOL(*value, g_value_get_boolean(gval));
            break;

        case G_TYPE_INT:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_LONG(*value, (long)g_value_get_int(gval));
            break;

        case G_TYPE_LONG:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_LONG(*value, (long)g_value_get_long(gval));
            break;

        case G_TYPE_UINT:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_LONG(*value, (long)g_value_get_uint(gval));
            break;

        case G_TYPE_ULONG:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_LONG(*value, (long)g_value_get_ulong(gval));
            break;

        case G_TYPE_FLOAT:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_DOUBLE(*value, (double)g_value_get_float(gval));
            break;

        case G_TYPE_DOUBLE:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_DOUBLE(*value, (double)g_value_get_double(gval));
            break;

        case G_TYPE_ENUM:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_LONG(*value, (long)g_value_get_enum(gval));
            break;

        case G_TYPE_FLAGS:
            MAKE_ZVAL_IF_NULL(*value);
            ZVAL_LONG(*value, (long)g_value_get_flags(gval));
            break;

        case G_TYPE_STRING:
            {
                const gchar *str = g_value_get_string(gval);

                MAKE_ZVAL_IF_NULL(*value);
                if (str != NULL) {
                    ZVAL_STRING(*value, (char *)str, 1);
                } else
                    ZVAL_NULL(*value);
            }
            break;

        case G_TYPE_OBJECT:
            phpg_gobject_new(value, g_value_get_object(gval) TSRMLS_CC);
            break;

        case G_TYPE_BOXED:
        {
            phpg_gboxed_marshal_t *gbm;

            if (G_VALUE_HOLDS(gval, G_TYPE_PHP_OBJECT)) {
                zval *object = (zval *) g_value_dup_boxed(gval);
                if (object) {
                    *value = object;
                    return SUCCESS;
                } else {
                    MAKE_ZVAL_IF_NULL(*value);
                    ZVAL_NULL(*value);
                    return FAILURE;
                }
            } else if (G_VALUE_HOLDS(gval, G_TYPE_VALUE_ARRAY)) {
                int i;
                zval *item;

                GValueArray *array = (GValueArray *) g_value_get_boxed(gval);
                MAKE_ZVAL_IF_NULL(*value);
                array_init(*value);
                for (i = 0; i < array->n_values; i++) {
                    MAKE_STD_ZVAL(item);
                    phpg_gvalue_to_zval(array->values + i, &item, copy_boxed TSRMLS_CC);
                    add_next_index_zval(*value, item);
                }
            } else if ((gbm = phpg_gboxed_lookup_custom(G_VALUE_TYPE(gval)))) {
                MAKE_STD_ZVAL(*value);
                return gbm->to_zval(gval, value TSRMLS_CC);
            } else {
                if (copy_boxed) {
                    phpg_gboxed_new(value, G_VALUE_TYPE(gval), g_value_get_boxed(gval), TRUE, TRUE TSRMLS_CC);
                } else {
                    phpg_gboxed_new(value, G_VALUE_TYPE(gval), g_value_get_boxed(gval), FALSE, FALSE TSRMLS_CC);
                }
            }
            break;
        }

        default:
            php_error(E_WARNING, "PHP-GTK internal error: unsupported type %s", g_type_name(G_VALUE_TYPE(gval)));
            MAKE_STD_ZVAL(*value);
            ZVAL_NULL(*value);
            return FAILURE;
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gvalue_from_zval() */
PHP_GTK_API int phpg_gvalue_from_zval(GValue *gval, zval *value TSRMLS_DC)
{
    switch (G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(gval))) {
        case G_TYPE_BOOLEAN:
            convert_to_boolean(value);
            g_value_set_boolean(gval, (gboolean)Z_BVAL_P(value));
            break;

        case G_TYPE_CHAR:
            convert_to_string(value);
            g_value_set_char(gval, Z_STRVAL_P(value)[0]);
            break;

        case G_TYPE_UCHAR:
            convert_to_string(value);
            g_value_set_uchar(gval, (guchar)Z_STRVAL_P(value)[0]);
            break;

        case G_TYPE_INT:
            convert_to_long(value);
            g_value_set_int(gval, (gint)Z_LVAL_P(value));
            break;

        case G_TYPE_UINT:
            convert_to_long(value);
            g_value_set_uint(gval, (guint)Z_LVAL_P(value));
            break;

        case G_TYPE_LONG:
            convert_to_long(value);
            g_value_set_long(gval, (glong)Z_LVAL_P(value));
            break;

        case G_TYPE_ULONG:
            convert_to_long(value);
            g_value_set_ulong(gval, (gulong)Z_LVAL_P(value));
            break;

        case G_TYPE_FLOAT:
            convert_to_double(value);
            g_value_set_float(gval, (gfloat)Z_DVAL_P(value));
            break;

        case G_TYPE_DOUBLE:
            convert_to_double(value);
            g_value_set_float(gval, Z_DVAL_P(value));
            break;

        case G_TYPE_ENUM:
        {
            gint val = 0;
            if (phpg_gvalue_get_enum(G_VALUE_TYPE(gval), value, &val) == FAILURE) {
                return FAILURE;
            }
            g_value_set_enum(gval, val);
            break;
        }

        case G_TYPE_FLAGS:
        {
            gint val = 0;
            if (phpg_gvalue_get_flags(G_VALUE_TYPE(gval), value, &val) == FAILURE) {
                return FAILURE;
            }
            g_value_set_flags(gval, val);
            break;
        }

        case G_TYPE_STRING:
            convert_to_string(value);
            g_value_set_string(gval, Z_STRVAL_P(value));
            break;

        case G_TYPE_BOXED:
        {
            phpg_gboxed_marshal_t *gbm;

            if (Z_TYPE_P(value) == IS_NULL) {
                g_value_set_boxed(gval, NULL);
            } else if (G_VALUE_HOLDS(gval, G_TYPE_PHP_OBJECT) && phpg_gtype_from_zval(value) == G_TYPE_PHP_OBJECT) {
                g_value_set_boxed(gval, value);
            } else if (Z_TYPE_P(value) == IS_OBJECT
                       && instanceof_function(Z_OBJCE_P(value), gboxed_ce TSRMLS_CC)
                       && G_VALUE_HOLDS(gval, ((phpg_gboxed_t*)PHPG_GET(value))->gtype)) {
                g_value_set_boxed(gval, PHPG_GBOXED(value));
            } else if ((gbm = phpg_gboxed_lookup_custom(G_VALUE_TYPE(gval)))) {
                return gbm->from_zval(value, gval TSRMLS_CC);
            } else
                return FAILURE;
            break;
        }

        case G_TYPE_OBJECT:
            if (Z_TYPE_P(value) == IS_NULL) {
                g_value_set_object(gval, NULL);
            } else if (Z_TYPE_P(value) == IS_OBJECT
                       && instanceof_function(Z_OBJCE_P(value), gobject_ce TSRMLS_CC)
                       && G_TYPE_CHECK_INSTANCE_TYPE(PHPG_GOBJECT(value), G_VALUE_TYPE(gval))) {
                g_value_set_object(gval, PHPG_GOBJECT(value));
            } else
                return FAILURE;
            break;

        default:
            php_error(E_WARNING, "PHP-GTK internal error: unsupported type %s", g_type_name(G_VALUE_TYPE(gval)));
            return FAILURE;

    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gvalues_to_array() */
PHP_GTK_API zval *phpg_gvalues_to_array(const GValue *values, uint n_values)
{
	zval *hash;
	zval *item;
	int i;

	MAKE_STD_ZVAL(hash);
	array_init(hash);
	for (i = 0; i < n_values; i++) {
		//item = phpg_gvalue_to_zval(&values[i], FALSE);
		if (!item) {
			MAKE_STD_ZVAL(item);
			ZVAL_NULL(item);
		}
		zend_hash_next_index_insert(Z_ARRVAL_P(hash), &item, sizeof(zval *), NULL);
	}

	return hash;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gvalue_get_enum() */
PHP_GTK_API int phpg_gvalue_get_enum(GType enum_type, zval *enum_val, gint *result)
{
    if (result == NULL)
        return FAILURE;

    if (enum_val == NULL) {
        *result = 0;
    } else if (Z_TYPE_P(enum_val) == IS_LONG) {
        *result = Z_LVAL_P(enum_val);
    } else if (Z_TYPE_P(enum_val) == IS_STRING) {
        GEnumClass *eclass = NULL;
        GEnumValue *info = NULL;

        if (enum_type != G_TYPE_NONE) {
            eclass = G_ENUM_CLASS(g_type_class_ref(enum_type));
        } else {
            php_error(E_WARNING, "PHP-GTK internal error: could not obtain the type of enum");
            return FAILURE;
        }

        info = g_enum_get_value_by_name(eclass, Z_STRVAL_P(enum_val));

        if (info == NULL) {
            info = g_enum_get_value_by_nick(eclass, Z_STRVAL_P(enum_val));
        }
        g_type_class_unref(eclass);
        
        if (info != NULL) {
            *result = info->value;
        } else {
            php_error(E_WARNING, "PHP-GTK internal error: could not convert '%s' to enum", Z_STRVAL_P(enum_val));
            return FAILURE;
        }
    } else {
        php_error(E_WARNING, "PHP-GTK internal error: enums must be strings or integers");
        return FAILURE;
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gvalue_get_flags() */
PHP_GTK_API int phpg_gvalue_get_flags(GType flags_type, zval *flags_val, gint *result)
{
    GFlagsClass *fclass = NULL;
    GFlagsValue *info = NULL;

    if (result == NULL)
        return FAILURE;

    if (flags_val == NULL) {
        *result = 0;
    } else if (Z_TYPE_P(flags_val) == IS_LONG) {
        *result = Z_LVAL_P(flags_val);
    } else if (Z_TYPE_P(flags_val) == IS_STRING) {
        if (flags_type != G_TYPE_NONE) {
            fclass = G_FLAGS_CLASS(g_type_class_ref(flags_type));
        } else {
            php_error(E_WARNING, "PHP-GTK internal error: could not obtain the type of flags");
            return FAILURE;
        }

        info = g_flags_get_value_by_name(fclass, Z_STRVAL_P(flags_val));
        if (info == NULL) {
            info = g_flags_get_value_by_nick(fclass, Z_STRVAL_P(flags_val));
        }
        g_type_class_unref(fclass);

        if (info != NULL) {
            *result = info->value;
        } else {
            php_error(E_WARNING, "PHP-GTK internal error: could not convert '%s' to flags", Z_STRVAL_P(flags_val));
            return FAILURE;
        }
    } else if (Z_TYPE_P(flags_val) == IS_ARRAY) {
		zval **flag;

        *result = 0;
        if (flags_type != G_TYPE_NONE) {
            fclass = G_FLAGS_CLASS(g_type_class_ref(flags_type));
        } else {
            php_error(E_WARNING, "PHP-GTK internal error: could not obtain the type of flags");
            return FAILURE;
        }

		for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(flags_val));
			 zend_hash_get_current_data(Z_ARRVAL_P(flags_val), (void **)&flag) == SUCCESS;
			 zend_hash_move_forward(Z_ARRVAL_P(flags_val))) {

			if (Z_TYPE_PP(flag) == IS_LONG) {
				*result |= Z_LVAL_PP(flag);
            } else if (Z_TYPE_PP(flag) == IS_STRING) {
                info = g_flags_get_value_by_name(fclass, Z_STRVAL_PP(flag));
                if (info == NULL) {
                    info = g_flags_get_value_by_nick(fclass, Z_STRVAL_PP(flag));
                }
                if (info != NULL) {
                    *result |= info->value;
                } else {
                    php_error(E_WARNING, "PHP-GTK internal error: could not convert '%s' to flags", Z_STRVAL_PP(flag));
                    g_type_class_unref(fclass);
                    return FAILURE;
                }
			} else {
				php_error(E_WARNING, "PHP-GTK flag arrays can contain only integers or strings");
                g_type_class_unref(fclass);
				return 0;
			}
		}
        g_type_class_unref(fclass);
    } else {
        php_error(E_WARNING, "PHP-GTK flags must be strings, integers, or arrays of strings or integers");
        return FAILURE;
    }

    return SUCCESS;
}
/* }}} */

#endif

/* vim: set fdm=marker et : */
