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

PHP_GTK_API zend_class_entry *phpg_generic_exception = NULL;
PHP_GTK_API zend_class_entry *phpg_type_exception = NULL;

void phpg_register_exceptions()
{
	zend_class_entry ce;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(ce, "PhpGtkGenericException", NULL);
	phpg_generic_exception = zend_register_internal_class_ex(&ce, zend_exception_get_default(), NULL TSRMLS_CC);
	phpg_generic_exception->ce_flags |= ZEND_ACC_FINAL;
	phpg_generic_exception->constructor->common.fn_flags |= ZEND_ACC_PROTECTED;

	INIT_CLASS_ENTRY(ce, "PhpGtkTypeException", NULL);
	phpg_type_exception = zend_register_internal_class_ex(&ce, zend_exception_get_default(), NULL TSRMLS_CC);
	phpg_type_exception->ce_flags |= ZEND_ACC_FINAL;
	phpg_type_exception->constructor->common.fn_flags |= ZEND_ACC_PROTECTED;
}

#endif /* HAVE_PHP_GTK */
