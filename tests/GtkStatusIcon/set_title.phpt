--TEST--
GtkStatusIcon->set_title method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkStatusIcon', 'set_title')) die('skip - GtkStatusIcon->set_title not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$icon = new GtkStatusIcon();

$icon->set_title('foobar');
var_dump($icon->get_title());

/* Wrong number args*/
$icon->set_title();
$icon->set_title('foobar', 1);

/* Arg 1 must be stringable */
$icon->set_title(array());
?>
--EXPECTF--
string(6) "foobar"

Warning: GtkStatusIcon::set_title() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkStatusIcon::set_title() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkStatusIcon::set_title() expects argument 1 to be string, array given in %s on line %d