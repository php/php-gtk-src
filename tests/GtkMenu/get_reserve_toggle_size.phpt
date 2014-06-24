--TEST--
GtkMenu->get_reserve_toggle_size method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkMenu', 'get_reserve_toggle_size')) die('skip - GtkMenu->get_reserve_toggle_size not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$menu = new GtkMenu();

var_dump($menu->get_reserve_toggle_size());
$menu->set_reserve_toggle_size(false);
var_dump($menu->get_reserve_toggle_size());

/* Wrong number args*/
$menu->get_reserve_toggle_size(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkMenu::get_reserve_toggle_size() expects exactly 0 parameters, 1 given in %s on line %d