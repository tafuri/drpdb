DROP PROCEDURE IF EXISTS function_overview;
#then_execute
CREATE PROCEDURE `function_overview`()
	LANGUAGE SQL
	DETERMINISTIC
	CONTAINS SQL
	SQL SECURITY DEFINER
	COMMENT ''
BEGIN
select symbols.name, symbols.size, functions.access, functions.virtuality, functions.frame_size, functions.inline_, functions.alloca_, function_types.this_adjust, function_types.this_type_symbol, function_types.call_convention, function_types.param_count from functions inner join symbols on symbols.symbol = functions.symbol inner join function_types on functions.type_symbol=function_types.symbol;
END