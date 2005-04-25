<?php

/*
 * This really is not a recommended way to load the PHP-GTK, but it will suffice
 * for the demo purposes.
 */
if( !class_exists('gtk')) {
	die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}

class StockItemInfo {
	public $stock_id = '';
	public $stock_item = null;
	public $small_icon = null;
	public $constant = '';
	public $accel_str = '';

	function __construct($stock_id = null) {
		$this->stock_id = $stock_id;
		if ($stock_id) {
			$this->constant = id_to_constant($stock_id);
		}
	}
}

class StockItemDisplay {
	public $type_label;
	public $constant_label;
	public $id_label;
	public $accel_label;
	public $icon_image;
}

class StockItemBrowserDemo extends GtkWindow {
	function __construct($parent = null)
	{
		parent::__construct();

		if (@$GLOBALS['framework']) {
			return;
		}

		if ($parent)
			$this->set_screen($parent->get_screen());
		else
			$this->connect_object('destroy', array('gtk', 'main_quit'));

		$this->set_title(__CLASS__);
		$this->set_position(Gtk::WIN_POS_CENTER);
		$this->set_default_size(-1, 500);
		$this->set_border_width(8);
		
		$this->add($this->__create_box());
		$this->show_all();
	}

	function __create_box()
	{
		$hbox = new GtkHBox(false, 8);

		$scrolled = new GtkScrolledWindow();
		$scrolled->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
		$scrolled->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
		$hbox->pack_start($scrolled, false, false, 0);

		$model = $this->create_model();
		$treeview = new GtkTreeView($model);
		$scrolled->add($treeview);

		$column = new GtkTreeViewColumn();
		$column->set_title('Icon and Constant');

		$cell_renderer = new GtkCellRendererPixbuf();
		$column->pack_start($cell_renderer, false);
		$column->set_attributes($cell_renderer, 'stock-id', 1);

		$cell_renderer = new GtkCellRendererText();
		$column->pack_start($cell_renderer, true);
		$column->set_cell_data_func($cell_renderer, 'constant_setter');

		$treeview->append_column($column);

		$cell_renderer = new GtkCellRendererText();
		$treeview->insert_column_with_data_func(-1, 'Label', $cell_renderer, 'label_setter');

		$cell_renderer = new GtkCellRendererText();
		$treeview->insert_column_with_data_func(-1, 'Accelerator', $cell_renderer, 'accel_setter');

		$cell_renderer = new GtkCellRendererText();
		$treeview->insert_column_with_data_func(-1, 'ID', $cell_renderer, 'id_setter');

		$align = new GtkAlignment(0.5, 0, 0, 0);
		$hbox->pack_end($align, true, true, 0);

		$frame = new GtkFrame('Selection Info');
		$align->add($frame);

		$vbox = new GtkVBox(false, 8);
		$vbox->set_border_width(4);
		$frame->add($vbox);

		$display = new StockItemDisplay();
		$treeview->set_data('stock-display', $display);

		$display->type_label = new GtkLabel();
		$display->constant_label = new GtkLabel();
		$display->id_label = new GtkLabel();
		$display->accel_label = new GtkLabel();
		$display->icon_image = new GtkImage();

		$vbox->pack_start($display->type_label, false, false, 0);
		$vbox->pack_start($display->icon_image, false, false, 0);
		$vbox->pack_start($display->accel_label, false, false, 0);
		$vbox->pack_start($display->constant_label, false, false, 0);
		$vbox->pack_start($display->id_label, false, false, 0);

		$selection = $treeview->get_selection();
		$selection->set_mode(Gtk::SELECTION_SINGLE);

		$selection->connect('changed', array($this, 'on_selection_changed'));

		
		return $hbox;
	}

	private function create_model()
	{
		$store = new GtkListStore(Gtk::TYPE_PHP_VALUE, Gtk::TYPE_STRING);

		$ids = Gtk::stock_list_ids();
		sort($ids);

		foreach ($ids as $id) {
			$info = new StockItemInfo($id);
			$stock_item = Gtk::stock_lookup($id);
			if ($stock_item)
				$info->stock_item = $stock_item;
			else
				$info->stock_item = array('', '', 0, 0, '');

			$icon_set = GtkIconFactory::lookup_default($id);
			if ($icon_set) {
				$sizes = $icon_set->get_sizes();
				$size = $sizes[0];
				for ($i = 0; $i < count($sizes); $i++) {
					if ($sizes[$i] == Gtk::ICON_SIZE_MENU) {
						$size = Gtk::ICON_SIZE_MENU;
						break;
					}
				}
				$info->small_icon = $this->render_icon($info->stock_id, $size);
				if ($size != Gtk::ICON_SIZE_MENU) {
					list($width, $height) = Gtk::icon_size_lookup(Gtk::ICON_SIZE_MENU);

					$info->small_icon = $info->small_icon->scale_simple($width, $height, 'bilinear');
				}

			} else {
				$info->small_icon = null;
			}

			if ($info->stock_item[3] == 0) {
				$info->accel_str = '';
			} else {
				$info->accel_str = '<'.Gtk::accelerator_get_label($info->stock_item[3], $info->stock_item[2]).'>';
			}

			$iter = $store->append();
			$store->set($iter, 0, $info, 1, $id);
		}

		return $store;
	}

	function on_selection_changed($selection)
	{
		$treeview = $selection->get_tree_view();
		$display = $treeview->get_data('stock-display');

		list($model, $iter) = $selection->get_selected();
		if ($iter) {
			$info = $model->get_value($iter, 0);

			if ($info->small_icon && $info->stock_item[1])
				$display->type_label->set_text('Item and Icon');
			else if ($info->small_icon)
				$display->type_label->set_text('Icon Only');
			else if ($info->stock_item[1])
				$display->type_label->set_text('Item Only');
			else
				$display->type_label->set_text('<unknown>');

			$display->constant_label->set_text($info->constant);
			$display->id_label->set_text($info->stock_id);

			if ($info->stock_item[1])
				$display->accel_label->set_text_with_mnemonic($info->stock_item[1].' '.$info->accel_str);
			else
				$display->accel_label->set_text('');

			if ($info->small_icon)
				$display->icon_image->set_from_stock($info->stock_id, get_largest_size($info->stock_id));
			else
				$display->icon_image->set_from_pixbuf(null);
		} else {
			$display->type_label->set_text('No Selected Item');
			$display->icon_image->set_from_pixbuf(null);
			$display->constant_label->set_text('');
			$display->id_label->set_text('');
			$display->accel_label->set_text('');
		}
	}
}

function id_to_constant($id)
{
	if (substr($id, 0, 3) == 'gtk') {
		$constant = 'Gtk::STOCK' . preg_replace('!-([^-]+)!e', '"_".strtoupper("$1")', substr($id, 3));
	} else {
		$constant = substr(preg_replace('!([^-]+)-?!e', 'strtoupper($1)', $id), 1);
	}

	return $constant;
}

function get_largest_size($id)
{
	$icon_set = GtkIconFactory::lookup_default($id);
	$best_size = Gtk::ICON_SIZE_INVALID;
	$best_pixels = 0;

	$sizes = $icon_set->get_sizes();
	foreach ($sizes as $size) {
		list($w, $h) = Gtk::icon_size_lookup($size);
		if ($w * $h > $best_pixels) {
			$best_size = $size;
			$best_pixels = $w * $h;
		}
	}

	return $best_size;
}

function constant_setter($column, $cell, $model, $iter)
{
	$info = $model->get_value($iter, 0);
	$cell->set_property('text', $info->constant);
}

function label_setter($column, $cell, $model, $iter)
{
	$info = $model->get_value($iter, 0);
	$cell->set_property('text', $info->stock_item[1]);
}

function accel_setter($column, $cell, $model, $iter)
{
	$info = $model->get_value($iter, 0);
	$cell->set_property('text', $info->accel_str);
}

function id_setter($column, $cell, $model, $iter)
{
	$info = $model->get_value($iter, 0);
	$cell->set_property('text', $info->stock_id);
}


$GLOBALS['class']		= 'StockItemBrowserDemo';
$GLOBALS['description']	= 'This source code for this demo doesn\'t demonstrate anything particularly useful in applications. The purpose of the "demo" is just to provide a handy place to browse the available stock icons and stock items.';

if (!@$GLOBALS['framework']) {
	new StockItemBrowserDemo();
	Gtk::main();
}
?>
