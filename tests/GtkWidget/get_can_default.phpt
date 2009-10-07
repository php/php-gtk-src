--TEST--
GtkWidget->get_can_default method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'get_can_default')) die('skip - GtkWidget->get_can_default not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->get_can_default());

$window->set_can_default(true);
var_dump($window->get_can_default());

/* Wrong number args*/
$window->get_can_default(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(false)
bool(true)

Warning: GtkWidget::get_can_default() expects exactly 0 parameters, 1 given in %s on line %d