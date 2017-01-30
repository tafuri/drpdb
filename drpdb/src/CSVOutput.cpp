#include <algorithm>
#include <iostream>
#include <cstdio>
#include <windows.h>
#include <fstream>
#include "drpdb.h"
#include "CSVWriter.h"
#include <locale>
#include <codecvt>

namespace
{
	struct Output
	{
		SymbolData& Results;
		std::string outdir;
		bool naked;
		char separator = ',';
		Output(SymbolData& Res)
			: Results(Res)
			, separator()
		{
		}
		void init()
		{
			outdir = getOption("-outdir");
			if (outdir.empty())
			{
				char buf[MAX_PATH] = {};
				GetCurrentDirectoryA(MAX_PATH, buf);
				outdir = buf;
			}
			naked = getFlag("-nocolumnheaders");
			separator = CSV::details::getSeparator();
		}

		void AppendHeader(std::string table, std::string& out)
		{

#define BEGIN_STRUCT(type, name, desc, category) if (table == #name ){ 
#define MEMBER(name, desc)  out += ( #name );  out += separator;
#define END_STRUCT()  ; if (out.back() == separator) out.pop_back(); out+="\n";}

#include "PDBReflection.inl"

		}


		template<class T>
		void PopulateTable(T TableBegin, T TableEnd, const std::string& name)
		{
			CSV::writer Ar(outdir + name + ".csv", true, separator);
			if (!naked)
			{
				AppendHeader(name, Ar.out);
			}
			while (TableBegin != TableEnd)
			{
				Ar << *TableBegin;
				Ar.backup();
				++TableBegin;
				Ar.out += "\n";
			}
			std::ofstream writer(Ar.outPath, std::ios::out | std::ios::binary);
			writer << Ar.out;
			if (writer.bad())
			{
				std::string err = "Write to ";
				err += Ar.outPath;
				err += " failed";
				set_error(err.data());
			}
		}

		template<class T>
		void BuildTable(const T& Table, const std::string& name)
		{
			PopulateTable(Table.begin(), Table.end(), name);
		}

		void GenerateCommands()
		{
#define BEGIN_STRUCT(type, name, desc, category) BuildTable(Results.type, #name );

#include "PDBReflection.inl"

		}
	};
}
namespace CSV
{
	void writer::backup()
	{
		out.pop_back();
	}

	void writer::operator<<(float V)
	{
		char Buf[64];
		_snprintf_s(Buf, 64, "%f", V);
		out += Buf;
		out += separator;
	}
	void writer::operator<<(int V)
	{
		char Buf[64];
		_snprintf_s(Buf, 64, "%d", V);
		out += Buf;
		out += separator;
	}
	void writer::operator<<(uint32_t V)
	{
		char Buf[64];
		_snprintf_s(Buf, 64, "%u", V);
		out += Buf;
		out += separator;
	}
	void writer::operator<<(unsigned long long V)
	{
		char Buf[90];
		_snprintf_s(Buf, 64, "%llu", V);
		out += Buf;
		out += separator;
	}
	void writer::operator<<(long long V)
	{
		char Buf[64];
		_snprintf_s(Buf, 64, "%lld", V);
		out += Buf;
		out += separator;
	}
	void writer::operator<<(long V)
	{
		char Buf[64];
		_snprintf_s(Buf, 64, "%ld", V);
		out += Buf;
		out += separator;
	}
	void writer::operator<<(unsigned long V)
	{
		char Buf[64];
		_snprintf_s(Buf, 64, "%lu", V);
		out += Buf;
		out += separator;
	}
	void writer::operator<<(const Sym::address_info& V)
	{
		*this << V.rv;
	}
	void writer::operator<<(const std::string& V)
	{
		out += "\"";
		auto copy = V;
		escape(copy, separator);
		out += std::move(copy);
		out += "\"";
		out += separator;
	}
	void writer::operator<<(bool V)
	{
		if (UseBitType)
		{
			out += V ? '1' : '0';
		}
		else
		{
			out += V ? "TRUE" : "FALSE";
		}
		out += separator;
	}

	namespace
	{
		void output(SymbolData& Data)
		{
			Output Result(Data);
			Result.init();
			Result.GenerateCommands();
		}

		static std::string describe()
		{
			return
				"    opt: -outdir=<output directory>\n    opt: -nocolumnheaders\n     opt: -uselocaleseparator";
		}
	}

	OutputEngine CreateEngine()
	{
		OutputEngine res;
		res.name = "csv";
		res.output = &output;
		res.describe = &describe;
		return res;
	}

	namespace details
	{
		char getSeparator()
		{
			char SeparatorTmp = ',';
			if (!getFlag("-uselocaleseparator"))
			{
				return SeparatorTmp;
			}
			auto nRequestedChars = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, nullptr, 0);
			if (2 == nRequestedChars) //we support only one char separator plus terminator (apparently possible up to tree)
			{
				std::vector<TCHAR> vSeparator(nRequestedChars, 0);
				if (0 != ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, &(vSeparator[0]), nRequestedChars))
				{
					SeparatorTmp = *std::begin(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(vSeparator[0]));
				}
			}
			return SeparatorTmp;
		}
	};
}