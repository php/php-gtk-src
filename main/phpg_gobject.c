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

#include "php_gtk.h"

#if HAVE_PHP_GTK

/*
 * GObject API and helper functions
 */

static const gchar *gobject_class_id   = "phpg_class";
static GQuark       gobject_class_key  = 0;
static const gchar *gobject_wrapper_id   = "phpg_wrapper";
static GQuark       gobject_wrapper_key  = 0;

HashTable phpg_prop_info;
/*
 * TODO: remove?
HashTable php_gtk_prop_getters;
HashTable php_gtk_prop_setters;
HashTable php_gtk_rsrc_hash;
HashTable php_gtk_type_hash;
HashTable php_gtk_prop_desc;
HashTable php_gtk_callback_hash;
*/

/* {{{ static phpg_destroy_object() */
static inline void phpg_destroy_object(phpg_gobject_t *object, zend_object_handle handle TSRMLS_DC)
{
    zend_hash_destroy(object->zobj.properties);
    FREE_HASHTABLE(object->zobj.properties);
	if (object->obj && object->dtor)
		object->dtor(object->obj);
    efree(object);
}
/* }}} */

/* {{{ static phpg_create_object() */
static zend_object_value phpg_create_object(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value zov;
	phpg_gobject_t *object;

	object = emalloc(sizeof(phpg_gobject_t));
	phpg_init_object(object, ce);
	object->obj  = NULL;
	object->dtor = NULL;

	zov.handlers = &php_gtk_handlers;
	zov.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) phpg_destroy_object, NULL TSRMLS_CC);

	return zov;
}
/* }}} */

/* {{{ static phpg_class_from_gtype() */
static zend_class_entry* phpg_class_from_gtype(GType gtype)
{
	zend_class_entry *ce = NULL;

	while (gtype != G_TYPE_INVALID
		   && (ce = g_type_get_qdata(gtype, gobject_class_key)) == NULL) {
		gtype = g_type_parent(gtype);
	}
	
	g_assert(ce != NULL);
	return ce;
}
/* }}} */


/* {{{ phpg_read_property() */
zval *phpg_read_property(zval *object, zval *member, zend_bool silent TSRMLS_DC)
{
	phpg_head_t *poh;
	zval tmp_member;
	zval result, *result_ptr = NULL;
	prop_info_t *pi = NULL;
	int ret;

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	/* XXX test for propagation to parent classes */

	ret = FAILURE;

	poh = (phpg_head_t *) zend_objects_get_address(object TSRMLS_CC);
	if (poh->pi_hash) {
		ret = zend_hash_find(poh->pi_hash, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &pi);
	}

	if (ret == SUCCESS) {
		ret = pi->read(poh, &result);
		if (ret == SUCCESS) {
			ALLOC_ZVAL(result_ptr);
			*result_ptr = result;
            /* ensure we're creating a temporary variable */
            result_ptr->refcount = 0;
            result_ptr->is_ref = 0;
        } else {
            result_ptr = EG(uninitialized_zval_ptr);
        }
	} else {
		result_ptr = zend_get_std_object_handlers()->read_property(object, member, silent TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}

	return result_ptr;
}
/* }}} */

/* {{{ phpg_write_property() */
void phpg_write_property(zval *object, zval *member, zval *value TSRMLS_DC)
{
	phpg_head_t *poh;
	zval tmp_member;
	prop_info_t *pi;
	int ret;

 	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

    ret = FAILURE;
	poh = (phpg_head_t *) zend_objects_get_address(object TSRMLS_CC);
	if (poh->pi_hash) {
		ret = zend_hash_find(poh->pi_hash, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &pi);
	}

    if (ret == SUCCESS && pi->write != NULL) {
        pi->write(poh, value);
    } else {
		zend_get_std_object_handlers()->write_property(object, member, value TSRMLS_CC);
    }

	if (member == &tmp_member) {
		zval_dtor(member);
	}
}
/* }}} */

/* {{{ phpg_get_properties() */
HashTable* phpg_get_properties(zval *object TSRMLS_DC)
{
	zend_class_entry *ce;
	HashTable *pi_hash;
	phpg_head_t *poh;
	prop_info_t *pi;
	zval result, *result_ptr;
	int ret;

	/*
	 * Find the nearest internal parent class
	 */
	ce = Z_OBJCE_P(object);
	while (ce->type != ZEND_INTERNAL_CLASS && ce->parent != NULL) {
		ce = ce->parent;
	}

	poh = (phpg_head_t *)zend_objects_get_address(object TSRMLS_CC);
	for (; ce != NULL; ce = ce->parent) {
		if (zend_hash_find(&phpg_prop_info, ce->name, ce->name_length+1, (void **) &pi_hash) == SUCCESS) {
			for (zend_hash_internal_pointer_reset(pi_hash);
				 zend_hash_get_current_data(pi_hash, (void **)&pi) == SUCCESS;
				 zend_hash_move_forward(pi_hash)) {

				ret = pi->read(poh, &result);
				if (ret == SUCCESS) {
					ALLOC_ZVAL(result_ptr);
					*result_ptr = result;
					INIT_PZVAL(result_ptr);
					zend_hash_update(poh->zobj.properties, (char *)pi->name, strlen(pi->name)+1, &result_ptr, sizeof(zval *), NULL);
				}
			}
		}
	}

	return poh->zobj.properties;
}
/* }}} */


/* {{{ PHP_GTK_API phpg_init_object() */
PHP_GTK_API void phpg_init_object(void *object, zend_class_entry *ce)
{
	zval *tmp;
	zend_class_entry *prop_ce;
	phpg_head_t *poh = (phpg_head_t *) object;

	poh->zobj.ce = ce;
	poh->zobj.in_get = 0;
	poh->zobj.in_set = 0;
	poh->pi_hash = NULL;

	ALLOC_HASHTABLE(poh->zobj.properties);
	zend_hash_init(poh->zobj.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(poh->zobj.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	/*
	 * Find the nearest internal parent class
	 */
	prop_ce = ce;
	while (prop_ce->type != ZEND_INTERNAL_CLASS && prop_ce->parent != NULL) {
		prop_ce = prop_ce->parent;
	}

    zend_hash_find(&phpg_prop_info, prop_ce->name, prop_ce->name_length+1, (void **) &poh->pi_hash);
}
/* }}} */

/* {{{ PHP_GTK_API phpg_register_class() */
PHP_GTK_API zend_class_entry* phpg_register_class(const char *class_name,
                                                  function_entry *class_methods,
                                                  zend_class_entry *parent,
                                                  prop_info_t *prop_info,
                                                  create_object_func_t create_obj_func,
                                                  GType gtype
                                                  TSRMLS_DC)
{
	zend_class_entry ce, *real_ce;
	HashTable pi_hash;
	prop_info_t *pi;

	if (!gobject_class_key) {
		gobject_class_key = g_quark_from_static_string(gobject_class_id);
	}

	memset(&ce, 0, sizeof(ce));

	ce.name = strdup(class_name);
	ce.name_length = strlen(class_name);
	ce.builtin_functions = class_methods;
	if (create_obj_func) {
		ce.create_object = create_obj_func;
	} else {
		ce.create_object = phpg_create_object;
	}

	real_ce = zend_register_internal_class_ex(&ce, parent, NULL TSRMLS_CC);

    if (prop_info) {
        pi = prop_info;
        if (pi->name) {
            zend_hash_init(&pi_hash, 1, NULL, NULL, 0);

            /*
             * Only register properties with reader functions.
             */
            while (pi->name && pi->read) {
                zend_hash_update(&pi_hash, (char *)pi->name, strlen(pi->name)+1, pi, sizeof(prop_info_t), NULL);
                pi++;
            }

            zend_hash_add(&phpg_prop_info, ce.name, ce.name_length+1, &pi_hash, sizeof(HashTable), NULL);
        }

        /*
           if (zend_hash_index_find(&php_gtk_prop_desc, (long)real_ce->parent, (void **)&parent_prop_desc) == SUCCESS) {
           if (parent_prop_desc->have_getter)
           prop_desc.have_getter = 1;
           if (parent_prop_desc->have_setter)
           prop_desc.have_setter = 1;
           }
           zend_hash_index_update(&php_gtk_prop_desc, (long)real_ce, (void *)&prop_desc, sizeof(prop_desc_t), NULL);
         */
    }

    if (gtype) {
        /* TODO store __gtype object */
        g_type_set_qdata(gtype, gobject_class_key, real_ce);
    }

	return real_ce;
}
/* }}} */

/* {{{ PHP_GTK_API phpg_gobject_new() */
PHP_GTK_API zval* phpg_gobject_new(GObject *obj)
{
	zval *zobj = NULL;
	zend_class_entry *ce = NULL;
	phpg_gobject_t *pobj = NULL;
    zend_object_handle handle;
	TSRMLS_FETCH();

	if (!gobject_wrapper_key) {
		gobject_wrapper_key = g_quark_from_static_string(gobject_wrapper_id);
	}

	MAKE_STD_ZVAL(zobj);
	ZVAL_NULL(zobj);

	if (obj == NULL) {
		return zobj;
	}

	/* TODO Need to
	 * 1. Lookup stored object based on wrapper key.
	 * 2. Make zval refer to stored object.
	 * 3. If not found, create new object and store it.
	 * 4. Install wrapper key.
	 */

	handle = (zend_object_handle)g_object_get_qdata(obj, gobject_wrapper_key);
	if ((void*)handle != NULL) {
		Z_TYPE_P(zobj) = IS_OBJECT;
		Z_OBJ_HANDLE_P(zobj) = handle;
		Z_OBJ_HT_P(zobj) = &php_gtk_handlers;
		zend_objects_store_add_ref(zobj TSRMLS_CC);
	} else {
		ce = phpg_class_from_gtype(G_OBJECT_TYPE(obj));
		object_init_ex(zobj, ce);
		g_object_ref(obj);

		/* TODO turn into a sink function */
		if (GTK_OBJECT_FLOATING(obj)) {
			g_object_ref(obj);
			gtk_object_sink(GTK_OBJECT(obj));
		}

		pobj = zend_object_store_get_object(zobj TSRMLS_CC);
		pobj->obj = obj;
		pobj->dtor = (phpg_dtor_t) gtk_object_unref;
		g_object_set_qdata(obj, gobject_wrapper_key, (void*)handle);
	}

	return zobj;
}
/* }}} */

/*
 * GObject PHP class definition
 */

static PHP_METHOD(GObject, __construct) {}

static zend_function_entry gobject_methods[] = {
	ZEND_ME(GObject, __construct, NULL, ZEND_ACC_PRIVATE)
	{NULL, NULL, NULL}
};

PHP_GTK_EXPORT_CE(gobject_ce) = NULL;

void phpg_gobject_register_self()
{
	if (gobject_ce) return;

	gobject_ce = phpg_register_class("GObject", gobject_methods, NULL, NULL, NULL, 0 TSRMLS_CC);
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et : */
