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

GHashTable *php_gtk_class_hash = NULL;

zend_module_entry gtk_module_entry = {
	"gtk",
	NULL,
	PHP_MINIT(gtk),
	PHP_MSHUTDOWN(gtk),
	PHP_RINIT(gtk),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(gtk),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(gtk),
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHP_GTK
ZEND_GET_MODULE(gtk)
#endif


static void init_gtk(void)
{
	HashTable *symbol_table;
	zval **z_argv, **z_argc, **entry;
	zval *tmp;
	char **argv;
	int argc, i;
	zend_bool no_argc = 0;
	PLS_FETCH();
	ELS_FETCH();
	SLS_FETCH();

	/* We check request_method to see if we've been called from command line or
	   Web server. Running GUI apps through a Web module can be dangerous to
	   your health. */
	if (SG(request_info).request_method != NULL) {
		php_error(E_ERROR, "php-gtk: PHP GTK+ support is not available under Web servers");
		return;
	}
 
	/*
	 * Since track_vars is always on, we just get the argc/argv values from
	 * there.
	 */
	symbol_table = PG(http_globals)[TRACK_VARS_SERVER]->value.ht;

	zend_hash_find(&EG(symbol_table), "argc", sizeof("argc"), (void **)&z_argc);
	zend_hash_find(symbol_table, "argv", sizeof("argv"), (void **)&z_argv);
	if (Z_TYPE_PP(z_argc) != IS_LONG || Z_TYPE_PP(z_argv) != IS_ARRAY) {
		php_error(E_ERROR, "php-gtk: argc/argv are corrupted");
	}

	argc = Z_LVAL_PP(z_argc);

	/*
	 * If the script was called via -f switch and no further arguments were
	 * given, argc will be 0 and that's not good for gtk_init_check(). We use
	 * the path to the script as the only argument, and remember that we won't
	 * have to update symbol table after gtk_init_check().
	 */
	if (argc == 0) {
		argc = 1;
		no_argc = 1;
		argv = (char **)g_new(char *, argc);
		argv[0] = g_strdup(SG(request_info).path_translated);
	} else {
		argv = (char **)g_new(char *, argc);
		i = 0;
		for (zend_hash_internal_pointer_reset(Z_ARRVAL_PP(z_argv));
			 zend_hash_get_current_data(Z_ARRVAL_PP(z_argv), (void **)&entry) == SUCCESS;
			 zend_hash_move_forward(Z_ARRVAL_PP(z_argv))) {
			argv[i++] = g_strndup(Z_STRVAL_PP(entry), Z_STRLEN_PP(entry));
		}
	}
	
	/* Ok, this is a hack. GTK+/GDK calls g_atexit() to set up a couple of cleanup
	   functions that should be called when the main program exits. However, if
	   we're compiled as .so library, libgtk.so will be unloaded first and the
	   pointers to the cleanup functions will be invalid. So we load libgtk.so
	   one more time to make sure it stays in memory even after our .so is
	   unloaded. */
	//DL_LOAD("libgtk.so");
			   
	if (!gtk_init_check(&argc, &argv)) {
		if (argv != NULL) {
			for (i = 0; i < argc; i++)
				g_free(argv[i]);
			g_free(argv);
		}
		php_error(E_ERROR, "php-gtk: Could not open display");
		return;
	}

	if (no_argc) {
		/* The -f switch case, simple. */
		g_free(argv[0]);
	} else {
		/* We always clean the argv array. */
		zend_hash_clean(Z_ARRVAL_PP(z_argv));

		/* Then if there are any arguments left after processing with
		   gtk_init_check(), we put them back into PHP's argv array and update
		   argc as well. */
		if (argv != NULL) {
			for (i = 0; i < argc; i++) {
				ALLOC_ZVAL(tmp);
				tmp->type = IS_STRING;
				tmp->value.str.len = strlen(argv[i]);
				tmp->value.str.val = estrndup(argv[i], tmp->value.str.len);
				INIT_PZVAL(tmp);
				zend_hash_next_index_insert(Z_ARRVAL_PP(z_argv), &tmp, sizeof(zval *), NULL);
			}
			g_free(argv);

			Z_LVAL_PP(z_argc) = argc;
		}
	}
}

/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
PHP_INI_END()
*/

static void release_gtk_object_rsrc(zend_rsrc_list_entry *rsrc)
{
	GtkObject *obj = (GtkObject *)rsrc->ptr;
	gtk_object_unref(obj);
}

PHP_MINIT_FUNCTION(gtk)
{
//	ZEND_INIT_MODULE_GLOBALS(gtk, NULL, NULL);

/* Remove comments if you have entries in php.ini
	REGISTER_INI_ENTRIES();
*/

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(gtk)
{
/* Remove comments if you have entries in php.ini
	UNREGISTER_INI_ENTRIES();
*/
	return SUCCESS;
}

/* Remove if there's nothing to do at request start */
PHP_RINIT_FUNCTION(gtk)
{
	le_gtk = zend_register_list_destructors_ex(release_gtk_object_rsrc, NULL, "GtkObject", module_number);

	php_gtk_class_hash = g_hash_table_new(g_str_hash, g_str_equal);
	zend_hash_init_ex(&php_gtk_prop_getters, 20, NULL, NULL, 1, 0);
	zend_hash_init_ex(&php_gtk_prop_setters, 20, NULL, NULL, 1, 0);
	php_gtk_register_classes();
	php_gtk_register_types(module_number);
	php_gtk_register_constants(module_number ELS_CC);
	
	init_gtk();

	zend_unset_timeout();
	zend_set_timeout(0);

	return SUCCESS;
}

/* Remove if there's nothing to do at request end */
PHP_RSHUTDOWN_FUNCTION(gtk)
{
	zend_hash_destroy(&php_gtk_prop_getters);
	zend_hash_destroy(&php_gtk_prop_setters);
	gtk_exit(0);

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

#endif	/* HAVE_PHP_GTK */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
