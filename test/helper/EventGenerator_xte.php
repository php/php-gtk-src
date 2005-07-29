<?php
/**
*   Event generation class for php-gtk2 regression tests
*   This one uses the xautomation tools, you can find them at
*    http://hoopajoo.net/projects/xautomation.html
*
*   @author Christian Weiske <cweiske@php.net>
*/

//required for System::which() functionality
require_once('System.php');


class EventGenerator_xte
{
    var $xte = null;



    function EventGenerator_xte()
    {
        $xte = System::which('xte');
        if ($xte !== false) {
            $this->xte = $xte;
        } else {
            echo "Couldn't find xte program!\r\nTests won't run!\r\n";
        }
    }//function EventGenerator_xte()



    /**
    *   Sends a key press (key down)
    *
    *   @param string   $key    Spoken keyname which shall be pressed
    */
    function sendKeyPress($key)
    {
        $key = $this->translateKey($key);
        exec($this->xte . ' \'keydown ' . $key . '\'');
    }//function sendKeyPress($key)



    /**
    *   Sends a key release (key up)
    *
    *   @param string   $key    Spoken keyname which shall be pressed
    */
    function sendKeyRelease($key)
    {
        $key = $this->translateKey($key);
        exec($this->xte . ' \'keyup ' . $key . '\'');
    }//function sendKeyRelease($key)



    /**
    *   Sends a key press+release
    *
    *   @param string   $key    Spoken keyname which shall be pressed
    */
    function sendKey($key)
    {
        $key = $this->translateKey($key);
        exec($this->xte . ' \'key ' . $key . '\'');
    }//function sendKey($key)



    /**
    *   Translates a given spoken key name in a
    *   key name which xte understands
    *
    *   @param string   $key Spoken keyname
    *   @return string  Key name for xte
    */
    function translateKey($key)
    {
        switch ($key) {
            case ' '        : return 'space';
            case 'space'    : return 'space';
            case 'home'     : return 'Home';
            case 'left'     : return 'Left';
            case 'up'       : return 'Up';
            case 'right'    : return 'Right';
            case 'down'     : return 'Down';
            case 'pgup'     : return 'Page_Up';
            case 'pgdown'   : return 'Page_Down';
            case 'end'      : return 'End';
            case 'enter'    : return 'Return';
            case 'return'   : return 'Return';
            case 'bksp'     : return 'Backspace';
            case 'backspace': return 'Backspace';
            case 'tab'      : return 'Tab';
            case 'esc'      : return 'Escape';
            case 'escape'   : return 'Escape';
            case 'del'      : return 'Delete';
            case 'delete'   : return 'Delete';
            case 'shift'    : return 'Shift_L';
            case 'shiftl'   : return 'Shift_L';
            case 'shiftr'   : return 'Shift_R';
            case 'ctrl'     : return 'Control_L';
            case 'ctrll'    : return 'Control_L';
            case 'ctrlr'    : return 'Control_R';
            case 'meta'     : return 'Meta_L';
            case 'metal'    : return 'Meta_L';
            case 'metar'    : return 'Meta_R';
            case 'alt'      : return 'Alt_L';
            case 'altl'     : return 'Alt_L';
            case 'altr'     : return 'Alt_R';
        }
        return $key;
    }//function translateKey($key)

}//class EventGenerator_xte
?>