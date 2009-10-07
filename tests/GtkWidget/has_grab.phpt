--TEST--
GtkWidget->has_grab method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'has_grab')) die('skip - GtkWidget->has_grab not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->has_grab());

$window->grab_add();

var_dump($window->has_grab());

/* Wrong number args*/
$window->has_grab(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(false)
bool(true)

Warning: GtkWidget::has_grab() expects exactly 0 parameters, 1 given in %s on line %d