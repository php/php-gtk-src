<?php
/**
*   Small event generator to do automated unit tests with PHP-Gtk2
*   Supports different drivers.
*
*   A generator has the following methods:
*   - sendKey($key)
*   - sendKeyPress($key)
*   - sendKeyRelease($key)
*   where $key is a string/char what key to send.
*   For example, " " would send a space, "a" a lowercase a
*   "ctrl" a left control and so on.
*
*   @author Christian Weiske <cweiske@php.net>
*/
class EventGenerator
{
    function createGenerator($type = null)
    {
        if ($type == null) {
            $type = EventGenerator::getDefaultGenerator();
        }
        $class = 'EventGenerator_' . $type;
        include_once($class . '.php');
        if (!class_exists($class)) {
            die('Couldn\'t load class "' . $class . '"' . "\r\n");
        }
        return new $class();
    }//function createGenerator($type = 'xte')



    function getDefaultGenerator()
    {
        if (file_exists('EventGenerator.config.php')) {
            include_once('EventGenerator.config.php');
        }
        if (defined('EVENT_GENERATOR')) {
            return EVENT_GENERATOR;
        }
        //no static class properties in php4
        return 'xte';
    }//function getDefaultGenerator()
}//class EventGenerator

?>