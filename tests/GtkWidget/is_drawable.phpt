--TEST--
GtkWidget->is_drawable method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'is_drawable')) die('skip - GtkWidget->is_drawable not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->is_drawable());

$window->show();

var_dump($window->is_drawable());

/* Wrong number args*/
$window->is_drawable(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(false)
bool(true)

Warning: GtkWidget::is_drawable() expects exactly 0 parameters, 1 given in %s on line %d