/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001 Andrei Zmievski <andrei@php.net>
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

static char *parse_arg_impl(zval **arg, va_list *va, char **spec, char *buf)
{
	char *spec_walk = *spec;
	char c = *spec_walk++;

	switch (c) {
		case 'h':
			{
				short *p = va_arg(*va, short *);
				if (Z_TYPE_PP(arg) != IS_LONG && Z_TYPE_PP(arg) != IS_BOOL)
					return "integer";
				else
					*p = (short)Z_LVAL_PP(arg);
			}
			break;

		case 'i':
			{
				int *p = va_arg(*va, int *);
				if (Z_TYPE_PP(arg) != IS_LONG && Z_TYPE_PP(arg) != IS_BOOL)
					return "integer";
				else
					*p = Z_LVAL_PP(arg);
			}
			break;

		case 'c':
			{
				char *p = va_arg(*va, char *);
				if (Z_TYPE_PP(arg) != IS_STRING || Z_STRLEN_PP(arg) != 1)
					return "char";
				else
					*p = Z_STRVAL_PP(arg)[0];
			}
			break;

		case 's':
			{
				char **p = va_arg(*va, char **);
				if (Z_TYPE_PP(arg) != IS_STRING)
					return "string";
				else
					*p = Z_STRVAL_PP(arg);
				if ((int)strlen(*p) != Z_STRLEN_PP(arg))
					return "string without null bytes";
				if (*spec_walk == '#') {
					int *p = va_arg(*va, int *);
					*p = Z_STRLEN_PP(arg);
					spec_walk++;
				}
			}
			break;

		case 'd':
			{
				double *p = va_arg(*va, double *);
				if (Z_TYPE_PP(arg) != IS_DOUBLE)
					return "double";
				else
					*p = Z_DVAL_PP(arg);
			}
			break;

		case 'b':
			{
				zend_bool *p = va_arg(*va, zend_bool *);
				if (Z_TYPE_PP(arg) != IS_BOOL)
					return "boolean";
				else
					*p = Z_BVAL_PP(arg);
			}
			break;

		case 'r':
			{
				zval **r = va_arg(*va, zval **);
				if (Z_TYPE_PP(arg) != IS_RESOURCE)
					return "resource";
				else
					*r = *arg;
			}
			break;

		case 'a':
			{
				zval **p = va_arg(*va, zval **);
				if (Z_TYPE_PP(arg) != IS_ARRAY)
					return "array";
				else {
					if (*spec_walk == '/') {
						SEPARATE_ZVAL(arg);
						spec_walk++;
					}
					*p = *arg;
				}
			}
			break;

		case 'O':
			{
				zval **p = va_arg(*va, zval **);
				zend_class_entry *ce = va_arg(*va, zend_class_entry *);
				if (Z_TYPE_PP(arg) != IS_OBJECT || !php_gtk_check_class(*arg, ce))
					return ce->name;
				else {
					if (*spec_walk == '/') {
						SEPARATE_ZVAL(arg);
						spec_walk++;
					}
					*p = *arg;
				}
			}
			break;

		case 'N':
			{
				zval **p = va_arg(*va, zval **);
				zend_class_entry *ce = va_arg(*va, zend_class_entry *);
				if (Z_TYPE_PP(arg) != IS_NULL && (Z_TYPE_PP(arg) != IS_OBJECT || !php_gtk_check_class(*arg, ce))) {
					sprintf(buf, "%s or null", ce->name);
					return buf;
				} else {
					if (*spec_walk == '/') {
						SEPARATE_ZVAL(arg);
						spec_walk++;
					}
					*p = *arg;
				}
			}
			break;

		case 'o':
			{
				zval **p = va_arg(*va, zval **);
				if (Z_TYPE_PP(arg) != IS_OBJECT)
					return "object";
				else {
					if (*spec_walk == '/') {
						SEPARATE_ZVAL(arg);
						spec_walk++;
					}
					*p = *arg;
				}
			}
			break;

		case 'V':
			{
				zval **p = va_arg(*va, zval **);
				if (*spec_walk == '/') {
					SEPARATE_ZVAL(arg);
					spec_walk++;
				}
				*p = *arg;
			}
			break;

		default:
			return "<unknown>";
	}

	*spec = spec_walk;

	return NULL;
}

static int parse_arg(int arg_num, zval **arg, va_list *va, char **spec, int quiet)
{
	char *expected_type;
	char *actual_type;
	char buf[1024];
	char errorbuf[1024];
	char objtype[1024];

	expected_type = parse_arg_impl(arg, va, spec, errorbuf);
	if (expected_type) {
		switch (Z_TYPE_PP(arg)) {
			case IS_NULL:
				actual_type = "NULL";
				break;

			case IS_LONG:
				actual_type = "integer";
				break;

			case IS_DOUBLE:
				actual_type = "double";
				break;

			case IS_STRING:
				actual_type = "string";
				break;

			case IS_ARRAY:
				actual_type = "array";
				break;

			case IS_OBJECT:
				sprintf(objtype, "object <%s>", Z_OBJCE_PP(arg)->name);
				actual_type = objtype;
				break;

			case IS_BOOL:
				actual_type = "boolean";
				break;

			case IS_RESOURCE:
				actual_type = "resource";
				break;

			default:
				actual_type = "unknown";
				break;

		}

		if (!quiet) {
			sprintf(buf, "%s() expects argument %d to be %s, %s given",
					get_active_function_name(), arg_num, expected_type, actual_type);
			php_error(E_WARNING, buf);
		}
		return 0;
	}

	return 1;
}

static int parse_va_args(int argc, zval ***args, char *format, va_list *va, int quiet)
{
	char *format_walk;
	char buf[1024];
	int c, i;
	int min_argc = -1;
	int max_argc = 0;

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

			case 'i': case 'h': case 'c':
			case 's': case 'd': case 'b':
			case 'a': case 'N': case 'r':
			case 'O': case 'o': case 'V':
				max_argc++;
				break;

			case '#':
			case '/':
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
			sprintf(buf, "%s() requires %s %d argument%s, %d given",
					get_active_function_name(),
					min_argc == max_argc ? "exactly" : argc < min_argc ? "at least" : "at most",
					argc < min_argc ? min_argc : max_argc,
					(argc < min_argc ? min_argc : max_argc) == 1 ? "" : "s",
					argc);
			php_error(E_WARNING, buf);
		}
		return 0;
	}

	for (i = 0; i < argc; i++) {
		if (*format == '|')
			format++;
		if (!parse_arg(i+1, args[i], va, &format, quiet))
			return 0;
	}

	return 1;
}

static int php_gtk_parse_args_impl(int argc, char *format, va_list *va, int quiet)
{
	zval ***args;
	int retval;

	args = (zval ***)emalloc(argc * sizeof(zval **));

	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		php_error(E_WARNING, "Could not obtain arguments for parsing in %s",
				  get_active_function_name());
		efree(args);
		return 0;
	}

	retval = parse_va_args(argc, args, format, va, quiet);
	efree(args);

	return retval;
}

int php_gtk_parse_args(int argc, char *format, ...)
{
	va_list va;
	int retval;

	va_start(va, format);
	retval = php_gtk_parse_args_impl( argc, format, &va, 0);
	va_end(va);

	return retval;
}

int php_gtk_parse_args_quiet(int argc, char *format, ...)
{
	va_list va;
	int retval;

	va_start(va, format);
	retval = php_gtk_parse_args_impl(argc, format, &va, 1);
	va_end(va);

	return retval;
}

static int php_gtk_parse_args_hash_impl(zval *hash, char *format, va_list *va, int quiet)
{
	zval ***args;
	int retval;

	args = php_gtk_hash_as_array(hash);
	retval = parse_va_args(zend_hash_num_elements(Z_ARRVAL_P(hash)), args, format, va, quiet);
	efree(args);

	return retval;
}

int php_gtk_parse_args_hash(zval *hash, char *format, ...)
{
	va_list va;
	int retval;

	va_start(va, format);
	retval = php_gtk_parse_args_hash_impl(hash, format, &va, 0);
	va_end(va);

	return retval;
}

int php_gtk_parse_args_hash_quiet(zval *hash, char *format, ...)
{
	va_list va;
	int retval;

	va_start(va, format);
	retval = php_gtk_parse_args_hash_impl(hash, format, &va, 1);
	va_end(va);

	return retval;
}

int php_gtk_check_class(zval *wrapper, zend_class_entry *expected_ce)
{
	zend_class_entry *ce;

	if (Z_TYPE_P(wrapper) != IS_OBJECT)
		return 0;

	for (ce = Z_OBJCE_P(wrapper); ce != NULL; ce = ce->parent) {
		if (ce == expected_ce)
			return 1;
	}

	return 0;
}

void php_gtk_invalidate(zval *wrapper)
{
	if (!wrapper) return;
	zval_dtor(wrapper);
	ZVAL_NULL(wrapper);
}

zend_bool php_gtk_is_callable(zval *callable, zend_bool syntax_only, char **callable_name)
{
	char *lcname;
	int retval = 0;
	ELS_FETCH();

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

	args = (zval ***)emalloc(argc * sizeof(zval **));

	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		php_error(E_WARNING, "Could not obtain arguments in %s",
				  get_active_function_name());
		efree(args);
		return NULL;
	}

	return args;
}

zval *php_gtk_func_args_as_hash(int argc, int start, int length)
{
	zval ***args;
	zval *hash;

	args = (zval ***)emalloc(argc * sizeof(zval **));

	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		php_error(E_WARNING, "Could not obtain arguments in %s",
				  get_active_function_name());
		efree(args);
		return NULL;
	}

	hash = php_gtk_array_as_hash(args, argc, start, length);

	efree(args);
	return hash;
}

zval *php_gtk_array_as_hash(zval ***values, int num_values, int start, int length)
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

static int php_gtk_count_specs(char *format, int endchar)
{
	int count = 0;
	int level = 0;
	while (level > 0 || *format != endchar) {
		switch (*format) {
			case '\0':
				php_error(E_WARNING, "%s(): internal error: unmatched parenthesis in format", get_active_function_name());
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

static zval *php_gtk_build_single(char **format, va_list *va);
static zval *php_gtk_build_hash(char **format, va_list *va, int endchar, int count);

static zval *php_gtk_build_single(char **format, va_list *va)
{
	zval *result;

	for (;;) {
		switch (*(*format)++) {
			case '(':
				return php_gtk_build_hash(format, va, ')', php_gtk_count_specs(*format, ')'));

			case '{':
				return php_gtk_build_hash(format, va, '}', php_gtk_count_specs(*format, '}'));

			case 'b':
				MAKE_STD_ZVAL(result);
				ZVAL_BOOL(result, (zend_bool)va_arg(*va, int));
				return result;

			case 'h':
			case 'i':
				MAKE_STD_ZVAL(result);
				ZVAL_LONG(result, (long)va_arg(*va, int));
				return result;

			case 'l':
				MAKE_STD_ZVAL(result);
				ZVAL_LONG(result, (long)va_arg(*va, long));
				break;

			case 'f':
			case 'd':
				MAKE_STD_ZVAL(result);
				ZVAL_DOUBLE(result, (double)va_arg(*va, double));
				return result;

			case 's':
				{
					char *str = va_arg(*va, char *);
					int len;

					MAKE_STD_ZVAL(result);
					if (str) {
						if (**format == '#') {
							++*format;
							len = va_arg(*va, int);
						} else
							len = strlen(str);
						ZVAL_STRINGL(result, str, len, 1);
					} else
						ZVAL_NULL(result);

					return result;
				}

			case 'V':
			case 'N':
				result = (zval *)va_arg(*va, zval *);
				if (*(*format - 1) != 'N')
					zval_add_ref(&result);
				return result;

			case ':':
			case ',':
			case ' ':
			case '\t':
				break;

			default:
				php_error(E_WARNING, "%s(): internal error: bad format spec while building value", get_active_function_name());
				return NULL;
		}
	}
}

static zval *php_gtk_build_hash(char **format, va_list *va, int endchar, int count)
{
	zval *result;
	zval *single;
	int i;

	if (count < 0)
		return NULL;

	MAKE_STD_ZVAL(result);
	array_init(result);

	if (endchar == ')') {
		for (i = 0; i < count; i++) {
			single = php_gtk_build_single(format, va);
			if (!single) {
				zval_ptr_dtor(&result);
				return NULL;
			}
			zend_hash_next_index_insert(Z_ARRVAL_P(result), &single, sizeof(zval *), NULL);
		}
	} else if (endchar == '}') {
		zval *key;

		for (i = 0; i < count; i += 2) {
			key = php_gtk_build_single(format, va);
			if (!key) {
				zval_ptr_dtor(&result);
				return NULL;
			}

			single = php_gtk_build_single(format, va);
			if (!single) {
				zval_ptr_dtor(&key);
				zval_ptr_dtor(&result);
				return NULL;
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
		php_error(E_WARNING, "%s(): internal error: unmatched parenthesis in format", get_active_function_name());
		return NULL;
	} else if (endchar)
		++*format;

	return result;
}

zval *php_gtk_build_value(char *format, ...)
{
	int count = php_gtk_count_specs(format, '\0');
	zval *result;
	va_list va;

	if (count <= 0) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	va_start(va, format);

	if (count == 1)
		result = php_gtk_build_single(&format, &va);
	else
		result = php_gtk_build_hash(&format, &va, '\0', count);

	va_end(va);

	if (result == NULL) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
	}

	return result;
}

#endif  /* HAVE_PHP_GTK */
