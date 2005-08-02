<?php
/**
*   Generates doc files for enumerations
*   @author Christian Weiske <cweiske@php.net>
*/
require_once('doc_templates.php');

class DocEnumGenerator
{
    private $type = 'gtk';
    private $outputfile = null;
    private $fileHandle = null;


    public function DocEnumGenerator($type, $outputfile)
    {
        $this->type = ucfirst(strtolower($type));
        $this->outputfile = $outputfile;
    }//public function DocEnumGenerator($type, $outputfile)



    public function createDocs()
    {
        global $enums_start, $enums_end;

        $this->openFile();
        $this->writePart(sprintf($enums_start, strtolower($this->type)));
        $this->getEnumGroups($this->type);
        $this->writePart($enums_end);
        $this->closeFile();
    }//public function createDocs()



    protected function getEnumGroups()
    {
        $rc          = new ReflectionClass($this->type);
        $arConstants = $rc->getConstants();
        $arGroups    = array();

        foreach ($arConstants as $constant => $value) {
            $group = substr($constant, 0, strpos($constant, '_'));
            $arGroups[$group][$value] = $constant;
        }

        ksort($arGroups);
        foreach ($arGroups as $group => $arConstants) {
            $this->writeEnumGroup($group, $arConstants);
        }
    }//protected function getEnumGroups()



    protected function writeEnumGroup($groupname, $arConstants)
    {
        global $enum_start, $enum_end, $enum_option;

        ksort($arConstants);
        $this->writePart(sprintf($enum_start, strtolower($this->type), strtolower($groupname), $this->type, ucfirst(strtolower($groupname))));
        foreach ($arConstants as $value => $constant) {
            $this->writePart(sprintf($enum_option, $value, $this->type, $constant));
        }
        $this->writePart($enum_end);
    }//protected function writeEnumGroup($arData)



    protected function openFile()
    {
        if ($this->outputfile !== null) {
            $this->fileHandle = fopen($this->outputfile, 'w+');
        }
    }//protected function openFile()



    protected function writePart($data)
    {
        if ($this->outputfile === null) {
            echo $data;
        } else {
            fwrite($this->fileHandle, $data);
        }
    }//protected function writePart($data)



    protected function closeFile()
    {
        if ($this->outputfile !== null) {
            fclose($this->fileHandle);
        }
    }//protected function closeFile()

}//class DocEnumGenerator


if (!class_exists('gtk') || version_compare(phpversion(), '5.0.0', 'ge') == 0) {
    die('This script can only be used with php5 and php-gtk2 installed' . "\r\n");
}

if ($argc < 2 || $argv[1] == '-h' || $argv[1] == '--help') {
    die(
'Enumeration documentation generator for php-gtk2
php docgen_enum.php <classtype> <outputfile>
  with <classtype> being one of gdk, gtk, atk, pango
'
    );
}

$type = $argv[1];

if ($argc >= 3) {
    $outputfile = $argv[2];
} else {
    $outputfile = null;
}

$deg = new DocEnumGenerator($type, $outputfile);
$deg->createDocs();

?>