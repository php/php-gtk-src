<?php
/**
*   Small window which displays various controls
*   to change settings of a given widget.
*   To see how the widget behaves
*
*   @author Christian Weiske <cweiske@php.net>
*/


class WidgetEditor extends GtkWindow
{
    private $table = null;
    
    
    function __construct($widget, $data)
    {
        parent::__construct();
        $this->set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
        $this->set_title('WidgetEditor');
        
        $this->table = new GtkTable(count($data), 2, false);
        
        foreach ($data as $nPos => $dat) {
            $this->addEditor($widget, $nPos, $dat);
        }
        
        $this->add($this->table);
        
        
        $this->show_all();
    }//function __construct($widget, $data)



    function addEditor($basewidget, $position, $data)
    {
        $this->table->attach(new GtkLabel($data[0]), 0, 1, $position, $position + 1);

        switch ($data[1]) {
            case GtkScale::gtype:
                $widget = $this->createScale($basewidget, $data[0], $data[2], $data[3]);
                break;
            case GtkComboBox::gtype:
                $widget = $this->createComboBox($basewidget, $data[0], $data[2]);
                break;
            case GtkToggleButton::gtype:
                if (!isset($data[2]) || !isset($data[3])) {
                    $data[2] = 'on';
                    $data[3] = 'off';
                }
                if (!isset($data[4])) {
                    $data[4] = true;
                }
                $widget = $this->createToggleButton($basewidget, $data[0], $data[2], $data[3], $data[4]);
                break;
            case GtkButton::gtype:
                if (!isset($data[2])) {
                    $data[2] = null;
                }
                $widget = $this->createButton($basewidget, $data[0], $data[2]);
                break;
            default:
                $widget = new GtkLabel('Unsupported type ' . $data[1]);
                break;
        }

        $this->table->attach($widget, 1, 2, $position, $position + 1);
    }//function addEditor($position, $data)



    function createScale($basewidget, $basefunction, $from, $to)
    {
        $scale = GtkHScale::new_with_range($from, $to, 1);
        
        $function = 'get_' . $basefunction;
        $scale->set_value($basewidget->$function());
        
        $scale->connect('change-value', array($this, 'set_scale'), $basewidget, 'set_' . $basefunction);
        
        return $scale;
    }//function createScale($basewidget, $function, $from, $to)



    function createComboBox($basewidget, $basefunction, $strConstants)
    {
        $combo = GtkComboBox::new_text();
        $type = substr($strConstants, 0, strpos($strConstants, '::'));
        $name = substr($strConstants, strpos($strConstants, '::') + 2);

        $r = new ReflectionClass($type);
        $arConstants = $r->getConstants();

        foreach ($arConstants as $id => $value) {
            if (substr($id, 0, strlen($name)) == $name) {
                $combo->insert_text($value, $type . '::' . $id);
            }
        }

        $function = 'get_' . $basefunction;
        $combo->set_active($basewidget->$function());
        $combo->connect('changed', array($this, 'set_combobox'), $basewidget, 'set_' . $basefunction);

        return $combo;
    }//function createComboBox($basewidget, $basefunction, $strConstants)



    function createToggleButton($basewidget, $basefunction, $text_on, $text_off, $bCallGetter)
    {
        $tb = new GtkToggleButton();
        $tb->set_label($tb->get_active() ? $text_on : $text_off);
        if ($bCallGetter) {
            $tb->set_active($basewidget->{'get_' . $basefunction}());
        }
        $tb->connect('toggled', array($this, 'set_toggled'), $basewidget, $basefunction, $text_on, $text_off, $bCallGetter);

        return $tb;
    }//function createToggleButton($basewidget, $basefunction, $text_on, $text_off)



    function createButton($basewidget, $function, $function2)
    {
        //call the function on the base widget
        $btn = new GtkButton($function);
        $btn->connect_object('clicked', array($basewidget, $function));

        if ($function2 === null) {
            return $btn;
        }

        $btn2 = new GtkButton($function2);
        $btn2->connect_object('clicked', array($basewidget, $function2));

        $hbox = new GtkHBox();
        $hbox->pack_start($btn);
        $hbox->pack_end($btn2);
        return $hbox;
    }//function createButton($basewidget, $function, $function2)



    function set_scale($scale, $type, $value, $widget, $function)
    {
        //I check it here because an out-of-bounds can occur while dragging the scale with the mouse
        $adjustment = $scale->get_adjustment();
        if ($value < $adjustment->lower) {
            $value = $adjustment->lower;
        } else if ($value > $adjustment->upper) {
            $value = $adjustment->upper;
        }
        
        $widget->$function($value);
    }//function set_scale($type, $value, $widget, $function)



    function set_combobox($combo, $widget, $function)
    {
        $value = $combo->get_active();
        $widget->$function($value);
    }//function set_combobox($combo, $widget, $function)
    
    
    
    function set_toggled($tb, $widget, $function, $text_on, $text_off, $bCallGetter) {
        $widget->{'set_' . $function}($tb->get_active());
        $tb->set_label($tb->get_active() ? $text_on : $text_off);
    }


}//class WidgetEditor extends GtkWindow


?>