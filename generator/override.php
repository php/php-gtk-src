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

class Overrides {
	var $ignores 		= array();
	var $glob_ignores 	= array();
	var $overrides		= array();
	var $extra_methods	= array();
	var $getprops		= array();

	function Overrides($file_name = null)
	{
		if (!isset($file_name))
			return;

		$fp = fopen($file_name, 'r');
		$contents = fread($fp, filesize($file_name));
		fclose($fp);

		$blocks = explode('%%', $contents);
		if (!count($blocks))
			return;

		foreach ($blocks as $block)
			$this->_parse_block(trim($block));
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
					$this->extra_methods[$words[2]][$func_cname] = array($func_name, $rest);
				else
					$this->overrides[$func_cname] = array($func_name, $rest);
				break;

			case 'getprop':
				if (count($words) >= 2) {
					list($class_name, $prop_name) = $words;
					$this->getprops[$class_name][$prop_name] = $rest;
				}
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

	function have_get_prop($class_name, $prop_name)
	{
		return isset($this->getprops[$class_name][$prop_name]);
	}

	function get_prop($class_name, $prop_name)
	{
		return $this->getprops[$class_name][$prop_name];
	}
}

?>
