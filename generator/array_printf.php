<?php

function aprintf($tmpl, $array)
{
	$array = (array)$array;
	if (count($array) < 1) return $tmpl;

	return preg_replace('!%\((\w+)\)!e', 'isset(\$array["$1"]) ? \$array["$1"] : ""', $tmpl);
}

?>
