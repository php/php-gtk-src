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

#ifndef PHP_IMLIB_H
#define PHP_IMLIB_H

#include "php_gtk.h"

#if HAVE_IMLIB

#include "gen_ce_imlib.h"
#include "gdk_imlib.h"

extern php_gtk_ext_entry imlib_ext_entry;
#define imlib_ext_ptr &imlib_ext_entry

void php_imlib_register_constants(int module_number TSRMLS_DC);
void php_imlib_register_classes();
void php_imlib_register_types(int module_number);


extern int le_php_gdk_imlib_image;
#define PHP_GDK_IMLIB_IMAGE_GET(w)        PHP_GTK_GET_GENERIC(w, GdkImlibImage*, le_php_gdk_imlib_image)
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_imlib_image_ce);


extern int le_php_gdk_imlib_render;
#define PHP_GDK_IMLIB_RENDER_GET(w)        PHP_GTK_GET_GENERIC(w, GdkImlibRender*, le_php_gdk_imlib_render)
PHP_GTK_API extern PHP_GTK_EXPORT_CE(gdk_imlib_render_ce);



#else

#define imlib_ext_ptr NULL

#endif	/* HAVE_IMLIB */

#define php_gtk_ext_imlib_ptr imlib_ext_ptr

#endif	/* PHP_IMLIB_H */
