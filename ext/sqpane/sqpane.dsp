# Microsoft Developer Studio Project File - Name="sqpane" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sqpane - Win32 Debug_TS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sqpane.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sqpane.mak" CFG="sqpane - Win32 Debug_TS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sqpane - Win32 Debug_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sqpane - Win32 Release_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sqpane - Win32 Debug_TS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_TS"
# PROP BASE Intermediate_Dir "Debug_TS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_TS"
# PROP Intermediate_Dir "Debug_TS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SQPANE_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "../../" /I "../../main" /I "../../../php4" /I "../../../php4/main" /I "../../../php4\Zend" /I "../../../php4\TSRM" /I "../../../src/gtk+" /I "../../../src/gtk+/gdk" /I "../../../src/glib" /I "../../../src/glib/glib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SQPANE_EXPORTS" /D "PHP_WIN32" /D "ZEND_WIN32" /D ZTS=1 /D ZEND_DEBUG=1 /D HAVE_PHP_GTK=1 /D GTK_SHARED=1 /D HAVE_SQPANE=1 /D "PHP_GTK_COMPILE_DL_SQPANE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 php4ts_debug.lib php_gtk.lib gtk-1.3.lib gdk-1.3.lib glib-1.3.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../win32/Debug_TS/php_gtk_sqpane.dll" /pdbtype:sept /libpath:"../../../php4/Debug_TS" /libpath:"../../win32/Debug_TS" /libpath:"../../../src/glib" /libpath:"../../../src/gtk+/gtk" /libpath:"../../../src/gtk+/gdk"

!ELSEIF  "$(CFG)" == "sqpane - Win32 Release_TS"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SQPANE_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "../../" /I "../../main" /I "../../../php4" /I "../../../php4/main" /I "../../../php4\Zend" /I "../../../php4\TSRM" /I "../../../src/gtk+" /I "../../../src/gtk+/gdk" /I "../../../src/glib" /I "../../../src/glib/glib" /I "../../../src/pango" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SQPANE_EXPORTS" /D "PHP_WIN32" /D "ZEND_WIN32" /D ZTS=1 /D ZEND_DEBUG=0 /D HAVE_PHP_GTK=1 /D GTK_SHARED=1 /D HAVE_SQPANE=1 /D "PHP_GTK_COMPILE_DL_SQPANE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 php4ts.lib php_gtk.lib gtk-1.3.lib gdk-1.3.lib glib-1.3.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../win32/Release_TS/php_gtk_sqpane.dll" /libpath:"../../../php4/Release_TS" /libpath:"../../win32/Release_TS" /libpath:"../../../src/glib" /libpath:"../../../src/gtk+/gtk" /libpath:"../../../src/gtk+/gdk"

!ENDIF 

# Begin Target

# Name "sqpane - Win32 Debug_TS"
# Name "sqpane - Win32 Release_TS"

# Begin Group "Parsers"
# PROP Default_Filter "defs"

# Begin Source File

SOURCE=.\sqpane.defs

!IF  "$(CFG)" == "sqpane - Win32 Release_TS"

USERDEP__LIBGL="sqpane.overrides"	"sqpane.defs"	
# Begin Custom Build
InputPath=.\sqpane.defs

"gen_sqpane.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\..\generator\generator.php -o ext\sqpane\sqpane.overrides -p sqpane -r ext\gtk%%2b\gtk.defs ext\sqpane\sqpane.defs >gen_sqpane.c 
	grep -h "^PHP_GTK_EXPORT_CE" gen_sqpane.c | sed -e "s/^/extern /" > gen_ce_sqpane.h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sqpane - Win32 Debug_TS"

USERDEP__LIBGL="sqpane.overrides"	"sqpane.defs"	
# Begin Custom Build
InputPath=.\sqpane.defs

"gen_sqpane.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\..\generator\generator.php -o ext\sqpane\sqpane.overrides -p sqpane -r ext\gtk%%2b\gtk.defs ext\sqpane\sqpane.defs >gen_sqpane.c 
	grep -h "^PHP_GTK_EXPORT_CE" gen_sqpane.c | sed -e "s/^/extern /" > gen_ce_sqpane.h 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gen_sqpane.c
# End Source File
# Begin Source File

SOURCE=.\gtksqpane.c
# End Source File
# Begin Source File

SOURCE=.\php_sqpane.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gen_ce_sqpane.h
# End Source File
# Begin Source File

SOURCE=.\php_sqpane.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Override Files"

# PROP Default_Filter "*.override"
# Begin Source File

SOURCE=..\ext\sqpane\sqpane.overrides
# End Source File
# End Group
# End Target
# End Project
