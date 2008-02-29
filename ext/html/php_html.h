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
 
/* $Id$ */

#ifndef PHP_HTML_H
#define PHP_HTML_H

#include "php_gtk.h"

#if HAVE_HTML

#include <gtkhtml/gtkhtml.h>

extern php_gtk_ext_entry html_ext_entry;
#define php_gtk_ext_html_ptr &html_ext_entry

void phpg_gtkhtml_register_classes(void);
void phpg_gtkhtml_register_constants(const char *strip_prefix);

#else

#define php_gtk_ext_html_ptr NULL

#endif	/* HAVE_HTML */

#endif	/* PHP_HTML_H */
