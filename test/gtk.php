<?php
/* $Id$ */

if (!extension_loaded('gtk')) {
	dl( 'php_gtk.' . PHP_SHLIB_SUFFIX);
}

$windows = array();

$book_closed_xpm = array("16 16 6 1",
						 "       c None s None",
						 ".      c black",
						 "X      c red",
						 "o      c yellow",
						 "O      c #808080",
						 "#      c white",
						 "                ",
						 "       ..       ",
						 "     ..XX.      ",
						 "   ..XXXXX.     ",
						 " ..XXXXXXXX.    ",
						 ".ooXXXXXXXXX.   ",
						 "..ooXXXXXXXXX.  ",
						 ".X.ooXXXXXXXXX. ",
						 ".XX.ooXXXXXX..  ",
						 " .XX.ooXXX..#O  ",
						 "  .XX.oo..##OO. ",
						 "   .XX..##OO..  ",
						 "    .X.#OO..    ",
						 "     ..O..      ",
						 "      ..        ",
						 "                ");

$book_open_xpm = array("16 16 4 1",
					   "       c None s None",
					   ".      c black",
					   "X      c #808080",
					   "o      c white",
					   "                ",
					   "  ..            ",
					   " .Xo.    ...    ",
					   " .Xoo. ..oo.    ",
					   " .Xooo.Xooo...  ",
					   " .Xooo.oooo.X.  ",
					   " .Xooo.Xooo.X.  ",
					   " .Xooo.oooo.X.  ",
					   " .Xooo.Xooo.X.  ",
					   " .Xooo.oooo.X.  ",
					   "  .Xoo.Xoo..X.  ",
					   "   .Xo.o..ooX.  ",
					   "    .X..XXXXX.  ",
					   "    ..X.......  ",
					   "     ..         ",
					   "                ");

function delete_event($window, $event)
{
	$window->hide();
	return true;
}

function close_window($widget)
{
	$window = $widget->get_toplevel();
	$window->hide();
}

function build_option_menu($items, $history = null)
{
	$omenu = &new GtkOptionMenu();
	$menu = &new GtkMenu();

	foreach ($items as $item_name => $callback_data) {
		$menu_item = &new GtkMenuItem($item_name);
		array_unshift($callback_data, 'activate');
		call_user_func_array(array(&$menu_item, 'connect'), $callback_data);
		$menu->append($menu_item);
		$menu_item->show();
	}

	$omenu->set_menu($menu);
	if ($history !== null)
		$omenu->set_history($history);

	return $omenu;
}


function toggle_reorderable($button, $clistwidget)
{
	global $clist; // until I fix callback arguments
	$clist->set_reorderable($button->get_active());
}


$ctree_data['books'] = 1;
$ctree_data['pages'] = 0;

function create_dnd()
{
	global	$windows;

	if (!isset($windows['dnd'])) {
		$targets = array(array('text/plain', 0, -1));

		function dnd_drag_data_get($widget, $context, $selection_data, $info, $time)
		{
			  print_r(func_get_args());
              $dnd_string = "Perl is the only language that looks\nthe same before and after RSA encryption";
			$selection_data->set($selection_data->target, 8, $dnd_string);
		}

		function dnd_drag_data_received($widget, $context, $x, $y, $data, $info, $time)
		{
			if ($data && $data->format == 8)
				print "Drop data of type " . $data->target->string . " was:\n'$data->data'.\n";
		}
		
		$window = &new GtkWindow;
		$windows['dnd'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('Drag-n-Drop');

		$box1 = &new GtkVBox();
		$window->add($box1);
		$box1->show();

		$box2 = &new GtkHBox(false, 5);
		$box2->set_border_width(10);
		$box1->pack_start($box2);
		$box2->show();
		
		$frame = &new GtkFrame('Drag');
		$box2->pack_start($frame);
		$frame->show();

		$box3 = &new GtkVBox(false, 5);
		$box3->set_border_width(5);
		$frame->add($box3);
		$box3->show();

		$button = &new GtkButton('Drag me!');
		$box3->pack_start($button);
		$button->show();
		$button->connect('drag_data_get', 'dnd_drag_data_get');
		$button->drag_source_set(GDK_BUTTON1_MASK|GDK_BUTTON3_MASK, $targets,
								 GDK_ACTION_COPY);

		$frame = &new GtkFrame('Drop');
		$box2->pack_start($frame);
		$frame->show();

		$box3 = &new GtkVBox(false, 5);
		$box3->set_border_width(5);
		$frame->add($box3);
		$box3->show();

		$button = &new GtkButton('To');
		$box3->pack_start($button);
		$button->show();
		//$button->realize();
		$button->connect('drag_data_received', 'dnd_drag_data_received');
		$button->drag_dest_set(GTK_DEST_DEFAULT_ALL, $targets, GDK_ACTION_COPY);

		$separator = &new GtkHSeparator();
		$box1->pack_start($separator, false);
		$separator->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2, false);
		$box2->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$box2->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
//	if ($windows['dnd']->flags() & GTK_VISIBLE)
//		$windows['dnd']->hide();
//	else
		$windows['dnd']->show();
}

function create_ctree()
{
	global	$windows,
			$ctree_data,
			$book_closed_xpm,
			$book_open_xpm;

	if (!isset($windows['ctree'])) {
		function rebuild_tree($button, $ctree)
		{
			global	$ctree_data;

			$d = $ctree_data['spin1']->get_value_as_int();
			$b = $ctree_data['spin2']->get_value_as_int();
			$p = $ctree_data['spin3']->get_value_as_int();

			$n = intval((pow($b, $d) - 1) / ($b - 1)) * ($p + 1);

			if ($n > 100000) {
				print "$n total items? Try less\n";
				return;
			}

			$ctree_data['books'] = 1;
			$ctree_data['pages'] = 0;

			$ctree->freeze();
			$ctree->clear();

			$text = array('Root', '');

			$parent = $ctree->insert_node(null, null, $text, 5,
										  $ctree_data['pixmap1'],
										  $ctree_data['mask1'],
										  $ctree_data['pixmap2'],
										  $ctree_data['mask2'], false, true);

			$style = &new GtkStyle();
			$style->base[GTK_STATE_NORMAL] = new GdkColor(0, 45000, 55000);
			$ctree->node_set_row_data($parent, $style);

			if ($ctree->line_style == GTK_CTREE_LINES_TABBED)
				$ctree->node_set_row_style($parent, $style);

			build_recursive($ctree, 1, $d, $b, $p, $parent);
			$ctree->thaw();
			after_press($ctree);
		}

		function build_recursive($ctree, $cur_depth, $depth, $num_books,
								 $num_pages, $parent)
		{
			global	$ctree_data;

			$sibling = null;
			for ($i = $num_pages + $num_books; $i > $num_books; $i--) {
				$ctree_data['pages']++;
				$text[0] = sprintf('Page %02d', rand() % 100);
				$text[1] = sprintf('Item %d-%d', $cur_depth, $i);
				$sibling = $ctree->insert_node($parent, $sibling, $text, 5,
											   $ctree_data['pixmap3'],
											   $ctree_data['mask3'], null, null,
											   true, false);
				if ($parent && $ctree->line_style == GTK_CTREE_LINES_TABBED)
					$ctree->node_set_row_style($sibling, $parent->row->style);
			}

			if ($cur_depth == $depth)
				return;

			for ($i = $num_books; $i > 0; $i--) {
				$ctree_data['books']++;

				$text[0] = sprintf('Book %02d', rand() % 100);
				$text[1] = sprintf('Item %d-%d', $cur_depth, $i);
				$sibling = $ctree->insert_node($parent, $sibling, $text, 5,
											   $ctree_data['pixmap1'],
											   $ctree_data['mask1'],
											   $ctree_data['pixmap2'],
											   $ctree_data['mask2'],
											   false, false);

				$style = &new GtkStyle();
				switch ($cur_depth % 3) {
					case 0:
						$color = &new GdkColor(10000 * ($cur_depth % 6),
											   0,
											   65535 - (($i * 10000) % 65535));
						$style->base[GTK_STATE_NORMAL] = $color;
						break;

					case 1:
						$color = &new GdkColor(10000 * ($cur_depth % 6),
											   65535 - (($i * 10000) % 65535),
											   0);
						$style->base[GTK_STATE_NORMAL] = $color;
						break;

					default:
						$color = &new GdkColor(65535 - (($i * 10000) % 65535),
											   0,
											   10000 * ($cur_depth % 6));
						$style->base[GTK_STATE_NORMAL] = $color;
						break;
				}

				$ctree->node_set_row_data($sibling, $style);
				if ($ctree->line_style == GTK_CTREE_LINES_TABBED)
					$ctree->node_set_row_style($sibling, $style);
				
				build_recursive($ctree, $cur_depth + 1, $depth, $num_books,
								$num_pages, $sibling);
			}
		}

		function after_press($ctree)
		{
			global	$ctree_data;

			$ctree_data['sel_label']->set_text(count($ctree->selection));
			$ctree_data['vis_label']->set_text($ctree->clist->rows);
			$ctree_data['book_label']->set_text($ctree_data['books']);
			$ctree_data['page_label']->set_text($ctree_data['pages']);
		}

		function ctree_click_column($ctree, $column)
		{
			if ($column == $ctree->sort_column) {
				if ($ctree->sort_type == GTK_SORT_ASCENDING)
					$ctree->set_sort_type(GTK_SORT_DESCENDING);
				else
					$ctree->set_sort_type(GTK_SORT_ASCENDING);
			} else
				$ctree->set_sort_column($column);

			$ctree->sort_recursive();
		}

		function change_row_height($adj, $ctree)
		{
			$ctree->set_row_height($adj->value);
		}

		function change_indent($adj, $ctree)
		{
			$ctree->set_indent($adj->value);
		}

		function change_spacing($adj, $ctree)
		{
			$ctree->set_spacing($adj->value);
		}

		function expand_all($button, $ctree)
		{
			$ctree->expand_recursive();
			after_press($ctree);
		}

		function collapse_all($button, $ctree)
		{
			$ctree->collapse_recursive();
			after_press($ctree);
		}

		function change_style($button, $ctree)
		{
			static $style1, $style2;

			if ($ctree->focus_row >= 0)
				$node = $ctree->row_list[$ctree->focus_row];
			else
				$node = $ctree->row_list[0];

			if (!is_object($node))
				return;

			if (!isset($style1)) {
				$col1 = &new GdkColor(0, 56000, 0);
				$col2 = &new GdkColor(32000, 0, 56000);

				$style1 = &new GtkStyle();
				$style1->base[GTK_STATE_NORMAL] = $col1;
				$style1->fg[GTK_STATE_SELECTED] = $col2;

				$style2 = &new GtkStyle();
				$style2->base[GTK_STATE_SELECTED] = $col2;
				$style2->fg[GTK_STATE_NORMAL] = $col1;
				$style2->base[GTK_STATE_NORMAL] = $col2;
				$style2->font = gdk::font_load("-*-courier-medium-*-*-*-*-300-*-*-*-*-*-*");
			}
			
			$ctree->node_set_cell_style($node, 1, $style1);
			$ctree->node_set_cell_style($node, 0, $style2);

			if ($node->children)
				$ctree->node_set_row_style($node->children[0], $style2);
		}

		function select_all($button, $ctree)
		{
			$ctree->select_recursive();
			after_press($ctree);
		}

		function unselect_all($button, $ctree)
		{
			$ctree->unselect_recursive();
			after_press($ctree);
		}

		function count_items($ctree, $node)
		{
			global	$ctree_data;

			if ($node->is_leaf)
				$ctree_data['pages']--;
			else
				$ctree_data['books']--;
		}

		function remove_selection($button, $ctree)
		{
			global	$ctree_data;

			$ctree->freeze();

			while (($node = $ctree->selection[0]) !== null) {
				if ($node->is_leaf)
					$ctree_data['pages']--;
				else
					$ctree->post_recursive($node, 'count_items');

				$ctree->remove_node($node);

				if ($ctree->selection_mode == GTK_SELECTION_BROWSE)
					break;
			}

			if ($ctree->selection_mode == GTK_SELECTION_EXTENDED &&
				$ctree->selection[0] === null && $ctree->focus_row >= 0) {
				$node = $ctree->node_nth($ctree->focus_row);
				if ($node)
					$ctree->select($node);
			}

			$ctree->thaw();
			after_press($ctree);
		}

		function set_background($ctree, $node)
		{
			if (!$node)
				return;

			if ($ctree->line_style != GTK_CTREE_LINES_TABBED) {
				if (!$node->is_leaf)
					$style = $ctree->node_get_row_data($node);
				else
					$style = $ctree->node_get_row_data($node->parent);
			}

			$ctree->node_set_row_style($node, $style);
		}

		function ctree_toggle_line_style($menu_item, $ctree, $line_style)
		{
			if (($ctree->line_style == GTK_CTREE_LINES_TABBED &&
				 $line_style != GTK_CTREE_LINES_TABBED) ||
				($ctree->line_style != GTK_CTREE_LINES_TABBED &&
				 $line_style == GTK_CTREE_LINES_TABBED)) {
				$ctree->pre_recursive(null, 'set_background');
			}

			$ctree->set_line_style($line_style);
		}

		function ctree_toggle_expander_style($menu_item, $ctree, $expander_style)
		{
			$ctree->set_expander_style($expander_style);
		}

		function ctree_toggle_justify($menu_item, $ctree, $justification)
		{
			$ctree->set_column_justification($ctree->tree_column, $justification);
		}

		function ctree_toggle_sel_mode($menu_item, $ctree, $sel_mode)
		{
			$ctree->set_selection_mode($sel_mode);
			after_press($ctree);
		}

		$window = &new GtkWindow;
		$windows['ctree'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('GtkCTree');
		$window->set_border_width(0);

		$tooltips = &new GtkTooltips();

		$vbox = &new GtkVBox();
		$window->add($vbox);
		$vbox->show();

		$hbox = &new GtkHBox(false, 5);
		$hbox->set_border_width(5);
		$vbox->pack_start($hbox, false);
		$hbox->show();

		$label = &new GtkLabel('Depth :');
		$hbox->pack_start($label, false);
		$label->show();

		$adj = &new GtkAdjustment(4, 1, 10, 1, 5, 0);
		$ctree_data['spin1'] = &new GtkSpinButton($adj, 0, 0);
		$hbox->pack_start($ctree_data['spin1'], false, true, 5);
		$ctree_data['spin1']->show();

		$label = &new GtkLabel('Books :');
		$hbox->pack_start($label, false);
		$label->show();

		$adj = &new GtkAdjustment(3, 1, 20, 1, 5, 0);
		$ctree_data['spin2'] = &new GtkSpinButton($adj, 0, 0);
		$hbox->pack_start($ctree_data['spin2'], false, true, 5);
		$ctree_data['spin2']->show();

		$label = &new GtkLabel('Pages :');
		$hbox->pack_start($label, false);
		$label->show();

		$adj = &new GtkAdjustment(5, 1, 20, 1, 5, 0);
		$ctree_data['spin3'] = &new GtkSpinButton($adj, 0, 0);
		$hbox->pack_start($ctree_data['spin3'], false, true, 5);
		$ctree_data['spin3']->show();
		
		$button = &new GtkButton('Close');
		$button->connect('clicked', 'close_window');
		$hbox->pack_end($button);
		$button->show();

		$button = &new GtkButton('Rebuild Tree');
		$hbox->pack_start($button);
		$button->show();

		$scrolled_win = &new GtkScrolledWindow();
		$scrolled_win->set_border_width(5);
		$scrolled_win->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
		$vbox->pack_start($scrolled_win);
		$scrolled_win->show();

		$ctree = &new GtkCTree(2, 0, array('Tree', 'Info'));
		$scrolled_win->add($ctree);
		$ctree->show();

		$ctree->set_column_auto_resize(0, true);
		$ctree->set_column_width(1, 200);
		$ctree->set_selection_mode(GTK_SELECTION_EXTENDED);
		$ctree->set_line_style(GTK_CTREE_LINES_DOTTED);
		$line_style = GTK_CTREE_LINES_DOTTED;

		$button->connect('clicked', 'rebuild_tree', $ctree);
		$ctree->connect('click_column', 'ctree_click_column');

		$ctree->connect_after('button_press_event', 'after_press');
		$ctree->connect_after('button_release_event', 'after_press');
		$ctree->connect_after('tree_move', 'after_press');
		$ctree->connect_after('end_selection', 'after_press');
		$ctree->connect_after('toggle_focus_row', 'after_press');
		$ctree->connect_after('select_all', 'after_press');
		$ctree->connect_after('unselect_all', 'after_press');
		$ctree->connect_after('scroll_vertical', 'after_press');

		$bbox = &new GtkHBox(false, 5);
		$bbox->set_border_width(5);
		$vbox->pack_start($bbox, false);
		$bbox->show();

		$mbox = &new GtkVBox(true, 5);
		$bbox->pack_start($mbox, false);
		$mbox->show();

		$label = &new GtkLabel('Row Height :');
		$mbox->pack_start($label, false, false);
		$label->show();

		$label = &new GtkLabel('Indent :');
		$mbox->pack_start($label, false, false);
		$label->show();

		$label = &new GtkLabel('Spacing :');
		$mbox->pack_start($label, false, false);
		$label->show();

		$mbox = &new GtkVBox(true, 5);
		$bbox->pack_start($mbox, false);
		$mbox->show();

		$adj = &new GtkAdjustment(20, 12, 100, 1, 10, 0);
		$spinner = &new GtkSpinButton($adj, 0, 0);
		$tooltips->set_tip($spinner, 'Row height of list items.', '');
		$adj->connect('value_changed', 'change_row_height', $ctree);
		$ctree->set_row_height($adj->value);
		$mbox->pack_start($spinner, false, false, 5);
		$spinner->show();

		$adj = &new GtkAdjustment(20, 0, 60, 1, 10, 0);
		$spinner = &new GtkSpinButton($adj, 0, 0);
		$tooltips->set_tip($spinner, 'Tree indentation.', '');
		$adj->connect('value_changed', 'change_indent', $ctree);
		$mbox->pack_start($spinner, false, false, 5);
		$spinner->show();

		$adj = &new GtkAdjustment(5, 0, 60, 1, 10, 0);
		$spinner = &new GtkSpinButton($adj, 0, 0);
		$tooltips->set_tip($spinner, 'Tree spacing.', '');
		$adj->connect('value_changed', 'change_spacing', $ctree);
		$mbox->pack_start($spinner, false, false, 5);
		$spinner->show();

		$mbox = &new GtkVBox(true, 5);
		$bbox->pack_start($mbox, false);
		$mbox->show();

		$hbox = &new GtkHBox(false, 5);
		$mbox->pack_start($hbox, false, false);
		$hbox->show();

		$button = &new GtkButton('Expand All');
		$button->connect('clicked', 'expand_all', $ctree);
		$hbox->pack_start($button);
		$button->show();

		$button = &new GtkButton('Collapse All');
		$button->connect('clicked', 'collapse_all', $ctree);
		$hbox->pack_start($button);
		$button->show();

		$button = &new GtkButton('Change Style');
		$button->connect('clicked', 'change_style', $ctree);
		$hbox->pack_start($button);
		$button->show();

		$button = &new GtkButton('Export Tree');
		$button->connect('clicked', 'export_ctree', $ctree);
		$hbox->pack_start($button);
		$button->show();

		$hbox = &new GtkHBox(false, 5);
		$mbox->pack_start($hbox, false, false);
		$hbox->show();

		$button = &new GtkButton('Select All');
		$button->connect('clicked', 'select_all', $ctree);
		$hbox->pack_start($button);
		$button->show();

		$button = &new GtkButton('Unselect All');
		$button->connect('clicked', 'unselect_all', $ctree);
		$hbox->pack_start($button);
		$button->show();

		$button = &new GtkButton('Remove Selection');
		$button->connect('clicked', 'remove_selection', $ctree);
		$hbox->pack_start($button);
		$button->show();

		$check = &new GtkCheckButton('Reorderable');
		$tooltips->set_tip($check, 'Tree items can be reordered by dragging.', '');
		$check->connect('clicked', 'toggle_reorderable', $ctree);
		$GLOBALS['clist'] = $ctree; // untill callback args work..
		$check->set_active(true);
		$hbox->pack_start($check, false);
		$check->show();

		$hbox = &new GtkHBox(false, 5);
		$mbox->pack_start($hbox, false, false);
		$hbox->show();

		$items1 = array('No lines'	=> array('ctree_toggle_line_style', $ctree, GTK_CTREE_LINES_NONE),
						'Solid'		=> array('ctree_toggle_line_style', $ctree, GTK_CTREE_LINES_SOLID),
						'Dotted'	=> array('ctree_toggle_line_style', $ctree, GTK_CTREE_LINES_DOTTED),
						'Tabbed'	=> array('ctree_toggle_line_style', $ctree, GTK_CTREE_LINES_TABBED));
		$omenu1 = build_option_menu($items1, 2);
		$tooltips->set_tip($omenu1, "The tree's line style.", '');
		$hbox->pack_start($omenu1, false);
		$omenu1->show();

		$items2 = array('None'		=> array('ctree_toggle_expander_style', $ctree, GTK_CTREE_EXPANDER_NONE),
						'Square'	=> array('ctree_toggle_expander_style', $ctree, GTK_CTREE_EXPANDER_SQUARE),
						'Triangle'	=> array('ctree_toggle_expander_style', $ctree, GTK_CTREE_EXPANDER_TRIANGLE),
						'Circular'	=> array('ctree_toggle_expander_style', $ctree, GTK_CTREE_EXPANDER_CIRCULAR));
		$omenu2 = build_option_menu($items2, 2);
		$tooltips->set_tip($omenu1, "The tree's expander style.", '');
		$hbox->pack_start($omenu2, false);
		$omenu2->show();

		$items3 = array('Left'	=> array('ctree_toggle_justify', $ctree, GTK_JUSTIFY_LEFT),
						'Right'	=> array('ctree_toggle_justify', $ctree, GTK_JUSTIFY_RIGHT));
		$omenu3 = build_option_menu($items3, 0);
		$tooltips->set_tip($omenu1, "The tree's justification.", '');
		$hbox->pack_start($omenu3, false);
		$omenu3->show();

		$items4 = array('Single'	=> array('ctree_toggle_sel_mode', $ctree, GTK_SELECTION_SINGLE),
						'Browse'	=> array('ctree_toggle_sel_mode', $ctree, GTK_SELECTION_BROWSE),
						'Multiple'	=> array('ctree_toggle_sel_mode', $ctree, GTK_SELECTION_MULTIPLE),
						'Extended'	=> array('ctree_toggle_sel_mode', $ctree, GTK_SELECTION_EXTENDED));
		$omenu4 = build_option_menu($items4, 1);
		$tooltips->set_tip($omenu1, "The list's selection mode.", '');
		$hbox->pack_start($omenu4, false);
		$omenu4->show();

		$window->realize();

		$mini_page_xpm = array("16 16 4 1",
							   "       c None s None",
							   ".      c black",
							   "X      c white",
							   "o      c #808080",
							   "                ",
							   "   .......      ",
							   "   .XXXXX..     ",
							   "   .XoooX.X.    ",
							   "   .XXXXX....   ",
							   "   .XooooXoo.o  ",
							   "   .XXXXXXXX.o  ",
							   "   .XooooooX.o  ",
							   "   .XXXXXXXX.o  ",
							   "   .XooooooX.o  ",
							   "   .XXXXXXXX.o  ",
							   "   .XooooooX.o  ",
							   "   .XXXXXXXX.o  ",
							   "   ..........o  ",
							   "    oooooooooo  ",
							   "                ");
		$transparent = &new GdkColor(0, 0, 0);

		list($ctree_data['pixmap1'], $ctree_data['mask1']) = Gdk::pixmap_create_from_xpm_d($window->window, $transparent, $book_closed_xpm);
		list($ctree_data['pixmap2'], $ctree_data['mask2']) = Gdk::pixmap_create_from_xpm_d($window->window, $transparent, $book_open_xpm);
		list($ctree_data['pixmap3'], $ctree_data['mask3']) = Gdk::pixmap_create_from_xpm_d($window->window, $transparent, $mini_page_xpm);

		$ctree->set_usize(0, 300);

		$frame = &new GtkFrame();
		$frame->set_border_width(0);
		$frame->set_shadow_type(GTK_SHADOW_OUT);
		$vbox->pack_start($frame, false);
		$frame->show();

		$hbox = &new GtkHBox(true, 2);
		$hbox->set_border_width(2);
		$frame->add($hbox);
		$hbox->show();

		$frame = &new GtkFrame();
		$frame->set_shadow_type(GTK_SHADOW_IN);
		$hbox->pack_start($frame, false);
		$frame->show();

		$hbox2 = &new GtkHBox();
		$hbox2->set_border_width(2);
		$frame->add($hbox2);
		$hbox2->show();

		$label = &new GtkLabel('Books :');
		$hbox2->pack_start($label, false);
		$label->show();

		$ctree_data['book_label'] = &new GtkLabel($ctree_data['books']);
		$hbox2->pack_start($ctree_data['book_label'], false, true, 5);
		$ctree_data['book_label']->show();

		$frame = &new GtkFrame();
		$frame->set_shadow_type(GTK_SHADOW_IN);
		$hbox->pack_start($frame, false);
		$frame->show();

		$hbox2 = &new GtkHBox();
		$hbox2->set_border_width(2);
		$frame->add($hbox2);
		$hbox2->show();

		$label = &new GtkLabel('Pages :');
		$hbox2->pack_start($label, false);
		$label->show();

		$ctree_data['page_label'] = &new GtkLabel($ctree_data['pages']);
		$hbox2->pack_start($ctree_data['page_label'], false, true, 5);
		$ctree_data['page_label']->show();

		$frame = &new GtkFrame();
		$frame->set_shadow_type(GTK_SHADOW_IN);
		$hbox->pack_start($frame, false);
		$frame->show();

		$hbox2 = &new GtkHBox();
		$hbox2->set_border_width(2);
		$frame->add($hbox2);
		$hbox2->show();

		$label = &new GtkLabel('Selected :');
		$hbox2->pack_start($label, false);
		$label->show();

		$ctree_data['sel_label'] = &new GtkLabel(count($ctree->selection));
		$hbox2->pack_start($ctree_data['sel_label'], false, true, 5);
		$ctree_data['sel_label']->show();

		$frame = &new GtkFrame();
		$frame->set_shadow_type(GTK_SHADOW_IN);
		$hbox->pack_start($frame, false);
		$frame->show();

		$hbox2 = &new GtkHBox();
		$hbox2->set_border_width(2);
		$frame->add($hbox2);
		$hbox2->show();

		$label = &new GtkLabel('Visible :');
		$hbox2->pack_start($label, false);
		$label->show();

		$ctree_data['vis_label'] = &new GtkLabel($ctree->clist->rows);
		$hbox2->pack_start($ctree_data['vis_label'], false, true, 5);
		$ctree_data['vis_label']->show();
		
		rebuild_tree(null, $ctree);
	}
//	if ($windows['ctree']->flags() & GTK_VISIBLE)//
//		$windows['ctree']->hide();
//	else
		$windows['ctree']->show();
}


function create_pixmap()
{
	global	$windows;

	if (!isset($windows['pixmap'])) {
		$window = &new GtkWindow;
		$windows['pixmap'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('GtkPixmap');
		$window->set_border_width(0);
		$window->realize();

		$box1 = &new GtkVBox();
		$window->add($box1);
		$box1->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2);
		$box2->show();

		$button = &new GtkButton();
		$box2->pack_start($button, false, false);
		$button->show();

		list($pixmap, $mask) = Gdk::pixmap_create_from_xpm($window->window, null, "window.xpm");
		$pixmapwid = &new GtkPixmap($pixmap, $mask);
		$label = &new GtkLabel("Pixmap\ntest");
		$box3 = &new GtkHBox();
		$box3->set_border_width(2);
		$box3->pack_start($pixmapwid);
		$box3->pack_start($label);
		$pixmapwid->show();
		$label->show();
		$button->add($box3);
		$box3->show();

		$separator = &new GtkHSeparator();
		$box1->pack_start($separator, false);
		$separator->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2, false);
		$box2->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$box2->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
//	if ($windows['pixmap']->flags() & GTK_VISIBLE)
//		$windows['pixmap']->hide();
//	else
		$windows['pixmap']->show();
}

function create_cursor_test()
{
	global	$windows;

	if (!isset($windows['cursor_test'])) {
		function expose_event($darea, $event)
		{
			$drawable = $darea->window;
			$style = $darea->get_style();
			$white_gc = $style->white_gc;
			$grey_gc = $style->bg_gc[GTK_STATE_NORMAL];
			$black_gc = $style->black_gc;
			$max_width = $drawable->width;
			$max_height = $drawable->height;

			gdk::draw_rectangle($drawable, $white_gc, true, 0, 0, $max_width,
								(int)($max_height / 2));
			gdk::draw_rectangle($drawable, $black_gc, true, 0, (int)($max_height / 2),
								$max_width, (int)($max_height / 2));
			gdk::draw_rectangle($drawable, $grey_gc, true, (int)($max_width / 3),
								(int)($max_height / 3), (int)($max_width / 3),
								(int)($max_height / 3));
		}

		function set_cursor($spinner, $darea, $cur_name)
		{
			$c = $spinner->get_value_as_int();
			$c = $c & 0xfe;
			$cursor = gdk::cursor_new($c);
			$window = $darea->window;
			$window->set_cursor($cursor);
			$cur_name->set_text($cursor->name);
		}

		function button_press($widget, $event, $spinner)
		{
			if ($event->type == GDK_BUTTON_PRESS) {
				if ($event->button == 1)
					$spinner->spin(GTK_SPIN_STEP_FORWARD, 0);
				else if ($event->button == 3)
					$spinner->spin(GTK_SPIN_STEP_BACKWARD, 0);
			}
		}

		$window = &new GtkWindow;
		$windows['cursor_test'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('Cursor Test');
		$window->set_border_width(0);

		$main_vbox = &new GtkVBox(false, 5);
		$main_vbox->set_border_width(0);
		$window->add($main_vbox);
		$main_vbox->show();

		$vbox = &new GtkVBox(false, 5);
		$vbox->set_border_width(10);
		$main_vbox->pack_start($vbox);
		$vbox->show();

		$hbox = &new GtkHBox(false, 5);
		$vbox->pack_start($hbox, false);
		$hbox->show();

		$label = &new GtkLabel('Cursor value: ');
		$label->set_alignment(0, 0.5);
		$hbox->pack_start($label, false);
		$label->show();

		$spinner = &new GtkSpinButton(new GtkAdjustment(0, 0, 152, 2, 10, 0), 0, 0);
		$hbox->pack_start($spinner);
		$spinner->show();

		$frame = &new GtkFrame('Cursor Area');
		$frame->set_border_width(10);
		$frame->set_label_align(0.5, 0);
		$vbox->pack_start($frame);
		$frame->show();

		$darea = &new GtkDrawingArea();
		$darea->set_usize(80, 80);
		$frame->add($darea);
		$darea->show();

		$cur_name = &new GtkLabel('');
		$vbox->pack_start($cur_name);
		$cur_name->show();

		$darea->connect('expose_event', 'expose_event');
		$darea->add_events(GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);
		$darea->connect('button_press_event', 'button_press', $spinner);
		$spinner->connect('changed', 'set_cursor', $darea, $cur_name);

		$separator = &new GtkHSeparator();
		$main_vbox->pack_start($separator, false);
		$separator->show();

		$vbox = &new GtkVBox(false, 5);
		$vbox->set_border_width(5);
		$main_vbox->pack_start($vbox, false);
		$vbox->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$vbox->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();

		$darea->connect_object('realize', 'set_cursor', $spinner, $darea, $cur_name);
	}
//	if ($windows['cursor_test']->flags() & GTK_VISIBLE)
//		$windows['cursor_test']->hide();
//	else
		$windows['cursor_test']->show();
}


function create_color_selection()
{
	global	$windows;

	if (!isset($windows['color_selection'])) {
		$window = &new GtkColorSelectionDialog('color selection dialog');
		$windows['color_selection'] = $window;
		$colorsel = $window->colorsel;
		$colorsel->set_opacity(true);
		$colorsel->set_update_policy(GTK_UPDATE_CONTINUOUS);
		$colorsel->set_color(0.4, 0.5, 0.7, 0.75);
		$window->set_position(GTK_WIN_POS_MOUSE);
		$window->connect('delete-event', 'delete_event');

		$cancel_button = $window->cancel_button;
		$cancel_button->connect('clicked', 'close_window');
		$ok_button = $window->ok_button;
		$ok_button->connect('clicked', 'close_window');
	}
//	if ($windows['color_selection']->flags() & GTK_VISIBLE)
//		$windows['color_selection']->hide();
//	else
		$windows['color_selection']->show();
}


function create_radio_buttons()
{
	global	$windows;

	if (!isset($windows['radio_buttons'])) {
		$window = &new GtkWindow;
		$windows['radio_buttons'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('radio buttons');
		$window->set_border_width(0);

		$box1 = &new GtkVBox();
		$window->add($box1);
		$box1->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2);
		$box2->show();

		$button1 = &new GtkRadioButton(null, 'button1');
		$button1->set_active(true);
		$box2->pack_start($button1);
		$button1->show();
		for ($i = 2; $i <= 4; $i++) {
			$button = &new GtkRadioButton($button1, 'button' . $i);
			$box2->pack_start($button);
			$button->show();
		}

		$separator = &new GtkHSeparator();
		$box1->pack_start($separator, false);
		$separator->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2, false);
		$box2->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$box2->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
//	if ($windows['radio_buttons']->flags() & GTK_VISIBLE)
//		$windows['radio_buttons']->hide();
//	else
		$windows['radio_buttons']->show();
}


function create_check_buttons()
{
	global	$windows;

	if (!isset($windows['check_buttons'])) {
		$window = &new GtkWindow;
		$windows['check_buttons'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('check buttons');
		$window->set_border_width(0);

		$box1 = &new GtkVBox();
		$window->add($box1);
		$box1->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2);
		$box2->show();

		for ($i = 1; $i <= 4; $i++) {
			$button = &new GtkCheckButton('button' . $i);
			$box2->pack_start($button);
			$button->show();
		}

		$separator = &new GtkHSeparator();
		$box1->pack_start($separator, false);
		$separator->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2, false);
		$box2->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$box2->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
//	if ($windows['check_buttons']->flags() & GTK_VISIBLE)
//		$windows['check_buttons']->hide();
//	else
		$windows['check_buttons']->show();
}

function create_clist()
{
	global	$windows;

	if (!isset($windows['clist'])) {
		$window = &new GtkWindow;
		$windows['clist'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('CList');
		$window->set_border_width(0);

		$vbox = &new GtkVBox();
		$window->add($vbox);
		$vbox->show();

		$scrolled_win = &new GtkScrolledWindow();
		$scrolled_win->set_border_width(5);
		$scrolled_win->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

		$titles = array("auto resize", "not resizeable", "max width 100",
						"min width 50", "hide column", "Title 5", "Title 6",
						"Title 7", "Title 8",  "Title 9",  "Title 10", "Title 11");
		$clist = &new GtkCList(12, $titles);
		$clist->connect('click_column', 'clist_click_column');
		$scrolled_win->add($clist);
		$clist->show();

		$hbox = &new GtkHBox(false, 5);
		$hbox->set_border_width(5);
		$vbox->pack_start($hbox, false, false);
		$hbox->show();

		function clist_click_column($clist, $column)
		{
			if ($column == 4)
				$clist->set_column_visibility($column, false);
			else if ($column == $clist->sort_column) {
				if ($clist->sort_type == GTK_SORT_ASCENDING)
					$clist->set_sort_type(GTK_SORT_DESCENDING);
				else
					$clist->set_sort_type(GTK_SORT_ASCENDING);
			} else
				$clist->set_sort_column($column);

			$clist->sort();
		}

		function insert_row_clist($button, $clist)
		{
			static	$style1, $style2, $style3;

			$text = array('This', 'is an', 'inserted', 'row.',
						  'This', 'is an', 'inserted', 'row.',
						  'This', 'is an', 'inserted', 'row.');

			if ($clist->focus_row >= 0)
				$row = $clist->insert($clist->focus_row, $text);
			else
				$row = $clist->prepend($text);

			if (!isset($style1)) {
				$col1 = &new GdkColor(0, 56000, 0);
				$col2 = &new GdkColor(32000, 0, 56000);

				$style = $clist->style;
				$style1 = $style->copy();
				$style1->base[GTK_STATE_NORMAL] = $col1;
				$style1->base[GTK_STATE_SELECTED] = $col2;

				$style2 = $style->copy();
				$style2->fg[GTK_STATE_NORMAL] = $col1;
				$style2->fg[GTK_STATE_SELECTED] = $col2;

				$style3 = $style->copy();
				$style3->fg[GTK_STATE_NORMAL] = $col1;
				$style3->base[GTK_STATE_NORMAL] = $col2;
				$style3->font = gdk::font_load ("-*-courier-medium-*-*-*-*-120-*-*-*-*-*-*");
			}

			$clist->set_cell_style($row, 3, $style1);
			$clist->set_cell_style($row, 4, $style2);
			$clist->set_cell_style($row, 0, $style3);
		}

		$button = &new GtkButton('Insert Row');
		$button->connect('clicked', 'insert_row_clist', $clist);
		$hbox->pack_start($button);
		$button->show();

		function add1000_clist($button, $clist)
		{
			$gtk_mini_xpm = array(
				"15 20 17 1",
				"       c None",
				".      c #14121F",
				"+      c #278828",
				"@      c #9B3334",
				"#      c #284C72",
				"$      c #24692A",
				"%      c #69282E",
				"&      c #37C539",
				"*      c #1D2F4D",
				"=      c #6D7076",
				"-      c #7D8482",
				";      c #E24A49",
				">      c #515357",
				",      c #9B9C9B",
				"'      c #2FA232",
				")      c #3CE23D",
				"!      c #3B6CCB",
				"               ",
				"      ***>     ",
				"    >.*!!!*    ",
				"   ***....#*=  ",
				"  *!*.!!!**!!# ",
				" .!!#*!#*!!!!# ",
				" @%#!.##.*!!$& ",
				" @;%*!*.#!#')) ",
				" @;;@%!!*$&)'' ",
				" @%.%@%$'&)$+' ",
				" @;...@$'*'*)+ ",
				" @;%..@$+*.')$ ",
				" @;%%;;$+..$)# ",
				" @;%%;@$$$'.$# ",
				" %;@@;;$$+))&* ",
				"  %;;;@+$&)&*  ",
				"   %;;@'))+>   ",
				"    %;@'&#     ",
				"     >%$$      ",
				"      >=       ");

			list($pixmap, $mask) =
				gdk::pixmap_create_from_xpm_d($clist->window,
											  $clist->style->white,
											  $gtk_mini_xpm);

			for ($i = 0; $i < 12; $i++)
				$texts[$i] = "Column $i";
			$texts[1] = 'Right';
			$texts[2] = 'Center';
			$texts[3] = null;

			$clist->freeze();
			for ($i = 0; $i < 1000; $i++) {
				$texts[0] = "CListRow " . (rand() % 10000);
				$row = $clist->append($texts);
				$clist->set_pixtext($row, 3, "gtk+", 5, $pixmap, $mask);
			}
			$clist->thaw();
		}

		function add10000_clist($button, $clist)
		{
			for ($i = 0; $i < 12; $i++)
				$texts[$i] = "Column $i";
			$texts[1] = 'Right';
			$texts[2] = 'Center';

			$clist->freeze();
			for ($i = 0; $i < 10000; $i++) {
				$texts[0] = "CListRow " . (rand() % 10000);
				$row = $clist->append($texts);
			}
			$clist->thaw();
		}

		function clist_remove_selection($button, $clist)
		{
			$clist->freeze();
			$selection = $clist->selection;
			
			while (($row = $clist->selection[0]) !== null) {
				$clist->remove($row);
				if ($clist->selection_mode == GTK_SELECTION_BROWSE)
					break;
			}
			
			if ($clist->selection_mode == GTK_SELECTION_EXTENDED &&
				$clist->selection[0] === null && $clist->focus_row >= 0)
				$clist->select_row($clist->focus_row, -1);
			$clist->thaw();
		}

		function toggle_title_buttons($button, $clist)
		{
			if ($button->get_active())
				$clist->column_titles_show();
			else
				$clist->column_titles_hide();
		}

		$button = &new GtkButton('Add 1,000 Rows With Pixmaps');
		$button->connect('clicked', 'add1000_clist', $clist);
		$hbox->pack_start($button);
		$button->show();

		$button = &new GtkButton('Add 10,000 Rows');
		$button->connect('clicked', 'add10000_clist', $clist);
		$hbox->pack_start($button);
		$button->show();

		$hbox = &new GtkHBox(false, 5);
		$hbox->set_border_width(5);
		$vbox->pack_start($hbox, false, false);
		$hbox->show();

		$button = &new GtkButton('Clear List');
		$button->connect_object('clicked', array($clist, 'clear'));
		$hbox->pack_start($button);
		$button->show();

		$button = &new GtkButton('Remove Selection');
		$button->connect('clicked', 'clist_remove_selection', $clist);
		$hbox->pack_start($button);
		$button->show();

		$button = &new GtkButton('Undo Selection');
		$button->connect_object('clicked', array($clist, 'undo_selection'));
		$hbox->pack_start($button);
		$button->show();

		$hbox = &new GtkHBox(false, 5);
		$hbox->set_border_width(5);
		$vbox->pack_start($hbox, false, false);
		$hbox->show();

		$button = &new GtkCheckButton('Show Title Buttons');
		$button->set_active(true);
		$button->connect('clicked', 'toggle_title_buttons', $clist);
		$hbox->pack_start($button, false);
		$button->show();

		$button = &new GtkCheckButton('Reorderable');
		$button->connect('clicked', 'toggle_reorderable', $clist);
		$GLOBALS['clist'] = $clist; // untill callback args work..
		$button->set_active(true);
		$hbox->pack_start($button, false);
		$button->show();

		$label = &new GtkLabel('Selection Mode: ');
		$hbox->pack_start($label, false);
		$label->show();

		function clist_toggle_sel_mode($menu_item, $clist, $sel_mode)
		{
			$clist->set_selection_mode($sel_mode);
		}

  		$items = array('Single'		=> array('clist_toggle_sel_mode', $clist, GTK_SELECTION_SINGLE),
					   'Browse'		=> array('clist_toggle_sel_mode', $clist, GTK_SELECTION_BROWSE),
					   'Multiple'	=> array('clist_toggle_sel_mode', $clist, GTK_SELECTION_MULTIPLE),
					   'Extended'	=> array('clist_toggle_sel_mode', $clist, GTK_SELECTION_EXTENDED));
		$clist_omenu = build_option_menu($items, 3);
		$hbox->pack_start($clist_omenu);
		$clist_omenu->show();

		$vbox->pack_start($scrolled_win);
		$scrolled_win->show();

		$clist->set_row_height(18);
		$clist->set_usize(-1, 300);

		$clist_rows = 0;
		for ($i = 1; $i < 12; $i++)
			$clist->set_column_width($i, 80);
		
		$clist->set_column_auto_resize(0, true);
		$clist->set_column_resizeable(1, false);
		$clist->set_column_max_width(2, 100);
		$clist->set_column_min_width(3, 50);
		$clist->set_selection_mode(GTK_SELECTION_EXTENDED);
		$clist->set_column_justification(1, GTK_JUSTIFY_RIGHT);
		$clist->set_column_justification(2, GTK_JUSTIFY_CENTER);

		for ($i = 0; $i < 12; $i++)
			$texts[$i] = "Column $i";

		$texts[1] = 'Right';
		$texts[2] = 'Center';

		$col1 = &new GdkColor(56000, 0, 0);
		$col2 = &new GdkColor(0, 56000, 32000);
		$style = &new GtkStyle;
		$style->fg[GTK_STATE_NORMAL] = $col1;
		$style->base[GTK_STATE_NORMAL] = $col2;
		$style->font = gdk::font_load ("-adobe-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*");

		for ($i = 0; $i < 10; $i++) {
			$texts[0] = sprintf('CListRow %d', $clist_rows++);
			$clist->append($texts);

			switch ($i % 4) {
				case 2:
					$clist->set_row_style($i, $style);
					break;

				default:
					$clist->set_cell_style($i, $i % 4, $style);
					break;
			}
		}

		$separator = &new GtkHSeparator();
		$vbox->pack_start($separator, false);
		$separator->show();

		$hbox = &new GtkHBox();
		$vbox->pack_start($hbox, false);
		$hbox->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$hbox->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
	if ($windows['clist']->flags() & GTK_VISIBLE)
		$windows['clist']->hide();
	else
		$windows['clist']->show();
}


function create_buttons()
{
	global	$windows;

	if (!isset($windows['buttons'])) {
		$window = &new GtkWindow;
		$windows['buttons'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('GtkButton');

		$box1 = &new GtkVBox();
		$window->add($box1);
		$box1->show();

		$table = &new GtkTable(3, 3);
		$table->set_row_spacings(5);
		$table->set_col_spacings(5);
		$table->set_border_width(10);
		$box1->pack_start($table);
		$table->show();

		$buttons = range(0, 8);
		array_walk($buttons, create_function('&$x', '$x = new GtkButton("button".($x+1));'));

		function toggle_show($b)
		{
			if ($b->flags() & GTK_VISIBLE)
				$b->hide();
			else
				$b->show();
		}

		for ($i = 0; $i < 9; $i++) {
			$buttons[$i]->connect('clicked',
								 create_function('$b, $b2', 'toggle_show($b2);'),
								 $buttons[($i+1)%9]);
			$x = (int)($i / 3);
			$y = $i % 3;
			$table->attach($buttons[$i], $x, $x + 1, $y, $y + 1);
			$buttons[$i]->show();
		}

		$separator = &new GtkHSeparator();
		$box1->pack_start($separator, false);
		$separator->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2, false);
		$box2->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$box2->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
//	if ($windows['buttons']->flags() & GTK_VISIBLE)
//		$windows['buttons']->hide();
//	else
		$windows['buttons']->show();
}


function create_labels()
{
	global	$windows;

	if (!isset($windows['labels'])) {
		$window = &new GtkWindow;
		$windows['labels'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('GtkLabel');

		$box = &new GtkVBox(false, 10);
		$box->set_border_width(10);
		$window->add($box);
		$box->show();

		$vbox = &new GtkVBox(false, 5);
		$hbox = &new GtkHBox(false, 5);
		$box->pack_start($hbox, false);
		$hbox->show();
		$hbox->pack_start($vbox, false, false);
		$vbox->show();
		$window->set_border_width(5);

		$frame = &new GtkFrame('Normal Label');
		$label = &new GtkLabel('This is a Normal label');
		$frame->add($label);
		$label->show();
		$vbox->pack_start($frame, false, false);
		$frame->show();

		$frame = &new GtkFrame('Multi-line Label');
		$label = &new GtkLabel("This is a Multi-line label.\nSecond line\nThird line");
		$frame->add($label);
		$label->show();
		$vbox->pack_start($frame, false, false);
		$frame->show();

		$frame = &new GtkFrame('Left Justified Label');
		$label = &new GtkLabel("This is a Left-Justified\nMulti-line label.\nThird line");
		$label->set_justify(GTK_JUSTIFY_LEFT);
		$frame->add($label);
		$label->show();
		$vbox->pack_start($frame, false, false);
		$frame->show();

		$frame = &new GtkFrame('Right Justified Label');
		$label = &new GtkLabel("This is a Right-Justified\nMulti-line label.\nThird line");
		$label->set_justify(GTK_JUSTIFY_RIGHT);
		$frame->add($label);
		$label->show();
		$vbox->pack_start($frame, false, false);
		$frame->show();

		$vbox = &new GtkVBox(false, 5);
		$hbox->pack_start($vbox);
		$vbox->show();

		$frame = &new GtkFrame('Line wrapped label');
		$label = &new GtkLabel("This is an example of a line-wrapped label.  It should not be taking ".
							   "up the entire             ".
							   "width allocated to it, but automatically wraps the words to fit.  ".
							   "The time has come, for all good men, to come to the aid of their party.  ".
							   "The sixth sheik's six sheep's sick.\n".
							   "     It supports multiple paragraphs correctly, and  correctly   adds ".
							   "many          extra  spaces. ");
		$label->set_line_wrap(true);
		$frame->add($label);
		$label->show();
		$vbox->pack_start($frame, false, false);
		$frame->show();

		$frame = &new GtkFrame('Filled, wrapped label');
		$label = &new GtkLabel("This is an example of a line-wrapped, filled label.  It should be taking ".
			     "up the entire              width allocated to it.  Here is a seneance to prove ".
			     "my point.  Here is another sentence. ".
			     "Here comes the sun, do de do de do.\n".
			     "    This is a new paragraph.\n".
			     "    This is another newer, longer, better paragraph.  It is coming to an end, ".
			     "unfortunately.");
		$label->set_line_wrap(true);
		$label->set_justify(GTK_JUSTIFY_FILL);
		$frame->add($label);
		$label->show();
		$vbox->pack_start($frame, false, false);
		$frame->show();

		$frame = &new GtkFrame('Underlined label');
		$label = &new GtkLabel("This label is underlined!\n".
							   "This one is underlined in ÆüËÜžì€ÎÆþÍÑquite a funky fashion");
		$label->set_justify(GTK_JUSTIFY_LEFT);
		$label->set_pattern('_________________________ _ _________ _ _____ _ __ __  ___ ____ _____');
		$frame->add($label);
		$label->show();
		$vbox->pack_start($frame, false, false);
		$frame->show();

		$separator = &new GtkHSeparator();
		$box->pack_start($separator, false);
		$separator->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$box->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
	if ($windows['labels']->flags() & GTK_VISIBLE)
		$windows['labels']->hide();
	else
		$windows['labels']->show();
}


function create_button_box()
{
	global	$windows;

	if (!isset($windows['button_box'])) {
		$window = &new GtkWindow;
		$windows['button_box'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('Button Boxes');
		$window->set_border_width(10);

		$main_vbox = &new GtkVBox(false, 10);
		$window->add($main_vbox);
		$main_vbox->show();

		function create_bbox($horizontal, $title, $spacing,
							 $child_w, $child_h, $layout)
		{
			$frame = &new GtkFrame($title);
			if ($horizontal)
				$bbox = &new GtkHButtonBox();
			else
				$bbox = &new GtkVButtonBox();
			$bbox->set_border_width(5);
			$bbox->set_layout($layout);
			$bbox->set_spacing($spacing);
			$bbox->set_child_size($child_w, $child_h);
			$frame->add($bbox);
			$bbox->show();

			$button = &new GtkButton('OK');
			$bbox->add($button);
			$button->show();

			$button = &new GtkButton('Cancel');
			$bbox->add($button);
			$button->show();

			$button = &new GtkButton('Help');
			$bbox->add($button);
			$button->show();

			$frame->show();

			return $frame;
		}

		$frame_horz = &new GtkFrame('Horizontal Button Boxes');
		$main_vbox->add($frame_horz);
		$frame_horz->show();

		$vbox = &new GtkVBox();
		$vbox->set_border_width(10);
		$frame_horz->add($vbox);
		$vbox->show();

		$vbox->pack_start(create_bbox(true, 'Spread', 40, 85, 20, GTK_BUTTONBOX_SPREAD), true, true, 10);
		$vbox->pack_start(create_bbox(true, 'Edge', 40, 85, 20, GTK_BUTTONBOX_EDGE), true, true, 10);
		$vbox->pack_start(create_bbox(true, 'Start', 40, 85, 20, GTK_BUTTONBOX_START), true, true, 10);
		$vbox->pack_start(create_bbox(true, 'End', 40, 85, 20, GTK_BUTTONBOX_END), true, true, 10);

		$frame_vert = &new GtkFrame('Vertical Button Boxes');
		$main_vbox->add($frame_vert);
		$frame_vert->show();

		$hbox = &new GtkHBox();
		$hbox->set_border_width(10);
		$frame_vert->add($hbox);
		$hbox->show();

		$hbox->pack_start(create_bbox(false, 'Spread', 30, 85, 20, GTK_BUTTONBOX_SPREAD), true, true, 5);
		$hbox->pack_start(create_bbox(false, 'Edge', 30, 85, 20, GTK_BUTTONBOX_EDGE), true, true, 5);
		$hbox->pack_start(create_bbox(false, 'Start', 30, 85, 20, GTK_BUTTONBOX_START), true, true, 5);
		$hbox->pack_start(create_bbox(false, 'End', 30, 85, 20, GTK_BUTTONBOX_END), true, true, 5);

		$separator = &new GtkHSeparator();
		$main_vbox->pack_start($separator, false);
		$separator->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$main_vbox->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}

	if ($windows['button_box']->flags() & GTK_VISIBLE)
		$windows['button_box']->hide();
	else
		$windows['button_box']->show();
}


function create_tooltips()
{
	global	$windows;

	if (!isset($windows['tooltips'])) {
		$window = &new GtkWindow;
		$windows['tooltips'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('Tooltips');
		$window->set_border_width(0);
		$window->set_policy(true, false, true);
		$window->set_usize(200, -2);

		$tooltips = &new GtkTooltips();

		$box1 = &new GtkVBox();
		$window->add($box1);
		$box1->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2);
		$box2->show();

		$button = &new GtkToggleButton('button1');
		$box2->pack_start($button);
		$button->show();
		$tooltips->set_tip($button, 'This is button 1', 'ContextHelp/buttons/1');

		$button = &new GtkToggleButton('button2');
		$box2->pack_start($button);
		$button->show();
		$tooltips->set_tip($button, "This is button 2.  This is also a really long tooltip which probably won't fit on a single line and will therefore need to be wrapped.  Hopefully the wrapping will work correctly.", 'ContextHelp/buttons/2_long');

		$toggle = &new GtkToggleButton('Override TipsQuery Label');
		$box2->pack_start($toggle);
		$toggle->show();
		$tooltips->set_tip($toggle, 'Toggle TipsQuery view.', 'Hi all!');

		$box3 = &new GtkVBox(false, 5);
		$box3->set_border_width(5);
		$box3->show();

		$tips_query = &new GtkTipsQuery();
		$tips_query->show();

		$button = &new GtkButton('[?]');
		$box3->pack_start($button, false, false, 0);
		$button->show();
		$button->connect_object('clicked', array($tips_query, 'start_query'));
		$tooltips->set_tip($button, 'Start the Tooltips Inspector', 'ContextHelp/buttons/?');

		function tips_query_widget_entered($tips_query, $widget, $tip_text,
										   $tip_private, $toggle)
		{
			if ($toggle->get_active()) {
				$tips_query->set_text($tip_text ? 'There is a Tip!' : 'There is no Tip!');
				/* don't let GtkTipsQuery reset its label */
				$tips_query->emit_stop_by_name('widget_entered');
			}
		}


		function tips_query_widget_selected($tips_query, $widget, $tip_text,
											$tip_private, $event)
		{
			if ($widget) {
				print 'Help "';
					print $tip_private ? $tip_private : 'None';
				print '" requested for <';
				print Gtk::type_name($widget->get_type());
				print ">\n";
			}

			return true;
		}

		$box3->pack_start($tips_query);
		$tips_query->set_caller($button);
		$tips_query->connect('widget_entered', 'tips_query_widget_entered', $toggle);
		$tips_query->connect('widget_selected', 'tips_query_widget_selected');

		$frame = &new GtkFrame('ToolTips Inspector');
		$frame->set_label_align(0.5, 0);
		$frame->set_border_width(0);
		$box2->pack_start($frame, true, true, 10);
		$frame->add($box3);
		$frame->show();
		$box2->set_child_packing($frame, true, true, 10, GTK_PACK_START);

		$separator = &new GtkHSeparator();
		$box1->pack_start($separator, false);
		$separator->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2, false);
		$box2->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$box2->pack_start($button);
		//$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();

		$tooltips->set_tip($button, 'Push this button to close window', 'push');
		$tooltips->enable();
	}
	if ($windows['tooltips']->flags() & GTK_VISIBLE)
		$windows['tooltips']->hide();
	else
		$windows['tooltips']->show();
}


function create_toggle_buttons()
{
	global	$windows;

	if (!isset($windows['toggle_buttons'])) {
		$window = &new GtkWindow;
		$windows['toggle_buttons'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('Toggle buttons');
		$window->set_border_width(0);

		$box1 = &new GtkVBox();
		$window->add($box1);
		$box1->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2);
		$box2->show();

		for ($i = 1; $i <= 4; $i++) {
			$button = &new GtkToggleButton('button' . $i);
			$box2->pack_start($button);
			$button->show();
		}

		$separator = &new GtkHSeparator();
		$box1->pack_start($separator, false);
		$separator->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2, false);
		$box2->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$box2->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
	if ($windows['toggle_buttons']->flags() & GTK_VISIBLE)
		$windows['toggle_buttons']->hide();
	else
		$windows['toggle_buttons']->show();
}


function create_entry()
{
	global	$windows;

	if (!isset($windows['entry'])) {
		$window = &new GtkWindow;
		$windows['entry'] = $window;
		$window->connect('delete-event', 'delete_event');
		$window->set_title('entry');
		$window->set_border_width(0);

		$box1 = &new GtkVBox();
		$window->add($box1);
		$box1->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2);
		$box2->show();

		$entry = &new GtkEntry();
		$entry->set_text('Hello World');
		$entry->select_region(0, 5);
		$box2->pack_start($entry);
		$entry->show();

		$strings[] = "item0";
		$strings[] = "item0 item1";
		$strings[] = "item0 item1 item2";
		$strings[] = "item0 item1 item2 item3";
		$strings[] = "item0 item1 item2 item3 item4";
		$strings[] = "item0 item1 item2 item3 item4 item5";
		$strings[] = "item0 item1 item2 item3 item4";
		$strings[] = "item0 item1 item2 item3";
		$strings[] = "item0 item1 item2";
		$strings[] = "item0 item1";
		$strings[] = "item0";

		$cb = &new GtkCombo();
		$cb->set_popdown_strings($strings);
		$cb_entry = $cb->entry;
		$cb_entry->set_text('Hello World');
		$cb_entry->select_region(0, -1);
		$box2->pack_start($cb);
		$cb->show();

		function entry_toggle_editable($check_button, $entry)
		{
			$entry->set_editable($check_button->get_active());
		}

		function entry_toggle_sensitive($check_button, $entry)
		{
			$entry->set_sensitive($check_button->get_active());
		}

		function entry_toggle_visibility($check_button, $entry)
		{
			$entry->set_visibility($check_button->get_active());
		}

		$editable_check = &new GtkCheckButton('Editable');
		$editable_check->connect('toggled', 'entry_toggle_editable', $entry);
		$editable_check->set_active(true);
		$box2->pack_start($editable_check, false);
		$editable_check->show();

		$visibility_check = &new GtkCheckButton('Visible');
		$visibility_check->connect('toggled', 'entry_toggle_visibility', $entry);
		$visibility_check->set_active(true);
		$box2->pack_start($visibility_check, false);
		$visibility_check->show();

		$sensitive_check = &new GtkCheckButton('Sensitive');
		$sensitive_check->connect('toggled', 'entry_toggle_sensitive', $entry);
		$sensitive_check->set_active(true);
		$box2->pack_start($sensitive_check, false);
		$sensitive_check->show();

		$separator = &new GtkHSeparator();
		$box1->pack_start($separator, false);
		$separator->show();

		$box2 = &new GtkVBox(false, 10);
		$box2->set_border_width(10);
		$box1->pack_start($box2, false);
		$box2->show();

		$button = &new GtkButton('close');
		$button->connect('clicked', 'close_window');
		$box2->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
	if ($windows['entry']->flags() & GTK_VISIBLE)
		$windows['entry']->hide();
	else
		$windows['entry']->show();
}

function toggle_resize($child)
{
	$paned = $child->parent;

	$is_child1 = ($child == $paned->child1);

	$resize = $is_child1 ? $paned->child1_resize : $paned->child2_resize;
	$shrink = $is_child1 ? $paned->child1_shrink : $paned->child2_shrink;

	$child->ref();

	// Zend doesn't yet support overloaded method calls
	$parent = $child->parent;
	$parent->remove($child);

	if ($is_child1)
		$paned->pack1($child, !$resize, $shrink);
	else
		$paned->pack2($child, !$resize, $shrink);

	$child->unref();
}

function toggle_shrink($child)
{
	$paned = $child->parent;
	
	$is_child1 = ($child == $paned->child1);

	$resize = $is_child1 ? $paned->child1_resize : $paned->child2_resize;
	$shrink = $is_child1 ? $paned->child1_shrink : $paned->child2_shrink;

	$child->ref();

	// Zend doesn't yet support overloaded method calls
	$parent = $child->parent;
	$parent->remove($child);

	if ($is_child1)
		$paned->pack1($child, $resize, !$shrink);
	else
		$paned->pack2($child, $resize, !$shrink);

	$child->unref();
}

function create_pane_options($paned, $frame_label, $label1, $label2)
{
	$frame = &new GtkFrame($frame_label);
	$frame->show();
	$frame->set_border_width(0);

	$table = &new GtkTable(3, 2, true);
	$table->show();
	$frame->add($table);

	$label = &new GtkLabel($label1);
	$label->show();
	$table->attach_defaults($label, 0, 1, 0, 1);

	$check_button = &new GtkCheckButton('Resize');
	$check_button->show();
	$table->attach_defaults($check_button, 0, 1, 1, 2);
	$check_button->connect_object('toggled', 'toggle_resize', $paned->child1);

	$check_button = &new GtkCheckButton('Shrink');
	$check_button->show();
	$table->attach_defaults($check_button, 0, 1, 2, 3);
	$check_button->set_active(true);
	$check_button->connect_object('toggled', 'toggle_shrink', $paned->child1);

	$label = &new GtkLabel($label2);
	$label->show();
	$table->attach_defaults($label, 1, 2, 0, 1);

	$check_button = &new GtkCheckButton('Resize');
	$check_button->show();
	$table->attach_defaults($check_button, 1, 2, 1, 2);
	$check_button->set_active(true);
	$check_button->connect_object('toggled', 'toggle_resize', $paned->child2);

	$check_button = &new GtkCheckButton('Shrink');
	$check_button->show();
	$table->attach_defaults($check_button, 1, 2, 2, 3);
	$check_button->set_active(true);
	$check_button->connect_object('toggled', 'toggle_shrink', $paned->child2);

	return $frame;
}

function create_panes()
{
	global $windows;

	if (!isset($windows['panes'])) {
		$window = &new GtkWindow;
		$windows['panes'] = $window;
		$window->connect('delete_event', 'delete_event');
		$window->set_title('GtkPane');

		$vbox = &new GtkVBox;
		$vbox->show();
		$window->add($vbox);

		$vpaned = &new GtkVPaned;
		$vpaned->show();
		$vbox->pack_start($vpaned, true, true, 0);
		$vpaned->set_border_width(5);

		$hpaned = &new GtkHPaned;
		$hpaned->show();
		$vpaned->add1($hpaned);

		$frame = &new GtkFrame;
		$frame->show();
		$frame->set_shadow_type(GTK_SHADOW_IN);
		$frame->set_usize(60, 60);
		$hpaned->add1($frame);

		$button = &new GtkButton('Hi there');
		$button->show();
		$frame->add($button);

		$frame = &new GtkFrame;
		$frame->show();
		$frame->set_shadow_type(GTK_SHADOW_IN);
		$frame->set_usize(80, 60);
		$hpaned->add2($frame);

		$frame = &new GtkFrame;
		$frame->show();
		$frame->set_shadow_type(GTK_SHADOW_IN);
		$frame->set_usize(60, 80);
		$vpaned->add2($frame);

		$vbox->pack_start(
			create_pane_options(
				$hpaned,
				'Horizontal',
				'Left',
				'Right'),
			false, false, 0);

		$vbox->pack_start(
			create_pane_options(
				$vpaned,
				'Vertical',
				'Top',
				'Bottom'),
			false, false, 0);

		$separator = &new GtkHSeparator();
		$separator->show();
		$vbox->pack_start($separator, false);

		$button = &new GtkButton('Close');
		$button->show();
		$button->connect('clicked', 'close_window');
		$vbox->pack_start($button);
		////$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
	}
	if ($windows['panes']->flags() & GTK_VISIBLE)
		$windows['panes']->hide();
	else
		$windows['panes']->show();
}

function file_selection_ok($button, $fs)
{
	print "selected '" . $fs->get_filename() . "'\n";
	$fs->hide();
}

function create_file_selection()
{
	global $windows;

	if (!isset($windows['file_selection'])) {
		$window = &new GtkFileSelection('File selection dialog');
		$windows['file_selection'] = $window;
		$window->hide_fileop_buttons();
		$window->set_position(GTK_WIN_POS_MOUSE);
		$window->connect('delete_event', 'delete_event');

		$button_ok = $window->ok_button;
		$button_ok->connect('clicked', 'file_selection_ok', $window);

		$button_cancel = $window->cancel_button;
		$button_cancel->connect('clicked', 'close_window');

		$action_area = $window->action_area;

		$button = &new GtkButton('Hide Fileops');
		$button->show();
		$button->connect_object('clicked', create_function('$w', '$w->hide_fileop_buttons();'), $window);
		$action_area->pack_start($button, false, false, 0);

		$button = &new GtkButton('Show Fileops');
		$button->show();
		$button->connect_object('clicked', create_function('$w', '$w->show_fileop_buttons();'), $window);
		$action_area->pack_start($button, false, false, 0);
	}
	if ($windows['file_selection']->flags() & GTK_VISIBLE)
		$windows['file_selection']->hide();
	else
		$windows['file_selection']->show();
}

function label_toggle($dialog, $label, $dialog)
{
	if (!$label)
	{
		$label = new GtkLabel('Dialog Test');
		$label->set_padding(10, 10);
		$label->connect_object('destroy', create_function('$w', '$w=null;'), &$label);
		$vbox = $dialog->vbox;
		$vbox->pack_start($label, true, true, 0);
		$label->show();
	} else
		$label->destroy();
}

function create_dialog()
{
	global $windows;
	static $label = null;

	if (!isset($windows['dialog'])) {
		$dialog = &new GtkDialog;
		$windows['dialog'] = $dialog;
		$dialog->set_title('GtkDialog');
		$dialog->set_border_width(0);
		$dialog->set_usize(200, 110);
		$dialog->connect('delete_event', 'delete_event');

		$button = &new GtkButton('Ok');
		//$button->set_flags(GTK_CAN_DEFAULT);
		$button->connect('clicked', 'close_window');

		$action_area = $dialog->action_area;
		$action_area->pack_start($button, true, true, 0);
		$button->grab_default();
		$button->show();

		$button = &new GtkButton('Toggle');
		$button->connect('clicked', 'label_toggle', &$label, $dialog);
		//$button->set_flags(GTK_CAN_DEFAULT);
		$action_area->pack_start($button, true, true, 0);
		$button->show();
	}
	if ($windows['dialog']->flags() & GTK_VISIBLE)
		$windows['dialog']->hide();
	else
		$windows['dialog']->show();
}

function event_watcher($object, $signal_id)
{
	echo 'Event watch: ' . gtk::signal_name($signal_id) . ' emitted for ' . gtk::type_name($object->get_type()) . "\n";
	return true;
}

function event_watcher_toggle($event_watcher_enter_id, $event_watcher_leave_id)
{
	if ($event_watcher_enter_id)
		event_watcher_down(&$event_watcher_enter_id, &$event_watcher_leave_id);
	else
	{
		$signal_id = Gtk::signal_lookup('enter_notify_event', GtkWidget::get_type());
		$event_watcher_enter_id = gtk::signal_add_emission_hook($signal_id, 'event_watcher');
		$signal_id = Gtk::signal_lookup('leave_notify_event', GtkWidget::get_type());
		$event_watcher_leave_id = gtk::signal_add_emission_hook($signal_id, 'event_watcher');
	}
}

function event_watcher_down($event_watcher_enter_id, $event_watcher_leave_id)
{
	if ($event_watcher_enter_id)
	{
		$signal_id = Gtk::signal_lookup('enter_notify_event', GtkWidget::get_type());
		gtk::signal_remove_emission_hook($signal_id, $event_watcher_enter_id);
		$event_watcher_enter_id = 0;
		$signal_id = Gtk::signal_lookup('leave_notify_event', GtkWidget::get_type());
		gtk::signal_remove_emission_hook($signal_id, $event_watcher_leave_id);
		$event_watcher_leave_id = 0;
	}
}
		
function create_event_watcher()
{
	static $dialog = null;
	static $event_watcher_enter_id = 0;
	static $event_watcher_leave_id = 0;

	if (!$dialog) {
		$dialog = new GtkDialog;
		$dialog->connect_object('destroy', 'event_watcher_down', &$event_watcher_enter_id, &$event_watcher_leave_id);
		$dialog->connect_object('destroy', create_function('$w', '$w = null;'), &$dialog);
		$dialog->set_title('Event Watcher');
		$dialog->set_border_width(0);
		$dialog->set_usize(200, 110);

		$vbox = $dialog->vbox;
		$action_area = $dialog->action_area;

		$button = &new GtkToggleButton('Activate Watch');
		$button->connect_object('clicked', 'event_watcher_toggle', &$event_watcher_enter_id, &$event_watcher_leave_id);
		$button->set_border_width(10);
		$vbox->pack_start($button);
		$button->show();

		$button = &new GtkButton('Close');
		$button->connect_object('clicked', 'event_watcher_down', &$event_watcher_enter_id, &$event_watcher_leave_id);
		$button->connect_object('clicked', array($dialog, 'destroy'));
		//$button->set_flags(GTK_CAN_DEFAULT);
		$action_area->pack_start($button);
		$button->grab_default();
		$button->show();
	}
	if (!($dialog->flags() & GTK_VISIBLE))
		$dialog->show();
	else
		$dialog->destroy();
}

/*
 * GtkNotebook
 */

$book_open = null;
$book_open_mask = null;
$book_closed = null;
$book_closed_mask = null;
$sample_notebook = null;

function page_switch($notebook, $page, $page_num)
{
	global $book_open, $book_open_mask,
	$book_closed, $book_closed_mask;

	/* The second parameter of the 'switch_page' callback
	doesn't work as expected; in fact, the GtkNotebookPage is
	not passed to it. So we'll do a dirty workaround here. */

	/* Set the icon of all pages to closed pixmap */
	foreach ($notebook->children() as $child) {
		$tab_label = $notebook->get_tab_label($child);
		$children = $tab_label->children();
		$pixwid = $children[0];
		$pixwid->set($book_closed, $book_closed_mask);
	}

	/* Set the icon of the current page to open pixmap */
	$tab_label = $notebook->get_tab_label($notebook->get_nth_page($page_num));
	$children = $tab_label->children();
	$pixwid = $children[0];
	$pixwid->set($book_open, $book_open_mask);
}

function tab_fill($button, $child, $notebook)
{
	list($expand,, $pack_type) = $notebook->query_tab_label_packing($child);
	$notebook->set_tab_label_packing($child, $expand, $button->get_active(), $pack_type);
}

function tab_expand($button, $child, $notebook)
{
	list(, $fill, $pack_type) = $notebook->query_tab_label_packing($child);
	$notebook->set_tab_label_packing($child, $button->get_active(), $fill, $pack_type);
}

function tab_pack($button, $child, $notebook)
{
	list($expand, $fill) = $notebook->query_tab_label_packing($child);
	$notebook->set_tab_label_packing($child, $expand, $fill, (int)$button->get_active());
}

function notebook_homogeneous($button, $notebook)
{
	$notebook->set_homogeneous_tabs($button->get_active());
}

function notebook_popup($button, $notebook)
{
	if ($button->get_active())
		$notebook->popup_enable();
	else
		$notebook->popup_disable();
}

function notebook_rotate($notebook)
{
	$notebook->set_tab_pos(($notebook->tab_pos + 1) % 4);
}

function show_all_pages($notebook)
{
	foreach ($notebook->children() as $child)
		$child->show();
}

function standard_notebook($menuitem, $notebook)
{
	$notebook->set_show_tabs(true);
	$notebook->set_scrollable(false);
	if (count($notebook->children()) == 15)
		for ($i = 0; $i < 10; $i++)
			$notebook->remove_page(5);
}

function notabs_notebook($menuitem, $notebook)
{
	$notebook->set_show_tabs(false);
	if (count($notebook->children()) == 15)
		for ($i = 0; $i < 10; $i++)
			$notebook->remove_page(5);
}

function scrollable_notebook($menuitem, $notebook)
{
	$notebook->set_show_tabs(true);
	$notebook->set_scrollable(true);
	if (count($notebook->children()) == 5)
		create_pages(&$notebook, 6, 15);
}

function create_pages($notebook, $start, $end)
{
	global $book_closed, $book_closed_mask,
		   $book_open, $book_open_mask;

	for ($i = $start; $i <= $end; $i++)
	{
		$child = &new GtkFrame("Page $i");
		$child->set_border_width(10);

		$vbox = &new GtkVBox(true, 0);
		$vbox->set_border_width(10);
		$child->add($vbox);

		$hbox = &new GtkHbox(true, 0);
		$vbox->pack_start($hbox, false, true, 5);

		$button = &new GtkCheckButton('Fill Tab');
		$hbox->pack_start($button, true, true, 5);
		$button->set_active(true);
		$button->connect('toggled', 'tab_fill', &$child, &$notebook);

		$button = &new GtkCheckButton('Expand Tab');
		$hbox->pack_start($button, true, true, 5);
		$button->connect('toggled', 'tab_expand', &$child, &$notebook);

		$button = &new GtkCheckButton('Pack end');
		$hbox->pack_start($button, true, true, 5);
		$button->connect('toggled', 'tab_pack', &$child, &$notebook);

		$button = &new GtkButton('Hide page');
		$vbox->pack_end($button, false, false, 5);
		$button->connect_object('clicked', array(&$child, 'hide'));

		$child->show_all();

		$label_box = &new GtkHBox(false, 0);
		$pixwid = &new GtkPixmap($book_closed, $book_closed_mask);
		$label_box->pack_start($pixwid, false, true, 0);
		$pixwid->set_padding(3, 1);

		$label = &new GtkLabel("Page $i");
		$label_box->pack_start($label, false, true, 0);
		$label_box->show_all();

		$menu_box = &new GtkHBox(false, 0);
		$pixwid = &new GtkPixmap($book_closed, $book_closed_mask);
		$menu_box->pack_start($pixwid, false, true, 0);
		$pixwid->set_padding(3, 1);

		$label = &new GtkLabel("Page $i");
		$menu_box->pack_start($label, false, true, 0);
		$menu_box->show_all();
		
		$notebook->append_page_menu($child, $label_box, $menu_box);
	}
}

function create_notebook()
{
	global $windows, $sample_notebook,
		   $book_open, $book_open_mask,
		   $book_closed, $book_closed_mask,
		   $book_open_xpm, $book_closed_xpm;

	$items = array(
		'Standard'	=>	array('standard_notebook', &$sample_notebook),
		'No tabs'	=>	array('notabs_notebook', &$sample_notebook),
		'Scrollable'=>	array('scrollable_notebook', &$sample_notebook)
	);

	if (!isset($windows['notebook'])) {
		$window = new GtkWindow;
		$windows['notebook'] = $window;
		$window->connect('delete_event', 'delete_event');
		$window->set_title('Notebook');
		$window->set_border_width(0);

		$box1 = &new GtkVBox(false, 0);
		$box1->show();
		$window->add($box1);

		$sample_notebook = new GtkNotebook;
		$sample_notebook->show();
		$sample_notebook->connect('switch_page', 'page_switch');
		$sample_notebook->set_tab_pos(GTK_POS_TOP);
		$box1->pack_start($sample_notebook, true, true, 0);
		$sample_notebook->set_border_width(10);
		$sample_notebook->realize();

		list($book_open, $book_open_mask) = Gdk::pixmap_create_from_xpm_d($sample_notebook->window, null, $book_open_xpm);
		list($book_closed, $book_closed_mask) = Gdk::pixmap_create_from_xpm_d($sample_notebook->window, null, $book_closed_xpm);

		create_pages(&$sample_notebook, 1, 5);

		$separator = &new GtkHSeparator;
		$separator->show();
		$box1->pack_start($separator, false, true, 10);

		$box2 = &new GtkHBox(false, 5);
		$box2->show();
		$box2->set_border_width(10);
		$box1->pack_start($box2, false, true, 0);

		$button = &new GtkCheckButton('popup menu');
		$button->show();
		$box2->pack_start($button, true, false, 0);
		$button->connect('clicked', 'notebook_popup', &$sample_notebook);

		$button = &new GtkCheckButton('homogeneous tabs');
		$button->show();
		$box2->pack_start($button, true, false, 0);
		$button->connect('clicked', 'notebook_homogeneous', &$sample_notebook);

		$box2 = &new GtkHBox(false, 5);
		$box2->show();
		$box2->set_border_width(10);
		$box1->pack_start($box2, false, true, 0);

		$label = &new GtkLabel('Notebook Style:');
		$label->show();
		$box2->pack_start($label, false, true, 0);

		$omenu = build_option_menu($items, 3, 0, &$sample_notebook);
		$omenu->show();
		$box2->pack_start($omenu, false, true, 0);

		$button = &new GtkButton('Show all Pages');
		$button->show();
		$box2->pack_start($button, false, true, 0);
		$button->connect_object('clicked', 'show_all_pages', &$sample_notebook);

		$box2 = &new GtkHBox(true, 10);
		$box2->show();
		$box2->set_border_width(10);
		$box1->pack_start($box2, false, true, 0);

		$button = &new GtkButton('prev');
		$button->show();
		$button->connect_object('clicked', array(&$sample_notebook, 'prev_page'));
		$box2->pack_start($button, true, true, 0);

		$button = &new GtkButton('next');
		$button->show();
		$button->connect_object('clicked', array(&$sample_notebook, 'next_page'));
		$box2->pack_start($button, true, true, 0);

		$button = &new GtkButton('rotate');
		$button->show();
		$button->connect_object('clicked', 'notebook_rotate', &$sample_notebook);
		$box2->pack_start($button, true, true, 0);

		$separator = &new GtkHSeparator;
		$separator->show();
		$box1->pack_start($separator, false, true, 5);

		$button = &new GtkButton('close');
		$button->show();
		$button->set_border_width(5);
		$button->connect('clicked', 'close_window');
		$box1->pack_start($button, false, false, 0);
		//$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
	}
	if ($windows['notebook']->flags() & GTK_VISIBLE)
		$windows['notebook']->hide();
	else
		$windows['notebook']->show();
}

function create_main_window()
{
	$buttons = array(
					 'buttons'			=> 'create_buttons',
					 'labels'			=> 'create_labels',
					 'button box'		=> 'create_button_box',
					 'toggle buttons'	=> 'create_toggle_buttons',
					 'check buttons'	=> 'create_check_buttons',
					 'radio buttons'	=> 'create_radio_buttons',
					 'tooltips'			=> 'create_tooltips',
					 'entry'			=> 'create_entry',
					 'clist'			=> 'create_clist',
					 'color selection'	=> 'create_color_selection',
					 'cursors'			=> 'create_cursor_test',
					 'ctree'			=> 'create_ctree',
					 'event watcher'	=> 'create_event_watcher',
					 'notebook'			=> 'create_notebook',
					 'drawing area'		=> null,
					 'file selection'	=> 'create_file_selection',
					 'dialog'			=> 'create_dialog',
					 'panes'			=> 'create_panes',
					 'pixmap'			=> 'create_pixmap',
					 'drag\'n\'drop'	=> 'create_dnd',
					);

	$window = &new GtkWindow();
	$window->set_policy(false, false, false);
	$window->set_name('main window');
	$window->set_usize(200, 400);
	$window->set_position(GTK_WIN_POS_CENTER);

	$window->connect_object('destroy', array('gtk', 'main_quit'));
	$window->connect_object('delete-event', array('gtk', 'false'));

	$box1 = &new GtkVBox();
	$window->add($box1);

	$scrolled_window = &new GtkScrolledWindow();
	$scrolled_window->set_border_width(10);
	$scrolled_window->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	$box1->pack_start($scrolled_window);

	$box2 = &new GtKVBox();
	$box2->set_border_width(10);
	$scrolled_window->add_with_viewport($box2);
	$box2->set_focus_vadjustment($scrolled_window->get_vadjustment());

	ksort($buttons);
	foreach ($buttons as $label	=> $function) {
		$button = &new GtkButton($label);
		if ($function)
			$button->connect('clicked', $function);
		else
			$button->set_sensitive(false);
		$box2->pack_start($button);
	}

	$separator = &new GtkHSeparator();
	$box1->pack_start($separator, false);

	$box2 = &new GtkVBox(false, 10);
	$box2->set_border_width(10);
	$box1->pack_start($box2, false);

	$button = &new GtkButton('close');
	$button->connect_object('clicked', array('gtk', 'main_quit'));
	$box2->pack_start($button);
	//$button->set_flags(GTK_CAN_DEFAULT);
	$button->grab_default();

	$window->show_all();
}

//#Gtk::rc_parse(dirname($argv[0]).'/testgtkrc');
create_main_window();
Gtk::main();

?>
