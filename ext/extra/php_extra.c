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

#include "php_extra.h"

#if HAVE_EXTRA

#ifdef PHP_GTK_COMPILE_DL_EXTRA
PHP_GTK_GET_EXTENSION(extra)
#endif

  
gint php_gtk_sheet_callback_traverse(GtkWidget *widget,
                    gint row, gint col, gint *new_row, gint *new_col,
                    gpointer data)
{
	zval *zrow, *zcol, *znew_row, *znew_col;
	zval *hash, *retval;
	
	 
	MAKE_STD_ZVAL(hash);
	array_init(hash);
	
	
	MAKE_STD_ZVAL(zrow); 
	ZVAL_LONG(zrow, 	 row );
	zend_hash_next_index_insert(Z_ARRVAL_P(hash), &zrow, sizeof(zval *), NULL);
	
	MAKE_STD_ZVAL(zcol); 	 
	ZVAL_LONG(zcol, 	 col );
	zend_hash_next_index_insert(Z_ARRVAL_P(hash), &zcol, sizeof(zval *), NULL);
	
	MAKE_STD_ZVAL(znew_row); 
	ZVAL_LONG(znew_row, 	 *new_row );
	zend_hash_next_index_insert(Z_ARRVAL_P(hash), &znew_row, sizeof(zval *), NULL);
	
	MAKE_STD_ZVAL(znew_col);
	ZVAL_LONG(znew_col, 	 *new_col );
	zend_hash_next_index_insert(Z_ARRVAL_P(hash), &znew_col, sizeof(zval *), NULL);
	retval = php_gtk_simple_signal_callback((GtkObject *) widget, data, hash);
	
	
	return 1;
}
PHP_GTK_XINIT_FUNCTION(extra)
{
	php_extra_register_constants(module_number TSRMLS_CC);
	php_extra_register_classes();
	 
	php_gtk_register_callback("GtkSheet::traverse",(GtkSignalFunc) php_gtk_sheet_callback_traverse);
	return SUCCESS;
}

php_gtk_ext_entry extra_ext_entry = {
	"extra",
	PHP_GTK_XINIT(extra),
	NULL,
};

#endif	/* HAVE_EXTRA */
