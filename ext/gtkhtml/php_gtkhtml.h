/*
* PHP-GTK-HTML - The PHP language bindings for GTKHTML
 *
 * Copyright (C) 2001,2002 Alan Knowles <alan_k@hklc.com>
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
 
/* $Id$: */

#ifndef PHP_HTML_H
#define PHP_HTML_H

#include "php_gtk.h"

#if HAVE_HTML

#include <gtkhtml/gtkhtml.h>
#include "gen_ce_gtkhtml.h"

extern int le_gtkhtmlstream;

extern php_gtk_ext_entry gtkhtml_ext_entry;

#define gtkhtml_ext_ptr &gtkhtml_ext_entry
#define php_gtk_ext_gtkhtml_ptr gtkhtml_ext_ptr



typedef struct {
	GtkHTMLStream           *gs;
	long                     id;
} php_gtkhtmlstream_s;



#else

#define gtkhtml_ext_ptr NULL
#define php_gtk_ext_gtkhtml_ptr gtkhtml_ext_ptr

#endif	/* HAVE_HTML */

#endif /* PHP_HTML_H */
