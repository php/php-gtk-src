<?php

require "Getopt.php";
require "scheme.php";

$result = Console_Getopt::getopt($argv, '');

if (!$result || count($result[1]) < 3)
    die("usage: php merge-defs.php generated-defs old-defs\n");

list($opts, $argv) = $result;

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

$new_defs->write_missing_defs($old_defs);
