#include <windows.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <fstream>
#include "drpdb.h"
#include "CSVWriter.h"
#include "SQLSchemaWriter.h"

namespace ODBC
{
	void listDrivers()
	{
		SQLHANDLE sEnv = SQL_NULL_HENV;
		SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sEnv);
		std::cout << "Available ODBC drivers:\r\n";

		SQLSetEnvAttr(sEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3_80, 0);

		SQLWCHAR desc[1024] = {};
		
		SQLSMALLINT desclen, attribslen;
		SQLUSMALLINT dir = SQL_FETCH_FIRST;
		while (SQLDrivers(sEnv, dir, desc, 1024, &desclen, nullptr, 0, &attribslen) == SQL_SUCCESS)
		{
			dir = SQL_FETCH_NEXT;
			std::wcout << "   " << desc << "\r\n";
		}
	}
	struct Output
	{
		SymbolData& Results;
		std::string ConnectionString;
		std::vector<std::string> UploadCommands;
		std::string TempDir;

		Output(SymbolData& Res )
			:Results(Res)
		{
			ConnectionString = getOption("-connect");
			if (getFlag("-drivers"))
			{
				listDrivers();
				exit(0);
			}
			//TempDir = OutDirEscaped() + "\\\\";

			Send();
		}


		void LoadTable(const std::string& name, const std::string& end)
		{
			std::string Cmd = "load data local infile '";
			//Cmd += TempDir;
			Cmd += name;
			Cmd += "_values.txt' into table ";
			Cmd += name;
			Cmd += " fields terminated by ',' optionally enclosed by '\"' lines terminated by '\\n'";
			Cmd += end;
			Cmd += ";";

			UploadCommands.push_back(Cmd);
		}
		template<class T>
		void CreateTable(std::string tablename, std::string& EndClause)
		{
			T Value;
			UploadCommands.push_back(std::string("DROP TABLE IF EXISTS ") + tablename + ";");
			SQL::schema_writer Ar(false);

			Ar.Result += "CREATE TABLE ";
			Ar.Result += tablename.c_str();
			Ar.Result += "(";
			Ar << Value;
			Ar.Result += Ar.Keys;
			if (Ar.Result.back() == ',')
			{
				Ar.Result.pop_back();
			}
			Ar.Result += ")";
			UploadCommands.push_back(Ar.Result);

			if (Ar.UseBitType)
			{
				Ar.LoadClause.pop_back();
				Ar.LoadClause += ") ";
			}
			
			EndClause = Ar.LoadClause + Ar.SetClause;
		}


		template<class T>
		void PopulateTable(T TableBegin, T TableEnd, const std::string& name)
		{
			CSV::writer Ar(TempDir + name + "_values.txt", false);
			while (TableBegin != TableEnd)
			{
				Ar << *TableBegin;
				Ar.backup();
				++TableBegin;
				Ar.out += "\n";
			}
			std::ofstream writer(Ar.outPath, std::ios::out | std::ios::binary);
			writer << Ar.out;
		}

		template<class T>
		void BuildTable(const std::vector<T>& Table, const std::string& name)
		{
			std::string EndClause;
			CreateTable<T>(name, EndClause);
			PopulateTable(Table.begin(), Table.end(), name);
		}

		template<class U, class T>
		void BuildTable(const std::unordered_map<U, T>& Table, const std::string& name)
		{
			std::string EndClause;
			CreateTable<T>(name, EndClause);
			PopulateTable(Table.begin(), Table.end(), name);
		}

		void Send()
		{
#define BEGIN_STRUCT(type, name, desc,category) BuildTable(Results.type, #name );

#include "PDBReflection.inl"

			if (ConnectionString.size() > 0)
			{
				SQLHENV hEnv;
				SQLAllocEnv(&hEnv);

				SQLHDBC hConn;
				SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hConn);

				SQLSetConnectAttr(hConn, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER)1u, 0);
				if (SUCCEEDED(SQLDriverConnectA(hConn, 0, (SQLCHAR*)ConnectionString.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT)))
				{
					for (auto&& CmdStr : UploadCommands)
					{
						SQLHANDLE Stmt;
						if (SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, hConn, &Stmt)))
						{
							if (SUCCEEDED(SQLPrepareA(Stmt, (SQLCHAR*)CmdStr.c_str(), static_cast<SQLINTEGER>(CmdStr.length()))))
							{

								if (!SUCCEEDED(SQLExecute(Stmt)))
								{
									//DebugBreak();
								}
								SQLFreeHandle(SQL_HANDLE_STMT, &Stmt);
							}
						}
					}
					SQLDisconnect(hConn);
				}

			}
		}
	};

	void output(SymbolData& Data)
	{
		Output Result(Data);
	}
	std::string describe()
	{
		return "   (incomplete!)\n    req: -connect=<odbc connection string>\n    opt: -drivers (lists available drivers)";
	}
	OutputEngine CreateEngine()
	{
		OutputEngine res;
		res.describe = &describe;
		res.name = "odbc";
		res.output = &output;
		return res;
	}
}
