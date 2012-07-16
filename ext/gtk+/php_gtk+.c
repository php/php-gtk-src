/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001-2008 Andrei Zmievski <andrei@php.net>
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

#include "php_gtk+.h"

PHP_GTK_API int le_gtk_object;
PHP_GTK_API zend_class_entry *php_gtk_exception_ce;
extern zend_class_entry *gtk_ce;
extern zend_class_entry *gdk_ce;

/* defined in php_gdk.c */
void phpg_gdk_register_keysyms();

/* TODO check pygtk version */
static void init_gtk(TSRMLS_C)
{
	HashTable *symbol_table;
	zval **z_argv = NULL, **z_argc = NULL, **entry;
	zval *tmp;
	char **argv = NULL;
	int argc, i;
	zend_bool no_argc = 0;

	/*
	 * Grab the argc/argv values from $_SERVER array.
	 */
	zend_is_auto_global("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);
	symbol_table = PG(http_globals)[TRACK_VARS_SERVER]->value.ht;
	zend_hash_find(symbol_table, "argc", sizeof("argc"), (void **)&z_argc);
	zend_hash_find(symbol_table, "argv", sizeof("argv"), (void **)&z_argv);
	if (!z_argc || !z_argv || Z_TYPE_PP(z_argc) != IS_LONG || Z_TYPE_PP(z_argv) != IS_ARRAY) {
		argc = 0;
	} else {
		argc = Z_LVAL_PP(z_argc);
	}

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
		if (SG(request_info).path_translated)
			argv[0] = g_strdup(SG(request_info).path_translated);
		else
			argv[0] = g_strdup("-");
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

	/*
	   We must always call gtk_set_locale() in order to get GTK+/GDK
	   correctly initialize multilingual support. Otherwise, application
	   will refuse any letters outside ASCII and font metrics will
	   be broken.
	 */
	gtk_set_locale();

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

/*
static void register_exception(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "PHPGTKException", NULL);
	php_gtk_exception_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
}
*/

static void phpg_register_stock_constants()
{
	GSList *stock_ids, *item;
	char buf[128];
	char *id, *p;
	int i, n;

	stock_ids = gtk_stock_list_ids();

	item = stock_ids;
	while (item) {
		id = (char *)item->data;
		p = strchr(id, '-');
		if (p) {
			n = snprintf(buf, sizeof(buf), "STOCK%s", p);
			for (i = sizeof("STOCK")-1; buf[i]; i++) {
				if (buf[i] == '-')
					buf[i] ='_';
				else
					buf[i] = toupper(buf[i]);
			}
			phpg_register_string_constant(gtk_ce, buf, n, id, strlen(id));
		}
		g_free(id);
		item = item->next;
	}

	g_slist_free(stock_ids);
}

#define register_atom(name) \
	{ \
		gchar *__atom_name = gdk_atom_name((GDK_##name)); \
		phpg_register_string_constant(gdk_ce, #name, strlen(#name), __atom_name, strlen(__atom_name)); \
		g_free(__atom_name); \
	}
static void phpg_register_atoms()
{
	register_atom(SELECTION_PRIMARY);
    register_atom(SELECTION_SECONDARY);
    register_atom(SELECTION_CLIPBOARD);
    register_atom(TARGET_BITMAP);
    register_atom(TARGET_COLORMAP);
    register_atom(TARGET_DRAWABLE);
    register_atom(TARGET_PIXMAP);
    register_atom(TARGET_STRING);
    register_atom(SELECTION_TYPE_ATOM);
    register_atom(SELECTION_TYPE_BITMAP);
    register_atom(SELECTION_TYPE_COLORMAP);
    register_atom(SELECTION_TYPE_DRAWABLE);
    register_atom(SELECTION_TYPE_INTEGER);
    register_atom(SELECTION_TYPE_PIXMAP);
    register_atom(SELECTION_TYPE_WINDOW);
    register_atom(SELECTION_TYPE_STRING);
}
#undef register_atom

PHP_GTK_XINIT_FUNCTION(gtk_plus)
{
	//register_exception(TSRMLS_C);

	init_gtk(TSRMLS_C);
	phpg_gtk_register_classes(TSRMLS_C);
	phpg_gdk_register_classes(TSRMLS_C);
	phpg_atk_register_classes(TSRMLS_C);
	phpg_pango_register_classes(TSRMLS_C);

	phpg_atk_register_constants("ATK_" TSRMLS_CC);
	phpg_pango_register_constants("PANGO_" TSRMLS_CC);
	phpg_gdk_register_constants("GDK_" TSRMLS_CC);
	phpg_gtk_register_constants("GTK_" TSRMLS_CC);

	phpg_register_stock_constants();
	phpg_gdk_register_keysyms();
	phpg_register_atoms();

	php_gtk_plus_register_types(TSRMLS_C);

	return SUCCESS;
}

PHP_GTK_XSHUTDOWN_FUNCTION(gtk_plus)
{
	//gtk_exit(0);

	return SUCCESS;
}

php_gtk_ext_entry gtk_plus_ext_entry = {
	"gtk+",
	PHP_GTK_XINIT(gtk_plus),
	PHP_GTK_XSHUTDOWN(gtk_plus)
};

#endif	/* HAVE_PHP_GTK */
