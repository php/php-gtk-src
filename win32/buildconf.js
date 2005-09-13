/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2004 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Wez Furlong <wez@thebrainroom.com>                           |
  +----------------------------------------------------------------------+
*/

/* $Id: buildconf.js,v 1.3 2005-09-13 17:02:10 sfox Exp $ */
// This generates a configure script for win32 build

var FSO = WScript.CreateObject("Scripting.FileSystemObject");
var MODULES = WScript.CreateObject("Scripting.Dictionary");
var dir = FSO.GetFolder("ext/gtk+");

if (FSO.FileExists('configure.js')) {
	FSO.DeleteFile('configure.js');
}

var C = FSO.CreateTextFile("configure.js", true);
var temp = FSO.CreateTextFile("win32/temp.bat", true);

WScript.StdOut.WriteBlankLines(1);
WScript.StdOut.WriteLine("Rebuilding configure.js");
WScript.StdOut.WriteBlankLines(1);

function file_get_contents(filename) {

	var F = FSO.OpenTextFile(filename, 1);
	var t = F.ReadAll();

	F.Close();
	return t;
}

function gen_functions() {

	/* generate source and header files for exported PHP-GTK functions */
	temp.WriteLine("/* usage: php generator.php [-l logfile] [-o overridesfile] [-p prefix] [-c functionclass ] [-r typesfile] [-f savefile] defsfile */");
	temp.WriteLine("mkdir win32\\logs");
	temp.WriteLine("php -q generator\\generator.php -l win32\\logs\\config_atk.log -o ext\\gtk+\\atk.overrides -p atk ext\\gtk+\\atk.defs > ext\\gtk+\\gen_atk.c");
	temp.WriteLine("php -q generator\\generator.php -l win32\\logs\\config_pango.log -o ext\\gtk+\\pango.overrides -p pango ext\\gtk+\\pango.defs > ext\\gtk+\\gen_pango.c");
	temp.WriteLine("php -q generator\\generator.php -l win32\\logs\\config_gdk.log -o ext\\gtk+\\gdk.overrides -p gdk ext\\gtk+\\gdk.defs > ext\\gtk+\\gen_gdk.c");
	temp.WriteLine("php -q generator\\generator.php -l win32\\logs\\config_gtk.log -o ext\\gtk+\\gtk.overrides -p gtk -r ext\\gtk+\\gdk.defs ext\\gtk+\\gtk.defs > ext\\gtk+\\gen_gtk.c");
	temp.WriteLine('grep -h "^PHP_GTK_EXPORT_CE" ext\\gtk+\\gen_atk.c | sed -e "s/^/PHP_GTK_API extern /" > ext\\gtk+\\gen_atk.h');
	temp.WriteLine('grep -h "^PHP_GTK_EXPORT_CE" ext\\gtk+\\gen_pango.c | sed -e "s/^/PHP_GTK_API extern /" > ext\\gtk+\\gen_pango.h');
	temp.WriteLine('grep -h "^PHP_GTK_EXPORT_CE" ext\\gtk+\\gen_gdk.c | sed -e "s/^/PHP_GTK_API extern /" > ext\\gtk+\\gen_gdk.h');
	temp.WriteLine('grep -h "^PHP_GTK_EXPORT_CE" ext\\gtk+\\gen_gtk.c | sed -e "s/^/PHP_GTK_API extern /" > ext\\gtk+\\gen_gtk.h');
	return;
}

function Module_Item(module_name, config_path, dir_line, deps, content) {

	this.module_name = module_name;
	this.config_path = config_path;
	this.dir_line = dir_line;
	this.deps = deps;
	this.content = content;
}

function gen_modules() {

	var module_names = (new VBArray(MODULES.Keys())).toArray();
	var i, mod_name, j;
	var item;
	var output = "";

	// first, look for modules with empty deps; emit those first
	for (i in module_names) {
		mod_name = module_names[i];
		item = MODULES.Item(mod_name);
		if (item.deps.length == 0) {
			MODULES.Remove(mod_name);
			output += emit_module(item);
		}
	}

	// now we are left with modules that have dependencies on other modules
	module_names = (new VBArray(MODULES.Keys())).toArray();
	output += emit_dep_modules(module_names);

	return output;
}

function emit_module(item) {

	return item.dir_line + item.content;
}

function emit_dep_modules(module_names) {

	var i, mod_name, j;
	var output = "";
	var item = null;

	for (i in module_names) {
		mod_name = module_names[i];

		if (MODULES.Exists(mod_name)) {
			item = MODULES.Item(mod_name);
			MODULES.Remove(mod_name);
			if (item.deps.length) {
				output += emit_dep_modules(item.deps);
			}
			output += emit_module(item);
		}
	}

	return output;
}

function find_config_w32(dirname) {

	if (!FSO.FolderExists(dirname)) {
		return;
	}

	var f = FSO.GetFolder(dirname);
	var	fc = new Enumerator(f.SubFolders);
	var c, i, ok, n;
	var item = null;
	var re_dep_line = new RegExp("ADD_EXTENSION_DEP\\([^,]*\\s*,\\s*['\"]([^'\"]+)['\"].*\\);", "gm");
	
	for (; !fc.atEnd(); fc.moveNext()) {
		ok = true;
		/* check if we already picked up a module with the same dirname;
		 * if we have, don't include it here */
		n = FSO.GetFileName(fc.item());
		
		if (n == 'CVS' || n == 'tests')
			continue;
			
		WScript.StdOut.Write("Checking " + dirname + "/" + n);
		if (MODULES.Exists(n)) {
			WScript.StdOut.WriteLine("Skipping " + dirname + "/" + n + " -- already have a module with that name");
			continue;
		}

			
		c = FSO.BuildPath(fc.item(), "config.w32");

		if (!FSO.FileExists(c)) {

			WScript.StdOut.WriteLine("		Not currently available for Windows systems");

		} else {

			WScript.StdOut.WriteLine("		Available");

			var dir_line = "configure_module_dirname = condense_path(FSO.GetParentFolderName('"
							   	+ c.replace(new RegExp('(["\\\\])', "g"), '\\$1') + "'));\r\n";
			var contents = file_get_contents(c);
			var deps = new Array();

			// parse out any deps from the file
			var calls = contents.match(re_dep_line);

			if (calls != null) {
				for (i = 0; i < calls.length; i++) {
					// now we need the extension name out of this thing
					if (calls[i].match(re_dep_line)) {
						deps[deps.length] = RegExp.$1;
					}
				}
			}

			item = new Module_Item(n, c, dir_line, deps, contents);
			MODULES.Add(n, item);
		}
	}
}

/* buildconf should clean up generated files */
if (FSO.FileExists('configure.bat')) {
	FSO.DeleteFile('configure.bat');
}

if (FSO.FolderExists('Release')) {
	FSO.DeleteFolder('Release');
}

if (FSO.FileExists('main\\php_gtk_version.h')) {
	FSO.DeleteFile('main\\php_gtk_version.h');
}

if (FSO.FileExists('Makefile')) {
	FSO.DeleteFile('Makefile');
}

iter = new Enumerator(dir.Files);
name = "";
for (; !iter.atEnd(); iter.moveNext()) {
	name = FSO.GetFileName(iter.item());
	if (name.match(new RegExp("gen_")) || name.match(new RegExp(".cache"))) {
		FSO.DeleteFile(iter.item());
	}
}

gen_functions();

// Write the head of the configure script
C.WriteLine("/* This file is automatically generated from win32/confutils.js */");
C.Write(file_get_contents("win32/confutils.js"));

// Pull in code from extensions
modules = file_get_contents("win32/config.w32.in");

// Pick up confs from extensions if present
find_config_w32("ext");

/* Now generate contents of module based on MODULES, chasing dependencies to
ensure that dependent modules are emitted first */
modules += gen_modules();

// Look for ARG_ENABLE or ARG_WITH calls
re = new RegExp("(ARG_(ENABLE|WITH)\([^;]+\);)", "gm");
calls = modules.match(re);
for (i = 0; i < calls.length; i++) {
	item = calls[i];
	C.WriteLine("try {");
	C.WriteLine(item);
	C.WriteLine("} catch (e) {");
	C.WriteLine('\tSTDOUT.WriteLine("problem: " + e);');
	C.WriteLine("}");
}

C.WriteBlankLines(1);
C.WriteLine("conf_process_args();");
C.WriteBlankLines(1);

// Comment out the calls from their original positions
modules = modules.replace(re, "/* $1 */");
C.Write(modules);

C.WriteBlankLines(1);
C.Write("generate_files();");

WScript.StdOut.WriteBlankLines(1);
WScript.StdOut.WriteLine("Now run 'cscript /nologo configure.js --help'");
