--TEST--
GtkWidget->get_visible method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'get_visible')) die('skip - GtkWidget->get_visible not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->get_visible());

$window->show();
var_dump($window->get_visible());

/* Wrong number args*/
$window->get_visible(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(false)
bool(true)

Warning: GtkWidget::get_visible() expects exactly 0 parameters, 1 given in %s on line %d