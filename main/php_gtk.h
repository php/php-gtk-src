/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001,2002 Andrei Zmievski <andrei@php.net>
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
 
/* $Id$: */

#ifndef _PHP_GTK_H
#define _PHP_GTK_H

#include "php.h"
#include "php_ini.h"
#ifdef PHP_WIN32
#include "config.w32.h"
#else
#include "config.h"
#endif

#if HAVE_PHP_GTK

#include "zend_objects_API.h"

#define PHP_GTK_VERSION "0.5.1"

#ifdef PHP_WIN32
# ifdef GTK_SHARED
#  define PHP_GTK_API __declspec(dllimport)
# else
#  define PHP_GTK_API __declspec(dllexport)
# endif
#else
# define PHP_GTK_API
#endif

#include "php_gtk_module.h"
#include <glib-object.h>

#define PHP_GTK_EXPORT_CE(ce) zend_class_entry *ce
#define PHP_GTK_EXPORT_FUNC(func) func


#define PHP_GTK_GET_GENERIC(w, type, le) ((type)php_gtk_get_object(w, le))


#define PHP_GTK_SEPARATE_RETURN(return_value, result)			\
	{															\
		zval *ret;												\
																\
		ret = result;											\
		SEPARATE_ZVAL(&ret);									\
		*return_value = *ret;									\
	}

#undef PG_ERROR
#define PG_ERROR -2

typedef void (*prop_getter_t)(zval *return_value, zval *object, char *property, int *result);
typedef int (*prop_setter_t)(zval *object, char *property, zval *value);

#define EXT_INIT_ARGS			int module_number TSRMLS_DC
#define EXT_SHUTDOWN_ARGS		void

#define PHP_GTK_XINIT(ext)		php_gtk_xinit_##ext
#define PHP_GTK_XSHUTDOWN(ext)	php_gtk_xshutdown_##ext

#define PHP_GTK_XINIT_FUNCTION(ext)		int PHP_GTK_XINIT(ext)(EXT_INIT_ARGS)
#define PHP_GTK_XSHUTDOWN_FUNCTION(ext)	int PHP_GTK_XSHUTDOWN(ext)(EXT_SHUTDOWN_ARGS)

typedef struct _php_gtk_ext_entry php_gtk_ext_entry;
struct _php_gtk_ext_entry {
	char *name;
	int (*ext_startup_func)(EXT_INIT_ARGS);
	int (*ext_shutdown_func)(EXT_SHUTDOWN_ARGS);
	int ext_started;
	void *handle;
};
 

typedef struct _php_gtk_closure php_gtk_closure;
struct _php_gtk_closure {
    GClosure	closure;
    zval 	*callback;
    char	*callback_filename;
    uint 	callback_lineno;
    zval 	*extra;
    int		pass_object;
};



#define PHP_GTK_GET_EXTENSION(name) \
    ZEND_DLEXPORT php_gtk_ext_entry *get_extension(void) { return &name##_ext_entry; }

#include "ext/gtk+/php_gtk+.h"

/* True globals. */
extern zend_llist php_gtk_ext_registry;
PHP_GTK_API extern GHashTable *php_gtk_class_hash;
extern HashTable php_gtk_rsrc_hash;
extern HashTable php_gtk_prop_getters;
extern HashTable php_gtk_prop_setters;
extern HashTable php_gtk_type_hash;
extern HashTable php_gtk_prop_desc;
extern HashTable php_gtk_callback_hash;


/* Function declarations. */

int php_gtk_startup_all_extensions(int module_number);
int php_gtk_startup_extensions(php_gtk_ext_entry **ext, int ext_count, int module_number);

static inline void php_gtk_set_object(zval *zobj, void *obj, php_gtk_dtor_t dtor, zend_bool boxed)
{
	php_gtk_object *wrapper = (php_gtk_object *) zend_object_store_get_object(zobj TSRMLS_CC);
	wrapper->obj = obj;
	wrapper->dtor = dtor;
	zend_objects_store_add_ref(zobj TSRMLS_CC);
	if (boxed) {
		zend_hash_index_update(&php_gtk_type_hash, (long)obj, (void *)&zobj, sizeof(zval *), NULL);
	}
}

PHP_GTK_API void *php_gtk_get_object(zval *wrapper, int rsrc_type);
int php_gtk_get_simple_enum_value(zval *enum_val, int *result);
PHP_GTK_API int php_gtk_get_enum_value(GtkType enum_type, zval *enum_val, int *result);
PHP_GTK_API void php_gtk_destroy_notify(gpointer user_data);
PHP_GTK_API void php_gtk_closure_marshal(
					GClosure 	*gobject_closure,
					GValue 		*return_value,
					guint 		n_param_values,
					const GValue 	*param_values,
					// this is the standard signal parameters..
					// the first of which is usually the object..
					gpointer 	invocation_hint,
					gpointer 	marshal_data
					);
void php_gtk_handler_marshal(gpointer a, gpointer data, int nargs, GtkArg *args);
zval *php_gtk_args_as_hash(int nargs, GtkArg *args);
GtkArg *php_gtk_hash_as_args(zval *hash, GtkType type, gint *nargs);
int php_gtk_args_from_hash(GtkArg *args, int nparams, zval *hash);
zval *php_gtk_arg_as_value(GtkArg *arg);
int php_gtk_arg_from_value(GtkArg *arg, zval *value);
zval *php_gtk_ret_as_value(GtkArg *ret);
void php_gtk_ret_from_value(GtkArg *ret, zval *value);
int php_gtk_get_flag_value(GtkType flag_type, zval *flag_val, int *result);
PHP_GTK_API zval php_gtk_get_property(zend_property_reference *property_reference);
PHP_GTK_API int php_gtk_set_property(zend_property_reference *property_reference, zval *value);
void php_gtk_call_function(INTERNAL_FUNCTION_PARAMETERS, zend_property_reference *property_reference);

PHP_GTK_API zend_class_entry* php_gtk_register_class(const char *class_name, function_entry *class_functions, zend_class_entry *parent, zend_bool have_getter, zend_bool have_setter, char **class_props TSRMLS_DC);

PHP_GTK_API void php_gtk_register_prop_getter(zend_class_entry *ce, prop_getter_t getter);
PHP_GTK_API void php_gtk_register_prop_setter(zend_class_entry *ce, prop_setter_t setter);

PHP_GTK_API void php_gtk_object_init(GtkObject *obj, zval *wrapper);

/* Utility functions. */
PHP_GTK_API int php_gtk_parse_args(int argc, char *format, ...);
int php_gtk_parse_args_quiet(int argc, char *format, ...);
int php_gtk_parse_args_hash(zval *hash, char *format, ...);
int php_gtk_parse_args_hash_quiet(zval *hash, char *format, ...);
PHP_GTK_API int php_gtk_check_class(zval *wrapper, zend_class_entry *expected_ce);
PHP_GTK_API void php_gtk_invalidate(zval *wrapper);
zend_bool php_gtk_is_callable(zval *callable, zend_bool syntax_only, char **callable_name);
zval *php_gtk_array_as_hash(zval ***values, int num_values, int start, int length);
zval ***php_gtk_hash_as_array(zval *hash);
zval ***php_gtk_func_args(int argc);
PHP_GTK_API zval *php_gtk_func_args_as_hash(int argc, int start, int length);
PHP_GTK_API zval *php_gtk_build_value(char *format, ...);
char *php_gtk_zval_type_name(zval *arg);

#define NOT_STATIC_METHOD() \
	if (!this_ptr) { \
		php_error(E_WARNING, "%s() is not a static method", get_active_function_name(TSRMLS_C)); \
		return; \
	}

PHP_GTK_API PHP_FUNCTION(no_constructor);
PHP_GTK_API PHP_FUNCTION(no_direct_constructor);

extern char *php_gtk_zval_type_name(zval *arg);

PHP_GTK_API ZEND_EXTERN_MODULE_GLOBALS(gtk);

#endif /* HAVE_PHP_GTK */

#endif	/* _PHP_GTK_H */
