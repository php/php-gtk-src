<?php

/* $Id$ */

if (!class_exists('gtk')) {
	if (strtoupper(substr(PHP_OS, 0,3) == 'WIN'))
		dl('php_gtk.dll');
	else
		dl('php_gtk.so');
}

function shutdown() {
	print("Shutting down.\n");
	Gtk::main_quit();
}

function EndDialog() {
	print("OK button clicked\n");
	return true;
}

function CancelDialog() {
	print("Cancel button clicked\n");
	return true;
}

function FileDialog() {
	$fs = &new GtkFileSelection('File Selection Example');
	$ok_button = $fs->ok_button;
	$ok_button->connect('clicked', 'enddialog');
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
