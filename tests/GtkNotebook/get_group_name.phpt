--TEST--
GtkNotebook->get_group_name() method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkNotebook', 'get_group_name')) die('skip - GtkNotebook->get_group_name not available, requires GTK 2.24 or higher');
?>
--FILE--
<?php
$notebook = new GtkNotebook();

var_dump($notebook->get_group_name());
$notebook->set_group_name('foobar');
var_dump($notebook->get_group_name());

/* Wrong number args*/
$notebook->get_group_name(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
NULL
string(6) "foobar"

Warning: GtkNotebook::get_group_name() expects exactly 0 parameters, 1 given in %s on line %d