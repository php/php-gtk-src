--TEST--
GtkWidget->is_sensitive method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'is_sensitive')) die('skip - GtkWidget->is_sensitive not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->is_sensitive());

$window->set_sensitive(false);

var_dump($window->is_sensitive());

/* Wrong number args*/
$window->is_sensitive(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkWidget::is_sensitive() expects exactly 0 parameters, 1 given in %s on line %d