<?php

/* Ugly, Ugly way of exporting gtkcontainer for sourceview */
$contents = file("ext/sourceview/gen_sourceview.c");
$tmpFile = fopen("ext/sourceview/gen_temp.c", "w+");

for($i=0; $i<5; $i++) {
    fwrite($tmpFile, $contents[$i]);
}

fwrite($tmpFile, "\nPHP_GTK_EXPORT_CE(gtkcontainer_ce);\n");

for($i=5; $i<=count($contents); $i++) {
    fwrite($tmpFile, $contents[$i]);
}

fclose($tmpFile);
$toPut = file("ext/sourceview/gen_temp.c");
file_put_contents("ext/sourceview/gen_sourceview.c", $toPut);

?>
