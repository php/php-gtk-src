--TEST--
GtkRange->get_flippable method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkRange', 'get_flippable')) die('skip - GtkRange->get_flippable not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$range = new GtkHScale();

var_dump($range->get_flippable());
$range->set_flippable(false);
var_dump($range->get_flippable());

/* Wrong number args*/
$range->get_flippable(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkRange::get_flippable() expects exactly 0 parameters, 1 given in %s on line %d