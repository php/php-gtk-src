<?php
/* This API is only available afer 2.12, before then use libglade */
if(Gtk::check_version(2, 12, 0))
die ('New Tooltip API only available in GTK 2.12 or higher');

$xml = '
<interface>"
  <object class="GtkWindow" id="window1">
    <child>
      <object class="GtkVBox" id="vbox">
        <child>
          <object class="GtkButton" id="button1">
            <property name="label">Foo</property>
            <property name="can-default">True</property>
            <property name="has-default">True</property>
            <signal name="clicked" handler="foo" />
          </object>
        </child>
        <child>
          <object class="GtkButton" id="button2">
            <property name="label">Bar</property>
            <signal name="clicked" handler="handler" />
          </object>
        </child>
      </object>
    </child>
    <signal name="destroy" handler="Gtk::main_quit" />
  </object>
</interface>';

function foo()
{
	echo "button was clicked\n";
}

class handler
{
	function doBar()
	{
		echo "another button was clicked\n";
	}
}

$builder = new GtkBuilder();
$builder->add_from_string($xml);
$builder->add_from_file(dirname(__FILE__) . '/notebook.xml');
$class = new handler();
$builder->connect_signals(array('handler' => array($class, 'doBar')));
// could also use $builder->connect_signals_instance if they were all in a single class

$builder->get_object('vbox')->add($builder->get_object('mynotebook'));

$objects = $builder->get_objects();
echo count($objects), " objects were created by GtkBuilder\n";

$builder->set_translation_domain('myapp');
echo $builder->get_translation_domain(), "\n";

var_dump($builder->get_type_from_name('GtkButton'));

$window = $builder->get_object('window1');
$window->show_all();
Gtk::main();
?>