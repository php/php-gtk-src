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

#include "php_html.h"

#if HAVE_HTML

#ifdef PHP_GTK_COMPILE_DL_HTML
PHP_GTK_GET_EXTENSION(html)
#endif

PHP_GTK_XINIT_FUNCTION(html)
{
	phpg_gtkhtml_register_classes();
    phpg_gtkhtml_register_constants("HTML_");
	
	return SUCCESS;
}

php_gtk_ext_entry html_ext_entry = {
	"html",
	PHP_GTK_XINIT(html),
	NULL,
};

#endif	/* HAVE_HTML */
