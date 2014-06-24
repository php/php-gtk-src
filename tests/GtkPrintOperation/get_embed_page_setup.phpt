--TEST--
GtkPrintOperation->get_embed_page_setup method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkPrintOperation', 'get_embed_page_setup')) die('skip - GtkPrintOperation->get_embed_page_setup not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$op = new GtkPrintOperation();

var_dump($op->get_embed_page_setup());
$op->set_embed_page_setup(true);
var_dump($op->get_embed_page_setup());

/* Wrong number args*/
$op->get_embed_page_setup(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(false)
bool(true)

Warning: GtkPrintOperation::get_embed_page_setup() expects exactly 0 parameters, 1 given in %s on line %d