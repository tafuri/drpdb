#pragma once

namespace SQL
{
	enum class db_index
	{
		NOTKEY,
		PRIMARYKEY,
		KEY
	};
	template<class T>
	struct cell
	{
		cell(const char* n, db_index k, const char* desc)
			:index_type(k), name(n), desc(desc)
		{}

		db_index index_type;
		const char* desc;
		const char* name;
	};
	struct schema_writer
	{
		std::string Result;
		std::string Keys;
		std::string LoadClause;
		std::string SetClause;
		const bool UseBitType;
		schema_writer(bool UseBitType_);

		void column_desc(db_index type, const char* name);
		void operator<<(const char* V);
		void operator<<(cell<float> V);
		void operator<<(cell<int> V);
		void operator<<(cell<uint32_t> V);
		void operator<<(cell<long> V);
		void operator<<(cell<unsigned long> V);
		void operator<<(cell<unsigned long long> V);
		void operator<<(cell<std::string> V);
		void operator<<(cell<unsigned char> V);
		void operator<<(cell<long long> V);
		void operator<<(cell<Sym::address_info> V);
		void operator<<(cell<bool> V);

#define BEGIN_ENUMERATION(enum_name) void operator<<(cell<Sym::enum_name> V) { column_desc(V.index_type, V.name); Result += " ENUM(";
#define ENUMERATOR(Enumerator, name) Result += "'"#name"',";
#define END_ENUMERATION() Result.pop_back(); Result += ")NOT NULL,";  }

#define BEGIN_STRUCT(type, name, desc, category) void operator<<(const Sym::type& V){ (void)V;
#define MEMBER(name, desc) *this << cell<decltype(V.name)>(#name, db_index::NOTKEY, desc);
#define UNIQUE_MEMBER(name, desc) *this << cell<decltype(V.name)>(#name, db_index::PRIMARYKEY, desc);
#define END_STRUCT()  }

#include "PDBReflection.inl"

	};
}
