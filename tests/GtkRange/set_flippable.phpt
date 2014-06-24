--TEST--
GtkRange->set_flippable() method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkRange', 'set_flippable')) die('skip - GtkRange->set_flippable not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$range = new GtkHScale();

$range->set_flippable(true);
var_dump($range->get_flippable());

/* Wrong number args*/
$range->set_flippable();
$range->set_flippable(true, 1);

/* Arg 1 must be stringable */
$range->set_flippable(array());
?>
--EXPECTF--
bool(true)

Warning: GtkRange::set_flippable() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkRange::set_flippable() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkRange::set_flippable() expects argument 1 to be boolean, array given in %s on line %d