--TEST--
GtkWidget->get_app_paintable method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'get_app_paintable')) die('skip - GtkWidget->get_app_paintable not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->get_app_paintable());

$window->set_app_paintable(true);
var_dump($window->get_app_paintable());

/* Wrong number args*/
$window->get_app_paintable(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(false)
bool(true)

Warning: GtkWidget::get_app_paintable() expects exactly 0 parameters, 1 given in %s on line %d