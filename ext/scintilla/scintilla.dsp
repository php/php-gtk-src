# Microsoft Developer Studio Project File - Name="scintilla" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=scintilla - Win32 Debug_TS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scintilla.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scintilla.mak" CFG="scintilla - Win32 Debug_TS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scintilla - Win32 Debug_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "scintilla - Win32 Release_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scintilla - Win32 Debug_TS"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCINTILLA_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "..\..\\" /I "..\..\main" /I "..\..\..\php4" /I "..\..\..\php4\main" /I "..\..\..\php4\Zend" /I "..\..\..\php4\TSRM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCINTILLA_EXPORTS" /D "PHP_WIN32" /D "ZEND_WIN32" /D ZTS=1 /D ZEND_DEBUG=1 /D HAVE_PHP_GTK=1 /D GTK_SHARED=1 /D HAVE_SCINTILLA=1 /D "PHP_GTK_COMPILE_DL_SCINTILLA" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 gtkscintilla.lib php4ts_debug.lib php_gtk.lib gtk.lib gdk.lib glib-2.0.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../win32/Debug_TS/php_gtk_scintilla.dll" /pdbtype:sept /libpath:"..\..\..\php4\Debug_TS" /libpath:"..\gtk+\Debug_TS"

!ELSEIF  "$(CFG)" == "scintilla - Win32 Release_TS"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCINTILLA_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "..\..\\" /I "..\..\main" /I "..\..\..\php4" /I "..\..\..\php4\main" /I "..\..\..\php4\Zend" /I "..\..\..\php4\TSRM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCINTILLA_EXPORTS" /D "PHP_WIN32" /D "ZEND_WIN32" /D ZTS=1 /D ZEND_DEBUG=0 /D HAVE_PHP_GTK=1 /D GTK_SHARED=1 /D HAVE_SCINTILLA=1 /D "PHP_GTK_COMPILE_DL_SCINTILLA" /D INCLUDE_DEPRECATED_FEATURES=1 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 gtkscintilla.lib php4ts.lib php_gtk.lib gtk.lib gdk.lib glib-2.0.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../win32/Release_TS/php_gtk_scintilla.dll" /libpath:"..\..\..\php4\Release_TS" /libpath:"..\gtk+\Release_TS"

!ENDIF 

# Begin Target

# Name "scintilla - Win32 Debug_TS"
# Name "scintilla - Win32 Release_TS"
# Begin Group "Parsers"

# PROP Default_Filter "defs"
# Begin Source File

SOURCE=.\scintilla.defs

!IF  "$(CFG)" == "scintilla - Win32 Debug_TS"

USERDEP__SCINT="scintilla.overrides"	"scintilla.defs"	
# Begin Custom Build
InputPath=.\scintilla.defs

"gen_scintilla.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\..\generator\generator.php -o ext\scintilla\scintilla.overrides -p scintilla -r ext\gtk%%2b\gtk.defs ext\scintilla\scintilla.defs >gen_scintilla.c 
	grep -h "^PHP_GTK_EXPORT_CE" gen_scintilla.c | sed -e "s/^/extern /" > gen_ce_scintilla.h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "scintilla - Win32 Release_TS"

USERDEP__SCINT="scintilla.overrides"	"scintilla.defs"	
# Begin Custom Build
InputPath=.\scintilla.defs

"gen_scintilla.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\..\generator\generator.php -o ext\scintilla\scintilla.overrides -p scintilla -r ext\gtk%%2b\gtk.defs ext\scintilla\scintilla.defs >gen_scintilla.c 
	grep -h "^PHP_GTK_EXPORT_CE" gen_scintilla.c | sed -e "s/^/extern /" > gen_ce_scintilla.h 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gen_scintilla.c
# End Source File
# Begin Source File

SOURCE=.\php_scintilla.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gen_ce_scintilla.h
# End Source File
# Begin Source File

SOURCE=.\php_scintilla.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Override Files"

# PROP Default_Filter "*.override"
# Begin Source File

SOURCE=.\scintilla.overrides
# End Source File
# End Group
# End Target
# End Project
