<?php

class NewType extends GtkLabel {
	public $__gproperties = array(
		'foo' => array(GObject::TYPE_STRING,  'foo property', 'new foo property', GObject::PARAM_READWRITE, 'default foo value'),
		'bar' => array(GObject::TYPE_OBJECT,  'bar property', 'new bar property', GObject::PARAM_READWRITE),
		'zoo' => array(GObject::TYPE_BOOLEAN, 'zoo property', 'new zoo property', GObject::PARAM_READABLE, 0),
		);
	private $foo;
	private $bar;
	private $zoo = 1;

	function __construct()
	{
		parent::__construct();
		$this->foo = 'abcdef';
	}

	function __get_gproperty($spec)
	{
		echo "__get_gproperty called for $spec\n";
		if ($spec->name == 'foo') {
			return $this->foo;
		} else if ($spec->name == 'bar') {
			return $this->bar;
		} else if ($spec->name == 'zoo') {
			return $this->zoo;
		} else {
			trigger_error('Unknown property');
		}
	}

	function __set_gproperty($spec, $value)
	{
		echo "__set_gproperty called for $spec = $value\n";
		if ($spec->name == 'foo') {
			$this->foo = $value;
		} else if ($spec->name == 'bar') {
			$this->bar = $value;
		} else {
			trigger_error('Unknown property');
		}
	}
}

GObject::register_type('NewType');
echo $c, "\n";

//var_dump(GObject::list_properties('NewType'));

$c = new NewType;
var_dump($c->get_property('foo'));
var_dump($c->get_property('bar'));
var_dump($c->get_property('zoo'));
//var_dump($c->get_property('unknown')); issues a warning
$c->set_property('bar', new GtkButton());
//$c->set_property('zoo', 0); issues a warning
echo $c->get_property('bar');
echo "\n";

