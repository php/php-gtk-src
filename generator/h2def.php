#!/usr/bin/php -q
<?php

error_reporting(E_ALL);
# ------------------ Find  definitions -----------------
require_once 'Console/Getopt.php';

class PHP_CGenerator_h2def {
    var $structs = array();
    var $objdefs = array();
    var $obj_name_pat = "[A-Z][a-z]+[A-Z][A-Za-z0-9]*";

    //Try to find object definitions in header files.
    var $classOcase = array(); // map of lcase names ot originalcase names.

    function find_objs($buf ) {
        $objdefs = array();
        # filter out comments from buffer.
        
        # first find all structures that look like they may represent a GtkObject
        
        $found = array();
        preg_match_all("/struct _([A-Z][a-z]+[A-Z][A-Za-z0-9]*)\s*{\s*" .
                         "([A-Z][a-z]*[A-Z][A-Za-z0-9]*)\s+/", $buf, $matches,PREG_SET_ORDER);
        
        
        $maybeobjdefs = array() ; # contains all possible objects from file
         
        foreach($matches as $m) {
            // base => parent...
            $found[$m[1]] = $m[2];
        }
        
       
        //$maybeobjdefs[] = array((m.group(1), m.group(2)))
          
        $matches = array();
        # now find all structures that look like they might represent a class:
        preg_match_all("/struct _([A-Z][a-z]+[A-Z][A-Za-z0-9]*)Class\s*{\s*" .
                        "([A-Z][a-z]*[A-Z][A-Za-z0-9]*)Class\s+/", $buf, $matches,PREG_SET_ORDER); 
        //print_r($found);
        foreach($matches as $m) {
            //print_r($m);
            if (isset($found[$m[1]]))  {
                $this->objdefs[$m[1]] = $found[$m[1]];
            }
        }
        
           
          
        
        $matches = array();
        preg_match_all(
            "/struct\s+_([A-Z][a-z]+[A-Z][A-Za-z0-9]*)\s*{([^}]+)};/", 
            $buf, $matches,PREG_SET_ORDER);
            foreach($matches as $m) {
            // base => parent...
            if (preg_match('/Class$/',$m[1])) {
                continue;
            }
            if (!isset($this->objdefs[$m[1]])) {
                $this->structs[$m[1]] = $m[2];
            }
        }
        
        
        
        
        if ($this->debug['o']) {
            print_r($this->objdefs);
        }
        
        if ($this->objdefs) {
            foreach($this->objdefs as $k=>$data) {
                $this->classOcase[strtolower($k)] = $k;
            }
        }
        foreach($this->structs as $k=>$data) {
            $this->classOcase[strtolower($k)] = $k;
        }
        
        // if it's in both - add it to objdefs;
        
    }
        
    function sort_obj_defs($objdefs) {
        return $objdefs;
            //objdefs.sort()  # not strictly needed, but looks nice
            /*pos = 0
            while pos < len(objdefs):
                klass,parent = objdefs[pos]
                for i in range(pos+1, len(objdefs)):
                    # parent below subclass ... reorder
                    if objdefs[i][0] == parent:
                        objdefs.insert(i+1, objdefs[pos])
                        del objdefs[pos]
                        break
                else:
                    pos = pos + 1
            return objdefs
        */
    }
    

    var $objdefs_regex = '';
    function write_objs($fh) {
        if (!$this->objdefs) {
            return;
        }
        fwrite($fh,";; -*- scheme -*-\n");
        fwrite($fh,"; object definitions ...\n");
        $this->objdefs_regex  = '';
        foreach($this->objdefs as $class=>$parent) {
            $cmodule = '';
            $pmodule = '';
            if (substr($class, 0,strlen($this->base))) {
                $cname = substr($class,strlen($this->base));
                $cmodule = substr($class,0,strlen($this->base));
            }
            if (substr($parent, 0,strlen($this->base))) {
                $pname = substr($parent,strlen($this->base));
                $pmodule = substr($parent,0,strlen($this->base));
            }
            if ($parent == 'GObject') {
                $pname = 'GObject';
                $pmodule = '';
            }
            
             
            fwrite($fh,"(object $cname \n");
            if ($cmodule) {
                fwrite($fh,"  (in-module $cmodule)\n");
            }
            if ($parent) {
                fwrite($fh,"  (parent $pname");
                if ($pmodule) {
                    fwrite($fh," ($pmodule)");
                }
                fwrite($fh,")\n");
            }
            fwrite($fh,"  (c-name $class )\n");
            # should do something about accessible fields
            fwrite($fh,")\n\n");
            
            
            $this->objdefs_regex .= $class.'|';
        }
        $this->objdefs_regex = substr($this->objdefs_regex,0,-1);
    }

    /*
        (struct Rectangle
          (in-module Gdk)
          (c-name GdkRectangle)
          (field (type-and-name gint16 x))
          (field (type-and-name gint16 y))
          (field (type-and-name guint16 width))
          (field (type-and-name guint16 height))
        )
    */

    function write_structs($fh) {
        if (!$this->structs) {
            return;
        }
        fwrite($fh,"; struct definitions ...\n");
        foreach($this->structs as $name=>$body) {
            // skip structs that are collections of function pointers..
            if (strpos($body,'(') !== false) {
                continue;
            }
                        
            if (substr($name, 0,strlen($this->base))) {
                $cname = substr($name,strlen($this->base));
                $cmodule = substr($name,0,strlen($this->base));
            } else {
                echo "unable to find cname";
                exit;
            }
            
            fwrite($fh,"(struct $cname \n");
            fwrite($fh,"  (in-module $cmodule)\n");
            fwrite($fh,"  (c-name $name)\n");
            
            // now the members..
            $matches = array();
            $lines = explode(';', $body);
            foreach ($lines as $l) {
                if (!strlen(trim($l))) {
                    continue;
                }
                $linebits[] = preg_split('/\s+/',trim($l));
            }
            //preg_match_all('/\s+((\S+)\s+(\S+))[^;]*;/',$body,$matches,PREG_SET_ORDER);
            foreach($linebits as $m) {
                fwrite($fh,"  (field (type-and-name {$m[0]} {$m[1]}))\n");
            } 
            fwrite($fh,")\n");
            
    
    
        }
    
    
    }
    
    
    
    
    
    
    # ------------------ Find enum definitions -----------------
    var $enums = array();
    
    function find_enums($buf) {
        # strip comments
        # bulk comments
        //pat = re.compile(r"""/[*](.|\n)*?[*]/""", re.MULTILINE)
        //buf=pat.sub('',buf)
    
        //buf = re.sub('\n', ' ', buf)
            /* curl hack - add other hacks here? */
        //$buf = preg_replace('/CINIT\((\w+).*$/im','CURLOPT_\\1, ',$buf);
        //$buf = preg_replace('/CFINIT\((\w+).*$/im','CURLFORM_\\1, ',$buf);
        
        
        if (!preg_match_all('/enum\s*{([^}]*)}\s*([A-Za-z_]*)(\s|;)/',$buf,$enums,PREG_SET_ORDER)) {
            return;
        }
         
        $ret = array();
        
            
        
        foreach($enums as $e) {
            // $e[1] = XXXX = 1,.....
            // $e[2] = GtkEnumType..
            //
            $lines = explode(",",$e[1]);
            
            
            
            $a = array();
            foreach($lines as $l) {
                
            
                if ($p = strpos($l, '=')) {
                    $l = substr($l,0,$p);
                }
                if (!strlen(trim($l))) {
                    continue;
                }
                $a[] = trim($l);
            }
            $ret[$e[2]]= $a;
        }
        //print_r($this->enums);
        $this->enums = array_merge($this->enums,$ret); 
    }
    function write_enums($fh) {
        fwrite($fh,";; Enumerations and flags ...\n\n");
        
         
        
        //echo "ENUM MODULE? $this->enumModule\n";        exit;
        
        foreach($this->enums as $e=>$a) {
            $ee = substr($e,strlen($this->enumModule));
            fwrite($fh,"(enum $ee\n");
            if ($this->enumModule) {
                fwrite($fh,"  (in-module $this->enumModule)\n");
            }
            if ($e) {
                fwrite($fh,"  (c-name $e)\n");
            }
            
             
            $lens = array();
            foreach(array_keys($a) as $i) {
                if (!$a[$i]) continue;
                $lens[strlen($a[$i])] = $a[$i];
            }
            ksort($lens);
            //print_r($lens);
            $test = array_values($lens); 
            //print_r($test);
            $l = 0;
            for ($i=0;$i<strlen($test[0]);$i++) {
                
                //echo "TEST $i: ". $test[0]{$i} .'!='. $test[1]{$i}."\n";
                if ($test[0]{$i} == '_') {
                    $l=$i;
                }
                if ($test[0]{$i} != @$test[1]{$i}) {
                    break;
                }
            }
            $shortstart = $l+1;
            
            
            
            foreach($a as $en) {
            
                 
                $short = strtolower(substr($en,$shortstart) );
                fwrite($fh,"  (value (name $short) (c-name $en))\n");
            }
            # should do something about accessible fields
            fwrite($fh,")\n\n");
        }
    }
            
         
    
    # ------------------ Find function definitions -----------------
    
    #comment_pat = re.compile(r"""(/[*](.|\n)*?[*]/)|(^;.*$)""", re.MULTILINE)
    
    function clean_func($buf) {
    
    }
        
                   
    var $funcs = array();
    var $regex = 0; // which regex to use..
    function find_func($buf) {
        $funcs = explode (';',$buf);
        //print_R($bits);
        $ret = array();
        foreach($funcs as $test) {
            $test = trim($test);
            if (strpos($test,'(')===false) {
                continue;
            }
            $split = explode('(',$test);
            
           // print_R($split);
            if (count($split == 2)) {
                list($left,$args) = $split;
            } // otherwise do what!!!!
            
            //print_r($split);
            $leftsplit = preg_split('/\s+/',trim($left));
            //print_r($leftsplit);
            
            
            $name = array_pop($leftsplit);
            $retval = array_pop($leftsplit);
            while ($name{0} == '*') {
                $retval.='*';
                $name = substr($name,1);
            }
            // if they stuck a space in it...
            if ($retval == '*') {
            	$retval = array_pop($leftsplit) . '*';
            }
            
            
            // ignore privates!
            if ($name{0} == '_') {
                continue;
            }
            // now args...
            $args = substr($args,0,strrpos($args,')'));
            
            // now explode on the commas...
            $args = explode(',',$args);
           // print_r($args);
            $newargs = array();
            foreach($args as $arg) {
                $arg = trim($arg);
                $right = '';
                $argbits =  preg_split('/\s+/',$arg);
                switch(count($argbits)) {
                    case 1:
                        $newargs[] = $argbits[0];
                        continue 2;    //???  right level?
                    case 2:
                        list($left,$right) = $argbits;
                        break;
                    case 3:
                        $left = "{$argbits[0]}-{$argbits[1]}";
                        $right = $argbits[2];
                        break;
                    default:
                        // oops dont know how to handle this!!!
                        error_log('oops too many paramenters: ' .count($argbits).' ' .$arg);
                        continue 3;
                }
                 
                // move all the **'s from right to left..
                while ($right{0} == '*') {
                    $left.='*';
                    $right = substr($right,1);
                }
                $newargs[] = "{$left} {$right}";
                
            }
          //  echo "ARGS:".print_r($newargs); 
            $ret[] = array($retval,$name,$newargs);
            
            
            
        }
        //print_R($ret);
        $this->funcs = array_merge($ret,$this->funcs );
        return;
        print_r($ret);
        
        exit;
        
        $ret = array();
        
        switch ($this->regex) {
            case 1:
                preg_match_all("/\n\s*(\w*)\s+(\w*)\s+\(([^)]*)\)\s*\(([^)]*)\)/i",$buf,$funcs,PREG_SET_ORDER); 
                if ($this->debug['f']) {
                    echo "\n\n parsed funcs\n"; print_r($funcs);
                }
                break;
            case 2:
                preg_match_all("/\n\s*([^ \t]*)(\s+)([^ \t]*)\s+\(([^)]*)\)/i",$buf,$funcs,PREG_SET_ORDER); 
                if ($this->debug['f']) {
                    echo "\n\n regex2\n"; print_r($funcs);
                }    
                break;
            case 3:
                preg_match_all("/\n\s*([^ \t]+)(\s+)([^ \t]+)\s*\(([^)]*)\)/i",$buf,$funcs,PREG_SET_ORDER); 
                if ($this->debug['f']) {
                    echo "\n\n regex3\n"; print_r($funcs);
                }
                break;
                
            default:
                // \n (space*) (word) (space) (word) (space) ( (not closebraket) 
                preg_match_all("/\n\s*(\w*)(\s+)(\w+)\s*\(([^)]*)\)/i",$buf,$funcs,PREG_SET_ORDER); 
                //preg_match_all("/\n\s*(\w*)(\s+)(\w+)\s*\(([^)]*)\)/i",$buf,$funcs,PREG_SET_ORDER); 
                if ($this->debug['f']) {
                    echo "\n\n regex1\n"; print_r($funcs);
                } 
                break;
                
        }
        if (!$funcs) {
            echo ";No function found you may what to try using the generator with the option -r 0|1|2|3 \n";
        }
        
        //print_R($funcs);
        
        
        foreach ($funcs as $def) {
            // has it got a return value?
            if ($this->debug['f']) {
                echo "CHECK:";print_r($def);
            }
            $name = trim($def[3]);
            $retval = trim($def[1]);
            if (!preg_match('/[a-z]+/i', $retval)) { 
                if ($this->debug['f']) {
                    echo "SKIP: invalid return\n";
                }
                continue;
            }
            
            if (trim($def[2]) != '') {  // for stuff like typedefs
                $retval = $def[2];
            } 
            // has it got  a function name ?
            if (!preg_match('/[a-z]+/i', $name)) { 
                if ($this->debug['f']) {
                    echo "SKIP: invalid name\n";
                }
                continue;
            }
            
            
            if ($name{0} == '*') {
                $retval .= '*';
                $name = substr($name,1);
            }
            // ignore privates!
            if ($name{0} == '_') {
                continue;
            }
            
            
            // replace const with const- in args
            $def[4] = preg_replace('/const\s*/','const-',$def[4]);
            $def[4] = preg_replace("/\n/"," ",$def[4]);
            $args_base = explode(",", $def[4]);
            $args = array();
            foreach ($args_base as $arg) {
                
                $bits = preg_split('/\s+/',trim($arg));
                //print_r($bits);
                
                if (@$bits[1] == '*') {
                    $bits[1] .= $bits[2];
                }
                
                if (isset($bits[1]) && ($bits[1]{0} == '*')) {
                    $args[] = $bits[0].'*        '. substr($bits[1],1);
                    continue;
                }
                $args[] = trim($arg);
            }
             
            $ret[] = array($retval,$name,$args);
        }
        if ($this->debug['f']) {
             print_r($ret); // ret type
        }
        
        $this->funcs = array_merge($this->funcs,$ret);
    }
       
     
    
    function write_func($fh, $name, $ret, $args) {
       /* doesnt handle methods yet! */
        if ($this->debug['F']) {
            print_r(func_get_args());
        }
        //print_r($args);
	
        $qbits = preg_split('/\s+/',$args[0]);
        $firstType = preg_replace('/[^a-z]/','',strtolower(str_replace('_','',$qbits[0])));
        $callnormalized = str_replace('_','',$name);
        
        //echo "FIRSTARG:$firstType FUNC:$callnormalized\n";
        
             //echo $this->objdefs_regex . '?=' . $args[0] . "\n";
            //if (preg_match('/^(' . $this->objdefs_regex  . ')[\*]*\s/', $args[0])) {
        $returntypeN = strtolower(preg_replace('/[^a-z]*/i','',$ret));
        
        $couldBeConstructorOf =  @strtolower($this->classOcase[
		strtolower(
			preg_replace('/[^a-z]*/i','',
				preg_replace('/_new(_[a-z]+)*$/','',$name)))
		]. '*');
        //echo "CBC:$couldBeConstructorOf \n";
        $isconstructor =
            (preg_match('/_new(_[a-z]+)*$/',$name) &&
                in_array(trim(strtolower($ret)) , array('gtkwidget*','gtkobject*',$couldBeConstructorOf))
            );
            

        
        if (!$isconstructor && substr($callnormalized,0,strlen($firstType)) == $firstType) {
         
            $this->write_method($fh, $name, $ret, $args);
            return;
        }
       
        
        
       
        fwrite($fh,"(function $name \n");
        # do in-module thingee
        fwrite($fh,"  (c-name $name)\n");
         
        //echo "RETTYPE: $returntypeN TEST: " .substr($callnormalized,0,strlen($returntypeN)) . "\n";
    
        if ( $isconstructor) {
            // eg. gtk_dog_new ** return GtkDog

            
            $s =  $this->classOcase[strtolower(preg_replace('/[^a-z]*/i','',preg_replace('/_new(_[a-z]+)*$/','',$name)))];
            if ($s) {
                fwrite($fh,"  (is-constructor-of ".$s.")\n");		
            }
        }  
	
        if ($ret != 'void') {
            fwrite($fh,"  (return-type $ret)\n");
        } else {
        
            fwrite($fh,"  (return-type none)\n");
        }
        foreach($args as $arg) {
            $arg = trim($arg);
        
            if ($arg == '...') {
                fwrite($fh,";  (varargs t)\n");
            } elseif (($arg == 'void') || ($arg == '')) { 
                break;
            } else {
                fwrite($fh,"  (parameter (type-and-name $arg))\n");
            }
        }
        fwrite($fh,")\n\n"); 
    }
     
    function write_method( $fh, $name, $ret, $args) {
       /* doesnt handle methods yet! */
        //print_r(func_get_args());
        $first = array_shift($args);
        $bits = preg_split('/\s+/',trim($first));
        $class = preg_replace('/[^a-z_]*/i','',$bits[0]);
        
        $cmodule = '';
        $mname = $name;
	//echo "TEST : $class - {$this->base}\n";
	// 
        $classOut = $class;
        if (substr($class, 0,strlen($this->base)) == $this->base) {
            $cname = substr($class,strlen($this->base));
            $cmodule = $this->base;
            $fbase = strtolower($cmodule.'_'.$cname.'_');
	    //echo "FBASE = $fbase?\n";
            $name = substr($name,strlen($fbase));
	    if ($name{0} == '_') {
	    	$name = substr($name,1);
	    }
            $classOut = "$cname ($cmodule)";
        }
        // only write methods of objects...
        if (!isset($this->objdefs[$class])) {
            echo "\n;Could not find $cname\n";
            return;
        }
        
        
        fwrite($fh,"(method $name \n");
        fwrite($fh,"  (of-object $classOut)\n");
        # do in-module thingee
        fwrite($fh,"  (c-name $mname)\n");
        if ($ret != 'void') {
            fwrite($fh,"  (return-type $ret)\n");
        } else {
            fwrite($fh,"  (return-type none)\n");
        }
        foreach($args as $arg) {
            $arg = trim($arg);
        
            if ($arg == '...') {
                fwrite($fh," ; (varargs t)\n");
            } elseif (($arg == 'void') || ($arg == '')) { 
                break;
            } else {
                fwrite($fh,"  (parameter (type-and-name $arg))\n");
            }
        }
        fwrite($fh,")\n\n"); 
    }
     
     
     
    /*
    def write_def(input,output=None):
        fp = open(input)
        buf = fp.read()
        fp.close()
    
        if type(output) == types.StringType:
            fp = open(output,'w')
        elif type(output) == types.FileType:
            fp = output
        else:
            fp = sys.stdout
    
        fwrite($fh,"\n;; From %s\n\n' % input)
        buf = define_func(buf, fp)
        fwrite($fh,"\n')
        */
# ------------------ Glue code -----------------
    /* some config stuff */
    
    var $base = 'Gtk';
    //var $enumPrefix = 'GTK_HTML';
    //var $enumModule = 'GtkHTML';
    
    //var $enumPrefix = 'CURLE';
    //var $enumModule = 'CURLE';
    function parseFile($in) {
    
        // set up enumModule !
        //echo "IN : $in\n";
        $dir = basename(dirname($in));
        $this->enumModule = $dir;
        /*
        $module_tmp = str_replace('.h','',basename($in));
        $module_ar = explode('-',$module_tmp);
        $this->enumModule = '';
        foreach($module_ar as $k) {
            $this->enumModule .= ucfirst($k);
        }
        */
    
    
        $buf = file_get_contents($in);
        if (preg_match('/GTK_ENABLE_BROKEN/',$buf)) {
            return;
        }
        /* clean it !*/
        $base = '';
        $n =0;
        // includes and stuff!
        // macros without continuation....
        $buf = preg_replace('|^#.*[^\\\]$|m' ,"",$buf);
        
        // macros with continuation.
        $buf = preg_replace("/\n#([^\n]+\\\\\n)*[^\n]+\n/","xx",$buf);
        
        // Macro's - uppercase stuff..
        $buf = preg_replace('/^[A-Z_][A-Z_ ]+$/m','',$buf);
        // empty lines
        $buf= preg_replace('/\n\s*\n/im',"\n",$buf);	
	
        while (($n !== false ) && ($n < strlen($buf))) {
            $p = strpos($buf,'/*',$n);
            if ($p === false) {
                $base .= substr($buf,$n);
                break;
            }
	    // found it.. add anything before..
	    $add = substr($buf,$n,$p-$n);
	    
	    
	    //echo "ADD $add\n";
            $base .= $add;
            $n = strpos($buf,'*/',$p)+2;
            
        }

        if ($this->debug['p']) {
        
            echo "--------base-----\n";
            echo $base;       
            echo "--------base end-----\n";         
        }
        $buf = $base;
        
        //$buf=preg_replace('/\/\*.*\*+\//is','',$buf);
    
        // do obj bit..
        $this->find_objs($buf);
        
        
        $buf = preg_replace('/struct _\w+\s*\{[^}]+\};/i','',$buf);
        
        $this->find_enums($buf);
        $buf = preg_replace('/typedef[^;]+;/i','',$buf);
        $this->find_func($buf);
    }
    var $debug = false;
    function parseOpts($opts) {
        //var_dump($opts);
        foreach($opts as $k) {
            switch($k[0]) {
                // what to use as a base..
                case 'b':
                    $this->base = $k[1];
                    break;
                // debug // -d fpo  == file/parse/objects
                case 'd':
                    
                    foreach(array('f','p','o','F') as $kk) {
                        $this->debug[$kk]=(false !== strpos($k[1],$kk));
                    }
                    break;
                // which regex to use
                case 'r':
                    $this->regex=$k[1];
                    break;
                
                // predefine objects!! object=parent
                case 'o':    
                    $bits = explode('=',$k[1]);
                    $this->objdefs[$bits[0]] = $bits[1];
                    break;
            }
        }
    
    
    
    }
    
    
    function PHP_CGenerator_h2def() {
    
        
        $args =Console_Getopt::readPHPArgv();
        //array_shift($args);
        $in =  Console_Getopt::getopt($args,"o:r:b:d::");
        $this->parseOpts($in[0]);
        
        $in = $in[1];
        //print_r($in);exit;
        foreach($in as $file) {
            $this->parseFile($file);
        }
        //print_r($this);
        $fh = fopen('php://stdout', 'w');
       
        fwrite($fh,"; from $file\n");
        $this->write_structs($fh);
        $this->write_objs($fh);
        
        $this->write_enums($fh);
        
        
        
        
        //echo $buf;
        
        foreach($this->funcs as $ar) {
            $this->write_func($fh,  $ar[1],$ar[0],$ar[2]);
        }
    }
}

$t = new PHP_CGenerator_h2def;


?>
