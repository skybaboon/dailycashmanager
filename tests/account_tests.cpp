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

#include "account.hpp"
#include "commodity.hpp"
#include "phatbooks_database_connection.hpp"
#include "phatbooks_tests_common.hpp"
#include <jewel/assert.hpp>
#include <sqloxx/general_typedefs.hpp>
#include <sqloxx/handle.hpp>
#include <UnitTest++/UnitTest++.h>
#include <wx/string.h>

using sqloxx::Handle;
using sqloxx::Id;

// TODO Put tests in here which exercise AmalgamatedBudget as well
// as just Account.

namespace phatbooks
{
namespace test
{

TEST_FIXTURE(TestFixture, test_account_exists)
{
	PhatbooksDatabaseConnection& dbc = *pdbc;

	// These were saved to the database in TestFixture		
	CHECK(Account::exists(dbc, "cash"));
	CHECK(Account::exists(dbc, "cASh"));  // case-insensitive
	CHECK(Account::exists(dbc, "food"));
	CHECK(!Account::exists(dbc, "food "));  // but doesn't trim spaces

	// These are way off
	CHECK(!Account::exists(dbc, "f oo d"));
	CHECK(!Account::exists(dbc, "phasdfo"));

	// Some edge cases
	CHECK(!Account::exists(dbc, ""));
	CHECK(!Account::exists(dbc, "*"));

	// Create another Account
	Handle<Account> a0(dbc);
	a0->set_account_type(AccountType::expense);
	a0->set_name("Gardening Supplies");
	a0->set_commodity(Handle<Commodity>(dbc, 1));
	a0->set_description("stuff for like, gardening");
	a0->set_visibility(Visibility::hidden);

	CHECK(!Account::exists(dbc, "Gardening Supplies"));
	a0->save();
	CHECK(Account::exists(dbc, "Gardening Supplies"));
	CHECK(Account::exists(dbc, "gardening supplies"));
	a0->set_name("bleugh");
	CHECK(!Account::exists(dbc, "bleugh"));
	CHECK(Account::exists(dbc, "gardening supplies"));
	a0->save();
	CHECK(Account::exists(dbc, "bleugh"));
	CHECK(!Account::exists(dbc, "gardening supplies"));
	CHECK(Account::exists(dbc, "food"));
}

TEST_FIXTURE(TestFixture, test_no_user_pl_accounts_saved)
{
	JEWEL_LOG_TRACE();
	PhatbooksDatabaseConnection& dbc = *pdbc;

	// These were saved to the database in TestFixture		
	JEWEL_ASSERT(Account::exists(dbc, "cash"));
	JEWEL_ASSERT(Account::exists(dbc, "cASh"));  // case-insensitive
	JEWEL_ASSERT(Account::exists(dbc, "food"));
	JEWEL_ASSERT(!Account::exists(dbc, "food "));  // but doesn't trim spaces

	CHECK(!Account::no_user_pl_accounts_saved(dbc));
	Handle<Account> const a0(dbc, Account::id_for_name(dbc, "food"));
	a0->remove();
	CHECK(Account::no_user_pl_accounts_saved(dbc));
	Handle<Account> const a1(dbc);
	a1->set_account_type(AccountType::revenue);
	a1->set_name("salary");
	a1->set_commodity(Handle<Commodity>(dbc, 1));
	a1->set_description("salaries and wages received");
	a1->set_visibility(Visibility::hidden);
	CHECK(Account::no_user_pl_accounts_saved(dbc));
	a1->save();
	CHECK(!Account::no_user_pl_accounts_saved(dbc));
	a1->remove();
	CHECK(Account::no_user_pl_accounts_saved(dbc));

	Handle<Account> const a2(dbc);
	a2->set_account_type(AccountType::pure_envelope);
	a2->set_name("fund - general");
	a2->set_commodity(Handle<Commodity>(dbc, 1));
	a2->set_description("");
	a2->set_visibility(Visibility::visible);
	CHECK(Account::no_user_pl_accounts_saved(dbc));
	a2->save();
	CHECK(!Account::no_user_pl_accounts_saved(dbc));
	a2->set_account_type(AccountType::asset);
	CHECK(!Account::no_user_pl_accounts_saved(dbc));
	a2->save();
	CHECK(Account::no_user_pl_accounts_saved(dbc));
	a2->set_account_type(AccountType::liability);
	a2->save();
	CHECK(Account::no_user_pl_accounts_saved(dbc));
	a2->set_account_type(AccountType::equity);
	a2->save();
	CHECK(Account::no_user_pl_accounts_saved(dbc));
	a2->set_account_type(AccountType::expense);
	CHECK(Account::no_user_pl_accounts_saved(dbc));
	a2->save();
	CHECK(!Account::no_user_pl_accounts_saved(dbc));
}

TEST_FIXTURE(TestFixture, test_get_and_set_account_name)
{
	JEWEL_LOG_TRACE();
	PhatbooksDatabaseConnection& dbc = *pdbc;
	Handle<Account> const a1(dbc, Account::id_for_name(dbc, "cash"));
	Handle<Account> const a2(dbc, Account::id_for_name(dbc, "food"));
	Id const id1 = a1->id();
	Id const id2 = a2->id();
	Handle<Account> a3(dbc);

	CHECK_EQUAL(a1->name(), "cash");
	CHECK(a1->name() != "Cash");
	CHECK(a1->name() != "");
	CHECK_EQUAL(a2->name(), "food");

	a2->set_name("hello");
	CHECK_EQUAL(a2->name(), "hello");
	Handle<Account> const a1b(dbc, id1);
	Handle<Account> const a2b(dbc, id2);
	CHECK_EQUAL(a1b->name(), "cash");
	CHECK_EQUAL(a2b->name(), "hello");
	a2b->ghostify();
	CHECK_EQUAL(a2->name(), "food");
	
	a3->set_name("hello");
	CHECK_EQUAL(a3->name(), "hello");

	a1->set_name("yep");
	a1->save();
	CHECK_EQUAL(a1->name(), "yep");
}

TEST_FIXTURE(TestFixture, test_get_and_set_account_commodity)
{
	// TODO
}

TEST_FIXTURE(TestFixture, test_get_and_set_account_type)
{
	// TODO
}

TEST_FIXTURE(TestFixture, test_account_super_type)
{
	// TODO
}

TEST_FIXTURE(TestFixture, test_get_and_set_account_description)
{
	// TODO
}

TEST_FIXTURE(TestFixture, test_get_and_set_account_visibility)
{
	// TODO
}

TEST_FIXTURE(TestFixture, test_account_technical_balance)
{
	// TODO
}

TEST_FIXTURE(TestFixture, test_account_friendly_balance)
{
	// TODO
}

TEST_FIXTURE(TestFixture, test_account_budget)
{
	// TODO
}

TEST_FIXTURE(TestFixture, test_account_budget_items)
{
	// TODO
}

}  // namespace test
}  // namespace phatbooks
