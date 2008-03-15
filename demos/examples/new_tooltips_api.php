<?php
/* This API is only available afer 2.12, before then use the OLD api (see example) */
if(Gtk::check_version(2, 12, 0))
die ('New Tooltip API only available in GTK 2.12 or higher');

/* Advanced tooltips functionality callbacks */
function markuptip($widget, $x, $y, $keyboard_mode, $tooltip)
{
	$tooltip->set_markup('<b>I am bold</b>');
	return true; // return false or nothing to NOT show a tooltip
}

function texttip($widget, $x, $y, $keyboard_mode, $tooltip)
{
	$tooltip->set_text('Foobar me');
	return true; // return false or nothing to NOT show a tooltip
}

function icontip($widget, $x, $y, $keyboard_mode, $tooltip)
{
	$tooltip->set_icon(GdkPixbuf::new_from_file(dirname(__FILE__) . '/../apple-red.png'));
	return true; // return false or nothing to NOT show a tooltip
}

function stocktip($widget, $x, $y, $keyboard_mode, $tooltip)
{
	$tooltip->set_icon_from_stock(Gtk::STOCK_HELP, Gtk::ICON_SIZE_LARGE_TOOLBAR);
	return true; // return false or nothing to NOT show a tooltip
}

function widgettip($widget, $x, $y, $keyboard_mode, $tooltip)
{
	$newwidget = new GtkEntry();
	$tooltip->set_custom($newwidget);
	return true; // return false or nothing to NOT show a tooltip
}

function textonlytip($widget, $x, $y, $keyboard_mode, $tooltip)
{
	// get current size of widet
	$size = $widget->get_allocation();
	// get current padding for widget
	list($width, $height) = $widget->get_padding();
	// create rectangle to actually use for tip
	$rect = new GdkRectangle($width, $height, $size->width - $width, $size->height - $height);
	// tell the tooltip to only appear over that area
	$tooltip->set_tip_area($rect);
	$tooltip->set_text('Only mouseover on text shows me');
	return true; // return false or nothing to NOT show a tooltip
}

function redboxtip($widget, $x, $y, $keyboard_mode, $tooltip)
{
	$tipwindow = $widget->get_tooltip_window();
	$tipwindow->modify_bg(Gtk::STATE_NORMAL, GdkColor::parse("#CC0000"));
	return true; // return false or nothing to NOT show a tooltip
}

/* Create base window */
$window = new GtkWindow();
$window->set_title('New Tooltip API');
$window->connect_simple('destroy', array('gtk', 'main_quit'));
$window->add($vbox = new GtkVBox());

// simple tooltips usage
$vbox->add($label = new GtkLabel('I have a plain tooltip'));
$label->set_tooltip_text('I am a tooltip, Dave');

// simple tooltips usage with markup
$vbox->add($label = new GtkLabel('I have a markup tooltip'));
$label->set_tooltip_markup('<i>I am cool and italic</i>');

// text
$vbox->add($text = new GtkLabel('Text Tooltip'));
$text->set_has_tooltip(true);
 // $text->set_property('has-tooltip', true); - notice this does the same thing
$text->connect('query-tooltip', 'texttip');

// markup
$vbox->add($text = new GtkLabel('Markup Tooltip'));
$text->set_has_tooltip(true);
$text->connect('query-tooltip', 'markuptip');

// icon
$vbox->add($text = new GtkLabel('Icon Tooltip'));
$text->set_has_tooltip(true);
$text->connect('query-tooltip', 'icontip');

// stock
$vbox->add($text = new GtkLabel('Stock Icon Tooltip'));
$text->set_has_tooltip(true);
$text->connect('query-tooltip', 'stocktip');

// widget
$vbox->add($text = new GtkLabel('Widget Tooltip'));
$text->set_has_tooltip(true);
$text->connect('query-tooltip', 'widgettip');

// custom tooltip area
$vbox->add($text = new GtkLabel('No tip on padding'));
$text->set_padding(15, 25);
$text->set_has_tooltip(true);
$text->connect('query-tooltip', 'textonlytip');

// be brave, use your own tips window
$vbox->add($text = new GtkLabel('Different Tooltips Window'));
$tipswindow = new GtkWindow(Gtk::WINDOW_POPUP);
$content = new GtkLabel('I feel pretty');
$tipswindow->add($content);
$content->show();
$text->set_tooltip_window($tipswindow);
$text->connect('query-tooltip', 'redboxtip');

// trigger tooltip query for the display on button press
$vbox->add($button = new GtkButton('Trigger Tooltip passing Display'));
$button->connect_simple('clicked', array('GtkTooltip', 'trigger_tooltip_query'), GdkDisplay::get_default());
$button->set_has_tooltip(true);
$button->connect('query-tooltip', 'texttip');

// trigger tooltip query for a display where the widget is on button press
$vbox->add($button = new GtkButton('Trigger Tooltip passing Widget'));
$button->connect_simple('clicked', array($label, 'trigger_tooltip_query'));
$button->set_has_tooltip(true);
$button->connect('query-tooltip', 'texttip');

/* Generic liststore for the next tests */
$store = new GtkListStore(Gobject::TYPE_STRING, Gobject::TYPE_STRING, GdkPixbuf::gtype);
$store->append(array('foo', 'tip1', $window->render_icon(Gtk::STOCK_OPEN, Gtk::ICON_SIZE_DIALOG)));
$store->append(array('bar', 'tip2', $window->render_icon(Gtk::STOCK_CLOSE, Gtk::ICON_SIZE_DIALOG)));
$store->append(array('baz', 'tip3', $window->render_icon(Gtk::STOCK_SAVE, Gtk::ICON_SIZE_DIALOG)));
$store->append(array('kitty', 'tip4', $window->render_icon(Gtk::STOCK_OK, Gtk::ICON_SIZE_DIALOG)));

// callback that tells us if we have an item at the spot
function checkcontext($widget, $x, $y, $keyboard_mode, $tooltip)
{
	$list = $widget->get_tooltip_context($x, $y, $keyboard_mode);
	if(!$list)
	{
		return false;
	}
	list($model, $path, $iter) = $list;
	$tooltip->set_text($model->get_value($iter, 1));
	$widget->set_tooltip_item($tooltip, $path); // use set_tooltip_row for textview
	$widget->set_tooltip_cell($tooltip, $path, null); // you can pass a specific cell renderer as the third arg here
	return true; //show tip
}

/* New tooltip iconview stuff */
$iconview = new GtkIconView($store);
$iconview->set_pixbuf_column(2);
$iconview->set_text_column(0);
$vbox->add($iconview);
// simple tooltips for an iconview
var_dump($iconview->get_tooltip_column()); // notice this is -1 for disabled
//$iconview->set_tooltip_column(1); // this is the "shortcut" way to set tips, see below

// fancy tooltip querying
$iconview->set_has_tooltip(true);
$iconview->connect('query-tooltip', 'checkcontext');

/* New tooltip treeview stuff */
$treeview = new GtkTreeView($store);
$treeview->append_column(new GtkTreeViewColumn('stuff', new GtkCellRendererText, 'text', 0));
$vbox->add($treeview);
// simple text only tooltips can be automatic on treeviews - use -1 to turn it off
$treeview->set_tooltip_column(1);
var_dump($treeview->get_tooltip_column());
// check our context on query_tooltip
$treeview->connect('query-tooltip', 'checkcontext');

/* Change overall tooltips settings */
$settings = GtkSettings::get_default();

/* Amount of time, in milliseconds, after which the browse mode will be disabled */
echo 'default is ';
var_dump($settings->get_property('gtk-tooltip-browse-mode-timeout'));
$settings->set_property('gtk-tooltip-browse-mode-timeout', 300);

/* Controls the time after which tooltips will appear when browse mode is enabled, in milliseconds
   Browse mode is enabled when the mouse pointer moves off an object where a tooltip
   was currently being displayed. If the mouse pointer hits another object before
   the browse mode timeout expires it will take this time to popup the tooltip for the new object. */
echo 'default is ';
var_dump($settings->get_property('gtk-tooltip-browse-timeout'));
$settings->set_property('gtk-tooltip-browse-timeout', 30);

/* Tooltip timeout */
echo 'default is ';
var_dump($settings->get_property('gtk-tooltip-timeout'));
$settings->set_property('gtk-tooltip-timeout', 400);

$window->show_all();
$label->trigger_tooltip_query();
Gtk::main();
?>