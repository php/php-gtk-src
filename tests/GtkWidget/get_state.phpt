--TEST--
GtkWidget->get_state method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'get_state')) die('skip - GtkWidget->get_state not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->get_state());

/* Wrong number args*/
$window->get_state(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
int(0)

Warning: GtkWidget::get_state() expects exactly 0 parameters, 1 given in %s on line %d