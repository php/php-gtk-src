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

/* $Id$ */

#include "php_gtk.h"

#if HAVE_PHP_GTK

/* XXX #include "ext/gtk+/php_gtk+.h" */

typedef struct {
	zend_bool have_getter : 1;
	zend_bool have_setter : 1;
} prop_desc_t;

static const char *php_gtk_wrapper_key  = "php_gtk::wrapper";

HashTable php_gtk_prop_getters;
HashTable php_gtk_prop_setters;
HashTable php_gtk_rsrc_hash;
HashTable php_gtk_type_hash;
HashTable php_gtk_prop_desc;
HashTable php_gtk_callback_hash;

PHP_GTK_API zend_object_handlers php_gtk_handlers;

/* Generic get/set property handlers. */
static inline zval* invoke_getter(zval *object, char *property)
{
	zend_class_entry *ce;
	prop_getter_t *getter;
	zval result, *result_ptr = NULL;
	int found = FAILURE;
	TSRMLS_FETCH();

	ZVAL_NULL(&result);
	for (ce = Z_OBJCE_P(object); ce != NULL && found != SUCCESS; ce = ce->parent) {
		if (zend_hash_index_find(&php_gtk_prop_getters, (long)ce, (void **)&getter) == SUCCESS) {
			(*getter)(&result, object, property, &found);
		}
	}
	if (found == SUCCESS) {
		ALLOC_ZVAL(result_ptr);
		*result_ptr = result;
		result_ptr->refcount = 0;
		//INIT_PZVAL(result_ptr);
	}

	return result_ptr;
}

static inline int invoke_setter(zval *object, char *property, zval *value)
{
	zend_class_entry *ce;
	prop_setter_t *setter;
	int found = FAILURE;
	TSRMLS_FETCH();

	for (ce = Z_OBJCE_P(object); ce != NULL && found != SUCCESS; ce = ce->parent) {
		if (zend_hash_index_find(&php_gtk_prop_setters, (long)ce, (void **)&setter) == SUCCESS) {
			found = (*setter)(object, property, value);
		}
	}

	return found;
}

static zval **php_gtk_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC)
{
	return zend_object_create_proxy(object, member TSRMLS_CC);
}

static zend_object_value phpg_create_object(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value zov;
	phpg_gobject_t *object;
	prop_desc_t *prop_desc;

	object = emalloc(sizeof(phpg_gobject_t));
	phpg_init_object(object, ce);
	object->obj  = NULL;
	object->dtor = NULL;

	zov.handlers = &php_gtk_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) php_gtk_destroy_object, NULL TSRMLS_CC);

	return zov;
}

PHP_GTK_API void php_gtk_object_init(GtkObject *obj, zval *zobj)
{
	gtk_object_ref(obj);
	gtk_object_sink(obj);

	php_gtk_set_object(zobj, obj, (php_gtk_dtor_t)gtk_object_unref, 0);
}

PHP_GTK_API void *php_gtk_get_object(zval *zobj)
{
	//void *obj;
	//int type;
	php_gtk_object *wrapper;
	TSRMLS_FETCH();
	
	if (Z_TYPE_P(zobj) != IS_OBJECT) {
		php_error(E_ERROR, "Wrapper is not an object");
	}

	wrapper = zend_object_store_get_object(zobj TSRMLS_CC);
	if (!wrapper->obj) {
		php_error(E_ERROR, "Underlying object missing");
	}

	/*
	if (zend_hash_index_find(Z_OBJPROP_P(zobj), 0, (void **)&handle) == FAILURE) {
		php_error(E_ERROR, "Underlying object missing");
	}
	obj = zend_list_find(Z_LVAL_PP(handle), &type);
	if (!obj || type != rsrc_type) {
		php_error(E_ERROR, "Underlying object missing or of invalid type");
	}
	*/

	return wrapper->obj;
}

PHP_GTK_API int php_gtk_get_simple_enum_value(zval *enum_val, int *result)
{
	if (!enum_val)
		return 0;

	if (Z_TYPE_P(enum_val) == IS_LONG) {
		*result = Z_LVAL_P(enum_val);
		return 1;
	}

	php_error(E_WARNING, "simple enum values must be integers");
	return 0;
}

PHP_GTK_API int php_gtk_get_enum_value(GType enum_type, zval *enum_val, int *result)
{
	if (!enum_val)
		return 0;

	if (Z_TYPE_P(enum_val) == IS_LONG) {
		*result = Z_LVAL_P(enum_val);
		return 1;
	} else if (Z_TYPE_P(enum_val) == IS_STRING) {
		GtkEnumValue *info = gtk_type_enum_find_value(enum_type, Z_STRVAL_P(enum_val));
		if (!info) {
			php_error(E_WARNING, "Could not translate enum value '%s'", Z_STRVAL_P(enum_val));
			return 0;
		}
		*result = info->value;
		return 1;
	}

	php_error(E_WARNING, "enum values must be integers or strings");
	return 0;
}

int php_gtk_get_flag_value(GType flag_type, zval *flag_val, int *result)
{
	if (!flag_val)
		return 0;

	if (Z_TYPE_P(flag_val) == IS_LONG) {
		*result = Z_LVAL_P(flag_val);
		return 1;
	} else if (Z_TYPE_P(flag_val) == IS_STRING) {
		GtkFlagValue *info = gtk_type_flags_find_value(flag_type, Z_STRVAL_P(flag_val));
		if (!info) {
			php_error(E_WARNING, "Could not translate flag value '%s'", Z_STRVAL_P(flag_val));
			return 0;
		}
		*result = info->value;
		return 1;
	} else if (Z_TYPE_P(flag_val) == IS_ARRAY) {
		zval **flag;

		for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(flag_val));
			 zend_hash_get_current_data(Z_ARRVAL_P(flag_val), (void **)&flag) == SUCCESS;
			 zend_hash_move_forward(Z_ARRVAL_P(flag_val))) {
			if (Z_TYPE_PP(flag) == IS_LONG)
				*result |= Z_LVAL_PP(flag);
			else if (Z_TYPE_PP(flag) == IS_STRING) {
				GtkFlagValue *info = gtk_type_flags_find_value(flag_type, Z_STRVAL_PP(flag));
				if (!info) {
					php_error(E_WARNING, "Could not translate flag value '%s'", Z_STRVAL_PP(flag));
					return 0;
				}
				*result |= info->value;
			} else {
				php_error(E_WARNING, "flag arrays can contain only integers or strings");
				return 0;
			}
		}

		return 1;
	}

	php_error(E_WARNING, "flag values must be integers or strings");
	return 0;
}

/* Generic callback marshal. */
PHP_GTK_API void php_gtk_callback_marshal(GtkObject *o, gpointer data, guint nargs, GtkArg *args)
{
	zval *gtk_args;
	zval *callback_data = (zval *)data;
	zval **callback, **extra = NULL, **pass_object = NULL;
	zval **callback_filename = NULL, **callback_lineno = NULL;
	zval *wrapper = NULL;
	zval *params;
	zval *retval = NULL;
	zval ***signal_args;
	char *callback_name;
	TSRMLS_FETCH();

	/* Callback is always passed as the first element. */
	zend_hash_index_find(Z_ARRVAL_P(callback_data), 0, (void **)&callback);

	/*
	 * If there is more than one element, it will be an array of:
	 *  [1] an array of extra arguments
	 *  [2] a flag indidicating whether the object should be passed to the
	 *      callback
	 *  [3] the filename where the callback was specified
	 *  [4] the line number where the callback was specified
	 */
	if (zend_hash_num_elements(Z_ARRVAL_P(callback_data)) > 1) {
		zend_hash_index_find(Z_ARRVAL_P(callback_data), 1, (void **)&extra);
		zend_hash_index_find(Z_ARRVAL_P(callback_data), 2, (void **)&pass_object);
		zend_hash_index_find(Z_ARRVAL_P(callback_data), 3, (void **)&callback_filename);
		zend_hash_index_find(Z_ARRVAL_P(callback_data), 4, (void **)&callback_lineno);
	}

	if (!zend_is_callable(*callback, 0, &callback_name)) {
		if (callback_filename)
			php_error(E_WARNING, "Unable to call signal callback '%s' specified in %s on line %ld", callback_name, Z_STRVAL_PP(callback_filename), Z_LVAL_PP(callback_lineno));
		else
			php_error(E_WARNING, "Unable to call callback '%s'", callback_name);
		efree(callback_name);
		return;
	}

	gtk_args = php_gtk_args_as_hash(nargs, args);
	
	/*
	 * If pass_object flag is not specified, or it's specified and true, and we
	 * have the actual object, construct the wrapper around it.
	 */
	if ((!pass_object || Z_LVAL_PP(pass_object)) && o)
		wrapper = php_gtk_new(o);

	/*
	 * If there is a wrapper, construct array of parameters, set wrapper as
	 * the first parameter, and append the array of GTK signal arguments to it.
	 */
	if (wrapper) {
		MAKE_STD_ZVAL(params);
		array_init(params);
		zend_hash_next_index_insert(Z_ARRVAL_P(params), &wrapper, sizeof(zval *), NULL);
		php_array_merge(Z_ARRVAL_P(params), Z_ARRVAL_P(gtk_args), 0 TSRMLS_CC);
		zval_ptr_dtor(&gtk_args);
	} else
		/* Otherwise, the only parameters will be GTK signal arguments. */
		params = gtk_args;

	/*
	 * If there are extra arguments specified by user, add them to the parameter
	 * array.
	 */
	if (extra)
		php_array_merge(Z_ARRVAL_P(params), Z_ARRVAL_PP(extra), 0 TSRMLS_CC);
	
	signal_args = php_gtk_hash_as_array(params);

	call_user_function_ex(EG(function_table), NULL, *callback, &retval, zend_hash_num_elements(Z_ARRVAL_P(params)), signal_args, 0, NULL TSRMLS_CC);

	if (retval) {
		if (args)
			php_gtk_ret_from_value(&args[nargs], retval);
		zval_ptr_dtor(&retval);
	}

	efree(signal_args);
	zval_ptr_dtor(&params);
}

void php_gtk_handler_marshal(gpointer a, gpointer data, int nargs, GtkArg *args)
{
	zval *callback_data = (zval *)data;
	zval **callback, **extra = NULL;
	zval **callback_filename = NULL, **callback_lineno = NULL;
	zval ***handler_args = NULL;
	int num_handler_args = 0;
	zval *retval = NULL;
	char *callback_name;
	TSRMLS_FETCH();

	/* Callback is always passed as the first element. */
	zend_hash_index_find(Z_ARRVAL_P(callback_data), 0, (void **)&callback);
	zend_hash_index_find(Z_ARRVAL_P(callback_data), 1, (void **)&extra);
	zend_hash_index_find(Z_ARRVAL_P(callback_data), 2, (void **)&callback_filename);
	zend_hash_index_find(Z_ARRVAL_P(callback_data), 3, (void **)&callback_lineno);

	if (!php_gtk_is_callable(*callback, 0, &callback_name)) {
		php_error(E_WARNING, "Unable to call handler callback '%s' specified in %s on line %ld", callback_name, Z_STRVAL_PP(callback_filename), Z_LVAL_PP(callback_lineno));
		efree(callback_name);
		return;
	}

	handler_args = php_gtk_hash_as_array(*extra);
	num_handler_args = zend_hash_num_elements(Z_ARRVAL_PP(extra));

	call_user_function_ex(EG(function_table), NULL, *callback, &retval, num_handler_args, handler_args, 0, NULL TSRMLS_CC);

	*GTK_RETLOC_BOOL(args[0]) = FALSE;
	if (retval) {
		if (zval_is_true(retval))
			*GTK_RETLOC_BOOL(args[0]) = TRUE;
		else
			*GTK_RETLOC_BOOL(args[0]) = FALSE;
		zval_ptr_dtor(&retval);
	}

	if (handler_args)
		efree(handler_args);
}

PHP_GTK_API void php_gtk_destroy_notify(gpointer user_data)
{
	zval *value = (zval *)user_data;
	zval_ptr_dtor(&value);
}

PHP_GTK_API zval *php_gtk_new(GtkObject *obj)
{
	zval *zobj;
	php_gtk_object *wrapper = NULL;
	zend_class_entry *ce;
	GType type;
	TSRMLS_FETCH();
	
	if (!obj) {
		MAKE_STD_ZVAL(zobj);
		ZVAL_NULL(zobj);
		return zobj;
	}

    if ((zobj = (zval *) gtk_object_get_data(obj, php_gtk_wrapper_key))) {
		zval_add_ref(&zobj);
		return zobj;
	}

	type = GTK_OBJECT_TYPE(obj);
	while ((ce = g_hash_table_lookup(php_gtk_class_hash, gtk_type_name(type))) == NULL)
		type = gtk_type_parent(type);

	MAKE_STD_ZVAL(zobj);
	object_init_ex(zobj, ce);
	gtk_object_ref(obj);
	wrapper = zend_object_store_get_object(zobj TSRMLS_CC);
	wrapper->obj = obj;
	wrapper->dtor = (php_gtk_dtor_t)gtk_object_unref;
	zend_objects_store_add_ref(zobj TSRMLS_CC);
	gtk_object_set_data(obj, php_gtk_wrapper_key, zobj);
	zval_add_ref(&zobj);

	return zobj;
}

zval *php_gtk_args_as_hash(int nargs, GtkArg *args)
{
	zval *hash;
	zval *item;
	int i;

	MAKE_STD_ZVAL(hash);
	array_init(hash);
	for (i = 0; i < nargs; i++) {
		item = php_gtk_arg_as_value(&args[i]);
		if (!item) {
			MAKE_STD_ZVAL(item);
			ZVAL_NULL(item);
		}
		zend_hash_next_index_insert(Z_ARRVAL_P(hash), &item, sizeof(zval *), NULL);
	}

	return hash;
}

GtkArg *php_gtk_hash_as_args(zval *hash, GType type, gint *nargs)
{
	int i;
	zval **item;
	gchar *err, buf[255];
	ulong num_key;
	GtkArg *arg = NULL;
	/* GtkArgInfo *info; */
	HashTable *ht;
	TSRMLS_FETCH();

	ht = HASH_OF(hash);

	gtk_type_class(type);
	*nargs = zend_hash_num_elements(ht);
	arg = g_new(GtkArg, *nargs);

	/* XXX
	for (zend_hash_internal_pointer_reset(ht), i = 0;
		 zend_hash_get_current_data(ht, (void **)&item) == SUCCESS;
		 zend_hash_move_forward(ht), i++) {
		if (zend_hash_get_current_key(ht, &arg[i].name, &num_key, 0) != HASH_KEY_IS_STRING) {
			php_error(E_WARNING, "array keys must be strings");
			g_free(arg);
			return NULL;
		}

		err = gtk_object_arg_get_info(type, arg[i].name, &info);
		if (!info) {
			php_error(E_WARNING, err);
			g_free(err);
			g_free(arg);
			return NULL;
		}

		arg[i].type = info->type;
		arg[i].name = info->name;

		if (!php_gtk_arg_from_value(&arg[i], *item)) {
			g_snprintf(buf, 255, "argument '%s': expected type %s, found %s",
					   arg[i].name, gtk_type_name(arg[i].type),
					   php_gtk_zval_type_name(*item));
			php_error(E_WARNING, buf);
			g_free(arg);
			return NULL;
		}
	}
	*/

	return arg;
}

int php_gtk_args_from_hash(GtkArg *args, int nparams, zval *hash)
{
	zval **item;
	HashTable *ht;
	int i;
	TSRMLS_FETCH();

	ht = HASH_OF(hash);

	for (zend_hash_internal_pointer_reset(ht), i = 0;
		 i < nparams && zend_hash_get_current_data(ht, (void **)&item) == SUCCESS;
		 zend_hash_move_forward(ht), i++) {
		if (!php_gtk_arg_from_value(&args[i], *item)) {
			gchar buf[512];

			if (args[i].name == NULL)
				g_snprintf(buf, 511, "argument %d: expected %s, %s found", i+1,
						   gtk_type_name(args[i].type),
						   php_gtk_zval_type_name(*item));
			else
				g_snprintf(buf, 511, "argument %d: expected %s, %s found", i+1,
						   gtk_type_name(args[i].type),
						   php_gtk_zval_type_name(*item));
			php_error(E_WARNING, buf);
			return 0;
		}
	}

	return 1;
}

zval *php_gtk_arg_as_value(GtkArg *arg)
{
	zval *value;
	TSRMLS_FETCH();

	/*
	switch (GTK_FUNDAMENTAL_TYPE(arg->type)) {
		case GTK_TYPE_INVALID:
		case GTK_TYPE_NONE:
			MAKE_STD_ZVAL(value);
			ZVAL_NULL(value);
			break;

		case GTK_TYPE_CHAR:
		case GTK_TYPE_UCHAR:
			MAKE_STD_ZVAL(value);
			ZVAL_STRINGL(value, &GTK_VALUE_CHAR(*arg), 1, 1);
			break;

		case GTK_TYPE_BOOL:
			MAKE_STD_ZVAL(value);
			ZVAL_BOOL(value, GTK_VALUE_BOOL(*arg));
			break;

		case GTK_TYPE_ENUM:
		case GTK_TYPE_FLAGS:
		case GTK_TYPE_INT:
			MAKE_STD_ZVAL(value);
			ZVAL_LONG(value, GTK_VALUE_INT(*arg));
			break;

		case GTK_TYPE_LONG:
			MAKE_STD_ZVAL(value);
			ZVAL_LONG(value, GTK_VALUE_LONG(*arg));
			break;

		case GTK_TYPE_UINT:
			MAKE_STD_ZVAL(value);
			ZVAL_LONG(value, GTK_VALUE_UINT(*arg));
			break;

		case GTK_TYPE_ULONG:
			MAKE_STD_ZVAL(value);
			ZVAL_LONG(value, GTK_VALUE_ULONG(*arg));
			break;

		case GTK_TYPE_FLOAT:
			MAKE_STD_ZVAL(value);
			ZVAL_DOUBLE(value, GTK_VALUE_FLOAT(*arg));
			break;

		case GTK_TYPE_DOUBLE:
			MAKE_STD_ZVAL(value);
			ZVAL_DOUBLE(value, GTK_VALUE_DOUBLE(*arg));
			break;

		case GTK_TYPE_STRING:
			MAKE_STD_ZVAL(value);
			if (GTK_VALUE_STRING(*arg) != NULL) {
				ZVAL_STRING(value, GTK_VALUE_STRING(*arg), 1);
			} else
				ZVAL_NULL(value);
			break;

		case GTK_TYPE_ARGS:
			value = php_gtk_args_as_hash(GTK_VALUE_ARGS(*arg).n_args,
										 GTK_VALUE_ARGS(*arg).args);
			break;

		case GTK_TYPE_OBJECT:
			value = php_gtk_new(GTK_VALUE_OBJECT(*arg));
			break;

		case GTK_TYPE_POINTER:
			php_error(E_NOTICE, "%s(): internal error: %s GTK_TYPE_POINTER unsupported",
					  get_active_function_name(TSRMLS_C), arg->name );
			MAKE_STD_ZVAL(value);
			ZVAL_NULL(value);
			break;

		case GTK_TYPE_BOXED:
			if (arg->type == GTK_TYPE_GDK_EVENT)
				value = php_gdk_event_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_GDK_WINDOW)
				value = php_gdk_window_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_GDK_COLOR)
				value = php_gdk_color_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_GDK_COLORMAP)
				value = php_gdk_colormap_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_GDK_VISUAL)
				value = php_gdk_visual_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_GDK_FONT)
				value = php_gdk_font_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_GDK_DRAG_CONTEXT)
				value = php_gdk_drag_context_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_ACCEL_GROUP)
				value = php_gtk_accel_group_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_STYLE)
				value = php_gtk_style_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_SELECTION_DATA)
				value = php_gtk_selection_data_new(GTK_VALUE_BOXED(*arg));
			else if (arg->type == GTK_TYPE_CTREE_NODE)
				value = php_gtk_ctree_node_new(GTK_VALUE_BOXED(*arg));
			else
				return NULL;
			break;

		case GTK_TYPE_FOREIGN:
			value = (zval *)GTK_VALUE_FOREIGN(*arg).data;
			zval_add_ref(&value);
			break;

		case GTK_TYPE_CALLBACK:
			value = (zval *)GTK_VALUE_CALLBACK(*arg).data;
			zval_add_ref(&value);
			break;

		case GTK_TYPE_SIGNAL:
			value = (zval *)GTK_VALUE_SIGNAL(*arg).d;
			zval_add_ref(&value);
			break;

		default:
			g_assert_not_reached();
			return NULL;
	}
*/

	return value;
}

int php_gtk_arg_from_value(GtkArg *arg, zval *value)
{
	/*
	switch (GTK_FUNDAMENTAL_TYPE(arg->type)) {
		case GTK_TYPE_NONE:
		case GTK_TYPE_INVALID:
			GTK_VALUE_INT(*arg) = 0;
			break;

		case GTK_TYPE_BOOL:
			convert_to_boolean(value);
			GTK_VALUE_BOOL(*arg) = Z_BVAL_P(value);
			break;

		case GTK_TYPE_CHAR:
		case GTK_TYPE_UCHAR:
			convert_to_string(value);
			GTK_VALUE_CHAR(*arg) = Z_STRVAL_P(value)[0];
			break;

		case GTK_TYPE_ENUM:
			if (!php_gtk_get_enum_value(arg->type, value, &(GTK_VALUE_ENUM(*arg))))
				return 0;
			break;

		case GTK_TYPE_FLAGS:
			if (!php_gtk_get_flag_value(arg->type, value, &(GTK_VALUE_FLAGS(*arg))))
				return 0;
			break;

		case GTK_TYPE_INT:
			convert_to_long(value);
			GTK_VALUE_INT(*arg) = Z_LVAL_P(value);
			break;

		case GTK_TYPE_UINT:
			convert_to_long(value);
			GTK_VALUE_UINT(*arg) = Z_LVAL_P(value);
			break;

		case GTK_TYPE_LONG:
			convert_to_long(value);
			GTK_VALUE_LONG(*arg) = Z_LVAL_P(value);
			break;

		case GTK_TYPE_ULONG:
			convert_to_long(value);
			GTK_VALUE_ULONG(*arg) = Z_LVAL_P(value);
			break;

		case GTK_TYPE_FLOAT:
			convert_to_double(value);
			GTK_VALUE_FLOAT(*arg) = (gfloat)Z_DVAL_P(value);
			break;

		case GTK_TYPE_DOUBLE:
			convert_to_double(value);
			GTK_VALUE_DOUBLE(*arg) = Z_DVAL_P(value);
			break;

		case GTK_TYPE_STRING:
			convert_to_string(value);
			GTK_VALUE_STRING(*arg) = Z_STRVAL_P(value);
			break;

		case GTK_TYPE_OBJECT:
			if (Z_TYPE_P(value) == IS_OBJECT && php_gtk_check_class(value, gtk_object_ce))
				GTK_VALUE_OBJECT(*arg) = PHP_GTK_GET(value);
			else
				return 0;
			break;

		case GTK_TYPE_BOXED:
			if (arg->type == GTK_TYPE_GDK_EVENT) {
				if (php_gtk_check_class(value, gdk_event_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GDK_EVENT_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_GDK_WINDOW) {
				if (php_gtk_check_class(value, gdk_window_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GDK_WINDOW_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_GDK_COLOR) {
				if (php_gtk_check_class(value, gdk_color_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GDK_COLOR_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_GDK_COLORMAP) {
				if (php_gtk_check_class(value, gdk_colormap_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GDK_COLORMAP_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_GDK_VISUAL) {
				if (php_gtk_check_class(value, gdk_visual_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GDK_VISUAL_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_GDK_FONT) {
				if (php_gtk_check_class(value, gdk_font_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GDK_FONT_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_GDK_DRAG_CONTEXT) {
				if (php_gtk_check_class(value, gdk_drag_context_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GDK_DRAG_CONTEXT_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_ACCEL_GROUP) {
				if (php_gtk_check_class(value, gtk_accel_group_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GTK_ACCEL_GROUP_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_STYLE) {
				if (php_gtk_check_class(value, gtk_style_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GTK_STYLE_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_SELECTION_DATA) {
				if (php_gtk_check_class(value, gtk_selection_data_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GTK_SELECTION_DATA_GET(value);
				else
					return 0;
			} else if (arg->type == GTK_TYPE_CTREE_NODE) {
				if (php_gtk_check_class(value, gtk_ctree_node_ce))
					GTK_VALUE_BOXED(*arg) = PHP_GTK_CTREE_NODE_GET(value);
				else
					return 0;
			} else
				return 0;
			break;

		case GTK_TYPE_FOREIGN:
			zval_add_ref(&value);
			GTK_VALUE_FOREIGN(*arg).data = value;
			GTK_VALUE_FOREIGN(*arg).notify = php_gtk_destroy_notify;
			break;

		case GTK_TYPE_SIGNAL:
			if (php_gtk_is_callable(value, 1, NULL)) {
				zval_add_ref(&value);
				GTK_VALUE_SIGNAL(*arg).f = NULL;
				GTK_VALUE_SIGNAL(*arg).d = value;
			} else
				return 0;
			break;

		case GTK_TYPE_CALLBACK:
			if (php_gtk_is_callable(value, 1, NULL)) {
				zval_add_ref(&value);
				GTK_VALUE_CALLBACK(*arg).marshal = php_gtk_callback_marshal;
				GTK_VALUE_CALLBACK(*arg).data = value;
				GTK_VALUE_CALLBACK(*arg).notify = php_gtk_destroy_notify;
			} else
				return 0;
			break;

		case GTK_TYPE_ARGS:
		case GTK_TYPE_POINTER:
		case GTK_TYPE_C_CALLBACK:
			php_error(E_WARNING, "Unsupported type");
			g_assert_not_reached();
			return 0;
	}
*/

	return 1;
}

zval *php_gtk_ret_as_value(GtkArg *ret)
{
	zval *value;
	TSRMLS_FETCH();

	/*
	switch (GTK_FUNDAMENTAL_TYPE(ret->type)) {
		case GTK_TYPE_INVALID:
		case GTK_TYPE_NONE:
			MAKE_STD_ZVAL(value);
			ZVAL_NULL(value);
			break;

		case GTK_TYPE_CHAR:
		case GTK_TYPE_UCHAR:
			MAKE_STD_ZVAL(value);
			ZVAL_STRINGL(value, GTK_RETLOC_CHAR(*ret), 1, 1);
			break;

		case GTK_TYPE_BOOL:
			MAKE_STD_ZVAL(value);
			ZVAL_BOOL(value, *GTK_RETLOC_BOOL(*ret));
			break;

		case GTK_TYPE_ENUM:
		case GTK_TYPE_FLAGS:
		case GTK_TYPE_INT:
			MAKE_STD_ZVAL(value);
			ZVAL_LONG(value, *GTK_RETLOC_INT(*ret));
			break;

		case GTK_TYPE_LONG:
			MAKE_STD_ZVAL(value);
			ZVAL_LONG(value, *GTK_RETLOC_LONG(*ret));
			break;

		case GTK_TYPE_UINT:
			MAKE_STD_ZVAL(value);
			ZVAL_LONG(value, *GTK_RETLOC_UINT(*ret));
			break;

		case GTK_TYPE_ULONG:
			MAKE_STD_ZVAL(value);
			ZVAL_LONG(value, *GTK_RETLOC_ULONG(*ret));
			break;

		case GTK_TYPE_FLOAT:
			MAKE_STD_ZVAL(value);
			ZVAL_DOUBLE(value, *GTK_RETLOC_FLOAT(*ret));
			break;

		case GTK_TYPE_DOUBLE:
			MAKE_STD_ZVAL(value);
			ZVAL_DOUBLE(value, *GTK_RETLOC_DOUBLE(*ret));
			break;

		case GTK_TYPE_STRING:
			MAKE_STD_ZVAL(value);
			if (*GTK_RETLOC_STRING(*ret) != NULL) {
				ZVAL_STRING(value, *GTK_RETLOC_STRING(*ret), 1);
			} else
				ZVAL_NULL(value);
			break;

		case GTK_TYPE_ARGS:
			return NULL;

		case GTK_TYPE_OBJECT:
			value = php_gtk_new(*GTK_RETLOC_OBJECT(*ret));
			break;

		case GTK_TYPE_POINTER:
			php_error(E_NOTICE, "%s(): internal error: GTK_TYPE_POINTER unsupported",
					  get_active_function_name(TSRMLS_C));
			MAKE_STD_ZVAL(value);
			ZVAL_NULL(value);
			break;

		case GTK_TYPE_BOXED:
			if (ret->type == GTK_TYPE_GDK_EVENT)
				value = php_gdk_event_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_GDK_WINDOW)
				value = php_gdk_window_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_GDK_COLOR)
				value = php_gdk_color_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_GDK_COLORMAP)
				value = php_gdk_colormap_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_GDK_VISUAL)
				value = php_gdk_visual_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_GDK_FONT)
				value = php_gdk_font_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_GDK_DRAG_CONTEXT)
				value = php_gdk_drag_context_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_ACCEL_GROUP)
				value = php_gtk_accel_group_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_STYLE)
				value = php_gtk_style_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_SELECTION_DATA)
				value = php_gtk_selection_data_new(*GTK_RETLOC_BOXED(*ret));
			else if (ret->type == GTK_TYPE_CTREE_NODE)
				value = php_gtk_ctree_node_new(*GTK_RETLOC_BOXED(*ret));
			else
				return NULL;
			break;

		default:
			g_assert_not_reached();
			return NULL;
	}
*/

	return value;
}

void php_gtk_ret_from_value(GtkArg *ret, zval *value)
{
	/*
	switch (GTK_FUNDAMENTAL_TYPE(ret->type)) {
		case GTK_TYPE_NONE:
		case GTK_TYPE_INVALID:
			break;

		case GTK_TYPE_BOOL:
			convert_to_boolean(value);
			*GTK_RETLOC_BOOL(*ret) = Z_BVAL_P(value);
			break;

		case GTK_TYPE_CHAR:
			convert_to_string(value);
			*GTK_RETLOC_BOOL(*ret) = Z_STRVAL_P(value)[0];
			break;

		case GTK_TYPE_ENUM:
			if (!php_gtk_get_enum_value(ret->type, value, GTK_RETLOC_ENUM(*ret)))
				*GTK_RETLOC_ENUM(*ret) = 0;
			break;

		case GTK_TYPE_FLAGS:
			if (!php_gtk_get_flag_value(ret->type, value, GTK_RETLOC_FLAGS(*ret)))
				*GTK_RETLOC_FLAGS(*ret) = 0;
			break;

		case GTK_TYPE_INT:
			convert_to_long(value);
			*GTK_RETLOC_INT(*ret) = Z_LVAL_P(value);
			break;

		case GTK_TYPE_UINT:
			convert_to_long(value);
			*GTK_RETLOC_UINT(*ret) = Z_LVAL_P(value);
			break;

		case GTK_TYPE_LONG:
			convert_to_long(value);
			*GTK_RETLOC_LONG(*ret) = Z_LVAL_P(value);
			break;

		case GTK_TYPE_ULONG:
			convert_to_long(value);
			*GTK_RETLOC_ULONG(*ret) = Z_LVAL_P(value);
			break;

		case GTK_TYPE_FLOAT:
			convert_to_double(value);
			*GTK_RETLOC_FLOAT(*ret) = (gfloat)Z_DVAL_P(value);
			break;

		case GTK_TYPE_DOUBLE:
			convert_to_double(value);
			*GTK_RETLOC_DOUBLE(*ret) = Z_DVAL_P(value);
			break;

		case GTK_TYPE_STRING:
			convert_to_string(value);
			*GTK_RETLOC_STRING(*ret) = g_strdup(Z_STRVAL_P(value));
			break;

		case GTK_TYPE_OBJECT:
			if (Z_TYPE_P(value) == IS_OBJECT && php_gtk_check_class(value, gtk_object_ce))
				*GTK_RETLOC_OBJECT(*ret) = PHP_GTK_GET(value);
			else
				*GTK_RETLOC_OBJECT(*ret) = NULL;
			break;

		case GTK_TYPE_BOXED:
			if (ret->type == GTK_TYPE_GDK_EVENT) {
				if (php_gtk_check_class(value, gdk_event_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GDK_EVENT_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_GDK_WINDOW) {
				if (php_gtk_check_class(value, gdk_window_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GDK_WINDOW_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_GDK_COLOR) {
				if (php_gtk_check_class(value, gdk_color_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GDK_COLOR_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_GDK_COLORMAP) {
				if (php_gtk_check_class(value, gdk_colormap_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GDK_COLORMAP_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_GDK_VISUAL) {
				if (php_gtk_check_class(value, gdk_visual_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GDK_VISUAL_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_GDK_FONT) {
				if (php_gtk_check_class(value, gdk_font_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GDK_FONT_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_GDK_DRAG_CONTEXT) {
				if (php_gtk_check_class(value, gdk_drag_context_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GDK_DRAG_CONTEXT_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_ACCEL_GROUP) {
				if (php_gtk_check_class(value, gtk_accel_group_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GTK_ACCEL_GROUP_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_STYLE) {
				if (php_gtk_check_class(value, gtk_style_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GTK_STYLE_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_SELECTION_DATA) {
				if (php_gtk_check_class(value, gtk_selection_data_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GTK_SELECTION_DATA_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else if (ret->type == GTK_TYPE_CTREE_NODE) {
				if (php_gtk_check_class(value, gtk_ctree_node_ce))
					*GTK_RETLOC_BOXED(*ret) = PHP_GTK_CTREE_NODE_GET(value);
				else
					*GTK_RETLOC_BOXED(*ret) = NULL;
			} else
				*GTK_RETLOC_BOXED(*ret) = NULL;
			break;

		case GTK_TYPE_POINTER:
			php_error(E_WARNING, "internal error: GTK_TYPE_POINTER not supported");
			break;

		default:
			g_assert_not_reached();
			break;
	}
*/
}


/* Callback registry ? */
/*
PHP_GTK_API zval php_gtk_get_property(zend_property_reference *property_reference)
{
	zval foo;
	INIT_ZVAL(foo);
	return foo;
}

PHP_GTK_API int php_gtk_set_property(zend_property_reference *property_reference, zval *value)
{
	return FAILURE;
}
*/

PHP_GTK_API void phpg_register_prop_getter(zend_class_entry *ce, prop_getter_t getter)
{
	zend_hash_index_update(&php_gtk_prop_getters, (long)ce, (void*)&getter,
						   sizeof(prop_getter_t), NULL);
}

PHP_GTK_API void php_gtk_register_prop_setter(zend_class_entry *ce, prop_setter_t setter)
{
	zend_hash_index_update(&php_gtk_prop_setters, (long)ce, (void*)&setter,
						   sizeof(prop_setter_t), NULL);
}

PHP_GTK_API void php_gtk_register_callback(char *class_and_method, GtkSignalFunc  call_function)
{
	char *lcname;
	
	lcname = emalloc(strlen(class_and_method)+1);
	
	strncpy(lcname, class_and_method, strlen(class_and_method));
	zend_str_tolower(lcname, strlen(class_and_method));
	zend_hash_update(&php_gtk_callback_hash,  lcname, strlen(class_and_method),(void**)  &call_function,
						   sizeof(void*), NULL);
}


PHP_GTK_API void php_gtk_signal_connect_impl(INTERNAL_FUNCTION_PARAMETERS, int pass_object, int after)
{
	char *name = NULL;
	zval *callback = NULL;
	zval *extra;
	zval *data;
	char *callback_filename;
	uint callback_lineno;
	char *lookup;
	zend_class_entry *ce;
	GtkSignalFunc *callback_cfunction;
	int lookup_len;
	
	NOT_STATIC_METHOD();

	if (ZEND_NUM_ARGS() < 2) {
		php_error(E_WARNING, "%s() requires at least 2 arguments, %d given",
				  get_active_function_name(TSRMLS_C), ZEND_NUM_ARGS());
		return;
	}

	if (!php_gtk_parse_args(2, "sV", &name, &callback))
		return;

	callback_filename = zend_get_executed_filename(TSRMLS_C);
	callback_lineno = zend_get_executed_lineno(TSRMLS_C);
	extra = php_gtk_func_args_as_hash(ZEND_NUM_ARGS(), 2, ZEND_NUM_ARGS());
	data = php_gtk_build_value("(VNisi)", callback, extra, pass_object, callback_filename, callback_lineno);
	ce = Z_OBJCE_P(this_ptr);
	lookup_len = ce->name_length + strlen(name) + 2;
	lookup = emalloc(lookup_len + 1);
	sprintf(lookup, "%s::%s", ce->name, name);
	 
	lookup[lookup_len] = '\0';
	zend_str_tolower(lookup,lookup_len);
	
	/* now see if it's a manually handled object::callback.. */
	if (zend_hash_find(&php_gtk_callback_hash, lookup, lookup_len , (void **) &callback_cfunction) == SUCCESS) {
			 
			switch (after) {
				case 0:
					RETURN_LONG( gtk_signal_connect(PHP_GTK_GET(this_ptr),
					    name,
					    (GtkSignalFunc)  *callback_cfunction,
					    data));
				case 1:
					RETURN_LONG( gtk_signal_connect_after(PHP_GTK_GET(this_ptr),
					    name,
					    (GtkSignalFunc)  *callback_cfunction,
					    data));
			}
			 
	}
	
	
	RETURN_LONG(gtk_signal_connect_full(PHP_GTK_GET(this_ptr), name, NULL,
										(GtkCallbackMarshal)php_gtk_callback_marshal,
										data, php_gtk_destroy_notify, FALSE, after));
}

PHP_GTK_API zval* php_gtk_simple_signal_callback(GtkObject *o, gpointer data, zval *gtk_args )
{
	guint nargs;
	GtkArg *args;
	zval *callback_data = (zval *)data;
	zval **callback, **extra = NULL, **pass_object = NULL;
	zval **callback_filename = NULL, **callback_lineno = NULL;
	zval *wrapper = NULL;
	zval *params;
	zval *retval = NULL;
	zval ***signal_args;
	char *callback_name;
	TSRMLS_FETCH();

	/* Callback is always passed as the first element. */
	zend_hash_index_find(Z_ARRVAL_P(callback_data), 0, (void **)&callback);

	/*
	 * If there is more than one element, it will be an array of:
	 *  [1] an array of extra arguments
	 *  [2] a flag indidicating whether the object should be passed to the
	 *      callback
	 *  [3] the filename where the callback was specified
	 *  [4] the line number where the callback was specified
	 */
	if (zend_hash_num_elements(Z_ARRVAL_P(callback_data)) > 1) {
		zend_hash_index_find(Z_ARRVAL_P(callback_data), 1, (void **)&extra);
		zend_hash_index_find(Z_ARRVAL_P(callback_data), 2, (void **)&pass_object);
		zend_hash_index_find(Z_ARRVAL_P(callback_data), 3, (void **)&callback_filename);
		zend_hash_index_find(Z_ARRVAL_P(callback_data), 4, (void **)&callback_lineno);
	}

	if (!php_gtk_is_callable(*callback, 0, &callback_name)) {
		if (callback_filename)
			php_error(E_WARNING, "Unable to call signal callback '%s' specified in %s on line %d", callback_name, Z_STRVAL_PP(callback_filename), Z_LVAL_PP(callback_lineno));
		else
			php_error(E_WARNING, "Unable to call callback '%s'", callback_name);
		efree(callback_name);
		return NULL;
	}

	/* not needed: - you have to provide this! 
	gtk_args = php_gtk_args_as_hash(nargs, args);
	*/
	
	/*
	 * If pass_object flag is not specified, or it's specified and true, and we
	 * have the actual object, construct the wrapper around it.
	 */
	if ((!pass_object || Z_LVAL_PP(pass_object)) && o)
		wrapper = php_gtk_new(o);

	/*
	 * If there is a wrapper, construct array of parameters, set wrapper as
	 * the first parameter, and append the array of GTK signal arguments to it.
	 */
	if (wrapper) {
		MAKE_STD_ZVAL(params);
		array_init(params);
		zend_hash_next_index_insert(Z_ARRVAL_P(params), &wrapper, sizeof(zval *), NULL);
		php_array_merge(Z_ARRVAL_P(params), Z_ARRVAL_P(gtk_args), 0 TSRMLS_CC);
		zval_ptr_dtor(&gtk_args);
	} else
		/* Otherwise, the only parameters will be GTK signal arguments. */
		params = gtk_args;

	/*
	 * If there are extra arguments specified by user, add them to the parameter
	 * array.
	 */
	if (extra)
		php_array_merge(Z_ARRVAL_P(params), Z_ARRVAL_PP(extra), 0 TSRMLS_CC);
	
	signal_args = php_gtk_hash_as_array(params);

	call_user_function_ex(EG(function_table), NULL, *callback, &retval, zend_hash_num_elements(Z_ARRVAL_P(params)), signal_args, 0, NULL TSRMLS_CC);
	
	efree(signal_args);
	zval_ptr_dtor(&params);
	
	return retval;
	/*
	if (retval) {
		if (args)
			php_gtk_ret_from_value(&args[nargs], retval);
		zval_ptr_dtor(&retval);
	}
	*/

	
}

#endif  /* HAVE_PHP_GTK */
