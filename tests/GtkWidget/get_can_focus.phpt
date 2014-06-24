--TEST--
GtkWidget->get_can_focus method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'get_can_focus')) die('skip - GtkWidget->get_can_focus not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->get_can_focus());

$window->set_can_focus(true);
var_dump($window->get_can_focus());

/* Wrong number args*/
$window->get_can_focus(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(false)
bool(true)

Warning: GtkWidget::get_can_focus() expects exactly 0 parameters, 1 given in %s on line %d