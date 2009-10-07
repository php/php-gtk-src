--TEST--
GtkPrintOperation->get_support_selection  method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkPrintOperation', 'get_support_selection')) die('skip - GtkPrintOperation->get_support_selection  not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$op = new GtkPrintOperation();

var_dump($op->get_support_selection ());
$op->set_support_selection (true);
var_dump($op->get_support_selection ());

/* Wrong number args*/
$op->get_support_selection (1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(false)
bool(true)

Warning: GtkPrintOperation::get_support_selection() expects exactly 0 parameters, 1 given in %s on line %d