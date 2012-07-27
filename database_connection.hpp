#ifndef GUARD_database_connection_hpp
#define GUARD_database_connection_hpp

#include <sqlite3.h>
#include <boost/noncopyable.hpp>
#include <string>

/**
 *
 * @todo Move sqloxx code to a separate library.
 * 
 * @todo m_connection should be made private if possible.
 *
 * @todo Make the DatabaseConnection class provide for a means
 * to check whether the particular file being connected to is
 * the right kind of file for the application that is using
 * the DatabaseConnection class. At the moment it just checks whether
 * the file exists. This could involve overriding some method or other.
 *
 * @todo Use boost::filesystem::path to make filepath passed to
 * activate_database more portable.
 *
 * @todo Consider supplying public member function to close any
 * database connections and shut down SQLite3. Current this is done
 * in the destructor, but this can't throw.
 *
 * @todo DatabaseConnection::is_valid should probably do more than
 * just check whether m_connection exists. It should probably also
 * at least check SQLite error status.
 */




namespace sqloxx
{



/**
 * Class to manage connection to SQLite3 database and execute
 * SQL on that database. 
 *
 * This class can be derived from and certain member functions
 * overridden to create application-specific database code.
 */
class DatabaseConnection:
	private boost::noncopyable
{
public:


	/**
	 * Initializes SQLite3 and creates a database connection
	 * initially set to null.
	 *
	 * @throws SQLiteException if SQLite3 initialization fails,
	 * or if database connection cannot be opened.
	 */
	DatabaseConnection();

	/**
	 * Closes any open SQLite3 database connection, and also
	 * shuts down SQLite3.
	 *
	 * Does not throw. If SQLite3 connection closure or shutdown fails,
	 * the application is aborted with a diagnostic message written to
	 * std::clog.
	 */
	virtual ~DatabaseConnection();

	/**
	 * Returns \c true iff the DatabaseConnection is connected to a 
	 * database.
	 */
	virtual bool is_valid();

	/**
	 * Points the datase connection to a specific file
	 * given by \c filename. If the file
	 * does not already exist it is created. Upon creation, the
	 * setup_tables function is called, to set up
	 * any application specific tables and initial data as desired.
	 * This function is empty by default, but you can inherit from
	 * this class and have your derived class override setup_tables
	 * as desired to perform your application-specific code
	 * to setup the database.
	 *
	 * @param filename file to connect to
	 *
	 * @throws SQLiteException if SQLite3
	 * if database connection cannot be opened to the specified file,
	 * or if a new file is created but setup_tables does not
	 * succeed.
	 */
	void open(char const* filename);	


protected:

	/**
	 * Wrapper class for sqlite_stmt*.
	 *
	 * @todo The constructor to create a SQLStatement should reject strings
	 * containing semicolons, since compound statements are not handled by
	 * step() properly. There should be some other class SQLMultiStatement or
	 * something, which can then executed using a wrapper around sqlite3_exec.
	 */
	class SQLStatement;

	/**
	 * Create application-specific tables in database.
	 * Certain tables containing specific "fixed" application data are
	 * populated with rows in this process.
	 *
	 * This function should be redefined in derived class if required
	 * to perform application-specific database setup.
	 *
	 * @throws sqloxx::SQLiteException if operation
	 * fails.
	 */
	virtual void setup_tables();

	/**
	 * Throws a SQLiteException with the current sqlite3_errmsg passed
	 * to the constructor of the exception.
	 *
	 * This is essentially to save typing.
	 *
	 * @throws SQLiteException whenever called
	 */
	void throw_sqlite_exception();

	/**
	 * Executes a string on the database connection.
	 * This should be used only where the developer has complete
	 * control of the string being passed, to prevent SQL injection
	 * attacks. Generally, the functions provided by SQLStatement should
	 * be the preferred means for building and executing SQL statements.
	 */
	void execute_sql(std::string const& str);

	
private:

	/**
	 * A connection to a SQLite3 database file.
	 *
	 * (Note this is a raw pointer not a smart pointer
	 * to facilitate more straightforward interaction with the SQLite
	 * C API.)
	 */
	sqlite3* m_connection;

};





class DatabaseConnection::SQLStatement:
	private boost::noncopyable
{
public:

	SQLStatement(DatabaseConnection& dbconn, std::string const& str);

	~SQLStatement();

	// Wrapper around SQLite bind functions
	void bind(std::string const& parameter_name, double value);
	void bind(std::string const& parameter_name, int value);
	void bind(std::string const& parameter_name, std::string const& str);

	// Wraps sqlite3_step
	// Returns true as long as there are further steps to go.
	bool step();


private:
	sqlite3_stmt* m_statement;
	DatabaseConnection& m_database_connection;

	// Return index no. of named parameter in statement
	int parameter_index(std::string const& parameter_name) const;

	// Check code is SQLITE_OK and if not finalize statement and
	// throw SQLiteException.
	void check_ok(int err_code);

};




}  // namespace sqloxx

#endif  // GUARD_database_connection_hpp
