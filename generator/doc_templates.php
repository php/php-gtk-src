<?php
/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001 Andrei Zmievski <andrei@php.net>
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
  </shortdesc>
  <desc>
   <para>
   </para>
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
   </shortdesc>
   <desc>
    <para>
    </para>
   </desc>
  </method>

EOD;

$constructor_start_tpl = <<<EOD

 <constructor id="%s.%s.constructor">
  <funcsynopsis>
   <funcprototype>

EOD;

$constructor_end_tpl = <<<EOD
   </funcprototype>
  </funcsynopsis>
  <shortdesc>
  </shortdesc>
  <desc>
   <para>
   </para>
  </desc>
 </constructor>

EOD;

$funcproto_tpl = <<<EOD
     <funcdef>%s <function>%s</function></funcdef>
%s
EOD;

$parameter_tpl = <<<EOD
     <paramdef>%s <parameter>%s</parameter></paramdef>

EOD;

$opt_parameter_tpl = <<<EOD
     <paramdef>%s <parameter><optional>%s = %s<optional></parameter></paramdef>

EOD;

$no_parameter_tpl = <<<EOD
     <paramdef>%s</paramdef>

EOD;

?>
