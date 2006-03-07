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

#include "php_mozembed.h"

#if HAVE_MOZEMBED

#ifdef COMPILE_DL_MOZEMBED
PHP_GTK_GET_EXTENSION(mozembed)
#endif

PHP_GTK_XINIT_FUNCTION(mozembed)
{
    phpg_gtkmozembed_register_classes();
	phpg_gtkmozembed_register_constants("GTK_MOZEMBED_");
	
	return SUCCESS;
}

php_gtk_ext_entry mozembed_ext_entry = {
	"mozembed",
	PHP_GTK_XINIT(mozembed),
	NULL,
};

#endif	/* HAVE_MOZEMBED */
