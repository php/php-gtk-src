--TEST--
GtkWidget->get_double_buffered method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'get_double_buffered')) die('skip - GtkWidget->get_double_buffered not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->get_double_buffered());

$window->set_double_buffered(false);
var_dump($window->get_double_buffered());

/* Wrong number args*/
$window->get_double_buffered(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkWidget::get_double_buffered() expects exactly 0 parameters, 1 given in %s on line %d