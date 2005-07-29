<?php
/**
*   Executes all the bug files and checks the output
*
*   Each bug file can be run with a "debug" as parameter
*   and will put out the error message
*/
require_once('bugconfig.php');

$counter = array('passed' => 0, 'error' => 0);

function runBugfile($file)
{
    global $counter;
    
    $lastline = trim(exec(PHP_EXECUTABLE . ' ' . $file));
    if ($lastline == 'ok') {
        $status = 'passed';
    } else {
        $status = 'error';
    }
    echo str_pad($file, 60, ' ') . $status . "\r\n";
    $counter[$status]++;
}

function percent($value, $all)
{
    if ($all == 0 || $value == 0) {
        return '  0.00';
    }
    return number_format(100 / $all * $value, 2);
}

$files = glob('bug_*.phpw');
foreach ($files as $bugfile)
{
    runBugfile($bugfile);
}

echo "\r\nScore:\r\n";
$sum = array_sum($counter);
foreach ($counter as $id => $value) {
    echo ' ' . str_pad($id, 7, ' ') . ': ' . str_pad($value, 5, ' ', STR_PAD_LEFT) . ' (' . percent($value, $sum) . '%)' . "\r\n";
}

?>