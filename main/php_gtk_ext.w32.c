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

#if HAVE_PHP_GTK

#include "ext/gtk+/php_gtk+.h"
#include "ext/libglade/php_libglade.h"


php_gtk_ext_entry *php_gtk_extensions[] = {
	php_gtk_ext_libglade_ptr,
	php_gtk_ext_gtk__ptr,

};

#define EXTCOUNT (sizeof(php_gtk_extensions)/sizeof(php_gtk_ext_entry *))
	

int php_gtk_startup_all_extensions(int module_number)
{
	return php_gtk_startup_extensions(php_gtk_extensions, EXTCOUNT, module_number);
}

#endif /* HAVE_PHP_GTK */
