<?php
/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001 Andrei Zmievski <andrei@php.net>
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

/*======================================================================*\
	Function: convert_typename
	Purpose:  Converts a typename to the uppercased and underscored
			  version used for type conversion macros and enum/flag
			  names.
\*======================================================================*/
function convert_typename($typename)
{
	preg_match_all('![A-Z]{1,2}[^A-Z]*!', $typename, $match);
	return '_' . strtoupper(implode('_', $match[0]));
}

/*======================================================================*\
	Function: enum_name
	Purpose:  Creates a GTK_TYPE_* name from the give typename
\*======================================================================*/
function enum_name($typename)
{
	$proper = convert_typename($typename);
	if (strlen($proper) > 4 && substr($proper, 0, 4) == '_GTK')
		return 'GTK_TYPE' . substr($proper, 4);
	else
		return 'GTK_TYPE' . $proper;
}

/*======================================================================*\
	Class:   Var_List
	Purpose: Format C variable list 
\*======================================================================*/
class Var_List {
	var $vars = array();

	function add($c_type, $name)
	{
		$this->vars[$c_type][] = $name;
	}

	function to_string()
	{
		$result = array();
		foreach (array_keys($this->vars) as $c_type)
			$result[] = "\t$c_type " . implode(', ', $this->vars[$c_type]) .  ";\n";
		if (count($result)) {
			$result[] = "\n";
			return implode('', $result);
		} else
			return '';
	}
}

/*======================================================================*\
	Class:    Arg_Type
	Purpose:  Base class for argument type handlers
\*======================================================================*/
class Arg_Type {
	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		trigger_error("This is an abstract class", E_USER_ERROR);
	}
	
	function write_return($type, &$var_list)
	{
		trigger_error("This is an abstract class", E_USER_ERROR);
	}
}

class None_Arg extends Arg_Type {
	function write_return($type, &$var_list)
	{
		return "	%s;\n" .
			   "	RETURN_NULL();";
	}
}

class String_Arg extends Arg_Type {
	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		if (isset($default)) {
			if ($default != 'NULL')
				$default = '"' . $default . '"';
			$var_list->add('char', '*' . $name . ' = ' . $default);
		} else
			$var_list->add('char', '*' . $name);
		$parse_list[] 	= '&' . $name;
		$parse_type[]	= " ";
		$arg_list[] 	= $name;
		return 's';
	}
	
	function write_return($type, &$var_list)
	{
		if (in_array($type, array('const-gchar*', 'const-char*', 'static_string'))) {
			$var_list->add('const gchar', '*ret');
			return "	ret = %s;\n" 							.
				   "	if (ret) {\n"							.
				   "		RETURN_STRING((char *)ret, 1);\n"	.
				   "	}\n"									.
				   "	else {\n"								.
				   "		RETURN_NULL();\n"					.
				   "	}";
		} else {
			$var_list->add('gchar', '*ret');
			return "	ret = %s;\n" 					.
				   "	if (ret) {\n"					.
				   "		RETURN_STRING(ret, 1);\n"	.
				   "		g_free(ret);\n"				.
				   "	} else\n"						.
				   "		RETURN_NULL();";
		}
	}
}

class StringRef_Arg extends Arg_Type {
	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		$var_list->add('zval', '*php_' . $name . ' = NULL');
		$parse_list[] 	= '&php_' . $name;
		$parse_type[]	= "F";
		$arg_list[] 	= "($type)&Z_STRVAL_P(php_$name)";
		$extra_code[]	= "	convert_to_string(php_$name);\n";
		return 'V';
	}

	function write_return($type, &$var_list)
	{
	}
}

class Char_Arg extends Arg_Type {
	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		if (isset($default))
			$var_list->add('char', $name . ' = \'' . $default . '\'');
		else
			$var_list->add('char', $name);
		$parse_list[] 	= '&' . $name;
		$parse_type[]	= " ";
		$arg_list[] 	= $name;
		return 'c';
	}
	
	function write_return($type, &$var_list)
	{
		$var_list->add('gchar', 'ret[2]');
		return "	ret[0] = %s;\n"    .
			   "	ret[1] = '\\0';\n" .
			   "	RETURN_STRINGL(ret, 1, 1);";
	}
}

class Int_Arg extends Arg_Type {
	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		if (isset($default))
			$var_list->add('long', $name . ' = ' . $default);
		else
			$var_list->add('long', $name);
		$parse_list[] 	= '&' . $name;
		$parse_type[]	= " ";
		$arg_list[] 	= "($type)$name";
		return 'i';
	}
	
	function write_return($type, &$var_list)
	{
		return '	RETURN_LONG(%s);';
	}
}

class IntRef_Arg extends Arg_Type {
	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		$var_list->add('zval', '*php_' . $name . ' = NULL');
		$parse_list[] 	= '&php_' . $name;
		$parse_type[]	= "F";
		$arg_list[] 	= "($type)&Z_LVAL_P(php_$name)";
		$extra_code[]	= "	convert_to_long(php_$name);\n";
		return 'V';
	}

	function write_return($type, &$var_list)
	{
	}
}

class Bool_Arg extends Arg_Type {
	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		if (isset($default))
			$var_list->add('zend_bool', $name . ' = ' . $default);
		else
			$var_list->add('zend_bool', $name);
		$parse_list[] 	= '&' . $name;
		$parse_type[]	= " ";
		$arg_list[] 	= "($type)$name";
		return 'b';
	}
	
	function write_return($type, &$var_list)
	{
		return '	RETURN_BOOL(%s);';
	}
}

class Double_Arg extends Arg_Type {
	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		if (isset($default))
			$var_list->add('double', $name . ' = ' . $default);
		else
			$var_list->add('double', $name);
		$parse_list[] 	= '&' . $name;
		$parse_type[]	= " ";
		$arg_list[] 	= $name;
		return 'd';
	}
	
	function write_return($type, &$var_list)
	{
		return '	RETURN_DOUBLE(%s);';
	}
}

class Enum_Arg extends Arg_Type {
	var $enum_tpl  = null;
	var $enum_name = null;
	var $type_code = null;

	function Enum_Arg($enum_name)
	{
		$this->enum_name = $enum_name;
		$this->type_code = enum_name($enum_name);
		$this->enum_tpl	 = "	if (php_%s && !php_gtk_get_enum_value(%s, php_%s, (gint *)&%s)) {\n" .
						   "		%sreturn;\n" .
						   "	}\n\n";
	}

	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		if (isset($default))
			$var_list->add($this->enum_name, $name . ' = ' . $default);
		else
			$var_list->add($this->enum_name, $name);
		$var_list->add('zval', '*php_' . $name . ' = NULL');
		$parse_list[] 	= '&php_' . $name;
		$parse_type[]	= " ";
		$arg_list[] 	= $name;
		$extra_code[] 	= sprintf($this->enum_tpl, $name, $this->type_code, $name, $name,
								  $in_constructor ?  "php_gtk_invalidate(this_ptr);\n\t\t" : "");
		return 'V';
	}
	
	function write_return($type, &$var_list)
	{
		return '	RETURN_LONG(%s);';
	}
}

class Flags_Arg extends Arg_Type {
	var $flag_tpl  = null;
	var $flag_name = null;
	var $type_code = null;

	function Flags_Arg($flag_name)
	{
		$this->flag_name = $flag_name;
		$this->type_code = enum_name($flag_name);
		$this->flag_tpl	 = "	if (php_%s && !php_gtk_get_flag_value(%s, php_%s, (gint *)&%s)) {\n" .
						   "		%sreturn;\n" .
						   "	}\n\n";
	}

	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		if (isset($default))
			$var_list->add($this->flag_name, $name . ' = ' . $default);
		else
			$var_list->add($this->flag_name, $name);
		$var_list->add('zval', '*php_' . $name . ' = NULL');
		$parse_list[] 	= '&php_' . $name;
		$parse_type[]	= " ";
		$arg_list[] 	= $name;
		$extra_code[] 	= sprintf($this->flag_tpl, $name, $this->type_code, $name, $name,
								  $in_constructor ?  "php_gtk_invalidate(this_ptr);\n\t\t" : "");
		return 'V';
	}
	
	function write_return($type, &$var_list)
	{
		return '	RETURN_LONG(%s);';
	}
}

class Object_Arg extends Arg_Type {
	var $obj_name = null;
	var $cast     = null;

	function Object_Arg($obj_name)
	{
		$this->obj_name = $obj_name;
		$this->cast = substr(convert_typename($obj_name), 1);
	}

	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		if ($null_ok) {
			if (isset($default)) {
				$var_list->add($this->obj_name, '*' . $name . ' = ' . $default);
				$var_list->add('zval', '*php_' . $name . ' = NULL');
				$extra_code[]	=	"	if (php_$name) {\n" .
									"		if (Z_TYPE_P(php_$name) == IS_NULL)\n" .
									"			$name = NULL;\n" .
									"		else\n" .
									"			$name = $this->cast(PHP_GTK_GET(php_$name));\n" .
									"	}\n";
			} else {
				$var_list->add($this->obj_name, '*' . $name . ' = NULL');
				$var_list->add('zval', '*php_' . $name);
				$extra_code[]	=	"	if (Z_TYPE_P(php_$name) != IS_NULL)\n" .
									"		$name = $this->cast(PHP_GTK_GET(php_$name));\n";
			}

			$parse_list[] 	= '&php_' . $name . ', ' . strtolower($this->cast) . '_ce';
			$parse_type[]	= " ";
			$arg_list[] 	= $name;

			return 'N';
		} else {
			if (isset($default)) {
				$var_list->add($this->obj_name, '*' . $name . ' = ' . $default);
				$var_list->add('zval', '*php_' . $name . ' = NULL');
				$parse_list[] 	= '&php_' . $name . ', ' . strtolower($this->cast) . '_ce';
				$parse_type[]	= " ";
				$arg_list[] 	= $name;
				$extra_code[]	=	"	if (php_$name)\n" .
									"		$name = $this->cast(PHP_GTK_GET(php_$name));\n";
			} else {
				$var_list->add('zval', '*' . $name);
				$parse_list[] 	= '&' . $name . ', ' . strtolower($this->cast) . '_ce';
				$parse_type[]	= " ";
				$arg_list[] 	= "$this->cast(PHP_GTK_GET($name))";
			}

			return 'O';
		}
	}
	
	function write_return($type, &$var_list)
	{
		$var_list->add('zval', '*ret');
		return 	"	ret = php_gtk_new((GtkObject *)%s);\n" .
				"	SEPARATE_ZVAL(&ret);\n" .
				"	*return_value = *ret;\n" .
				"	return;";
	}
}

class Rect_Arg extends Arg_Type {
	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$parse_type,
						 $in_constructor)
	{
		$var_list->add(substr($type, 0, -1), $name);
		$parse_list[] 	= "&($name.x)";
		$parse_type[]	= " ";
		$parse_list[] 	= "&($name.y)";
		$parse_type[]	= " ";
		$parse_list[] 	= "&($name.width)";
		$parse_type[]	= " ";
		$parse_list[] 	= "&($name.height)";
		$parse_type[]	= " ";
		$arg_list[] 	= "&$name";
		return 'iiii';
	}
}

class Boxed_Arg extends Arg_Type {
	var $php_type	 = null;

	function Boxed_Arg($php_type)
	{
		$this->php_type 	= $php_type;
	}

	function write_param($type, $name, $default, $null_ok, &$var_list,
						 &$parse_list, &$arg_list, &$extra_code, &$array_code)
	{
		$var_list->add('zval', '*' . $name);
		$parse_list[]	= '&' . $name;
	    $parse_list[]	= $this->php_type . '_ce';
		$arg_list[]		= 'PHP_' . strtoupper($this->php_type) . '_GET(' . $name . ')';
		return 'O';
	}

	function write_return($type, &$var_list)
	{
		$var_list->add(substr($type, 0, -1), '*ret');
		return "	ret = %s;\n" .
			   "	*return_value = *php_" . $this->php_type . "_new(ret);\n" .
			   "	return;";
	}
}

class Atom_Arg extends Int_Arg {
	function write_return($type, &$var_list)
	{
		return "	*return_value = *php_gdk_atom_new(%s);\n" .
			   "	return;";
	}
}

class Arg_Matcher {
	var $arg_types = array();

	function register($type, $handler)
	{
		$this->arg_types[$type] = $handler;
	}

	function register_enum($type)
	{
		$this->arg_types[$type] = new Enum_Arg($type);
	}

	function register_flag($type)
	{
		$this->arg_types[$type] = new Flags_Arg($type);
	}

	function register_object($type)
	{
		$obj_arg = new Object_Arg($type);
		$this->register($type, $obj_arg);
		$this->register($type . '*', $obj_arg);
	}

	function register_boxed($type, $php_type)
	{
		$handler = new Boxed_Arg($php_type);
		$this->register($type . '*', $handler);
	}

	function &get($type)
	{
		return $this->arg_types[$type];
	}
}

$matcher = new Arg_Matcher();

$arg = new None_Arg();
$matcher->register(null, $arg);
$matcher->register('none', $arg);

$arg = new String_Arg();
$matcher->register('char*', $arg);
$matcher->register('gchar*', $arg);
$matcher->register('const-char*', $arg);
$matcher->register('const-gchar*', $arg);
$matcher->register('string', $arg);
$matcher->register('static_string', $arg);
$matcher->register('unsigned-char*', $arg);
$matcher->register('guchar*', $arg);

$arg = new StringRef_Arg();
$matcher->register('char**', $arg);
$matcher->register('gchar**', $arg);

$arg = new Char_Arg();
$matcher->register('char', $arg);
$matcher->register('gchar', $arg);
$matcher->register('guchar', $arg);

$arg = new Int_Arg();
$matcher->register('int', $arg);
$matcher->register('gint', $arg);
$matcher->register('guint', $arg);
$matcher->register('short', $arg);
$matcher->register('gshort', $arg);
$matcher->register('gushort', $arg);
$matcher->register('long', $arg);
$matcher->register('glong', $arg);
$matcher->register('gulong', $arg);

$matcher->register('guint8', $arg);
$matcher->register('gint8', $arg);
$matcher->register('guint16', $arg);
$matcher->register('gint16', $arg);
$matcher->register('guint32', $arg);
$matcher->register('gint32', $arg);
$matcher->register('GtkType', $arg);

$arg = new IntRef_Arg();
$matcher->register('gint*', $arg);
$matcher->register('guint*', $arg);
$matcher->register('guint8*', $arg);

$arg = new Bool_Arg();
$matcher->register('gboolean', $arg);

$arg = new Double_Arg();
$matcher->register('double', $arg);
$matcher->register('gdouble', $arg);
$matcher->register('float', $arg);
$matcher->register('gfloat', $arg);

$arg = new Rect_Arg();
$matcher->register('GdkRectangle*', $arg);

$arg = new Atom_Arg();
$matcher->register('GdkAtom', $arg);

$matcher->register_boxed('GdkEvent', 'gdk_event');
$matcher->register_boxed('GdkWindow', 'gdk_window');
$matcher->register('GdkPixmap*', $matcher->get('GdkWindow*'));
$matcher->register('GdkBitmap*', $matcher->get('GdkWindow*'));
$matcher->register('GdkDrawable*', $matcher->get('GdkWindow*'));
$matcher->register_boxed('GdkColor', 'gdk_color');
$matcher->register_boxed('GdkColormap', 'gdk_colormap');
$matcher->register_boxed('GdkCursor', 'gdk_cursor');
$matcher->register_boxed('GdkVisual', 'gdk_visual');
$matcher->register_boxed('GdkFont', 'gdk_font');
$matcher->register_boxed('GdkGC', 'gdk_gc');
$matcher->register_boxed('GdkDragContext', 'gdk_drag_context');
$matcher->register_boxed('GtkSelectionData', 'gtk_selection_data');
$matcher->register_boxed('GtkCTreeNode', 'gtk_ctree_node');
$matcher->register_boxed('GtkAccelGroup', 'gtk_accel_group');
$matcher->register_boxed('GtkStyle', 'gtk_style');

?>
