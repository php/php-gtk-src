<?php
/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001-2008 Andrei Zmievski <andrei@php.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* $Id$ */

$class_start_tpl = <<<EOD
<classentry id="%s.%s">
 <classmeta>
  <classtitle>%s</classtitle>
  <classparent>%s</classparent>
  <shortdesc>
%s
  </shortdesc>
  <desc>
%s
  </desc>
 </classmeta>

EOD;

$class_end_tpl = <<<EOD

</classentry>

EOD;

$methods_start_tpl = <<<EOD

 <methods>

EOD;

$methods_end_tpl = <<<EOD

 </methods>

EOD;

$method_start_tpl = <<<EOD

  <method id="%s.%s.method.%s">
   <funcsynopsis>
    <funcprototype>

EOD;

$method_func_start_tpl = <<<EOD

  <method id="%s.method.%s">
   <funcsynopsis>
    <funcprototype>

EOD;

$method_end_tpl = <<<EOD
    </funcprototype>
   </funcsynopsis>
   <shortdesc>
%s
   </shortdesc>
   <desc>
%s
   </desc>
  </method>

EOD;

$constructors_start_tpl = <<<EOD

 <constructors>

EOD;

$constructors_end_tpl = <<<EOD

 </constructors>

EOD;

$constructor_start_tpl = <<<EOD

  <constructor id="%s.%s.constructor">
   <funcsynopsis>
    <funcprototype>

EOD;

$constructor_alt_start_tpl = <<<EOD

  <constructor id="%s.%s.constructor.%s">
   <funcsynopsis>
    <funcprototype>

EOD;

$constructor_end_tpl = <<<EOD
    </funcprototype>
   </funcsynopsis>
   <shortdesc>
%s
   </shortdesc>
   <desc>
%s
   </desc>
  </constructor>

EOD;

$funcproto_tpl = <<<EOD
     <funcdef>%s <function>%s</function></funcdef>
%s
EOD;

$cbfuncproto_tpl = <<<EOD
     <funcdef>%s <cbfunction>callback</cbfunction></funcdef>
%s
EOD;

$parameter_tpl = <<<EOD
     <paramdef>%s <parameter>%s</parameter></paramdef>

EOD;

$opt_parameter_tpl = <<<EOD
     <paramdef>%s <parameter><optional>%s = %s</optional></parameter></paramdef>

EOD;

$no_parameter_tpl = <<<EOD
     <paramdef>%s</paramdef>

EOD;

$props_start_tpl= <<<EOD

 <properties>

EOD;

$props_end_tpl= <<<EOD

 </properties>

EOD;

$prop_start_tpl = <<<EOD

  <property id="%s.%s.property.%s" type="ro">
   <propname>%s</propname>
   <proptype>%s</proptype>

EOD;

$prop_end_tpl = <<<EOD
   <shortdesc>
   </shortdesc>
   <desc>
%s
   </desc>
  </property>

EOD;

$signals_start_tpl= <<<EOD

 <signals>

EOD;

$signals_end_tpl= <<<EOD

 </signals>

EOD;

$signal_start_tpl = <<<EOD

  <signal id="%s.%s.signal.%s">
   <signalname>%s</signalname>
   <funcsynopsis>
    <funcprototype>

EOD;

$signal_end_tpl = <<<EOD
    </funcprototype>
   </funcsynopsis>
   <shortdesc>
   </shortdesc>
   <desc>

   </desc>
  </signal>
  
EOD;

$etter_start_tpl = <<<EOD
    <para>
     &seealso;
EOD;

$etter_end_tpl = <<<EOD

    </para>

EOD;

$etter_link = <<<EOD

     <function class="%s">%s</function>
EOD;


$prop_link = <<<EOD

     <propname class="%s">%s</propname>
EOD;

$refentry_start_tpl = <<<EOD

<refentry id="%s.functions">
 <refmeta>
   <refentrytitle>%s Functions</refentrytitle>
   <shortdesc>
     %s functions not belonging to any class.
   </shortdesc>
 </refmeta>

EOD;

$refentry_end_tpl = <<<EOD

</refentry>

EOD;


$enums_start = <<<EOD
<?xml version="1.0" encoding="utf-8" ?>
<enums id="%senum">

EOD;

$enums_end = <<<EOD
</enums>

EOD;

$enum_start = <<<EOD


 <enum id="%s.enum.%s">
  <enumname>%s%s</enumname>
  <desc>
   <simpara>
    
   </simpara>
  </desc>

EOD;

$enum_end = <<<EOD
 </enum>

EOD;

$enum_option = <<<EOD
  <enumoption>
   <value>%d</value>
   <optionname>%s::%s</optionname>
   <shortdesc>
    
   </shortdesc>
  </enumoption>

EOD;


/* vim: set et sts=4: */
?>
