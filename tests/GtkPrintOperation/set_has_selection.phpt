--TEST--
GtkPrintOperation->set_has_selection method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkPrintOperation', 'set_has_selection')) die('skip - GtkPrintOperation->set_has_selection not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$op = new GtkPrintOperation();

$op->set_has_selection(false);
var_dump($op->get_has_selection());

/* Wrong number args*/
$op->set_has_selection();
$op->set_has_selection(true, 1);

/* Arg 1 must be stringable */
$op->set_has_selection(array());
?>
--EXPECTF--
bool(false)

Warning: GtkPrintOperation::set_has_selection() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkPrintOperation::set_has_selection() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkPrintOperation::set_has_selection() expects argument 1 to be boolean, array given in %s on line %d