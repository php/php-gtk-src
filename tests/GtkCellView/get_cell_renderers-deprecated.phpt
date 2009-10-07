--TEST--
GtkCellView->get_cell_renderers method DEPRECATED
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if($error = Gtk::check_version(2, 18, 0)) die('skip -requires GTK 2.18 or higher ' . $error);
?>
--INI--
error_reporting = E_ALL | E_DEPRECATED;
--FILE--
<?php
$cell = new GtkCellView();

var_dump($cell->get_cell_renderers());
?>
--EXPECTF--
%s: : since GTK+ 2.18, please use GtkCellLayout::get_cells() in %s on line %d

(%s:%d): Gtk-CRITICAL **: gtk_cell_view_set_cell_data: assertion `cell_view->priv->displayed_row != NULL' failed
array(0) {
}