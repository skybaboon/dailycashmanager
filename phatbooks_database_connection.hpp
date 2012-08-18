#ifndef GUARD_phatbooks_database_connection_hpp
#define GUARD_phatbooks_database_connection_hpp

/** \file phatbooks_database_connection.hpp
 *
 * \brief Header file pertaining to PhatbooksDatabaseConnection class.
 *
 * \author Matthew Harvey
 * \date 04 July 2012.
 *
 * Copyright (c) 2012, Matthew Harvey. All rights reserved.
 */


#include "account.hpp"
#include "commodity.hpp"
#include "database_connection.hpp"

namespace phatbooks
{

/** Phatbooks-specific database connection class.
 * See API documentation for sqloxx::DatabaseConnection,
 * for parts of API inherited from sqloxx::DatabaseConnection.
 *
 * @todo If speed becomes a problem, I should find a way to avoid
 * having to call sqlite3_prepare_v2, by caching previously prepared
 * sqlite3_stmt* somehow. I think the only feasible way would be to store it
 * as a static member
 * variable of DatabaseConnection. That way the sqlite3_stmt can
 * be freed using sqlite3_finalize in the destructor.
 * There would end up being several of the pre-prepared statements.
 * I tried making the prepared SQL statement a static variable in the
 * \c function, but this meant I couldn't call sqlite3_finalize on the
 * statement at the appropriate time.
 * Having said this, there's no point unless
 * it causes a problem by being too slow.
 */
class PhatbooksDatabaseConnection:
	public sqloxx::DatabaseConnection
{
public:
	/**
	 * Exhibits the same throwing behaviour (if any) as
	 * sqloxx::DatabaseConnection.
	 */
	PhatbooksDatabaseConnection();
	
	/** Store an Account object in the database
	 * 
	 * @todo Verify that throwing behaviour is as documented.
	 * 
	 * @throws std::runtime_error if p_account has invalid
	 * commodity abbreviation. (Commodity abbreviation of
	 * p_account must correspond to a commodity that has already
	 * been stored in the database.)
	 *
	 * @throws phatbooks::BadTable if the table is not properly
	 * set up for storing \c Account objects, for example if the table
	 * does not have a single-column primary key.
	 * 
	 * @throws sqloxx::TableSizeException if the table has reached its
	 * maximum size and therefore cannot accept any additional rows.
	 *
	 * @throws sqloxx:DatabaseExceptions, or some derivative thereof, if
	 * something else goes wrong in finding the primary key to be
	 * assigned to the inserted object. (This is not expected ever to occur
	 * unless there is a heap allocation issue or the database is corrupt.)
	 *
	 * @throws sqloxx::InvalidConnection if the database connection is invalid
	 * at the time the storage is attempted.
	 *
	 * @throws sqloxx::SQLiteException, or some derivative thereof, if there
	 * is some error binding the data for \c p_account to the SQL statement
	 * involved in storing the account, or in executing the
	 * resulting SQL statement. This not expected to occur except
	 * in cases of a corrupt database or memory allocation failure.
	 *
	 * @param p_account the Account to be stored.
	 *
	 * @returns the integer primary key of the Account just stored.
	 * (This is an autoincrementing primary key.)
	 */
	IdType store(Account const& p_account);

	/** Store a Commodity object in the database
	 *
	 * @todo Verify that throwing behaviour is as documented.
	 *
	 * Exceptions throwing behaviour is essentially the same as that
	 * of the \c store function for \c Account.
	 *
	 * @param p_commodity the Commodity to be stored.
	 *
	 * @returns the integer primary key of the Account just stored.
	 * (This is an autoincrementing primary key.)
	 */
	IdType store(Commodity const& p_commodity);

	/**
	 * @returns \c true if and only if \c p_name is the name of an Account
	 * stored in the database.
	 *
	 * @param p_name name of Account.
	 *
	 * @throws SQLiteException or an exception derived therefrom, if
	 * something goes wrong with the SQL involved in executing
	 * the function. This might occur if \c p_name contains punctuation
	 * marks, spaces or etc., or is a SQL keyword, so that it cannot validly
	 * be a string parameter in a SQL statement. A SQLiteException or
	 * derivative might also be thrown if the database connection is invalid
	 * or the database corrupt. It is not guaranteed that these are the only
	 * circumstances in which an exception might be thrown.
	 */
	bool has_account_named(std::string const& p_name);

	/**
	 * Creates tables required for Phatbooks, and inserts rows
	 * into certain tables to provide application-level data where
	 * required. If the database already contains these tables,
	 * the function does nothing. This function should always be
	 * called after calling DatabaseConnection::open.
	 *
	 * @throws SQLiteException, or some derivative thereof, if setup is
	 * unsuccessful.
	 */
	void setup();

private:
	bool setup_has_occurred();
	static std::string const s_setup_flag;

};  // PhatbooksDatabaseConnection



}  // namespace phatbooks


#endif  // GUARD_phatbooks_database_connection_hpp
