--TEST--
GtkPrintOperation->set_embed_page_setup method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkPrintOperation', 'set_embed_page_setup')) die('skip - GtkPrintOperation->set_embed_page_setup not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$op = new GtkPrintOperation();

$op->set_embed_page_setup(true);
var_dump($op->get_embed_page_setup());

/* Wrong number args*/
$op->set_embed_page_setup();
$op->set_embed_page_setup(true, 1);

/* Arg 1 must be stringable */
$op->set_embed_page_setup(array());
?>
--EXPECTF--
bool(true)

Warning: GtkPrintOperation::set_embed_page_setup() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkPrintOperation::set_embed_page_setup() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkPrintOperation::set_embed_page_setup() expects argument 1 to be boolean, array given in %s on line %d