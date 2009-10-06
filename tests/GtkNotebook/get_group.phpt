--TEST--
GtkNotebook->get_group method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkNotebook', 'get_group')) die('skip - GtkNotebook->get_group not available, requires GTK 2.12 or higher');
?>
--FILE--
<?php
$notebook = new GtkNotebook();

// should be null
var_dump($notebook->get_group());

// must be int
$notebook->set_group('foobar');
var_dump($notebook->get_group());

/* Wrong number args */
$notebook->get_group(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
int(0)
int(%d)

Warning: GtkNotebook::get_group() expects exactly 0 parameters, 1 given in %s on line %d