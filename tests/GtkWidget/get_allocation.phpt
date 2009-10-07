--TEST--
GtkWidget->get_allocation method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
?>
--FILE--
<?php
$window = new GtkWindow();

// underlying implementation changes with gtk 2.18 - but looks same to userland
var_dump($window->get_allocation());

/* Wrong number args */
$window->get_allocation(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
object(GdkRectangle)#%d (4) {
  ["x"]=>
  int(%i)
  ["y"]=>
  int(%i)
  ["width"]=>
  int(%i)
  ["height"]=>
  int(%i)
}

Warning: GtkWidget::get_allocation() expects exactly 0 parameters, 1 given in %s on line %d