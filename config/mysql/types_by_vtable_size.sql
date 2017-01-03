DROP PROCEDURE IF EXISTS types_by_vtable_size;
#then_execute
CREATE PROCEDURE `types_by_vtable_size`()
	LANGUAGE SQL
	DETERMINISTIC
	CONTAINS SQL
	SQL SECURITY DEFINER
	COMMENT ''
BEGIN
select symbols.*, user_types.vtable_count from user_types natural join symbols order by vtable_count desc;
END