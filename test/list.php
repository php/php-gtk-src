<?

if (strtoupper(substr(PHP_OS, 0, 3)) == 'WIN')
	dl('php_gtk.dll');
else
	dl('php_gtk.so');


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
 * Create a new top-level window and connect the signals to the appropriate
 * functions. Note that all constructors must be assigned by reference.
 */
$window = &new GtkWindow();
$window->connect('destroy', destroy);
$window->connect('delete-event', delete_event);

/*
 * Create a button and connect its 'clicked' signal to destroy() function.
 */
$button = &new GtkButton('Close');
$button->connect('clicked', destroy);

/*
 * Create a vertical layout box.
 */
$box = &new GtkVBox(false, 10);
$box->set_border_width(10);

/*
 * Create a list widget and populate it with month names.
 */
$list = &new GtkList();
$list->set_selection_mode(GTK_SELECTION_BROWSE);
foreach (range(1, 12) as $month_num)
	$items[] = &new GtkListItem(strftime('%B', mktime(0, 0, 0, $month_num)));
$list->append_items($items);

/*
 * Create a scrolled window and add the list widget to it - this provides
 * automatic scrollbars.
 */
$scrolled_window = &new GtkScrolledWindow();
$scrolled_window->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
$scrolled_window->add_with_viewport($list);

/*
 * Add scrolled window and button to the vertical layout box.
 */
$box->pack_start($scrolled_window);
$box->pack_start($button, false);

/*
 * Add layout box to the window, set window attributes and show everything.
 */
$window->add($box);
$window->set_title('PHP Rules!');
$window->set_name('MainWindow');
$window->set_usize(150, 200);
$window->show_all();

/* Run the main loop. */
Gtk::main();

?>
