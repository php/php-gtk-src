--TEST--
GtkStatusIcon->get_title method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkStatusIcon', 'get_title')) die('skip - GtkStatusIcon->get_title not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$icon = new GtkStatusIcon();

var_dump($icon->get_title());

$icon->set_title('foobar');
var_dump($icon->get_title());

/* Wrong number args*/
$icon->get_title(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
NULL
string(6) "foobar"

Warning: GtkStatusIcon::get_title() expects exactly 0 parameters, 1 given in %s on line %d