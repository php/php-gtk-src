<?php
/* $Id$ */

if ($OS == 'Windows_NT')
	dl('php_gtk.dll');
else
	dl('php_gtk.so');

function delete_event()
{
	var_dump(func_get_args());
	return false;
}

function shutdown() {
	var_dump(func_get_args());
	print("Shutting down.\n");
	Gtk::main_quit();
}

function clicked()
{
	var_dump(func_get_args());
}

$window = &new GtkWindow();
$window->connect('destroy', 'shutdown');
$window->connect('delete-event', 'delete_event', 'foo');
$window->set_border_width(0);

$box = &new GtkVBox(FALSE, 0);
$table = &new GtkTable(2, 1, FALSE);
$table->set_row_spacings(10);
$table->set_col_spacings(10);
$table->set_border_width(10);
$box->pack_start($table, TRUE, TRUE, 0);

$button = &new GtkButton("Press Me");
$button->connect('clicked', 'clicked');

$table->attach($button, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

$label = &new GtkLabel("PHP Rules!\nPHP is simply the coolest scripting language!");
$label->set_justify(GTK_JUSTIFY_LEFT);
//$label->set_uposition(0,0);

$table->attach($label, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

$window->set_title("PHP Rules!");
$window->set_name("MainWindow");
$window->set_uposition(100,200);
$window->set_usize(600,150);
$window->add($box);
$window->show_all();
Gtk::main();

?>
