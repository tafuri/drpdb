DROP PROCEDURE IF EXISTS types_by_vtable_size;
#then_execute
CREATE PROCEDURE `types_by_vtable_size`()
	LANGUAGE SQL
	DETERMINISTIC
	CONTAINS SQL
	SQL SECURITY DEFINER
	COMMENT ''
BEGIN
select symbols.*, usertypes.vtable_size from usertypes natural join symbols order by vtable_size desc;
END