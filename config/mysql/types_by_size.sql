DROP PROCEDURE IF EXISTS types_by_size;
#then_execute
CREATE PROCEDURE `types_by_size`()
	LANGUAGE SQL
	DETERMINISTIC
	CONTAINS SQL
	SQL SECURITY DEFINER
	COMMENT ''
BEGIN
 select symbol, name, size from symbols where tag='udt' order by size desc;
END