@echo off
SET zip_file=php-gtk-%1-win32.zip
md \php-gtk-dist
cd \php-gtk-dist
md php4
md winnt
md test
copy \php4\php.exe php4
copy \php4\php-cgi.exe php4
copy \php4\php_win.exe php4
copy \php4\php4ts.dll php4
copy \php4\php_gtk.dll php4
copy \php4\php_gtk_combobutton.dll php4
copy \php4\php_gtk_libglade.dll php4
copy \php4\php_gtk_scintilla.dll php4
copy \php4\php_gtk_scrollpane.dll php4
copy \php4\php_gtk_spaned.dll php4
copy \php4\php_gtk_sqpane.dll php4
copy \php\php4\php.ini-dist winnt\php.ini
type \php\php-gtk\win32\php.ini >>winnt\php.ini
copy \php\php-gtk\win32\php.ini php4\php.ini-gtk

copy \php4\iconv.dll php4
copy \php4\libgdk-0.dll php4
copy \php4\libglade-0.1.dll php4
copy \php4\libglib-2.0-0.dll php4
copy \php4\libgmodule-2.0-0.dll php4
copy \php4\libgobject-2.0-0.dll php4
copy \php4\libgthread-2.0-0.dll php4
copy \php4\libgtk-0.dll php4
copy \php4\libintl-1.dll php4
copy \php4\libxml2.dll php4

copy \php\php-gtk\test\combobutton.php test
copy \php\php-gtk\test\fileselection.php test
copy \php\php-gtk\test\gtk.php test
copy \php\php-gtk\test\hello.php test
copy \php\php-gtk\test\list.php test
copy \php\php-gtk\test\scribble.php test
copy \php\php-gtk\test\scrollpane.php test
copy \php\php-gtk\test\spaned.php test
copy \php\php-gtk\test\sqpane.php test
copy \php\php-gtk\test\testgtkrc test
copy \php\php-gtk\test\testgtkrc2 test
copy \php\php-gtk\test\window.xpm test
copy \php\php-gtk\win32\README.txt 
copy \php\php-gtk\NEWS
"c:\Program Files\WinZip\WZZip" -ampr %zip_file% 
copy %zip_file% \php\php-gtk-web\distributions
del %zip_file%
cd \php\php-gtk\win32