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

#include "php_scrollpane.h"

#if HAVE_SCROLLPANE

#ifdef PHP_GTK_COMPILE_DL_SCROLLPANE
PHP_GTK_GET_EXTENSION(scrollpane)
#endif

PHP_GTK_XINIT_FUNCTION(scrollpane)
{
	php_scrollpane_register_constants(module_number TSRMLS_CC);
	php_scrollpane_register_classes();

	return SUCCESS;
}

php_gtk_ext_entry scrollpane_ext_entry = {
	"scrollpane",
	PHP_GTK_XINIT(scrollpane),
	NULL,
};

#endif	/* HAVE_SCROLLPANE */
