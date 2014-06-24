--TEST--
GtkLabel->get_current_uri method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkLabel', 'get_current_uri')) die('skip - GtkLabel->get_current_uri not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$label = new GtkLabel();

var_dump($label->get_current_uri());

/* Wrong number args*/
$label->get_current_uri(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
NULL

Warning: GtkLabel::get_current_uri() expects exactly 0 parameters, 1 given in %s on line %d