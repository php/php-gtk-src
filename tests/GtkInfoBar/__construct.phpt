--TEST--
GtkInfoBar->__construct method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!class_exists('GtkInfoBar')) die('skip - GtkInfoBar not available, requires GTK 2.18 or higher');
if(!method_exists('GtkInfoBar', '__construct')) die('skip - GtkInfoBar->__construct not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$bar = new GtkInfoBar();

var_dump($bar instanceof GtkInfoBar);
echo $bar;

/* Wrong number args*/
try {
    new GtkInfoBar(1);
    trigger_error('GtkInfoBar did not throw exception');
} catch (PhpGtkConstructException $e) {
    echo $e->getMessage();
}

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(true)
[GtkInfoBar object (GtkInfoBar Gtk+ type)]
Warning: GtkInfoBar::__construct() expects exactly 0 parameters, 1 given in %s on line %d
could not construct GtkInfoBar object