/*
 * PHP-GTK-HTML - The PHP language bindings for GTKHTML
 *
 * Copyright (C) 2001 Alan Knowles <alan_k@hklc.com>
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
 
 
#include "php_gtk.h"
#include "php_gtkhtml.h"
#include "SAPI.h"
#include <gtk/gtk.h>
#ifdef GTKHTML_HAVE_GCONF
#include <gconf/gconf.h>
#endif



#if HAVE_HTML

#ifdef PHP_GTK_COMPILE_DL_GTKHTML
PHP_GTK_GET_EXTENSION(gtkhtml)
#endif

PHP_GTK_XINIT_FUNCTION(gtkhtml)
{
#ifdef GTKHTML_HAVE_GCONF 
	GConfError *gconf_error;
	char **argv;	  
	
	argv = (char **)g_new(char *, 1);
	argv[0] = g_strdup(SG(request_info).path_translated);
	/* 	printf("argv0 %s", argv[0]); */
	if (!gconf_init(1, argv, &gconf_error))
		gconf_error_destroy(gconf_error);
	gconf_error = NULL;
#endif 

	gdk_rgb_init ();

	gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
	gtk_widget_set_default_visual (gdk_rgb_get_visual ());

	php_gtkhtml_register_constants(module_number TSRMLS_CC);
	php_gtkhtml_register_classes();

	return SUCCESS;
}

php_gtk_ext_entry gtkhtml_ext_entry = {
	"gtkhtml",
	PHP_GTK_XINIT(gtkhtml),
	NULL,
};

#endif	/* HAVE_HTML */
