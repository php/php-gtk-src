--TEST--
GtkWidget->is_visible method DEPRECATED
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if($error = Gtk::check_version(2, 18, 0)) die('skip -requires GTK 2.18 or higher ' . $error);
?>
--INI--
error_reporting = E_ALL | E_DEPRECATED;
--FILE--
<?php
$window = new GtkWindow();

$window->is_visible();
?>
--EXPECTF--
%s: : since GTK+ 2.18, please use GtkWidget::get_visible() in %s on line %d