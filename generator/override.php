<?php

class Overrides {
	var $ignores 		= array();
	var $glob_ignores 	= array();
	var $overrides		= array();

	function Overrides($file_name = null)
	{
		if (!isset($file_name))
			return;

		$fp = fopen($file_name, 'r');
		$contents = fread($fp, filesize($file_name));
		fclose($fp);

		$blocks = explode('%%', $contents);
		if (!count($blocks))
			return;

		foreach ($blocks as $block)
			$this->_parse_block(trim($block));
	}

	function _parse_block($block)
	{
		if (strpos($block, "\n"))
			list($line, $rest) = explode("\n", $block, 2);
		else {
			$line = $block;
			$rest = '';
		}
		$words = preg_split('!\s+!', $line, -1, PREG_SPLIT_NO_EMPTY);
		$command = array_shift($words);

		switch ($command) {
			case 'ignore':
				foreach ($words as $func)
					$this->ignores[$func] = true;
				foreach (preg_split('!\s+!', $rest, -1, PREG_SPLIT_NO_EMPTY) as $func)
					$this->ignores[$func] = true;
				break;

			case 'ignore-glob':
				foreach ($words as $func)
					$this->glob_ignores[] = $func;
				foreach (preg_split('!\s+!', $rest, -1, PREG_SPLIT_NO_EMPTY) as $func)
					$this->glob_ignores[] = $func;
				break;

			case 'override':
				list($func_cname, $func_name) = $words;
				$this->overrides[$func_cname] = array($func_name, $rest);
				break;
		}
	}

	function is_ignored($name)
	{
		if (isset($this->ignores[$name]))
			return true;

		foreach ($this->glob_ignores as $glob) {
			if (preg_match('!' . str_replace('*', '.*', $glob) . '!', $name))
				return true;
		}

		return false;
	}

	function is_overriden($name)
	{
		return isset($this->overrides[$name]);
	}

	function get_override($name)
	{
		return $this->overrides[$name];
	}
}

?>
