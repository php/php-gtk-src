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

/* $Id$ */

#include "php_gtk.h"

#if HAVE_PHP_GTK

#include "gen_gdk.h"

/* {{{ phpg_rectangle_from_zval */
PHP_GTK_API int phpg_rectangle_from_zval(zval *value, GdkRectangle *rectangle TSRMLS_DC)
{
	phpg_return_val_if_fail(rectangle != NULL, FAILURE);

	if (phpg_gboxed_check(value, GDK_TYPE_RECTANGLE, TRUE TSRMLS_CC)) {
		*rectangle = *(GdkRectangle *)PHPG_GBOXED(value);
		return SUCCESS;
	}

	if (Z_TYPE_P(value) == IS_ARRAY
		&& php_gtk_parse_args_hash(value, "iiii", &rectangle->x, &rectangle->y,
								   &rectangle->width, &rectangle->height)) {
		return SUCCESS;
	}

	php_error(E_WARNING, "unable to convert to type GdkRectangle");

	return FAILURE;
}
/* }}} */

/* {{{ GdkRectangle marshalling */
static int phpg_gdk_rectangle_from_zval(const zval *value, GValue *gvalue TSRMLS_DC)
{
    GdkRectangle rect;

    if (phpg_rectangle_from_zval((zval *)value, &rect TSRMLS_CC) == FAILURE) {
        return FAILURE;
    }

    g_value_set_boxed(gvalue, &rect);
    
    return SUCCESS;
}

static int phpg_gdk_rectangle_to_zval(const GValue *gvalue, zval **value TSRMLS_DC)
{
    GdkRectangle *rect = (GdkRectangle *) g_value_get_boxed(gvalue);
    phpg_gboxed_new(value, GDK_TYPE_RECTANGLE, rect, TRUE, TRUE TSRMLS_CC);

    return SUCCESS;
}
/* }}} */

/* {{{ phpg_tree_path_from_zval */
PHP_GTK_API int phpg_tree_path_from_zval(const zval *value, GtkTreePath **path TSRMLS_DC)
{
    switch (Z_TYPE_P(value)) {
        case IS_STRING:
            *path = gtk_tree_path_new_from_string(Z_STRVAL_P(value));
            phpg_return_val_if_fail_quiet(*path != NULL, FAILURE);
            break;

        case IS_LONG:
            *path = gtk_tree_path_new();
            gtk_tree_path_append_index(*path, Z_LVAL_P(value));
            break;

        case IS_ARRAY:
        {
            zval **index;

            if (zend_hash_num_elements(Z_ARRVAL_P(value)) < 1) {
                return FAILURE;
            }

            *path = gtk_tree_path_new();
            for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(value));
                 zend_hash_get_current_data(Z_ARRVAL_P(value), (void**)&index) == SUCCESS;
                 zend_hash_move_forward(Z_ARRVAL_P(value))) {
                if (Z_TYPE_PP(index) != IS_LONG) {
                    php_error(E_WARNING, "Tree path elements have to be integers");
                    gtk_tree_path_free(*path);
                    return FAILURE;
                }
                gtk_tree_path_append_index(*path, Z_LVAL_PP(index));
            }
            break;
        }

        default:
            return FAILURE;
    }

    return SUCCESS;
}
/* }}} */

/* {{{ phpg_tree_path_to_zval */
PHP_GTK_API int phpg_tree_path_to_zval(GtkTreePath *path, zval **value TSRMLS_DC)
{
    int depth, i, *indices;

    phpg_return_val_if_fail_quiet(path != NULL, FAILURE);
    depth = gtk_tree_path_get_depth(path);
    indices = gtk_tree_path_get_indices(path);
    MAKE_ZVAL_IF_NULL(*value);
    array_init(*value);
    for (i = 0; i < depth; i++) {
        add_next_index_long(*value, indices[i]);
    }

    return SUCCESS;
}
/* }}} */

/* {{{ GtkTreePath marshalling */
static int phpg_gtk_tree_path_from_zval(const zval *value, GValue *gvalue TSRMLS_DC)
{
    GtkTreePath *path = NULL;

    if (phpg_tree_path_from_zval(value, &path TSRMLS_CC) == FAILURE) {
        return FAILURE;
    }

    g_value_set_boxed(gvalue, path);
    gtk_tree_path_free(path);
    return SUCCESS;
}

static int phpg_gtk_tree_path_to_zval(const GValue *gvalue, zval **value TSRMLS_DC)
{
    GtkTreePath *path = (GtkTreePath *)g_value_get_boxed(gvalue);;
    return phpg_tree_path_to_zval(path, value TSRMLS_CC);;
}
/* }}} */

/* {{{ Style Array Helper */

typedef struct {
    GtkStyle *style; /* here only so we can make sure it persists */
    gpointer array;
    style_helper_type type;
    HashTable *properties; /* dummy */
} style_helper;

static const int STYLE_NUM_STATES = 5;

static zval *style_helper_read_dimension(zval *object, zval *offset, int type TSRMLS_DC)
{
    zval *result = NULL;
    long index;
    style_helper *sh = (style_helper *) zend_object_store_get_object(object TSRMLS_CC);

	MAKE_STD_ZVAL(result);
	ZVAL_NULL(result);
	result->refcount = 0;
	result->is_ref = 0;

    if (Z_TYPE_P(offset) != IS_LONG) {
        php_error(E_WARNING, "Illegal index type");
		return EG(uninitialized_zval_ptr);
    }
    
    index = Z_LVAL_P(offset);
    if (index < 0 || index >= STYLE_NUM_STATES) {
        php_error(E_WARNING, "Index out of range");
		return EG(uninitialized_zval_ptr);
    }

    switch (sh->type) {
        case STYLE_COLOR_ARRAY:
        {
            GdkColor *array = (GdkColor *) sh->array;
            phpg_gboxed_new(&result, GDK_TYPE_COLOR, &array[index], TRUE, TRUE TSRMLS_CC);
            break;
        }

        case STYLE_GC_ARRAY:
        {
            GdkGC **array = (GdkGC **) sh->array;
            phpg_gobject_new(&result, (GObject *)array[index] TSRMLS_CC);
            break;
        }

        case STYLE_PIXMAP_ARRAY:
        {
            GdkPixmap **array = (GdkPixmap **) sh->array;
            phpg_gobject_new(&result, (GObject *)array[index] TSRMLS_CC);
            break;
        }
            
        default:
            g_assert_not_reached();
            break;
    }

    return result;
}

static void style_helper_write_dimension(zval *object, zval *offset, zval *value TSRMLS_DC)
{
    long index;
    style_helper *sh = (style_helper *) zend_object_store_get_object(object TSRMLS_CC);

    if (Z_TYPE_P(offset) != IS_LONG) {
        php_error(E_WARNING, "Illegal index type");
		return;
    }

    index = Z_LVAL_P(offset);
    if (index < 0 && index >= STYLE_NUM_STATES) {
        php_error(E_WARNING, "Index out of range");
        return;
    }

    switch (sh->type) {
        case STYLE_COLOR_ARRAY:
        {
            GdkColor *array = (GdkColor *) sh->array;
            if (!phpg_gboxed_check(value, GDK_TYPE_COLOR, TRUE TSRMLS_CC)) {
                php_error(E_WARNING, "Can only assign a GdkColor object");
                return;
            }
            array[index] = *(GdkColor *)PHPG_GBOXED(value);
            break;
        }

        case STYLE_GC_ARRAY:
        {
            GdkGC **array = (GdkGC **) sh->array;
            if (!phpg_object_check(value, gdkgc_ce TSRMLS_CC)) {
                php_error(E_WARNING, "Can only assign a GdkGC object");
                return;
            }
            if (array[index]) {
                g_object_unref(array[index]);
            }
            array[index] = GDK_GC(g_object_ref(PHPG_GOBJECT(value)));
            break;
        }

        case STYLE_PIXMAP_ARRAY:
        {
            GdkPixmap **array = (GdkPixmap **) sh->array;
            if (Z_TYPE_P(value) != IS_NULL && !phpg_object_check(value, gdkpixmap_ce TSRMLS_CC)) {
                php_error(E_WARNING, "Can only assign a GdkPixmap object or null");
                return;
            }
            if (array[index]) {
                g_object_unref(array[index]);
            }
            if (Z_TYPE_P(value) == IS_NULL) {
                array[index] = NULL;
            } else {
                array[index] = GDK_PIXMAP(g_object_ref(PHPG_GOBJECT(value)));
            }
            break;
        }

        default:
            g_assert_not_reached();
            break;
    }
}

static int style_helper_has_dimension(zval *object, zval *offset, int check_empty TSRMLS_DC)
{
    if (Z_TYPE_P(offset) != IS_LONG) {
        php_error(E_WARNING, "Illegal index type");
		return 0;
    }

    if (Z_LVAL_P(offset) < 0 && Z_LVAL_P(offset) >= STYLE_NUM_STATES) {
        php_error(E_WARNING, "Index out of range");
        return 0;
    }

    return 1;
}

static HashTable* style_helper_get_properties(zval *object TSRMLS_DC)
{
 /* int i;
    zval *result;
    zval offset; */
    style_helper *sh = (style_helper *) zend_object_store_get_object(object TSRMLS_CC);

    /* 
    offset.type = IS_LONG;
    for (i = 0; i < STYLE_NUM_STATES; i++) {
        offset.value.lval = i;
        result = style_helper_read_dimension(object, &offset, BP_VAR_R TSRMLS_CC);
        INIT_PZVAL(result);
        zend_hash_index_update(sh->properties, i, &result, sizeof(zval *), NULL);
    }
    */

    return sh->properties;
}

int style_helper_get_class_name(zval *object, char **class_name, zend_uint *class_name_len, int parent TSRMLS_DC)
{
	*class_name_len = sizeof("StyleHelper");
	*class_name = estrndup("StyleHelper", sizeof("StyleHelper")-1);
    return SUCCESS;
}

static int style_helper_count_elements(zval *object, long *count TSRMLS_DC)
{
    *count = STYLE_NUM_STATES;
    return SUCCESS;
}

static zend_object_handlers style_helper_handlers = {
	ZEND_OBJECTS_STORE_HANDLERS,
	NULL,						 /* read_property */
	NULL,						 /* write_property */
	style_helper_read_dimension, /* read dimension */
	style_helper_write_dimension,/* write_dimension */
	NULL,						 /* get_property_ptr_ptr */
	NULL,	                  	 /* get */
	NULL,		                 /* set */
	NULL,			 			 /* has_property */
	NULL,						 /* unset_property */
	style_helper_has_dimension,	 /* has_dimension */
	NULL,                        /* unset_dimension */
	style_helper_get_properties, /* get_properties */
	NULL,						 /* get_method */
	NULL,						 /* call_method */
	NULL,						 /* get_constructor */
	NULL,						 /* get_class_entry */
	style_helper_get_class_name, /* get_class_name */
	NULL,						 /* compare_objects */
	NULL,						 /* cast_object */
	style_helper_count_elements, /* count_elements */
};

static void style_helper_free_storage(style_helper *sh TSRMLS_DC)
{
    zend_hash_destroy(sh->properties);
    FREE_HASHTABLE(sh->properties);
    g_object_unref(sh->style);
    efree(sh);
}

PHP_GTK_API void phpg_create_style_helper(zval **zobj, GtkStyle *style, int type, gpointer array TSRMLS_DC)
{
	style_helper *sh = emalloc(sizeof(style_helper));

    sh->array = array;
    sh->type = type;
    sh->style = g_object_ref(style);
	ALLOC_HASHTABLE(sh->properties);
	zend_hash_init(sh->properties, 0, NULL, ZVAL_PTR_DTOR, 0);

    assert(zobj != NULL);
    if (*zobj == NULL) {
        MAKE_STD_ZVAL(*zobj);
    }
	Z_TYPE_PP(zobj) = IS_OBJECT;

	Z_OBJ_HANDLE_PP(zobj) = zend_objects_store_put(sh, NULL, (zend_objects_free_object_storage_t) style_helper_free_storage, NULL TSRMLS_CC);
	Z_OBJ_HT_PP(zobj) = &style_helper_handlers;
}

/* }}} */

/* {{{ GtkTreeModel */
int phpg_model_set_row(GtkTreeModel *model, GtkTreeIter *iter, zval *items)
{
    gint n_cols, i;
    GtkTreeModel *child;
    GtkTreeIter child_iter;
    zval **item;
    TSRMLS_FETCH();

    assert(items != NULL);

    if (!GTK_IS_LIST_STORE(model) && !GTK_IS_TREE_STORE(model) &&
        !GTK_IS_TREE_MODEL_SORT(model) && !GTK_IS_TREE_MODEL_FILTER(model)) {
        php_error(E_WARNING, "Cannot set row: unknown model type");
        return FAILURE;
    }

    if (GTK_IS_TREE_MODEL_SORT(model)) {
        child = gtk_tree_model_sort_get_model(GTK_TREE_MODEL_SORT(model));
        gtk_tree_model_sort_convert_iter_to_child_iter(GTK_TREE_MODEL_SORT(model), &child_iter, iter);
        return phpg_model_set_row(child, &child_iter, items);
    }

    if (GTK_IS_TREE_MODEL_FILTER(model)) {
        child = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(model));
        gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(model), &child_iter, iter);
        return phpg_model_set_row(child, &child_iter, items);
    }
    
    n_cols = gtk_tree_model_get_n_columns(model);
    if (zend_hash_num_elements(Z_ARRVAL_P(items)) != n_cols) {
        php_error(E_WARNING, "Cannot set row: number of row elements does not match the model");
        return FAILURE;
    }

    for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(items)), i = 0;
         zend_hash_get_current_data(Z_ARRVAL_P(items), (void **)&item) == SUCCESS;
         zend_hash_move_forward(Z_ARRVAL_P(items)), i++) {
        GValue value = { 0, };
        g_value_init(&value, gtk_tree_model_get_column_type(model, i));
        if (phpg_gvalue_from_zval(&value, *item TSRMLS_CC) == FAILURE) {
            php_error(E_WARNING, "Cannot set row: type of element %d does not match the model", i);
            return FAILURE;
        }

        if (GTK_IS_LIST_STORE(model)) {
            gtk_list_store_set_value(GTK_LIST_STORE(model), iter, i, &value);
        } else if (GTK_IS_TREE_STORE(model)) {
            gtk_tree_store_set_value(GTK_TREE_STORE(model), iter, i, &value);
        }

        g_value_unset(&value);
    }

    return SUCCESS;
}
/* }}} */

void php_gtk_plus_register_types()
{
    phpg_gboxed_register_custom(GDK_TYPE_RECTANGLE,
                                phpg_gdk_rectangle_from_zval,
                                phpg_gdk_rectangle_to_zval);

    phpg_gboxed_register_custom(GTK_TYPE_TREE_PATH,
                                phpg_gtk_tree_path_from_zval,
                                phpg_gtk_tree_path_to_zval);
}

/* {{{ old code */
#if 0
#ifndef PHP_WIN32
#include <gdk/gdkx.h>
#endif

int le_php_gtk_wrapper;
int le_gdk_window;
int le_gdk_bitmap;
int le_gdk_color;
int le_gdk_colormap;
int le_gdk_cursor;
int le_gdk_visual;
int le_gdk_font;
int le_gdk_gc;
int le_gdk_drag_context;
int le_gtk_accel_group;
int le_gtk_style;

PHP_GTK_EXPORT_CE(gdk_event_ce);
PHP_GTK_EXPORT_CE(gdk_window_ce);
PHP_GTK_EXPORT_CE(gdk_pixmap_ce);
PHP_GTK_EXPORT_CE(gdk_bitmap_ce);
PHP_GTK_EXPORT_CE(gdk_color_ce);
PHP_GTK_EXPORT_CE(gdk_colormap_ce);
PHP_GTK_EXPORT_CE(gdk_atom_ce);
PHP_GTK_EXPORT_CE(gdk_cursor_ce);
PHP_GTK_EXPORT_CE(gdk_visual_ce);
PHP_GTK_EXPORT_CE(gdk_font_ce);
PHP_GTK_EXPORT_CE(gdk_gc_ce);
PHP_GTK_EXPORT_CE(gdk_drag_context_ce);
PHP_GTK_EXPORT_CE(gtk_selection_data_ce);
PHP_GTK_EXPORT_CE(gtk_ctree_node_ce);
PHP_GTK_EXPORT_CE(gtk_accel_group_ce);
PHP_GTK_EXPORT_CE(gtk_style_ce);
PHP_GTK_EXPORT_CE(gtk_box_child_ce);
PHP_GTK_EXPORT_CE(gtk_table_child_ce);
PHP_GTK_EXPORT_CE(gtk_fixed_child_ce);
PHP_GTK_EXPORT_CE(gtk_clist_row_ce);

static char *php_gdk_window_properties[] = {
    "width",
    "height",
    "colormap",
    "type",
    "depth",
#ifndef PHP_WIN32
	"xid",
#endif
	"x",
	"y",
	"pointer",
	"pointer_state",
	"parent",
	"toplevel",
	"children",
    NULL
};

static char *php_gdk_pixmap_properties[] = {
    "width",
    "height",
    "colormap",
    "type",
    "depth",
#ifndef PHP_WIN32
	"xid",
#endif
    NULL
};

static char *php_gdk_bitmap_properties[] = {
    "width",
    "height",
    "colormap",
    "type",
    "depth",
#ifndef PHP_WIN32
	"xid",
#endif
    NULL
};

static char *php_gdk_color_properties[] = {
	"red",
	"green",
	"blue",
	"pixel",
    NULL
};

static char *php_gdk_colormap_properties[] = {
	"colors",
    NULL
};

static char *php_gdk_cursor_properties[] = {
	"type",
	"name",
    NULL
};

static char *php_gdk_visual_properties[] = {
	"type",
	"depth",
	"byte_order",
	"colormap_size",
	"bits_per_rgb",
	"red_mask",
	"red_shift",
	"red_prec",
	"green_mask",
	"green_shift",
	"green_prec",
	"blue_mask",
	"blue_shift",
	"blue_prec",
    NULL
};

static char *php_gdk_font_properties[] = {
	"type",
	"ascent",
	"descent",
    NULL
};

static char *php_gdk_gc_properties[] = {
	"foreground",
	"background",
	"font",
	"function",
	"fill",
	"tile",
	"stipple",
	"clip_mask",
	"subwindow_mode",
	"ts_x_origin",
	"ts_y_origin",
	"clip_x_origin",
	"clip_y_origin",
	"graphics_exposures",
	"line_width",
	"line_style",
	"cap_style",
	"join_style",
    NULL
};

static char *php_gdk_drag_context_properties[] = {
	"protocol",
	"is_source",
	"source_window",
	"dest_window",
	"targets",
	"actions",
	"suggested_action",
	"action",
	"start_time",
    NULL
};

static char *php_gtk_selection_data_properties[] = {
	"selection",
	"target",
	"type",
	"format",
	"length",
	"data",
    NULL
};

static char *php_gtk_ctree_node_properties[] = {
	"parent",
	"sibling",
	"children",
	"pixmap_closed",
	"pixmap_opened",
	"mask_closed",
	"mask_opened",
	"level",
	"is_leaf",
	"expanded",
	"row",
    NULL
};

static char *php_gtk_style_properties[] = {
	"black",
	"white",
	"font",
	"black_gc",
	"white_gc",
	"colormap",
	"fg",
	"bg",
	"light",
	"dark",
	"mid",
	"text",
	"base",
	"fg_gc",
	"bg_gc",
	"light_gc",
	"dark_gc",
	"mid_gc",
	"text_gc",
	"base_gc",
	"bg_pixmap",
    NULL
};

static char *php_gtk_box_child_properties[] = {
	"widget",
	"padding",
	"expand",
	"fill",
	"pack",
    NULL
};

static char *php_gtk_table_child_properties[] = {
	"widget",
	"left_attach",
	"right_attach",
	"top_attach",
	"bottom_attach",
	"xpadding",
	"ypadding",
	"xexpand",
	"yexpand",
	"xshrink",
	"yshrink",
	"xfill",
	"yfill",
    NULL
};

static char *php_gtk_fixed_child_properties[] = {
	"widget",
	"x",
	"y",
    NULL
};

static char *php_gtk_clist_row_properties[] = {
	"state",
	"foreground",
	"background",
	"style",
	"fg_set",
	"bg_set",
	"selectable",
    NULL
};

/* GdkEvent */
static function_entry php_gdk_event_functions[] = {
	{"GdkEvent", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_event_new(GdkEvent *event)
{
	zval *result;
	zval *value;
	TSRMLS_FETCH();

	MAKE_STD_ZVAL(result);

	if (!event) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_event_ce);
	php_gtk_set_object(result, event, NULL, 0);

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
			add_property_zval(result, "area", php_gdk_rectangle_new(&event->expose.area));
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
			add_property_bool(result, "is_hint", event->motion.is_hint);
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
	gint x, y;
    GdkModifierType mask = 0;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

    gdk_window_get_pointer(PHP_GDK_WINDOW_GET(this_ptr), &x, &y, &mask);
    *return_value = *php_gtk_build_value("(iii)", x, y, mask);
}

PHP_FUNCTION(gdk_window_set_cursor)
{
	zval *php_cursor = NULL;
	GdkCursor *cursor = NULL;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "|O", &php_cursor, gdk_cursor_ce))
		return;

	if (php_cursor)
		cursor = PHP_GDK_CURSOR_GET(php_cursor);
	gdk_window_set_cursor(PHP_GDK_WINDOW_GET(this_ptr), cursor);
	RETURN_NULL();
}

PHP_FUNCTION(gdk_window_new_gc)
{
	zval *php_values = NULL;
	zval **value;
	char *key;
	uint key_len;
	ulong num_key;
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

	gc = gdk_gc_new_with_values(Z_OBJCE_P(this_ptr) == gdk_bitmap_ce ?
								PHP_GDK_BITMAP_GET(this_ptr) :
								PHP_GDK_WINDOW_GET(this_ptr), &values, mask);
	*return_value = *php_gdk_gc_new(gc);
	gdk_gc_unref(gc);
}

PHP_FUNCTION(gdk_window_property_get)
{
	zval *php_property;
	GdkAtom property, type = 0;
	gint delete = FALSE;
	GdkAtom atype;
	gint aformat, alength;
	guchar *data;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "O|Oi", &php_property, gdk_atom_ce, &type, gdk_atom_ce, &delete)) {
		gchar *prop_name;

		if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s|Oi", &prop_name, &type, gdk_atom_ce, &delete))
			return;

		property = gdk_atom_intern(prop_name, FALSE);
	} else
		property = PHP_GDK_ATOM_GET(php_property);

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
						  get_active_function_name(TSRMLS_C));
				break;
		}
		g_free(data);
		*return_value = *php_gtk_build_value("(NiN)", php_gdk_atom_new(atype), aformat, php_data);
	}
}

PHP_FUNCTION(gdk_window_property_change)
{
	zval *php_property;
	GdkAtom property, type;
	gint format;
	zval *php_mode, *php_data;
	GdkPropMode mode;
	guchar *data = NULL;
	gint n_elements;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "OOiVV/", &php_property, gdk_atom_ce,
							&type, gdk_atom_ce, &format, &php_mode, &php_data)) {
		gchar *prop_name;

		if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "sOiVV/", &prop_name, &type,
								gdk_atom_ce, &format, &php_mode, &php_data))
			return;

		property = gdk_atom_intern(prop_name, FALSE);
	} else
		property = PHP_GDK_ATOM_GET(php_property);

	if (!php_gtk_get_enum_value(GTK_TYPE_GDK_PROP_MODE, php_mode, (gint *)&mode))
		return;

	switch (format) {
		case 8:
			if (Z_TYPE_P(php_data) != IS_STRING) {
				php_error(E_WARNING, "%s() expects data to be a string for format=8", get_active_function_name(TSRMLS_C));
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
					php_error(E_WARNING, "%s() expects data to be an array for format=16", get_active_function_name(TSRMLS_C));
					return;
				}
				n_elements = zend_hash_num_elements(Z_ARRVAL_P(php_data));
				data16 = g_new(guint16, n_elements);
				i = 0;
				for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(php_data));
					 zend_hash_get_current_data(Z_ARRVAL_P(php_data), (void **)&data_temp);
					 zend_hash_move_forward(Z_ARRVAL_P(php_data))) {
					convert_to_long_ex(data_temp);
					data16[i++] = (guint16)Z_LVAL_PP(data_temp); 
				}
			}	
			break;

		case 32:
			{
				guint32 *data32;
				zval **data_temp;
				int i;

				if (Z_TYPE_P(php_data) != IS_ARRAY) {
					php_error(E_WARNING, "%s() expects data to be an array for format=32", get_active_function_name(TSRMLS_C));
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
			php_error(E_WARNING, "%s() expects format to be 8, 16, or 32", get_active_function_name(TSRMLS_C));
			break;
	}

	gdk_property_change(PHP_GDK_WINDOW_GET(this_ptr), property, type, format,
						mode, data, n_elements);
	if (format != 8)
		g_free(data);
}

PHP_FUNCTION(gdk_window_property_delete)
{
	zval *php_property;
	GdkAtom property;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "O", &php_property, gdk_atom_ce)) {
		gchar *prop_name;

		if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "s", &prop_name))
			return;

		property = gdk_atom_intern(prop_name, FALSE);
	} else
		property = PHP_GDK_ATOM_GET(php_property);

	gdk_property_delete(PHP_GDK_WINDOW_GET(this_ptr), property);
}

PHP_FUNCTION(gdk_window_set_icon)
{
	zval *php_icon_window, *php_pixmap, *php_mask;
	GdkWindow *icon_window = NULL;
	GdkPixmap *pixmap = NULL;
	GdkBitmap *mask = NULL;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "NNN", &php_icon_window,
							gdk_window_ce, &php_pixmap, gdk_pixmap_ce,
							&php_mask, gdk_bitmap_ce))
		return;

	if (Z_TYPE_P(php_icon_window) != IS_NULL)
		icon_window = PHP_GDK_WINDOW_GET(php_icon_window);
	if (Z_TYPE_P(php_pixmap) != IS_NULL)
		pixmap = PHP_GDK_PIXMAP_GET(php_pixmap);
	if (Z_TYPE_P(php_mask) != IS_NULL)
		mask = PHP_GDK_BITMAP_GET(php_mask);

	gdk_window_set_icon(PHP_GDK_WINDOW_GET(this_ptr), icon_window, pixmap, mask);
}

PHP_FUNCTION(gdk_window_copy_area)
{
	zval *gc, *php_src_window;
	GdkDrawable *src_window = NULL;
	int x, y, src_x, src_y, width, height;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "OiiViiii", &gc, gdk_gc_ce, &x, &y,
							&php_src_window, &src_x, &src_y, &width, &height))
		return;

	if (Z_TYPE_P(php_src_window) != IS_NULL) {
		if (php_gtk_check_class(php_src_window, gdk_window_ce))
			src_window = (GdkDrawable *)PHP_GDK_WINDOW_GET(php_src_window);
		else if(php_gtk_check_class(php_src_window, gdk_pixmap_ce))
			src_window = (GdkDrawable *)PHP_GDK_PIXMAP_GET(php_src_window);
		else if(php_gtk_check_class(php_src_window, gdk_bitmap_ce))
			src_window = (GdkDrawable *)PHP_GDK_BITMAP_GET(php_src_window);
		else {
			php_error(E_WARNING, "%s() expects source drawable to be GdkWindow, GdkPixmap, or GdkBitmap", get_active_function_name(TSRMLS_C));
			return;
		}
	}

	gdk_window_copy_area(PHP_GDK_WINDOW_GET(this_ptr), PHP_GDK_GC_GET(gc), x, y,
						 src_window, src_x, src_y, width, height);
}

static function_entry php_gdk_window_functions[] = {
	{"GdkWindow",		PHP_FN(no_direct_constructor), NULL},
	{"raise", 			PHP_FN(gdk_window_raise), NULL},
	{"lower", 			PHP_FN(gdk_window_lower), NULL},
	{"get_pointer", 	PHP_FN(gdk_window_get_pointer), NULL},
	{"set_cursor", 		PHP_FN(gdk_window_set_cursor), NULL},
	{"new_gc", 			PHP_FN(gdk_window_new_gc), NULL},
	{"property_get", 	PHP_FN(gdk_window_property_get), NULL},
	{"property_change", PHP_FN(gdk_window_property_change), NULL},
	{"property_delete", PHP_FN(gdk_window_property_delete), NULL},
	{"set_icon",		PHP_FN(gdk_window_set_icon), NULL},
	{"copy_area",		PHP_FN(gdk_window_copy_area), NULL},
	{NULL, NULL, NULL}
};

static function_entry php_gdk_bitmap_functions[] = {
	{"GdkBitmap",		PHP_FN(no_direct_constructor), NULL},
	{"new_gc", 			PHP_FN(gdk_window_new_gc), NULL},
	{"copy_area",		PHP_FN(gdk_window_copy_area), NULL},
	{NULL, NULL, NULL}
};

PHP_FUNCTION(gdkpixmap)
{
	GdkWindow *window = NULL;
	zval *php_window;
	long width, height, depth = -1;
	GdkPixmap *wrapped_obj = NULL;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "Nii|i", &php_window, gdk_window_ce, &width, &height, &depth)) {
		php_gtk_invalidate(this_ptr);
		return;
	}

	if (Z_TYPE_P(php_window) != IS_NULL)
		window = PHP_GDK_WINDOW_GET(php_window);

	wrapped_obj = gdk_pixmap_new(window, (gint)width, (gint)height, (gint)depth);
	if (!wrapped_obj) {
		php_error(E_WARNING, "%s(): could not create GtkWindow object",
				  get_active_function_name(TSRMLS_C));
		return;
	}

	php_gtk_set_object(this_ptr, wrapped_obj, (php_gtk_dtor_t)gdk_pixmap_unref, 1);
}

static function_entry php_gdk_pixmap_functions[] = {
	{"GdkPixmap",		PHP_FN(gdkpixmap), NULL},
	{"new_gc", 			PHP_FN(gdk_window_new_gc), NULL},
	{"property_get", 	PHP_FN(gdk_window_property_get), NULL},
	{"property_change", PHP_FN(gdk_window_property_change), NULL},
	{"property_delete", PHP_FN(gdk_window_property_delete), NULL},
	{"copy_area",		PHP_FN(gdk_window_copy_area), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_window_new(GdkWindow *window)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!window) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)window, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_window_ce);

	gdk_window_ref(window);
	php_gtk_set_object(result, window, (php_gtk_dtor_t)gdk_window_unref, 1);

	return result;
}

zval *php_gdk_pixmap_new(GdkPixmap *pixmap)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!pixmap) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)pixmap, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_pixmap_ce);

	gdk_pixmap_ref(pixmap);
	php_gtk_set_object(result, pixmap, (php_gtk_dtor_t)gdk_pixmap_unref, 1);

	return result;
}

zval *php_gdk_bitmap_new(GdkWindow *bitmap)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!bitmap) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)bitmap, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_bitmap_ce);

	gdk_bitmap_ref(bitmap);
	php_gtk_set_object(result, bitmap, (php_gtk_dtor_t)gdk_bitmap_unref, 1);

	return result;
}

static void gdk_window_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GdkWindow *win;
	gint x, y;
	GdkModifierType p_mask;
	TSRMLS_FETCH();
	
	*result = SUCCESS;

	if (Z_OBJCE_P(object) == gdk_bitmap_ce) {
		win = PHP_GDK_BITMAP_GET(object);
	} else {
		win = PHP_GDK_WINDOW_GET(object);
	}

	if (!strcmp(prop_name, "width")) {
		gdk_window_get_size(win, &x, NULL);
		RETURN_LONG(x);
	} else if (!strcmp(prop_name, "height")) {
		gdk_window_get_size(win, NULL, &y);
		RETURN_LONG(y);
	} else if (!strcmp(prop_name, "colormap")) {
		if (Z_OBJCE_P(object) != gdk_bitmap_ce) {
			*return_value = *php_gdk_colormap_new(gdk_window_get_colormap(win));
		}
		return;
	} else if (!strcmp(prop_name, "type")) {
		RETURN_LONG(gdk_window_get_type(win));
	} else if (!strcmp(prop_name, "depth")) {
		gdk_window_get_geometry(win, NULL, NULL, NULL, NULL, &x);
		RETURN_LONG(x);
	}
#ifndef PHP_WIN32
	else if (!strcmp(prop_name, "xid")) {
		RETURN_LONG(GDK_WINDOW_XWINDOW(win));
	}
#endif

	if (Z_OBJCE_P(object) == gdk_window_ce) {
		if (!strcmp(prop_name, "x")) {
			gdk_window_get_position(win, &x, NULL);
			RETURN_LONG(x);
		} else if (!strcmp(prop_name, "y")) {
			gdk_window_get_position(win, NULL, &y);
			RETURN_LONG(y);
		} else if (!strcmp(prop_name, "pointer")) {
			gdk_window_get_pointer(win, &x, &y, NULL);
			*return_value = *php_gtk_build_value("(ii)", x, y);
			return;
		} else if (!strcmp(prop_name, "pointer_state")) {
			gdk_window_get_pointer(win, NULL, NULL, &p_mask);
			RETURN_LONG(p_mask);
		} else if (!strcmp(prop_name, "parent")) {
			GdkWindow *parent = gdk_window_get_parent(win);
			if (parent)
				*return_value = *php_gdk_window_new(parent);
			return;
		} else if (!strcmp(prop_name, "toplevel")) {
			*return_value = *php_gdk_window_new(gdk_window_get_toplevel(win));
			return;
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
			return;
		}
	}

	*result = FAILURE;
}

static void release_gdk_window_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkWindow *obj = (GdkWindow *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	if (gdk_window_get_type(obj) == GDK_WINDOW_PIXMAP)
		gdk_pixmap_unref(obj);
	else
		gdk_window_unref(obj);
}

static void release_gdk_bitmap_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkBitmap *obj = (GdkBitmap *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gdk_bitmap_unref(obj);
}

/* GdkColor */
PHP_FUNCTION(gdkcolor)
{
	char *color_spec;
	GdkColor color;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "hhh", &color.red, &color.green, &color.blue)) {
		if (!php_gtk_parse_args_quiet(ZEND_NUM_ARGS(), "s", &color_spec)) {
			php_error(E_WARNING, "%s() requires a color spec or RGB triplet", get_active_function_name(TSRMLS_C));
			php_gtk_invalidate(this_ptr);
			return;
		} else if (!gdk_color_parse(color_spec, &color)) {
			php_error(E_WARNING, "%s() could not parse color spec '%s'", get_active_function_name(TSRMLS_C), color_spec);
			php_gtk_invalidate(this_ptr);
			return;
		}
	}

	php_gtk_set_object(this_ptr, gdk_color_copy(&color), (php_gtk_dtor_t)gdk_color_free, 0);
}

static function_entry php_gdk_color_functions[] = {
	{"GdkColor", PHP_FN(gdkcolor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_color_new(GdkColor *color)
{
	zval *result;
	GdkColor *result_obj;
	TSRMLS_FETCH();

	MAKE_STD_ZVAL(result);

	if (!color) {
		ZVAL_NULL(result);
		return result;
	}

	object_init_ex(result, gdk_color_ce);

	result_obj = gdk_color_copy(color);
	php_gtk_set_object(result, result_obj, (php_gtk_dtor_t)gdk_color_free, 0);

	return result;
}

static void gdk_color_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GdkColor *color = PHP_GDK_COLOR_GET(object);

	*result = SUCCESS;

	if (!strcmp(prop_name, "red")) {
		RETURN_LONG(color->red);
	} else if (!strcmp(prop_name, "green")) {
		RETURN_LONG(color->green);
	} else if (!strcmp(prop_name, "blue")) {
		RETURN_LONG(color->blue);
	} else if (!strcmp(prop_name, "pixel")) {
		RETURN_LONG(color->pixel);
	}

	*result = FAILURE;
}

static int gdk_color_set_property(zval *object, char *prop_name, zval *value)
{
	GdkColor *color = PHP_GDK_COLOR_GET(object);

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

static void release_gdk_color_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkColor *obj = (GdkColor *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

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
			php_error(E_WARNING, "%s() requires a color spec or RGB triplet", get_active_function_name(TSRMLS_C));
			return;
		} else if (!gdk_color_parse(color_spec, &color)) {
			php_error(E_WARNING, "%s() could not parse color spec '%s'", get_active_function_name(TSRMLS_C), color_spec);
			return;
		}
	}

	if (!gdk_colormap_alloc_color(PHP_GDK_COLORMAP_GET(this_ptr), &color, writeable, best_match)) {
		php_error(E_NOTICE, "%s() could not allocate color");
		return;
	}

	*return_value = *php_gdk_color_new(&color);
}

static function_entry php_gdk_colormap_functions[] = {
	{"GdkColormap", PHP_FN(no_direct_constructor), NULL},
	{"size", 		PHP_FN(gdk_colormap_size), NULL},
	{"alloc",		PHP_FN(gdk_colormap_alloc), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_colormap_new(GdkColormap *cmap)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!cmap) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)cmap, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_colormap_ce);

	gdk_colormap_ref(cmap);
	php_gtk_set_object(result, cmap, (php_gtk_dtor_t)gdk_colormap_unref, 1);

	return result;
}

static void release_gdk_colormap_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkColormap *obj = (GdkColormap *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gdk_colormap_unref(obj);
}

static void gdk_colormap_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GdkColormap *cmap = PHP_GDK_COLORMAP_GET(object);

	*result = SUCCESS;

	if (!strcmp(prop_name, "colors") && cmap->colors) {
        int i;

        array_init(return_value);

        for (i = 0; i < cmap->size; i++)
            add_next_index_zval(return_value, php_gdk_color_new(&cmap->colors[i]));

		return;
	}

	*result = FAILURE;
}


/* GdkAtom */
/* TODO maybe change this to have a real constructor */
static function_entry php_gdk_atom_functions[] = {
	{"GdkAtom", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_atom_new(GdkAtom atom)
{
	zval *result;
	gchar *atom_name;
	TSRMLS_FETCH();

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_atom_ce);
	add_property_long_ex(result, "atom", sizeof("atom"), atom TSRMLS_CC);
	atom_name = gdk_atom_name(atom);
	if (atom_name)
		add_property_string_ex(result, "string", sizeof("string"), atom_name, 0 TSRMLS_CC);
	else
		add_property_null_ex(result, "string", sizeof("string") TSRMLS_CC);

	return result;
}

/* GdkCursor */
static function_entry php_gdk_cursor_functions[] = {
	{"GdkCursor", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_cursor_new(GdkCursor *cursor)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!cursor) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)cursor, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_cursor_ce);
	php_gtk_set_object(result, cursor, (php_gtk_dtor_t)gdk_cursor_destroy, 1);

	return result;
}

static void release_gdk_cursor_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkCursor *obj = (GdkCursor *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gdk_cursor_destroy(obj);
}

static void gdk_cursor_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GdkCursor *cursor = PHP_GDK_CURSOR_GET(object);

	*result = SUCCESS;

	if (!strcmp(prop_name, "type")) {
		RETURN_LONG(cursor->type);
	} else if (!strcmp(prop_name, "name")) {
		GtkEnumValue *vals = gtk_type_enum_get_values(GTK_TYPE_GDK_CURSOR_TYPE);
		while (vals->value_name != NULL && vals->value != (unsigned)cursor->type)
			vals++;
		if (vals->value_nick) {
			RETURN_STRING(vals->value_nick, 1);
		} else {
			RETURN_STRINGL("*unknown*", sizeof("*unknown*"), 1);
		}
	}

	*result = FAILURE;
}


/* GdkVisual */
static function_entry php_gdk_visual_functions[] = {
	{"GdkVisual", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_visual_new(GdkVisual *visual)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!visual) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)visual, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_visual_ce);
	gdk_visual_ref(visual);
	php_gtk_set_object(result, visual, (php_gtk_dtor_t)gdk_visual_unref, 1);

	return result;
}

static void release_gdk_visual_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkVisual *obj = (GdkVisual *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gdk_visual_unref(obj);
}

static void gdk_visual_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GdkVisual *visual = PHP_GDK_VISUAL_GET(object);

	*result = SUCCESS;

	if (!strcmp(prop_name, "type")) {
		RETURN_LONG(visual->type);
	} else if (!strcmp(prop_name, "depth")) {
		RETURN_LONG(visual->depth);
	} else if (!strcmp(prop_name, "byte_order")) {
		RETURN_LONG(visual->byte_order);
	} else if (!strcmp(prop_name, "colormap_size")) {
		RETURN_LONG(visual->colormap_size);
	} else if (!strcmp(prop_name, "bits_per_rgb")) {
		RETURN_LONG(visual->bits_per_rgb);
	} else if (!strcmp(prop_name, "red_mask")) {
		RETURN_LONG(visual->red_shift);
	} else if (!strcmp(prop_name, "red_shift")) {
		RETURN_LONG(visual->red_prec);
	} else if (!strcmp(prop_name, "red_prec")) {
		RETURN_LONG(visual->red_mask);
	} else if (!strcmp(prop_name, "green_mask")) {
		RETURN_LONG(visual->green_mask);
	} else if (!strcmp(prop_name, "green_shift")) {
		RETURN_LONG(visual->green_shift);
	} else if (!strcmp(prop_name, "green_prec")) {
		RETURN_LONG(visual->green_prec);
	} else if (!strcmp(prop_name, "blue_mask")) {
		RETURN_LONG(visual->blue_mask);
	} else if (!strcmp(prop_name, "blue_shift")) {
		RETURN_LONG(visual->blue_shift);
	} else if (!strcmp(prop_name, "blue_prec")) {
		RETURN_LONG(visual->blue_prec);
	}

	*result = FAILURE;
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
	{"GdkFont",	PHP_FN(no_direct_constructor), NULL},
	{"width", 	PHP_FN(gdk_font_width), NULL},
	{"height", 	PHP_FN(gdk_font_height), NULL},
	{"measure", PHP_FN(gdk_font_measure), NULL},
	{"extents",	PHP_FN(gdk_font_extents), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_font_new(GdkFont *font)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!font) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)font, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_font_ce);
	gdk_font_ref(font);
	php_gtk_set_object(result, font, (php_gtk_dtor_t)gdk_font_unref, 1);

	return result;
}

static void release_gdk_font_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkFont *obj = (GdkFont *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gdk_font_unref(obj);
}

static void gdk_font_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GdkFont *font = PHP_GDK_FONT_GET(object);

	*result = SUCCESS;

	if (!strcmp(prop_name, "type")) {
		RETURN_LONG(font->type);
	} else if (!strcmp(prop_name, "ascent")) {
		RETURN_LONG(font->ascent);
	} else if (!strcmp(prop_name, "descent")) {
		RETURN_LONG(font->descent);
	}

	*result = FAILURE;
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
					  get_active_function_name(TSRMLS_C)); 
			g_free(dash_list);
			return;
		}
		dash_list[i] = (guchar)Z_LVAL_PP(temp);
		if (dash_list[i] == 0) {
			php_error(E_WARNING, "%s() expects array elements to be non-zero",
					  get_active_function_name(TSRMLS_C)); 
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
	{"GdkGC",		PHP_FN(no_direct_constructor), NULL},
	{"set_dashes",	PHP_FN(gdk_gc_set_dashes), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_gc_new(GdkGC *gc)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!gc) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)gc, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_gc_ce);
	gdk_gc_ref(gc);
	php_gtk_set_object(result, gc, (php_gtk_dtor_t)gdk_gc_unref, 1);

	return result;
}

static void release_gdk_gc_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkGC *obj = (GdkGC *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gdk_gc_unref(obj);
}

#if 0
static void gdk_colormap_query_color (GdkColormap *colormap, gulong pixel, GdkColor *result)
{
#ifndef PHP_WIN32
	XColor xcolor;
#endif
	GdkVisual *visual;

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
#endif

static void gdk_gc_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GdkGC *gc = PHP_GDK_GC_GET(object);
	GdkGCValues gcv;

	*result = SUCCESS;

	gdk_gc_get_values(gc, &gcv);

	if (!strcmp(prop_name, "foreground")) {
		*return_value = *php_gdk_color_new(&gcv.foreground);
		return;
	} else if (!strcmp(prop_name, "background")) {
		*return_value = *php_gdk_color_new(&gcv.background);
		return;
	} else if (!strcmp(prop_name, "font")) {
		*return_value = *php_gdk_font_new(gcv.font);
		return;
	} else if (!strcmp(prop_name, "function")) {
		RETURN_LONG(gcv.function);
	} else if (!strcmp(prop_name, "fill")) {
		RETURN_LONG(gcv.fill);
	} else if (!strcmp(prop_name, "tile")) {
		if (gcv.tile)
			*return_value = *php_gdk_pixmap_new(gcv.tile);
		return;
	} else if (!strcmp(prop_name, "stipple")) {
		if (gcv.stipple)
			*return_value = *php_gdk_pixmap_new(gcv.stipple);
		return;
	} else if (!strcmp(prop_name, "clip_mask")) {
		if (gcv.clip_mask)
			*return_value = *php_gdk_pixmap_new(gcv.clip_mask);
		return;
	} else if (!strcmp(prop_name, "subwindow_mode")) {
		RETURN_LONG(gcv.subwindow_mode);
	} else if (!strcmp(prop_name, "ts_x_origin")) {
		RETURN_LONG(gcv.ts_x_origin);
	} else if (!strcmp(prop_name, "ts_y_origin")) {
		RETURN_LONG(gcv.ts_y_origin);
	} else if (!strcmp(prop_name, "clip_x_origin")) {
		RETURN_LONG(gcv.clip_x_origin);
	} else if (!strcmp(prop_name, "clip_y_origin")) {
		RETURN_LONG(gcv.clip_y_origin);
	} else if (!strcmp(prop_name, "graphics_exposures")) {
		RETURN_LONG(gcv.graphics_exposures);
	} else if (!strcmp(prop_name, "line_width")) {
		RETURN_LONG(gcv.line_width);
	} else if (!strcmp(prop_name, "line_style")) {
		RETURN_LONG(gcv.line_style);
	} else if (!strcmp(prop_name, "cap_style")) {
		RETURN_LONG(gcv.cap_style);
	} else if (!strcmp(prop_name, "join_style")) {
		RETURN_LONG(gcv.join_style);
	}

	*result = FAILURE;
}

static int gdk_gc_set_property(zval *object, char *prop_name, zval *value)
{
	GdkGC *gc = PHP_GDK_GC_GET(object);
	zend_bool type_mismatch = 0;
	int i;
	static char *props[] = {
		"function", "fill", "subwindow_mode", "ts_x_origin", "ts_y_origin",
		"clip_x_origin", "clip_y_origin", "graphics_exposures", "line_width",
		"line_style", "cap_style", "join_style", "foreground", "background",
		"font", "tile", "stipple", "clip_mask" };

	for (i = 0; i < sizeof(props)/sizeof(char *); i++) {
		if (!strcmp(prop_name, props[i]))
			break;
	}
	if (i == sizeof(props)/sizeof(char *))
		return FAILURE;

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
		else
			type_mismatch = 1;
	} else if (php_gtk_check_class(value, gdk_color_ce)) {
		GdkColor *c = PHP_GDK_COLOR_GET(value);

		if (!strcmp(prop_name, "foreground"))
			gdk_gc_set_foreground(gc, c);
		else if (!strcmp(prop_name, "background"))
			gdk_gc_set_background(gc, c);
		else
			type_mismatch = 1;
	} else if (php_gtk_check_class(value, gdk_font_ce)) {
		if (!strcmp(prop_name, "font"))
			gdk_gc_set_font(gc, PHP_GDK_FONT_GET(value));
		else
			type_mismatch = 1;
	} else if (php_gtk_check_class(value, gdk_pixmap_ce) || php_gtk_check_class(value, gdk_bitmap_ce) || Z_TYPE_P(value) == IS_NULL) {
		GdkDrawable *d = NULL;

		if (Z_TYPE_P(value) != IS_NULL) {
			if (php_gtk_check_class(value, gdk_pixmap_ce))
				d = PHP_GDK_PIXMAP_GET(value);
			else
				d = PHP_GDK_BITMAP_GET(value);
		}

		if (!strcmp(prop_name, "tile"))
			gdk_gc_set_tile(gc, d);
		else if (!strcmp(prop_name, "stipple"))
			gdk_gc_set_stipple(gc, d);
		else if (!strcmp(prop_name, "clip_mask"))
			gdk_gc_set_clip_mask(gc, d);
		else
			type_mismatch = 1;
	} else
		type_mismatch = 1;

	if (type_mismatch) {
		php_error(E_WARNING, "Type mismatch trying to set property '%s'", prop_name);
		return PG_ERROR;
	} else
		return SUCCESS;
}


/* GdkDragContext */
static function_entry php_gdk_drag_context_functions[] = {
	{"GdkDragContext", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gdk_drag_context_new(GdkDragContext *context)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!context) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)context, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_drag_context_ce);
	gdk_drag_context_ref(context);
	php_gtk_set_object(result, context, (php_gtk_dtor_t)gdk_drag_context_unref, 1);

	return result;
}

static void release_gdk_drag_context_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkDragContext *obj = (GdkDragContext *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gdk_drag_context_unref(obj);
}

static void gdk_drag_context_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GdkDragContext *context = PHP_GDK_DRAG_CONTEXT_GET(object);

	*result = SUCCESS;

	if (!strcmp(prop_name, "protocol")) {
		RETURN_LONG(context->protocol);
	} else if (!strcmp(prop_name, "is_source")) {
		RETURN_BOOL(context->is_source);
	} else if (!strcmp(prop_name, "source_window")) {
		if (context->source_window)
			*return_value = *php_gdk_window_new(context->source_window);
		return;
	} else if (!strcmp(prop_name, "dest_window")) {
		if (context->dest_window)
			*return_value = *php_gdk_window_new(context->dest_window);
		return;
	} else if (!strcmp(prop_name, "targets")) {
		GList *tmp;

		array_init(return_value);
		for (tmp = context->targets; tmp; tmp = tmp->next)
			add_next_index_zval(return_value, php_gdk_atom_new(GPOINTER_TO_INT(tmp->data)));
		return;
	} else if (!strcmp(prop_name, "actions")) {
		RETURN_LONG(context->actions);
	} else if (!strcmp(prop_name, "suggested_action")) {
		RETURN_LONG(context->suggested_action);
	} else if (!strcmp(prop_name, "action")) {
		RETURN_LONG(context->action);
	} else if (!strcmp(prop_name, "start_time")) {
		RETURN_LONG(context->start_time);
	}

	*result = FAILURE;
}


/* GtkSelectionData */
PHP_FUNCTION(gtk_selection_data_set)
{
	zval *type;
	int format, length;
	guchar *data;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "Ois#", &type, gdk_atom_ce, &format, &data, &length))
		return;

	gtk_selection_data_set(PHP_GTK_SELECTION_DATA_GET(this_ptr), PHP_GDK_ATOM_GET(type), format, data, length);
	RETURN_NULL();
}

static function_entry php_gtk_selection_data_functions[] = {
	{"GtkSelectionData", PHP_FN(no_direct_constructor), NULL},
	{"set", 			 PHP_FN(gtk_selection_data_set), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_selection_data_new(GtkSelectionData *data)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!data) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)data, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gtk_selection_data_ce);
	php_gtk_set_object(result, data, NULL, 1);

	return result;
}

static void gtk_selection_data_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GtkSelectionData *data = PHP_GTK_SELECTION_DATA_GET(object);

	*result = SUCCESS;

	if (!strcmp(prop_name, "selection")) {
		*return_value = *php_gdk_atom_new(data->selection);
		return;
	} else if (!strcmp(prop_name, "target")) {
		*return_value = *php_gdk_atom_new(data->target);
		return;
	} else if (!strcmp(prop_name, "type")) {
		*return_value = *php_gdk_atom_new(data->type);
		return;
	} else if (!strcmp(prop_name, "format")) {
		RETURN_LONG(data->format);
	} else if (!strcmp(prop_name, "length")) {
		RETURN_LONG(data->length);
	} else if (!strcmp(prop_name, "data") && data->length > -1) {
		RETURN_STRINGL(data->data, data->length, 1);
	}

	*result = FAILURE;
}

/* GtkCtreeNode */
static function_entry php_gtk_ctree_node_functions[] = {
	{"GtkCTreeNode", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_ctree_node_new(GtkCTreeNode *node)
{
	zval *result;
	zval **wrapper_ptr;
	static zval *static_ptr = NULL;
	TSRMLS_FETCH();

	if (!node) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)node, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gtk_ctree_node_ce);
	php_gtk_set_object(result, node, NULL, 1);
	if (static_ptr == NULL) {
		static_ptr = result;
	}

	return result;
}

static void gtk_ctree_node_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GtkCTreeNode *node = PHP_GTK_CTREE_NODE_GET(object);
	GtkCTreeNode *temp;

	*result = SUCCESS;

	if (!strcmp(prop_name, "parent")) {
		temp = GTK_CTREE_ROW(node)->parent;
		if (temp)
			*return_value = *php_gtk_ctree_node_new(temp);
		return;
	} else if (!strcmp(prop_name, "sibling")) {
		temp = GTK_CTREE_ROW(node)->sibling;
		if (temp)
			*return_value = *php_gtk_ctree_node_new(temp);
		return;
	} else if (!strcmp(prop_name, "children")) {
		zval *php_node;

		temp = GTK_CTREE_ROW(node)->children;

        array_init(return_value);
        while (temp) {
            php_node = php_gtk_ctree_node_new(temp);
            add_next_index_zval(return_value, php_node);
            temp = GTK_CTREE_ROW(temp)->sibling;
        }

		return;
	} else if (!strcmp(prop_name, "pixmap_closed")) {
		GdkPixmap *pixmap = GTK_CTREE_ROW(node)->pixmap_closed;
		if (pixmap)
			*return_value = *php_gdk_pixmap_new(pixmap);
		return;
	} else if (!strcmp(prop_name, "pixmap_opened")) {
		GdkPixmap *pixmap = GTK_CTREE_ROW(node)->pixmap_opened;
		if (pixmap)
			*return_value = *php_gdk_pixmap_new(pixmap);
		return;
	} else if (!strcmp(prop_name, "mask_closed")) {
		GdkBitmap *mask = GTK_CTREE_ROW(node)->mask_closed;
		if (mask)
			*return_value = *php_gdk_bitmap_new(mask);
		return;
	} else if (!strcmp(prop_name, "mask_opened")) {
		GdkBitmap *mask = GTK_CTREE_ROW(node)->mask_opened;
		if (mask)
			*return_value = *php_gdk_bitmap_new(mask);
		return;
	} else if (!strcmp(prop_name, "level")) {
		RETURN_LONG(GTK_CTREE_ROW(node)->level);
	} else if (!strcmp(prop_name, "is_leaf")) {
		RETURN_BOOL(GTK_CTREE_ROW(node)->is_leaf);
	} else if (!strcmp(prop_name, "expanded")) {
		RETURN_BOOL(GTK_CTREE_ROW(node)->expanded);
	} else if (!strcmp(prop_name, "row")) {
		*return_value = *php_gtk_clist_row_new(&GTK_CTREE_ROW(node)->row);
		return;
	}

	*result = FAILURE;
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
	php_gtk_set_object(this_ptr, accel_group, (php_gtk_dtor_t)gtk_accel_group_unref, 1);
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
	{"GtkAccelGroup",	PHP_FN(gtkaccelgroup), NULL},
	{"lock", 			PHP_FN(gtk_accel_group_lock), NULL},
	{"unlock", 			PHP_FN(gtk_accel_group_unlock), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_accel_group_new(GtkAccelGroup *group)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!group) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)group, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gtk_accel_group_ce);
	gtk_accel_group_ref(group);
	php_gtk_set_object(result, group, (php_gtk_dtor_t)gtk_accel_group_unref, 1);

	return result;
}

static void release_gtk_accel_group_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GtkAccelGroup *obj = (GtkAccelGroup *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gtk_accel_group_unref(obj);
}


/* GtkStyle */
PHP_FUNCTION(gtkstyle)
{
	GtkStyle *ret;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	ret = gtk_style_new();
	php_gtk_set_object(this_ptr, ret, (php_gtk_dtor_t)gtk_style_unref, 1);
}

PHP_FUNCTION(gtk_style_copy)
{
	GtkStyle *style;

	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	style = gtk_style_copy(PHP_GTK_STYLE_GET(this_ptr));
	PHP_GTK_SEPARATE_RETURN(return_value, php_gtk_style_new(style));
	gtk_style_unref(style);
}

static function_entry php_gtk_style_functions[] = {
	{"GtkStyle", PHP_FN(gtkstyle), NULL},
	{"copy",	 PHP_FN(gtk_style_copy), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_style_new(GtkStyle *style)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!style) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)style, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gtk_style_ce);
	gtk_style_ref(style);
	php_gtk_set_object(result, style, (php_gtk_dtor_t)gtk_style_unref, 1);

	return result;
}

static void release_gtk_style_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GtkStyle *obj = (GtkStyle *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gtk_style_unref(obj);
}

typedef enum {
	STYLE_COLOR_ARRAY,
	STYLE_GC_ARRAY,
	STYLE_PIXMAP_ARRAY
} style_array_type;

static void style_helper_get(zval *result, style_array_type type, gpointer array)
{
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
                    add_next_index_zval(result, php_gdk_pixmap_new(pixmap_array[i]));
            }
            break;

        default:
            g_assert_not_reached();
            break;
    }
}

static void gtk_style_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GtkStyle *style = PHP_GTK_STYLE_GET(object);

	*result = SUCCESS;

	if (!strcmp(prop_name, "black")) {
		*return_value = *php_gdk_color_new(&style->black);
		return;
	} else if (!strcmp(prop_name, "white")) {
		*return_value = *php_gdk_color_new(&style->white);
		return;
	} else if (!strcmp(prop_name, "font")) {
		*return_value = *php_gdk_font_new(style->font);
		return;
	} else if (!strcmp(prop_name, "black_gc")) {
		*return_value = *php_gdk_gc_new(style->black_gc);
		return;
	} else if (!strcmp(prop_name, "white_gc")) {
		*return_value = *php_gdk_gc_new(style->white_gc);
		return;
	} else if (!strcmp(prop_name, "colormap")) {
		if (style->colormap)
			*return_value = *php_gdk_colormap_new(style->colormap);
		return;
	} else if (!strcmp(prop_name, "fg")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->fg);
		return;
	} else if (!strcmp(prop_name, "bg")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->bg);
		return;
	} else if (!strcmp(prop_name, "light")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->light);
		return;
	} else if (!strcmp(prop_name, "dark")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->dark);
		return;
	} else if (!strcmp(prop_name, "mid")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->mid);
		return;
	} else if (!strcmp(prop_name, "text")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->text);
		return;
	} else if (!strcmp(prop_name, "base")) {
		style_helper_get(return_value, STYLE_COLOR_ARRAY, style->base);
		return;
	} else if (!strcmp(prop_name, "fg_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->fg_gc);
		return;
	} else if (!strcmp(prop_name, "bg_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->bg_gc);
		return;
	} else if (!strcmp(prop_name, "light_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->light_gc);
		return;
	} else if (!strcmp(prop_name, "dark_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->dark_gc);
		return;
	} else if (!strcmp(prop_name, "mid_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->mid_gc);
		return;
	} else if (!strcmp(prop_name, "text_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->text_gc);
		return;
	} else if (!strcmp(prop_name, "base_gc")) {
		style_helper_get(return_value, STYLE_GC_ARRAY, style->base_gc);
		return;
	} else if (!strcmp(prop_name, "bg_pixmap")) {
		style_helper_get(return_value, STYLE_PIXMAP_ARRAY, style->bg_pixmap);
		return;
	}

	*result = FAILURE;
}

#if 0
static int style_helper_set(style_array_type type, gpointer array, zval *value, char *prop_name)
{
	zend_overloaded_element *property;

	if (next) {
		property = (zend_overloaded_element *)next->data;
		if (Z_TYPE_P(property) == OE_IS_ARRAY && Z_TYPE(property->element) == IS_LONG) {
			*element = next;
			prop_index = Z_LVAL(property->element);
			if (prop_index < 0 || prop_index >= 5) {
				php_error(E_WARNING, "style index out of range");
				return PG_ERROR;
			}

			switch (type) {
				case STYLE_COLOR_ARRAY:
					{
						GdkColor *color_array = (GdkColor *)array;
						if (!php_gtk_check_class(value, gdk_color_ce)) {
							php_error(E_WARNING, "can only assign a GdkColor");
							return PG_ERROR;
						}
						color_array[prop_index] = *PHP_GDK_COLOR_GET(value);
					}
					break;

				case STYLE_GC_ARRAY:
					{
						GdkGC **gc_array = (GdkGC **)array;
						if (!php_gtk_check_class(value, gdk_gc_ce)) {
							php_error(E_WARNING, "can only assign a GdkGC");
							return PG_ERROR;
						}
						if (gc_array[prop_index])
							gdk_gc_unref(gc_array[prop_index]);
						gc_array[prop_index] = gdk_gc_ref(PHP_GDK_GC_GET(value));
					}
					break;

				case STYLE_PIXMAP_ARRAY:
					{
						GdkWindow **pixmap_array = (GdkWindow **)array;
						if (!php_gtk_check_class(value, gdk_pixmap_ce) && Z_TYPE_P(value) != IS_NULL) {
							php_error(E_WARNING, "can only assign a GdkPixmap or null");
							return PG_ERROR;
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
		} else {
			php_error(E_WARNING, "property '%s' cannot be indexed by strings", prop_name);
			return PG_ERROR;
		}
	} else {
		php_error(E_WARNING, "cannot assign directly to '%s'", prop_name);
		return PG_ERROR;
	}

	return SUCCESS;
}
#endif

static int gtk_style_set_property(zval *object, char *prop_name, zval *value)
{
	GtkStyle *style = PHP_GTK_STYLE_GET(object);

	if (!strcmp(prop_name, "font")) {
		if (php_gtk_check_class(value, gdk_font_ce)) {
			if (style->font)
				gdk_font_unref(style->font);
			style->font = gdk_font_ref(PHP_GDK_FONT_GET(value));
		} else {
			php_error(E_WARNING, "'%s' property should be a GdkFont", prop_name);
			return PG_ERROR;
		}
	} else if (!strcmp(prop_name, "black")) {
		if (php_gtk_check_class(value, gdk_color_ce))
			style->black = *PHP_GDK_COLOR_GET(value);
		else {
			php_error(E_WARNING, "'%s' property should be a GdkColor", prop_name);
			return PG_ERROR;
		}
	} else if (!strcmp(prop_name, "white")) {
		if (php_gtk_check_class(value, gdk_color_ce))
			style->white = *PHP_GDK_COLOR_GET(value);
		else {
			php_error(E_WARNING, "'%s' property should be a GdkColor", prop_name);
			return PG_ERROR;
		}
	} else if (!strcmp(prop_name, "black_gc")) {
		if (php_gtk_check_class(value, gdk_gc_ce)) {
			if (style->black_gc)
				gdk_gc_unref(style->black_gc);
			style->black_gc = gdk_gc_ref(PHP_GDK_GC_GET(value));
		} else {
			php_error(E_WARNING, "'%s' property should be a GdkGC", prop_name);
			return PG_ERROR;
		}
	} else if (!strcmp(prop_name, "white_gc")) {
		if (php_gtk_check_class(value, gdk_gc_ce)) {
			if (style->white_gc)
				gdk_gc_unref(style->white_gc);
			style->white_gc = gdk_gc_ref(PHP_GDK_GC_GET(value));
		} else {
			php_error(E_WARNING, "'%s' property should be a GdkGC", prop_name);
			return PG_ERROR;
		}
	} else if (!strcmp(prop_name, "colormap")) {
		if (php_gtk_check_class(value, gdk_colormap_ce)) {
			if (style->colormap)
				gdk_colormap_unref(style->colormap);
			style->colormap = gdk_colormap_ref(PHP_GDK_COLORMAP_GET(value));
		} else {
			php_error(E_WARNING, "'%s' property should be a GdkColormap", prop_name);
			return PG_ERROR;
		}
	}
#if 0
	else if (!strcmp(prop_name, "fg")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->fg, value, prop_name);
	} else if (!strcmp(prop_name, "bg")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->bg, value, prop_name);
	} else if (!strcmp(prop_name, "light")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->light, value, prop_name);
	} else if (!strcmp(prop_name, "dark")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->dark, value, prop_name);
	} else if (!strcmp(prop_name, "mid")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->mid, value, prop_name);
	} else if (!strcmp(prop_name, "text")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->text, value, prop_name);
	} else if (!strcmp(prop_name, "base")) {
		return style_helper_set(STYLE_COLOR_ARRAY, style->base, value, prop_name);
	} else if (!strcmp(prop_name, "fg_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->fg_gc, value, prop_name);
	} else if (!strcmp(prop_name, "bg_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->bg_gc, value, prop_name);
	} else if (!strcmp(prop_name, "light_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->light_gc, value, prop_name);
	} else if (!strcmp(prop_name, "dark_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->dark_gc, value, prop_name);
	} else if (!strcmp(prop_name, "mid_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->mid_gc, value, prop_name);
	} else if (!strcmp(prop_name, "text_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->text_gc, value, prop_name);
	} else if (!strcmp(prop_name, "base_gc")) {
		return style_helper_set(STYLE_GC_ARRAY, style->base_gc, value, prop_name);
	} else if (!strcmp(prop_name, "bg_pixmap")) {
		return style_helper_set(STYLE_PIXMAP_ARRAY, style->bg_pixmap, value, prop_name);
	}
#endif
	else {
		return FAILURE;
	}

	return SUCCESS;
}


static function_entry php_gtk_box_child_functions[] = {
	{"GtkBoxChild", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_box_child_new(GtkBoxChild *box_child)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!box_child) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)box_child, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gtk_box_child_ce);
	php_gtk_set_object(result, box_child, NULL, 1);

	return result;
}

static void gtk_box_child_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GtkBoxChild *box_child = PHP_GTK_GET_GENERIC(object, GtkBoxChild*, le_php_gtk_wrapper);

	*result = SUCCESS;

	if (!strcmp(prop_name, "widget")) {
		*return_value = *php_gtk_new((GtkObject *)box_child->widget);
		return;
	} else if (!strcmp(prop_name, "padding")) {
		RETURN_LONG(box_child->padding);
	} else if (!strcmp(prop_name, "expand")) {
		RETURN_BOOL(box_child->expand);
	} else if (!strcmp(prop_name, "fill")) {
		RETURN_BOOL(box_child->fill);
	} else if (!strcmp(prop_name, "pack")) {
		RETURN_BOOL(box_child->pack);
	}

	*result = FAILURE;
}

/* GtkTableChild */
static function_entry php_gtk_table_child_functions[] = {
	{"GtkTableChild", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_table_child_new(GtkTableChild *table_child)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!table_child) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)table_child, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gtk_table_child_ce);
	php_gtk_set_object(result, table_child, NULL, 1);

	return result;
}

static void gtk_table_child_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GtkTableChild *table_child = PHP_GTK_GET_GENERIC(object, GtkTableChild*, le_php_gtk_wrapper);

	*result = SUCCESS;

	if (!strcmp(prop_name, "widget")) {
		*return_value = *php_gtk_new((GtkObject *)table_child->widget);
		return;
	} else if (!strcmp(prop_name, "left_attach")) {
		RETURN_LONG(table_child->left_attach);
	} else if (!strcmp(prop_name, "right_attach")) {
		RETURN_LONG(table_child->right_attach);
	} else if (!strcmp(prop_name, "top_attach")) {
		RETURN_LONG(table_child->top_attach);
	} else if (!strcmp(prop_name, "bottom_attach")) {
		RETURN_LONG(table_child->bottom_attach);
	} else if (!strcmp(prop_name, "xpadding")) {
		RETURN_LONG(table_child->xpadding);
	} else if (!strcmp(prop_name, "ypadding")) {
		RETURN_LONG(table_child->ypadding);
	} else if (!strcmp(prop_name, "xexpand")) {
		RETURN_BOOL(table_child->xexpand);
	} else if (!strcmp(prop_name, "yexpand")) {
		RETURN_BOOL(table_child->yexpand);
	} else if (!strcmp(prop_name, "xshrink")) {
		RETURN_BOOL(table_child->xshrink);
	} else if (!strcmp(prop_name, "yshrink")) {
		RETURN_BOOL(table_child->yshrink);
	} else if (!strcmp(prop_name, "xfill")) {
		RETURN_BOOL(table_child->xfill);
	} else if (!strcmp(prop_name, "yfill")) {
		RETURN_BOOL(table_child->yfill);
	}

	*result = FAILURE;
}

/* GtkFixedChild */
static function_entry php_gtk_fixed_child_functions[] = {
	{"GtkFixedChild", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_fixed_child_new(GtkFixedChild *fixed_child)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!fixed_child) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)fixed_child, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gtk_fixed_child_ce);
	php_gtk_set_object(result, fixed_child, NULL, 1);

	return result;
}

static void gtk_fixed_child_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GtkFixedChild *fixed_child = PHP_GTK_GET_GENERIC(object, GtkFixedChild*, le_php_gtk_wrapper);

	*result = SUCCESS;

	if (!strcmp(prop_name, "widget")) {
		*return_value = *php_gtk_new((GtkObject *)fixed_child->widget);
		return;
	} else if (!strcmp(prop_name, "x")) {
		RETURN_LONG(fixed_child->x);
	} else if (!strcmp(prop_name, "y")) {
		RETURN_LONG(fixed_child->y);
	}

	*result = FAILURE;
}


/* GtkCListRow */
static function_entry php_gtk_clist_row_functions[] = {
	{"GtkCListRow", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

zval *php_gtk_clist_row_new(GtkCListRow *clist_row)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!clist_row) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)clist_row, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gtk_clist_row_ce);
	php_gtk_set_object(result, clist_row, NULL, 1);

	return result;
}

static void gtk_clist_row_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	GtkCListRow *clist_row = PHP_GTK_GET_GENERIC(object, GtkCListRow*, le_php_gtk_wrapper);

	*result = SUCCESS;

	/* TODO add 'data' and 'cell' here */
	if (!strcmp(prop_name, "state")) {
		RETURN_LONG(clist_row->state);
	} else if (!strcmp(prop_name, "foreground")) {
		*return_value = *php_gdk_color_new(&clist_row->foreground);
		return;
	} else if (!strcmp(prop_name, "background")) {
		*return_value = *php_gdk_color_new(&clist_row->background);
		return;
	} else if (!strcmp(prop_name, "style")) {
		*return_value = *php_gtk_style_new(clist_row->style);
		return;
	} else if (!strcmp(prop_name, "fg_set")) {
		RETURN_BOOL(clist_row->fg_set);
	} else if (!strcmp(prop_name, "bg_set")) {
		RETURN_BOOL(clist_row->bg_set);
	} else if (!strcmp(prop_name, "selectable")) {
		RETURN_BOOL(clist_row->selectable);
	}
	
	*result = FAILURE;
}


void php_gtk_plus_register_types(int module_number)
{
    zend_class_entry ce;
	TSRMLS_FETCH();

    /*
	le_php_gtk_wrapper = zend_register_list_destructors_ex(NULL, NULL, "Generic PHP-GTK wrapper", module_number);
	le_gdk_window = zend_register_list_destructors_ex(release_gdk_window_rsrc, NULL, "GdkWindow", module_number);
	le_gdk_bitmap = zend_register_list_destructors_ex(release_gdk_bitmap_rsrc, NULL, "GdkBitmap", module_number);
	le_gdk_color = zend_register_list_destructors_ex(release_gdk_color_rsrc, NULL, "GdkColor", module_number);
	le_gdk_colormap = zend_register_list_destructors_ex(release_gdk_colormap_rsrc, NULL, "GdkColormap", module_number);
	le_gdk_cursor = zend_register_list_destructors_ex(release_gdk_cursor_rsrc, NULL, "GdkCursor", module_number);
	le_gdk_visual = zend_register_list_destructors_ex(release_gdk_visual_rsrc, NULL, "GdkVisual", module_number);
	le_gdk_font = zend_register_list_destructors_ex(release_gdk_font_rsrc, NULL, "GdkFont", module_number);
	le_gdk_gc = zend_register_list_destructors_ex(release_gdk_gc_rsrc, NULL, "GdkGC", module_number);
	le_gdk_drag_context = zend_register_list_destructors_ex(release_gdk_drag_context_rsrc, NULL, "GdkDragContext", module_number);
	le_gtk_accel_group = zend_register_list_destructors_ex(release_gtk_accel_group_rsrc, NULL, "GtkAccelGroup", module_number);
	le_gtk_style = zend_register_list_destructors_ex(release_gtk_style_rsrc, NULL, "GtkStyle", module_number);
    */

	gdk_event_ce = php_gtk_register_class("GdkEvent", php_gdk_event_functions, NULL, 0, 0, NULL TSRMLS_CC);

    gdk_window_ce = php_gtk_register_class("GdkWindow", php_gdk_window_functions, NULL, 1, 0, php_gdk_window_properties TSRMLS_CC);
	gdk_pixmap_ce = php_gtk_register_class("GdkPixmap", php_gdk_pixmap_functions, NULL, 1, 0, php_gdk_pixmap_properties TSRMLS_CC);
	gdk_bitmap_ce = php_gtk_register_class("GdkBitmap", php_gdk_bitmap_functions, NULL, 1, 0, php_gdk_bitmap_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gdk_window_ce, gdk_window_get_property);
	php_gtk_register_prop_getter(gdk_pixmap_ce, gdk_window_get_property);
	php_gtk_register_prop_getter(gdk_bitmap_ce, gdk_window_get_property);

	gdk_color_ce = php_gtk_register_class("GdkColor", php_gdk_color_functions, NULL, 1, 1, php_gdk_color_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gdk_color_ce, gdk_color_get_property);
	php_gtk_register_prop_setter(gdk_color_ce, gdk_color_set_property);

	gdk_colormap_ce = php_gtk_register_class("GdkColormap", php_gdk_colormap_functions, NULL, 1, 0, php_gdk_colormap_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gdk_colormap_ce, gdk_colormap_get_property);

	INIT_CLASS_ENTRY(ce, "GdkAtom", php_gdk_atom_functions);
	gdk_atom_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	
	gdk_cursor_ce = php_gtk_register_class("GdkCursor", php_gdk_cursor_functions, NULL, 1, 0, php_gdk_cursor_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gdk_cursor_ce, gdk_cursor_get_property);
	
	gdk_visual_ce = php_gtk_register_class("GdkVisual", php_gdk_visual_functions, NULL, 1, 0, php_gdk_visual_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gdk_visual_ce, gdk_visual_get_property);

	gdk_font_ce = php_gtk_register_class("GdkFont", php_gdk_font_functions, NULL, 1, 0, php_gdk_font_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gdk_font_ce, gdk_font_get_property);

	gdk_gc_ce = php_gtk_register_class("GdkGC", php_gdk_gc_functions, NULL, 1, 1, php_gdk_gc_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gdk_gc_ce, gdk_gc_get_property);
	php_gtk_register_prop_setter(gdk_gc_ce, gdk_gc_set_property);

	gdk_drag_context_ce = php_gtk_register_class("GdkDragContext", php_gdk_drag_context_functions, NULL, 1, 0, php_gdk_drag_context_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gdk_drag_context_ce, gdk_drag_context_get_property);

	gtk_selection_data_ce = php_gtk_register_class("GtkSelectionData", php_gtk_selection_data_functions, NULL, 1, 0, php_gtk_selection_data_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gtk_selection_data_ce, gtk_selection_data_get_property);

	gtk_ctree_node_ce = php_gtk_register_class("GtkCTreeNode", php_gtk_ctree_node_functions, NULL, 1, 0, php_gtk_ctree_node_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gtk_ctree_node_ce, gtk_ctree_node_get_property);

	gtk_accel_group_ce = php_gtk_register_class("GtkAccelGroup", php_gtk_accel_group_functions, NULL, 0, 0, NULL TSRMLS_CC);

	gtk_style_ce = php_gtk_register_class("GtkStyle", php_gtk_style_functions, NULL, 1, 1, php_gtk_style_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gtk_style_ce, gtk_style_get_property);
	php_gtk_register_prop_setter(gtk_style_ce, gtk_style_set_property);

	gtk_box_child_ce = php_gtk_register_class("GtkBoxChild", php_gtk_box_child_functions, NULL, 1, 0, php_gtk_box_child_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gtk_box_child_ce, gtk_box_child_get_property);

	gtk_table_child_ce = php_gtk_register_class("GtkTableChild", php_gtk_table_child_functions, NULL, 1, 0, php_gtk_table_child_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gtk_table_child_ce, gtk_table_child_get_property);

	gtk_fixed_child_ce = php_gtk_register_class("GtkFixedChild", php_gtk_fixed_child_functions, NULL, 1, 0, php_gtk_fixed_child_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gtk_fixed_child_ce, gtk_fixed_child_get_property);

	gtk_clist_row_ce = php_gtk_register_class("GtkCListRow", php_gtk_clist_row_functions, NULL, 1, 0, php_gtk_clist_row_properties TSRMLS_CC);
	php_gtk_register_prop_getter(gtk_clist_row_ce, gtk_clist_row_get_property);
}

#endif

/* }}} */

#endif

/* vim: set fdm=marker et sts=4: */
