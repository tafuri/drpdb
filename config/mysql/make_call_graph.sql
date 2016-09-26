DROP PROCEDURE IF EXISTS make_call_graph;
#then_execute
CREATE DEFINER=`root`@`localhost` PROCEDURE `make_call_graph`()
	LANGUAGE SQL
	DETERMINISTIC
	CONTAINS SQL
	SQL SECURITY DEFINER
	COMMENT ''
BEGIN
select target_symbol "Target", source_symbol "Source" from calls;
select target_symbol "Id", name "Label" from calls inner join symbols on calls.target_symbol=symbols.symbol
union all
select source_symbol "Id", name "Label" from calls inner join symbols on calls.source_symbol=symbols.symbol;
END