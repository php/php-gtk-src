<?php
/* $Id$ */

if (strtoupper(substr(PHP_OS, 0, 3)) == 'WIN')
	dl('php_gtk.dll');
else
	dl('php_gtk.so');

$windows = array();

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
		$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
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
							   "This one is underlined in 日本語の入用quite a funky fashion");
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
		$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
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
		$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
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
		$frame->set_label_align(0.5, 0.0);
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
		$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();

		$tooltips->set_tip($button, 'Push this button to close window', 'push');
		$tooltips->enable();
	}
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
		$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
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
		$button->set_flags(GTK_CAN_DEFAULT);
		$button->grab_default();
		$button->show();
	}
	$windows['entry']->show();
}


function create_main_window()
{
	$buttons = array(
					 'buttons'			=> 'create_buttons',
					 'labels'			=> 'create_labels',
					 'button box'		=> 'create_button_box',
					 'toggle buttons'	=> 'create_toggle_buttons',
					 'check buttons'	=> null,
					 'radio buttons'	=> null,
					 'tooltips'			=> 'create_tooltips',
					 'entry'			=> 'create_entry',
					);

	$window = &new GtkWindow();
	$window->set_policy(false, false, false);
	$window->set_name('main window');
	$window->set_usize(200, 400);
	$window->set_uposition(20, 20);

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
	$button->set_flags(GTK_CAN_DEFAULT);
	$button->grab_default();

	$window->show_all();
}

Gtk::rc_parse('testgtkrc');
create_main_window();
Gtk::main();

?>
