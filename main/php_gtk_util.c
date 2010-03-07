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

/* $Id$ */

#include "php_gtk.h"

#if HAVE_PHP_GTK

inline char *php_gtk_zval_type_name(zval *arg)
{
	TSRMLS_FETCH();
	switch (Z_TYPE_P(arg)) {
		case IS_NULL:
			return "null";

		case IS_LONG:
			return "integer";

		case IS_DOUBLE:
			return "double";

		case IS_STRING:
			return "string";

		case IS_ARRAY:
			return "array";

		case IS_OBJECT:
			return Z_OBJCE_P(arg)->name;

		case IS_BOOL:
			return "boolean";

		case IS_RESOURCE:
			return "resource";

		default:
			return "unknown";
	}
}

static char *parse_arg_impl(int arg_num, zval **arg, va_list *va, char **spec, char *buf, int as_zval TSRMLS_DC)
{
	char *spec_walk = *spec;
	char c = *spec_walk++;
	int return_null = 0;

	while (*spec_walk == '/' || *spec_walk == '!') {
		if (*spec_walk == '/') {
			SEPARATE_ZVAL_IF_NOT_REF(arg);
		} else if (*spec_walk == '!' && Z_TYPE_PP(arg) == IS_NULL) {
			return_null = 1;
		}
		spec_walk++;
	}

	switch (c) {

		case 'i':
			{
				switch (Z_TYPE_PP(arg)) {
					case IS_STRING:
						{
							double d;
							long l;
							int type;

							if ((type = is_numeric_string(Z_STRVAL_PP(arg), Z_STRLEN_PP(arg), &l, &d, 0)) == 0) {
								return "integer";
							}
							if (as_zval) goto ret_zval;
						}
						/* break omitted intenationally */

					case IS_BOOL:
					case IS_LONG:
					case IS_DOUBLE:
						convert_to_long_ex(arg);
						if (as_zval) goto ret_zval;
						*va_arg(*va, int *) = Z_LVAL_PP(arg);
						break;

					case IS_NULL:
					case IS_ARRAY:
					case IS_OBJECT:
					case IS_RESOURCE:
					default:
						return "integer";
				}
			}
			break;

		case 'l':
			{
				switch (Z_TYPE_PP(arg)) {
					case IS_STRING:
						{
							double d;
							long l;
							int type;

							if ((type = is_numeric_string(Z_STRVAL_PP(arg), Z_STRLEN_PP(arg), &l, &d, 0)) == 0) {
								return "integer";
							}
							if (as_zval) goto ret_zval;
						}
						/* break omitted intenationally */

					case IS_BOOL:
					case IS_LONG:
					case IS_DOUBLE:
						convert_to_long_ex(arg);
						if (as_zval) goto ret_zval;
						*va_arg(*va, long *) = Z_LVAL_PP(arg);
						break;

					case IS_NULL:
					case IS_ARRAY:
					case IS_OBJECT:
					case IS_RESOURCE:
					default:
						return "integer";
				}
			}
			break;

		case 'c':
			{
				if (Z_TYPE_PP(arg) != IS_STRING || Z_STRLEN_PP(arg) != 1)
					return "char";
				if (as_zval) goto ret_zval;
				*va_arg(*va, char *) = Z_STRVAL_PP(arg)[0];
			}
			break;

		case 's':
			{
				switch (Z_TYPE_PP(arg)) {
					case IS_NULL:
						if (return_null) {
							*va_arg(*va, char **) = NULL;
							if (*spec_walk == '#') {
								*va_arg(*va, int *) = 0;
								spec_walk++;
							}
						}
						/* drop through */
					case IS_STRING:
					case IS_LONG:
					case IS_DOUBLE:
					case IS_BOOL: {
						convert_to_string_ex(arg);
						if (as_zval) goto ret_zval;
						*va_arg(*va, char **) = Z_STRVAL_PP(arg);
						if (*spec_walk == '#') {
							*va_arg(*va, int *) = Z_STRLEN_PP(arg);
							spec_walk++;
						}
						break;
								  }

					case IS_ARRAY:
					case IS_OBJECT:
					case IS_RESOURCE:
					default:
						return "string";
				}
			}
			break;

		case 'u':
			{
				switch (Z_TYPE_PP(arg)) {
					case IS_NULL:
					case IS_STRING:
					case IS_LONG:
					case IS_DOUBLE:
					case IS_BOOL:
					{
						gchar *utf8 = NULL;
						gsize utf8_len = 0;
						zend_bool free_utf8 = 0;

						convert_to_string_ex(arg);
						if ((int)strlen(Z_STRVAL_PP(arg)) != Z_STRLEN_PP(arg))
							return "string without null bytes";

						if (as_zval) {
							utf8 = phpg_to_utf8(Z_STRVAL_PP(arg), Z_STRLEN_PP(arg), &utf8_len, &free_utf8 TSRMLS_CC);
							if (utf8) {
								if (free_utf8) {
									SEPARATE_ZVAL(arg);
									zval_dtor(*arg);
									ZVAL_STRINGL(*arg, utf8, utf8_len, 1);
									g_free(utf8);
								}
							} else {
								return "string in supported encoding";
							}

							goto ret_zval;
						}

						utf8 = phpg_to_utf8(Z_STRVAL_PP(arg), Z_STRLEN_PP(arg), &utf8_len, &free_utf8 TSRMLS_CC);
						if (utf8) {
							*va_arg(*va, char **) = utf8;
							if (*spec_walk == '#') {
								*va_arg(*va, int *) = utf8_len;
								spec_walk++;
							}
							*va_arg(*va, zend_bool *) = free_utf8;
						} else {
							return "string in supported encoding";
						}
						break;
					}

					case IS_ARRAY:
					case IS_OBJECT:
					case IS_RESOURCE:
					default:
						return "string";
				}
			}
			break;

		case 'd':
			{
				switch (Z_TYPE_PP(arg)) {
					case IS_STRING:
						{
							long l;
							double d;
							int type;

							if ((type = is_numeric_string(Z_STRVAL_PP(arg), Z_STRLEN_PP(arg), &l, &d, 0)) == 0) {
								return "double";
							}
							if (as_zval) goto ret_zval;
						}
						/* break omitted intenationally */

					case IS_LONG:
					case IS_DOUBLE:
					case IS_BOOL:
						convert_to_double_ex(arg);
						if (as_zval) goto ret_zval;
						*va_arg(*va, double *) = Z_DVAL_PP(arg);
						break;

					case IS_NULL:
					case IS_ARRAY:
					case IS_OBJECT:
					case IS_RESOURCE:
					default:
						return "double";
				}
			}
			break;

		case 'b':
			{
				switch (Z_TYPE_PP(arg)) {
					case IS_NULL:
					case IS_STRING:
					case IS_LONG:
					case IS_DOUBLE:
					case IS_BOOL:
						convert_to_boolean_ex(arg);
						if (as_zval) goto ret_zval;
						*va_arg(*va, zend_bool *) = Z_BVAL_PP(arg);
						break;

					case IS_ARRAY:
					case IS_OBJECT:
					case IS_RESOURCE:
					default:
						return "boolean";
				}
			}
			break;

		case 'r':
			{
				if (Z_TYPE_PP(arg) != IS_RESOURCE)
					return "resource";
				goto ret_zval;
			}
			break;

		case 'a':
			{
				if (Z_TYPE_PP(arg) != IS_ARRAY)
					return "array";
				goto ret_zval;
			}
			break;

		case 'O':
			{
				zval **p = va_arg(*va, zval **);
				zend_class_entry *ce = va_arg(*va, zend_class_entry *);
				if (Z_TYPE_PP(arg) == IS_OBJECT && (!ce || instanceof_function(Z_OBJCE_PP(arg), ce TSRMLS_CC))) {
					*p = *arg;
				} else {
					if (return_null) {
						*p = NULL;
					} else {
						return ce ? ce->name : "object";
					}
				}
			}
			break;

		case 'N':
			{
				zval **p = va_arg(*va, zval **);
				zend_class_entry *ce = va_arg(*va, zend_class_entry *);
				if (Z_TYPE_PP(arg) != IS_NULL && (Z_TYPE_PP(arg) != IS_OBJECT || !instanceof_function(Z_OBJCE_PP(arg), ce TSRMLS_CC))) {
					sprintf(buf, "%s or null", ce->name);
					return buf;
				} else {
					*p = *arg;
				}
			}
			break;

		case 'o':
			{
				if (Z_TYPE_PP(arg) != IS_OBJECT)
					return "object";
				goto ret_zval;
			}
			break;

		case 'C':
			{
				zend_class_entry **lookup, **pce = va_arg(*va, zend_class_entry **);
				zend_class_entry *ce_base = *pce;

				if (return_null && Z_TYPE_PP(arg) == IS_NULL) {
					*pce = NULL;
					break;
				}
				convert_to_string_ex(arg);
				if (zend_lookup_class(Z_STRVAL_PP(arg), Z_STRLEN_PP(arg), &lookup TSRMLS_CC) == FAILURE) {
					*pce = NULL;
				} else {
					*pce = *lookup;
				}
				if (ce_base) {
					if ((!*pce || !instanceof_function(*pce, ce_base TSRMLS_CC)) && !return_null) {
						char *space;
						char *class_name = get_active_class_name(&space TSRMLS_CC);
						zend_error(E_WARNING, "%s%s%s() expects parameter %d to be a class name derived from %s, '%s' given",
							   class_name, space, get_active_function_name(TSRMLS_C),
							   arg_num, ce_base->name, Z_STRVAL_PP(arg));
						*pce = NULL;
						return "";
					}
				}
				if (!*pce) {
					char *space;
					char *class_name = get_active_class_name(&space TSRMLS_CC);
					zend_error(E_WARNING, "%s%s%s() expects parameter %d to be a valid class name, '%s' given",
						   class_name, space, get_active_function_name(TSRMLS_C),
						   arg_num, Z_STRVAL_PP(arg));
					return "";
				}
				break;

			}
			break;
		case 'V':
ret_zval:
			{
				zval **p = va_arg(*va, zval **);
				*p = *arg;
			}
			break;

		default:
			return "<unknown>";
	}

	*spec = spec_walk;

	return NULL;
}

static int parse_arg(int arg_num, zval **arg, va_list *va, char **spec, int as_zval, int quiet TSRMLS_DC)
{
	char *expected_type;
	char buf[1024];
	char errorbuf[1024];

	expected_type = parse_arg_impl(arg_num, arg, va, spec, errorbuf, as_zval TSRMLS_CC);
	if (expected_type) {
		if (!quiet && *expected_type) {
			sprintf(buf, "%s::%s() expects argument %d to be %s, %s given",
					get_active_class_name(NULL TSRMLS_CC),
					get_active_function_name(TSRMLS_C), arg_num, expected_type,
					php_gtk_zval_type_name(*arg));
			php_error(E_WARNING, "%s", buf);
		}
		return 0;
	}

	return 1;
}

static int parse_va_args(int argc, zval ***args, char *format, va_list *va, int quiet TSRMLS_DC)
{
	char *format_walk;
	char buf[1024];
	int c, i;
	int min_argc = -1;
	int max_argc = 0;
	int as_zval = 0;

	/*
	 * First we check that the number of arguments matches the number specified
	 * in the format string.
	 */
	for (format_walk = format; *format_walk; format_walk++) {
		c = *format_walk;
		switch (c) {
			case '|':
				min_argc = max_argc;
				break;

			case 'i': case 'h': case 'l': case 'c':
			case 's': case 'd': case 'b': case 'u':
			case 'a': case 'N': case 'r': case 'C':
			case 'O': case 'o': case 'V':
				max_argc++;
				break;

			case '#':
			case '/':
			case '^':
			case '!':
				/* Pass */
				break;

			default:
				g_assert_not_reached();
				break;
		}
	}

	if (min_argc < 0)
		min_argc = max_argc;

	if (argc < min_argc || argc > max_argc) {
		if (!quiet) {
			sprintf(buf, "%s::%s() requires %s %d argument%s, %d given",
					get_active_class_name(NULL TSRMLS_CC),
					get_active_function_name(TSRMLS_C),
					min_argc == max_argc ? "exactly" : argc < min_argc ? "at least" : "at most",
					argc < min_argc ? min_argc : max_argc,
					(argc < min_argc ? min_argc : max_argc) == 1 ? "" : "s",
					argc);
			php_error(E_WARNING, "%s", buf);
		}
		return 0;
	}

	for (i = 0; i < argc; i++) {
		as_zval = 0;
		if (*format == '|')
			format++;
		if (*format == '^') {
			format++;
			as_zval = 1;
		}
		if (!parse_arg(i+1, args[i], va, &format, as_zval, quiet TSRMLS_CC))
			return 0;
	}

	return 1;
}

static int php_gtk_parse_args_impl(int argc, char *format, va_list *va, int quiet TSRMLS_DC)
{
	zval ***args;
	int retval;

	args = (zval ***)emalloc(argc * sizeof(zval **));

	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		php_error(E_WARNING, "Could not obtain arguments for parsing in %s::%s()",
				  get_active_class_name(NULL TSRMLS_CC),
				  get_active_function_name(TSRMLS_C));
		efree(args);
		return 0;
	}

	retval = parse_va_args(argc, args, format, va, quiet TSRMLS_CC);
	efree(args);

	return retval;
}

#define RETURN_IF_ZERO_ARGS(num_args, type_spec, quiet)  { \
    int __num_args = (num_args); \
    if (0 == (type_spec)[0] && 0 != __num_args && !(quiet)) { \
        char *__space; \
        char *__class_name = get_active_class_name(&__space TSRMLS_CC); \
        zend_error(E_WARNING, "%s%s%s() expects exactly 0 parameters, %d given", \
                   __class_name, __space, \
                   get_active_function_name(TSRMLS_C), __num_args); \
        return 0; \
    }\
}    

int php_gtk_parse_args(int argc, char *format, ...)
{
	va_list va;
	int retval;
	TSRMLS_FETCH();

	RETURN_IF_ZERO_ARGS(argc, format, 0);

	va_start(va, format);
	retval = php_gtk_parse_args_impl( argc, format, &va, 0 TSRMLS_CC);
	va_end(va);

	return retval;
}

PHP_GTK_API int php_gtk_parse_args_quiet(int argc, char *format, ...)
{
	va_list va;
	int retval;
	TSRMLS_FETCH();

	RETURN_IF_ZERO_ARGS(argc, format, 1);

	va_start(va, format);
	retval = php_gtk_parse_args_impl(argc, format, &va, 1 TSRMLS_CC);
	va_end(va);

	return retval;
}

PHP_GTK_API int php_gtk_parse_varargs(int argc, int min_args, zval **varargs, char *format, ...)
{
	va_list va;
	int retval;
	zval ***args;
	TSRMLS_FETCH();

	if (argc < min_args) {
		php_error(E_WARNING, "%s::%s() requires at least %d arguments, %d given",
				  get_active_class_name(NULL TSRMLS_CC),
				  get_active_function_name(TSRMLS_C), min_args, argc);
		return 0;
	}

	args = (zval ***)emalloc(argc * sizeof(zval **));
	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		php_error(E_WARNING, "Could not obtain arguments for parsing in %s::%s()",
				  get_active_class_name(NULL TSRMLS_CC),
				  get_active_function_name(TSRMLS_C));
		efree(args);
		return 0;
	}

	va_start(va, format);
	retval = php_gtk_parse_args_impl(min_args, format, &va, 0 TSRMLS_CC);
	va_end(va);
	if (varargs) {
		*varargs = php_gtk_array_as_hash(args, argc, min_args, argc-min_args);
	}
	efree(args);

	return retval;
}

static int php_gtk_parse_args_hash_impl(zval *hash, char *format, va_list *va, int quiet)
{
	zval ***args;
	int retval;
	TSRMLS_FETCH();

	args = php_gtk_hash_as_array(hash);
	retval = parse_va_args(zend_hash_num_elements(Z_ARRVAL_P(hash)), args, format, va, quiet TSRMLS_CC);
	efree(args);

	return retval;
}

static int php_gtk_parse_varargs_hash_impl(zval *hash, int min_args, zval **varargs, char *format, va_list *va, int quiet)
{
	zval ***args;
	int retval;
	int argc = zend_hash_num_elements(Z_ARRVAL_P(hash));
	TSRMLS_FETCH();

	if (argc < min_args) {
		php_error(E_WARNING, "%s::%s() requires at least %d arguments, %d given",
				  get_active_class_name(NULL TSRMLS_CC),
				  get_active_function_name(TSRMLS_C), min_args, argc);
		return 0;
	}

	args = php_gtk_hash_as_array(hash);
	retval = parse_va_args(min_args, args, format, va, quiet TSRMLS_CC);
	if (varargs) {
		*varargs = php_gtk_array_as_hash(args, argc, min_args, argc-min_args);
	}
	efree(args);

	return retval;
}

PHP_GTK_API int php_gtk_parse_args_hash(zval *hash, char *format, ...)
{
	va_list va;
	int retval;

	va_start(va, format);
	retval = php_gtk_parse_args_hash_impl(hash, format, &va, 0);
	va_end(va);

	return retval;
}

PHP_GTK_API int php_gtk_parse_args_hash_quiet(zval *hash, char *format, ...)
{
	va_list va;
	int retval;

	va_start(va, format);
	retval = php_gtk_parse_args_hash_impl(hash, format, &va, 1);
	va_end(va);

	return retval;
}

PHP_GTK_API int php_gtk_parse_varargs_hash(zval *hash, int min_args, zval **varargs, char *format, ...)
{
	va_list va;
	int retval;

	va_start(va, format);
	retval = php_gtk_parse_varargs_hash_impl(hash, min_args, varargs, format, &va, 0);
	va_end(va);

	return retval;
}

PHP_GTK_API int php_gtk_check_class(zval *wrapper, zend_class_entry *expected_ce)
{
	TSRMLS_FETCH();

	if (Z_TYPE_P(wrapper) != IS_OBJECT)
		return 0;

	if (instanceof_function(Z_OBJCE_P(wrapper), expected_ce TSRMLS_CC))
		return 1;
	else
		return 0;
}

PHP_GTK_API void php_gtk_invalidate(zval *wrapper)
{
	zval *ex;
	TSRMLS_FETCH();

	MAKE_STD_ZVAL(ex);
	/* object_init_ex(ex, php_gtk_exception_ce); */
	EG(exception) = ex;
	//if (!wrapper) return;
	//zend_objects_store_delete_obj(wrapper TSRMLS_CC);
}

zend_bool php_gtk_is_callable(zval *callable, zend_bool syntax_only, char **callable_name)
{
	char *lcname;
	int retval = 0;
	TSRMLS_FETCH();

	switch (Z_TYPE_P(callable)) {
		case IS_STRING:
			if (syntax_only)
				return 1;

			lcname = estrndup(Z_STRVAL_P(callable), Z_STRLEN_P(callable));
			zend_str_tolower(lcname, Z_STRLEN_P(callable));
			if (zend_hash_exists(EG(function_table), lcname, Z_STRLEN_P(callable)+1)) 
				retval = 1;
			efree(lcname);
			if (!retval && callable_name)
				*callable_name = estrndup(Z_STRVAL_P(callable), Z_STRLEN_P(callable));
			break;

		case IS_ARRAY:
			{
				zval **method;
				zval **obj;
				zend_class_entry *ce;
				char name_buf[1024];
				char callable_name_len;

				if (zend_hash_index_find(Z_ARRVAL_P(callable), 0, (void **) &obj) == SUCCESS &&
					zend_hash_index_find(Z_ARRVAL_P(callable), 1, (void **) &method) == SUCCESS &&
					(Z_TYPE_PP(obj) == IS_OBJECT || Z_TYPE_PP(obj) == IS_STRING) &&
					Z_TYPE_PP(method) == IS_STRING) {

					if (syntax_only)
						return 1;

					if (Z_TYPE_PP(obj) == IS_STRING) {
						int found;

						lcname = estrndup(Z_STRVAL_PP(obj), Z_STRLEN_PP(obj));
						zend_str_tolower(lcname, Z_STRLEN_PP(obj));
						found = zend_hash_find(EG(class_table), lcname, Z_STRLEN_PP(obj) + 1, (void**)&ce);
						efree(lcname);
						if (found == FAILURE) {
							if (callable_name) {
								callable_name_len = snprintf(name_buf, 1024, "%s::%s", Z_STRVAL_PP(obj), Z_STRVAL_PP(method));
								*callable_name = estrndup(name_buf, callable_name_len);
							}
							break;
						}
					} else
						ce = Z_OBJCE_PP(obj);
					lcname = estrndup(Z_STRVAL_PP(method), Z_STRLEN_PP(method));
					zend_str_tolower(lcname, Z_STRLEN_PP(method));
					if (zend_hash_exists(&ce->function_table, lcname, Z_STRLEN_PP(method)+1))
						retval = 1;
					if (!retval && callable_name) {
						callable_name_len = snprintf(name_buf, 1024, "%s::%s", ce->name, Z_STRVAL_PP(method));
						*callable_name = estrndup(name_buf, callable_name_len);
					}
					efree(lcname);
				} else if (callable_name)
					*callable_name = estrndup("Array", sizeof("Array")-1);
			}
			break;

		default:
			if (callable_name) {
				zval expr_copy;
				int use_copy;

				zend_make_printable_zval(callable, &expr_copy, &use_copy);
				*callable_name = estrndup(Z_STRVAL(expr_copy), Z_STRLEN(expr_copy));
				zval_dtor(&expr_copy);
			}
			break;
	}

	return retval;
}

zval ***php_gtk_func_args(int argc)
{
	zval ***args;
	TSRMLS_FETCH();

	args = (zval ***)emalloc(argc * sizeof(zval **));

	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		php_error(E_WARNING, "Could not obtain arguments in %s::%s()",
				  get_active_class_name(NULL TSRMLS_CC),
				  get_active_function_name(TSRMLS_C));
		efree(args);
		return NULL;
	}

	return args;
}

PHP_GTK_API zval *php_gtk_func_args_as_hash(int argc, int start, int length)
{
	zval ***args;
	zval *hash;
	TSRMLS_FETCH();

	args = (zval ***)emalloc(argc * sizeof(zval **));

	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		php_error(E_WARNING, "Could not obtain arguments in %s::%s()",
				  get_active_class_name(NULL TSRMLS_CC),
				  get_active_function_name(TSRMLS_C));
		efree(args);
		return NULL;
	}

	hash = php_gtk_array_as_hash(args, argc, start, length);

	efree(args);
	return hash;
}

PHP_GTK_API zval *php_gtk_array_as_hash(zval ***values, int num_values, int start, int length)
{
	zval *hash;
	int i;

	/* Clamp the start.. */
	if (start > num_values)
		start = num_values;
	else if (start < 0 && (start = num_values+start) < 0)
		start = 0;

	/* ..and the length */
	if (length < 0)
		length = num_values-start+length;
	else if (start+length > num_values)
		length = num_values-start;

	if (length == 0) {
		return NULL;
	}

	MAKE_STD_ZVAL(hash);
	array_init(hash);

	for (i = start; i < start+length; i++) {
		zval_add_ref(values[i]);
		zend_hash_next_index_insert(Z_ARRVAL_P(hash), values[i], sizeof(zval *), NULL);
	}

	return hash;
}

zval ***php_gtk_hash_as_array(zval *hash)
{
	int argc;
	int i = 0;
	zval ***values;

	argc = zend_hash_num_elements(Z_ARRVAL_P(hash));
	values = (zval ***)emalloc(argc * sizeof(zval **));
	for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(hash));
		 zend_hash_get_current_data(Z_ARRVAL_P(hash), (void **)&values[i++]) == SUCCESS;
		 zend_hash_move_forward(Z_ARRVAL_P(hash)));

	return values;
}

PHP_GTK_API zval*** php_gtk_hash_as_array_offset(zval *hash, int offset, int *total)
{
	int argc = 0;
	zval ***values;

	if (hash) {
		argc = zend_hash_num_elements(Z_ARRVAL_P(hash));
	}

	values = (zval ***)emalloc((argc + offset) * sizeof(zval **));
	*total = argc + offset;

	if (hash) {
		for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(hash));
			 zend_hash_get_current_data(Z_ARRVAL_P(hash), (void **)&values[offset++]) == SUCCESS;
			 zend_hash_move_forward(Z_ARRVAL_P(hash)));
	}

	return values;
}

static int php_gtk_count_specs(char *format, int endchar TSRMLS_DC)
{
	int count = 0;
	int level = 0;

	while (level > 0 || *format != endchar) {
		switch (*format) {
			case '\0':
				php_error(E_WARNING, "%s::%s(): internal error: unmatched parenthesis in format",
						  get_active_class_name(NULL TSRMLS_CC),
						  get_active_function_name(TSRMLS_C));
				return -1;

			case '(':
			case '{':
				if (level == 0)
					count++;
				level++;
				break;

			case ')':
			case '}':
				level--;
				break;

			case '#':
			case ':':
			case ',':
			case ' ':
			case '\t':
				break;

			default:
				if (level == 0)
					count++;
		}
		format++;
	}

	return count;
}

static zend_bool php_gtk_build_single(zval **result, char **format, va_list *va TSRMLS_DC);
static zend_bool php_gtk_build_hash(zval **result_p, char **format, va_list *va, int endchar, int count TSRMLS_DC);

static zend_bool php_gtk_build_single(zval **result, char **format, va_list *va TSRMLS_DC)
{
	assert(result != NULL);

	for (;;) {
		switch (*(*format)++) {
			case '(':
				return php_gtk_build_hash(result, format, va, ')', php_gtk_count_specs(*format, ')' TSRMLS_CC) TSRMLS_CC);

			case '{':
				return php_gtk_build_hash(result, format, va, '}', php_gtk_count_specs(*format, '}' TSRMLS_CC) TSRMLS_CC);

			case 'b':
				MAKE_ZVAL_IF_NULL(*result);
				ZVAL_BOOL(*result, (zend_bool)va_arg(*va, int));
				return 1;

			case 'h':
			case 'i':
				MAKE_ZVAL_IF_NULL(*result);
				ZVAL_LONG(*result, (long)va_arg(*va, int));
				return 1;

			case 'l':
				MAKE_ZVAL_IF_NULL(*result);
				ZVAL_LONG(*result, (long)va_arg(*va, long));
				return 1;

			case 'f':
			case 'd':
				MAKE_ZVAL_IF_NULL(*result);
				ZVAL_DOUBLE(*result, (double)va_arg(*va, double));
				return 1;

			case 's':
				{
					char *str = va_arg(*va, char *);
					int len;

					MAKE_ZVAL_IF_NULL(*result);
					if (str) {
						if (**format == '#') {
							++*format;
							len = va_arg(*va, int);
						} else
							len = strlen(str);
						ZVAL_STRINGL(*result, str, len, 1);
					} else
						ZVAL_NULL(*result);

					return 1;
				}

			case 'u':
				{
					char *str = va_arg(*va, char *);
					char *cp_str;
					int len;
					gsize cp_len;
					zend_bool free_result;

					MAKE_ZVAL_IF_NULL(*result);
					if (str) {
						if (**format == '#') {
							++*format;
							len = va_arg(*va, int);
						} else
							len = strlen(str);
						cp_str = phpg_from_utf8(str, len, &cp_len, &free_result TSRMLS_CC);
						if (cp_str) {
							ZVAL_STRINGL(*result, cp_str, cp_len, 1);
						} else {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not convert string from UTF-8");
							ZVAL_NULL(*result);
						}
						if (free_result) {
							g_free(cp_str);
						}
					} else {
						ZVAL_NULL(*result);
					}

					return 1;
				}

			case 'V':
			case 'N':
				*result = (zval *)va_arg(*va, zval *);
				assert(*result != NULL);
				if (*(*format - 1) != 'N')
					zval_add_ref(result);
				return 1;

			case 'n':
				MAKE_ZVAL_IF_NULL(*result);
				ZVAL_NULL(*result);
				return 1;

			case ':':
			case ',':
			case ' ':
			case '\t':
				break;

			default:
				php_error(E_WARNING, "%s::%s(): internal error: bad format spec while building value",
						  get_active_class_name(NULL TSRMLS_CC),
						  get_active_function_name(TSRMLS_C));
				return 0;
		}
	}
	
	return 0;
}

static zend_bool php_gtk_build_hash(zval **result_p, char **format, va_list *va, int endchar, int count TSRMLS_DC)
{
	zval *result;
	int i;

	assert(result_p != NULL);

	if (count < 0)
		return 0;

	MAKE_STD_ZVAL(result);
	array_init(result);

	if (endchar == ')') {
		for (i = 0; i < count; i++) {
			zval *single = NULL;
			if (!php_gtk_build_single(&single, format, va TSRMLS_CC)) {
				zval_ptr_dtor(&result);
				return 0;
			}
			zend_hash_next_index_insert(Z_ARRVAL_P(result), &single, sizeof(zval *), NULL);
		}
	} else if (endchar == '}') {
		for (i = 0; i < count; i += 2) {
			zval *key = NULL;
			zval *single = NULL;

			if (!php_gtk_build_single(&key, format, va TSRMLS_CC)) {
				zval_ptr_dtor(&result);
				return 0;
			}

			if (!php_gtk_build_single(&single, format, va TSRMLS_CC)) {
				zval_ptr_dtor(&key);
				zval_ptr_dtor(&result);
				return 0;
			}

			if (Z_TYPE_P(key) != IS_STRING && Z_TYPE_P(key) != IS_LONG)
				convert_to_string(key);

			if (Z_TYPE_P(key) == IS_LONG)
				add_index_zval(result, Z_LVAL_P(key), single);
			else
				add_assoc_zval_ex(result, Z_STRVAL_P(key), Z_STRLEN_P(key)+1, single);
			zval_ptr_dtor(&key);
		}
	}
	if (**format != endchar) {
		zval_ptr_dtor(&result);
		php_error(E_WARNING, "%s::%s(): internal error: unmatched parenthesis in format",
				  get_active_class_name(NULL TSRMLS_CC),
				  get_active_function_name(TSRMLS_C));
		return 0;
	} else if (endchar)
		++*format;

	if (*result_p) {
		REPLACE_ZVAL_VALUE(result_p, result, 0);
		FREE_ZVAL(result);
	} else {
		*result_p = result;
	}

	return 1;
}

PHP_GTK_API void php_gtk_build_value(zval **result, char *format, ...)
{
	int count;
	va_list va;
	TSRMLS_FETCH();

    assert(result != NULL);

	count = php_gtk_count_specs(format, '\0' TSRMLS_CC);
	if (count <= 0) {
		return;
	}

	MAKE_ZVAL_IF_NULL(*result);
	ZVAL_NULL(*result);

	va_start(va, format);

	if (count == 1)
		php_gtk_build_single(result, &format, &va TSRMLS_CC);
	else
		php_gtk_build_hash(result, &format, &va, '\0', count TSRMLS_CC);

	va_end(va);
}

PHP_GTK_API void phpg_warn_deprecated(char *msg TSRMLS_DC)
{
#ifndef E_DEPRECATED
	char *space;
	char *class_name = get_active_class_name(&space TSRMLS_CC);

	php_error(E_WARNING, "%s%s%s() is deprecated%s%s", class_name, space,
			  get_active_function_name(TSRMLS_C), msg?": ":"", msg?msg:"");
#else
    php_error(E_DEPRECATED, "%s%s", msg?": ":"", msg?msg:"");
#endif
}

#endif  /* HAVE_PHP_GTK */
