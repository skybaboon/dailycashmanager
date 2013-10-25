/*
 * This file is part of the Phatbooks project and is distributed under the
 * terms of the license contained in the file LICENSE.txt distributed
 * with this package.
 * 
 * Author: Matthew Harvey <matthew@matthewharvey.net>
 *
 * Copyright (c) 2012-2013, Matthew Harvey.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef GUARD_make_default_accounts_hpp_29520491757206846
#define GUARD_make_default_accounts_hpp_29520491757206846

#include "account_type.hpp"
#include <sqloxx/handle_fwd.hpp>
#include <vector>

namespace phatbooks
{

// begin forward declarations

class Account;
class PhatbooksDatabaseConnection;

// end forward declarations

/**
 * @returns a vector of handles to "suggested default Accounts" that might be
 * presented to the user as a starting point when setting up a new Phatbooks
 * file. The Accounts will all names and AccountTypes initialized, and will 
 * have an empty wxString as their description. They will \e not have their
 * Commodity initialized though - this must be done before the Accounts
 * are saved.
 *
 * @param p_database_connection a connection to the database with which
 * the returned sqloxx::Handle<Account> instances will be associated
 * (but note, calling this function will \e not cause the Accounts to be
 * saved to the database).
 */
std::vector<sqloxx::Handle<Account> >
make_default_accounts(PhatbooksDatabaseConnection& p_database_connection);


/**
 * Behaves like make_default_accounts(PhatbooksDatabaseConnection&), but
 * instead of returning a vector, it populates the vector passed to \e vec.
 *
 * @param vec the vector which the function will populate with
 * sqloxx::Handle<Account>, which will be pushed onto the back of
 * e\ vec - which need not be empty when passed to the function.
 */
void make_default_accounts
(	PhatbooksDatabaseConnection& p_database_connection,
	std::vector<sqloxx::Handle<Account> >& vec
);

/**
 * Behaves like the two-parameter function with the same name, but only
 * populates \e vec with default Accounts of AccountType p_account_type.
 * The sqloxx::Handle<Account> will be pushed onto the back of \e vec - which
 * need not be empty when passed to the function.
 */
void make_default_accounts
(	PhatbooksDatabaseConnection& p_database_connection,
	std::vector<sqloxx::Handle<Account> >& vec,
	AccountType p_account_type
);


}  // namespace phatbooks

#endif  // GUARD_make_default_accounts_hpp_29520491757206846