<?php

require "Getopt.php";
require "scheme.php";

$result = Console_Getopt::getopt($argv, 'p');

if (!$result || count($result[1]) < 3)
    die("usage: php merge-defs.php [-p] generated-defs old-defs\n");

$merge_params = false;

list($opts, $argv) = $result;
foreach ($opts as $opt) {
	if ($opt[0] == 'p') {
		$merge_params = true;
	}
}

class Merge_Parser extends Defs_Parser {
	function handle_include()
	{
		/* pass */
	}
}

$new_defs = new Merge_Parser($argv[1]);
$old_defs = new Merge_Parser($argv[2]);

$new_defs->start_parsing();
$old_defs->start_parsing();

$new_defs->merge($old_defs, $merge_params);

$new_defs->write_defs();
