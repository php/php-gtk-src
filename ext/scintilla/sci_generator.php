<?php
$row = 0;
require dirname(__FILE__) . "/../../generator/Getopt.php";
require dirname(__FILE__) . "/../../generator/lineoutput.php";

function fixFunctionName($name){
	$len = strlen($name);
	$r = '';
	$name = str_replace('BackSpace','Backspace',$name); //fix set_back_space_unindents
	$name = str_replace('UnIndents','Unindents',$name); //fix set_back_space_unindents
	$name = str_replace('EOL','Eol',$name); //fix set_eol_mode
	$name = str_replace('KeyWords','Keywords',$name); //fix set_keywords
	$state = 'start';
	for($x=0;$x<$len;$x++){
		$char = $name[$x];
		switch($state){
			case 'start':
				if($char >='A' && $char <= 'Z'){
					$r .= ($x)? '_'.strtolower($char): strtolower($char);
					$state = 'name';
				}else{
					$r .= $char;
				}
			break;
			case 'name':
				if($char > 'Z'){
					$r .= $char;
					$state = 'start';
				}else{
					$r .= strtolower($char);
				}
			break;
		}
	}
	return $r;
}

$result = Console_Getopt::getopt($argv, 'i:');
if (!$result || count($result[1]) < 1)
    die("usage: php sci_generator.php -i scintilla source directory");

list($opts, $argv) = $result;
$path = '';
foreach ($opts as $opt) {
	list($opt_spec, $opt_arg) = $opt;
	if ($opt_spec == 'i') {
			$path = $opt_arg;
	}
}
$savefile = dirname(__FILE__) . "/gen_scintilla.c";

if($sfp = new LineOutput(fopen($savefile, 'w'), $savefile)){
	if($handle = fopen("ext/scintilla/scintilla.in", "r")){
		while (1 && ($data = fgets($handle, 1000)) !== FALSE){
			$sfp->write($data);
		}
	}
	$sfp->write("\n/*Start of generated code*/\n\n");
	$const = array();
	if($handle = fopen("$path/include/Scintilla.h", "r")){
		while (1 && ($data = fgets($handle, 1000)) !== FALSE){
			preg_match('!\#define\s(SCI_\w+)\s([-]*\d+)!is',$data,$match);
			$const[$match[2]] = $match[1];
		}
		fclose($handle);
	}
	
	if($handle = fopen("$path/include/Scintilla.iface", "r")){
		while (1 && ($data = fgets($handle, 1000)) !== FALSE){
			//echo $data;
			if(preg_match('!cat Deprecated!is',$data)) break;
			if(empty($data) || substr($data,0,1)=='#'){
				continue;
			}
			//$cmd = substr($data,0,3);
			$bits = split(' ',$data);
			//print_r($bits);
			switch($bits[0]){
				case 'val':
					$def = split('=',$bits[1]);
					$np = split('_',$def[0]);
					if(!isset($starts[$np[0]])) $starts[$np[0]] = 1;
					switch($np[0]){
						case 'SCI':
							continue;
						break;
						case 'SC':
							unset($np[0]);
						break;
						case 'VISIBLE':
						case 'CARET':
						case 'KEYWORDSET':
						case 'EDGE':
						case 'INDICS':
						case 'INDIC2':
						case 'INDIC1':
						case 'INDIC0':
						case 'INDIC':
						case 'STYLE':
						case 'MARKER':
						case 'INVALID':
							
						break;
						case 'SCWS':
						case 'SCFIND':
						case 'SCEN':
						case 'SCMOD':
						case 'SCLEX':
						case 'SCN':
							$np[0] = str_replace('SC','',$np[0]);
						break;
						case 'SCE':
							unset($np[0]);
							switch($np[1]){
								case 'P':$np[1] = 'PY';break;
								case 'H':$np[1] = 'HTML';break;
								case 'HJ':$np[1] = 'HTML_JS';break;
								case 'HJA':$np[1] = 'HTML_JS_ASP';break;
								case 'HB':$np[1] = 'HTML_VB';break;
								case 'HBA':$np[1] = 'HTML_VB_ASP';break;
								case 'HP':$np[1] = 'HTML_PY';break;
								case 'HPA':$np[1] = 'HTML_PY_ASP';break;
								case 'HPHP':$np[1] = 'HTML_PHP';break;
								case 'PL':$np[1] = 'PEARL';break;
								case 'B':$np[1] = 'VB';break;
								case 'RB':$np[1] = 'RUBY';break;
								case 'L':$np[1] = 'LATEX';break;
								case 'F':$np[1] = 'FORTRAN';break;
								case 'V':$np[1] = 'VERILOG';break;
							}
						break;
						case 'SCK':
							$np[0] = 'KEY';
						break;
					}
					$sc = 'SCINTILLA_'.implode('_',$np);
					$vl = trim($def[0]);
					$constants[$sc] = $vl;
					//echo "found define\n";
				break;
				case 'fun':
				case 'set':
				case 'get':
					preg_match('!'.$bits[0].'\s(\w+)\s(\w+)=(\d+)\((.*?)\)!',$data,$match);
					$fname = fixFunctionName($match[2]);
					//print_r($match);
					$vars = split(',',$match[4]);
					//print_r($vars);
					$functions[$fname]['return'] = $match[1];
					$functions[$fname]['message'] = $match[3];
					$functions[$fname]['decl'] = array();
					$functions[$fname]['pre'] = array();
					$functions[$fname]['post'] = array();
					$x = 0;
					foreach($vars as $var){
						$v = split(' ',trim($var));
						if(empty($v[0])){
							$functions[$fname]['vars'][$x]['org_type'] = 'void';
							$functions[$fname]['vars'][$x]['type'] = 'void';
							$functions[$fname]['vars'][$x]['name'] = '';
						}else{
							$type = trim($v[0]);
							$name = trim($v[1]);
							switch($type){
								case 'colour':
								case 'keymod':
								case 'bool':
								case 'int':
								case 'position':
									$rtype = 'long';
								break;
								case 'findtext':
								case 'cells':
								case 'string':
									$rtype = 'char';
								break;
								default:
									$rtype = $type;
								break;
							}
							$functions[$fname]['vars'][$x]['org_type'] = $type;
							$functions[$fname]['vars'][$x]['type'] = $rtype;
							$functions[$fname]['vars'][$x]['name'] = $name;
						}
						
						$x++;
						//print_r($var);						
					}
					$arginfo[$fname]['name'] = "arginfo_scintilla_$fname";
				break;
			}
			$row++;
		}
		fclose($handle);
		//print_r($starts);
		$c = 0;
		$call = 'SCINTILLA_MESSAGE(';
		foreach($functions as $fname=>$data){
			//if($fname == 'set_back_space_unindents') print_r($functions[$fname]);
			//if($c == 10) break;
			//echo "$fname\n";
			$org1 = $data['vars'][0]['org_type'];
			$org2 = $data['vars'][1]['org_type'];
			$type1 = $data['vars'][0]['type'];
			$type2 = $data['vars'][1]['type'];
			$name1 = $data['vars'][0]['name'];
			$name2 = $data['vars'][1]['name'];
			$message = $const[$functions[$fname]['message']];
			switch($fname){
				case 'set_caret_policy':
				case 'format_range':
					continue; //skip these
				break;
				case 'get_text_range':
					//returns selected text with terminating 0
					$functions[$fname]['decl'][] = "long start, end, length;";
					$functions[$fname]['decl'][] = "char *text;";
					$functions[$fname]['decl'][] = "struct  TextRange tr = {{0, 0}, 0};";
					$functions[$fname]['chk1'][] = "i";
					$functions[$fname]['chk2'][] = "&start";
					$functions[$fname]['chk1'][] = "i";
					$functions[$fname]['chk2'][] = "&end";
					$functions[$fname]['pre'][] = "length = end - start + 1;";
					$functions[$fname]['pre'][] = "if(length > 1) {";
					$functions[$fname]['pre'][] = "\ttext = emalloc(length);";
					$functions[$fname]['pre'][] = "\ttr.chrg.cpMin = start;";
					$functions[$fname]['pre'][] = "\ttr.chrg.cpMax = end;";
					$functions[$fname]['pre'][] = "\ttr.lpstrText = text;";
					$functions[$fname]['call'] = "\tSCINTILLA_MESSAGE($message, 0, (sptr_t) &tr);";
					$functions[$fname]['post'][] = "\tRETVAL_STRINGL(text, length-1, 1);";
					$functions[$fname]['post'][] = "\tefree(text);";
					$functions[$fname]['post'][] = "} else {";
					$functions[$fname]['post'][] = "\tRETURN_EMPTY_STRING();\n\t}";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, start)";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, end)";
				break;
				case 'find_text':
					$functions[$fname]['decl'][] = "long flags, chrg_min=0, chrg_max=0, retval;";
					$functions[$fname]['decl'][] = "char *text;";
					$functions[$fname]['decl'][] = "struct TextToFind ft = {{0, 0}, 0, {0, 0}};";
					$functions[$fname]['chk1'][] = "i";
					$functions[$fname]['chk2'][] = "&flags";
					$functions[$fname]['chk1'][] = "s";
					$functions[$fname]['chk2'][] = "&text";
					$functions[$fname]['chk1'][] = "|i";
					$functions[$fname]['chk2'][] = "&chrg_min";
					$functions[$fname]['chk1'][] = "i";
					$functions[$fname]['chk2'][] = "&chrg_max";
					$functions[$fname]['pre'][] = "if(!chrg_min && !chrg_max)\n\t\tchrg_max = SCINTILLA_MESSAGE(SCI_GETLENGTH, 0, 0);";
					$functions[$fname]['pre'][] = "ft.chrg.cpMin = chrg_min;";
					$functions[$fname]['pre'][] = "ft.chrg.cpMax = chrg_max;";
					$functions[$fname]['pre'][] = "ft.lpstrText = text;";
					$functions[$fname]['call'] = "retval = SCINTILLA_MESSAGE($message, (int) flags, (sptr_t) &ft);";
					$functions[$fname]['post'][] = "RETURN_LONG(retval);";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, flags)";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, text)";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, chrg_min)";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, chrg_max)";
				break;
				case 'add_text':
					$functions[$fname]['decl'][] = "long doc_len, length=0;";
					$functions[$fname]['decl'][] = "char *text;";
					$functions[$fname]['chk1'][] = "s";
					$functions[$fname]['chk2'][] = "&text";
					$functions[$fname]['chk1'][] = "|i";
					$functions[$fname]['chk2'][] = "&length";
					$functions[$fname]['pre'][] = "doc_len = strlen(text);";
					$functions[$fname]['pre'][] = "if(!length || length > doc_len)\n\t\tlength = doc_len;";
					$functions[$fname]['call'] = "SCINTILLA_MESSAGE($message, (int) length, (sptr_t) text);";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, text)";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, length)";
				break;
				case 'get_property':
				case 'get_property_expanded':
					//returns exact length of buffer no terminating 0
					$functions[$fname]['decl'][] = "long length=0;";
					$functions[$fname]['decl'][] = "char *key, *value;";
					$functions[$fname]['chk1'][] = "s";
					$functions[$fname]['chk2'][] = "&key";
					$functions[$fname]['pre'][] = "length = SCINTILLA_MESSAGE($message, (sptr_t) key, 0);";
					$functions[$fname]['pre'][] = "value = emalloc(length);";
					$functions[$fname]['call'] = "length = SCINTILLA_MESSAGE($message, (sptr_t) key, (sptr_t) value);";
					$functions[$fname]['post'][] = "RETVAL_STRINGL(value, length, 1);";
					$functions[$fname]['post'][] = "efree(value);";
					$arginfo[$fname]['vars'][0] = "ZEND_ARG_INFO(0, key)";
				break;
				case 'get_line':
					//returns line with line ending no terminating 0
					$functions[$fname]['decl'][] = "long line, length=0;";
					$functions[$fname]['decl'][] = "char *text;";
					$functions[$fname]['chk1'][] = "i";
					$functions[$fname]['chk2'][] = "&line";
					$functions[$fname]['pre'][] = "length = SCINTILLA_MESSAGE($message, (int) line, 0);";
					$functions[$fname]['pre'][] = "text = emalloc(length);";
					$functions[$fname]['call'] = "SCINTILLA_MESSAGE($message, (int) line, (sptr_t) text);";
					$functions[$fname]['post'][] = "RETVAL_STRINGL(text, length, 1);";
					$functions[$fname]['post'][] = "efree(text);";
					$arginfo[$fname]['vars'][0] = "ZEND_ARG_INFO(0, line)";
				break;
				case 'get_text':
					//reurns length-1 + 0
					$functions[$fname]['decl'][] = "long doc_len, length=0;";
					$functions[$fname]['decl'][] = "char *text;";
					$functions[$fname]['chk1'][] = "|i";
					$functions[$fname]['chk2'][] = "&length";
					$functions[$fname]['pre'][] = "doc_len = SCINTILLA_MESSAGE(SCI_GETLENGTH, 0, 0);";
					$functions[$fname]['pre'][] = "if(!length || length > doc_len)\n\t\t length = doc_len;";
					$functions[$fname]['pre'][] = "length ++;";
					$functions[$fname]['pre'][] = "text = emalloc(length);";
					$functions[$fname]['call'] = "SCINTILLA_MESSAGE($message, (int) length, (sptr_t) text);";
					$functions[$fname]['post'][] = "RETVAL_STRINGL(text, length-1, 1);";
					$functions[$fname]['post'][] = "efree(text);";
					$arginfo[$fname]['vars'][0] = "ZEND_ARG_INFO(0, length)";
				break;
				case 'get_cur_line':
				case 'get_sel_text':
					//returns length of selection with terminating 0
					$functions[$fname]['decl'][] = "long length=0;";
					$functions[$fname]['decl'][] = "char *text;";
					$functions[$fname]['pre'][] = "length = SCINTILLA_MESSAGE($message, 0, 0);";
					$functions[$fname]['pre'][] = "if(length > 1) {";
					$functions[$fname]['pre'][] = "\ttext = emalloc(length);";
					$functions[$fname]['call'] = "\tSCINTILLA_MESSAGE($message, (int) length, (sptr_t) text);";
					$functions[$fname]['post'][] = "\tRETVAL_STRINGL(text, length-1, 1);";
					$functions[$fname]['post'][] = "\tefree(text);";
					$functions[$fname]['post'][] = "} else {";
					$functions[$fname]['post'][] = "\tRETURN_EMPTY_STRING();\n\t}";
				break;
				case 'target_as_utf8':
					//returns exact length of buffer no terminating 0
					$functions[$fname]['decl'][] = "long length=0;";
					$functions[$fname]['decl'][] = "char *text;";
					$functions[$fname]['pre'][] = "length = SCINTILLA_MESSAGE($message, 0, 0);";
					$functions[$fname]['pre'][] = "if(length) {";
					$functions[$fname]['pre'][] = "\ttext = emalloc(length);";
					$functions[$fname]['call'] = "\tSCINTILLA_MESSAGE($message, 0, (sptr_t) text);";
					$functions[$fname]['post'][] = "\tRETVAL_STRINGL(text, length, 1);";
					$functions[$fname]['post'][] = "\tefree(text);";
					$functions[$fname]['post'][] = "} else {";
					$functions[$fname]['post'][] = "\tRETURN_EMPTY_STRING();\n\t}";
				break;
				case 'encoded_from_utf8':
					//returns exact length of buffer no terminating 0
					$functions[$fname]['decl'][] = "long length=0;";
					$functions[$fname]['decl'][] = "char *text_in, *text_out;";
					$functions[$fname]['chk1'][] = "s";
					$functions[$fname]['chk2'][] = "&text_in";
					$functions[$fname]['pre'][] = "length = SCINTILLA_MESSAGE($message, (sptr_t) text_in, 0);";
					$functions[$fname]['pre'][] = "if(length) {";
					$functions[$fname]['pre'][] = "\ttext_out = emalloc(length);";
					$functions[$fname]['call'] = "\tSCINTILLA_MESSAGE($message, (sptr_t) text_in, (sptr_t) text_out);";
					$functions[$fname]['post'][] = "\tRETVAL_STRINGL(text_out, length, 1);";
					$functions[$fname]['post'][] = "\tefree(text_out);";
					$functions[$fname]['post'][] = "} else {";
					$functions[$fname]['post'][] = "\tRETURN_EMPTY_STRING();\n\t}";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, text_in)";
				break;
				case 'get_styled_text':
					//returns selected text with 2 terminating 0
					$functions[$fname]['decl'][] = "long start, end, length;";
					$functions[$fname]['decl'][] = "char *text;";
					$functions[$fname]['decl'][] = "struct  TextRange tr = {{0, 0}, 0};";
					$functions[$fname]['chk1'][] = "i";
					$functions[$fname]['chk2'][] = "&start";
					$functions[$fname]['chk1'][] = "i";
					$functions[$fname]['chk2'][] = "&end";
					$functions[$fname]['pre'][] = "length = ((end - start) * 2) + 2;";
					$functions[$fname]['pre'][] = "if(length > 1) {";
					$functions[$fname]['pre'][] = "\ttext = emalloc(length);";
					$functions[$fname]['pre'][] = "\ttr.chrg.cpMin = start;";
					$functions[$fname]['pre'][] = "\ttr.chrg.cpMax = end;";
					$functions[$fname]['pre'][] = "\ttr.lpstrText = text;";
					$functions[$fname]['call'] = "\tSCINTILLA_MESSAGE($message, 0, (sptr_t) &tr);";
					$functions[$fname]['post'][] = "\tRETVAL_STRINGL(text, length-2, 1);";
					$functions[$fname]['post'][] = "\tefree(text);";
					$functions[$fname]['post'][] = "} else {";
					$functions[$fname]['post'][] = "\tRETURN_EMPTY_STRING();\n\t}";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, start)";
					$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, end)";
				break;
				default:
					switch($functions[$fname]['return']){
						case 'void':
							$functions[$fname]['call'] = $call;
						break;
						default:
							$functions[$fname]['call'] = "retval = $call";
						break;
					}
					if($type1 == $type2){
						switch($type1){
							case 'void':
								$functions[$fname]['call'] .= $message.', 0, 0);';
							break;
							case 'long':
								$functions[$fname]['decl'][] = "long $name1, $name2;\n";
								$functions[$fname]['chk1'][] = "i";
								$functions[$fname]['chk2'][] = "&$name1";
								$functions[$fname]['chk1'][] = "i";
								$functions[$fname]['chk2'][] = "&$name2";
								$functions[$fname]['call'] .= $message.", (int) $name1, (int) $name2);";
							break;
							case 'char':
								$functions[$fname]['decl'][] = "char *$name1, *$name2;";
								$functions[$fname]['chk1'][] = "s";
								$functions[$fname]['chk2'][] = "&$name1";
								$functions[$fname]['chk1'][] = "s";
								$functions[$fname]['chk2'][] = "&$name2";
								$functions[$fname]['call'] .= $message.", (sptr_t) $name1, (sptr_t) $name2);";
							break;
							default:
								if(!isset($typelist[$org1])) $typelist[3][$org1] = 1;
							break;
						}
						if($type1 != 'void'){
							$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, $name1)";
							$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, $name2)";
						}
					}else{
						$done = false;
						switch($type1){
							case 'void':
								$functions[$fname]['call'] .= $message.', 0, ';
							break;
							case 'long':
								$functions[$fname]['decl'][] = "long $name1;";
								$functions[$fname]['chk1'][] = "i";
								$functions[$fname]['chk2'][] = "&$name1";
								$arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, $name1)";
								$functions[$fname]['call'] .= $message.", (int) $name1, ";
							break;
							case 'char':
								$functions[$fname]['decl'][] = "char *$name1;";
								$functions[$fname]['chk1'][] = "s";
								$functions[$fname]['chk2'][] = "&$name1";
								$functions[$fname]['call'] .= $message.", (sptr_t) $name1, ";
							break;
							default:
								$functions[$fname]['call'] .= $message.", 0, ";
								if(!isset($typelist[1][$org1])) $typelist[1][$org1] = $fname;
							break;
						}
						if(!$done){
							switch($type2){
								case 'void':
									$functions[$fname]['call'] .= "0);";
								break;
								case 'long':
									$functions[$fname]['decl'][] = "long $name2;";
									$functions[$fname]['chk1'][] = "i";
									$functions[$fname]['chk2'][] = "&$name2";
									$functions[$fname]['call'] .= "(int) $name2);";
								break;
								case 'char':
									$functions[$fname]['decl'][] = "char *$name2;";
									$functions[$fname]['chk1'][] = "s";
									$functions[$fname]['chk2'][] = "&$name2";
									$functions[$fname]['call'] .= "(sptr_t) $name2);";
								break;
	
								default:
									$functions[$fname]['call'] .= "0);";
									if(!isset($typelist[2][$org2])) $typelist[2][$org2] = $fname;
								break;
							}
							if($type2 != 'void') $arginfo[$fname]['vars'][] = "ZEND_ARG_INFO(0, $name2)";
						}
					}
					$return = $functions[$fname]['return'];
					switch($return){
						case 'void':
							
						break;
						case 'int':
						case 'position':
						case 'colour':
							$functions[$fname]['decl'][] = "long retval;";
							$functions[$fname]['post'][] = "RETURN_LONG(retval);";
						break;
						case 'bool':
							$functions[$fname]['decl'][] = "long retval;";
							$functions[$fname]['post'][] = "RETURN_LONG(retval);";
						break;
						default:
							if(!isset($typelist[3][$return])) $typelist[2][$return] = $fname;
						break;
					}
				break;
			}
			if(isset($arginfo[$fname]['vars']) && count($arginfo[$fname]['vars'])>0){
				$function_entry[$fname] = "PHP_ME(Scintilla, $fname,";
				for($x=strlen($fname);$x<30;$x++){
					$functions[$fname]['function_entry'] .= ' ';
				}
				$function_entry[$fname] .= "arginfo_scintilla_$fname, ZEND_ACC_PUBLIC)";
				
			}else{
				$function_entry[$fname] = "PHP_ME(Scintilla, $fname, NULL, ZEND_ACC_PUBLIC)";
			}
			$sfp->write("static\nPHP_METHOD(Scintilla, $fname)\n{\n");
			foreach($functions[$fname]['decl'] as $decl){
				$sfp->write("\t$decl\n");
			}
			$sfp->write("\n\tNOT_STATIC_METHOD();\n\n");
			
			$chkcount = count($functions[$fname]['chk1']);
			if($chkcount){
				$chk1 = '';
				$chk2 = '';
				for($i=0;$i<$chkcount;$i++){
					$chk1 .= $functions[$fname]['chk1'][$i];
					$chk2 .= (empty($chk2))? $functions[$fname]['chk2'][$i]:', '.$functions[$fname]['chk2'][$i];
				}
				$sfp->write("\tif(!php_gtk_parse_args(ZEND_NUM_ARGS(), \"$chk1\", $chk2))\n\t\treturn;\n");
			}else{
				$sfp->write("\tif(!php_gtk_parse_args(ZEND_NUM_ARGS(), \"\"))\n\t\treturn;\n");
			}
			if(count($functions[$fname]['pre'])>0){
				$sfp->write("\n");
				foreach($functions[$fname]['pre'] as $pre){
					$sfp->write("\t$pre\n");
				}
			}
			$sfp->write("\n\t".$functions[$fname]['call']."\n");
			if(count($functions[$fname]['post'])> 0){
				$sfp->write("\n");
				foreach($functions[$fname]['post'] as $post){
					$sfp->write("\t$post\n");
				}
			}
			$sfp->write("}\n\n");
			//print_r($functions[$fname]);
			$c++;
		}
		foreach($arginfo as $info){
			if(isset($info['vars']) && count($info['vars'])>0){
				$name = $info['name'];
				$sfp->write("\nstatic\nZEND_BEGIN_ARG_INFO($name, 0)\n");
				foreach($info['vars'] as $var){
					$sfp->write("\t$var\n");
				}
				$sfp->write("ZEND_END_ARG_INFO();\n");
			}
		}
		$sfp->write("\nstatic function_entry scintilla_methods[] = {\n\tPHP_ME(Scintilla, __construct, NULL, ZEND_ACC_PUBLIC)\n");
		foreach($function_entry as $entry){
			$sfp->write("\t$entry\n");
		}
		$sfp->write("\t{ NULL, NULL, NULL }\n};\n");
		
		$sfp->write("void phpg_scintilla_register_classes(void)\n{\n\tTSRMLS_FETCH();\n\n\tscintilla_ce = phpg_register_class(\"GtkScintilla\", scintilla_methods, gtkcontainer_ce, 0, NULL, NULL, GTK_TYPE_SCINTILLA TSRMLS_CC);\n}\n\n");
		$sfp->write("void phpg_scintilla_register_constants(const char *strip_prefix)\n{\n\tTSRMLS_FETCH();\n\n");
		$sfp->write("\tphpg_register_int_constant(scintilla_ce, \"gtype\", sizeof(\"gtype\")-1, GTK_TYPE_FRAME);\n");
		foreach($constants as $const=>$val){
			$sfp->write("\tphpg_register_int_constant(scintilla_ce, \"$const\", sizeof(\"$const\")-1, $val);\n");
		}
		$sfp->write("};\n");
		$sfp->write("\n#endif /* HAVE_SCINTILLA */\n");
		//print_r($const);
		print_r($typelist);  //missing types
		//print_r($function_entry);
		//print_r($arginfo);
	}
}
