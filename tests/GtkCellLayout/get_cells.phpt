--TEST--
GtkCellLayout->get_cells method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if($error = Gtk::check_version(2, 18, 0)) die('skip -requires GTK 2.18 or higher ' . $error);
?>
--INI--
error_reporting = E_ALL | E_DEPRECATED;
--FILE--
<?php

// Use any class that implements the GtkCellLayout interface

// First try empty 
$column = new GtkTreeViewColumn();

var_dump($column->get_cells());

// Giving a paramenter should result in warning
var_dump($column->get_cells(1));

unset($column);

// Now lets try with a cell renderer
$column = new GtkTreeViewColumn("test", new GtkCellRendererText(), "text", 0);

// Should return an array
var_dump($column->get_cells());

?>
--EXPECTF--
array(0) {
}
PHP Warning:  GtkTreeViewColumn::get_cells() expects exactly 0 parameters, 1 given in %s on line %d
NULL
array(1) {
  [0]=>
  object(GtkCellRendererText)#2 (1) {
    ["gtype"]=>
    int(%i)
  }
}
