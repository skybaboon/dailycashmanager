#include "shared_sql_statement.hpp"
#include "database_connection.hpp"
#include "detail/sql_statement.hpp"
#include <jewel/debug_log.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

using boost::shared_ptr;
using std::endl;  // for debug logging
using std::string;


namespace sqloxx
{


SharedSQLStatement::SharedSQLStatement
(	DatabaseConnection& p_database_connection,
	string const& p_statement_text
):
	m_sql_statement
	(	p_database_connection.provide_sql_statement(p_statement_text)
	)
{
}

SharedSQLStatement::~SharedSQLStatement()
{
	m_sql_statement->reset();
	m_sql_statement->clear_bindings();
	m_sql_statement->unlock();
}



template <>
int
SharedSQLStatement::extract<int>(int index)
{
	return m_sql_statement->extract<int>(index);
}


template <>
boost::int64_t
SharedSQLStatement::extract<boost::int64_t>(int index)
{
	return m_sql_statement->extract<boost::int64_t>(index);
}


template <>
double
SharedSQLStatement::extract<double>(int index)
{
	return m_sql_statement->extract<double>(index);
}


template <>
std::string
SharedSQLStatement::extract<std::string>(int index)
{
	return m_sql_statement->extract<std::string>(index);
}


void
SharedSQLStatement::bind(std::string const& parameter_name, int value)
{
	m_sql_statement->bind(parameter_name, value);
	return;
}


void
SharedSQLStatement::bind
(	std::string const& parameter_name, boost::int64_t value
)
{
	m_sql_statement->bind(parameter_name, value);
	return;
}


void
SharedSQLStatement::bind
(	std::string const& parameter_name, std::string const& value
)
{
	m_sql_statement->bind(parameter_name, value);
	return;
}


bool
SharedSQLStatement::step()
{
	return m_sql_statement->step();
}


void
SharedSQLStatement::step_final()
{
	m_sql_statement->step_final();
	return;
}


void
SharedSQLStatement::reset()
{
	m_sql_statement->reset();
	return;
}


void
SharedSQLStatement::clear_bindings()
{
	m_sql_statement->clear_bindings();
	return;
}



}  // namespace sqloxx
