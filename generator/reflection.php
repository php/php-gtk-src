<?php
/**
* small helper for manual reflection generation
*/

if (!isset($argv[1]) || !file_exists($argv[1])) {
    die('Pass the .c source file as first parameter' . "\r\n");
}


$tmpl_block_arginfo = <<<EOD
static
ZEND_BEGIN_ARG_INFO(%s, 0)
%s
ZEND_END_ARG_INFO();


EOD;
$tmpl_block_arginfoex = <<<EOD
static
ZEND_BEGIN_ARG_INFO_EX(%s, 0, 0, 1)
%s
ZEND_END_ARG_INFO();


EOD;
$tmpl_arginfo = <<<EOD
//    ZEND_ARG_INFO(0, %)

EOD;
$tmpl_argobjinfo = <<<EOD
//    ZEND_ARG_OBJ_INFO(0, %s, %s, 0)

EOD;


$file = $argv[1];
$arLines = file($file);

foreach ($arLines as $nLine => $strLine) {
    if (substr($strLine, 0, 18) == 'static PHP_METHOD(') {
        preg_match_all('/^static\\s*PHP_METHOD\\s*\\(\\s*([a-zA-Z0-9_-]+)\\s*,\\s*([a-zA-Z0-9_-]+)\\s*\\)$/', $strLine, $arMatches);
        if (count($arMatches[1]) == 0) {
            echo 'Unsupported PHP_METHOD: "' . $strLine . '"' . "\r\n";
            continue;
        }
        
        $strFuncName = 'arginfo_' . strtolower($arMatches[1][0]) . '_' . strtolower($arMatches[2][0]);
        echo sprintf($tmpl_block_arginfoex, $strFuncName, null);
        
    }//static php method
}//foreach line

?>