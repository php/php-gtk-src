<?php
/* $Id$ */

if (!extension_loaded('gtk')) {
	dl( 'php_gtk.' . PHP_SHLIB_SUFFIX);
}

function shutdown() {
	print "Shutting down.\n" ;
	Gtk::main_quit();
}

function EndDialog($button, $fs) {
	print "OK button clicked, ";
	print $fs->get_filename();
	print " selected.\n";
	return true;
}

function CancelDialog() {
	print("Cancel button clicked\n");
	return true;
}

function FileDialog() {
	$fs = &new GtkFileSelection('File Selection Example');
	$ok_button = $fs->ok_button;
	$ok_button->connect('clicked', 'enddialog', $fs);
	$ok_button->connect_object('clicked', array($fs, 'destroy'));
	$cancel_button = $fs->cancel_button;
	$cancel_button->connect('clicked', 'canceldialog');
	$cancel_button->connect_object('clicked', array($fs, 'destroy'));
	$fs->show();
}

$window = &new GtkWindow();
$window->connect('destroy', 'shutdown');

$button = &new GtkButton('Open File Dialog');
$button->connect('clicked', 'FileDialog');
$window->add($button);

$window->show_all();
Gtk::main();

?>
