#include "entry.hpp"
#include "entry_impl.hpp"
#include "journal.hpp"
#include "phatbooks_database_connection.hpp"
#include "sqloxx/handle.hpp"
#include <boost/shared_ptr.hpp>
#include <string>

using boost::shared_ptr;
using sqloxx::Handle;
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
(	PhatbooksDatabaseConnection& p_database_connection
):
	m_impl(Handle<EntryImpl>(p_database_connection))
{
}

Entry::Entry
(	PhatbooksDatabaseConnection& p_database_connection,
	Id p_id
):
	m_impl(Handle<EntryImpl>(p_database_connection, p_id))
{
}


Entry
Entry::create_unchecked
(	PhatbooksDatabaseConnection& p_database_connection,
	Id p_id
)
{
	return Entry
	(	Handle<EntryImpl>::create_unchecked
		(	p_database_connection,
			p_id
		)
	);
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

void
Entry::set_whether_reconciled(bool p_is_reconciled)
{
	m_impl->set_whether_reconciled(p_is_reconciled);
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

bool
Entry::is_reconciled() const
{
	return m_impl->is_reconciled();
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

void
Entry::remove()
{
	m_impl->remove();
	return;
}

void
Entry::ghostify()
{
	m_impl->ghostify();
	return;
}

Entry::Entry(sqloxx::Handle<EntryImpl> const& p_handle):
	m_impl(p_handle)
{
}


}  // namespace phatbooks

