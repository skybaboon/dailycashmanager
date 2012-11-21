#include "entry.hpp"
#include "entry_impl.hpp"
#include "phatbooks_database_connection.hpp"
#include "sqloxx/database_connection.hpp"
#include <boost/shared_ptr.hpp>
#include <string>

using boost::shared_ptr;
using sqloxx::get_handle;
using std::string;


namespace phatbooks
{

class Account;

void
Entry::setup_tables(PhatbooksDatabaseConnection& dbc)
{
	EntryImpl::setup_tables(dbc);
	return;
}


Entry::Entry
(	shared_ptr<PhatbooksDatabaseConnection> const&
		 p_database_connection
):
	m_impl(get_handle<EntryImpl>(p_database_connection))
{
}

Entry::Entry
(	shared_ptr<PhatbooksDatabaseConnection> const&
		p_database_connection,
	Id p_id
):
	m_impl(get_handle<EntryImpl>(p_database_connection, p_id))
{
}

void
Entry::set_journal_id(Journal::Id p_journal_id)
{
	m_impl->set_journal_id(p_journal_id);
	return;
}

void
Entry::set_account(Account const& p_account)
{
	m_impl->set_account(p_account);
	return;
}

void
Entry::set_comment(string const& p_comment)
{
	m_impl->set_comment(p_comment);
	return;
}

void
Entry::set_amount(jewel::Decimal const& p_amount)
{
	m_impl->set_amount(p_amount);
	return;
}

string
Entry::comment() const
{
	return m_impl->comment();
}

jewel::Decimal
Entry::amount() const
{
	return m_impl->amount();
}

Account
Entry::account() const
{
	return m_impl->account();
}

Account::Id
Entry::id() const
{
	return m_impl->id();
}

void
Entry::save()
{
	m_impl->save();
	return;
}


}  // namespace phatbooks
