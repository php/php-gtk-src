<?php

class NewType extends GtkButton {
	public $__gsignals = array(
		'clicked' => 'override',
		'mashed'  => array(GObject::SIGNAL_RUN_LAST, GObject::TYPE_BOOLEAN, array(GObject::TYPE_LONG, GtkRequisition::gtype)),
		);

	function __construct()
	{
		parent::__construct();
	}

	function __do_clicked()
	{
		echo "do_clicked called\n";
	}

	function __do_mashed($arg, $arg2)
	{
		echo "NewType: class closure for `mashed` called with arguments {$arg}, ", get_class($arg2), "\n";
	}
}

class NewType2 extends NewType {
	function __do_mashed($arg, $arg2)
	{
		echo "NewType2: class closure for `mashed` called, chaining up to NewType\n";
		NewType::__do_mashed($arg, $arg2);
	}
}

function my_handler($obj, $arg, $arg2, $ex)
{
	echo "my_handler called with args {$arg}, ", get_class($arg2), " and extra {$ex}\n";
	return true;
}

GObject::register_type('NewType');
GObject::register_type('NewType2');

$c = new NewType;
$d = new NewType2;
$r = $c->size_request();
$c->connect('mashed', 'my_handler', 99);
$c->emit('clicked');
var_dump($c->emit('mashed', 42, $r));
var_dump($d->emit('mashed', 24, $r));
