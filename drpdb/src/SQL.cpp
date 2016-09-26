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
	void schema_writer::common(DBKeyType type, const char* name)
	{
		if (type == PRIMARYKEY)
		{
			Keys += "PRIMARY ";
		}
		if (type != NOTKEY)
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
	void schema_writer::operator<<(DBEntry<float> V) { common(V.Keyness, V.Name);	Result += " FLOAT NOT NULL,"; }
	void schema_writer::operator<<(DBEntry<int> V) { common(V.Keyness, V.Name);	Result += " INT NOT NULL,"; }
	void schema_writer::operator<<(DBEntry<uint32_t> V) { common(V.Keyness, V.Name);	Result += " INT UNSIGNED NOT NULL,"; }
	void schema_writer::operator<<(DBEntry<long> V) { common(V.Keyness, V.Name);	Result += " BIGINT NOT NULL,"; }
	void schema_writer::operator<<(DBEntry<unsigned long> V) { common(V.Keyness, V.Name);	Result += " INT UNSIGNED NOT NULL,"; }
	void schema_writer::operator<<(DBEntry<unsigned long long> V) { common(V.Keyness, V.Name);	Result += " BIGINT UNSIGNED NOT NULL,"; }
	void schema_writer::operator<<(DBEntry<std::string> V) { common(V.Keyness, V.Name);	Result += " VARCHAR(2047) NOT NULL,"; }
	void schema_writer::operator<<(DBEntry<unsigned char> V) { common(V.Keyness, V.Name); Result += " INT(3) UNSIGNED NOT NULL, "; }
	void schema_writer::operator<<(DBEntry<Sym::address_info> V)
	{
		std::string name = V.Name;
		name += "_";
		std::string temp = name;
		*this << DBEntry<decltype(Sym::address_info::rv)>{(temp + "rv").c_str(), DBKeyType::NOTKEY};

	}
	void schema_writer::operator<<(DBEntry<bool> V)
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
