<?php

error_reporting(E_ALL);
/* $Id$ */

if (!extension_loaded('php-gtk')) {
	dl( 'php_gtk.' . PHP_SHLIB_SUFFIX);
}
/*
 * Called when delete-event happens. Returns false to indicate that the event
 * should proceed.
 */
function delete_event()
{
	return false;
}

/*
 * Called when the window is being destroyed. Simply quit the main loop.
 */
function destroy()
{
	Gtk::main_quit();
}

/*
 * Called when button is clicked. Print the message and destroy the window.
 */
function hello()
{
	global	$window;
	print "Hello World!\n";
	$window->destroy();
}

/*
 * Create a new top-level window and connect the signals to the appropriate
 * functions. Note that all constructors must be assigned by reference.
 */
$window = new GtkWindow();
$window->connect('destroy', 'destroy');
$window->connect('delete-event', 'delete_event');
$window->set_border_width(10);

/*
 * Create a button, connect its clicked signal to hello() function and add
 * the button to the window.
 */
$button = new GtkButton('Hello World!');
$button->connect('clicked', 'hello');
/*
 * Create a new tooltips object and use it to set a tooltip for the button.
 */
$tt = new GtkTooltips();
$tt->set_delay(200);
$tt->set_tip($button, 'Prints "Hello World!"', '');
$tt->enable();

$window->add($button);
$x= $button->parent;
echo "VARDUMP: ";var_dump($x);
$button->visible = 0;
$button->sensitive = 0;
/*
 * Show the window and all its child widgets.
 */
$window->show_all();

/* Run the main loop. */
Gtk::main();

?>
