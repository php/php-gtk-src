<?php

class Enum_Def {
	var $def_type 	= 'enum';
	var $name 		= null;
	var $in_module 	= null;
	var $c_name 	= null;
	var $values		= array();

	function Enum_Def($args)
	{
		$this->name = array_shift($args);

		foreach ($args as $arg) {
			if (!is_array($arg) || count($arg) < 2)
				continue;

			if ($arg[0] == 'in-module')
				$this->in_module = $arg[1];
			else if ($arg[0] == 'c-name')
				$this->c_name = $arg[1];
			else if ($arg[0] == 'value') {
				$value_name = null;
				$value_c_name = null;
				foreach (array_slice($arg, 1) as $value_arg) {
					if ($value_arg[0] == 'name')
						$value_name = $value_arg[1];
					else if ($value_arg[0] == 'c-name')
						$value_c_name = $value_arg[1];
				}
				$this->values[] = array($value_name, $value_c_name);
			}
		}
	}
}

class Flag_Def extends Enum_Def {
	var $def_type = 'flags';

	function Flag_Def($args) {
		parent::Enum_Def($args);
	}
}

class Object_Def {
	var $def_type 	= 'object';
	var $name 		= null;
	var $in_module 	= null;
	var $parent		= null;
	var $c_name 	= null;
	var $ce			= null;
	var $fields		= array();
	var $methods	= array();

	function Object_Def($args)
	{
		$this->name = array_shift($args);

		foreach ($args as $arg) {
			if (!is_array($arg) || count($arg) < 2)
				continue;

			if ($arg[0] == 'in-module')
				$this->in_module = $arg[1];
			else if ($arg[0] == 'parent')
				$this->parent = $arg[1];
			else if ($arg[0] == 'c-name')
				$this->c_name = $arg[1];
			else if ($arg[0] == 'field') {
				$field_type = null;
				$field_name = null;
				foreach (array_slice($arg, 1) as $field_arg) {
					if ($field_arg[0] == 'type-and-name') {
						$field_type = $field_arg[1];
						$field_name = $field_arg[2];
					}
				}
				$this->fields[] = array($field_type, $field_name);
			}
		}
		$this->ce = strtolower($this->in_module . '_' . $this->name . '_ce');
	}
}

class Method_Def {
	var $name 			= null;
	var $of_object 		= null;
	var $c_name 		= null;
	var $return_type	= null;
	var $params			= array();
	var $varargs		= false;

	function Method_Def($args)
	{
		$this->name = array_shift($args);

		foreach ($args as $arg) {
			if (!is_array($arg) || count($arg) < 2)
				continue;

			if ($arg[0] == 'of-object') {
				if (count($arg) > 2)
					$this->of_object = array($arg[1], $arg[2][0]);
				else
					$this->of_object = array($arg[1], null);
			}
			else if ($arg[0] == 'c-name')
				$this->c_name = $arg[1];
			else if ($arg[0] == 'return-type')
				$this->return_type = $arg[1];
			else if ($arg[0] == 'parameter') {
				$param_type = null;
				$param_name = null;
				$param_default = null;
				$param_null = false;
				foreach (array_slice($arg, 1) as $param_arg) {
					if ($param_arg[0] == 'type-and-name') {
						$param_type = $param_arg[1];
						$param_name = $param_arg[2];
					}
					else if ($param_arg[0] == 'default')
						$param_default = $param_arg[1];
					else if ($param_arg[0] == 'null-ok')
						$param_null = true;
				}
				$this->params[] = array($param_type, $param_name,
										$param_default, $param_null);
			} else if ($arg[0] == 'varargs')
				$this->varargs = $arg[1] == 't';
		}
	}
}

class Function_Def {
	var $name 				= null;
	var $in_module 			= null;
	var $is_constructor_of	= null;
	var $c_name 			= null;
	var $return_type		= null;
	var $params				= array();
	var $varargs			= false;

	function Function_Def($args) {
		$this->name = array_shift($args);

		foreach ($args as $arg) {
			if (!is_array($arg) || count($arg) < 2)
				continue;

			if ($arg[0] == 'in-module')
				$this->in_module = $arg[1];
			else if ($arg[0] == 'is-constructor-of')
				$this->is_constructor_of = $arg[1];
			else if ($arg[0] == 'c-name')
				$this->c_name = $arg[1];
			else if ($arg[0] == 'return-type')
				$this->return_type = $arg[1];
			else if ($arg[0] == 'parameter') {
				$param_type = null;
				$param_name = null;
				$param_default = null;
				$param_null = false;
				foreach (array_slice($arg, 1) as $param_arg) {
					if ($param_arg[0] == 'type-and-name') {
						$param_type = $param_arg[1];
						$param_name = $param_arg[2];
					}
					else if ($param_arg[0] == 'default')
						$param_default = $param_arg[1];
					else if ($param_arg[0] == 'null-ok')
						$param_null = true;
				}
				$this->params[] = array($param_type, $param_name,
										$param_default, $param_null);
			} else if ($arg[0] == 'varargs')
				$this->varargs = $arg[1] == 't';
		}
	}
}

?>
