<?php
$win = new GtkWindow();
$source = new GtkSourceView();
$source->set_show_line_numbers(true);
$source->set_show_line_markers(true);

$win->add($source);
$win->maximize();
$win->show_all();
$win->set_title("GtkSourceView Demo"); 
$win->connect_simple("destroy", array("Gtk", "main_quit"));

Gtk::main();
?>