DROP PROCEDURE IF EXISTS make_inheritance_graph;
#then_execute
CREATE DEFINER=`root`@`localhost` PROCEDURE `make_inheritance_graph`()
	LANGUAGE SQL
	NOT DETERMINISTIC
	CONTAINS SQL
	SQL SECURITY DEFINER
	COMMENT ''
BEGIN
select base_classes.child_symbol "Source", base_classes.parent_symbol "Target" from baseclasses;
select user_types.symbol "Id", symbols.name "Label" from symbols inner join user_types on user_types.symbol = symbols.symbol;
END