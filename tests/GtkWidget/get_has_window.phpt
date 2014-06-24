--TEST--
GtkWidget->get_has_window method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'get_has_window')) die('skip - GtkWidget->get_has_window not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->get_has_window());

$window->set_has_window(false);
var_dump($window->get_has_window());

/* Wrong number args*/
$window->get_has_window(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkWidget::get_has_window() expects exactly 0 parameters, 1 given in %s on line %d