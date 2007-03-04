dnl $Id$
dnl config.m4 for html module

define(html_required_version,   3.10.0)

PHP_GTK_ARG_ENABLE(html,for html support,
[  --enable-html     Enable GtkHtml support],no)

if test "$PHP_GTK_HTML" != "no"; then
  PKG_CHECK_MODULES(HTML, [libgtkhtml-3.8 >= html_required_version],
  have_html=yes, have_html=no)
  if test "$have_html" != "yes"; then
    AC_MSG_RESULT([Unable to locate libgtkhtml version html_required_version or higher: not building])
  else
    AC_DEFINE(HAVE_HTML, 1, [html support])
    PHP_EVAL_INCLINE($HTML_CFLAGS)
    if test "$php_gtk_ext_shared" = "yes"; then
      PHP_EVAL_LIBLINE($HTML_LIBS, HTML_SHARED_LIBADD)
      PHP_SUBST(HTML_SHARED_LIBADD)
    else
      PHP_EVAL_LIBLINE($HTML_LIBS, PHP_GTK2_SHARED_LIBADD)
    fi
    PHP_GTK_EXTENSION(html, $php_gtk_ext_shared, php_html.c, gen_html.c)
  fi
fi
