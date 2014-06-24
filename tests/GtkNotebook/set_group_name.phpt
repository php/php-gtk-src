--TEST--
GtkNotebook->set_group_name() method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkNotebook', 'set_group_name')) die('skip - GtkNotebook->set_group_name not available, requires GTK 2.24 or higher');
?>
--FILE--
<?php
$notebook = new GtkNotebook();

var_dump($notebook->get_group_name());
$notebook->set_group_name('foobar');
var_dump($notebook->get_group_name());

/* Wrong number args*/
$notebook->set_group_name();
$notebook->set_group_name(1, 1);

/* Arg 1 must be stringable */
$notebook->set_group_name(array());
?>
--EXPECTF--
NULL
string(6) "foobar"

Warning: GtkNotebook::set_group_name() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkNotebook::set_group_name() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkNotebook::set_group_name() expects argument 1 to be string, array given in %s on line %d