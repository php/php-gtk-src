/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001 Andrei Zmievski <andrei@ispi.net>
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

int le_gdk_event;
int le_gdk_window;
int le_gdk_color;
int le_gdk_colormap;
int le_gdk_cursor;
int le_gdk_visual;

zend_class_entry *gdk_event_ce;
zend_class_entry *gdk_window_ce;
zend_class_entry *gdk_color_ce;
zend_class_entry *gdk_colormap_ce;
zend_class_entry *gdk_atom_ce;
zend_class_entry *gdk_cursor_ce;
zend_class_entry *gdk_visual_ce;

/* GdkEvent */
zval *php_gdk_event_new(GdkEvent *obj)
{
	zval *result;
	zval *value;

	MAKE_STD_ZVAL(result);

	if (!obj) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_event_ce);

	php_gtk_set_object(result, obj, le_gdk_event);

	add_property_long(result, "type", obj->type);
	
	if (obj->any.window) {
		value = php_gdk_window_new(obj->any.window);
		add_property_zval(result, "window", value);
	} else
		add_property_null(result, "window");
	add_property_bool(result, "send_event", obj->any.send_event);

	switch (obj->type) {
		case GDK_NOTHING:
		case GDK_DELETE:
		case GDK_DESTROY:
			break;

		case GDK_EXPOSE:
			value = php_gtk_build_value("(iiii)",
										obj->expose.area.x,
										obj->expose.area.y,
										obj->expose.area.width,
										obj->expose.area.height);
			zend_hash_update(Z_OBJPROP_P(result), "area", sizeof("area"), &value, sizeof(zval *), NULL);
			add_property_long(result, "count", obj->expose.count);
			break;

		case GDK_MOTION_NOTIFY:
			add_property_long(result, "time", obj->motion.time);
			add_property_double(result, "x", obj->motion.x);
			add_property_double(result, "y", obj->motion.y);
			add_property_double(result, "pressure", obj->motion.pressure);
			add_property_double(result, "xtilt", obj->motion.xtilt);
			add_property_double(result, "ytilt", obj->motion.ytilt);
			add_property_long(result, "state", obj->motion.state);
			add_property_long(result, "is_hint", obj->motion.is_hint);
			add_property_long(result, "source", obj->motion.source);
			add_property_long(result, "deviceid", obj->motion.deviceid);
			add_property_double(result, "x_root", obj->motion.x_root);
			add_property_double(result, "y_root", obj->motion.y_root);
			break;

		case GDK_BUTTON_PRESS:      /*GdkEventButton            button*/
		case GDK_2BUTTON_PRESS:     /*GdkEventButton            button*/
		case GDK_3BUTTON_PRESS:     /*GdkEventButton            button*/
		case GDK_BUTTON_RELEASE:    /*GdkEventButton            button*/
			add_property_long(result, "time", obj->button.time);
			add_property_double(result, "x", obj->button.x);
			add_property_double(result, "y", obj->button.y);
			add_property_double(result, "pressure", obj->button.pressure);
			add_property_double(result, "xtilt", obj->button.xtilt);
			add_property_double(result, "ytilt", obj->button.ytilt);
			add_property_long(result, "state", obj->button.state);
			add_property_long(result, "button", obj->button.button);
			add_property_long(result, "source", obj->button.source);
			add_property_long(result, "deviceid", obj->button.deviceid);
			add_property_double(result, "x_root", obj->button.x_root);
			add_property_double(result, "y_root", obj->button.y_root);
			break;

		case GDK_KEY_PRESS:
		case GDK_KEY_RELEASE:
			add_property_long(result, "time", obj->key.time);
			add_property_long(result, "state", obj->key.state);
			add_property_long(result, "keyval", obj->key.keyval);
			add_property_stringl(result, "string", obj->key.string, obj->key.length, 1);
			break;

		case GDK_ENTER_NOTIFY:
		case GDK_LEAVE_NOTIFY:
			if (obj->crossing.subwindow) {
				value = php_gdk_window_new(obj->crossing.subwindow);
				add_property_zval(result, "subwindow", value);
			} else
				add_property_null(result, "subwindow");
			add_property_long(result, "time", obj->crossing.time);
			add_property_double(result, "x", obj->crossing.x);
			add_property_double(result, "y", obj->crossing.y);
			add_property_double(result, "x_root", obj->crossing.x_root);
			add_property_double(result, "y_root", obj->crossing.y_root);
			add_property_long(result, "mode", obj->crossing.mode);
			add_property_long(result, "detail", obj->crossing.detail);
			add_property_bool(result, "focus", obj->crossing.focus);
			add_property_long(result, "state", obj->crossing.state);
			break;

		case GDK_FOCUS_CHANGE:
			add_property_bool(result, "in", obj->focus_change.in);
			break;

		case GDK_CONFIGURE:
			add_property_long(result, "x", obj->configure.x);
			add_property_long(result, "y", obj->configure.y);
			add_property_long(result, "width", obj->configure.width);
			add_property_long(result, "height", obj->configure.height);
			break;

		case GDK_MAP:
		case GDK_UNMAP:
			break;

		case GDK_PROPERTY_NOTIFY:
			add_property_zval(result, "atom", php_gdk_atom_new(obj->property.atom));
			add_property_long(result, "time", obj->property.time);
			add_property_long(result, "state", obj->property.state);
			break;

		case GDK_SELECTION_CLEAR:
		case GDK_SELECTION_REQUEST:
		case GDK_SELECTION_NOTIFY:
			add_property_zval(result, "selection", php_gdk_atom_new(obj->selection.selection));
			add_property_zval(result, "target", php_gdk_atom_new(obj->selection.target));
			add_property_zval(result, "property", php_gdk_atom_new(obj->selection.property));
			add_property_long(result, "requestor", obj->selection.requestor);
			add_property_long(result, "time", obj->selection.time);
			break;

		case GDK_PROXIMITY_IN:
		case GDK_PROXIMITY_OUT:
			add_property_long(result, "time", obj->proximity.time);
			add_property_long(result, "source", obj->proximity.source);
			add_property_long(result, "deviceid", obj->proximity.deviceid);
			break;

		case GDK_DRAG_ENTER:
		case GDK_DRAG_LEAVE:
		case GDK_DRAG_MOTION:
		case GDK_DRAG_STATUS:
		case GDK_DROP_START:
		case GDK_DROP_FINISHED:
			/* TODO add context property */
			add_property_long(result, "time", obj->dnd.time);
			add_property_long(result, "x_root", obj->dnd.x_root);
			add_property_long(result, "y_root", obj->dnd.y_root);
			break;

		case GDK_CLIENT_EVENT:
			add_property_zval(result, "message_type", php_gdk_atom_new(obj->client.message_type));
			add_property_long(result, "data_format", obj->client.data_format);
			add_property_stringl(result, "data", obj->client.data.b, 20, 1);
			break;

		case GDK_VISIBILITY_NOTIFY:
			add_property_long(result, "state", obj->visibility.state);
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
	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	gdk_window_raise(PHP_GDK_WINDOW_GET(this_ptr));
}

PHP_FUNCTION(gdk_window_lower)
{
	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	gdk_window_lower(PHP_GDK_WINDOW_GET(this_ptr));
}

PHP_FUNCTION(gdk_window_get_pointer)
{
	guint32 deviceid;
    gdouble x = 0.0, y = 0.0, pressure = 0.0, xtilt = 0.0, ytilt = 0.0;
    GdkModifierType mask = 0;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &deviceid))
		return;

    gdk_input_window_get_pointer(PHP_GDK_WINDOW_GET(this_ptr), deviceid, &x, &y, &pressure, &xtilt, &ytilt, &mask);
    *return_value = *php_gtk_build_value("(dddddi)", x, y, pressure, xtilt, ytilt, mask);
}

PHP_FUNCTION(gdk_window_set_cursor)
{
	zval *cursor;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &cursor, gdk_cursor_ce))
		return;

	gdk_window_set_cursor(PHP_GDK_WINDOW_GET(this_ptr), PHP_GDK_CURSOR_GET(cursor));
}

static function_entry php_gdk_window_functions[] = {
	/* TODO implement other functions similar to PyGtk */
	{"raise", PHP_FN(gdk_window_raise), NULL},
	{"lower", PHP_FN(gdk_window_lower), NULL},
	{"get_pointer", PHP_FN(gdk_window_get_pointer), NULL},
	{"set_cursor", PHP_FN(gdk_window_set_cursor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_window_new(GdkWindow *obj)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!obj) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_window_ce);

	gdk_window_ref(obj);
	php_gtk_set_object(result, obj, le_gdk_window);

	return result;
}

static void gdk_window_get_property(zval *result, zval *object, zval *property)
{
	zval *value;
	GdkWindow *win = PHP_GDK_WINDOW_GET(object);
	gint x, y;
	GdkModifierType p_mask;
	char *prop_name;

	ZVAL_NULL(result);
	if (Z_TYPE_P(property) == IS_LONG)
		return;
	else
		prop_name = Z_STRVAL_P(property);
	
	if (!strcmp(prop_name, "width")) {
		gdk_window_get_size(win, &x, NULL);
		ZVAL_LONG(result, x);
	} else if (!strcmp(prop_name, "height")) {
		gdk_window_get_size(win, NULL, &y);
		ZVAL_LONG(result, y);
	} else if (!strcmp(prop_name, "x")) {
		gdk_window_get_position(win, &x, NULL);
		ZVAL_LONG(result, x);
	} else if (!strcmp(prop_name, "y")) {
		gdk_window_get_position(win, NULL, &y);
		ZVAL_LONG(result, y);
	} else if (!strcmp(prop_name, "colormap")) {
		*result = *php_gdk_colormap_new(gdk_window_get_colormap(win));
	} else if (!strcmp(prop_name, "pointer")) {
		gdk_window_get_pointer(win, &x, &y, NULL);
		*result = *php_gtk_build_value("(ii)", x, y);
	} else if (!strcmp(prop_name, "pointer_state")) {
		gdk_window_get_pointer(win, NULL, NULL, &p_mask);
		ZVAL_LONG(result, p_mask);
	} else if (!strcmp(prop_name, "parent")) {
		GdkWindow *parent = gdk_window_get_parent(win);
		if (parent)
			*result = *php_gdk_window_new(parent);
	} else if (!strcmp(prop_name, "toplevel")) {
		*result = *php_gdk_window_new(gdk_window_get_toplevel(win));
	} else if (!strcmp(prop_name, "children")) {
		GList *children, *tmp;
		zval *child;
		children = gdk_window_get_children(win);
		array_init(result);
		for (tmp = children; tmp; tmp = tmp->next) {
			child = php_gdk_window_new(tmp->data);
			zend_hash_next_index_insert(Z_ARRVAL_P(result), &child, sizeof(zval *), NULL);
		}
		g_list_free(children);
	} else if (!strcmp(prop_name, "type")) {
		ZVAL_LONG(result, gdk_window_get_type(win));
	} else if (!strcmp(prop_name, "depth")) {
		gdk_window_get_geometry(win, NULL, NULL, NULL, NULL, &x);
		ZVAL_LONG(result, x);
	}
#ifdef WITH_XSTUFF
	else if (!strcmp(prop_name, "xid")) {
		ZVAL_LONG(result, GDK_WINDOW_XWINDOW(win));
	}
#endif
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

zval *php_gdk_color_new(GdkColor *obj)
{
	zval *result;
	GdkColor *result_obj;

	MAKE_STD_ZVAL(result);

	if (!obj) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_color_ce);

	result_obj = gdk_color_copy(obj);
	php_gtk_set_object(result, result_obj, le_gdk_color);

	return result;
}

static void gdk_color_get_property(zval *result, zval *object, zval *property)
{
	GdkColor *color = PHP_GDK_COLOR_GET(object);
	char *prop_name;

	ZVAL_NULL(result);
	if (Z_TYPE_P(property) == IS_LONG)
		return;
	else
		prop_name = Z_STRVAL_P(property);


	if (!strcmp(prop_name, "red")) {
		ZVAL_LONG(result, color->red);
	} else if (!strcmp(prop_name, "green")) {
		ZVAL_LONG(result, color->green);
	} else if (!strcmp(prop_name, "blue")) {
		ZVAL_LONG(result, color->blue);
	} else if (!strcmp(prop_name, "pixel")) {
		ZVAL_LONG(result, color->pixel);
	}
}

static void gdk_color_set_property(zval *object, char *prop_name, zval *value)
{
	GdkColor *color = PHP_GDK_COLOR_GET(object);

	convert_to_long(value);
	if (!strcmp(prop_name, "red")) {
		color->red = (gushort)Z_LVAL_P(value);
	} else if (!strcmp(prop_name, "green")) {
		color->green = (gushort)Z_LVAL_P(value);
	} else if (!strcmp(prop_name, "blue")) {
		color->blue = (gushort)Z_LVAL_P(value);
	}
}

static void release_gdk_color_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkColor *obj = (GdkColor *)rsrc->ptr;
	gdk_color_free(obj);
}


/* GdkColormap */
PHP_FUNCTION(gdk_colormap_size)
{
	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	ZVAL_LONG(return_value, PHP_GDK_COLORMAP_GET(this_ptr)->size);
}

PHP_FUNCTION(gdk_colormap_alloc)
{
	gchar *color_spec;
    GdkColor color = {0, 0, 0, 0};
	zend_bool writeable = 0, best_match = 1;

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
	{"size", 	PHP_FN(gdk_colormap_size), NULL},
	{"alloc",	PHP_FN(gdk_colormap_alloc), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_colormap_new(GdkColormap *obj)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!obj) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_colormap_ce);

	gdk_colormap_ref(obj);
	php_gtk_set_object(result, obj, le_gdk_colormap);

	return result;
}

static void release_gdk_colormap_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkColormap *obj = (GdkColormap *)rsrc->ptr;
	gdk_colormap_unref(obj);
}

static void gdk_colormap_get_property(zval *result, zval *object, zval *property)
{
	GdkColormap *cmap = PHP_GDK_COLORMAP_GET(object);
	int prop_index;

	ZVAL_NULL(result);
	if (Z_TYPE_P(property) == IS_STRING)
		return;
	else
		prop_index = Z_LVAL_P(property);
	if (prop_index < 0 || prop_index >= cmap->size) {
		php_error(E_WARNING, "colormap index out of range");
		return;
	}

	result = php_gdk_color_new(&cmap->colors[prop_index]);
}


/* GdkAtom */
inline GdkAtom php_gdk_atom_get(zval *wrapper)
{
	zval **atom;
	
	zend_hash_find(Z_OBJPROP_P(wrapper), "atom", sizeof("atom"), (void**)&atom);
	return (GdkAtom)Z_LVAL_PP(atom);
}

zval *php_gdk_atom_new(GdkAtom obj)
{
	zval *result;
	gchar *atom_name;

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_atom_ce);
	add_property_long_ex(result, "atom", sizeof("atom"), obj);
	atom_name = gdk_atom_name(obj);
	if (atom_name)
		add_property_string_ex(result, "string", sizeof("string"), gdk_atom_name(obj), 0);
	else
		add_property_null_ex(result, "string", sizeof("string"));

	return result;
}

/* GdkCursor */
zval *php_gdk_cursor_new(GdkCursor *obj)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!obj) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_cursor_ce);
	php_gtk_set_object(result, obj, le_gdk_cursor);

	return result;
}

static void release_gdk_cursor_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkCursor *obj = (GdkCursor *)rsrc->ptr;
	gdk_cursor_destroy(obj);
}

static void gdk_cursor_get_property(zval *result, zval *object, zval *property)
{
	GdkCursor *cursor = PHP_GDK_CURSOR_GET(object);
	char *prop_name;

	ZVAL_NULL(result);
	if (Z_TYPE_P(property) == IS_LONG)
		return;
	else
		prop_name = Z_STRVAL_P(property);

	if (!strcmp(prop_name, "type")) {
		ZVAL_LONG(result, cursor->type);
	} else if (!strcmp(prop_name, "name")) {
		GtkEnumValue *vals = gtk_type_enum_get_values(GTK_TYPE_GDK_CURSOR_TYPE);
		while (vals->value_name != NULL && vals->value != cursor->type)
			vals++;
		if (vals->value_nick) {
			ZVAL_STRING(result, vals->value_nick, 1);
		} else {
			ZVAL_STRINGL(result, "*unknown*", sizeof("*unknown*"), 1);
		}
	}
}


/* GdkVisual */
zval *php_gdk_visual_new(GdkVisual *obj)
{
	zval *result;

	MAKE_STD_ZVAL(result);

	if (!obj) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_visual_ce);
	gdk_visual_ref(obj);
	php_gtk_set_object(result, obj, le_gdk_visual);

	return result;
}

static void release_gdk_visual_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkVisual *obj = (GdkVisual *)rsrc->ptr;
	gdk_visual_unref(obj);
}

static void gdk_visual_get_property(zval *result, zval *object, zval *property)
{
	GdkVisual *visual = PHP_GDK_VISUAL_GET(object);
	char *prop_name;

	ZVAL_NULL(result);
	if (Z_TYPE_P(property) == IS_LONG)
		return;
	else
		prop_name = Z_STRVAL_P(property);

	if (!strcmp(prop_name, "type")) {
		ZVAL_LONG(result, visual->type);
	} else if (!strcmp(prop_name, "depth")) {
		ZVAL_LONG(result, visual->depth);
	} else if (!strcmp(prop_name, "byte_order")) {
		ZVAL_LONG(result, visual->byte_order);
	} else if (!strcmp(prop_name, "colormap_size")) {
		ZVAL_LONG(result, visual->colormap_size);
	} else if (!strcmp(prop_name, "bits_per_rgb")) {
		ZVAL_LONG(result, visual->bits_per_rgb);
	} else if (!strcmp(prop_name, "red_mask")) {
		ZVAL_LONG(result, visual->red_shift);
	} else if (!strcmp(prop_name, "red_shift")) {
		ZVAL_LONG(result, visual->red_prec);
	} else if (!strcmp(prop_name, "red_prec")) {
		ZVAL_LONG(result, visual->red_mask);
	} else if (!strcmp(prop_name, "green_mask")) {
		ZVAL_LONG(result, visual->green_mask);
	} else if (!strcmp(prop_name, "green_shift")) {
		ZVAL_LONG(result, visual->green_shift);
	} else if (!strcmp(prop_name, "green_prec")) {
		ZVAL_LONG(result, visual->green_prec);
	} else if (!strcmp(prop_name, "blue_mask")) {
		ZVAL_LONG(result, visual->blue_mask);
	} else if (!strcmp(prop_name, "blue_shift")) {
		ZVAL_LONG(result, visual->blue_shift);
	} else if (!strcmp(prop_name, "blue_prec")) {
		ZVAL_LONG(result, visual->blue_prec);
	}
}


/* Generic get/set property handlers. */
static zval php_gtk_get_property(zend_property_reference *property_reference)
{
	zval result;
	zend_overloaded_element *overloaded_property;
	zend_llist_element *element;
	zval *object = property_reference->object;

	for (element=property_reference->elements_list->head; element; element=element->next) {
		overloaded_property = (zend_overloaded_element *) element->data;
		if ((Z_TYPE_P(overloaded_property) == OE_IS_OBJECT &&
			Z_TYPE(overloaded_property->element) == IS_LONG) ||
			Z_TYPE_P(object) != IS_OBJECT) {
			convert_to_null(&result);
			return result;
		}

		if (php_gtk_check_class(object, gdk_window_ce)) {
			gdk_window_get_property(&result, object, &overloaded_property->element);
		} else if (php_gtk_check_class(object, gdk_color_ce)) {
			gdk_color_get_property(&result, object, &overloaded_property->element);
		} else if (php_gtk_check_class(object, gdk_colormap_ce)) {
			gdk_colormap_get_property(&result, object, &overloaded_property->element);
		} else if (php_gtk_check_class(object, gdk_cursor_ce)) {
			gdk_cursor_get_property(&result, object, &overloaded_property->element);
		} else if (php_gtk_check_class(object, gdk_visual_ce)) {
			gdk_visual_get_property(&result, object, &overloaded_property->element);
		} else {
			convert_to_null(&result);
			return result;
		}
		object = &result;

		zval_dtor(&overloaded_property->element);
	}

    return result;
}

static int php_gtk_set_property(zend_property_reference *property_reference, zval *value)
{
	zval result;
	zend_overloaded_element *overloaded_property;
	zend_llist_element *element;
	zval *object = property_reference->object;
	int retval;

	for (element=property_reference->elements_list->head; element && element!=property_reference->elements_list->tail; element=element->next) {
		overloaded_property = (zend_overloaded_element *) element->data;
		if (Z_TYPE_P(overloaded_property) != OE_IS_OBJECT ||
			Z_TYPE(overloaded_property->element) != IS_STRING ||
			Z_TYPE_P(object) != IS_OBJECT) {
			return FAILURE;
		}

		if (php_gtk_check_class(object, gdk_window_ce)) {
			gdk_window_get_property(&result, object, &overloaded_property->element);
		} else if (php_gtk_check_class(object, gdk_color_ce)) {
			gdk_color_get_property(&result, object, &overloaded_property->element);
		} else if (php_gtk_check_class(object, gdk_colormap_ce)) {
			gdk_colormap_get_property(&result, object, &overloaded_property->element);
		} else if (php_gtk_check_class(object, gdk_cursor_ce)) {
			gdk_cursor_get_property(&result, object, &overloaded_property->element);
		} else if (php_gtk_check_class(object, gdk_visual_ce)) {
			gdk_visual_get_property(&result, object, &overloaded_property->element);
		} else {
			return FAILURE;
		}
		object = &result;

		zval_dtor(&overloaded_property->element);
	}

	retval = FAILURE;
	overloaded_property = (zend_overloaded_element *) element->data;
	if (php_gtk_check_class(object, gdk_color_ce)) {
		gdk_color_set_property(object, Z_STRVAL(overloaded_property->element), value);
		retval =  SUCCESS;
	}

	zval_dtor(&overloaded_property->element);
	return retval;
}


void php_gtk_register_types(int module_number)
{
	zend_class_entry ce;

	le_gdk_event = zend_register_list_destructors_ex(NULL, NULL, "GdkEvent", module_number);
	le_gdk_window = zend_register_list_destructors_ex(release_gdk_window_rsrc, NULL, "GdkWindow", module_number);
	le_gdk_color = zend_register_list_destructors_ex(release_gdk_color_rsrc, NULL, "GdkColor", module_number);
	le_gdk_colormap = zend_register_list_destructors_ex(release_gdk_colormap_rsrc, NULL, "GdkColormap", module_number);
	le_gdk_colormap = zend_register_list_destructors_ex(release_gdk_cursor_rsrc, NULL, "GdkCursor", module_number);
	le_gdk_visual = zend_register_list_destructors_ex(release_gdk_visual_rsrc, NULL, "GdkVisual", module_number);


	INIT_CLASS_ENTRY(ce, "gdkevent", NULL);
	gdk_event_ce = zend_register_internal_class_ex(&ce, NULL, NULL);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkwindow", php_gdk_window_functions, NULL, php_gtk_get_property, NULL);
	gdk_window_ce = zend_register_internal_class_ex(&ce, NULL, NULL);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkcolor", php_gdk_color_functions, NULL, php_gtk_get_property, php_gtk_set_property);
	gdk_color_ce = zend_register_internal_class_ex(&ce, NULL, NULL);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkcolormap", php_gdk_colormap_functions, NULL, php_gtk_get_property, php_gtk_set_property);
	gdk_colormap_ce = zend_register_internal_class_ex(&ce, NULL, NULL);

	INIT_CLASS_ENTRY(ce, "gdkatom", NULL);
	gdk_atom_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	
	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkcursor", NULL, NULL, php_gtk_get_property, NULL);
	gdk_cursor_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
	
	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkvisual", NULL, NULL, php_gtk_get_property, NULL);
	gdk_visual_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
}

#endif
