#pragma once

namespace SQL
{
	enum class DBKeyType
	{
		NOTKEY,
		PRIMARYKEY,
		KEY
	};
	template<class T>
	struct cell
	{
		cell(const char* n, DBKeyType k)
			:Keyness(k), Name(n)
		{}

		DBKeyType  Keyness;
		const char* Name;
	};
	struct schema_writer
	{
		std::string Result;
		std::string Keys;
		std::string LoadClause;
		std::string SetClause;
		const bool UseBitType;
		schema_writer(bool UseBitType_);

		void column_desc (DBKeyType type, const char* name);
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

#define BEGIN_ENUMERATION(name) void operator<<(cell<Sym::name> V) { column_desc(V.Keyness, V.Name); Result += " ENUM(";
#define ENUMERATOR(Enumerator, name) Result += "'"#name"',";
#define END_ENUMERATION() Result.pop_back(); Result += ")NOT NULL,";  }

#define BEGIN_STRUCT(type, name, desc, category) void operator<<(const Sym::type& V){ (void)V;
#define MEMBER(name, desc) *this << cell<decltype(V.name)>(#name, DBKeyType::NOTKEY);
#define UNIQUE_MEMBER(name, desc) *this << cell<decltype(V.name)>(#name, DBKeyType::PRIMARYKEY);
#define END_STRUCT()  }

#include "PDBReflection.inl"

	};
}
