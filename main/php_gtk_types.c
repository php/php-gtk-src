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
#include <gdk/gdkx.h>

int le_gdk_event;
int le_gdk_window;
int le_gdk_color;
int le_gdk_colormap;
int le_gdk_cursor;
int le_gdk_visual;
int le_gdk_font;
int le_gdk_gc;
int le_gdk_drag_context;
int le_gtk_selection_data;
int le_gtk_ctree_node;
int le_gtk_accel_group;
int le_gtk_style;

zend_class_entry *gdk_event_ce;
zend_class_entry *gdk_window_ce;
zend_class_entry *gdk_pixmap_ce;
zend_class_entry *gdk_bitmap_ce;
zend_class_entry *gdk_color_ce;
zend_class_entry *gdk_colormap_ce;
zend_class_entry *gdk_atom_ce;
zend_class_entry *gdk_cursor_ce;
zend_class_entry *gdk_visual_ce;
zend_class_entry *gdk_font_ce;
zend_class_entry *gdk_gc_ce;
zend_class_entry *gdk_drag_context_ce;
zend_class_entry *gtk_selection_data_ce;
zend_class_entry *gtk_ctree_node_ce;
zend_class_entry *gtk_accel_group_ce;
zend_class_entry *gtk_style_ce;

static PHP_FUNCTION(wrap_no_direct_constructor)
{
	php_error(E_WARNING, "Class %s cannot be constructed directly",
			  get_active_function_name());
	php_gtk_invalidate(this_ptr);
}

/* GdkEvent */
static function_entry php_gdk_event_functions[] = {
	{"gdkevent", PHP_FN(wrap_no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_event_new(GdkEvent *event)
{
	zval *result;
	zval *value;

	MAKE_STD_ZVAL(result);

	if (!event) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_event_ce);

	php_gtk_set_object(result, event, le_gdk_event);

	add_property_long(result, "type", event->type);
	
	if (event->any.window) {
		value = php_gdk_window_new(event->any.window);
		add_property_zval(result, "window", value);
	} else
		add_property_null(result, "window");
	add_property_bool(result, "send_event", event->any.send_event);

	switch (event->type) {
		case GDK_NOTHING:
		case GDK_DELETE:
		case GDK_DESTROY:
			break;

		case GDK_EXPOSE:
			value = php_gtk_build_value("(iiii)",
										event->expose.area.x,
										event->expose.area.y,
										event->expose.area.width,
										event->expose.area.height);
			zend_hash_update(Z_OBJPROP_P(result), "area", sizeof("area"), &value, sizeof(zval *), NULL);
			add_property_long(result, "count", event->expose.count);
			break;

		case GDK_MOTION_NOTIFY:
			add_property_long(result, "time", event->motion.time);
			add_property_double(result, "x", event->motion.x);
			add_property_double(result, "y", event->motion.y);
			add_property_double(result, "pressure", event->motion.pressure);
			add_property_double(result, "xtilt", event->motion.xtilt);
			add_property_double(result, "ytilt", event->motion.ytilt);
			add_property_long(result, "state", event->motion.state);
			add_property_long(result, "is_hint", event->motion.is_hint);
			add_property_long(result, "source", event->motion.source);
			add_property_long(result, "deviceid", event->motion.deviceid);
			add_property_double(result, "x_root", event->motion.x_root);
			add_property_double(result, "y_root", event->motion.y_root);
			break;

		case GDK_BUTTON_PRESS:      /*GdkEventButton            button*/
		case GDK_2BUTTON_PRESS:     /*GdkEventButton            button*/
		case GDK_3BUTTON_PRESS:     /*GdkEventButton            button*/
		case GDK_BUTTON_RELEASE:    /*GdkEventButton            button*/
			add_property_long(result, "time", event->button.time);
			add_property_double(result, "x", event->button.x);
			add_property_double(result, "y", event->button.y);
			add_property_double(result, "pressure", event->button.pressure);
			add_property_double(result, "xtilt", event->button.xtilt);
			add_property_double(result, "ytilt", event->button.ytilt);
			add_property_long(result, "state", event->button.state);
			add_property_long(result, "button", event->button.button);
			add_property_long(result, "source", event->button.source);
			add_property_long(result, "deviceid", event->button.deviceid);
			add_property_double(result, "x_root", event->button.x_root);
			add_property_double(result, "y_root", event->button.y_root);
			break;

		case GDK_KEY_PRESS:
		case GDK_KEY_RELEASE:
			add_property_long(result, "time", event->key.time);
			add_property_long(result, "state", event->key.state);
			add_property_long(result, "keyval", event->key.keyval);
			add_property_stringl(result, "string", event->key.string, event->key.length, 1);
			break;

		case GDK_ENTER_NOTIFY:
		case GDK_LEAVE_NOTIFY:
			if (event->crossing.subwindow) {
				value = php_gdk_window_new(event->crossing.subwindow);
				add_property_zval(result, "subwindow", value);
			} else
				add_property_null(result, "subwindow");
			add_property_long(result, "time", event->crossing.time);
			add_property_double(result, "x", event->crossing.x);
			add_property_double(result, "y", event->crossing.y);
			add_property_double(result, "x_root", event->crossing.x_root);
			add_property_double(result, "y_root", event->crossing.y_root);
			add_property_long(result, "mode", event->crossing.mode);
			add_property_long(result, "detail", event->crossing.detail);
			add_property_bool(result, "focus", event->crossing.focus);
			add_property_long(result, "state", event->crossing.state);
			break;

		case GDK_FOCUS_CHANGE:
			add_property_bool(result, "in", event->focus_change.in);
			break;

		case GDK_CONFIGURE:
			add_property_long(result, "x", event->configure.x);
			add_property_long(result, "y", event->configure.y);
			add_property_long(result, "width", event->configure.width);
			add_property_long(result, "height", event->configure.height);
			break;

		case GDK_MAP:
		case GDK_UNMAP:
			break;

		case GDK_PROPERTY_NOTIFY:
			add_property_zval(result, "atom", php_gdk_atom_new(event->property.atom));
			add_property_long(result, "time", event->property.time);
			add_property_long(result, "state", event->property.state);
			break;

		case GDK_SELECTION_CLEAR:
		case GDK_SELECTION_REQUEST:
		case GDK_SELECTION_NOTIFY:
			add_property_zval(result, "selection", php_gdk_atom_new(event->selection.selection));
			add_property_zval(result, "target", php_gdk_atom_new(event->selection.target));
			add_property_zval(result, "property", php_gdk_atom_new(event->selection.property));
			add_property_long(result, "requestor", event->selection.requestor);
			add_property_long(result, "time", event->selection.time);
			break;

		case GDK_PROXIMITY_IN:
		case GDK_PROXIMITY_OUT:
			add_property_long(result, "time", event->proximity.time);
			add_property_long(result, "source", event->proximity.source);
			add_property_long(result, "deviceid", event->proximity.deviceid);
			break;

		case GDK_DRAG_ENTER:
		case GDK_DRAG_LEAVE:
		case GDK_DRAG_MOTION:
		case GDK_DRAG_STATUS:
		case GDK_DROP_START:
		case GDK_DROP_FINISHED:
			add_property_zval(result, "context", php_gdk_drag_context_new(event->dnd.context));
			add_property_long(result, "time", event->dnd.time);
			add_property_long(result, "x_root", event->dnd.x_root);
			add_property_long(result, "y_root", event->dnd.y_root);
			break;

		case GDK_CLIENT_EVENT:
			add_property_zval(result, "message_type", php_gdk_atom_new(event->client.message_type));
			add_property_long(result, "data_format", event->client.data_format);
			add_property_stringl(result, "data", event->client.data.b, 20, 1);
			break;

		case GDK_VISIBILITY_NOTIFY:
			add_property_long(result, "state", event->visibility.state);
			break;

		case GDK_NO_EXPOSE:
			break;

		default:
			break;
	}

	return result;
}


/* GdkWindow */
PHP_FUNCTION(gdk_window_raise)
{
	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	gdk_window_raise(PHP_GDK_WINDOW_GET(this_ptr));
	RETURN_NULL();
}

PHP_FUNCTION(gdk_window_lower)
{
	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	gdk_window_lower(PHP_GDK_WINDOW_GET(this_ptr));
	RETURN_NULL();
}

PHP_FUNCTION(gdk_window_get_pointer)
{
	guint32 deviceid;
    gdouble x = 0.0, y = 0.0, pressure = 0.0, xtilt = 0.0, ytilt = 0.0;
    GdkModifierType mask = 0;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &deviceid))
		return;

    gdk_input_window_get_pointer(PHP_GDK_WINDOW_GET(this_ptr), deviceid, &x, &y, &pressure, &xtilt, &ytilt, &mask);
    *return_value = *php_gtk_build_value("(dddddi)", x, y, pressure, xtilt, ytilt, mask);
}

PHP_FUNCTION(gdk_window_set_cursor)
{
	zval *cursor;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &cursor, gdk_cursor_ce))
		return;

	gdk_window_set_cursor(PHP_GDK_WINDOW_GET(this_ptr), PHP_GDK_CURSOR_GET(cursor));
	RETURN_NULL();
}

PHP_FUNCTION(gdk_window_new_gc)
{
	zval *php_values = NULL;
	zval **value;
	char *key;
	ulong key_len, num_key;
	HashTable *hash;
	GdkGCValues values;
	GdkGCValuesMask mask = 0;
	GdkGC *gc;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "|a", &php_values))
		return;

	if (php_values) {
		hash = HASH_OF(php_values);
		zend_hash_internal_pointer_reset(hash);
		while (zend_hash_get_current_data(hash, (void**)&value) == SUCCESS) {
			if (zend_hash_get_current_key_ex(hash, &key, &key_len, &num_key, 0, NULL) == HASH_KEY_IS_STRING) {
				if (!strncmp(key, "foreground", key_len)) {
					GdkColor *c;

					if (!php_gtk_check_class(*value, gdk_color_ce)) {
						php_error(E_WARNING, "foreground value has to be a GdkColor");
						return;
					}
					c = PHP_GDK_COLOR_GET(*value);
					mask |= GDK_GC_FOREGROUND;
					values.foreground.red 	= c->red;
					values.foreground.green = c->green;
					values.foreground.blue 	= c->blue;
					values.foreground.pixel = c->pixel;
				} else if (!strncmp(key, "background", key_len)) {
					GdkColor *c;

					if (!php_gtk_check_class(*value, gdk_color_ce)) {
						php_error(E_WARNING, "background value has to be a GdkColor");
						return;
					}
					c = PHP_GDK_COLOR_GET(*value);
					mask |= GDK_GC_BACKGROUND;
					values.background.red 	= c->red;
					values.background.green = c->green;
					values.background.blue 	= c->blue;
					values.background.pixel = c->pixel;
				} else if (!strncmp(key, "font", key_len)) {
					if (!php_gtk_check_class(*value, gdk_font_ce)) {
						php_error(E_WARNING, "font value has to be a GdkFont");
						return;
					}
					mask |= GDK_GC_FONT;
					values.font = PHP_GDK_FONT_GET(*value);
				} else if (!strncmp(key, "tile", key_len)) {
					if (!php_gtk_check_class(*value, gdk_window_ce)) {
						php_error(E_WARNING, "tile value has to be a GdkPixmap");
						return;
					}
					mask |= GDK_GC_TILE;
					values.tile = PHP_GDK_WINDOW_GET(*value);
				} else if (!strncmp(key, "stipple", key_len)) {
					if (!php_gtk_check_class(*value, gdk_window_ce)) {
						php_error(E_WARNING, "stipple value has to be a GdkPixmap");
						return;
					}
					mask |= GDK_GC_STIPPLE;
					values.stipple = PHP_GDK_WINDOW_GET(*value);
				} else if (!strncmp(key, "clip_mask", key_len)) {
					if (!php_gtk_check_class(*value, gdk_window_ce)) {
						php_error(E_WARNING, "clip_mask value has to be a GdkPixmap");
						return;
					}
					mask |= GDK_GC_CLIP_MASK;
					values.clip_mask = PHP_GDK_WINDOW_GET(*value);
				}
			}
			zend_hash_move_forward(hash);
		}
	}

	gc = gdk_gc_new_with_values(PHP_GDK_WINDOW_GET(this_ptr), &values, mask);
	*return_value = *php_gdk_gc_new(gc);
	gdk_gc_unref(gc);
}

PHP_FUNCTION(gdk_window_property_get)
{
	GdkAtom property, type = 0;
	gint delete = FALSE;
	GdkAtom atype;
	gint aformat, alength;
	guchar *data;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "O|Oi", &property, gdk_atom_ce, &type, gdk_atom_ce, &delete)) {
		gchar *prop_name;

		if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s|Oi", &prop_name, &type, gdk_atom_ce, &delete))
			return;

		property = gdk_atom_intern(prop_name, FALSE);
	}

	if (gdk_property_get(PHP_GDK_WINDOW_GET(this_ptr), property, type, 0, 9999, delete,
						 &atype, &aformat, &alength, &data)) {
		zval *php_data;
		int i;
		guint16 *data16;
		guint32 *data32;

		MAKE_STD_ZVAL(php_data);

		switch (aformat) {
			case 8:
				ZVAL_STRINGL(php_data, data, alength, 1);
				break;
				
			case 16:
				data16 = (guint16 *)data;
				array_init(php_data);
				for (i = 0; i < alength; i++)
					add_next_index_long(php_data, data16[i]);
				break;

			case 32:
				data32 = (guint32 *)data;
				array_init(php_data);
				for (i = 0; i < alength; i++)
					add_next_index_long(php_data, data32[i]);
				break;

			default:
				php_error(E_WARNING, "%s() got property format != 8, 16, or 32",
						  get_active_function_name());
				break;
		}
		g_free(data);
		*return_value = *php_gtk_build_value("(NiN)", php_gdk_atom_new(atype), aformat, php_data);
	}
}

PHP_FUNCTION(gdk_window_property_change)
{
	GdkAtom property, type;
	gint format;
	zval *php_mode, *php_data;
	GdkPropMode mode;
	guchar *data = NULL;
	gint n_elements;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "OOiVV", &property, gdk_atom_ce,
							&type, gdk_atom_ce, &format, &php_mode, &php_data)) {
		gchar *prop_name;

		if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "sOiVV/", &prop_name, &type,
								gdk_atom_ce, &format, &php_mode, &php_data))
			return;

			property = gdk_atom_intern(prop_name, FALSE);
	}

	if (!php_gtk_get_enum_value(GTK_TYPE_GDK_PROP_MODE, php_mode, (gint *)&mode))
		return;

	switch (format) {
		case 8:
			if (Z_TYPE_P(php_data) != IS_STRING) {
				php_error(E_WARNING, "%s() expects data to be a string for format=8", get_active_function_name());
				return;
			}
			data = Z_STRVAL_P(php_data);
			n_elements = Z_STRLEN_P(php_data);
			break;

		case 16:
			{
				guint16 *data16;
				zval **data_temp;
				int i;

				if (Z_TYPE_P(php_data) != IS_ARRAY) {
					php_error(E_WARNING, "%s() expects data to be an array for format=16", get_active_function_name());
					return;
				}
				n_elements = zend_hash_num_elements(Z_ARRVAL_P(php_data));
				data16 = g_new(guint16, n_elements);
				i = 0;
				for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(php_data));
					 zend_hash_get_current_data(Z_ARRVAL_P(php_data), (void **)&data_temp);
					 zend_hash_move_forward(Z_ARRVAL_P(php_data))) {
					convert_to_long_ex(data_temp);
					data16[i++] = Z_LVAL_PP(data_temp); 
				}
			}	
			break;

		case 32:
			{
				guint32 *data32;
				zval **data_temp;
				int i;

				if (Z_TYPE_P(php_data) != IS_ARRAY) {
					php_error(E_WARNING, "%s() expects data to be an array for format=32", get_active_function_name());
					return;
				}
				n_elements = zend_hash_num_elements(Z_ARRVAL_P(php_data));
				data32 = g_new(guint32, n_elements);
				i = 0;
				for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(php_data));
					 zend_hash_get_current_data(Z_ARRVAL_P(php_data), (void **)&data_temp);
					 zend_hash_move_forward(Z_ARRVAL_P(php_data))) {
					convert_to_long_ex(data_temp);
					data32[i++] = Z_LVAL_PP(data_temp); 
				}
			}	
			break;

		default:
			php_error(E_WARNING, "%s() expects format to be 8, 16, or 32", get_active_function_name());
			break;
	}

	gdk_property_change(PHP_GDK_WINDOW_GET(this_ptr), property, type, format,
						mode, data, n_elements);
	if (format != 8)
		g_free(data);
}

PHP_FUNCTION(gdk_window_property_delete)
{
	GdkAtom property;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "O", &property, gdk_atom_ce)) {
		gchar *prop_name;

		if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "s", &prop_name))
			return;

		property = gdk_atom_intern(prop_name, FALSE);
	}

	gdk_property_delete(PHP_GDK_WINDOW_GET(this_ptr), property);
}

static function_entry php_gdk_window_functions[] = {
	{"gdkwindow",		PHP_FN(wrap_no_direct_constructor), NULL},
	{"raise", 			PHP_FN(gdk_window_raise), NULL},
	{"lower", 			PHP_FN(gdk_window_lower), NULL},
	{"get_pointer", 	PHP_FN(gdk_window_get_pointer), NULL},
	{"set_cursor", 		PHP_FN(gdk_window_set_cursor), NULL},
	{"new_gc", 			PHP_FN(gdk_window_new_gc), NULL},
	{"property_get", 	PHP_FN(gdk_window_property_get), NULL},
	{"property_change", PHP_FN(gdk_window_property_change), NULL},
	{"property_delete", PHP_FN(gdk_window_property_delete), NULL},
	{NULL, NULL, NULL}
};

PHP_FUNCTION(gdk_pixmap_create_from_xpm)
{
	zval *window, *php_trans_color;
	gchar *filename;
	GdkColor *trans_color = NULL;
	GdkPixmap *pixmap = NULL;
	GdkBitmap *mask;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "ONs", &window, gdk_window_ce,
							&php_trans_color, gdk_color_ce, &filename))
		return;

	if (Z_TYPE_P(php_trans_color) != IS_NULL)
		trans_color = PHP_GDK_COLOR_GET(php_trans_color);

	pixmap = gdk_pixmap_create_from_xpm(PHP_GDK_WINDOW_GET(window), &mask, trans_color, filename);
	if (!pixmap) {
		php_error(E_WARNING, "%s() cannot load pixmap", get_active_function_name());
		return;
	}
	
	*return_value = *php_gtk_build_value("(NN)", php_gdk_window_new(pixmap), php_gdk_window_new(mask));
	gdk_pixmap_unref(pixmap);
	gdk_bitmap_unref(mask);
}

static function_entry php_gdk_pixmap_functions[] = {
	{"gdkpixmap",		PHP_FN(wrap_no_direct_constructor), NULL},
	{"new_gc", 			PHP_FN(gdk_window_new_gc), NULL},
	{"property_get", 	PHP_FN(gdk_window_property_get), NULL},
	{"property_change", PHP_FN(gdk_window_property_change), NULL},
	{"property_delete", PHP_FN(gdk_window_property_delete), NULL},
	{"create_from_xpm", PHP_FN(gdk_pixmap_create_from_xpm), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_window_new(GdkWindow *window)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!window) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_window_ce);

	gdk_window_ref(window);
	php_gtk_set_object(result, window, le_gdk_window);

	return result;
}

static void gdk_window_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	zval *value;
	GdkWindow *win = PHP_GDK_WINDOW_GET(object);
	gint x, y;
	GdkModifierType p_mask;
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);
	
	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "width")) {
		gdk_window_get_size(win, &x, NULL);
		ZVAL_LONG(return_value, x);
	} else if (!strcmp(prop_name, "height")) {
		gdk_window_get_size(win, NULL, &y);
		ZVAL_LONG(return_value, y);
	} else if (!strcmp(prop_name, "x")) {
		gdk_window_get_position(win, &x, NULL);
		ZVAL_LONG(return_value, x);
	} else if (!strcmp(prop_name, "y")) {
		gdk_window_get_position(win, NULL, &y);
		ZVAL_LONG(return_value, y);
	} else if (!strcmp(prop_name, "colormap")) {
		*return_value = *php_gdk_colormap_new(gdk_window_get_colormap(win));
	} else if (!strcmp(prop_name, "pointer")) {
		gdk_window_get_pointer(win, &x, &y, NULL);
		*return_value = *php_gtk_build_value("(ii)", x, y);
	} else if (!strcmp(prop_name, "pointer_state")) {
		gdk_window_get_pointer(win, NULL, NULL, &p_mask);
		ZVAL_LONG(return_value, p_mask);
	} else if (!strcmp(prop_name, "parent")) {
		GdkWindow *parent = gdk_window_get_parent(win);
		if (parent)
			*return_value = *php_gdk_window_new(parent);
	} else if (!strcmp(prop_name, "toplevel")) {
		*return_value = *php_gdk_window_new(gdk_window_get_toplevel(win));
	} else if (!strcmp(prop_name, "children")) {
		GList *children, *tmp;
		zval *child;
		children = gdk_window_get_children(win);
		array_init(return_value);
		for (tmp = children; tmp; tmp = tmp->next) {
			child = php_gdk_window_new(tmp->data);
			zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &child, sizeof(zval *), NULL);
		}
		g_list_free(children);
	} else if (!strcmp(prop_name, "type")) {
		ZVAL_LONG(return_value, gdk_window_get_type(win));
	} else if (!strcmp(prop_name, "depth")) {
		gdk_window_get_geometry(win, NULL, NULL, NULL, NULL, &x);
		ZVAL_LONG(return_value, x);
	}
#ifndef PHP_WIN32
	else if (!strcmp(prop_name, "xid")) {
		ZVAL_LONG(return_value, GDK_WINDOW_XWINDOW(win));
	}
#endif
	else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}


static void release_gdk_window_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkWindow *obj = (GdkWindow *)rsrc->ptr;
	if (gdk_window_get_type(obj) == GDK_WINDOW_PIXMAP)
		gdk_pixmap_unref(obj);
	else
		gdk_window_unref(obj);
}


/* GdkColor */
PHP_FUNCTION(gdkcolor)
{
	char *color_spec;
	GdkColor color;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "hhh", &color.red, &color.green, &color.blue)) {
		if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "s", &color_spec)) {
			php_error(E_WARNING, "%s() requires a color spec or RGB triplet", get_active_function_name());
			php_gtk_invalidate(this_ptr);
			return;
		} else if (!gdk_color_parse(color_spec, &color)) {
			php_error(E_WARNING, "%s() could not parse color spec '%s'", get_active_function_name(), color_spec);
			php_gtk_invalidate(this_ptr);
			return;
		}
	}

	php_gtk_set_object(this_ptr, gdk_color_copy(&color), le_gdk_color);
}

static function_entry php_gdk_color_functions[] = {
	{"gdkcolor", PHP_FN(gdkcolor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_color_new(GdkColor *color)
{
	zval *result;
	GdkColor *result_obj;

	MAKE_STD_ZVAL(result);

	if (!color) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_color_ce);

	result_obj = gdk_color_copy(color);
	php_gtk_set_object(result, result_obj, le_gdk_color);

	return result;
}

static void gdk_color_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GdkColor *color = PHP_GDK_COLOR_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "red")) {
		ZVAL_LONG(return_value, color->red);
	} else if (!strcmp(prop_name, "green")) {
		ZVAL_LONG(return_value, color->green);
	} else if (!strcmp(prop_name, "blue")) {
		ZVAL_LONG(return_value, color->blue);
	} else if (!strcmp(prop_name, "pixel")) {
		ZVAL_LONG(return_value, color->pixel);
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}

static int gdk_color_set_property(zval *object, zend_llist_element **element, zval *value)
{
	GdkColor *color = PHP_GDK_COLOR_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	convert_to_long(value);
	if (!strcmp(prop_name, "red")) {
		color->red = (gushort)Z_LVAL_P(value);
	} else if (!strcmp(prop_name, "green")) {
		color->green = (gushort)Z_LVAL_P(value);
	} else if (!strcmp(prop_name, "blue")) {
		color->blue = (gushort)Z_LVAL_P(value);
	} else {
		return FAILURE;
	}

	return SUCCESS;
}

static void release_gdk_color_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkColor *obj = (GdkColor *)rsrc->ptr;
	gdk_color_free(obj);
}


/* GdkColormap */
PHP_FUNCTION(gdk_colormap_size)
{
	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	ZVAL_LONG(return_value, PHP_GDK_COLORMAP_GET(this_ptr)->size);
}

PHP_FUNCTION(gdk_colormap_alloc)
{
	gchar *color_spec;
    GdkColor color = {0, 0, 0, 0};
	zend_bool writeable = 0, best_match = 1;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "hhh|bb", &color.red, &color.green, &color.blue, &writeable, &best_match)) {
		if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "s|bb", &color_spec, &writeable, &best_match)) {
			php_error(E_WARNING, "%s() requires a color spec or RGB triplet", get_active_function_name());
			return;
		} else if (!gdk_color_parse(color_spec, &color)) {
			php_error(E_WARNING, "%s() could not parse color spec '%s'", get_active_function_name(), color_spec);
			return;
		}
	}

	if (!gdk_colormap_alloc_color(PHP_GDK_COLORMAP_GET(this_ptr), &color, writeable, best_match)) {
		php_error(E_WARNING, "%s() could not allocate color");
		return;
	}

	*return_value = *php_gdk_color_new(&color);
}

static function_entry php_gdk_colormap_functions[] = {
	{"gdkcolormap", PHP_FN(wrap_no_direct_constructor), NULL},
	{"size", 		PHP_FN(gdk_colormap_size), NULL},
	{"alloc",		PHP_FN(gdk_colormap_alloc), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_colormap_new(GdkColormap *cmap)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!cmap) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_colormap_ce);

	gdk_colormap_ref(cmap);
	php_gtk_set_object(result, cmap, le_gdk_colormap);

	return result;
}

static void release_gdk_colormap_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkColormap *obj = (GdkColormap *)rsrc->ptr;
	gdk_colormap_unref(obj);
}

static void gdk_colormap_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GdkColormap *cmap = PHP_GDK_COLORMAP_GET(object);
	zend_overloaded_element *property = (zend_overloaded_element *)(*element)->data;
	zend_llist_element *next;
	char *prop_name = Z_STRVAL(property->element);
	int prop_index, i;

	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "colors") && cmap->colors) {
		next = (*element)->next;
		if (next) {
			property = (zend_overloaded_element *)next->data;
			if (Z_TYPE_P(property) == OE_IS_ARRAY && Z_TYPE(property->element) == IS_LONG) {
				*element = next;
				prop_index = Z_LVAL(property->element);
				if (prop_index < 0 || prop_index >= cmap->size) {
					php_error(E_WARNING, "colormap index out of range");
					return;
				}
				*return_value = *php_gdk_color_new(&cmap->colors[prop_index]);
			}
		} else {
			array_init(return_value);
			
			for (i = 0; i < cmap->size; i++)
				add_next_index_zval(return_value, php_gdk_color_new(&cmap->colors[i]));
		}
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}


/* GdkAtom */
/* TODO maybe change this to have a real constructor */
static function_entry php_gdk_atom_functions[] = {
	{"gdkatom", PHP_FN(wrap_no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

inline GdkAtom php_gdk_atom_get(zval *wrapper)
{
	zval **atom;
	
	zend_hash_find(Z_OBJPROP_P(wrapper), "atom", sizeof("atom"), (void**)&atom);
	return (GdkAtom)Z_LVAL_PP(atom);
}

zval *php_gdk_atom_new(GdkAtom atom)
{
	zval *result;
	gchar *atom_name;

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_atom_ce);
	add_property_long_ex(result, "atom", sizeof("atom"), atom);
	atom_name = gdk_atom_name(atom);
	if (atom_name)
		add_property_string_ex(result, "string", sizeof("string"), atom_name, 0);
	else
		add_property_null_ex(result, "string", sizeof("string"));

	return result;
}

/* GdkCursor */
static function_entry php_gdk_cursor_functions[] = {
	{"gdkcursor", PHP_FN(wrap_no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_cursor_new(GdkCursor *cursor)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!cursor) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_cursor_ce);
	php_gtk_set_object(result, cursor, le_gdk_cursor);

	return result;
}

static void release_gdk_cursor_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkCursor *obj = (GdkCursor *)rsrc->ptr;
	gdk_cursor_destroy(obj);
}

static void gdk_cursor_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GdkCursor *cursor = PHP_GDK_CURSOR_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "type")) {
		ZVAL_LONG(return_value, cursor->type);
	} else if (!strcmp(prop_name, "name")) {
		GtkEnumValue *vals = gtk_type_enum_get_values(GTK_TYPE_GDK_CURSOR_TYPE);
		while (vals->value_name != NULL && vals->value != (unsigned)cursor->type)
			vals++;
		if (vals->value_nick) {
			ZVAL_STRING(return_value, vals->value_nick, 1);
		} else {
			ZVAL_STRINGL(return_value, "*unknown*", sizeof("*unknown*"), 1);
		}
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}


/* GdkVisual */
static function_entry php_gdk_visual_functions[] = {
	{"gdkvisual", PHP_FN(wrap_no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_visual_new(GdkVisual *visual)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!visual) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_visual_ce);
	gdk_visual_ref(visual);
	php_gtk_set_object(result, visual, le_gdk_visual);

	return result;
}

static void release_gdk_visual_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkVisual *obj = (GdkVisual *)rsrc->ptr;
	gdk_visual_unref(obj);
}

static void gdk_visual_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GdkVisual *visual = PHP_GDK_VISUAL_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "type")) {
		ZVAL_LONG(return_value, visual->type);
	} else if (!strcmp(prop_name, "depth")) {
		ZVAL_LONG(return_value, visual->depth);
	} else if (!strcmp(prop_name, "byte_order")) {
		ZVAL_LONG(return_value, visual->byte_order);
	} else if (!strcmp(prop_name, "colormap_size")) {
		ZVAL_LONG(return_value, visual->colormap_size);
	} else if (!strcmp(prop_name, "bits_per_rgb")) {
		ZVAL_LONG(return_value, visual->bits_per_rgb);
	} else if (!strcmp(prop_name, "red_mask")) {
		ZVAL_LONG(return_value, visual->red_shift);
	} else if (!strcmp(prop_name, "red_shift")) {
		ZVAL_LONG(return_value, visual->red_prec);
	} else if (!strcmp(prop_name, "red_prec")) {
		ZVAL_LONG(return_value, visual->red_mask);
	} else if (!strcmp(prop_name, "green_mask")) {
		ZVAL_LONG(return_value, visual->green_mask);
	} else if (!strcmp(prop_name, "green_shift")) {
		ZVAL_LONG(return_value, visual->green_shift);
	} else if (!strcmp(prop_name, "green_prec")) {
		ZVAL_LONG(return_value, visual->green_prec);
	} else if (!strcmp(prop_name, "blue_mask")) {
		ZVAL_LONG(return_value, visual->blue_mask);
	} else if (!strcmp(prop_name, "blue_shift")) {
		ZVAL_LONG(return_value, visual->blue_shift);
	} else if (!strcmp(prop_name, "blue_prec")) {
		ZVAL_LONG(return_value, visual->blue_prec);
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}


/* GdkFont */
PHP_FUNCTION(gdk_font_width)
{
	char *text;
	int length;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s#", &text, &length))
		return;

	RETURN_LONG(gdk_text_width(PHP_GDK_FONT_GET(this_ptr), text, length));
}

PHP_FUNCTION(gdk_font_height)
{
	char *text;
	int length;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s#", &text, &length))
		return;

	RETURN_LONG(gdk_text_height(PHP_GDK_FONT_GET(this_ptr), text, length));
}

PHP_FUNCTION(gdk_font_measure)
{
	char *text;
	int length;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s#", &text, &length))
		return;

	RETURN_LONG(gdk_text_measure(PHP_GDK_FONT_GET(this_ptr), text, length));
}

PHP_FUNCTION(gdk_font_extents)
{
	char *text;
	int length, lbearing, rbearing, width, ascent, descent;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s#", &text, &length))
		return;

    gdk_text_extents(PHP_GDK_FONT_GET(this_ptr), text, length, &lbearing, &rbearing, &width, &ascent, &descent);
	*return_value = *php_gtk_build_value("(iiiii)", lbearing, rbearing, width, ascent, descent);
}

static function_entry php_gdk_font_functions[] = {
	{"gdkfont",	PHP_FN(wrap_no_direct_constructor), NULL},
	{"width", 	PHP_FN(gdk_font_width), NULL},
	{"height", 	PHP_FN(gdk_font_height), NULL},
	{"measure", PHP_FN(gdk_font_measure), NULL},
	{"extents",	PHP_FN(gdk_font_extents), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_font_new(GdkFont *font)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!font) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_font_ce);
	gdk_font_ref(font);
	php_gtk_set_object(result, font, le_gdk_font);

	return result;
}

static void release_gdk_font_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkFont *obj = (GdkFont *)rsrc->ptr;
	gdk_font_unref(obj);
}

static void gdk_font_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GdkFont *font = PHP_GDK_FONT_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "type")) {
		ZVAL_LONG(return_value, font->type);
	} else if (!strcmp(prop_name, "ascent")) {
		ZVAL_LONG(return_value, font->ascent);
	} else if (!strcmp(prop_name, "descent")) {
		ZVAL_LONG(return_value, font->descent);
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}


/* GdkGC */
PHP_FUNCTION(gdk_gc_set_dashes)
{
	gint dash_offset, length, i = 0;
	zval *php_list, **temp;
	guchar *dash_list;
	HashTable *hash;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "ia", &dash_offset, &php_list))
		return;

	hash = HASH_OF(php_list);
	length = zend_hash_num_elements(hash);
	dash_list = g_new(char, length);
	zend_hash_internal_pointer_reset(hash);
	while (zend_hash_get_current_data(hash, (void**)&temp) == SUCCESS) {
		if (Z_TYPE_PP(temp) != IS_LONG) {
			php_error(E_WARNING, "%s() expects array elements to be integers",
					  get_active_function_name()); 
			g_free(dash_list);
			return;
		}
		dash_list[i] = (guchar)Z_LVAL_PP(temp);
		if (dash_list[i] == 0) {
			php_error(E_WARNING, "%s() expects array elements to be non-zero",
					  get_active_function_name()); 
			g_free(dash_list);
			return;
		}
		zend_hash_move_forward(hash);
	}

	gdk_gc_set_dashes(PHP_GDK_GC_GET(this_ptr), dash_offset, dash_list, length);
	g_free(dash_list);
	RETURN_NULL();
}

static function_entry php_gdk_gc_functions[] = {
	{"gdkgc",		PHP_FN(wrap_no_direct_constructor), NULL},
	{"set_dashes",	PHP_FN(gdk_gc_set_dashes), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_gc_new(GdkGC *gc)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!gc) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_gc_ce);
	gdk_gc_ref(gc);
	php_gtk_set_object(result, gc, le_gdk_gc);

	return result;
}

static void release_gdk_gc_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkGC *obj = (GdkGC *)rsrc->ptr;
	gdk_gc_unref(obj);
}


static void gdk_colormap_query_color (GdkColormap *colormap, gulong pixel, GdkColor *result)
{
#ifndef PHP_WIN32
	XColor xcolor;
#endif
	GdkVisual *visual;

	g_return_if_fail (GDK_IS_COLORMAP (colormap));

	visual = gdk_colormap_get_visual (colormap);

	switch (visual->type) {
		case GDK_VISUAL_DIRECT_COLOR:
		case GDK_VISUAL_TRUE_COLOR:
			result->red = 65535. * (double)((pixel & visual->red_mask) >>
											visual->red_shift) / ((1 << visual->red_prec) - 1);
			result->green = 65535. * (double)((pixel & visual->green_mask) >>
											  visual->green_shift) / ((1 << visual->green_prec) - 1);
			result->blue = 65535. * (double)((pixel & visual->blue_mask) >>
											 visual->blue_shift) / ((1 << visual->blue_prec) - 1);
			break;
		case GDK_VISUAL_STATIC_GRAY:
		case GDK_VISUAL_GRAYSCALE:
			result->red = result->green = result->blue = 65535. *
				(double)pixel/((1<<visual->depth) - 1);
			break;
		case GDK_VISUAL_STATIC_COLOR:
#ifdef PHP_WIN32
			g_assert_not_reached ();
#else
			xcolor.pixel = pixel;
			XQueryColor(GDK_DISPLAY(), GDK_COLORMAP_XCOLORMAP(colormap), &xcolor);
			result->red = xcolor.red;
			result->green = xcolor.green;
			result->blue = xcolor.blue;
#endif
			break;
		case GDK_VISUAL_PSEUDO_COLOR:
			result->red = colormap->colors[pixel].red;
			result->green = colormap->colors[pixel].green;
			result->blue = colormap->colors[pixel].blue;
			break;
		default:
			g_assert_not_reached ();
			break;
	}
}

static void gdk_gc_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GdkGC *gc = PHP_GDK_GC_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);
	GdkGCValues gcv;

	ZVAL_NULL(return_value);

	gdk_gc_get_values(gc, &gcv);

	if (!strcmp(prop_name, "foreground")) {
		*return_value = *php_gdk_color_new(&gcv.foreground);
	} else if (!strcmp(prop_name, "background")) {
		*return_value = *php_gdk_color_new(&gcv.background);
	} else if (!strcmp(prop_name, "font")) {
		*return_value = *php_gdk_font_new(gcv.font);
	} else if (!strcmp(prop_name, "function")) {
		ZVAL_LONG(return_value, gcv.function);
	} else if (!strcmp(prop_name, "fill")) {
		ZVAL_LONG(return_value, gcv.fill);
	} else if (!strcmp(prop_name, "tile")) {
		if (gcv.tile)
			*return_value = *php_gdk_window_new(gcv.tile);
	} else if (!strcmp(prop_name, "stipple")) {
		if (gcv.stipple)
			*return_value = *php_gdk_window_new(gcv.stipple);
	} else if (!strcmp(prop_name, "clip_mask")) {
		if (gcv.clip_mask)
			*return_value = *php_gdk_window_new(gcv.clip_mask);
	} else if (!strcmp(prop_name, "subwindow_mode")) {
		ZVAL_LONG(return_value, gcv.subwindow_mode);
	} else if (!strcmp(prop_name, "ts_x_origin")) {
		ZVAL_LONG(return_value, gcv.ts_x_origin);
	} else if (!strcmp(prop_name, "ts_y_origin")) {
		ZVAL_LONG(return_value, gcv.ts_y_origin);
	} else if (!strcmp(prop_name, "clip_x_origin")) {
		ZVAL_LONG(return_value, gcv.clip_x_origin);
	} else if (!strcmp(prop_name, "clip_y_origin")) {
		ZVAL_LONG(return_value, gcv.clip_y_origin);
	} else if (!strcmp(prop_name, "graphics_exposures")) {
		ZVAL_LONG(return_value, gcv.graphics_exposures);
	} else if (!strcmp(prop_name, "line_width")) {
		ZVAL_LONG(return_value, gcv.line_width);
	} else if (!strcmp(prop_name, "line_style")) {
		ZVAL_LONG(return_value, gcv.line_style);
	} else if (!strcmp(prop_name, "cap_style")) {
		ZVAL_LONG(return_value, gcv.cap_style);
	} else if (!strcmp(prop_name, "join_style")) {
		ZVAL_LONG(return_value, gcv.join_style);
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}

static int gdk_gc_set_property(zval *object, zend_llist_element **element, zval *value)
{
	GdkGC *gc = PHP_GDK_GC_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	if (Z_TYPE_P(value) == IS_LONG) {
		int iv = Z_LVAL_P(value);
		GdkGCValues v;

		gdk_gc_get_values(gc, &v);
		if (!strcmp(prop_name, "function"))
			gdk_gc_set_function(gc, iv);
		else if (!strcmp(prop_name, "fill"))
			gdk_gc_set_fill(gc, iv);
		else if (!strcmp(prop_name, "subwindow_mode"))
			gdk_gc_set_subwindow(gc, iv);
		else if (!strcmp(prop_name, "ts_x_origin"))
			gdk_gc_set_ts_origin(gc, iv, v.ts_y_origin);
		else if (!strcmp(prop_name, "ts_y_origin"))
			gdk_gc_set_ts_origin(gc, v.ts_x_origin, iv);
		else if (!strcmp(prop_name, "clip_x_origin"))
			gdk_gc_set_clip_origin(gc, iv, v.clip_y_origin);
		else if (!strcmp(prop_name, "clip_y_origin"))
			gdk_gc_set_clip_origin(gc, v.clip_x_origin, iv);
		else if (!strcmp(prop_name, "graphics_exposures"))
			gdk_gc_set_exposures(gc, iv);
		else if (!strcmp(prop_name, "line_width"))
			gdk_gc_set_line_attributes(gc, iv, v.line_style, v.cap_style, v.join_style);
		else if (!strcmp(prop_name, "line_style"))
			gdk_gc_set_line_attributes(gc, v.line_width, iv, v.cap_style, v.join_style);
		else if (!strcmp(prop_name, "cap_style"))
			gdk_gc_set_line_attributes(gc, v.line_width, v.line_style, iv, v.join_style);
		else if (!strcmp(prop_name, "join_style"))
			gdk_gc_set_line_attributes(gc, v.line_width, v.line_style, v.cap_style, iv);
		else {
			php_error(E_WARNING, "'%s' property should be a long ", prop_name);
			return FAILURE;
		}
	} else if (php_gtk_check_class(value, gdk_color_ce)) {
		GdkColor *c = PHP_GDK_COLOR_GET(value);

		if (!strcmp(prop_name, "foreground"))
			gdk_gc_set_foreground(gc, c);
		else if (!strcmp(prop_name, "background")) {
			GdkGCValues gcv;
			gdk_gc_set_background(gc, c);
		}
		else {
			php_error(E_WARNING, "'%s' property should be a GdkColor", prop_name);
			return FAILURE;
		}
	} else if (php_gtk_check_class(value, gdk_font_ce)) {
		if (!strcmp(prop_name, "font"))
			gdk_gc_set_font(gc, PHP_GDK_FONT_GET(value));
		else {
			php_error(E_WARNING, "'%s' property should be a GdkFont", prop_name);
			return FAILURE;
		}
	} else if (php_gtk_check_class(value, gdk_window_ce) || Z_TYPE_P(value) == IS_NULL) {
		GdkWindow *w = (Z_TYPE_P(value) == IS_NULL) ? NULL : PHP_GDK_WINDOW_GET(value);
		
		if (!strcmp(prop_name, "tile"))
			gdk_gc_set_tile(gc, w);
		else if (!strcmp(prop_name, "stipple"))
			gdk_gc_set_stipple(gc, w);
		else if (!strcmp(prop_name, "clip_mask"))
			gdk_gc_set_clip_mask(gc, w);
		else {
			php_error(E_WARNING, "'%s' property should be a GdkWindow or null", prop_name);
			return FAILURE;
		}
	} else {
		php_error(E_WARNING, "type mismatch trying to set '%s' property", prop_name);
		return FAILURE;
	}

	return SUCCESS;
}


/* GdkDragContext */
static function_entry php_gdk_drag_context_functions[] = {
	{"gdkdragcontext", PHP_FN(wrap_no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_drag_context_new(GdkDragContext *context)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!context) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_drag_context_ce);
	gdk_drag_context_ref(context);
	php_gtk_set_object(result, context, le_gdk_drag_context);

	return result;
}

static void release_gdk_drag_context_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkDragContext *obj = (GdkDragContext *)rsrc->ptr;
	gdk_drag_context_unref(obj);
}

static void gdk_drag_context_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GdkDragContext *context = PHP_GDK_DRAG_CONTEXT_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "protocol")) {
		ZVAL_LONG(return_value, context->protocol);
	} else if (!strcmp(prop_name, "is_source")) {
		ZVAL_BOOL(return_value, context->is_source);
	} else if (!strcmp(prop_name, "source_window")) {
		if (context->source_window)
			*return_value = *php_gdk_window_new(context->source_window);
	} else if (!strcmp(prop_name, "dest_window")) {
		if (context->dest_window)
			*return_value = *php_gdk_window_new(context->dest_window);
	} else if (!strcmp(prop_name, "targets")) {
		GList *tmp;

		array_init(return_value);
		for (tmp = context->targets; tmp; tmp = tmp->next)
			add_next_index_zval(return_value, php_gdk_atom_new(GPOINTER_TO_INT(tmp->data)));
	} else if (!strcmp(prop_name, "actions")) {
		ZVAL_LONG(return_value, context->actions);
	} else if (!strcmp(prop_name, "suggested_action")) {
		ZVAL_LONG(return_value, context->suggested_action);
	} else if (!strcmp(prop_name, "action")) {
		ZVAL_LONG(return_value, context->action);
	} else if (!strcmp(prop_name, "start_time")) {
		ZVAL_LONG(return_value, context->start_time);
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}


/* GtkSelectionData */
PHP_FUNCTION(gtk_selection_data_set)
{
	GdkAtom type;
	int format, length;
	guchar *data;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "iis#", &type, &format, &data, &length))
		return;

	gtk_selection_data_set(PHP_GTK_SELECTION_DATA_GET(this_ptr), type, format, data, length);
	RETURN_NULL();
}

static function_entry php_gtk_selection_data_functions[] = {
	{"gdkselectiondata", PHP_FN(wrap_no_direct_constructor), NULL},
	{"set", 			 PHP_FN(gtk_selection_data_set), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_selection_data_new(GtkSelectionData *data)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!data) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gtk_selection_data_ce);
	php_gtk_set_object(result, data, le_gtk_selection_data);

	return result;
}

static void gtk_selection_data_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GtkSelectionData *data = PHP_GTK_SELECTION_DATA_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "selection")) {
		*return_value = *php_gdk_atom_new(data->selection);
	} else if (!strcmp(prop_name, "target")) {
		*return_value = *php_gdk_atom_new(data->target);
	} else if (!strcmp(prop_name, "type")) {
		*return_value = *php_gdk_atom_new(data->type);
	} else if (!strcmp(prop_name, "format")) {
		ZVAL_LONG(return_value, data->format);
	} else if (!strcmp(prop_name, "length")) {
		ZVAL_LONG(return_value, data->length);
	} else if (!strcmp(prop_name, "data") && data->length > -1) {
		ZVAL_STRINGL(return_value, data->data, data->length, 1);
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}

/* GtkCtreeNode */
static function_entry php_gtk_ctree_node_functions[] = {
	{"gtkctreenode", PHP_FN(wrap_no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_ctree_node_new(GtkCTreeNode *node)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!node) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gtk_ctree_node_ce);
	php_gtk_set_object(result, node, le_gtk_ctree_node);

	return result;
}

static void gtk_ctree_node_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GtkCTreeNode *node = PHP_GTK_CTREE_NODE_GET(object);
	GtkCTreeNode *temp;
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "parent")) {
		temp = GTK_CTREE_ROW(node)->parent;
		if (temp)
			*return_value = *php_gtk_ctree_node_new(temp);
	} else if (!strcmp(prop_name, "sibling")) {
		temp = GTK_CTREE_ROW(node)->sibling;
		if (temp)
			*return_value = *php_gtk_ctree_node_new(temp);
	} else if (!strcmp(prop_name, "children")) {
		zval *php_node;
		temp = GTK_CTREE_ROW(node)->children;

		array_init(return_value);
		while (temp) {
			php_node = php_gtk_ctree_node_new(temp);
			add_next_index_zval(return_value, php_node);
			zval_ptr_dtor(&php_node);
			temp = GTK_CTREE_ROW(temp)->sibling;
		}
	} else if (!strcmp(prop_name, "level")) {
		ZVAL_LONG(return_value, GTK_CTREE_ROW(node)->level);
	} else if (!strcmp(prop_name, "is_leaf")) {
		ZVAL_BOOL(return_value, GTK_CTREE_ROW(node)->is_leaf);
	} else if (!strcmp(prop_name, "expanded")) {
		ZVAL_BOOL(return_value, GTK_CTREE_ROW(node)->expanded);
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}


/* GtkAccelGroup */
PHP_FUNCTION(gtkaccelgroup)
{
	GtkAccelGroup *accel_group;
	
	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "")) {
		php_gtk_invalidate(this_ptr);
		return;
	}

	accel_group = gtk_accel_group_new();
	php_gtk_set_object(this_ptr, accel_group, le_gtk_accel_group);
}

PHP_FUNCTION(gtk_accel_group_lock)
{
	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	gtk_accel_group_lock(PHP_GTK_ACCEL_GROUP_GET(this_ptr));
}

PHP_FUNCTION(gtk_accel_group_unlock)
{
	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	gtk_accel_group_unlock(PHP_GTK_ACCEL_GROUP_GET(this_ptr));
}

static function_entry php_gtk_accel_group_functions[] = {
	{"gtkaccelgroup",	PHP_FN(gtkaccelgroup), NULL},
	{"lock", 			PHP_FN(gtk_accel_group_lock), NULL},
	{"unlock", 			PHP_FN(gtk_accel_group_unlock), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_accel_group_new(GtkAccelGroup *group)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!group) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gtk_accel_group_ce);
	gtk_accel_group_ref(group);
	php_gtk_set_object(result, group, le_gtk_accel_group);

	return result;
}

static void release_gtk_accel_group_rsrc(zend_rsrc_list_entry *rsrc)
{
	GtkAccelGroup *group = (GtkAccelGroup *)rsrc->ptr;
	gtk_accel_group_unref(group);
}


/* GtkStyle */
PHP_FUNCTION(gtkstyle)
{
	GtkStyle *ret;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	ret = gtk_style_new();
	*return_value = *php_gtk_style_new(ret);
}

PHP_FUNCTION(gtk_style_copy)
{
	GtkStyle *style;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	style = gtk_style_copy(PHP_GTK_STYLE_GET(this_ptr));
	*return_value = *php_gtk_style_new(style);
	gtk_style_unref(style);
}

static function_entry php_gtk_style_functions[] = {
	{"gtkstyle", PHP_FN(gtkstyle), NULL},
	{"copy",	 PHP_FN(gtk_style_copy), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_style_new(GtkStyle *style)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!style) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gtk_style_ce);
	gtk_style_ref(style);
	php_gtk_set_object(result, style, le_gtk_style);

	return result;
}

static void release_gtk_style_rsrc(zend_rsrc_list_entry *rsrc)
{
	GtkStyle *style = (GtkStyle *)rsrc->ptr;
	gtk_style_unref(style);
}

typedef enum {
	STYLE_COLOR_ARRAY,
	STYLE_GC_ARRAY,
	STYLE_PIXMAP_ARRAY
} style_array_type;

static void style_helper_get(zval *result, style_array_type type, gpointer array, zend_llist_element **element)
{
	zend_overloaded_element *property;
	zend_llist_element *next = (*element)->next;
	int prop_index;

	if (next) {
		property = (zend_overloaded_element *)next->data;
		if (Z_TYPE_P(property) == OE_IS_ARRAY && Z_TYPE(property->element) == IS_LONG) {
			*element = next;
			prop_index = Z_LVAL(property->element);
			if (prop_index < 0 || prop_index >= 5) {
				php_error(E_WARNING, "style index out of range");
				return;
			}

			switch (type) {
				case STYLE_COLOR_ARRAY:
					{
						GdkColor *color_array = (GdkColor *)array;
						*result = *php_gdk_color_new(&color_array[prop_index]);
					}
					return;

				case STYLE_GC_ARRAY:
					{
						GdkGC **gc_array = (GdkGC **)array;
						*result = *php_gdk_gc_new(gc_array[prop_index]);
					}
					return;

				case STYLE_PIXMAP_ARRAY:
					{
						GdkWindow **pixmap_array = (GdkPixmap **)array;
						if (pixmap_array[prop_index])
							*result = *php_gdk_window_new(pixmap_array[prop_index]);
					}
					return;

				default:
					g_assert_not_reached();
					break;
			}
		}
	} else {
		int i;

		array_init(result);
		switch (type) {
			case STYLE_COLOR_ARRAY:
				{
					GdkColor *color_array = (GdkColor *)array;
					for (i = 0; i < 5; i++)
						add_next_index_zval(result, php_gdk_color_new(&color_array[i]));
				}
				break;

			case STYLE_GC_ARRAY:
				{
					GdkGC **gc_array = (GdkGC **)array;
					for (i = 0; i < 5; i++)
						add_next_index_zval(result, php_gdk_gc_new(gc_array[i]));
				}
				break;

			case STYLE_PIXMAP_ARRAY:
				{
					GdkWindow **pixmap_array = (GdkWindow **)array;
					for (i = 0; i < 5; i++)
						add_next_index_zval(result, php_gdk_window_new(pixmap_array[i]));
				}
				break;

			default:
				g_assert_not_reached();
				break;
		}
	}
}

static void gtk_style_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	GtkStyle *style = PHP_GTK_STYLE_GET(object);
	zend_overloaded_element *property = (zend_overloaded_element *)(*element)->data;
	char *prop_name = Z_STRVAL(property->element);

	ZVAL_NULL(return_value);

	if (!strcmp(prop_name, "black")) {
		*return_value = *php_gdk_color_new(&style->black);
	} else if (!strcmp(prop_name, "white")) {
		*return_value = *php_gdk_color_new(&style->white);
	} else if (!strcmp(prop_name, "font")) {
		*return_value = *php_gdk_font_new(style->font);
	} else if (!strcmp(prop_name, "black_gc")) {
		*return_value = *php_gdk_gc_new(style->black_gc);
	} else if (!strcmp(prop_name, "white_gc")) {
		*return_value = *php_gdk_gc_new(style->white_gc);
	} else if (!strcmp(prop_name, "colormap")) {
		if (style->colormap)
			*return_value = *php_gdk_colormap_new(style->colormap);
	} else if (!strcmp(prop_name, "fg")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->fg, element);
	} else if (!strcmp(prop_name, "bg")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->bg, element);
	} else if (!strcmp(prop_name, "light")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->light, element);
	} else if (!strcmp(prop_name, "dark")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->dark, element);
	} else if (!strcmp(prop_name, "mid")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->mid, element);
	} else if (!strcmp(prop_name, "text")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->text, element);
	} else if (!strcmp(prop_name, "base")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->base, element);
	} else if (!strcmp(prop_name, "fg_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->fg_gc, element);
	} else if (!strcmp(prop_name, "bg_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->bg_gc, element);
	} else if (!strcmp(prop_name, "light_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->light_gc, element);
	} else if (!strcmp(prop_name, "dark_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->dark_gc, element);
	} else if (!strcmp(prop_name, "mid_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->mid_gc, element);
	} else if (!strcmp(prop_name, "text_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->text_gc, element);
	} else if (!strcmp(prop_name, "base_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->base_gc, element);
	} else if (!strcmp(prop_name, "bg_pixmap")) {
		style_helper_get(return_value, STYLE_PIXMAP_ARRAY, style->bg_pixmap, element);
	} else {
		*found = FAILURE;
	}

	*found = SUCCESS;
}

static int style_helper_set(style_array_type type, gpointer array, zval *value, zend_llist_element **element)
{
	zend_overloaded_element *property;
	zend_llist_element *next = (*element)->next;
	int prop_index;

	if (next) {
		property = (zend_overloaded_element *)next->data;
		if (Z_TYPE_P(property) == OE_IS_ARRAY && Z_TYPE(property->element) == IS_LONG) {
			*element = next;
			prop_index = Z_LVAL(property->element);
			if (prop_index < 0 || prop_index >= 5) {
				php_error(E_WARNING, "style index out of range");
				return FAILURE;
			}

			switch (type) {
				case STYLE_COLOR_ARRAY:
					{
						GdkColor *color_array = (GdkColor *)array;
						if (!php_gtk_check_class(value, gdk_color_ce)) {
							php_error(E_WARNING, "can only assign a GdkColor");
							return FAILURE;
						}
						color_array[prop_index] = *PHP_GDK_COLOR_GET(value);
					}
					break;

				case STYLE_GC_ARRAY:
					{
						GdkGC **gc_array = (GdkGC **)array;
						if (!php_gtk_check_class(value, gdk_gc_ce)) {
							php_error(E_WARNING, "can only assign a GdkGC");
							return FAILURE;
						}
						if (gc_array[prop_index])
							gdk_gc_unref(gc_array[prop_index]);
						gc_array[prop_index] = gdk_gc_ref(PHP_GDK_GC_GET(value));
					}
					break;

				case STYLE_PIXMAP_ARRAY:
					{
						GdkWindow **pixmap_array = (GdkWindow **)array;
						if (!php_gtk_check_class(value, gdk_window_ce) && Z_TYPE_P(value) != IS_NULL) {
							php_error(E_WARNING, "can only assign a GdkPixmap or null");
							return FAILURE;
						}
						if (pixmap_array[prop_index])
							gdk_pixmap_unref(pixmap_array[prop_index]);
						if (Z_TYPE_P(value) != IS_NULL)
							pixmap_array[prop_index] = gdk_pixmap_ref(PHP_GDK_WINDOW_GET(value));
						else
							pixmap_array[prop_index] = NULL;
					}
					break;

				default:
					g_assert_not_reached();
					break;
			}
		} else
			return FAILURE;
	} else
		return FAILURE;

	return SUCCESS;
}

static int gtk_style_set_property(zval *object, zend_llist_element **element, zval *value)
{
	GtkStyle *style = PHP_GTK_STYLE_GET(object);
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	if (!strcmp(prop_name, "font")) {
		if (php_gtk_check_class(value, gdk_font_ce)) {
			if (style->font)
				gdk_font_unref(style->font);
			style->font = gdk_font_ref(PHP_GDK_FONT_GET(value));
		} else {
			php_error(E_WARNING, "'%s' property should be a GdkFont", prop_name);
			return FAILURE;
		}
	} else if (!strcmp(prop_name, "black")) {
		if (php_gtk_check_class(value, gdk_color_ce))
			style->black = *PHP_GDK_COLOR_GET(value);
		else {
			php_error(E_WARNING, "'%s' property should be a GdkColor", prop_name);
			return FAILURE;
		}
	} else if (!strcmp(prop_name, "white")) {
		if (php_gtk_check_class(value, gdk_color_ce))
			style->white = *PHP_GDK_COLOR_GET(value);
		else {
			php_error(E_WARNING, "'%s' property should be a GdkColor", prop_name);
			return FAILURE;
		}
	} else if (!strcmp(prop_name, "black_gc")) {
		if (php_gtk_check_class(value, gdk_gc_ce)) {
			if (style->black_gc)
				gdk_gc_unref(style->black_gc);
			style->black_gc = gdk_gc_ref(PHP_GDK_GC_GET(value));
		} else {
			php_error(E_WARNING, "'%s' property should be a GdkGC", prop_name);
			return FAILURE;
		}
	} else if (!strcmp(prop_name, "white_gc")) {
		if (php_gtk_check_class(value, gdk_gc_ce)) {
			if (style->white_gc)
				gdk_gc_unref(style->white_gc);
			style->white_gc = gdk_gc_ref(PHP_GDK_GC_GET(value));
		} else {
			php_error(E_WARNING, "'%s' property should be a GdkGC", prop_name);
			return FAILURE;
		}
	} else if (!strcmp(prop_name, "colormap")) {
		if (php_gtk_check_class(value, gdk_colormap_ce)) {
			if (style->colormap)
				gdk_colormap_unref(style->colormap);
			style->colormap = gdk_colormap_ref(PHP_GDK_COLORMAP_GET(value));
		} else {
			php_error(E_WARNING, "'%s' property should be a GdkColormap", prop_name);
			return FAILURE;
		}
	} else if (!strcmp(prop_name, "fg")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->fg, value, element);
	} else if (!strcmp(prop_name, "bg")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->bg, value, element);
	} else if (!strcmp(prop_name, "light")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->light, value, element);
	} else if (!strcmp(prop_name, "dark")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->dark, value, element);
	} else if (!strcmp(prop_name, "mid")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->mid, value, element);
	} else if (!strcmp(prop_name, "text")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->text, value, element);
	} else if (!strcmp(prop_name, "base")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->base, value, element);
	} else if (!strcmp(prop_name, "fg_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->fg_gc, value, element);
	} else if (!strcmp(prop_name, "bg_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->bg_gc, value, element);
	} else if (!strcmp(prop_name, "light_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->light_gc, value, element);
	} else if (!strcmp(prop_name, "dark_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->dark_gc, value, element);
	} else if (!strcmp(prop_name, "mid_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->mid_gc, value, element);
	} else if (!strcmp(prop_name, "text_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->text_gc, value, element);
	} else if (!strcmp(prop_name, "base_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->base_gc, value, element);
	} else if (!strcmp(prop_name, "bg_pixmap")) {
		return style_helper_set(STYLE_PIXMAP_ARRAY, style->bg_pixmap, value, element);
	} else {
		return FAILURE;
	}

	return SUCCESS;
}


void php_gtk_register_types(int module_number)
{
	zend_class_entry ce;

	le_gdk_event = zend_register_list_destructors_ex(NULL, NULL, "GdkEvent", module_number);
	le_gdk_window = zend_register_list_destructors_ex(release_gdk_window_rsrc, NULL, "GdkWindow", module_number);
	le_gdk_color = zend_register_list_destructors_ex(release_gdk_color_rsrc, NULL, "GdkColor", module_number);
	le_gdk_colormap = zend_register_list_destructors_ex(release_gdk_colormap_rsrc, NULL, "GdkColormap", module_number);
	le_gdk_cursor = zend_register_list_destructors_ex(release_gdk_cursor_rsrc, NULL, "GdkCursor", module_number);
	le_gdk_visual = zend_register_list_destructors_ex(release_gdk_visual_rsrc, NULL, "GdkVisual", module_number);
	le_gdk_font = zend_register_list_destructors_ex(release_gdk_font_rsrc, NULL, "GdkFont", module_number);
	le_gdk_gc = zend_register_list_destructors_ex(release_gdk_gc_rsrc, NULL, "GdkGC", module_number);
	le_gdk_drag_context = zend_register_list_destructors_ex(release_gdk_drag_context_rsrc, NULL, "GdkDragContext", module_number);
	le_gtk_selection_data = zend_register_list_destructors_ex(NULL, NULL, "GtkSelectionData", module_number);
	le_gtk_ctree_node = zend_register_list_destructors_ex(NULL, NULL, "GtkCTreeNode", module_number);
	le_gtk_accel_group = zend_register_list_destructors_ex(release_gtk_accel_group_rsrc, NULL, "GtkAccelGroup", module_number);
	le_gtk_style = zend_register_list_destructors_ex(release_gtk_style_rsrc, NULL, "GtkStyle", module_number);


	INIT_CLASS_ENTRY(ce, "gdkevent", php_gdk_event_functions);
	gdk_event_ce = zend_register_internal_class_ex(&ce, NULL, NULL);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkwindow", php_gdk_window_functions, NULL, php_gtk_get_property, NULL);
	gdk_window_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkpixmap", php_gdk_pixmap_functions, NULL, php_gtk_get_property, NULL);
	gdk_pixmap_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkbitmap", php_gdk_window_functions, NULL, php_gtk_get_property, NULL);
	gdk_bitmap_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gdk_window_ce, gdk_window_get_property);
	php_gtk_register_prop_getter(gdk_pixmap_ce, gdk_window_get_property);
	php_gtk_register_prop_getter(gdk_bitmap_ce, gdk_window_get_property);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkcolor", php_gdk_color_functions, NULL, php_gtk_get_property, php_gtk_set_property);
	gdk_color_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gdk_color_ce, gdk_color_get_property);
	php_gtk_register_prop_setter(gdk_color_ce, gdk_color_set_property);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkcolormap", php_gdk_colormap_functions, NULL, php_gtk_get_property, php_gtk_set_property);
	gdk_colormap_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gdk_colormap_ce, gdk_colormap_get_property);

	INIT_CLASS_ENTRY(ce, "gdkatom", php_gdk_atom_functions);
	gdk_atom_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	
	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkcursor", php_gdk_cursor_functions, NULL, php_gtk_get_property, NULL);
	gdk_cursor_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gdk_cursor_ce, gdk_cursor_get_property);
	
	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkvisual", php_gdk_visual_functions, NULL, php_gtk_get_property, NULL);
	gdk_visual_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gdk_visual_ce, gdk_visual_get_property);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkfont", php_gdk_font_functions, NULL, php_gtk_get_property, NULL);
	gdk_font_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gdk_font_ce, gdk_font_get_property);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkgc", php_gdk_gc_functions, NULL, php_gtk_get_property, php_gtk_set_property);
	gdk_gc_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gdk_gc_ce, gdk_gc_get_property);
	php_gtk_register_prop_setter(gdk_gc_ce, gdk_gc_set_property);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkdragcontext", php_gdk_drag_context_functions, NULL, php_gtk_get_property, NULL);
	gdk_drag_context_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gdk_drag_context_ce, gdk_drag_context_get_property);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gtkselectiondata", php_gtk_selection_data_functions, NULL, php_gtk_get_property, NULL);
	gtk_selection_data_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gtk_selection_data_ce, gtk_selection_data_get_property);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gtkctreenode", php_gtk_ctree_node_functions, NULL, php_gtk_get_property, NULL);
	gtk_ctree_node_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gtk_ctree_node_ce, gtk_ctree_node_get_property);

	INIT_CLASS_ENTRY(ce, "gtkaccelgroup", php_gtk_accel_group_functions);
	gtk_accel_group_ce = zend_register_internal_class_ex(&ce, NULL, NULL);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gtkstyle", php_gtk_style_functions, NULL, php_gtk_get_property, php_gtk_set_property);
	gtk_style_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	php_gtk_register_prop_getter(gtk_style_ce, gtk_style_get_property);
	php_gtk_register_prop_setter(gtk_style_ce, gtk_style_set_property);
}

#endif
