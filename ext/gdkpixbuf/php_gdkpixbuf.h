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

#ifndef PHP_GDKPIXBUF_H
#define PHP_GDKPIXBUF_H

#include "php_gtk.h"

#if HAVE_GDKPIXBUF

#include <gdk-pixbuf/gdk-pixbuf.h>

extern int le_gdk_pixbuf;

#include "gen_ce_gdkpixbuf.h"

extern php_gtk_ext_entry gdkpixbuf_ext_entry;
#define gdkpixbuf_ext_ptr &gdkpixbuf_ext_entry

void php_gdk_pixbuf_register_constants(int module_number TSRMLS_DC);
void php_gdk_pixbuf_register_classes();
zval *php_gdk_pixbuf_new(GdkPixbuf *pixbuf);

#define PHP_GDK_PIXBUF_GET(w)	PHP_GTK_GET_GENERIC(w, GdkPixbuf*, le_gdk_pixbuf)

#else

#define gdkpixbuf_ext_ptr NULL

#endif	/* HAVE_GDKPIXBUF */

#define php_gtk_ext_gdkpixbuf_ptr gdkpixbuf_ext_ptr

#endif	/* PHP_GDKPIXBUF_H */
