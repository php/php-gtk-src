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
 
/* $Id$: */

#include "php_gtk.h"
#include "SAPI.h"

#if HAVE_PHP_GTK

HashTable php_gtk_ext_registry;
GHashTable *php_gtk_class_hash = NULL;

zend_module_entry gtk_module_entry = {
	"php-gtk",
	NULL,
	PHP_MINIT(gtk),
	PHP_MSHUTDOWN(gtk),
	PHP_RINIT(gtk),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(gtk),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(gtk),
	STANDARD_MODULE_PROPERTIES
};

ZEND_DECLARE_MODULE_GLOBALS(gtk)

#ifdef COMPILE_DL_PHP_GTK
ZEND_GET_MODULE(gtk)
#endif

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY    ("gtk.codepage",	"CP1252",	PHP_INI_SYSTEM,	OnUpdateString, codepage,	zend_gtk_globals,	gtk_globals)
PHP_INI_END()


PHP_MINIT_FUNCTION(gtk)
{
	ZEND_INIT_MODULE_GLOBALS(gtk, NULL, NULL);

	REGISTER_INI_ENTRIES();

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(gtk)
{
/* Remove comments if you have entries in php.ini
	UNREGISTER_INI_ENTRIES();
*/
	return SUCCESS;
}

static void php_gtk_ext_destructor(php_gtk_ext_entry *ext)
{
	if (ext->ext_started && ext->ext_shutdown_func)
		ext->ext_shutdown_func();
	ext->ext_started = 0;
}

/* Remove if there's nothing to do at request start */
PHP_RINIT_FUNCTION(gtk)
{
	zend_hash_init_ex(&php_gtk_ext_registry, 10, NULL, (void (*)(void *))php_gtk_ext_destructor, 1, 0);

	php_gtk_class_hash = g_hash_table_new(g_str_hash, g_str_equal);
	zend_hash_init_ex(&php_gtk_prop_getters, 20, NULL, NULL, 1, 0);
	zend_hash_init_ex(&php_gtk_prop_setters, 20, NULL, NULL, 1, 0);
	zend_hash_init_ex(&php_gtk_rsrc_hash, 50, NULL, NULL, 1, 0);
	zend_hash_init_ex(&php_gtk_type_hash, 50, NULL, NULL, 1, 0);
	
	zend_unset_timeout(TSRMLS_C);
	zend_set_timeout(0);

	if (php_gtk_startup_all_extensions(module_number) == FAILURE) {
		php_error(E_WARNING, "Unable to start extensions");
		return FAILURE;
	}

	return SUCCESS;
}

/* Remove if there's nothing to do at request end */
PHP_RSHUTDOWN_FUNCTION(gtk)
{
	zend_hash_destroy(&php_gtk_prop_getters);
	zend_hash_destroy(&php_gtk_prop_setters);
	zend_hash_destroy(&php_gtk_rsrc_hash);
	zend_hash_destroy(&php_gtk_type_hash);

	zend_hash_destroy(&php_gtk_ext_registry);

	return SUCCESS;
}

PHP_MINFO_FUNCTION(gtk)
{
	char buf[32];

	php_info_print_table_start();
	php_info_print_table_row(2, "GTK+ support", "enabled");
	sprintf(buf, "%d.%d.%d", gtk_major_version, gtk_minor_version, gtk_micro_version);
	php_info_print_table_row(2, "GTK+ v", buf);
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}

PHP_FUNCTION(no_constructor)
{
 	php_error(E_WARNING, "%s: An abstract or unimplemented class", get_active_function_name(TSRMLS_C));
	php_gtk_invalidate(this_ptr);
}

PHP_FUNCTION(no_direct_constructor)
{
	php_error(E_WARNING, "Class %s cannot be constructed directly", get_active_function_name(TSRMLS_C));
	php_gtk_invalidate(this_ptr);
}

static int php_gtk_startup_extension(php_gtk_ext_entry *ext, int module_number)
{
	TSRMLS_FETCH();

	if (ext) {
		if (ext->ext_startup_func) {
			if (ext->ext_startup_func(module_number TSRMLS_CC) == FAILURE) {
				php_error(E_WARNING, "Unable to start '%s' PHP-GTK extension", ext->name);
				return FAILURE;
			}
		}
		ext->ext_started = 1;
		zend_hash_add(&php_gtk_ext_registry, ext->name, strlen(ext->name)+1,
					  (void *)ext, sizeof(php_gtk_ext_entry), NULL);
	}

	return SUCCESS;
}

int php_gtk_startup_extensions(php_gtk_ext_entry **ext, int ext_count, int module_number)
{
	php_gtk_ext_entry **end = ext + ext_count;

	while (ext < end) {
		if (*ext) {
			if (php_gtk_startup_extension(*ext, module_number)==FAILURE) {
				return FAILURE;
			}
		}
		ext++;
	}

	return SUCCESS;
}

#endif	/* HAVE_PHP_GTK */
