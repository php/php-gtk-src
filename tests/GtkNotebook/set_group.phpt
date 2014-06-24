--TEST--
GtkNotebook->set_group method
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

// set to whatever
$notebook->set_group('foobar');
var_dump($notebook->get_group());

// set to null
$notebook->set_group(null);
var_dump($notebook->get_group());

// set to variable
$var = 'silly';
$notebook->set_group($var);
var_dump($notebook->get_group());

// set to null
$notebook->set_group();
var_dump($notebook->get_group());

// set to same as another notebook
$notebook2 = new GtkNotebook();
$notebook2->set_group($var);
$notebook->set_group($notebook2);
var_dump($notebook->get_group() === $notebook2->get_group());

/* Wrong number args */
$notebook->set_group(null, 1);

/* All args are acceptable, so no arg type checking */
?>
--EXPECTF--
int(0)
int(%d)
int(%d)
int(%d)
int(0)
bool(true)

Warning: GtkNotebook::set_group() requires at most 1 argument, 2 given in %s on line %d