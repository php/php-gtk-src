<?php
/**
* PHP has real problems if a method requires a parameter of a certain
*  class and the class doesn't exist... 
*
* here we check that
* 
* The classes spit out by this script have to be entered in 
*  generator.php function is_in_php()
*/

$arFiles = array(
    'gen_atk.c',
    'gen_gdk.c',
    'gen_gtk.c',
    'gen_pango.c'
);
$strRelPath = '../ext/gtk+/';


chdir(dirname(__FILE__));
$arDeclClasses = get_declared_classes();
$arFoundClasses = array();
foreach ($arFiles as $strFile) {
    $arLines = file($strRelPath . $strFile);
    
    foreach ($arLines as $strLine) {
        if (strpos($strLine, 'ZEND_ARG_OBJ_INFO') && preg_match_all('/ZEND_ARG_OBJ_INFO\\(.+,.+,\\s*([^,]+)\\s*,.+\\)/', $strLine, $arMatches)) {
            $strClass = $arMatches[1][0];
            if (!in_array($strClass,$arDeclClasses) && !in_array($strClass, $arFoundClasses)) {
                echo "'" . $strClass . "' => 'int',\r\n";
                $arFoundClasses[] = $strClass;
            }
        }
    }
}
?>