<?php
/**
*   FIXED by Andrei on 2005-10-19 22:38
*   php-gtk/ext/gtk+/gtktreeview.overrides:1.20
*   Problem reason: Wrong alloc used
*/
// Create a list store.
$listStore = new GtkListStore(Gtk::TYPE_STRING, Gtk::TYPE_LONG, Gtk::TYPE_DOUBLE);

// Add some product data.
$iter = $listStore->append(array('Crisscott T-Shirts',       10, 19.95));
$iter = $listStore->prepend(array('PHP-GTK Bumper Stickers', 37, 1.99));
$iter = $listStore->prepend(array('Pro PHP-GTK',             23, 44.95));

/**
 * Weird issue 1:
 * When the array is initialized to another array first
 * there is not problem, but if the array is created inside
 * the call to insert, it fails with the following message:

(listing9-3.php:1918): GLib-GObject-WARNING **: gvalue.c:89: cannot initialize GValue with type `gdouble', the value has already been initialized as `(null)'
PHP Warning:  PHP-GTK internal error: unsupported type (null) in /home/scott/authoring/Apress/chapter9/listing9-3.php on line 23
PHP Warning:  Cannot set row: type of element 2 does not match the model in /home/scott/authoring/Apress/chapter9/listing9-3.php on line 23

(listing9-3.php:1918): GLib-GObject-CRITICAL **: g_value_unset: assertion `G_IS_VALUE (value)' failed

 */
$iter = $listStore->insert(2, array('Crisscott Pencils', 18, .99));
// Comment the above line and uncomment these two to test.
//$tmp = array('Crisscott Pencils', 18, .99);
//$iter = $listStore->insert(2, $tmp);

/**
 * Weird issue 2:
 * If insert is called twice, weird issue 1 is not a problem.
 * Uncomment the two following lines to test. 
 * Make sure weird issue 1 is commented just to isolate the
 * issue.
 */
//$iter = $listStore->insert(2, array('Crisscott Pencils', 18, .99));
//$iter = $listStore->insert(3, array('Crisscott Pens', 18, .99));

// Create a veiw to show the list.
$view = new GtkTreeView();
$view->set_model($listStore);

// Create columns for each type of data.
$column = new GtkTreeViewColumn();
$column->set_title('Product Name');
$view->insert_column($column, 0);
// Create a renderer for the column.
$cell_renderer = new GtkCellRendererText();
$column->pack_start($cell_renderer, true);
$column->set_attributes($cell_renderer, 'text', 0);

// Create columns for each type of data.
$column = new GtkTreeViewColumn();
$column->set_title('Inventory');
$view->insert_column($column, 1);
// Create a renderer for the column.
$cell_renderer = new GtkCellRendererText();
$column->pack_start($cell_renderer, true);
$column->set_attributes($cell_renderer, 'text', 1);

// Create columns for each type of data.
$column = new GtkTreeViewColumn();
$column->set_title('Price');
$view->insert_column($column, 2);
// Create a renderer for the column.
$cell_renderer = new GtkCellRendererText();
$column->pack_start($cell_renderer, true);
$column->set_attributes($cell_renderer, 'text', 2);

// Create a window and show everything.
$window = new GtkWindow();
$window->add($view);
$window->show_all();
$window->connect_simple('destroy', array('gtk', 'main_quit'));
gtk::main();

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
?>
