<?php
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

/* $Id$ */

set_time_limit(300);

require "Getopt.php";
require "arg_types.php";
require "override.php";
require "scheme.php";
require "docmerger.php";
require "doc_templates.php";

class DocGenerator {
	var $parser 	= null;
	var $overrides  = null;
	var $prefix		= null;
	var $output_dir = null;
	var $fp			= null;
	var $docmerger  = null;

	function DocGenerator(&$parser, &$overrides, &$docmerger, $prefix, $output_dir)
	{
		$this->parser 	 	= &$parser;
		$this->overrides	= &$overrides;
		$this->docmerger    = &$docmerger;
		$this->prefix	 	= $prefix;
		if ($output_dir)
			$this->output_dir	= $output_dir;
		else
			$this->fp = fopen('php://stdout', 'w');
	}

	function create_docs($classes)
	{
		foreach ($classes as $key => $val)
			$classes[$key] = strtolower($val);
								 
		function sort_objects($a, $b)
		{
			return strcmp($a->c_name, $b->c_name);
		}
		$parser_objects = $this->parser->objects;
		usort($parser_objects, 'sort_objects');

		foreach ($parser_objects as $object) {
			$object_lcname = strtolower($object->in_module . $object->name);
			if (count($classes) && !in_array($object_lcname, $classes))
				continue;

			if ($this->output_dir) {
				print "Generating $object_lcname.xml...";
				$this->fp = fopen($this->output_dir.'/'.$object_lcname.'.xml', 'w');
				$this->write_class($object);
				fclose($this->fp);
				print "\n";
			} else
				$this->write_class($object);
		}

		if (count($classes) == 0) {
			if ($this->output_dir) {
				print "Generating ".$this->prefix."-functions.xml...";
				$this->fp = fopen($this->output_dir.'/'.$this->prefix.'-functions.xml', 'w');
				$this->write_functions();
				fclose($this->fp);
				print "\n";
			} else
				$this->write_functions();
		}
	}
	
	function register_types($parser = null)
	{
		global	$matcher;

		if (!$parser)
			$parser = $this->parser;

		foreach ($parser->objects as $object)
			$matcher->register_object($object->c_name);

		foreach ($parser->enums as $enum) {
			if ($enum->def_type == 'flags')
				$matcher->register_flag($enum->c_name);
			else
				$matcher->register_enum($enum->c_name);
		}
	}

	function write_class($object)
	{
		global	$class_start_tpl,
				$class_end_tpl,
				$update_docs;

		$object_name = $object->in_module . $object->name;

		if($update_docs)
			$this->docmerger->prepair($object_name);

		fwrite($this->fp,
			   sprintf($class_start_tpl,
					   $this->prefix,
					   strtolower($object_name),
					   $object_name,
					   $object->parent[1] . $object->parent[0],
					      $update_docs?$this->docmerger->get_section_contents(NULL, CLASSENTRY, SHORTDESC):NULL,
					   $update_docs?$this->docmerger->get_section_contents(NULL, CLASSENTRY, DESC):NULL
					   ));

		$this->write_constructor($object);
		$this->write_methods($object);
		$this->write_properties($object);
		if($update_docs)
			fwrite($this->fp,
				  $this->docmerger->get_section_contents(NULL, SIGNALS, NULL));

		fwrite($this->fp, $class_end_tpl);
	}

	function write_properties($object)
	{
		global	$props_start_tpl,
				$props_end_tpl;

		if (count($object->fields) == 0)
			return;
		
		fwrite($this->fp, $props_start_tpl);

		foreach ($object->fields as $field) {
			$this->write_property($object, $field);
		}
		
		fwrite($this->fp, $props_end_tpl);
	}

	function write_property($object, $field)
	{
		global	$prop_start_tpl,
				$prop_end_tpl;

		list($field_type, $field_name) = $field;
		if (($doc_type = $this->get_type($field_type)) === false) {
			$doc_type = 'XXX';
		}

		fwrite($this->fp, sprintf($prop_start_tpl, $this->prefix,
								  strtolower($object->in_module . $object->name),
								  $field_name, $field_name, $doc_type));
		fwrite($this->fp, $prop_end_tpl);
	}

	function write_methods($object)
	{
		global	$methods_start_tpl,
				$methods_end_tpl;

		$methods = $this->parser->find_methods($object);
		if (count($methods))
			fwrite($this->fp, $methods_start_tpl);

		foreach ($methods as $method) {
			if ($this->overrides->is_overriden($method->c_name)) {
				$this->write_method($method, true);
			} else if (!$this->overrides->is_ignored($method->c_name)) {
				$this->write_method($method, false);
			}
		}

		if (count($methods))
			fwrite($this->fp, $methods_end_tpl);
	}

	function write_functions()
	{
		foreach ($this->parser->functions as $function) {
			if ($this->overrides->is_overriden($function->c_name)) {
				$this->write_method($function, true);
			} else if (!$this->overrides->is_ignored($function->c_name)) {
				$this->write_method($function, false);
			}
		}
	}

	function write_constructor($object)
	{
		global	$constructor_start_tpl,
				$constructor_end_tpl,
				$funcproto_tpl,
				$no_parameter_tpl,
				$update_docs;

		$constructor = $this->parser->find_constructor($object);
		
		if ($constructor) {
			if ($this->overrides->is_overriden($constructor->c_name)) {
				$overriden = true;
			} else if (!$this->overrides->is_ignored($constructor->c_name)) {
				$overriden = false;
			} else
				return;
		} else
			return;

		if (!$overriden) {
			if (($paramdef = $this->get_paramdef($constructor)) === false)
				return;
		}

		$object_name = $constructor->is_constructor_of;

		fwrite($this->fp,
			   sprintf($constructor_start_tpl,
					   $this->prefix,
					   strtolower($object_name)));

		$funcdef = sprintf($funcproto_tpl,
						   '',
						   $object_name,
						   $overriden ? sprintf($no_parameter_tpl, 'XXX') : $paramdef);
		fwrite($this->fp, preg_replace('!^ !m', '', $funcdef));

		fwrite($this->fp, 
			   sprintf($constructor_end_tpl,
					   $update_docs?$this->docmerger->get_section_contents(NULL, CONSTRUCTOR, SHORTDESC):NULL,
					   $update_docs?$this->docmerger->get_section_contents(NULL, CONSTRUCTOR, DESC):NULL
					   ));	
	}

	function write_method($method, $overriden)
	{
		global	$method_start_tpl,
				$method_func_start_tpl,
				$method_end_tpl,
				$funcproto_tpl,
				$no_parameter_tpl,
				$update_docs;

		if (!$overriden) {
			if (($paramdef = $this->get_paramdef($method)) === false ||
				($return = $this->get_type($method->return_type)) === false)
				return;
		}

		if (isset($method->of_object)) {
			$object_name = $method->of_object[1] . $method->of_object[0];

			fwrite($this->fp,
				   sprintf($method_start_tpl,
						   $this->prefix,
						   strtolower($object_name),
						   $method->name));
		} else
			fwrite($this->fp,
				   sprintf($method_func_start_tpl,
						   $this->prefix,
						   $method->name));

		fwrite($this->fp,
			   sprintf($funcproto_tpl,
					   $overriden ? 'XXX' : $return,
					   $method->name,
					   $overriden ? sprintf($no_parameter_tpl, 'XXX') : $paramdef));
		
		fwrite($this->fp, 
			   sprintf($method_end_tpl,
					   $update_docs?$this->docmerger->get_section_contents($method->name, METHOD, SHORTDESC):NULL,
					   $update_docs?$this->docmerger->get_section_contents($method->name, METHOD, DESC):NULL
					   ));
	}

	function get_paramdef($function)
	{
		global	$matcher,
				$parameter_tpl,
				$no_parameter_tpl,
				$opt_parameter_tpl;

		if ($function->varargs)
			return false;

		foreach ($function->params as $params_array) {
			list($param_type, $param_name, $param_default, $param_null) = $params_array;

			if (($doc_type = $this->get_type($param_type)) === false)
				return false;

			if (isset($param_default))
				$paramdef .= sprintf($opt_parameter_tpl, $doc_type, $param_name, $param_default);
			else
				$paramdef .= sprintf($parameter_tpl, $doc_type, $param_name);
		}

		return $paramdef ? $paramdef : sprintf($no_parameter_tpl, 'void');
	}

	function get_type($in_type)
	{
		global	$matcher;
		static 	$type_map = 
			array('none'			=> 'void',
				  
				  'char*'			=> 'string',
				  'gchar*'			=> 'string',
				  'const-char*'		=> 'string',
				  'const-gchar*'	=> 'string',
				  'string'			=> 'string',
				  'static_string'	=> 'string',
				  'unsigned-char*'	=> 'string',
				  'guchar*'			=> 'string',

				  'char'			=> 'char',
				  'gchar'			=> 'char',
				  'guchar'			=> 'char',

				  'int'				=> 'int',
				  'gint'			=> 'int',
				  'guint'			=> 'int',
				  'short'			=> 'int',
				  'gshort'			=> 'int',
				  'gushort'			=> 'int',
				  'long'			=> 'int',
				  'glong'			=> 'int',
				  'gulong'			=> 'int',

				  'guint8'			=> 'int',
				  'gint8'			=> 'int',
				  'guint16'			=> 'int',
				  'gint16'			=> 'int',
				  'guint32'			=> 'int',
				  'gint32'			=> 'int',
				  'GtkType'			=> 'int',

				  'gboolean'		=> 'bool',

				  'double'			=> 'double',
				  'gdouble'			=> 'double',
				  'float'			=> 'double',
				  'gfloat'			=> 'double',

				  'GdkDrawable*'	=> 'GdkWindow');

		$type_handler = &$matcher->get($in_type);
		if ($type_handler === null)
			return false;

		if (isset($type_map[$in_type]))
			return $type_map[$in_type];
		else {
			$in_type = str_replace('*', '', $in_type);
			$type_handler_class = get_class($type_handler);
			if ($type_handler_class == 'object_arg' ||
				$type_handler_class == 'boxed_arg')
				return "<classname>$in_type</classname>";
			else if ($type_handler_class == 'enum_arg' ||
					 $type_handler_class == 'flags_arg')
				return "<enumname>$in_type</enumname>";
			else
				return $in_type;
		}
	}
}

/* For backwards compatibility. */
chdir(dirname(__FILE__));

$argc = $HTTP_SERVER_VARS['argc'];
$argv = $HTTP_SERVER_VARS['argv'];

/* An ugly hack to counteract PHP's pernicious desire to treat + as an argument
   separator in command-line version. */
array_walk($argv, create_function('&$x', '$x = urldecode($x);'));

$result = Console_Getopt::getopt($argv, 'o:p:r:d:s:l:u');
if (!$result || count($result[1]) < 2)
	die(
"Usage: php -q generator.php [OPTION] defsfile [class ...]

  -o <file>         use overrides in <file>
  -p <prefix>       use <prefix> for docs
  -r <file>         register types from <file>
  -d <path>         output files to this directory
  -s <path>         documentation dir
  -l <lang>         Language
  -u                Update existing docs
");

list($opts, $argv) = $result;

$prefix = 'gtk';
$overrides = new Overrides();
$lang = 'en';
$update_docs = FALSE;

foreach ($opts as $opt) {
	list($opt_spec, $opt_arg) = $opt;
	if ($opt_spec == 'o') {
		$overrides = new Overrides($opt_arg);
	} else if ($opt_spec == 'p') {
		$prefix = $opt_arg;
	} else if ($opt_spec == 'r') {
		$type_parser = new Defs_Parser($opt_arg);
		$type_parser->start_parsing();
		DocGenerator::register_types($type_parser);
	} else if ($opt_spec == 'd') {
		$output_dir = $opt_arg;
	} else if ($opt_spec == 's') {
		$docs_dir =	$opt_arg;
	} else if ($opt_spec ==	'l') {
		$lang =	$opt_arg;
	} else if ($opt_spec ==	'u') {
		$update_docs = TRUE;
	}
}

$parser = new Defs_Parser($argv[1]);

if($update_docs)
	$docmerger = new DocMerger($docs_dir, $lang, $prefix);

$generator = new DocGenerator($parser, $overrides, $docmerger, $prefix,	$output_dir);

$parser->start_parsing();
$generator->register_types();
$generator->create_docs(array_slice($argv, 2));

?>
