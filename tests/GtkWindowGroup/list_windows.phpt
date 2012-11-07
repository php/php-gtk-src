--TEST--
GtkWindowGroup->list_windows method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWindowGroup', 'list_windows')) die('skip - GtkStatusIcon->get_title not available, requires GTK 2.14 or higher');
?>
--FILE--
<?php

$group = new GtkWindowGroup();

$window = new GtkWindow();
$window2 = new GtkWindow();

//Add a couple of windows to test
$group->add_window($window);
$group->add_window($window2);

$list = $group->list_windows();

var_dump(is_array($list));

foreach ($list as $obj){
    echo $obj.PHP_EOL;
}

/* Test wrong arg */
var_dump($group->list_windows(1));
?>
--EXPECTF--
bool(true)
[GtkWindow object (GtkWindow Gtk+ type)]
[GtkWindow object (GtkWindow Gtk+ type)]
PHP Warning:  GtkWindowGroup::list_windows() expects exactly 0 parameters, 1 given in %s on line %d
NULL
