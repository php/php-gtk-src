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
 
/* $Id$: */

#ifndef _PHP_GTK_H
#define _PHP_GTK_H

#include "php_gtk_module.h"
#include "gtk/gtkinvisible.h"

#if HAVE_PHP_GTK

#include <gtk/gtk.h>

extern int le_gtk;
extern int le_gdk_event;
extern int le_gdk_window;
extern int le_gdk_color;

extern zend_class_entry *gdk_event_ce;
extern zend_class_entry *gdk_window_ce;
extern zend_class_entry *gdk_color_ce;

/* Useful macros. */
#define PHP_GTK_GET(w) 			((GtkObject *)php_gtk_get_object(w, le_gtk))
#define PHP_GDK_EVENT_GET(w)	((GdkEvent *)php_gtk_get_object(w, le_gdk_event))
#define PHP_GDK_WINDOW_GET(w)	((GdkWindow *)php_gtk_get_object(w, le_gdk_window))
#define PHP_GDK_COLOR_GET(w)	((GdkColor *)php_gtk_get_object(w, le_gdk_color))

/* True globals. */
extern GHashTable *php_gtk_class_hash;

/* Function declarations. */

void php_gtk_register_classes(void);
void php_gtk_register_types(int module_number);
void php_gtk_register_constants(int module_number ELS_DC);
void php_gtk_set_object(zval *wrapper, void *obj, int rsrc_type);
void *php_gtk_get_object(zval *wrapper, int rsrc_type);
int php_gtk_get_enum_value(GtkType enum_type, zval *enum_val, int *result);
void php_gtk_destroy_notify(gpointer user_data);
void php_gtk_callback_marshal(GtkObject *o, gpointer data, guint nargs, GtkArg *args);
void php_gtk_ret_from_value(GtkArg *ret, zval *value);
zval *php_gtk_args_as_hash(int nargs, GtkArg *args);
zval *php_gtk_arg_as_value(GtkArg *arg);
int php_gtk_get_flag_value(GtkType flag_type, zval *flag_val, int *result);

/* Constructors and initializers. */
void php_gtk_object_init(GtkObject *obj, zval *wrapper);
zval *php_gtk_new(GtkObject *obj);
zval *php_gdk_event_new(GdkEvent *obj);
zval *php_gdk_window_new(GdkWindow *obj);
zval *php_gdk_color_new(GdkColor *obj);

/* Utility functions. */
int php_gtk_parse_args(int argc, char *format, ...);
int php_gtk_parse_args_quiet(int argc, char *format, ...);
int php_gtk_check_class(zval *wrapper, zend_class_entry *expected_ce);
void php_gtk_invalidate(zval *wrapper);
zend_bool php_gtk_check_callable(zval *function);
zval *php_gtk_array_as_hash(zval ***values, int num_values, int start, int length);
zval ***php_gtk_hash_as_array(zval *hash);
zval ***php_gtk_func_args(int argc);
zval *php_gtk_func_args_as_hash(int argc, int start, int length);
zval *php_gtk_build_value(char *format, ...);

#define NOT_STATIC_METHOD() \
	if (!this_ptr) { \
		php_error(E_WARNING, "%s() is not a static method", get_active_function_name()); \
		return; \
	}

#endif /* HAVE_PHP_GTK */

#endif	/* _PHP_GTK_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
