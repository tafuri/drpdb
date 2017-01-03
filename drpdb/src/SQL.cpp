#include "drpdb.h"
#include "SQLSchemaWriter.h"
#include "stringutils.h"
namespace SQL
{
	schema_writer::schema_writer(bool UseBitType_)
		:UseBitType(UseBitType_)
	{
		if (UseBitType)
		{
			LoadClause = " (";
		}
	}
	void schema_writer::column_desc(db_index type, const char* name)
	{
		if (type == db_index::PRIMARYKEY)
		{
			Keys += "PRIMARY ";
		}
		if (type != db_index::NOTKEY)
		{
			Keys += "KEY (";
			Keys += name;
			Keys += "),";
		}
		Result += " ";
		Result += name;
		if (UseBitType)
		{
			LoadClause += name;
			LoadClause += ",";
		}

	}
	static void column_comment(const char* comment, std::string& out)
	{
		out += " COMMENT '";
		std::string temp = comment;
		temp = replace(temp, "'", "''");
		out += temp;
		out += "',";
	}
	void schema_writer::operator<<(const char* V) { Result += V; }
	void schema_writer::operator<<(cell<float> V) {	column_desc(V.index_type, V.name);	Result += " FLOAT NOT NULL"; column_comment(V.desc, Result);	}
	void schema_writer::operator<<(cell<int> V) { column_desc(V.index_type, V.name);	Result += " INT NOT NULL"; column_comment(V.desc, Result); }
	void schema_writer::operator<<(cell<uint32_t> V) { column_desc(V.index_type, V.name);	Result += " INT UNSIGNED NOT NULL"; column_comment(V.desc, Result); }
	void schema_writer::operator<<(cell<long> V) { column_desc(V.index_type, V.name);	Result += " BIGINT NOT NULL"; column_comment(V.desc, Result); }
	void schema_writer::operator<<(cell<unsigned long> V) { column_desc(V.index_type, V.name);	Result += " INT UNSIGNED NOT NULL"; column_comment(V.desc, Result); }
	void schema_writer::operator<<(cell<unsigned long long> V) { column_desc(V.index_type, V.name);	Result += " BIGINT UNSIGNED NOT NULL"; column_comment(V.desc, Result); }
	void schema_writer::operator<<(cell<std::string> V) { column_desc(V.index_type, V.name);	Result += " VARCHAR(2047) NOT NULL"; column_comment(V.desc, Result); }
	void schema_writer::operator<<(cell<unsigned char> V) { column_desc(V.index_type, V.name); Result += " INT(3) UNSIGNED NOT NULL"; column_comment(V.desc, Result); }
	void schema_writer::operator<<(cell<long long> V) { column_desc(V.index_type, V.name); Result += " BIGINT NOT NULL"; column_comment(V.desc, Result); }
	void schema_writer::operator<<(cell<Sym::address_info> V)
	{
		std::string name = V.name;
		name += "_";
		std::string temp = name;
		*this << cell<decltype(Sym::address_info::rv)>{(temp + "rv").c_str(), db_index::NOTKEY, V.desc};

	}
	void schema_writer::operator<<(cell<bool> V)
	{
		Result += " ";
		Result += V.name;

		if (UseBitType)
		{
			Result += " BIT(1) NOT NULL";
			column_comment(V.desc, Result);

			LoadClause += "@tmp_";
			LoadClause += V.name;
			LoadClause += ",";


			if (SetClause.size() == 0)
			{
				SetClause = "SET ";
			}
			else
			{
				SetClause += ", ";
			}
			SetClause += V.name;
			SetClause += "= CAST(@tmp_";
			SetClause += V.name;
			SetClause += " AS UNSIGNED)";
		}
		else
		{
			Result += " BOOLEAN";
			column_comment(V.desc, Result);
		}

	}
}
