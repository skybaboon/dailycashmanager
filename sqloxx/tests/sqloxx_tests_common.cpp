#include "sqloxx_tests_common.hpp"
#include "sqloxx/database_connection.hpp"
#include "sqloxx/detail/sql_statement.hpp"
#include "sqloxx/detail/sqlite_dbconn.hpp"
#include "sqloxx/shared_sql_statement.hpp"
#include <boost/filesystem.hpp>
#include <jewel/stopwatch.hpp>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using jewel::Stopwatch;
using std::abort;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using sqloxx::detail::SQLStatement;
using sqloxx::detail::SQLiteDBConn;

namespace sqloxx
{
namespace tests
{


bool file_exists(boost::filesystem::path const& filepath)
{
	return boost::filesystem::exists
	(	boost::filesystem::status(filepath)
	);
}


void catch_check_ok(DatabaseConnection& dbc)
{
	try
	{
		dbc.check_ok();
	}
	catch (SQLiteException& e)
	{
		cerr << "Exception caught by DatabaseConnection::check_ok()."
			 << endl;
		cerr << "Error message: " << e.what() << endl;
		cerr << "Failing test." << endl;
		CHECK(false);
	}
	return;
}


void abort_if_exists(boost::filesystem::path const& filepath)
{
	if (file_exists(filepath))
	{
		cerr << "File named \"" << filepath.string() << "\" already "
			 << "exists. Test aborted." << endl;
		std::abort();
	}
	return;
}

void
do_speed_test()
{
	string const filename("aaksjh237nsal");
	int const loops = 5000;
	
	vector<string> statements;
	statements.push_back
	(	"insert into dummy(colA, colB) values(3, 'hi')"
	);
	statements.push_back
	(	"select colA, colB from dummy where colB = "
		" 'asfkjasdlfkasdfasdf' and colB = '-90982097';"
	);
	statements.push_back
	(	"insert into dummy(colA, colB) values(198712319, 'aasdfhasdkjhash');"
	);
	statements.push_back
	(	"select colA, colB from dummy where colA = "
		" 'noasdsjhasdfkjhasdkfjh' and colB = '-9987293879';"
	);

	vector<string>::size_type const num_statements = statements.size();

	string const table_creation_string
	(	"create table dummy(colA int not null, colB text)"
	);
	

	// With SharedSQLStatement
	DatabaseConnection db;
	db.open(filename);
	db.execute_sql(table_creation_string);

	cout << "Timing with SharedSQLStatement." << endl;
	db.execute_sql("begin");
	Stopwatch sw1;
	for (int i = 0; i != loops; ++i)
	{
		SharedSQLStatement s(db, statements[i % num_statements]);
		// s.step_final();
	}
	sw1.log();
	db.execute_sql("end");
	boost::filesystem::remove(filename);


	// With SQLStatement
	SQLiteDBConn sdbc;
	sdbc.open(filename);
	sdbc.execute_sql(table_creation_string);

	cout << "Timing with SQLStatement." << endl;
	sdbc.execute_sql("begin");
	Stopwatch sw0;
	for (int i = 0; i != loops; ++i)
	{
		SQLStatement s(sdbc, statements[i % num_statements]);
		// s.step_final();
	}
	sw0.log();
	sdbc.execute_sql("end");
	boost::filesystem::remove(filename);
	
	return;
}

	

DatabaseConnectionFixture::DatabaseConnectionFixture():
	filepath("Testfile_01")
{
	if (boost::filesystem::exists(boost::filesystem::status(filepath)))
	{
		cerr << "File named \"" << filepath.string()
		     << "\" already exists. Test aborted."
			 << endl;
		abort();
	}
	dbc.open(filepath);
	assert (dbc.is_valid());
}


DatabaseConnectionFixture::~DatabaseConnectionFixture()
{
	assert (dbc.is_valid());
	boost::filesystem::remove(filepath);
	assert (!file_exists(filepath));
}


}  // namespace sqloxx
}  // namespace tests
