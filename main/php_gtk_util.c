/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Andrei Zmievski <andrei@php.net>                            |
   +----------------------------------------------------------------------+
 */

#include "php_gtk.h"

#if HAVE_PHP_GTK

static char *parse_arg_impl(zval **arg, va_list *va, int spec, char *buf)
{
	switch (spec) {
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

		case 'a':
			{
				zval **p = va_arg(*va, zval **);
				if (Z_TYPE_PP(arg) != IS_ARRAY)
					return "array";
				else
					*p = *arg;
			}
			break;

		case 'O':
			{
				zval **p = va_arg(*va, zval **);
				zend_class_entry *ce = va_arg(*va, zend_class_entry *);
				if (Z_TYPE_PP(arg) != IS_OBJECT || !php_gtk_check_class(*arg, ce))
					return ce->name;
				else
					*p = *arg;
			}
			break;

		case 'N':
			{
				zval **p = va_arg(*va, zval **);
				zend_class_entry *ce = va_arg(*va, zend_class_entry *);
				if (Z_TYPE_PP(arg) != IS_NULL && (Z_TYPE_PP(arg) != IS_OBJECT || !php_gtk_check_class(*arg, ce))) {
					sprintf(buf, "%s or null", ce->name);
					return buf;
				} else
					*p = *arg;
			}
			break;

		case 'o':
			{
				zval **p = va_arg(*va, zval **);
				if (Z_TYPE_PP(arg) != IS_OBJECT)
					return "object";
				else
					*p = *arg;
			}
			break;

		case 'V':
			{
				zval **p = va_arg(*va, zval **);
				*p = *arg;
			}
			break;
	}

	return NULL;
}

static int parse_arg(int arg_num, zval **arg, va_list *va, int spec)
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

		sprintf(buf, "%s() expects argument %d to be %s, %s given",
				get_active_function_name(), arg_num, expected_type, actual_type);
		php_error(E_WARNING, buf);
		return 0;
	}

	return 1;
}

static int parse_va_args(int argc, zval ***args, char *format, va_list *va)
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

			case 'i': case 'c':
			case 's': case 'd': case 'b':
			case 'a': case 'N':
			case 'O': case 'o': case 'V':
				max_argc++;
				break;

			default:
				g_assert_not_reached();
				break;
		}
	}

	if (min_argc < 0)
		min_argc = max_argc;

	if (argc < min_argc || argc > max_argc) {
		sprintf(buf, "%s() requires %s %d argument%s, %d given",
				get_active_function_name(),
				min_argc == max_argc ? "exactly" : argc < min_argc ? "at least" : "at most",
				argc < min_argc ? min_argc : max_argc,
				(argc < min_argc ? min_argc : max_argc) == 1 ? "" : "s",
				argc);
		php_error(E_WARNING, buf);
		return 0;
	}

	for (i = 0; i < argc; i++, format++) {
		if (*format == '|')
			format++;
		if (!parse_arg(i+1, args[i], va, *format))
			return 0;
	}

	return 1;
}

int php_gtk_parse_args(int argc, char *format, ...)
{
	zval ***args;
	va_list va;
	int retval;

	args = (zval ***)emalloc(argc * sizeof(zval **));

	if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
		php_error(E_WARNING, "Could not obtain arguments for parsing in %s",
				  get_active_function_name());
		efree(args);
		return 0;
	}

	va_start(va, format);
	retval = parse_va_args(argc, args, format, &va);
	va_end(va);
	efree(args);

	return retval;
}

int php_gtk_check_class(zval *wrapper, zend_class_entry *expected_ce)
{
	zend_class_entry *ce;

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

zend_bool php_gtk_check_callable(zval *function)
{
	char *lcname;
	int retval = 0;
	ELS_FETCH();

	switch (Z_TYPE_P(function)) {
		case IS_STRING:
			lcname = estrndup(Z_STRVAL_P(function), Z_STRLEN_P(function));
			zend_str_tolower(lcname, Z_STRLEN_P(function));
			if (zend_hash_exists(EG(function_table), lcname, Z_STRLEN_P(function)+1)) 
				retval = 1;
			efree(lcname);
			break;

		case IS_ARRAY:
			{
				zval **method;
				zval **obj;
				
				if (zend_hash_index_find(Z_ARRVAL_P(function), 0, (void **) &obj) == SUCCESS &&
					zend_hash_index_find(Z_ARRVAL_P(function), 1, (void **) &method) == SUCCESS &&
					Z_TYPE_PP(obj) == IS_OBJECT &&
					Z_TYPE_PP(method) == IS_STRING) {
					lcname = estrndup(Z_STRVAL_PP(method), Z_STRLEN_PP(method));
					zend_str_tolower(lcname, Z_STRLEN_PP(method));
					if (zend_hash_exists(&Z_OBJCE_PP(obj)->function_table, lcname, Z_STRLEN_PP(method)+1))
						retval = 1;
					efree(lcname);
				}
			}
			break;

		default:
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
				if (level == 0)
					count++;
				level++;
				break;

			case ')':
				level--;
				break;

			case '#':
			case ' ':
			case '\t':
				break;

			default:
				if (level == 0)
					count++;
				break;
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

			case 'b':
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

	for (i = 0; i < count; i++) {
		single = php_gtk_build_single(format, va);
		if (!single) {
			zval_dtor(result);
			return NULL;
		}
		zend_hash_next_index_insert(Z_ARRVAL_P(result), &single, sizeof(zval *), NULL);
	}
	if (**format != endchar) {
		zval_dtor(result);
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

	if (count < 0)
		return NULL;
	else if (count == 0) {
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
	return result;
}

#endif  /* HAVE_PHP_GTK */
