dnl $Id$
dnl config.m4 for spell module

define(spell_required_version,   2.0.0)

PHP_GTK_ARG_WITH(spell,for spell support,
[  --with-spell            Enable GtkSpell support],no)

if test "$PHP_GTK_SPELL" != "no"; then
  PKG_CHECK_MODULES(SPELL, [gtkspell-2.0 >= spell_required_version],
  have_spell=yes, have_spell=no)
  if test "$have_spell" != "yes"; then
    AC_MSG_RESULT([Unable to locate spell version spell_required_version or higher: not building])
  else
    AC_DEFINE(HAVE_SPELL, 1, [spell support])
    PHP_EVAL_INCLINE($SPELL_CFLAGS)
    if test "$php_gtk_ext_shared" = "yes"; then
      PHP_EVAL_LIBLINE($SPELL_LIBS, PHP_GTK2_SPELL_SHARED_LIBADD)
      PHP_SUBST(PHP_GTK2_SPELL_SHARED_LIBADD)
    else
      PHP_EVAL_LIBLINE($SPELL_LIBS, PHP_GTK2_SHARED_LIBADD)
    fi
    PHP_GTK_EXTENSION(spell, $php_gtk_ext_shared, php_spell.c)
  fi
fi
