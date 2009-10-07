--TEST--
GtkWidget->has_focus method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'has_focus')) die('skip - GtkWidget->has_focus not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->has_focus());

/* Wrong number args*/
$window->has_focus(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(false)

Warning: GtkWidget::has_focus() expects exactly 0 parameters, 1 given in %s on line %d