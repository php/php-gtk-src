--TEST--
GtkLabel->get_track_visited_links method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkLabel', 'get_track_visited_links')) die('skip - GtkLabel->set_track_visited_links not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$label = new GtkLabel();

var_dump($label->get_track_visited_links());
$label->set_track_visited_links(false);
var_dump($label->get_track_visited_links());

/* Wrong number args*/
$label->get_track_visited_links(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkLabel::get_track_visited_links() expects exactly 0 parameters, 1 given in %s on line %d