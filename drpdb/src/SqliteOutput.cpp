#include <windows.h>
#include "mysql.h"
#include "drpdb.h"

#include <sqlite3.h>
#include <sstream>

namespace Sqlite {
using namespace Sym;
namespace detail {
struct StructDesc
{
	const char* table;
	const char* desc;
	TableType category;
};

struct MemberDesc
{
	const char* name;
	const char* type;
	const char* desc;
	const char* sql_type;
	bool is_unique;
	bool is_enum;
};

struct EnumValueDesc
{
	int value;
	const char* name;
};

template<typename T, typename Enable = void>
struct SqlTypeHelper;

template<typename T>
struct SqlTypeHelper<T, std::enable_if_t<std::is_integral<T>::value>>
{
	constexpr const char* operator()( ) const { return "INTEGER"; }
};
template<typename T>
struct SqlTypeHelper<T, std::enable_if_t<std::is_enum<T>::value>>
{
	constexpr const char* operator()( ) const { return "INTEGER"; }
};

template<typename T>
struct SqlTypeHelper<T, std::enable_if_t<std::is_floating_point<T>::value>>
{
	constexpr const char* operator()( ) const { return "REAL"; }
};

template<>
struct SqlTypeHelper<std::string, void>
{
	constexpr const char* operator()( ) const { return "TEXT"; }
};

template<>
struct SqlTypeHelper<address_info, void>
{
	constexpr const char* operator()( ) const { return "INTEGER"; }
};

template<typename T>
constexpr const char* SqlType( )
{
	return SqlTypeHelper<T>{}();
}

template<typename T>
constexpr MemberDesc GetMemDesc( const char* name, const char* desc, bool unique )
{
	return{
		name,
		typeid(T).name( ),
		desc,
		SqlType<T>( ),
		unique,
		std::is_enum<T>::value
	};
}

template<typename T>
struct Reflection;

// Generate reflection info for structs
#define BEGIN_STRUCT(_type, _table, _desc, _category) \
template<> struct Reflection<_type> { \
	using type = _type; \
	const char* table = #_table; \
	const char* desc = _desc; \
	const TableType category = _category; \
	static const MemberDesc* Members() { static const MemberDesc descs[] = {
#define MEMBER(_name, _desc) \
		GetMemDesc<decltype(type::_name)>(#_name, _desc, false),
#define UNIQUE_MEMBER(_name, _desc) \
		GetMemDesc<decltype(type::_name)>(#_name, _desc, true),
#define END_STRUCT() \
		{nullptr, nullptr, nullptr, false, false } \
	}; return descs; } \
};
#include "PDBReflection.inl"
#undef END_STRUCT
#undef MEMBER
#undef UNIQUE_MEMBER
#undef BEGIN_STRUCT
// Generate reflection info for types
#define BEGIN_ENUMERATION(_name) \
template<> struct Reflection<_name> { \
	using type = _name; \
	const char* name = #_name; \
	static const EnumValueDesc* Members() { static const EnumValueDesc descs[] = {
#define ENUMERATOR(_name, _value) \
		{static_cast<int>(_name::_value), #_value},
#define END_ENUMERATION( ) \
		{-1, nullptr}, \
	}; return descs; } \
};
#include "PDBReflection.inl"
#undef BEGIN_ENUMERATION
#undef ENUMERATOR
#undef END_ENUMERATION
} // namespace detail

using detail::Reflection;

// Generate full statement string for insert of type
template<typename T>
std::string GetInsertStatement( const std::string& tablename )
{
	auto* member = Reflection<T>{}.Members( );
	std::stringstream stmt;
	stmt << "INSERT INTO " << tablename << " VALUES(";
	while(member->name)
	{
		stmt << "?";
		auto next = member + 1;
		if (next->name)
			stmt << ",";
		member = next;
	}
	stmt << ");";
	return stmt.str( );
}

std::string FixEnumName( std::string name )
{
	{
		const auto pos = name.find( "enum " );
		if (pos != std::string::npos)
			name.erase( pos, 5 );
	}
	const auto pos = name.find( "::" );
	if (pos != std::string::npos)
		name.replace( pos, 2, "_" );
	return name;
}

std::string RemoveLastComma( std::string str )
{
	if (str.back( ) == ',')
		str.pop_back( );
	return str;
}

struct SqliteStatement
{
	SqliteStatement( sqlite3* _db, const std::string& sql )
		: db( _db )
	{
		if (sqlite3_prepare_v2( db, sql.data( ), (int)sql.length( ), &stmt, nullptr ) != SQLITE_OK)
			throw std::runtime_error( sqlite3_errmsg( db ) );
	}
	~SqliteStatement( )
	{
		sqlite3_finalize( stmt );
	}

	// Reset statement for another insert
	void reset( )
	{
		if (sqlite3_reset( stmt ) != SQLITE_OK)
			throw std::runtime_error( sqlite3_errmsg( db ) );
		if (sqlite3_clear_bindings( stmt ) != SQLITE_OK)
			throw std::runtime_error( sqlite3_errmsg( db ) );
		bind_pos = 1;
	}
	// Perform insert
	void execute( )
	{
		int rc;
		while ((rc = sqlite3_step( stmt )) == SQLITE_ROW);
		if (rc != SQLITE_DONE)
			throw std::runtime_error( sqlite3_errmsg( db ) );
	}
	// Bind next value
	template<typename T>
	void bind( const T& value )
	{
		if (!bindValue( value, bind_pos++ ))
			throw std::runtime_error( sqlite3_errmsg( db ) );
	}

	// Generate bind function for types
#define BEGIN_ENUMERATION(enum_name) void bind(enum_name e) { bind(static_cast<int>(e));}
#define BEGIN_STRUCT(type, name, desc, category) void bindStruct(const type& V) {
#define MEMBER(name, desc) bind(V.name);
#define END_STRUCT() }
#include "PDBReflection.inl"
#undef BEGIN_ENUMERATION
#undef BEGIN_STRUCT
#undef MEMBER
#undef END_STRUCT

private:
	// Bind functions
	bool bindValue( float value, int pos )
	{
		return sqlite3_bind_double( stmt, pos, value ) == SQLITE_OK;
	}
	bool bindValue( const address_info& info, int pos )
	{
		return bindValue( info.rv, pos );
	}
	bool bindValue( int value, int pos )
	{
		return sqlite3_bind_int( stmt, pos, value ) == SQLITE_OK;
	}
	bool bindValue( unsigned int value, int pos )
	{
		return sqlite3_bind_int64( stmt, pos, value ) == SQLITE_OK;
	}
	bool bindValue( long value, int pos )
	{
		return sqlite3_bind_int64( stmt, pos, value ) == SQLITE_OK;
	}
	bool bindValue( unsigned long value, int pos )
	{
		return sqlite3_bind_int64( stmt, pos, value ) == SQLITE_OK;
	}
	bool bindValue( long long value, int pos )
	{
		return sqlite3_bind_int64( stmt, pos, value ) == SQLITE_OK;
	}
	bool bindValue( unsigned long long value, int pos )
	{
		return sqlite3_bind_int64( stmt, pos, value ) == SQLITE_OK;
	}
	bool bindValue( const char* value, int pos )
	{
		return sqlite3_bind_text( stmt, pos, value, -1, nullptr ) == SQLITE_OK;
	}
	bool bindValue( const std::string& value, int pos )
	{
		return sqlite3_bind_text( stmt, pos, value.data( ), (int)value.length( ), nullptr ) == SQLITE_OK;
	}
	sqlite3* db{ nullptr };
	sqlite3_stmt* stmt{ nullptr };
	int bind_pos{ 1 };
};

class SqliteDatabase
{
	sqlite3 *db;
	uint32_t options;
public:
	enum Options
	{
		O_ENFORCE_UNIQUE = 0x0001,
		O_VACUUM = 0x0002,
		O_JOURNAL_MODE_MEMORY = 0x0004,
	};
	explicit SqliteDatabase( const char* filename, uint32_t opt )
		: db( nullptr )
		, options( opt )
	{
		const int rc = sqlite3_open( filename, &db );
		if (rc != SQLITE_OK)
			throw std::runtime_error( sqlite3_errstr( rc ) );
		// Option needed?
		execute( "PRAGMA synchronous = OFF" );
		if (options & O_JOURNAL_MODE_MEMORY)
			execute( "PRAGMA journal_mode = MEMORY" );
	}
	~SqliteDatabase( )
	{
		sqlite3_close( db );
	}
	SqliteDatabase( const SqliteDatabase& ) = delete;
	SqliteDatabase& operator=( const SqliteDatabase& ) = delete;
	void Build( const SymbolData& results )
	{
		BuildEnums( );
		BuildTables( results );
		CreateViews( );
		if (options & O_VACUUM)
			execute( "VACUUM;" );
	}
private:
	template<class T>
	void BuildTable( const std::vector<T>& Table, const std::string& name )
	{
		execute( "BEGIN TRANSACTION;" );
		execute( "DROP TABLE IF EXISTS " + name + ";" );
		CreateTable<T>( name );
		if (!Table.empty( )) {
			SqliteStatement stmt( db, GetInsertStatement<T>( name ) );
			for (auto& row : Table) {
				stmt.bindStruct( row );
				stmt.execute( );
				stmt.reset( );
			}
		}
		execute( "END TRANSACTION;" );
	}
	template<class T>
	void CreateTable( std::string tablename )
	{
		Reflection<T> ti;

		std::stringstream stmt;
		const bool enforce_uniuque = options & O_ENFORCE_UNIQUE;

		stmt << "CREATE TABLE " << tablename << "(";
		auto* member = ti.Members( );
		while (member->name)
		{
			stmt << member->name << " " << member->sql_type;
			if (enforce_uniuque && member->is_unique)
				stmt << " UNIQUE";
			auto next = member + 1;
			if (next->name)
				stmt << ",";
			member = next;
		}
		stmt << ");";
		execute( stmt.str( ) );
	}
	template<typename T>
	void BuildEnum( )
	{
		static_assert(std::is_enum<T>::value, "T is not a valid enum type");
		Reflection<T> ti;
		// Cleanup name
		auto table_name = FixEnumName( ti.name );
		execute( "BEGIN TRANSACTION;" );
		execute( "DROP TABLE IF EXISTS " + table_name + ";" );
		execute( "CREATE TABLE " + table_name + " (value INTEGER PRIMARY KEY, name TEXT);" );
		SqliteStatement stmt( db, "INSERT INTO " + table_name + " VALUES(?,?);" );
		auto* member = ti.Members( );
		while(member->name)
		{
			stmt.bind( member->value );
			stmt.bind( member->name );
			stmt.execute( );
			stmt.reset( );
			member++;
		}
		execute( "END TRANSACTION;" );

	}
	void CreateViews()
	{
		static const char*const Views[][2] = {
			{ "function_overview", "SELECT s.name, s.size, f.access, f.virtuality, f.frame_size, f.inline_, f.alloca_, ft.this_adjust, ft.this_type_symbol, ft.call_convention, ft.param_count FROM functions AS f JOIN symbols AS s ON s.symbol = f.symbol JOIN function_types AS ft ON f.type_symbol=ft.symbol" },
			{ "functions_by_size", "SELECT name, size FROM functions NATURAL JOIN symbols ORDER BY size DESC LIMIT 1000" },
			{ "types_by_size", "SELECT symbol, name, size FROM symbols JOIN Tag ON symbols.tag = Tag.value WHERE Tag.name = 'UDT' ORDER BY size DESC" },
			{ "types_by_vtable_size", "SELECT symbols.*, user_types.vtable_count FROM user_types NATURAL JOIN symbols ORDER BY vtable_count DESC" },
			{ "num_includes", "SELECT compilands.library AS Unit, COUNT(compilands.library) AS Includes FROM included JOIN compilands ON included.compiland_symbol=compilands.symbol GROUP BY Unit ORDER BY Includes DESC;"},

		};
		for(auto& view : Views)
		{
			{
				std::stringstream stmt;
				stmt << "DROP VIEW IF EXISTS " << view[0] << ";";
				execute( stmt.str( ) );
			}
			{
				std::stringstream stmt;
				stmt << "CREATE VIEW " << view[0] << " AS " << view[1] << ";";
				execute( stmt.str( ) );
			}
		}

	}
	void BuildEnums()
	{
	#define BEGIN_ENUMERATION(name) BuildEnum<name>();
	#include "PDBReflection.inl"
	#undef BEGIN_STRUCT
	}
	void BuildTables( const SymbolData& Results )
	{
	#define BEGIN_STRUCT(type, name, desc,category) BuildTable(Results.type, #name );
	#include "PDBReflection.inl"
	#undef BEGIN_STRUCT
	}
	void execute( const std::string& stmt )
	{
		char* errMsg{ nullptr };
		if (sqlite3_exec( db, stmt.c_str( ), nullptr, 0, &errMsg ) != SQLITE_OK)
		{
			throw std::runtime_error( errMsg );
		}
	}
};

void output( SymbolData& Data )
{
	try {
		uint32_t options{ 0 };

		if (getOption( "-lowmem" ).empty( ))
			options |= SqliteDatabase::Options::O_JOURNAL_MODE_MEMORY;
		if (getOption( "-novacuum" ).empty( ))
			options |= SqliteDatabase::Options::O_VACUUM;
		SqliteDatabase database( getOption( "-outfile" ).c_str( ), options );
		database.Build( Data );
	}
	catch (const std::runtime_error& e)
	{
		set_error( e.what( ) );
	}
}

static std::string describe( )
{
	return
		R"(
    req: -outfile=<output file>
    opt: -lowmem   (slower but less memory usage)
    opt: -novacuum (do not vacuum database)
)";
}

OutputEngine CreateEngine( )
{
	OutputEngine res;
	res.name = "sqlite";
	res.output = &output;
	res.describe = &describe;
	return res;
}
} // namespace Sqlite
