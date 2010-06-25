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
    const MAX = 65000;
    protected $filename = null;
    protected $line     = array();
    protected $header = '';
    protected $body = array();
    protected $current = 0;

    function LineOutput($filename)
    {
        $this->body[0] = '';
        $this->filename = $filename;
        $this->line[0] = 0;
    }

    function get_total()
    {
        return count($this->line);
    }

    function get_current()
    {
        return $this->current;
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
        $this->set_line($this->line[$this->current] + 1, $this->filename);
    }

    function save()
    {
        $files = '';
        foreach($this->body as $id => $data) {
            if ($id == 0) {
                $string = '';
            } else {
                $string =  '-' . $id;
            }
            $filename = substr($this->filename, 0, -2) . $string . '.c';
            $files .= ' ' . basename($filename);
            
            $header = $this->header;
            
			// this is a cheeky fix for the link sysem failing on MacOS when these
			// files get broken into multiple files.
			// bob - 20100625			
			if(PHP_OS == "Darwin" && $id > 0) {
				$header = preg_replace('/^PHP_GTK_EXPORT_CE\(.+?\);$/ms','',$header);
			}
            
            file_put_contents($filename, $header . $this->body[$id]);
        }
        file_put_contents('php://stdout', $files, FILE_APPEND);
    }

    function write($string)
    {
        $this->body[$this->current] .= $string;
        $this->line[$this->current] += substr_count($string, "\n");
    }

    function write_header($string)
    {
        $this->header .= $string;
        $increase = substr_count($string, "\n");
        foreach($this->line as $id => $value) {
            $this->line[$id] += $increase;
        }
    }

    function write_all($string)
    {
        foreach($this->body as $id => $value) {
            $this->body[$id] .= $string;
            $this->line[$id] += substr_count($string, "\n");
        }
    }

    function check_size()
    {
        if ($this->line[$this->current] >= self::MAX) {
            return true;
        }
        return false;
    }

    function new_file()
    {
        $this->current++;
        $this->body[$this->current] = '';
        $this->line[$this->current] = 0;
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
