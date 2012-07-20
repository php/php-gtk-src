--TEST--
GtkNotebook->group-name property
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkNotebook', 'set_group_name')) die('skip - GtkNotebook->group-name not available, requires GTK 2.24 or higher');
?>
--FILE--
<?php
$notebook = new GtkNotebook();

var_dump($notebook->get_property('group-name'));
$notebook->set_property('group-name', 'foobar');
var_dump($notebook->get_property('group-name'));

/* Arg type must be intable */
$notebook->set_property('group-name', new stdclass);
?>
--EXPECTF--
NULL
string(6) "foobar"

Catchable fatal error: Object of class stdClass could not be converted to string in %s on line %d