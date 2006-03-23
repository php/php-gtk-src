<?php

/* Ugly, Ugly way of exporting gtkcontainer for sourceview */
$contents = file("ext/sourceview/gen_sourceview.c");
$tmpFile = fopen("ext/sourceview/gen_temp.c", "w+");

for($i=0; $i<16; $i++) {
    fwrite($tmpFile, $contents[$i]);
}

fwrite($tmpFile, "\nPHP_GTK_EXPORT_CE(gtkcontainer_ce);\n");
fwrite($tmpFile, "PHP_GTK_EXPORT_CE(gdkpixbuf_ce);\n");
fwrite($tmpFile, "PHP_GTK_EXPORT_CE(gtktextbuffer_ce);\n");
fwrite($tmpFile, "PHP_GTK_EXPORT_CE(gtktexttagtable_ce);\n");
fwrite($tmpFile, "PHP_GTK_EXPORT_CE(gtktextview_ce);\n");
fwrite($tmpFile, "PHP_GTK_EXPORT_CE(gtktextmark_ce);");

for($i=16; $i<=count($contents); $i++) {
    fwrite($tmpFile, $contents[$i]);
}

fclose($tmpFile);
$toPut = file("ext/sourceview/gen_temp.c");
file_put_contents("ext/sourceview/gen_sourceview.c", $toPut);

?>
