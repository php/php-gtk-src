--TEST--
GtkWidget->get_sensitive method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'get_sensitive')) die('skip - GtkWidget->get_sensitive not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->get_sensitive());

$window->set_sensitive(false);
var_dump($window->get_sensitive());

/* Wrong number args*/
$window->get_sensitive(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkWidget::get_sensitive() expects exactly 0 parameters, 1 given in %s on line %d