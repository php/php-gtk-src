<?php

function aprintf($tmpl, $array)
{
	$array = (array)$array;
	if (count($array) < 1) return $tmpl;

	$callback =
		function ($matches) use ($array)
		{
			return isset($array[$matches[1]]) ? $array[$matches[1]] : "";
		};

	return preg_replace_callback('!%\((\w+)\)!', $callback, $tmpl);
}
?>