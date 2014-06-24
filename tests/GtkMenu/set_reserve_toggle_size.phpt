--TEST--
GtkMenu->set_reserve_toggle_size method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkMenu', 'set_reserve_toggle_size')) die('skip - GtkMenu->set_reserve_toggle_size not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$menu = new GtkMenu();

$menu->set_reserve_toggle_size(false);
var_dump($menu->get_reserve_toggle_size());

/* Wrong number args*/
$menu->set_reserve_toggle_size();
$menu->set_reserve_toggle_size(true, 1);

/* Arg 1 must be stringable */
$menu->set_reserve_toggle_size(array());
?>
--EXPECTF--
bool(false)

Warning: GtkMenu::set_reserve_toggle_size() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkMenu::set_reserve_toggle_size() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkMenu::set_reserve_toggle_size() expects argument 1 to be boolean, array given in %s on line %d