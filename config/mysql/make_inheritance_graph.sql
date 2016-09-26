DROP PROCEDURE IF EXISTS make_inheritance_graph;
#then_execute
CREATE DEFINER=`root`@`localhost` PROCEDURE `make_inheritance_graph`()
	LANGUAGE SQL
	NOT DETERMINISTIC
	CONTAINS SQL
	SQL SECURITY DEFINER
	COMMENT ''
BEGIN
select baseclasses.child_symbol "Source", baseclasses.parent_symbol "Target" from baseclasses;
select usertypes.symbol "Id", symbols.name "Label" from symbols inner join usertypes on usertypes.symbol = symbols.symbol;
END