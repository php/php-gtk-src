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

class Overrides {
    var $ignores            = array();
    var $glob_ignores       = array();
    var $overrides          = array();
    var $prop_overrides     = array();
    var $handler_overrides  = array();
    var $extra_methods      = array();
    var $getprops           = array();
    var $register_classes   = array();
    var $headers            = '';
    var $constants          = '';

    function Overrides($file_name = null)
    {
        if (!isset($file_name))
            return;

        $this->read_file($file_name);
    }

    function read_file($file_name)
    {
        $old_dir = getcwd();

        $fp = fopen($file_name, 'r');
        $contents = fread($fp, filesize($file_name));
        fclose($fp);

        $blocks = explode('%%', $contents);
        if (!count($blocks))
            return;

        $new_dir = dirname(realpath($file_name));
        if ($new_dir != $old_dir)
            chdir($new_dir);

        foreach ($blocks as $block)
            $this->_parse_block(trim($block));

        chdir($old_dir);
    }

    function _parse_block($block)
    {
        if (strpos($block, "\n"))
            list($line, $rest) = explode("\n", $block, 2);
        else {
            $line = $block;
            $rest = '';
        }
        $words = preg_split('!\s+!', $line, -1, PREG_SPLIT_NO_EMPTY);
        $command = array_shift($words);

        switch ($command) {
            case 'ignore':
                foreach ($words as $func)
                    $this->ignores[$func] = true;
                foreach (preg_split('!\s+!', $rest, -1, PREG_SPLIT_NO_EMPTY) as $func)
                    $this->ignores[$func] = true;
                break;

            case 'ignore-glob':
                foreach ($words as $func)
                    $this->glob_ignores[] = $func;
                foreach (preg_split('!\s+!', $rest, -1, PREG_SPLIT_NO_EMPTY) as $func)
                    $this->glob_ignores[] = $func;
                break;

            case 'override':
                $func_cname = $words[0];
                if (isset($words[1])) 
                    $func_name = $words[1];
                else
                    $func_name = $func_cname;
                if (isset($words[2]))
                    $flags = $words[2];
                else
                    $flags = null;
                $this->overrides[$func_cname] = array($func_name, $rest, $flags);
                break;

            case 'override-prop':
                $class = $words[0];
                $prop = $words[1];
                if (isset($words[2]))
                    $type = $words[2];
                else
                    $type = 'read';
                $this->prop_overrides[$class][$prop][$type] = $rest;
                break;

            case 'override-handler':
                $class = $words[0];
                $handler = $words[1];
                $this->handler_overrides[$class][$handler] = $rest;
                break;

            case 'include':
                $file_name = $words[0];
                $this->read_file($file_name);
                foreach (preg_split(',\s+,', $rest, -1, PREG_SPLIT_NO_EMPTY) as $file_name) {
                    $this->read_file($file_name);
                }
                break;

            case 'add':
                if (count($words) < 2) break;
                $class = $words[0];
                $method = $words[1];
                $this->extra_methods[strtolower($class)][$method] = "\n" . $rest . "\n";
                break;

            case 'getprop':
                if (count($words) >= 2) {
                    list($class_name, $prop_name) = $words;
                    $this->getprops[$class_name][$prop_name] = $rest;
                }
                break;

            case 'headers':
                $this->headers .= "\n" . $rest . "\n";
                break;

            case 'constants':
                $this->constants .= "\n" . $rest . "\n";
                break;

            case 'register_class':
                $this->register_classes[$words[0]] = $rest;
                break;
        }
    }

    function is_ignored($name)
    {
        if (isset($this->ignores[$name]))
            return true;

        foreach ($this->glob_ignores as $glob) {
            if (preg_match('!' . str_replace('*', '.*', $glob) . '!', $name))
                return true;
        }

        return false;
    }

    function is_overriden($name)
    {
        return isset($this->overrides[$name]);
    }

    function get_override($name)
    {
        return $this->overrides[$name];
    }

    function is_prop_overriden($class, $name)
    {
        return isset($this->prop_overrides[$class][$name]);
    }

    function get_prop_override($class, $name)
    {
        return $this->prop_overrides[$class][$name];
    }

    function is_handler_overriden($class, $handler)
    {
        return isset($this->handler_overrides[$class][$handler]);
    }

    function get_handler_override($class, $handler)
    {
        return $this->handler_overrides[$class][$handler];
    }
    
    function get_extra_methods($class)
    {
        return $this->extra_methods[strtolower($class)];
    }

    function have_get_prop($class_name, $prop_name)
    {
        return isset($this->getprops[$class_name][$prop_name]);
    }

    function get_prop($class_name, $prop_name)
    {
        return $this->getprops[$class_name][$prop_name];
    }

    function get_headers()
    {
        return $this->headers;
    }

    function get_constants()
    {
        return $this->constants;
    }

    function get_register_classes()
    {
        return $this->register_classes;
    }
}

/* vim: set et sts=4: */
?>
