 
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
