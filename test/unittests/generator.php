<?php
// Use PHPUnit2 to create the skeletons.
//ATTENTION: needs phpunit2 bugfix
// http://pear.php.net/bugs/bug.php?id=6965
die("deactived for security reasons");
require_once 'PHPUnit/Util/Skeleton.php';

// Get all of the classes in PHP-GTK 2.
/* Get all classes and filter out Gtk classes only */
// Taken from Anant's updater script.
$classes = array();
$allClasses = get_declared_classes();

$extensions = array('Gtk', 'Gdk', 'Atk', 'Pango');
foreach ($extensions as $extension) {
    foreach($allClasses as $oneClass) {
        if(strpos($oneClass, $extension) === 0) {
            $classes[] = $oneClass;
        }
    }
}

sort($classes);

foreach ($classes as $class) {
    // Create a skeleton for the class.
    $skeleton = new PHPUnit_Util_Skeleton($class);

    // Write the test.
    $skeleton->write();
}
?>