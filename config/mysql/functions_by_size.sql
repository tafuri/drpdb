DROP PROCEDURE IF EXISTS functions_by_size;
#then_execute
CREATE DEFINER=`root`@`localhost` PROCEDURE `functions_by_size`()
	LANGUAGE SQL
	NOT DETERMINISTIC
	CONTAINS SQL
	SQL SECURITY DEFINER
	COMMENT ''
BEGIN
select name, size from functions natural join symbols where parent_class_id != parent_symbol_id order by size desc limit 1000 ;
END