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

class LineOutput {
    var $fp       = null;
    var $filename = null;
    var $line     = null;

    function LineOutput($fp, $filename)
    {
        $this->fp = $fp;
        $this->filename = $filename;
        $this->line = 1;
    }

    function set_line($line, $filename)
    {
        if (WIN_OS) {
            $this->write(sprintf("//line %d \"%s\"\n", $line, $filename));
        } else {
            $this->write(sprintf("#line %d \"%s\"\n", $line, $filename));
        }
    }

    function reset_line()
    {
        $this->set_line($this->line + 1, $this->filename);
    }

    function write($string)
    {
        fwrite($this->fp, $string);
        $this->line += substr_count($string, "\n");
    }
}

class Coverage {
    var $n_written = 0;
    var $n_skipped = 0;
    var $id        = null;

    function Coverage($id)
    {
        $this->id = $id;
    }

    function written()
    {
        $this->n_written++;
    }

    function skipped()
    {
        $this->n_skipped++;
    }

    function get_stats()
    {
        $total = $this->n_written + $this->n_skipped;
        if ($total) {
            $pcnt_written = ((float)$this->n_written*100.0)/$total;
        } else {
            $pcnt_written = 0.0;
        }
        return array($this->id, $this->n_written, $total, $pcnt_written);
    }
}

/* vim: set et sts=4 fdm=marker: */

?>
