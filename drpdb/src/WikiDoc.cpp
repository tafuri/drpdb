#include "drpdb.h"
namespace
{
	struct folio
	{
		std::string header;
		std::string master_tables;
		std::string data_types;
		std::string functions;
		std::string files;
		folio()
		{
			header = "Below is a basic overview of the database schema. This information represents our understanding of the DIA API to the best of our current knowledge.";
			master_tables = "#Master tables\n";
			data_types = "#Tables representing data types\n";
			functions = "#Tables representing functions and function calls\n";
			files = "#Tables representing file structure\n";
		}
	};
	std::string* add_table(Sym::TableType type, char* name, const char* desc, folio& folio)
	{
		std::string* output = &folio.master_tables;
		if (type == Sym::TableType::Data)
		{
			output = &folio.data_types;
		}
		else if (type == Sym::TableType::Function)
		{
			output = &folio.functions;
		}
		else if (type == Sym::TableType::Files)
		{
			output = &folio.files;
		}

		(*output) += "##";
		(*output) += name;
		(*output) += "\n";
		(*output) += desc;
		(*output) += "\n";
		return output;
	}

	void add_member(const char* name, const char* desc, std::string& out)
	{
		out += "+ ";
		out += name;
		out += ": ";
		out += desc;
		out += "\n";
	}
}
namespace WikiDoc
{
	void Output(const char* filename)
	{
		folio output;
		std::string* cur_section;

#define BEGIN_STRUCT(type, name, desc, category) cur_section = add_table(category, #name, desc, output);
#define MEMBER(name, desc) add_member(#name, desc, *cur_section);
#define END_STRUCT() *cur_section += "\r\n";
#include "PDBReflection.inl"
#undef BEGIN_STRUCT
#undef MEMBER
#undef END_STRUCT
		std::ofstream out(filename);
		out << output.header;
		out << "\n";
		out << output.master_tables;
		out << "\n";
		out << output.data_types;
		out << "\n";
		out << output.functions;
		out << "\n";
		out << output.files;
	}
}
