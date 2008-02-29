<?php
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

require "definitions.php";

$char_table = str_repeat(' ', 256);
foreach (range(0, 255) as $i)
    $char_table[$i] = chr($i);
$translation_table = preg_replace('![^a-zA-Z_]!', '_', $char_table);

function parse($filename)
{
    if (!is_file($filename)) {
        die("Unable to open file $filename\n");
    }
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
    var $boxed          = array();  // boxed types
    var $pointers       = array();  // pointers
    var $c_name         = array();  // C names of entities
    var $gtkversion     = null;     // gtk lib version

    public function __construct($arg, $gtkversion = 0)
    {
        if (version_compare($gtkversion, '2.6', '<')) {
            $gtkversion = '2.6';
        }

        $this->gtkversion = $gtkversion;

        switch (gettype($arg)) {
            case 'string':
                $this->file_path = dirname($arg);
                $this->_set_file_version($arg, $this->gtkversion);
                $this->_parse_or_load();
                break;

            case 'array':
                $this->parse_tree = $arg;
                break;

            default:
                trigger_error('Constructor argument must be filename or array');
                break;
        }
    }

    private function _set_file_version($file_name, $gtkversion)
    {
        while (version_compare($gtkversion, '2.6', '>')) {

            $split = explode('.', $file_name);
            $try   = $split[0].'-'.$gtkversion.'.'.$split[1];

            if (file_exists($try)) {
                $file_name = $try;
                break;
            }

            $point = substr($gtkversion, 2);
            $gtkversion = '2.'.($point - 2);
            $this->_set_file_version($file_name, $gtkversion);
        }
        $this->file_name = $file_name;
    }

    private function _parse_or_load()
    {
        $cache_file = $this->file_name.'.cache';
        if (@is_file($cache_file) &&
            filemtime($cache_file) > filemtime($this->file_name)) {
            error_log("Loading cache '". basename($cache_file) ."'");
            $fp = fopen($cache_file, 'r');
            $this->parse_cache = fread($fp, filesize($cache_file));
            fclose($fp);
        } else {
            error_log("Parsing file '$this->file_name'.");
            $this->parse_tree = parse($this->file_name, 'r');
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
                $cache_file = $this->file_name . '.cache';
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

    function merge_parse_tree($parser)
    {
        $this->parse_tree = array_merge($this->parse_tree, $parser->parse_tree);
    }

    function merge($old, $merge_params)
    {
        $all_defs = array_merge($this->objects, $this->boxed, $this->interfaces, $this->pointers);

        foreach ($all_defs as $def) {
            if (isset($old->c_name[$def->c_name])) {
                $def->merge($old->c_name[$def->c_name]);
            }
        }
    
        $all_defs = array_merge($this->functions, $this->constructors, $this->methods);
        foreach ($all_defs as $def) {
            if (isset($old->c_name[$def->c_name])) {
                $def->merge($old->c_name[$def->c_name], $merge_params);
            }
        }
    }

    function sort_defs($defs)
    {
        usort($defs, create_function('$a, $b', 'return strcasecmp($a->c_name, $b->c_name);'));
        return $defs;
    }

    function cmp_defs($a, $b)
    {
        if (($ca = get_class($a)) == ($cb = get_class($b))) {
            return strcasecmp($a->c_name, $b->c_name);
        } else {
            return strcmp($ca, $cb);
        }
    }

    function write_defs($fp = null)
    {
        if (!$fp) {
            $fp = STDOUT;
        }

        if ($this->boxed) {
            $my_defs = $this->boxed;
            usort($my_defs, array($this, 'cmp_defs'));
            fwrite($fp, ";; Boxed types ...\n\n");
            foreach ($my_defs as $object) {
                $object->write_defs($fp);
            }
        }

        if ($this->enums) {
            $my_defs = $this->enums;
            usort($my_defs, array($this, 'cmp_defs'));
            fwrite($fp, ";; Enumerations ...\n\n");
            foreach ($my_defs as $object) {
                $object->write_defs($fp);
            }
        }

        if ($this->interfaces) {
            $my_defs = $this->interfaces;
            usort($my_defs, array($this, 'cmp_defs'));
            fwrite($fp, ";; Interfaces ...\n\n");
            foreach ($my_defs as $object) {
                $object->write_defs($fp);
            }
        }

        /* don't sort objects */
        if ($this->objects) {
            fwrite($fp, ";; Objects ...\n\n");
            foreach ($this->objects as $object) {
                $object->write_defs($fp);
            }
        }

        if ($this->pointers) {
            $my_defs = $this->pointers;
            usort($my_defs, array($this, 'cmp_defs'));
            fwrite($fp, ";; Pointers ...\n\n");
            foreach ($my_defs as $pointer) {
                $pointer->write_defs($fp);
            }
        }

        if ($this->functions || $this->constructors || $this->methods) {
            $my_defs = $this->sort_defs(array_merge($this->functions, $this->constructors, $this->methods));
            fwrite($fp, ";; Functions, constructors, and methods ...\n\n");
            foreach ($my_defs as $object) {
                $object->write_defs($fp);
            }
        }
    }

    function write_missing_defs($old, $fp = null)
    {
        if (!$fp) {
            $fp = STDOUT;
        }

        foreach ($this->sort_defs($this->boxed) as $object) {
            if (!isset($old->c_name[$object->c_name])) {
                $object->write_defs($fp);
            }
        }

        foreach ($this->sort_defs($this->interfaces) as $object) {
            if (!isset($old->c_name[$object->c_name])) {
                $object->write_defs($fp);
            }
        }

        foreach ($this->sort_defs($this->objects) as $object) {
            if (!isset($old->c_name[$object->c_name])) {
                $object->write_defs($fp);
            }
        }

        foreach ($this->sort_defs($this->enums) as $object) {
            if (!isset($old->c_name[$object->c_name])) {
                $object->write_defs($fp);
            }
        }

        foreach ($this->sort_defs(array_merge($this->constructors, $this->functions, $this->methods)) as $object) {
            if (!isset($old->c_name[$object->c_name])) {
                $object->write_defs($fp);
            }
        }
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
        $this->enums[]  = $enum_def;
        $this->c_name[$enum_def->c_name] = $enum_def;
    }

    function handle_define_flags($arg)
    {
        $flag_def       = new Flag_Def($arg);
        $this->enums[]  = $flag_def;
        $this->c_name[$flag_def->c_name] = $flag_def;
    }

    function handle_define_function($arg)
    {
        $function_def       = new Function_Def($arg);
        if (isset($function_def->is_constructor_of)) {
            $old_def = @$this->functions[$function_def->c_name];
            if (!isset($old_def) || $old_def->c_name == $old_def->name) {
                $this->constructors[$function_def->c_name] = $function_def;
            }
        } else {
            $old_def = @$this->functions[$function_def->c_name];
            if (!isset($old_def) || $old_def->c_name == $old_def->name) {
                $this->functions[$function_def->c_name] = $function_def;
            }
        }
        $this->c_name[$function_def->c_name] = $function_def;
    }

    function handle_define_method($arg)
    {
        $method_def         = new Method_Def($arg);
        $this->methods[]    = $method_def;
        $this->c_name[$method_def->c_name] = $method_def;
    }

    function handle_define_object($arg)
    {
        $object_def         = new Object_Def($arg);
        $this->objects[]    = $object_def;
        $this->c_name[$object_def->c_name] = $object_def;
    }

    function handle_define_interface($arg)
    {
        $interface_def      = new Interface_Def($arg);
        $this->interfaces[$interface_def->c_name] = $interface_def;
        $this->c_name[$interface_def->c_name] = $interface_def;
    }

    function handle_define_boxed($arg)
    {
        $boxed_def          = new Boxed_Def($arg);
        $this->boxed[]      = $boxed_def;
        $this->c_name[$boxed_def->c_name] = $boxed_def;
    }

    function handle_define_pointer($arg)
    {
        $pointer_def        = new Pointer_Def($arg);
        $this->pointers[]   = $pointer_def;
        $this->c_name[$pointer_def->c_name] = $pointer_def;
    }

    function handle_include($arg)
    {
        $include_file = $this->file_path . DIRECTORY_SEPARATOR . $arg[0];
        error_log("Parsing file '$include_file'.");
        $include_tree = parse($include_file, 'r');
        $this->start_parsing($include_tree);
    }

    function handle_define_virtual($arg)
    {
        /* do nothing for now */
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

    function find_constructor($obj, $overrides)
    {
        $constructors = array();
        foreach ($this->constructors as $constructor) {
            if ($constructor->is_constructor_of == $obj->c_name &&
                !$overrides->is_ignored($constructor->c_name)) {
                $constructors[] = $constructor;
            }
        }
        return $constructors;
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
#var_dump($d->boxed);
#var_dump($d->interfaces);

/* vim: set et sts=4: */
?>
