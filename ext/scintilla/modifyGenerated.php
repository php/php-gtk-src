<?php

/* Ugly, Ugly way of exporting classes req for scintilla */
$contents = file("ext/scintilla/gen_scintilla.c");
$tmpFile = fopen("ext/scintilla/gen_temp.c", "w+");

for($i=0; $i<5; $i++) {
    fwrite($tmpFile, $contents[$i]);
}

fwrite($tmpFile, "\nPHP_GTK_EXPORT_CE(gtkframe_ce);\n");

for($i=5; $i<=count($contents); $i++) {
    fwrite($tmpFile, $contents[$i]);
}

fclose($tmpFile);
$toPut = file("ext/scintilla/gen_temp.c");
file_put_contents("ext/scintilla/gen_scintilla.c", $toPut);

?>
