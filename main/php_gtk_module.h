/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Andrei Zmievski <andrei@php.net>                            |
   +----------------------------------------------------------------------+
 */
 
/* $Id$: */

#ifndef _PHP_GTK_MODULE_H
#define _PHP_GTK_MODULE_H

#include "php.h"
#include "php_ini.h"
#ifdef PHP_WIN32
#include "config.w32.h"
#else
#include "php_config.h"
#endif

#include "ext/standard/info.h"
#include "ext/standard/php_array.h"

#if HAVE_PHP_GTK

extern zend_module_entry gtk_module_entry;
#define phpext_php_gtk_ptr &gtk_module_entry


#ifdef PHP_WIN32
#define PHP_GTK_API __declspec(dllexport)
#else
#define PHP_GTK_API
#endif

PHP_MINIT_FUNCTION(gtk);
PHP_MSHUTDOWN_FUNCTION(gtk);
PHP_RINIT_FUNCTION(gtk);
PHP_RSHUTDOWN_FUNCTION(gtk);
PHP_MINFO_FUNCTION(gtk);

typedef struct {
	int dummy;
} php_gtk_globals;

#ifdef ZTS
#define GTKG(v) (gtk_globals->v)
#define GTKLS_FETCH() php_gtk_globals *gtk_globals = ts_resource(gd_gtk_id)
#else
#define GTKG(v) (gtk_globals.v)
#define GTKLS_FETCH()
#endif

#else

#define phpext_php_gtk_ptr NULL

#endif /* HAVE_PHP_GTK */

#endif	/* _PHP_GTK_MODULE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
