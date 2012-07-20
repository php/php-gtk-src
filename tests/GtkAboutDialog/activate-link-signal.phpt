--TEST--
GtkAboutDialog::activate-link signal
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkNotebook', 'set_group_name')) die('skip - GtkAboutDialog::activate-link not available, requires GTK 2.24 or higher');
?>
--FILE--
<?php
function activate_link_callback($label, $uri) {
	var_dump($uri);
	var_dump($label instanceof GtkLabel);
	return true;
}
$dialog = new GtkAboutDialog();
$dialog->connect('activate-link', 'activate_link_callback');
$dialog->emit('activate-link', 'http://foobar');
?>
--EXPECTF--
string(13) "http://foobar"
bool(false)