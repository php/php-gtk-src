--TEST--
GtkRange->round-digits property
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkRange', 'set_round_digits')) die('skip - GtkRange->round-digits not available, requires GTK 2.24 or higher');
?>
--FILE--
<?php
$range = new GtkHScale();

var_dump($range->get_property('round-digits'));
$range->set_property('round-digits', 5);
var_dump($range->get_property('round-digits'));

/* Arg type must be intable */
$range->set_property('round-digits', new stdclass);
?>
--EXPECTF--
int(1)
int(5)

Notice: Object of class stdClass could not be converted to int in %s on line %d