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

#include "php_imlib.h"

#if HAVE_IMLIB

#ifdef PHP_GTK_COMPILE_DL_IMLIB
PHP_GTK_GET_EXTENSION(imlib)
#endif

PHP_GTK_XINIT_FUNCTION(imlib)
{
	
	
	 
	gdk_imlib_init();
	php_gdkimlib_register_constants(module_number TSRMLS_CC);
	
	php_gdkimlib_register_classes();
	php_imlib_register_types(module_number); 
	return SUCCESS;
}

php_gtk_ext_entry imlib_ext_entry = {
	"imlib",
	PHP_GTK_XINIT(imlib),
	NULL,
};
 
 
 
static void release_gdk_imlib_image_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	GdkImlibImage *obj = (GdkImlibImage *)rsrc->ptr;
	zval **wrapper_ptr;

	if (zend_hash_index_find(&php_gtk_type_hash, (long)obj, (void **)&wrapper_ptr) == SUCCESS) {
		zval_ptr_dtor(wrapper_ptr);
	}

	gdk_imlib_destroy_image(obj);
}
 
 
 
 
 
int le_php_gdk_imlib_image;
PHP_GTK_EXPORT_CE(gdk_imlib_image_ce);

static function_entry php_gdk_imlib_image_functions[] = {
	{"GdkImlibImage", PHP_FN(no_direct_constructor), NULL},
	{"gdkimlibimage", PHP_FN(no_direct_constructor), NULL},
	{NULL, NULL, NULL}
};

 


void php_imlib_register_types(int module_number)
{
	zend_class_entry ce;
	TSRMLS_FETCH();
	
	le_php_gdk_imlib_image = zend_register_list_destructors_ex(release_gdk_imlib_image_rsrc, NULL, "GdkImlibImage", module_number);
	INIT_CLASS_ENTRY(ce, "gdkimlibimage", php_gdk_imlib_image_functions);
	gdk_imlib_image_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	 

}
	

#endif	/* HAVE_IMLIB */
