dnl $Id$
dnl config.m4 for scintilla module

define(gthread_required_version, 2.6.0)

PHP_GTK_ARG_ENABLE(scintilla,for scintilla support,
[  --enable-scintilla          Enable Scintilla support],no)

if test "$PHP_GTK_SCINTILLA" != "no"; then
  AC_DEFINE(HAVE_SCINTILLA, 1, [scintilla support])
  AM_PATH_GLIB_2_0(gthread_required_version,,[AC_MSG_ERROR(The scintilla extension requires GThread gthread_required_version or higher)],gthread)
  PHP_EVAL_INCLINE($GLIB_CFLAGS)
  PHP_EVAL_LIBLINE($GLIB_LIBS -lstdc++, PHP_GTK2_SHARED_LIBADD)
  PHP_REQUIRE_CXX()

  scintilla_lib_srcs="libscintilla/src/AutoComplete.cxx \
libscintilla/src/CallTip.cxx \
libscintilla/src/CellBuffer.cxx \
libscintilla/src/CharClassify.cxx \
libscintilla/src/ContractionState.cxx \
libscintilla/src/Document.cxx \
libscintilla/src/DocumentAccessor.cxx \
libscintilla/src/Editor.cxx \
libscintilla/src/ExternalLexer.cxx \
libscintilla/src/Indicator.cxx \
libscintilla/src/KeyMap.cxx \
libscintilla/src/KeyWords.cxx \
libscintilla/src/LexAPDL.cxx \
libscintilla/src/LexAU3.cxx \
libscintilla/src/LexAVE.cxx \
libscintilla/src/LexAda.cxx \
libscintilla/src/LexAsm.cxx \
libscintilla/src/LexAsn1.cxx \
libscintilla/src/LexBaan.cxx \
libscintilla/src/LexBash.cxx \
libscintilla/src/LexBasic.cxx \
libscintilla/src/LexBullant.cxx \
libscintilla/src/LexCLW.cxx \
libscintilla/src/LexCPP.cxx \
libscintilla/src/LexCSS.cxx \
libscintilla/src/LexCaml.cxx \
libscintilla/src/LexCmake.cxx \
libscintilla/src/LexConf.cxx \
libscintilla/src/LexCrontab.cxx \
libscintilla/src/LexCsound.cxx \
libscintilla/src/LexD.cxx \
libscintilla/src/LexEScript.cxx \
libscintilla/src/LexEiffel.cxx \
libscintilla/src/LexErlang.cxx \
libscintilla/src/LexFlagship.cxx \
libscintilla/src/LexForth.cxx \
libscintilla/src/LexFortran.cxx \
libscintilla/src/LexGui4Cli.cxx \
libscintilla/src/LexHTML.cxx \
libscintilla/src/LexHaskell.cxx \
libscintilla/src/LexInno.cxx \
libscintilla/src/LexKix.cxx \
libscintilla/src/LexLisp.cxx \
libscintilla/src/LexLout.cxx \
libscintilla/src/LexLua.cxx \
libscintilla/src/LexMMIXAL.cxx \
libscintilla/src/LexMPT.cxx \
libscintilla/src/LexMSSQL.cxx \
libscintilla/src/LexMatlab.cxx \
libscintilla/src/LexMetapost.cxx \
libscintilla/src/LexNsis.cxx \
libscintilla/src/LexOpal.cxx \
libscintilla/src/LexOthers.cxx \
libscintilla/src/LexPB.cxx \
libscintilla/src/LexPOV.cxx \
libscintilla/src/LexPS.cxx \
libscintilla/src/LexPascal.cxx \
libscintilla/src/LexPerl.cxx \
libscintilla/src/LexPython.cxx \
libscintilla/src/LexRebol.cxx \
libscintilla/src/LexRuby.cxx \
libscintilla/src/LexSQL.cxx \
libscintilla/src/LexScriptol.cxx \
libscintilla/src/LexSmalltalk.cxx \
libscintilla/src/LexSpecman.cxx \
libscintilla/src/LexSpice.cxx \
libscintilla/src/LexTADS3.cxx \
libscintilla/src/LexTCL.cxx \
libscintilla/src/LexTeX.cxx \
libscintilla/src/LexVB.cxx \
libscintilla/src/LexVHDL.cxx \
libscintilla/src/LexVerilog.cxx \
libscintilla/src/LexYAML.cxx \
libscintilla/src/LineMarker.cxx \
libscintilla/src/PropSet.cxx \
libscintilla/src/RESearch.cxx \
libscintilla/src/ScintillaBase.cxx \
libscintilla/src/Style.cxx \
libscintilla/src/StyleContext.cxx \
libscintilla/src/UniConversion.cxx \
libscintilla/src/ViewStyle.cxx \
libscintilla/src/WindowAccessor.cxx \
libscintilla/src/XPM.cxx"

  scintilla_srcs="libscintilla/PlatGTK.cxx \
libscintilla/ScintillaGTK.cxx"

  PHP_GTK_EXTENSION(scintilla, $php_gtk_ext_shared, $scintilla_lib_srcs $scintilla_srcs php_scintilla.c libscintilla/scintilla-marshal.c, gen_scintilla.c, -I@ext_srcdir@ -I@ext_srcdir@/libscintilla -I@ext_srcdir@/libscintilla/include -I@ext_srcdir@/libscintilla/src)
  PHP_ADD_BUILD_DIR($ext_builddir/libscintilla/src)
fi
