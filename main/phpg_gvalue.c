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

/* {{{ php_gvalue_as_zval() */
PHP_GTK_API zval* php_gvalue_as_zval(const GValue *gval, zend_bool copy_boxed)
{
    zval *value = NULL;

    switch (G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(gval))) {
        case G_TYPE_NONE:
            MAKE_STD_ZVAL(value);
            ZVAL_NULL(value);
            break;

        case G_TYPE_CHAR:
            {
                gchar val = g_value_get_char(gval);
                MAKE_STD_ZVAL(value);
                ZVAL_STRINGL(value, (char *)&val, 1, 1);
            }

        case G_TYPE_UCHAR:
            {
                guchar val = g_value_get_uchar(gval);
                MAKE_STD_ZVAL(value);
                ZVAL_STRINGL(value, (char *)&val, 1, 1);
            }
            break;

        case G_TYPE_BOOLEAN:
            MAKE_STD_ZVAL(value);
            ZVAL_BOOL(value, g_value_get_boolean(gval));
            break;

        case G_TYPE_INT:
            MAKE_STD_ZVAL(value);
            ZVAL_LONG(value, (long)g_value_get_int(gval));
            break;

        case G_TYPE_LONG:
            MAKE_STD_ZVAL(value);
            ZVAL_LONG(value, (long)g_value_get_long(gval));
            break;

        case G_TYPE_UINT:
            MAKE_STD_ZVAL(value);
            ZVAL_LONG(value, (long)g_value_get_uint(gval));
            break;

        case G_TYPE_ULONG:
            MAKE_STD_ZVAL(value);
            ZVAL_LONG(value, (long)g_value_get_ulong(gval));
            break;

        case G_TYPE_FLOAT:
            MAKE_STD_ZVAL(value);
            ZVAL_DOUBLE(value, (double)g_value_get_float(gval));
            break;

        case G_TYPE_DOUBLE:
            MAKE_STD_ZVAL(value);
            ZVAL_DOUBLE(value, (double)g_value_get_double(gval));
            break;

        case G_TYPE_ENUM:
            MAKE_STD_ZVAL(value);
            ZVAL_LONG(value, (long)g_value_get_enum(gval));
            break;

        case G_TYPE_FLAGS:
            MAKE_STD_ZVAL(value);
            ZVAL_LONG(value, (long)g_value_get_flags(gval));
            break;

        case G_TYPE_STRING:
            {
                const gchar *str = g_value_get_string(gval);
                MAKE_STD_ZVAL(value);

                if (str != NULL) {
                    ZVAL_STRING(value, (char *)str, 1);
                } else
                    ZVAL_NULL(value);
            }
            break;


            /*
        case G_TYPE_INTERFACE:
            break;
            */

        default:
            php_error(E_WARNING, "PHP-GTK: internal error: unsupported type %s", g_type_name(G_VALUE_TYPE(gval)));
            break;
    }

    return value;
}
/* }}} */

/* {{{ php_gvalue_from_val() */
PHP_GTK_API int php_gvalue_from_zval(GValue *gval, zval *value)
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

        case G_TYPE_STRING:
            convert_to_string(value);
            g_value_set_string(gval, Z_STRVAL_P(value));
            break;

        default:
            php_error(E_WARNING, "PHP-GTK: internal error: unsupported type %s", g_type_name(G_VALUE_TYPE(gval)));
            return FAILURE;

    }

    return SUCCESS;
}
/* }}} */

/* {{{ php_gvalue_enum_get() */
PHP_GTK_API int php_gvalue_enum_get(GType enum_type, zval *enum_val, gint *result)
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
            php_error(E_WARNING, "PHP-GTK: internal error: could not obtain the type of enum");
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
            php_error(E_WARNING, "PHP-GTK: internal error: could not convert '%s' to enum", Z_STRVAL_P(enum_val));
            return FAILURE;
        }
    } else {
        php_error(E_WARNING, "PHP-GTK: internal error: enums must be strings or integers");
        return FAILURE;
    }

    return SUCCESS;
}
/* }}} */

gint php_gvalue_flags_get()
{
}

#endif

/* vim: set fdm=marker et : */
