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
 
/* $Id$: */

#ifndef _PHP_GTK_MODULE_H
#define _PHP_GTK_MODULE_H

#include "ext/standard/info.h"
#include "ext/standard/php_array.h"

#if HAVE_PHP_GTK

extern zend_module_entry gtk_module_entry;
#define phpext_php_gtk_ptr &gtk_module_entry

PHP_MINIT_FUNCTION(gtk);
PHP_MSHUTDOWN_FUNCTION(gtk);
PHP_RINIT_FUNCTION(gtk);
PHP_RSHUTDOWN_FUNCTION(gtk);
PHP_MINFO_FUNCTION(gtk);

ZEND_BEGIN_MODULE_GLOBALS(gtk)
   char *codepage;
ZEND_END_MODULE_GLOBALS(gtk)

#ifdef ZTS
# define GTK_G(v) TSRMG(gtk_globals_id, zend_gtk_globals *, v)
#else
# define GTK_G(v)	(gtk_globals.v)
#endif

#else

#define phpext_php_gtk_ptr NULL

#endif /* HAVE_PHP_GTK */

#endif	/* _PHP_GTK_MODULE_H */
