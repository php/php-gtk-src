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

#include "php_gdkpixbuf.h"

#if HAVE_GDKPIXBUF

#ifdef PHP_GTK_COMPILE_DL_GDKPIXBUF
PHP_GTK_GET_EXTENSION(gdkpixbuf)
#endif

int le_gdk_pixbuf;

static void release_gdk_pixbuf_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkPixbuf *obj = (GdkPixbuf *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gdk_pixbuf_unref(obj); 
}

zval *php_gdk_pixbuf_new(GdkPixbuf *pixbuf)
{
	zval *result;
	zval **wrapper_ptr;
	TSRMLS_FETCH();

	if (!pixbuf) {
		MAKE_STD_ZVAL(result);
		ZVAL_NULL(result);
		return result;
	}

	if (zend_hash_index_find(&php_gtk_type_hash, (long)pixbuf, (void **)&wrapper_ptr) == SUCCESS) {
		zval_add_ref(wrapper_ptr);
		return *wrapper_ptr;
	}

	MAKE_STD_ZVAL(result);
	object_init_ex(result, gdk_pixbuf_ce);

	gdk_pixbuf_ref(pixbuf);
	php_gtk_set_object(result, pixbuf, le_gdk_pixbuf);

	return result;
}

PHP_GTK_XINIT_FUNCTION(gdkpixbuf)
{
	le_gdk_pixbuf = zend_register_list_destructors_ex(release_gdk_pixbuf_rsrc, NULL, "GdkPixbuf", module_number);

	php_gdk_pixbuf_register_constants(module_number TSRMLS_CC);
	php_gdk_pixbuf_register_classes();

	gdk_rgb_init();

	return SUCCESS;
}

php_gtk_ext_entry gdkpixbuf_ext_entry = {
	"gdkpixbuf",
	PHP_GTK_XINIT(gdkpixbuf),
	NULL,
};

#endif	/* HAVE_GDKPIXBUF */
