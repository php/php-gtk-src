/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001-2004 Andrei Zmievski <andrei@php.net>
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

#if HAVE_PHP_GTK

/*
 * Closures API and helper functions
 */

typedef struct {
	GClosure closure;
	zval *callback;
	zval *user_args;
	zend_bool use_signal_object;
} phpg_closure_t;

static void phpg_closure_invalidate(gpointer data, GClosure *closure)
{
}

PHP_GTK_API GClosure* phpg_closure_new(zval *callback, zval *user_args, zend_bool use_signal_object)
{
}

#endif /* HAVE_PHP_GTK */

/* vim: set fdm=marker et sts=4: */

