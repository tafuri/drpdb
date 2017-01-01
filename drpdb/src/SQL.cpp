#include "drpdb.h"
#include "SQLSchemaWriter.h"

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
	void schema_writer::column_desc(DBKeyType type, const char* name)
	{
		if (type == DBKeyType::PRIMARYKEY)
		{
			Keys += "PRIMARY ";
		}
		if (type != DBKeyType::NOTKEY)
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
	void schema_writer::operator<<(const char* V) { Result += V; }
	void schema_writer::operator<<(cell<float> V) { column_desc(V.Keyness, V.Name);	Result += " FLOAT NOT NULL,"; }
	void schema_writer::operator<<(cell<int> V) { column_desc(V.Keyness, V.Name);	Result += " INT NOT NULL,"; }
	void schema_writer::operator<<(cell<uint32_t> V) { column_desc(V.Keyness, V.Name);	Result += " INT UNSIGNED NOT NULL,"; }
	void schema_writer::operator<<(cell<long> V) { column_desc(V.Keyness, V.Name);	Result += " BIGINT NOT NULL,"; }
	void schema_writer::operator<<(cell<unsigned long> V) { column_desc(V.Keyness, V.Name);	Result += " INT UNSIGNED NOT NULL,"; }
	void schema_writer::operator<<(cell<unsigned long long> V) { column_desc(V.Keyness, V.Name);	Result += " BIGINT UNSIGNED NOT NULL,"; }
	void schema_writer::operator<<(cell<std::string> V) { column_desc(V.Keyness, V.Name);	Result += " VARCHAR(2047) NOT NULL,"; }
	void schema_writer::operator<<(cell<unsigned char> V) { column_desc(V.Keyness, V.Name); Result += " INT(3) UNSIGNED NOT NULL, "; }
	void schema_writer::operator<<(cell<long long> V) { column_desc(V.Keyness, V.Name); Result += " BIGINT NOT NULL, "; }
	void schema_writer::operator<<(cell<Sym::address_info> V)
	{
		std::string name = V.Name;
		name += "_";
		std::string temp = name;
		*this << cell<decltype(Sym::address_info::rv)>{(temp + "rv").c_str(), DBKeyType::NOTKEY};

	}
	void schema_writer::operator<<(cell<bool> V)
	{
		Result += " ";
		Result += V.Name;

		if (UseBitType)
		{
			Result += " BIT(1) NOT NULL,";

			LoadClause += "@tmp_";
			LoadClause += V.Name;
			LoadClause += ",";


			if (SetClause.size() == 0)
			{
				SetClause = "SET ";
			}
			else
			{
				SetClause += ", ";
			}
			SetClause += V.Name;
			SetClause += "= CAST(@tmp_";
			SetClause += V.Name;
			SetClause += " AS UNSIGNED)";
		}
		else
		{
			Result += " BOOLEAN,";
		}

	}
}
