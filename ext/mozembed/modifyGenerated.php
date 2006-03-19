<?php

/* Ugly, Ugly way of exporting gtkbin for mozembed */
$contents = file("ext/mozembed/gen_mozembed.c");
$tmpFile = fopen("ext/mozembed/gen_temp.c", "w+");

for($i=0; $i<5; $i++) {
    fwrite($tmpFile, $contents[$i]);
}

fwrite($tmpFile, "\nPHP_GTK_EXPORT_CE(gtkbin_ce);\n");

for($i=6; $i<=count($contents); $i++) {
    fwrite($tmpFile, $contents[$i]);
}

fclose($tmpFile);
$toPut = file("ext/mozembed/gen_temp.c");
file_put_contents("ext/mozembed/gen_mozembed.c", $toPut);

?>
