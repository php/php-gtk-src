<?php

$fp = fopen('../src/php_gtk_gen_reg_items.h', 'w');
foreach (array_slice($argv, 1) as $module) {
	fwrite($fp, "	php_" . $module . "_register_classes();\n");
	fwrite($fp, "	php_" . $module .  "_register_constants(module_number ELS_CC);\n");
}
fclose($fp);

$fp = fopen('../src/php_gtk_gen_reg_items_decl.h', 'w');
foreach (array_slice($argv, 1) as $module) {
	fwrite($fp, "void	php_" . $module . "_register_classes(void);\n");
	fwrite($fp, "void	php_" . $module .  "_register_constants(int module_number ELS_DC);\n");
}
fclose($fp);

?>
