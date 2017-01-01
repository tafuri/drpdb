#pragma once
#include <algorithm>
#include <string>
#include "stringutils.h"
using namespace Sym;
namespace CSV
{
	struct writer
	{
		std::string out;
		std::string outPath;
		bool UseBitType;
		writer(std::string path, bool UseBitType)
			: outPath(std::move(path))
			, UseBitType(UseBitType)
		{}

		void backup();

		template<class T, class Y>
		void operator<<(std::pair<T, Y> V)
		{
			(*this) << V.second;
		}
		void operator<<(float V);
		void operator<<(int V);
		void operator<<(uint32_t V);
		void operator<<(unsigned long long V);
		void operator<<(long V);
		void operator<<(unsigned long V);
		void operator<<(const Sym::address_info& V);
		void operator<<(const std::string& V);
		void operator<<(bool V);
		void operator<<(long long V);


#define BEGIN_STRUCT(type, name, desc,category) void operator<<(const type &V){
#define MEMBER(name, desc) *this << V.name;
#define END_STRUCT() }

#define BEGIN_ENUMERATION(name) void operator<<(name V){ out += "\""; switch (V) {
#define ENUMERATOR(Tp, name) case Tp::name: out+= #name; break;
#define END_ENUMERATION() } out += "\","; }
#include "PDBReflection.inl"

	};
}
