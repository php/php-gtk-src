<?php
/* $Id$ */

if (strtoupper(substr($OS, 0, 3)) == 'WIN')
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

function destroy($window)
{
	Gtk::main_quit();
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

		$vbox = &new GtkVBox(false, 5);
		$hbox = &new GtkHBox(false, 5);
		$window->add($hbox);
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
	}
	$windows['button_box']->show();
}


function create_main_window()
{
	$buttons = array(
					 'buttons'			=> 'create_buttons',
					 'labels'			=> 'create_labels',
					 'button box'		=> 'create_button_box',
					 'toggle buttons'	=> null,
					 'check buttons'	=> null,
					 'radio buttons'	=> null,
					 'tooltips'			=> null,
					);

	$window = &new GtkWindow();
	$window->set_policy(false, false, false);
	$window->set_name('main window');
	$window->set_usize(200, 400);
	$window->set_uposition(20, 20);

	$window->connect('destroy', 'destroy');
	$window->connect('delete-event', 'destroy');

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
	$button->connect('clicked', 'destroy');
	$box2->pack_start($button);
	$button->set_flags(GTK_CAN_DEFAULT);
	$button->grab_default();

	$window->show_all();
}

create_main_window();
Gtk::main();

?>
