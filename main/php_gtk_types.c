#include "php_gtk.h"

#if HAVE_PHP_GTK

int le_gdk_event;
int le_gdk_window;
int le_gdk_color;

zend_class_entry *gdk_event_ce;
zend_class_entry *gdk_window_ce;
zend_class_entry *gdk_color_ce;

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
		zend_hash_update(Z_OBJPROP_P(result), "window", sizeof("window"),
						 &value, sizeof(zval *), NULL);
	} else
		add_property_unset(result, "window");
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
				/* TODO set subwindow attribute */
			} else
				add_property_unset(result, "window");
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
			/* TODO set atom property */
			add_property_long(result, "time", obj->property.time);
			add_property_long(result, "state", obj->property.state);
			break;

		case GDK_SELECTION_CLEAR:
		case GDK_SELECTION_REQUEST:
		case GDK_SELECTION_NOTIFY:
			/* TODO set selection property */
			/* TODO set target property */
			/* TODO set property property */
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
			/* TODO add message_type property */
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

static void gdk_window_get_property(zval *result, zval *object, char *prop_name)
{
	zval *value;
	GdkWindow *win = PHP_GDK_WINDOW_GET(object);
	gint x, y;
	GdkModifierType p_mask;

	ZVAL_NULL(result);
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
		/* TODO return colormap */
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

static void gdk_color_get_property(zval *result, zval *object, char *prop_name)
{
	GdkColor *color = PHP_GDK_COLOR_GET(object);

	ZVAL_NULL(result);
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


static void release_gdk_color_rsrc(zend_rsrc_list_entry *rsrc)
{
	GdkColor *obj = (GdkColor *)rsrc->ptr;
	gdk_color_free(obj);
}


zval php_gtk_get_property(zend_property_reference *property_reference)
{
	zval result, *value;
	zend_overloaded_element *overloaded_property;
	zend_llist_element *element;
	zval *object = property_reference->object;

	for (element=property_reference->elements_list->head; element; element=element->next) {
		overloaded_property = (zend_overloaded_element *) element->data;
		if (Z_TYPE_P(overloaded_property) != OE_IS_OBJECT ||
			Z_TYPE(overloaded_property->element) != IS_STRING ||
			Z_TYPE_P(object) != IS_OBJECT) {
			convert_to_null(&result);
			return result;
		}

		if (Z_OBJCE_P(object) == gdk_window_ce) {
			gdk_window_get_property(&result, object, Z_STRVAL(overloaded_property->element));
		} else if (Z_OBJCE_P(object) == gdk_color_ce) {
			gdk_color_get_property(&result, object, Z_STRVAL(overloaded_property->element));
		} else {
			convert_to_null(&result);
			return result;
		}
		object = &result;

		zval_dtor(&overloaded_property->element);
	}

    return result;
}

void php_gtk_register_types(int module_number)
{
	zend_class_entry ce;

	le_gdk_event = zend_register_list_destructors_ex(NULL, NULL, "GdkEvent", module_number);
	le_gdk_window = zend_register_list_destructors_ex(release_gdk_window_rsrc, NULL, "GdkWindow", module_number);
	le_gdk_color = zend_register_list_destructors_ex(release_gdk_color_rsrc, NULL, "GdkColor", module_number);

	INIT_CLASS_ENTRY(ce, "gdkevent", NULL);
	gdk_event_ce = zend_register_internal_class_ex(&ce, NULL, NULL);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkwindow", NULL, NULL, php_gtk_get_property, NULL);
	gdk_window_ce = zend_register_internal_class_ex(&ce, NULL, NULL);

	INIT_OVERLOADED_CLASS_ENTRY(ce, "gdkcolor", NULL, NULL, php_gtk_get_property, NULL);
	gdk_color_ce = zend_register_internal_class_ex(&ce, NULL, NULL);
}

#endif
