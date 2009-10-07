--TEST--
GtkPrintOperation->get_n_pages_to_print method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkPrintOperation', 'get_n_pages_to_print')) die('skip - GtkPrintOperation->get_n_pages_to_print not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$op = new GtkPrintOperation();

var_dump($op->get_n_pages_to_print());

/* Wrong number args*/
$op->get_n_pages_to_print(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
int(-1)

Warning: GtkPrintOperation::get_n_pages_to_print() expects exactly 0 parameters, 1 given in %s on line %d