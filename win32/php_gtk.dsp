# Microsoft Developer Studio Project File - Name="php_gtk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=php_gtk - Win32 Debug_TS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "php_gtk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "php_gtk.mak" CFG="php_gtk - Win32 Debug_TS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "php_gtk - Win32 Release_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "php_gtk - Win32 Debug_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "php_gtk - Win32 Release_TS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_TS"
# PROP BASE Intermediate_Dir "Release_TS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_TS"
# PROP Intermediate_Dir "Release_TS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_GTK_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "..\\" /I "..\main" /I "..\..\php4" /I "..\..\php4\main" /I "..\..\php4\Zend" /I "..\..\php4\TSRM" /I "..\..\src\gtk+" /I "..\..\src\gtk+\gdk" /I "..\..\src\glib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_GTK_EXPORTS" /D "PHP_WIN32" /D "ZEND_WIN32" /D ZTS=1 /D ZEND_DEBUG=0 /D HAVE_PHP_GTK=1 /D "COMPILE_DL_PHP_GTK" /YX /FD /I /php4" /I /php4/main" /I /php4/Zend" /I /php4/TSRM" /I /src/gtk+" /I /src/gtk+/gdk" /I /src/glib" /I /php4" /I /php4/main" /I /php4/Zend" /I /php4/TSRM" /I /src/gtk+" /I /src/gtk+/gdk" /I /src/glib" " " " " " " " " " " " " " " /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 php4ts.lib gtk-1.3.lib gdk-1.3.lib glib-1.3.lib gnu-intl.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /libpath:"..\..\php4\Release_TS" /libpath:"..\..\src\glib" /libpath:"..\..\src\gtk+\gtk" /libpath:"..\..\src\gtk+\gdk" /libpath:"..\..\src\intl"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "php_gtk - Win32 Debug_TS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_TS"
# PROP BASE Intermediate_Dir "Debug_TS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_TS"
# PROP Intermediate_Dir "Debug_TS"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_GTK_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "..\\" /I "..\main" /I "..\..\php4" /I "..\..\php4\main" /I "..\..\php4\Zend" /I "..\..\php4\TSRM" /I "..\..\src\gtk+" /I "..\..\src\gtk+\gdk" /I "..\..\src\glib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_GTK_EXPORTS" /D "PHP_WIN32" /D "ZEND_WIN32" /D ZTS=1 /D ZEND_DEBUG=1 /D HAVE_PHP_GTK=1 /D "COMPILE_DL_PHP_GTK" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 php4ts_debug.lib gtk-1.3.lib glib-1.3.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\php4\Debug_TS" /libpath:"..\..\src\glib" /libpath:"..\..\src\gtk+\gtk"

!ENDIF 

# Begin Target

# Name "php_gtk - Win32 Release_TS"
# Name "php_gtk - Win32 Debug_TS"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\ext\gtk+\php_gdk.c"
# End Source File
# Begin Source File

SOURCE=..\main\php_gtk.c
# End Source File
# Begin Source File

SOURCE=..\main\php_gtk_ext.w32.c
# End Source File
# Begin Source File

SOURCE=..\main\php_gtk_object.c
# End Source File
# Begin Source File

SOURCE=..\main\php_gtk_util.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\main\php_gtk.h
# End Source File
# Begin Source File

SOURCE=..\main\php_gtk_module.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Override Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\generator\gdk.defs
# End Source File
# Begin Source File

SOURCE=..\generator\gdk.overrides
# End Source File
# Begin Source File

SOURCE="..\generator\gtk-extrafuncs.defs"
# End Source File
# Begin Source File

SOURCE=..\generator\gtk.defs
# End Source File
# Begin Source File

SOURCE=..\generator\gtk.overrides
# End Source File
# Begin Source File

SOURCE=..\generator\libglade.defs
# End Source File
# Begin Source File

SOURCE=..\generator\libglade.overrides
# End Source File
# End Group
# Begin Group "Gtk+"

# PROP Default_Filter ""
# Begin Group "Source Files No. 1"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE="..\ext\gtk+\gen_ce_gtk.h"
# End Source File
# Begin Source File

SOURCE="..\ext\gtk+\gen_gdk.c"
# End Source File
# Begin Source File

SOURCE="..\ext\gtk+\gen_gtk.c"
# End Source File
# Begin Source File

SOURCE="..\ext\gtk+\php_gtk+.c"
# End Source File
# Begin Source File

SOURCE="..\ext\gtk+\php_gtk+_types.c"
# End Source File
# End Group
# Begin Group "Header Files No. 1"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE="..\ext\gtk+\gen_gtk.h"
# End Source File
# Begin Source File

SOURCE="..\ext\gtk+\php_gtk+.h"
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\README.txt
# End Source File
# End Target
# End Project
