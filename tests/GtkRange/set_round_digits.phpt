--TEST--
GtkRange->set_round_digits() method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkRange', 'set_flippable')) die('skip - GtkRange->set_round_digits not available, requires GTK 2.24 or higher');
?>
--FILE--
<?php
$range = new GtkHScale();

$range->set_round_digits(4);
var_dump($range->get_round_digits());

/* Wrong number args*/
$range->set_round_digits();
$range->set_round_digits(1, 1);

/* Arg 1 must be intable */
$range->set_round_digits(array());
?>
--EXPECTF--
int(4)

Warning: GtkRange::set_round_digits() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkRange::set_round_digits() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkRange::set_round_digits() expects argument 1 to be integer, array given in %s on line %d