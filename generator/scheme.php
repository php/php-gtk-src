<?php
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

require "definitions.php";

$char_table = str_repeat(' ', 256);
foreach (range(0, 255) as $i)
    $char_table[$i] = chr($i);
$translation_table = preg_replace('![^a-zA-Z_]!', '_', $char_table);

function parse($filename)
{
    $fp = fopen($filename, 'r');
    $stack = array(array());
    $whitespace = " \t\n\r";
    $nonsymbol = $whitespace . '();\'"';
    $lineno = 0;
    $openlines = array();
    while ($line = fgets($fp, 4096)) {
        $pos = 0;
        $len = strlen($line);
        $lineno++;
        while ($pos < $len) {
            if (strpos($whitespace, $line{$pos}) !== false) {
                $pos++; continue;
            } else if ($line{$pos} == ';') {
                break;
            } else if (substr($line, $pos, 2) == "'(") {
                $pos++; continue;
            } else if ($line{$pos} == '(') {
                array_push($stack, array());
                $openlines[] = $lineno;
            } else if ($line{$pos} == ')') {
                if (!$openlines) {
                    trigger_error("[$filename:$lineno] Closing parenthesis found without a match");
                    return;
                }
                $closed = array_pop($stack);
                array_push($stack[count($stack)-1], $closed);
                array_pop($openlines);
            } else if ($line{$pos} == '"') {
                if (!$openlines) {
                    trigger_error("[$filename:$lineno] String found outside of s-expression");
                    return;
                }
                $endpos = strpos($line, '"', $pos+1);
                if ($endpos === false) {
                    trigger_error("[$filename:$lineno] Unclosed quoted string");
                    return;
                }
                array_push($stack[count($stack)-1], substr($line, $pos+1, $endpos-$pos-1));
                $pos = $endpos;
            } else {
                if (!$openlines) {
                    trigger_error("[$filename:$lineno] Identifier found outside of s-expression");
                    return;
                }
                $span = strcspn($line, $nonsymbol, $pos);
                $symbol = substr($line, $pos, $span);
                $pos += $span-1;
                if (is_numeric($symbol)) {
                    $symbol = (double)$symbol;
                }
                array_push($stack[count($stack)-1], $symbol);
            }
            $pos++;
        }
    }
    if ($openlines) {
        trigger_error(sprintf("[$filename] Unclosed parentheses found on lines: %s", join(', ', $openlines)));
        return;
    }
    return $stack[0];
}

class Defs_Parser {
    var $parse_tree     = null;
    var $parse_cache    = null;
    var $file_path      = null;
    var $file_name      = null;
    var $objects        = array();  // objects
    var $functions      = array();  // module functions
    var $constructors   = array();  // object constructors
    var $methods        = array();  // object methods
    var $enums          = array();  // enums and flags
    var $interfaces     = array();  // interfaces
    var $boxes          = array();  // boxed types
    var $c_name         = array();  // C names of entities

    function Defs_Parser($arg)
    {
        switch (gettype($arg)) {
            case 'string':
                $this->_parse_or_load($arg);
                break;

            case 'array':
                $this->parse_tree = $arg;
                break;

            default:
                trigger_error('Constructor argument must be filename or array');
                break;
        }
    }

    function _parse_or_load($defs_file)
    {
        $cache_file = $defs_file.'.cache';
        if (@is_file($cache_file) &&
            filemtime($cache_file) > filemtime($defs_file)) {
            error_log("Loading cache \"$cache_file\"");
            $fp = fopen($cache_file, 'r');
            $this->parse_cache = fread($fp, filesize($cache_file));
            fclose($fp);
        } else {
            error_log("Parsing file \"$defs_file\".");
            $this->file_name = basename($defs_file);
            $this->file_path = dirname($defs_file);
            $this->parse_tree = parse($defs_file, 'r');
        }
    }

    function start_parsing($tree = NULL)
    {
        if (isset($this->parse_cache)) {
            $this->unserialize($this->parse_cache);
        } else {
            if (!isset($tree))
                $tree = $this->parse_tree;
            foreach ($tree as $node)
                $this->handle($node);

            /*
             * Sort the objects list so that the child objects always come after
             * parent ones.
             */
            $objects = $this->objects;
            $pos = 0;
            $num_obj = count($objects);
            while ($pos < $num_obj) {
                $object = $objects[$pos];
                $parent = $object->parent;
                for ($i = $pos+1; $i < $num_obj; $i++) {
                    if ($objects[$i]->c_name == $parent) {
                        unset($objects[$pos]);
                        array_splice($objects, $i+1, 0, array($object));
                        break;
                    }
                }
                if ($i == $num_obj)
                    $pos++;
            }
            $this->objects = $objects;

            if (is_writeable($this->file_path)) {
                $cache_file = $this->file_path . '/' . $this->file_name . '.cache';
                $fp = fopen($cache_file, 'w');
                fwrite($fp, $this->serialize());
                fclose($fp);
            }
        }
    }

    function serialize()
    {
        return serialize((array)$this);
    }

    function unserialize($buffer)
    {
        foreach (unserialize($buffer) as $var => $content) {
            $this->$var = $content;
        }
    }

    function merge($parser)
    {
        $this->parse_tree = array_merge($this->parse_tree, $parser->parse_tree);
    }

    function handle($node)
    {
        global  $char_table,
                $translation_table;

        $cmd = "handle_" . strtr($node[0], $char_table, $translation_table);
        if (method_exists($this, $cmd))
            $this->$cmd(array_slice($node, 1));
        else
            $this->handle_unknown($node);
    }

    function handle_define_enum($arg)
    {
        $enum_def       = new Enum_Def($arg);
        if (basename($this->file_path) == 'gtk+')
            $enum_def->simple = false;
        $this->enums[]  = &$enum_def;
        $this->c_name[] = &$enum_def->c_name;
    }

    function handle_define_flags($arg)
    {
        $flag_def       = new Flag_Def($arg);
        $this->enums[]  = &$flag_def;
        $this->c_name[] = &$flag_def->c_name;
    }

    function handle_define_function($arg)
    {
        $function_def       = new Function_Def($arg);
        if (isset($function_def->is_constructor_of))
            $this->constructors[] = &$function_def;
        else
            $this->functions[] = &$function_def;
        $this->c_name[]     = &$function_def->c_name;
    }

    function handle_define_method($arg)
    {
        $method_def         = new Method_Def($arg);
        $this->methods[]    = &$method_def;
        $this->c_name[]     = &$method_def->c_name;
    }

    function handle_define_object($arg)
    {
        $object_def         = new Object_Def($arg);
        $this->objects[]    = &$object_def;
        $this->c_name[]     = &$object_def->c_name;
    }

    function handle_define_interface($arg)
    {
        $interface_def      = new Interface_Def($arg);
        $this->interfaces[$interface_def->in_module . $interface_def->name] = &$interface_def;
        $this->c_name[]     = &$interface_def->c_name;
    }

    function handle_define_boxed($arg)
    {
        $boxed_def          = new Boxed_Def($arg);
        $this->boxes[]      = &$boxed_def;
        $this->c_name[]     = &$boxed_def->c_name;
    }

    function handle_define_pointer($arg)
    {
        fprintf(STDERR, "TODO: implement Pointer_Def\n");
    }

    function handle_include($arg)
    {
        $include_file = $this->file_path . "/" . $arg[0];
        error_log("Parsing file \"$include_file\".");
        $include_tree = parse($include_file, 'r');
        $this->start_parsing($include_tree);
    }

    function handle_unknown($node)
    {
        /* noop */
        die("unknown node:\n". var_export($node, 1));
    }

    function find_methods($object)
    {
        $obj_methods = array();

        foreach ($this->methods as $method) {
            if ($method->of_object == $object->c_name)
                $obj_methods[] = $method;
        }

        return $obj_methods;
    }

    function find_constructor($obj)
    {
        foreach ($this->constructors as $constructor) {
            if ($constructor->is_constructor_of == $obj->c_name)
                return $constructor;
        }
    }

    function find_parent($obj)
    {
        if (isset($obj->parent)) {
            return $this->objects[$obj->parent[1] . $obj->parent[0]];
        }
    }
}

#require './arg_types.php';
#$d = new Defs_Parser('test.defs');
#$d->start_parsing();
#var_dump($d->constructors);
#var_dump($d->functions);
#var_dump($d->methods);
#var_dump($d->objects);
#var_dump($d->enums);
#var_dump($d->boxes);
#var_dump($d->interfaces);

/* vim: set et sts=4: */
?>
