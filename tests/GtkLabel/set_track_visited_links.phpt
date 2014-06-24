--TEST--
GtkLabel->set_track_visited_links method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkLabel', 'set_track_visited_links')) die('skip - GtkLabel->set_track_visited_links not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$label = new GtkLabel();

$label->set_track_visited_links(true);
var_dump($label->get_track_visited_links());

/* Wrong number args*/
$label->set_track_visited_links();
$label->set_track_visited_links(true, 1);

/* Arg 1 must be stringable */
$label->set_track_visited_links(array());
?>
--EXPECTF--
bool(true)

Warning: GtkLabel::set_track_visited_links() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkLabel::set_track_visited_links() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkLabel::set_track_visited_links() expects argument 1 to be boolean, array given in %s on line %d