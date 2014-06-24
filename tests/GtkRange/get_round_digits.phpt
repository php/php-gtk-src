--TEST--
GtkRange->get_round_digits() method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkRange', 'get_round_digits')) die('skip - GtkRange->get_round_digits available, requires GTK 2.24 or higher');
?>
--FILE--
<?php
$range = new GtkHScale();

var_dump($range->get_round_digits());
$range->set_round_digits(2);
var_dump($range->get_round_digits());

/* Wrong number args*/
$range->get_round_digits(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
int(1)
int(2)

Warning: GtkRange::get_round_digits() expects exactly 0 parameters, 1 given in %s on line %d