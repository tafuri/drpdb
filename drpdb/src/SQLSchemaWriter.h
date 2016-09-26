#pragma once

namespace SQL
{
	enum DBKeyType
	{
		NOTKEY,
		PRIMARYKEY,
		KEY
	};
	template<class T>
	struct DBEntry
	{
		DBEntry(const char* n, DBKeyType k)
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

		void common(DBKeyType type, const char* name);
		void operator<<(const char* V);
		void operator<<(DBEntry<float> V);
		void operator<<(DBEntry<int> V);
		void operator<<(DBEntry<uint32_t> V);
		void operator<<(DBEntry<long> V);
		void operator<<(DBEntry<unsigned long> V);
		void operator<<(DBEntry<unsigned long long> V);
		void operator<<(DBEntry<std::string> V);
		void operator<<(DBEntry<unsigned char> V);
		void operator<<(DBEntry<Sym::address_info> V);
		void operator<<(DBEntry<bool> V);

#define BEGIN_ENUMERATION(name) void operator<<(DBEntry<Sym::name> V) { common(V.Keyness, V.Name); Result += " ENUM(";
#define ENUMERATOR(Enumerator, name) Result += "'"#name"',";
#define END_ENUMERATION() Result.pop_back(); Result += ")NOT NULL,";  }

#define BEGIN_STRUCT(type, name) void operator<<(const Sym::type& V){ (void)V;
#define MEMBER(name) *this << DBEntry<decltype(V.name)>(#name, NOTKEY);
#define UNIQUE_MEMBER(name) *this << DBEntry<decltype(V.name)>(#name, PRIMARYKEY);
#define END_STRUCT()  }

#include "PDBReflection.inl"

	};
}
